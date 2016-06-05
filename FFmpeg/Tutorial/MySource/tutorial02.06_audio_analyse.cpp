extern "C" {
#include<libavformat/avformat.h>
#include<libavcodec/avcodec.h>
#include<libavutil/avutil.h>
}

int main(int argc, char** argv)
{
	char* input_file = "D:\\CppWorkSpace\\FFmpegTutorial\\x64\\Debug\\out.mp4";
	
	AVFormatContext *pFormatCtx = NULL;
	AVCodecContext  *aCodecCtxOrig = NULL;
	AVCodecContext  *aCodecCtx = NULL;
	AVCodec         *aCodec = NULL;

	AVFrame			*pFrame = NULL;
	AVPacket        packet;

	av_register_all();
	avformat_open_input(&pFormatCtx, input_file, NULL, NULL);
	avformat_find_stream_info(pFormatCtx, NULL);

	int audioStream = -1;
	for (int i = 0; i<pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) 
		{
			audioStream = i;
		}
	}

	aCodecCtxOrig = pFormatCtx->streams[audioStream]->codec;
	aCodec = avcodec_find_decoder(aCodecCtxOrig->codec_id);

	// Copy context
	aCodecCtx = avcodec_alloc_context3(aCodec);
	avcodec_copy_context(aCodecCtx, aCodecCtxOrig);
	avcodec_open2(aCodecCtx, aCodec, NULL);

	pFrame = av_frame_alloc();

	while (av_read_frame(pFormatCtx, &packet) >= 0) {

		if (packet.stream_index == audioStream) {
			int consumed_pkt_size = 0;
			while (consumed_pkt_size < packet.size)
			{
				int got_frame = 0, len = 0;
				len = avcodec_decode_audio4(aCodecCtx, pFrame, &got_frame, &packet);
				consumed_pkt_size += len;
				if (got_frame)
				{
					pFrame->linesize[0];
					uint8_t * ch1_ptr = pFrame->extended_data[0];
					uint8_t * ch2_ptr = pFrame->extended_data[1];
					uint8_t * data0 = pFrame->data[0];
					int j=0;
				}
			}
			av_free_packet(&packet);
		}
		else {
			av_free_packet(&packet);
		}
	}

	av_frame_free(&pFrame);
	avcodec_close(aCodecCtxOrig);
	avcodec_close(aCodecCtx);

	// Close the video file
	avformat_close_input(&pFormatCtx);

	return 0;

}