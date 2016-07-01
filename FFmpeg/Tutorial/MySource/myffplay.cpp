
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
}
#include <sdl/SDL.h>
#include <sdl/SDL_thread.h>

#undef main /* Prevents SDL from overriding main() */

#include <stdio.h>
#include <assert.h>
#include <Windows.h>

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

#define SDL_AUDIO_BUFFER_SIZE 4096   // SDL audio buffer in samples
#define MAX_AUDIO_FRAME_SIZE 192000  // audio buffer in bytes

#define PICTURE_QUEUE_SIZE 16

#define NAN            av_int2float(0x7fc00000)

//暂且作为全局变量
int quit = 0;
SDL_Surface     *screen;

typedef struct PacketQueue {
	AVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
} PacketQueue;

void packet_queue_init(PacketQueue *q) {
	memset(q, 0, sizeof(PacketQueue));
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}

/* 会在堆空间上开辟一个新的 pkt，把传进来的pkt拷给它*/
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
			SDL_CondWait(q->cond, q->mutex);
		}
	}
	SDL_UnlockMutex(q->mutex);
	return ret;
}

typedef struct VideoPicture {
	SDL_Overlay *bmp;
	int width, height; /* source height & width */
	int allocated;
	double pts;
	double duration;
} VideoPicture;

typedef struct PictureQueue {
	VideoPicture queue[PICTURE_QUEUE_SIZE];
	int rindex;
	int windex;
	int size;
	int max_size;
	int rindex_shown;
	SDL_mutex *mutex;
	SDL_cond *cond;
	PacketQueue *pktq;
} PictureQueue;

static int pict_queue_init(PictureQueue *f, PacketQueue *pktq, int max_size)
{
	int i;
	memset(f, 0, sizeof(PictureQueue));
	if (!(f->mutex = SDL_CreateMutex()))
		return AVERROR(ENOMEM);
	if (!(f->cond = SDL_CreateCond()))
		return AVERROR(ENOMEM);
	f->pktq = pktq;
	f->max_size = min(max_size, PICTURE_QUEUE_SIZE);
	for (i = 0; i < f->max_size; i++)
		f->queue[i] = *(VideoPicture*)av_mallocz(sizeof(VideoPicture));
	return 0;
}
static VideoPicture *pict_queue_peek(PictureQueue *f)
{
	return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}
static VideoPicture *pict_queue_peek_last(PictureQueue *f)
{
	return &f->queue[f->rindex];
}
static void pict_queue_push(PictureQueue *f)
{
	if (++f->windex == f->max_size)
		f->windex = 0;
	SDL_LockMutex(f->mutex);
	f->size++;
	SDL_CondSignal(f->cond);
	SDL_UnlockMutex(f->mutex);
}
static void pict_queue_next(PictureQueue *f)
{
	if (!f->rindex_shown) {
		f->rindex_shown = 1;
		return;
	}
	if (++f->rindex == f->max_size)
		f->rindex = 0;
	SDL_LockMutex(f->mutex);
	f->size--;
	SDL_CondSignal(f->cond);
	SDL_UnlockMutex(f->mutex);
}


typedef struct VideoState {

	AVFormatContext *pFormatCtx;
	int             videoStream, audioStream;

	double          audio_clock;
	AVStream        *audio_st;
	AVCodecContext  *audio_ctx;
	PacketQueue     audioq;
	uint8_t         audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
	unsigned int    audio_buf_size;
	unsigned int    audio_buf_index;
	AVFrame         audio_frame;
	AVPacket        audio_pkt;
	uint8_t         *audio_pkt_data;
	int             audio_pkt_size;
	int             audio_hw_buf_size;
	SwrContext		*swr_ctx;

	double          frame_timer;
	double          frame_last_pts;
	double          frame_last_delay;
	double          video_clock; ///<pts of last decoded frame / predicted pts of next decoded frame
	AVStream        *video_st;
	AVCodecContext  *video_ctx;
	PacketQueue     videoq;
	struct SwsContext *sws_ctx;

	PictureQueue    pictq;

	SDL_Thread      *parse_tid;
	SDL_Thread      *video_tid;

	char            filename[1024];
	int             quit;
} VideoState;

