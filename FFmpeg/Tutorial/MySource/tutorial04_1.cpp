extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avstring.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
}

#include <sdl/SDL.h>
#include <sdl/SDL_thread.h>


#undef main


#include <stdio.h>
#include <assert.h>
#include <math.h>


#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)

#define FF_REFRESH_EVENT (SDL_USEREVENT)
#define FF_QUIT_EVENT (SDL_USEREVENT + 1)

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, VIDEO_PICTURE_QUEUE_SIZE)

#define FF_ALLOC_EVENT   (SDL_USEREVENT)
#define FF_QUIT_EVENT    (SDL_USEREVENT + 2)

typedef struct PacketQueue {
	AVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
} PacketQueue;


typedef struct Frame {
	AVFrame *frame;
	SDL_Overlay *bmp;
	int allocated;
	int width;
	int height;
} Frame;

typedef struct FrameQueue {
	Frame queue[FRAME_QUEUE_SIZE];
	int rindex;
	int windex;
	int size;
	int max_size;
	SDL_mutex *mutex;
	SDL_cond *cond;
	PacketQueue *pktq;
} FrameQueue;

typedef struct VideoState {
	SDL_Thread *read_tid;
	SDL_Thread *audio_tid;
	SDL_Thread *video_tid;
	AVFormatContext *ic;

	FrameQueue pictq;
	FrameQueue sampq;
	
	int audio_stream;
	AVStream *audio_st;
	PacketQueue audioq;
	uint8_t *audio_buf;
	unsigned int audio_buf_size; /* in bytes */
	int audio_buf_index; /* in bytes */
	struct SwrContext *swr_ctx;

	int video_stream;
	AVStream *video_st;
	PacketQueue videoq;
	struct SwsContext *img_convert_ctx;

	char filename[1024];

	/*ffplay.c把以下封装在了别的结构体里，比如Decoder*/
	AVCodecContext *audio_ctx;
	AVCodecContext *video_ctx;

} VideoState;

SDL_Surface     *screen;
SDL_mutex       *screen_mutex;

/* Since we only have one decoding thread, the Big Struct
can be global in case we need it. */
VideoState *global_video_state;

void packet_queue_init(PacketQueue *q) {
	memset(q, 0, sizeof(PacketQueue));
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}
int packet_queue_put(PacketQueue *q, AVPacket *pkt) {

	AVPacketList *pkt1;
	if (av_dup_packet(pkt) < 0) {
		return -1;
	}
	pkt1 = (AVPacketList*)av_malloc(sizeof(AVPacketList));
	if (!pkt1)
		return -1;
	pkt1->pkt = *pkt;
	pkt1->next = NULL;

	SDL_LockMutex(q->mutex);

	if (!q->last_pkt)
		q->first_pkt = pkt1;
	else
		q->last_pkt->next = pkt1;
	q->last_pkt = pkt1;
	q->nb_packets++;
	q->size += pkt1->pkt.size;
	SDL_CondSignal(q->cond);

	SDL_UnlockMutex(q->mutex);
	return 0;
}
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
	AVPacketList *pkt1;
	int ret;

	SDL_LockMutex(q->mutex);

	for (;;) {

		pkt1 = q->first_pkt;
		if (pkt1) {
			q->first_pkt = pkt1->next;
			if (!q->first_pkt)
				q->last_pkt = NULL;
			q->nb_packets--;
			q->size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			av_free(pkt1);
			ret = 1;
			break;
		}
		else if (!block) {
			ret = 0;
			break;
		}
		else {
			printf("waiting q->cond\n");
			SDL_CondWait(q->cond, q->mutex);
		}
	}
	SDL_UnlockMutex(q->mutex);
	return ret;
}

