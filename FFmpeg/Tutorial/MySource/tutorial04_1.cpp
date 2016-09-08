extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avstring.h>
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
	int abort_request; /* 停止读包线程标志 */
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

		if (global_video_state->quit) {
			ret = -1;
			break;
		}

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

int audio_decode_frame(VideoState *is, uint8_t *audio_buf, int buf_size) {

	int len1, resampled_data_size = 0;
	AVPacket *pkt = &is->audio_pkt;

	for (;;) {
		while (is->audio_pkt_size > 0) {
			int got_frame = 0;
			len1 = avcodec_decode_audio4(is->audio_ctx, &is->audio_frame, &got_frame, pkt);
			if (len1 < 0) {
				/* if error, skip frame */
				is->audio_pkt_size = 0;
				break;
			}
			if (got_frame) {

				// ---------------


				//swr_set_compensation(swr_ctx,0, frame.nb_samples*44100/ aCodecCtx->sample_rate);

				//准备调用 swr_convert 的其他4个必须参数: out,out_samples_per_ch,in,in_samples_per_ch
				uint8_t **out = &audio_buf;
				const uint8_t **in = (const uint8_t **)is->audio_frame.extended_data;
				//调用 swr_convert 进行转换
				int len2 = 0;
				len2 = swr_convert(is->swr_ctx, out, is->audio_frame.nb_samples, in, is->audio_frame.nb_samples);
				resampled_data_size = len2 * 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
				//memcpy(audio_buf, frame.data[0], data_size);

				// ----------------

			}
			is->audio_pkt_data += len1;
			is->audio_pkt_size -= len1;
			if (resampled_data_size <= 0) {
				/* No data yet, get more frames */
				continue;
			}
			/* We have data, return it and come back for more later */
			return resampled_data_size;
		}
		if (pkt->data)
			av_free_packet(pkt);

		if (is->quit) {
			return -1;
		}
		/* next packet */
		if (packet_queue_get(&is->audioq, pkt, 1) < 0) {
			return -1;
		}
		is->audio_pkt_data = pkt->data;
		is->audio_pkt_size = pkt->size;
	}
}

