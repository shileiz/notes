/*

tutorial03.02 的主要目的：
在可以正常播放音频之后，把读包用单独的线程单独去做。
在主线程里收消息，收到刷新视频的消息时，拿出一个视频包，解码，显示，然后。
即视频的显示是在主线程里完成的。
刷新视频的消息固定以40ms一次发送：send_refresh_video_event()

*/

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

typedef struct PacketQueue {
	AVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
} PacketQueue;

PacketQueue audioq, videoq;

int quit = 0;

//暂且作为全局变量
SwrContext * swr_ctx = NULL;
SDL_Surface     *screen;
int  videoStream, audioStream;
SDL_Overlay     *bmp;
struct SwsContext *sws_ctx = NULL;

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

static Uint32 send_refresh_video_event(Uint32 interval, void *opaque) {
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.data1 = opaque;
	SDL_PushEvent(&event);
	return interval; 
}

void decode_and_display_video(void *param) {
	AVFrame         *pFrame = NULL;
	int frameFinished;
	AVCodecContext *pCodecCtx = (AVCodecContext*)param;
	AVPacket packet;
	pFrame = av_frame_alloc();
	SDL_Rect        rect;

	packet_queue_get(&videoq, &packet, 1);

	// Decode video frame
	avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

	// Did we get a video frame?
	if (frameFinished) {
		SDL_LockYUVOverlay(bmp);

		AVPicture pict;
		pict.data[0] = bmp->pixels[0];
		pict.data[1] = bmp->pixels[2];
		pict.data[2] = bmp->pixels[1];

		pict.linesize[0] = bmp->pitches[0];
		pict.linesize[1] = bmp->pitches[2];
		pict.linesize[2] = bmp->pitches[1];

		// Convert the image into YUV format that SDL uses	
		sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
			pFrame->linesize, 0, pCodecCtx->height,
			pict.data, pict.linesize);

		SDL_UnlockYUVOverlay(bmp);

		rect.x = 0;
		rect.y = 0;
		rect.w = pCodecCtx->width;
		rect.h = pCodecCtx->height;
		SDL_DisplayYUVOverlay(bmp, &rect);
		av_free_packet(&packet);
	}
	else {
		av_free_packet(&packet);
	}
}