static int frame_queue_init(FrameQueue *f, PacketQueue *pktq)
{
	int i;
	memset(f, 0, sizeof(FrameQueue));
	if (!(f->mutex = SDL_CreateMutex()))
		return AVERROR(ENOMEM);
	if (!(f->cond = SDL_CreateCond()))
		return AVERROR(ENOMEM);
	f->pktq = pktq;
	f->max_size = FRAME_QUEUE_SIZE;
	for (i = 0; i < f->max_size; i++)
		if (!(f->queue[i].frame = av_frame_alloc()))
			return AVERROR(ENOMEM);
	return 0;
}
static void frame_queue_push(FrameQueue *f)
{
	if (++f->windex == f->max_size)
		f->windex = 0;
	SDL_LockMutex(f->mutex);
	f->size++;
	SDL_CondSignal(f->cond);
	SDL_UnlockMutex(f->mutex);
}
static Frame *frame_queue_peek_writable(FrameQueue *f)
{
	/* wait until we have space to put a new frame */
	SDL_LockMutex(f->mutex);
	while (f->size >= f->max_size) {
		SDL_CondWait(f->cond, f->mutex);
	}
	SDL_UnlockMutex(f->mutex);
	return &f->queue[f->windex];
}
static Frame *frame_queue_peek(FrameQueue *f)
{
	return &f->queue[(f->rindex) % f->max_size];
}
static Frame *frame_queue_peek_readable(FrameQueue *f)
{
	/* wait until we have a readable a new frame */
	SDL_LockMutex(f->mutex);
	while (f->size <= 0) {
		SDL_CondWait(f->cond, f->mutex);
	}
	SDL_UnlockMutex(f->mutex);
	return &f->queue[f->rindex % f->max_size];
}
static void frame_queue_next(FrameQueue *f)
{
	av_frame_unref(f->queue[f->rindex].frame);
	if (++f->rindex == f->max_size)
		f->rindex = 0;
	SDL_LockMutex(f->mutex);
	f->size--;
	SDL_CondSignal(f->cond);
	SDL_UnlockMutex(f->mutex);
}


/*
* ZSL
* 虽然名字叫 decode，其实并不解码，只是从 is->sampq 里拿出 af（audioframe,Frame类型）
* 把 af->frame(AVFrame类型)里的 data 经过 swr_convert() 之后，存入 is->audio_buf
* 返回存入的大小(即 resample 之后的大小)
*
*/
int audio_decode_frame(VideoState *is) {

	int resampled_data_size;
	Frame *af;
	af = frame_queue_peek_readable(&is->sampq);
	frame_queue_next(&is->sampq);
	if (!is->swr_ctx) {
		is->swr_ctx = swr_alloc_set_opts(NULL,
			AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, is->audio_ctx->sample_rate,
			av_get_default_channel_layout(is->audio_ctx->channels), is->audio_ctx->sample_fmt, is->audio_ctx->sample_rate,
			0, NULL);
		swr_init(is->swr_ctx);
	}
	const uint8_t **in = (const uint8_t **)af->frame->extended_data;
	uint8_t **out = &is->audio_buf;
	int out_size = av_samples_get_buffer_size(NULL, 2, af->frame->nb_samples, AV_SAMPLE_FMT_S16, 0);
	int len2;
	av_fast_malloc(&is->audio_buf, &is->audio_buf_size, out_size);
	len2 = swr_convert(is->swr_ctx, out, af->frame->nb_samples, in, af->frame->nb_samples);
	resampled_data_size = len2 * 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
	return resampled_data_size;
}

void sdl_audio_callback(void *userdata, Uint8 *stream, int len) {

	VideoState *is = (VideoState *)userdata;
	int len1, audio_size;

	while (len > 0) {
		if (is->audio_buf_index >= is->audio_buf_size) {
			/* We have already sent all our data; get more */
			audio_size = audio_decode_frame(is);
			if (audio_size < 0) {
				//TODO
				/* if error, just output silence */
			}
			else {
				is->audio_buf_size = audio_size;
			}
			is->audio_buf_index = 0;
		}
		len1 = is->audio_buf_size - is->audio_buf_index;
		if (len1 > len)
			len1 = len;
		memcpy(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1);
		len -= len1;
		stream += len1;
		is->audio_buf_index += len1;
	}
}

void video_display(VideoState *is) {
	SDL_Rect rect;
	Frame *vp;
	vp = frame_queue_peek(&is->pictq);
	if (vp->bmp) {
		rect.x = 0;
		rect.y = 0;
		rect.w = screen->w;
		rect.h = screen->h;
		SDL_LockMutex(screen_mutex);
		SDL_DisplayYUVOverlay(vp->bmp, &rect);
		SDL_UnlockMutex(screen_mutex);
	}
}


/* allocate a picture (needs to do that in main thread to avoid
potential locking problems */
void alloc_picture(void *userdata) {
	VideoState *is = (VideoState *)userdata;
	Frame *vp;
	vp = &is->pictq.queue[is->pictq.windex];
	if (vp->bmp) {
		// we already have one make another, bigger/smaller
		SDL_FreeYUVOverlay(vp->bmp);
		vp->bmp = NULL;
	}
	// Allocate a place to put our YUV image on that screen
	SDL_LockMutex(screen_mutex);
	vp->bmp = SDL_CreateYUVOverlay(is->video_ctx->width,
		is->video_ctx->height,
		SDL_YV12_OVERLAY,
		screen);
	SDL_UnlockMutex(screen_mutex);

	vp->width = is->video_ctx->width;
	vp->height = is->video_ctx->height;
	vp->allocated = 1;
}