// 把 is->pictq 写指针位置的 picture 分配了
void alloc_picture(void *userdata) {

	VideoState *is = (VideoState *)userdata;
	VideoPicture *vp;

	vp = &is->pictq.queue[is->pictq.windex];
	if (vp->bmp) {
		// we already have one make another, bigger/smaller
		SDL_FreeYUVOverlay(vp->bmp);
	}
	// Allocate a place to put our YUV image on that screen
	SDL_LockMutex(is->pictq.mutex);
	vp->bmp = SDL_CreateYUVOverlay(is->video_ctx->width,
		is->video_ctx->height,
		SDL_YV12_OVERLAY,
		screen);
	SDL_UnlockMutex(is->pictq.mutex);

	vp->width = is->video_ctx->width;
	vp->height = is->video_ctx->height;
	vp->allocated = 1;

}

// 往 is->pictq 写入一幅图，移动写指针
int queue_picture(VideoState *is, AVFrame *pFrame, double pts, double duration) {
	VideoPicture *vp;
	int dst_pix_fmt;
	AVPicture pict;
	/* wait until we have space for a new pic */
	SDL_LockMutex(is->pictq.mutex);
	while (is->pictq.size >= PICTURE_QUEUE_SIZE &&
		!is->quit) {
		SDL_CondWait(is->pictq.cond, is->pictq.mutex);
	}
	SDL_UnlockMutex(is->pictq.mutex);

	if (is->quit)
		return -1;

	// windex is set to 0 initially
	vp = &is->pictq.queue[is->pictq.windex];

	/* allocate or resize the buffer! */
	if (!vp->bmp ||
		vp->width != is->video_ctx->width ||
		vp->height != is->video_ctx->height) {
		vp->allocated = 0;
		alloc_picture(is);
		if (is->quit) {
			return -1;
		}
	}

	/* We have a place to put our picture on the queue */

	if (vp->bmp) {

		SDL_LockYUVOverlay(vp->bmp);
		vp->pts = pts;
		vp->duration = duration;
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
		
		vp->pts = pts;
		vp->duration = duration;

		/* 移动写指针 */
		pict_queue_push(&is->pictq);
	}
	return 0;
}


double get_audio_clock(VideoState *is) {
	double pts;
	int hw_buf_size, bytes_per_sec, n;

	pts = is->audio_clock; /* maintained in the audio thread */
	hw_buf_size = is->audio_buf_size - is->audio_buf_index;
	bytes_per_sec = 0;
	n = is->audio_ctx->channels * 2;
	if (is->audio_st) {
		bytes_per_sec = is->audio_ctx->sample_rate * n;
	}
	if (bytes_per_sec) {
		pts -= (double)hw_buf_size / bytes_per_sec;
	}
	return pts;
}


// return 0：发完 event， timer 就失效。即调用一次，只发一次 event。
static Uint32 send_refresh_video_event(Uint32 interval, void *opaque) {
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.data1 = opaque;
	SDL_PushEvent(&event);
	return 0; 
}

// 从 videoq 拿出 pkt，解码成 frame，frame 转换成 pict，放入 pictq
int decode_video_thread(void *param) {
	VideoState *is = (VideoState*)param;
	AVFrame         *pFrame = NULL;
	int frameFinished;
	AVPacket packet;
	pFrame = av_frame_alloc();
	VideoPicture    pict;
	AVRational tb = is->video_st->time_base;
	AVRational frame_rate = av_guess_frame_rate(is->pFormatCtx, is->video_st, NULL);
	double  pts, duration;

	for (;;) {
		packet_queue_get(&is->videoq, &packet, 1);
		avcodec_decode_video2(is->video_ctx, pFrame, &frameFinished, &packet);
		
		/* 因为 ffplay 里自己封装了一个 Frame，所以它这里直接用了 fram->pts，而我们要样 best effort timestamp */
		pts = (av_frame_get_best_effort_timestamp(pFrame) == AV_NOPTS_VALUE) ? NAN : av_frame_get_best_effort_timestamp(pFrame) * av_q2d(tb);
		duration = (frame_rate.num && frame_rate.den ? av_q2d(av_inv_q(frame_rate)) : 0);
		if (frameFinished) {
			queue_picture(is, pFrame, pts, duration);
		}
		else {
			av_free_packet(&packet);
		}
	}
	return 0;
}