int read_packet_thread(void *param) {
	AVFormatContext *pFormatCtx = (AVFormatContext*)param;
	AVPacket packet;
	while (av_read_frame(pFormatCtx, &packet) >= 0) {
		// Is this a packet from the video stream?
		if (packet.stream_index == videoStream) {
			packet_queue_put(&videoq, &packet);
		}
		else if (packet.stream_index == audioStream) {
			packet_queue_put(&audioq, &packet);
		}
		else {
			av_free_packet(&packet);
		}
	}
	return 0;
}

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size) {

	static AVPacket pkt;
	static uint8_t *audio_pkt_data = NULL;
	static int audio_pkt_size = 0;
	static AVFrame frame;
	int len1, resampled_data_size = 0;

	for (;;) {
		while (audio_pkt_size > 0) {
			int got_frame = 0;
			len1 = avcodec_decode_audio4(aCodecCtx, &frame, &got_frame, &pkt);
			if (len1 < 0) {
				/* if error, skip frame */
				audio_pkt_size = 0;
				break;
			}
			audio_pkt_data += len1;
			audio_pkt_size -= len1;

			if (got_frame) {

				// ---------------

				//准备调用 swr_convert 的其他4个必须参数: out,out_samples_per_ch,in,in_samples_per_ch
				uint8_t **out = &audio_buf;
				const uint8_t **in = (const uint8_t **)frame.extended_data;
				//int out_samples_per_ch = buf_size/ (av_get_bytes_per_sample(AV_SAMPLE_FMT_S16)*2);
				//调用 swr_convert 进行转换
				int len2 = 0;
				len2 = swr_convert(swr_ctx, out, frame.nb_samples, in, frame.nb_samples);
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

		if (packet_queue_get(&audioq, &pkt, 1) < 0) {
			return -1;
		}
		audio_pkt_data = pkt.data;
		audio_pkt_size = pkt.size;
	}

}

void audio_callback(void *userdata, Uint8 *stream, int len) {

	AVCodecContext *aCodecCtx = (AVCodecContext *)userdata;
	int len1, audio_size;

	static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
	static unsigned int audio_buf_size = 0;
	static unsigned int audio_buf_index = 0;

	while (len > 0) {
		if (audio_buf_index >= audio_buf_size) {
			/* We have already sent all our data; get more */
			audio_size = audio_decode_frame(aCodecCtx, audio_buf, sizeof(audio_buf));
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
	AVFormatContext *pFormatCtx = NULL;
	AVCodecContext  *pCodecCtxOrig = NULL;
	AVCodecContext  *pCodecCtx = NULL;
	AVCodec         *pCodec = NULL;
	AVCodecContext  *aCodecCtxOrig = NULL;
	AVCodecContext  *aCodecCtx = NULL;
	AVCodec         *aCodec = NULL;
	SDL_Event       event;
	SDL_AudioSpec   wanted_spec, spec;
	int i = 0;

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
	if (avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) != 0)
		return -1; // Couldn't open file

				   // Retrieve stream information
	if (avformat_find_stream_info(pFormatCtx, NULL)<0)
		return -1; // Couldn't find stream information

	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, argv[1], 0);


	videoStream = -1;
	audioStream = -1;
	for (i = 0; i<pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO &&
			videoStream < 0) {
			videoStream = i;
		}
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO &&
			audioStream < 0) {
			audioStream = i;
		}
	}
	if (videoStream == -1)
		return -1; 
	if (audioStream == -1)
		return -1;

	//开启读 packet 线程
	packet_queue_init(&audioq);
	packet_queue_init(&videoq);
	SDL_CreateThread(read_packet_thread, pFormatCtx);

	//打开 audio codec/codc context
	aCodecCtxOrig = pFormatCtx->streams[audioStream]->codec;
	aCodec = avcodec_find_decoder(aCodecCtxOrig->codec_id);
	if (!aCodec) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1;
	}

	// Copy context
	aCodecCtx = avcodec_alloc_context3(aCodec);
	if (avcodec_copy_context(aCodecCtx, aCodecCtxOrig) != 0) {
		fprintf(stderr, "Couldn't copy codec context");
		return -1; // Error copying codec context
	}

	// Set audio settings from codec info
	wanted_spec.freq = aCodecCtx->sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = aCodecCtx->channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
	wanted_spec.callback = audio_callback;
	wanted_spec.userdata = aCodecCtx;
	// 需要先把解出来的 raw audio 转换成 SDL 需要的格式
	// 根据 raw audio 的格式 和 SDL 的格式设置 swr_ctx
	swr_ctx = swr_alloc_set_opts(NULL,
		AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, aCodecCtx->sample_rate,
		av_get_default_channel_layout(aCodecCtx->channels), aCodecCtx->sample_fmt, aCodecCtx->sample_rate,
		0, NULL);
	//初始化 swr_ctx
	swr_init(swr_ctx);

	if (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
		fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
		return -1;
	}

	avcodec_open2(aCodecCtx, aCodec, NULL);

	// 开始音频解码/播放线程
	SDL_PauseAudio(0);

	//打开视频 codec/codec context 
	pCodecCtxOrig = pFormatCtx->streams[videoStream]->codec;

	// Find the decoder for the video stream
	pCodec = avcodec_find_decoder(pCodecCtxOrig->codec_id);
	if (pCodec == NULL) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1; // Codec not found
	}

	// Copy context
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
		fprintf(stderr, "Couldn't copy codec context");
		return -1; // Error copying codec context
	}

	// Open codec
	if (avcodec_open2(pCodecCtx, pCodec, NULL)<0)
		return -1; // Could not open codec


#ifndef __DARWIN__
	screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 0, 0);
#else
	screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 24, 0);
#endif
	if (!screen) {
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}

	bmp = SDL_CreateYUVOverlay(pCodecCtx->width,
		pCodecCtx->height,
		SDL_YV12_OVERLAY,
		screen);

	sws_ctx = sws_getContext(pCodecCtx->width,
		pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width,
		pCodecCtx->height,
		PIX_FMT_YUV420P,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
		);

	//开启视频 解码/显示 线程
	AVRational frame_rate = av_guess_frame_rate(pFormatCtx,pFormatCtx->streams[videoStream],NULL);
	int delay = (int)(av_q2d(av_inv_q(frame_rate))*1000);
	SDL_AddTimer(delay, send_refresh_video_event, pCodecCtx);

	for (;;) {
		SDL_WaitEvent(&event);
		switch (event.type) {
		case SDL_QUIT:
			SDL_Quit();
			return 0;
			break;
		case SDL_USEREVENT:
			decode_and_display_video(event.user.data1);
			break;
		default:
			break;
		}
	}
	return 0;
}