int video_thread(void *arg)
{
	VideoState *is = (VideoState *)arg;
	AVPacket pkt1, *pkt = &pkt1;
	int got_frame = 0;
	AVFrame *frame = av_frame_alloc();
	Frame *vp;


	for (;;) {
		printf("video:packet_queue_get()\n");
		if (packet_queue_get(&is->videoq, pkt, 1) < 0) {
			// means we quit getting packets
			break;
		}
		// Decode video frame
		printf("decode_video2()\n");
		avcodec_decode_video2(is->video_ctx, frame, &got_frame, pkt);
		// Did we get a video frame?
		if (got_frame) 
		{
			vp = frame_queue_peek_writable(&is->pictq);
			if (!vp->bmp || !vp->allocated ||
				vp->width != frame->width ||
				vp->height != frame->height) 
			{
				SDL_Event event;
				vp->allocated = 0;
				vp->width = frame->width;
				vp->height = frame->height;
				/* the allocation must be done in the main thread to avoid
				locking problems. */
				event.type = FF_ALLOC_EVENT;
				event.user.data1 = is;
				SDL_PushEvent(&event);
				/* wait until the picture is allocated */
				SDL_LockMutex(is->pictq.mutex);
				while (!vp->allocated) {
					SDL_CondWait(is->pictq.cond, is->pictq.mutex);
				}
				SDL_UnlockMutex(is->pictq.mutex);
			}
			/* if the frame is not skipped, then display it */
			if (vp->bmp) 
			{
				AVPicture pict = { { 0 } };
				/* get a pointer on the bitmap */
				SDL_LockYUVOverlay(vp->bmp);
				pict.data[0] = vp->bmp->pixels[0];
				pict.data[1] = vp->bmp->pixels[2];
				pict.data[2] = vp->bmp->pixels[1];
				pict.linesize[0] = vp->bmp->pitches[0];
				pict.linesize[1] = vp->bmp->pitches[2];
				pict.linesize[2] = vp->bmp->pitches[1];
				is->img_convert_ctx = sws_getCachedContext(is->img_convert_ctx,
					vp->width, vp->height, is->video_ctx->pix_fmt, vp->width, vp->height,
					AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
				sws_scale(is->img_convert_ctx, frame->data, frame->linesize,
					0, vp->height, pict.data, pict.linesize);
				SDL_UnlockYUVOverlay(vp->bmp);
				/* 移动指针 */
				frame_queue_push(&is->pictq);
			}
			av_free_packet(pkt);
		}
	}
	av_frame_free(&frame);
	printf("video_thread exit\n");
	return 0;
}

static int audio_thread(void *arg)
{
	VideoState *is = (VideoState*)arg;
	AVFrame *frame = av_frame_alloc();
	Frame *af;
	int got_frame = 0;
	AVPacket pkt1, *pkt = &pkt1; 
	do {
		if (packet_queue_get(&is->audioq, pkt, 1) < 0) {
			break;
		}
		avcodec_decode_audio4(is->audio_ctx, frame, &got_frame, pkt);
		if (got_frame) {
			af = frame_queue_peek_writable(&is->sampq);
			av_frame_move_ref(af->frame, frame);
			frame_queue_push(&is->sampq);
			av_free_packet(pkt);
		}
	} while(1);
	av_frame_free(&frame);
	printf("audio_thread exit\n");
	return 0;
}

/* open a given stream. Return 0 if OK 
   这里还分别打开了音频解码线程、视频解码线程、音频播放线程
*/
static int stream_component_open(VideoState *is, int stream_index)
{
	AVFormatContext *ic = is->ic;
	AVCodecContext *avctx;
	AVCodec *codec;
	SDL_AudioSpec wanted_spec, spec;
	int ret = 0;

	if (stream_index < 0 || stream_index >= ic->nb_streams)
		return -1;
	avctx = ic->streams[stream_index]->codec;
	codec = avcodec_find_decoder(avctx->codec_id);
	avctx->codec_id = codec->id;
	avcodec_open2(avctx, codec, NULL);

	switch (avctx->codec_type) {
	case AVMEDIA_TYPE_AUDIO:
		wanted_spec.freq = avctx->sample_rate;
		wanted_spec.format = AUDIO_S16SYS;
		wanted_spec.channels = avctx->channels;
		wanted_spec.silence = 0;
		wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
		wanted_spec.callback = sdl_audio_callback;
		wanted_spec.userdata = is;
		if (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
			fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
			return -1;
		}
		is->audio_buf_size = 0;
		is->audio_buf_index = 0;
		is->audio_stream = stream_index;
		is->audio_st = ic->streams[stream_index];
		is->audio_ctx = avctx;
		is->audio_tid = SDL_CreateThread(audio_thread, is);
		SDL_PauseAudio(0);
		break;
	case AVMEDIA_TYPE_VIDEO:
		is->video_stream = stream_index;
		is->video_st = ic->streams[stream_index];
		is->video_ctx = avctx;
		is->video_tid = SDL_CreateThread(video_thread, is);
		break;
	default:
		break;
	}
}

static int read_thread(void *arg)
{
	VideoState *is = (VideoState*)arg;
	AVFormatContext *ic = NULL;
	int i;
	int st_index[AVMEDIA_TYPE_NB]; // 讲解一下
	AVPacket pkt1, *pkt = &pkt1;

	memset(st_index, -1, sizeof(st_index));

	ic = avformat_alloc_context();
	avformat_open_input(&ic, is->filename, NULL, NULL);
	is->ic = ic;

	avformat_find_stream_info(ic, NULL);
	
	for (i = 0; i<ic->nb_streams; i++) 
	{
		if (ic->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) 
			st_index[AVMEDIA_TYPE_VIDEO] = i;
		if (ic->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
			st_index[AVMEDIA_TYPE_AUDIO] = i;
	}

	/* open the streams，并开启相应的解码线程 */
	if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
		stream_component_open(is, st_index[AVMEDIA_TYPE_AUDIO]);
	}

	if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
		stream_component_open(is, st_index[AVMEDIA_TYPE_VIDEO]);
	}

	if (is->video_stream < 0 && is->audio_stream < 0) {
		av_log(NULL, AV_LOG_FATAL, "Failed to open file '%s' or configure filtergraph\n",
			is->filename);
		goto fail;
	}

	for (;;) {

		/* if the queue are full, no need to read more */
		// 暂时把以下这个判断注掉，不限制packet queue的大小
		// 不然的话，可能因为视频消耗packet比较慢（比如我们强制41ms刷新一次），导致视频queue超过max
		// 此时如果不再读包，会导致音频没包可解，听起来就是杂音
		/*
		if (is->audioq.size > MAX_AUDIOQ_SIZE ||
		is->videoq.size > MAX_VIDEOQ_SIZE) {
		SDL_Delay(10);
		continue;
		}
		*/
		if (av_read_frame(ic, pkt) < 0) {
			//break;
		}
		if (pkt->stream_index == is->audio_stream) {
			packet_queue_put(&is->audioq, pkt);
		}
		else if (pkt->stream_index == is->video_stream) {
			packet_queue_put(&is->videoq, pkt);
		}
		else {
			av_free_packet(pkt);
		}
	}
fail:
	/* close each stream */
	// TODO close streams
	if (ic) {
		avformat_close_input(&ic);
		is->ic = NULL;
	}
	SDL_Event event;
	event.type = FF_QUIT_EVENT;
	event.user.data1 = is;
	SDL_PushEvent(&event);
	printf("read_thread exit\n");
	return 0;
}