void display_pict(void *param) {
	VideoState* is = (VideoState*)param;
	SDL_Rect     rect;
	VideoPicture *pict;

	pict = &is->pictq.queue[is->pictq.rindex];
	rect.x = 0;
	rect.y = 0;
	rect.w = pict->width;
	rect.h = pict->height;
	SDL_LockMutex(is->pictq.mutex);
	SDL_DisplayYUVOverlay(pict->bmp, &rect);
	SDL_UnlockMutex(is->pictq.mutex);
}

// 从 pictq 取出一幅图，看看应该什么时候显示，起个一次性timer，让它在那时候显示。
// TODO：如果该幅图像已经过期了，则丢弃。
int video_refresh(void *param) {
	VideoState *is = (VideoState*)param;
	double last_duration, duration, delay, ref_clock, diff;
	VideoPicture *vp, *lastvp;

	for (;;)
	{
		/*lastvp：刚刚显示过的那个pict*/
		lastvp = pict_queue_peek_last(&is->pictq);
	
		/*vp：想要显示的那个pict*/
		vp = pict_queue_peek(&is->pictq);
	
		/*
		刚刚显示过的那个pict的duration，
		那个图片的duration是多久，我们就需要 delay多久再显示当前图片
		*/
		last_duration = vp->pts - lastvp->pts;
	
		/*
		如果相邻两帧pts相同，导致计算出来的上一帧的duration等于0
		则以上一帧的 pict.duration 作为 last_duration 
		( pict 的 duration 是 framerate 的倒数）
		*/
		if (isnan(last_duration) || last_duration <= 0)
			last_duration = lastvp ->duration;

		/* ----------- */
		/*音视频同步： 调整即将显示的图片的 delay*/
		delay = last_duration;
		ref_clock = get_audio_clock(is);
		diff = vp->pts - ref_clock;
		if (diff <= -0.015) {
			delay = 0;
		}
		else if (diff >= 0.015) {
			delay = 2 * delay;
		}

		if (delay == 0) 
			delay = 0.010;

		/* ----------- */

		// TODO: 判断是否需要丢帧，需要引入 time 变量


		// 起一个一次性 timer，到时间后发个 event，显示当前pict
		//SDL_AddTimer((int)(delay * 1000 + 0.5),send_refresh_video_event,is);
		av_usleep((int64_t)(delay* 1000000.0));
		SDL_Event event;
		event.type = SDL_USEREVENT;
		event.user.data1 = is;
		SDL_PushEvent(&event);

		//一次循环结束，取出的图片处理完成（显示了或者丢掉了），移动 pictq 的读指针
		pict_queue_next(&is->pictq);
	}

	return 0;
}


int read_packet_thread(void *param) {
	VideoState *is = (VideoState*)param;
	/*不用担心这个临时变量每次被覆盖导致pkt队列里的pkt被冲掉，因为packet_queue_put的时候会在堆上分配一个
	packet，然后把这个packt拷贝给堆上分配的那个，然后把堆上那个入队*/
	AVPacket packet;
	while (av_read_frame(is->pFormatCtx, &packet) >= 0) {
		// Is this a packet from the video stream?
		if (packet.stream_index == is->videoStream) {
			packet_queue_put(&is->videoq, &packet);
		}
		else if (packet.stream_index == is->audioStream) {
			packet_queue_put(&is->audioq, &packet);
		}
		else {
			av_free_packet(&packet);
		}
	}
	return 0;
}

