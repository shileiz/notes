
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
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


//������Ϊȫ�ֱ���
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

		if (quit) {
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

// �� is->pictq дָ��λ�õ� picture ������
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

// �� is->pictq д��һ��ͼ���ƶ�дָ��
int queue_picture(VideoState *is, AVFrame *pFrame, double pts, double duration) {
	VideoPicture *vp;
	int dst_pix_fmt;
	AVPicture pict;
	/* wait until we have space for a new pic */
	SDL_LockMutex(is->pictq.mutex);
	while (is->pictq.size <= PICTURE_QUEUE_SIZE &&
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
		
		/* �ƶ�дָ�� */
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


// return 0������ event�� timer ��ʧЧ��������һ�Σ�ֻ��һ�� event��
static Uint32 send_refresh_video_event(Uint32 interval, void *opaque) {
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.data1 = opaque;
	SDL_PushEvent(&event);
	return 0; 
}

// �� videoq �ó� pkt������� frame��frame ת���� pict������ pictq
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
		pts = (pFrame->pts == AV_NOPTS_VALUE) ? NAN : pFrame->pts * av_q2d(tb);
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

// �� pictq ȡ��һ��ͼ������Ӧ��ʲôʱ����ʾ�����һ����timer����������ʱ����ʾ��
// TODO������÷�ͼ���Ѿ������ˣ�������
int video_refresh(void *param) {

	VideoState *is = (VideoState*)param;
	double last_duration, duration, delay, ref_clock, diff;
	VideoPicture *vp, *lastvp;

	for (;;)
	{
		/*lastvp���ո���ʾ�����Ǹ�pict*/
		lastvp = pict_queue_peek_last(&is->pictq);
	
		/*vp����Ҫ��ʾ���Ǹ�pict*/
		vp = pict_queue_peek(&is->pictq);
	
		/*
		�ո���ʾ�����Ǹ�pict��duration��
		�Ǹ�ͼƬ��duration�Ƕ�ã����Ǿ���Ҫ delay�������ʾ��ǰͼƬ
		*/
		last_duration = vp->pts - lastvp->pts;
	
		/*
		���������֡pts��ͬ�����¼����������һ֡��duration����0
		������һ֡�� pict.duration ��Ϊ last_duration 
		( pict �� duration �� framerate �ĵ�����
		*/
		if (last_duration == NAN || last_duration <= 0)
			last_duration = lastvp ->duration;

		/* ----------- */
		/*����Ƶͬ���� ����������ʾ��ͼƬ�� delay*/
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

		// TODO: �ж��Ƿ���Ҫ��֡����Ҫ���� time ����


		// ��һ��һ���� timer����ʱ��󷢸� event����ʾ��ǰpict
		SDL_AddTimer((int)(delay * 1000 + 0.5),send_refresh_video_event,is);

		//һ��ѭ��������ȡ����ͼƬ������ɣ���ʾ�˻��߶����ˣ����ƶ� pictq �Ķ�ָ��
		pict_queue_next(&is->pictq);
	}

	return 0;
}

int read_packet_thread(void *param) {
	VideoState *is = (VideoState*)param;
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

	static AVPacket pkt;
	static uint8_t *audio_pkt_data = NULL;
	static int audio_pkt_size = 0;
	static AVFrame frame;
	int len1, resampled_data_size = 0;

	for (;;) {
		while (audio_pkt_size > 0) {
			int got_frame = 0;
			len1 = avcodec_decode_audio4(is->audio_ctx, &frame, &got_frame, &pkt);
			if (len1 < 0) {
				/* if error, skip frame */
				audio_pkt_size = 0;
				break;
			}
			audio_pkt_data += len1;
			audio_pkt_size -= len1;

			if (got_frame) {

				// ---------------

				//׼������ swr_convert ������4���������: out,out_samples_per_ch,in,in_samples_per_ch
				uint8_t **out = &audio_buf;
				const uint8_t **in = (const uint8_t **)frame.extended_data;
				//int out_samples_per_ch = buf_size/ (av_get_bytes_per_sample(AV_SAMPLE_FMT_S16)*2);
				//���� swr_convert ����ת��
				int len2 = 0;
				len2 = swr_convert(is->swr_ctx, out, frame.nb_samples, in, frame.nb_samples);
				resampled_data_size = len2 * 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
				//memcpy(audio_buf, frame.data[0], data_size);

				// ----------------

			}
			/* We have data, return it and come back for more later */
			return resampled_data_size;
		}
		if (pkt.data)
			av_free_packet(&pkt);

		if (quit) {
			return -1;
		}

		if (packet_queue_get(&is->audioq, &pkt, 1) < 0) {
			return -1;
		}
		audio_pkt_data = pkt.data;
		audio_pkt_size = pkt.size;
	}

}

void audio_callback(void *userdata, Uint8 *stream, int len) {

	VideoState *is = (VideoState *)userdata;
	int len1, audio_size;

	static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
	static unsigned int audio_buf_size = 0;
	static unsigned int audio_buf_index = 0;

	while (len > 0) {
		if (audio_buf_index >= audio_buf_size) {
			/* We have already sent all our data; get more */
			audio_size = audio_decode_frame(is, audio_buf, sizeof(audio_buf));
			if (audio_size < 0) {
				/* If error, output silence */
				audio_buf_size = 1024; // arbitrary?
				memset(audio_buf, 0, audio_buf_size);
			}
			else {
				audio_buf_size = audio_size;
			}
			audio_buf_index = 0;
		}
		len1 = audio_buf_size - audio_buf_index;
		if (len1 > len)
			len1 = len;
		memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1);
		len -= len1;
		stream += len1;
		audio_buf_index += len1;
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


	//������ packet �߳�
	packet_queue_init(&is->audioq);
	packet_queue_init(&is->videoq);
	SDL_CreateThread(read_packet_thread, is);

	//�� audio codec/codc context
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
	wanted_spec.userdata = is->audio_ctx;
	// ��Ҫ�Ȱѽ������ raw audio ת���� SDL ��Ҫ�ĸ�ʽ
	// ���� raw audio �ĸ�ʽ �� SDL �ĸ�ʽ���� swr_ctx
	is->swr_ctx = swr_alloc_set_opts(NULL,
		AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, is->audio_ctx->sample_rate,
		av_get_default_channel_layout(is->audio_ctx->channels), is->audio_ctx->sample_fmt, is->audio_ctx->sample_rate,
		0, NULL);
	//��ʼ�� swr_ctx
	swr_init(is->swr_ctx);

	if (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
		fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
		return -1;
	}

	avcodec_open2(is->audio_ctx, aCodec, NULL);

	// ��ʼ��Ƶ����/�����߳�
	SDL_PauseAudio(0);

	//����Ƶ codec/codec context 
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


#ifndef __DARWIN__
	screen = SDL_SetVideoMode(is->video_ctx->width, is->video_ctx->height, 0, 0);
#else
	screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 24, 0);
#endif
	if (!screen) {
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}

	//������Ƶ�����߳�
	pict_queue_init(&is->pictq, &is->videoq, 16);
	SDL_CreateThread(decode_video_thread, is);

	//������Ƶˢ���߳�
	SDL_CreateThread(video_refresh, is);
	
	// ������ѭ��
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