static void event_loop(VideoState *is)
{
	SDL_Event event;
	for (;;) {
		SDL_PumpEvents();
		while (!SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_ALLEVENTS)) {
			av_usleep(40000); //TODO 40000改为帧率分之一
			video_display(is);
			SDL_PumpEvents();
			}
		switch (event.type) {
		case SDL_QUIT:
		case FF_QUIT_EVENT:
			SDL_Quit();
			exit(0);
			break;
		case FF_ALLOC_EVENT:
			printf("alloc_picture\n");
			alloc_picture(event.user.data1);
			break;
		default:
			break;
		}
	}
}


int main(int argc, char **argv)
{
	int flags;
	VideoState *is;
	av_register_all();

	flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;
	if (SDL_Init(flags)) {
		av_log(NULL, AV_LOG_FATAL, "Could not initialize SDL - %s\n", SDL_GetError());
		exit(1);
	}


	screen = SDL_SetVideoMode(640, 480, 0, 0);
	screen_mutex = SDL_CreateMutex();

	/* ffplay.c 把下述内容封装到了函数 stream_open() 里 */
	is = (VideoState*)av_mallocz(sizeof(VideoState));
	av_strlcpy(is->filename, argv[1], sizeof(is->filename));
	frame_queue_init(&is->pictq, &is->videoq);
	frame_queue_init(&is->sampq, &is->audioq);
	packet_queue_init(&is->videoq);
	packet_queue_init(&is->audioq);
	is->read_tid = SDL_CreateThread(read_thread, is); /* 开启读包线程，里面会再开启音频解码线程、音频播放线程、视频解码线程*/

	/* event_loop 目前只处理退出和 allocate Pictur*/
	event_loop(is);
	return 0;
}