void audio_callback(void *userdata, Uint8 *stream, int len) {

	VideoState *is = (VideoState *)userdata;
	int len1, audio_size;

	while (len > 0) {
		if (is->audio_buf_index >= is->audio_buf_size) {
			/* We have already sent all our data; get more */
			audio_size = audio_decode_frame(is, is->audio_buf, sizeof(is->audio_buf));
			if (audio_size < 0) {
				/* If error, output silence */
				is->audio_buf_size = 1024;
				memset(is->audio_buf, 0, is->audio_buf_size);
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

static Uint32 sdl_refresh_timer_cb(Uint32 interval, void *opaque) {
	SDL_Event event;
	event.type = FF_REFRESH_EVENT;
	event.user.data1 = opaque;
	SDL_PushEvent(&event);
	return 0; /* 0 means stop timer */
}

/* schedule a video refresh in 'delay' ms */
/*
不要用 SDL Timer 了：
1. 比如你定了一个 40 毫秒的 timer，可能10毫秒甚至1毫秒就到时间了
2. ffplay（ffmpeg2.8） 已经不用 SDL timer 了，使用 av_usleep() 代替。
TODO，替换掉
*/
static void schedule_refresh(VideoState *is, int delay) {
	SDL_AddTimer(delay, sdl_refresh_timer_cb, is);
}

void video_display(VideoState *is) {

	SDL_Rect rect;
	VideoPicture *vp;

	vp = &is->pictq[is->pictq_rindex];
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

/*
每次timer到时间会进来（timer到时间发 FF_REFRESH_EVENT，收到 FF_REFRESH_EVENT 会进来）
一个timer只进一次timer就失效了。不过本函数里面会再起一个timer。
从is->pictq拿出一个 VideoPicture 进行显示，然后pictq的读指针向前移动一步
*/
void video_refresh_timer(void *userdata) {

	VideoState *is = (VideoState *)userdata;
	VideoPicture *vp;

	if (is->video_st) {
		if (is->pictq_size == 0) {
			schedule_refresh(is, 1);
		}
		else {
			//vp = &is->pictq[is->pictq_rindex];
			/* Now, normally here goes a ton of code
			about timing, etc. we're just going to
			guess at a delay for now. You can
			increase and decrease this value and hard code
			the timing - but I don't suggest that ;)
			We'll learn how to do it for real later.
			*/
			schedule_refresh(is, 41);

			/* show the picture! */
			video_display(is);

			/* update queue for next picture! */
			if (++is->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE) {
				is->pictq_rindex = 0;
			}
			SDL_LockMutex(is->pictq_mutex);
			is->pictq_size--;
			SDL_CondSignal(is->pictq_cond);
			SDL_UnlockMutex(is->pictq_mutex);
		}
	}
	else {
		schedule_refresh(is, 100);
	}
}

/*
为写指针所在的VideoPicture（is->pictq[is->pictq_windex]）在堆空间分配一个 SDL_Overlay
*/
void alloc_picture(void *userdata) {

	VideoState *is = (VideoState *)userdata;
	VideoPicture *vp;

	vp = &is->pictq[is->pictq_windex];
	if (vp->bmp) {
		// we already have one make another, bigger/smaller
		SDL_FreeYUVOverlay(vp->bmp);
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

int queue_picture(VideoState *is, AVFrame *pFrame) {

	VideoPicture *vp;
	int dst_pix_fmt;
	AVPicture pict;

	/* wait until we have space for a new pic */
	SDL_LockMutex(is->pictq_mutex);
	while (is->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE &&
		!is->quit) {
		SDL_CondWait(is->pictq_cond, is->pictq_mutex);
	}
	SDL_UnlockMutex(is->pictq_mutex);

	if (is->quit)
		return -1;

	// windex is set to 0 initially
	vp = &is->pictq[is->pictq_windex];

	/* allocate or resize the buffer! */
	if (!vp->bmp ||
		vp->width != is->video_ctx->width ||
		vp->height != is->video_ctx->height) {
		SDL_Event event;

		vp->allocated = 0;
		alloc_picture(is);
		if (is->quit) {
			return -1;
		}
	}

	/* We have a place to put our picture on the queue */

	if (vp->bmp) {

		SDL_LockYUVOverlay(vp->bmp);

		dst_pix_fmt = PIX_FMT_YUV420P;
		/* point pict at the queue */

		pict.data[0] = vp->bmp->pixels[0];
		pict.data[1] = vp->bmp->pixels[2];
		pict.data[2] = vp->bmp->pixels[1];

		pict.linesize[0] = vp->bmp->pitches[0];
		pict.linesize[1] = vp->bmp->pitches[2];
		pict.linesize[2] = vp->bmp->pitches[1];

		// Convert the image into YUV format that SDL uses
		sws_scale(is->sws_ctx, (uint8_t const * const *)pFrame->data,
			pFrame->linesize, 0, is->video_ctx->height,
			pict.data, pict.linesize);

		SDL_UnlockYUVOverlay(vp->bmp);
		/* now we inform our display thread that we have a pic ready */
		if (++is->pictq_windex == VIDEO_PICTURE_QUEUE_SIZE) {
			is->pictq_windex = 0;
		}
		SDL_LockMutex(is->pictq_mutex);
		is->pictq_size++;
		SDL_UnlockMutex(is->pictq_mutex);
	}
	return 0;
}

/*
从 is->videoq 拿出一个 packet 解码，把解出来的 frame 里的图像转为 VideoPicture 放到 is->pictq 里。然后释放掉拿出来的 packet。
目前 is->pictq 的长度是 1：#define VIDEO_PICTURE_QUEUE_SIZE 1，所以每次只能放进去一个frame，放的过程中如果发现 pictq 满了，则等待 pictq_cond 发生

*/
int video_thread(void *arg) {
	VideoState *is = (VideoState *)arg;
	AVPacket pkt1, *packet = &pkt1;
	int frameFinished;
	AVFrame *pFrame;

	pFrame = av_frame_alloc();

	for (;;) {
		if (packet_queue_get(&is->videoq, packet, 1) < 0) {
			// means we quit getting packets
			break;
		}
		// Decode video frame
		avcodec_decode_video2(is->video_ctx, pFrame, &frameFinished, packet);
		// Did we get a video frame?
		if (frameFinished) {
			if (queue_picture(is, pFrame) < 0) {
				break;
			}
		}
		av_free_packet(packet);
	}
	av_frame_free(&pFrame);
	return 0;
}

/* open a given stream. Return 0 if OK 
   这里还分别打开了音频解码线程和视频解码线程
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
		wanted_spec.callback = audio_callback;
		wanted_spec.userdata = is;
		if (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
			fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
			return -1;
		}
		is->audio_buf_size = 0;
		is->audio_buf_index = 0;
		is->audio_stream = stream_index;
		is->audio_st = ic->streams[stream_index];
		is->audio_tid = SDL_CreateThread(audio_thread, is);
		SDL_PauseAudio(0);
		break;
	case AVMEDIA_TYPE_VIDEO:
		is->video_stream = stream_index;
		is->video_st = ic->streams[stream_index];
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
			break;
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
	/* wait until the end */
	while (!is->abort_request) {
		SDL_Delay(100);
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
	return 0;
}

int main(int argc, char **argv)
{
	VideoState *is;
	is = (VideoState*)av_mallocz(sizeof(VideoState));
	av_strlcpy(is->filename, argv[1], sizeof(is->filename));

	/* register all codecs, demux and protocols */
	av_register_all();

	frame_queue_init(&is->pictq, &is->videoq);
	frame_queue_init(&is->sampq, &is->audioq);

	packet_queue_init(&is->videoq);
	packet_queue_init(&is->audioq);

	is->read_tid = SDL_CreateThread(read_thread, is);

	/* --------- ------------------ ---------------- */

	int flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;

#if !defined(_WIN32) && !defined(__APPLE__)
	flags |= SDL_INIT_EVENTTHREAD; /* Not supported on Windows or Mac OS X */
#endif
	if (SDL_Init(flags)) {
		av_log(NULL, AV_LOG_FATAL, "Could not initialize SDL - %s\n", SDL_GetError());
		av_log(NULL, AV_LOG_FATAL, "(Did you set the DISPLAY variable?)\n");
		exit(1);
	}

	SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);

	if (av_lockmgr_register(lockmgr)) {
		av_log(NULL, AV_LOG_FATAL, "Could not initialize lock manager!\n");
		do_exit(NULL);
	}

	av_init_packet(&flush_pkt);
	flush_pkt.data = (uint8_t *)&flush_pkt;

	is = stream_open(input_filename, file_iformat);
	if (!is) {
		av_log(NULL, AV_LOG_FATAL, "Failed to initialize VideoState!\n");
		do_exit(NULL);
	}

	event_loop(is);

	/* never returns */

	return 0;
}
