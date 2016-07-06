extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/time.h>
}
#include <sdl/SDL.h>

#undef main /* Prevents SDL from overriding main() */

int main(int argc, char *argv[]) {
	AVFormatContext *pFormatCtx = NULL;
	int             i,videoStream;
	AVCodecContext  *pCodecCtxOrig = NULL;
	AVCodecContext  *pCodecCtx = NULL;
	AVCodec         *pCodec = NULL;
	AVFrame         *pFrame = NULL;
	AVPacket        packet;
	int             frameFinished;
	struct SwsContext *sws_ctx = NULL;

	SDL_Overlay     *bmp;
	SDL_Surface     *screen;
	SDL_Rect        rect;

	av_register_all();

	SDL_Init(SDL_INIT_VIDEO);

	avformat_open_input(&pFormatCtx, argv[1], NULL, NULL); 

	avformat_find_stream_info(pFormatCtx, NULL); 

	videoStream = -1;
	for (i = 0; i<pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
			break;
		}

	pCodecCtxOrig = pFormatCtx->streams[videoStream]->codec;
	pCodec = avcodec_find_decoder(pCodecCtxOrig->codec_id);

	pCodecCtx = pFormatCtx->streams[videoStream]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	avcodec_open2(pCodecCtx, pCodec, NULL);
	pFrame = av_frame_alloc();

	screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 0, 0);

	bmp = SDL_CreateYUVOverlay(pCodecCtx->width,
		pCodecCtx->height,
		SDL_IYUV_OVERLAY,
		screen);

	sws_ctx = sws_getContext(pCodecCtx->width,
		pCodecCtx->height,
		pCodecCtx->pix_fmt,  // yuv420p,或者别的 yuv422，rgb23
		pCodecCtx->width,
		pCodecCtx->height,
		PIX_FMT_YUV420P,   // yuv420p
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
	);

	AVRational frame_rate = av_guess_frame_rate(pFormatCtx, 
		pFormatCtx->streams[videoStream],NULL);
	double duration_per_frame = av_q2d(av_inv_q(frame_rate));

	while (av_read_frame(pFormatCtx, &packet) >= 0) {
		if (packet.stream_index == videoStream) {
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

			if (frameFinished) {
				SDL_LockYUVOverlay(bmp);

				int linesize[3] = {*bmp->pitches,*(bmp->pitches+1),*(bmp->pitches+2)};

				sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
					pFrame->linesize, 0, pCodecCtx->height,
					bmp->pixels, linesize);

				SDL_UnlockYUVOverlay(bmp);

				rect.x = 0;
				rect.y = 0;
				rect.w = pCodecCtx->width;
				rect.h = pCodecCtx->height;
				av_usleep((int64_t)(duration_per_frame * 1000000.0));
				SDL_DisplayYUVOverlay(bmp, &rect);

			}
		}

		av_free_packet(&packet);
	}

	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avcodec_close(pCodecCtxOrig);
	avformat_close_input(&pFormatCtx);

	return 0;
}