int audio_decode_frame(VideoState *is, uint8_t *audio_buf, int buf_size) {

	int len1, resampled_data_size = 0;
	AVPacket *pkt = &is->audio_pkt;
	int n;
	double pts;
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
			pts = is->audio_clock;
			// 2 means: 2 bytes/sample
			n = 2 * is->audio_ctx->channels;
			// 这里除出来是个秒数，表示这次解码出来的音频数据能播几秒
			is->audio_clock += (double)resampled_data_size /
				(double)(n * is->audio_ctx->sample_rate);
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

int main(int argc, char *argv[]) {
	VideoState      *is = NULL;
	AVCodecContext  *pCodecCtxOrig = NULL;
	AVCodecContext  *aCodecCtxOrig = NULL;
	AVCodec         *aCodec = NULL;
	AVCodec         *vCodec = NULL;
	SDL_Event       event;
	SDL_AudioSpec   wanted_spec, spec;
	int i = 0;

	is = (VideoState*)av_mallocz(sizeof(VideoState));

	if (argc < 2) {
		fprintf(stderr, "Usage: test <file>\n");
		exit(1);
	}
	// Register all formats and codecs
	av_register_all();

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
		exit(1);
	}

	// Open video file
	if (avformat_open_input(&is->pFormatCtx, argv[1], NULL, NULL) != 0)
		return -1; // Couldn't open file

				   // Retrieve stream information
	if (avformat_find_stream_info(is->pFormatCtx, NULL)<0)
		return -1; // Couldn't find stream information

	// Dump information about file onto standard error
	av_dump_format(is->pFormatCtx, 0, argv[1], 0);

	for (i = 0; i<is->pFormatCtx->nb_streams; i++) {
		if (is->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			is->videoStream = i;
			is->video_st = is->pFormatCtx->streams[i];
		}
		if (is->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			is->audioStream = i;
			is->audio_st = is->pFormatCtx->streams[i];
		}
	}


	//开启读 packet 线程
	packet_queue_init(&is->audioq);
	memset(&is->audio_pkt, 0, sizeof(is->audio_pkt));
	//packet_queue_init(&is->videoq);
	SDL_CreateThread(read_packet_thread, is);

	//打开 audio codec/codc context
	aCodecCtxOrig = is->pFormatCtx->streams[is->audioStream]->codec;
	aCodec = avcodec_find_decoder(aCodecCtxOrig->codec_id);
	if (!aCodec) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1;
	}

	// Copy context
	is->audio_ctx = avcodec_alloc_context3(aCodec);
	if (avcodec_copy_context(is->audio_ctx, aCodecCtxOrig) != 0) {
		fprintf(stderr, "Couldn't copy codec context");
		return -1; // Error copying codec context
	}

	// Set audio settings from codec info
	wanted_spec.freq = is->audio_ctx->sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = is->audio_ctx->channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
	wanted_spec.callback = audio_callback;
	wanted_spec.userdata = is;
	// 需要先把解出来的 raw audio 转换成 SDL 需要的格式
	// 根据 raw audio 的格式 和 SDL 的格式设置 swr_ctx
	is->swr_ctx = swr_alloc_set_opts(NULL,
		AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, is->audio_ctx->sample_rate,
		av_get_default_channel_layout(is->audio_ctx->channels), is->audio_ctx->sample_fmt, is->audio_ctx->sample_rate,
		0, NULL);
	//初始化 swr_ctx
	swr_init(is->swr_ctx);

	if (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
		fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
		return -1;
	}

	avcodec_open2(is->audio_ctx, aCodec, NULL);

	// 开始音频解码/播放线程
	SDL_PauseAudio(0);

	//打开视频 codec/codec context 
	pCodecCtxOrig = is->pFormatCtx->streams[is->videoStream]->codec;

	// Find the decoder for the video stream
	vCodec = avcodec_find_decoder(pCodecCtxOrig->codec_id);
	if (vCodec == NULL) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1; // Codec not found
	}

	// Copy context
	is->video_ctx = avcodec_alloc_context3(vCodec);
	if (avcodec_copy_context(is->video_ctx, pCodecCtxOrig) != 0) {
		fprintf(stderr, "Couldn't copy codec context");
		return -1; // Error copying codec context
	}

	// Open codec
	if (avcodec_open2(is->video_ctx, vCodec, NULL)<0)
		return -1; // Could not open codec
	is->sws_ctx = sws_getContext(is->video_ctx->width,
		is->video_ctx->height,
		is->video_ctx->pix_fmt,
		is->video_ctx->width,
		is->video_ctx->height,
		PIX_FMT_YUV420P,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
		);

#ifndef __DARWIN__
	screen = SDL_SetVideoMode(is->video_ctx->width, is->video_ctx->height, 0, 0);
#else
	screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 24, 0);
#endif
	if (!screen) {
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}

	//开启视频解码线程
	pict_queue_init(&is->pictq, &is->videoq, 16);
	SDL_CreateThread(decode_video_thread, is);

	//开启视频刷新线程
	SDL_CreateThread(video_refresh, is);
	
	// 进入主循环
	for (;;) {
		SDL_WaitEvent(&event);
		switch (event.type) {
		case SDL_QUIT:
			SDL_Quit();
			return 0;
			break;
		case SDL_USEREVENT:
			display_pict(event.user.data1);
			break;
		default:
			break;
		}
	}
	return 0;
}
