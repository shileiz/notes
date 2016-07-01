
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}


int main(int argc, char *argv[]) {

	AVFormatContext   *pFormatCtx = NULL;
	int               i,videoStream,windex=0;
	AVCodecContext    *pCodecCtx = NULL;
	AVCodec           *pCodec = NULL;
	AVFrame           *pFrame = NULL;
	AVPacket          packet;
	int               gotframe;
	uint8_t           *buffer = NULL;
	int               height, width;
	FILE *yuv_file = fopen("output.yuv", "ab");

	av_register_all();

	avformat_open_input(&pFormatCtx, argv[1], NULL, NULL);

	avformat_find_stream_info(pFormatCtx, NULL);

	av_dump_format(pFormatCtx, 0, argv[1], 0);

	videoStream = -1;
	for (i = 0; i<pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
			break;
		}

	pCodecCtx = pFormatCtx->streams[videoStream]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	avcodec_open2(pCodecCtx, pCodec, NULL);
	height= pCodecCtx->height;
	width = pCodecCtx->width;
	buffer = (uint8_t *)av_malloc(height * width * 3 / 2);

	pFrame = av_frame_alloc();
	while (av_read_frame(pFormatCtx, &packet) >= 0) {

		if (packet.stream_index == videoStream) {

			avcodec_decode_video2(pCodecCtx, pFrame, &gotframe, &packet);

			if (gotframe) {

				memset(buffer, 0, height * width * 3 / 2);
				windex = 0;
				for (i = 0; i<height; i++)
				{
					memcpy(buffer + windex, pFrame->data[0] + i * pFrame->linesize[0], width);
					windex += width;
				}
				for (i = 0; i<height / 2; i++)
				{
					memcpy(buffer + windex, pFrame->data[1] + i * pFrame->linesize[1], width / 2);
					windex += width / 2;
				}
				for (i = 0; i<height / 2; i++)
				{
					memcpy(buffer + windex, pFrame->data[2] + i * pFrame->linesize[2], width / 2);
					windex += width / 2;
				}
				fwrite(buffer, 1, height * width * 3 / 2, yuv_file);
				
			}
		}
		av_free_packet(&packet);
	}

	av_free(buffer);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
	fclose(yuv_file);
	return 0;
}
