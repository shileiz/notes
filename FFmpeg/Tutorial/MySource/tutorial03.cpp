//extern "C" {
//#include <libavcodec/avcodec.h>
//#include <libavformat/avformat.h>
//#include <libswscale/swscale.h>
//#include <libswresample/swresample.h>
//}
//
//#include <sdl/SDL.h>
//#include <sdl/SDL_thread.h>
//
//#undef main /* Prevents SDL from overriding main() */
//
//typedef struct PacketQueue {
//	AVPacketList *first_pkt, *last_pkt;
//	int nb_packets;
//	int size;
//	SDL_mutex *mutex;
//	SDL_cond *cond;
//} PacketQueue;
//
//void packet_queue_init(PacketQueue *q) {
//	memset(q, 0, sizeof(PacketQueue));
//	q->mutex = SDL_CreateMutex();
//	q->cond = SDL_CreateCond();
//}
//
//int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
//
//	AVPacketList *pkt1;
//	if (av_dup_packet(pkt) < 0) {
//		return -1;
//	}
//	pkt1 = (AVPacketList *)av_malloc(sizeof(AVPacketList));
//	if (!pkt1)
//		return -1;
//	pkt1->pkt = *pkt;
//	pkt1->next = NULL;
//
//	SDL_LockMutex(q->mutex);
//
//	if (!q->last_pkt)
//		q->first_pkt = pkt1;
//	else
//		q->last_pkt->next = pkt1;
//	q->last_pkt = pkt1;
//	q->nb_packets++;
//	q->size += pkt1->pkt.size;
//	SDL_CondSignal(q->cond);
//
//	SDL_UnlockMutex(q->mutex);
//	return 0;
//}
//static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
//{
//	AVPacketList *pkt1;
//	int ret;
//
//	SDL_LockMutex(q->mutex);
//
//	for (;;) {
//
//		pkt1 = q->first_pkt;
//		if (pkt1) {
//			q->first_pkt = pkt1->next;
//			if (!q->first_pkt)
//				q->last_pkt = NULL;
//			q->nb_packets--;
//			q->size -= pkt1->pkt.size;
//			*pkt = pkt1->pkt;
//			av_free(pkt1);
//			ret = 1;
//			break;
//		}
//		else if (!block) {
//			ret = 0;
//			break;
//		}
//		else {
//			SDL_CondWait(q->cond, q->mutex);
//		}
//	}
//	SDL_UnlockMutex(q->mutex);
//	return ret;
//}
//
//// 暂且使用全局变量
//PacketQueue		audioq;
//SwrContext      *swr_ctx;
//uint8_t			audio_buf[1024*1024];
//unsigned int	audio_buf_read_index = 0;
//unsigned int	audio_buf_size = 0;
//
//void audio_callback(void *userdata, Uint8 *stream, int len);
//
//
//int main(int argc, char *argv[]) {
//	AVFormatContext *pFormatCtx = NULL;
//	int             i, videoStream, audioStream;
//	AVPacket        packet;
//	AVCodecContext  *aCodecCtx = NULL;
//	AVCodec         *aCodec = NULL;
//	SDL_AudioSpec   wanted_spec, spec;
//	
//	memset(audio_buf,0,sizeof(audio_buf));
//
//	av_register_all();
//
//	SDL_Init(SDL_INIT_AUDIO);
//
//	avformat_open_input(&pFormatCtx, argv[1], NULL, NULL);
//	avformat_find_stream_info(pFormatCtx, NULL);
//
//	audioStream = -1;
//	for (i = 0; i<pFormatCtx->nb_streams; i++) {
//		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
//			audioStream = i;
//			break;
//		}
//	}
//
//	aCodecCtx = pFormatCtx->streams[audioStream]->codec;
//	aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
//
//	wanted_spec.freq = aCodecCtx->sample_rate;
//	wanted_spec.format = AUDIO_S16SYS;
//	wanted_spec.channels = aCodecCtx->channels;
//	wanted_spec.samples = 1024;
//	wanted_spec.callback = audio_callback;
//	wanted_spec.userdata = aCodecCtx;
//	swr_ctx = swr_alloc_set_opts(NULL,
//		AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, aCodecCtx->sample_rate,
//		av_get_default_channel_layout(aCodecCtx->channels), aCodecCtx->sample_fmt, aCodecCtx->sample_rate,
//		0, NULL);
//	swr_init(swr_ctx);
//	SDL_OpenAudio(&wanted_spec, &spec);
//
//	avcodec_open2(aCodecCtx, aCodec, NULL);
//
//	packet_queue_init(&audioq);
//	SDL_PauseAudio(0);
//
//	i = 0;
//	while (av_read_frame(pFormatCtx, &packet) >= 0) {
//		if (packet.stream_index == audioStream) {
//			packet_queue_put(&audioq, &packet);
//		}
//		else {
//			av_free_packet(&packet);
//		}
//	}
//
//	getchar();
//
//	avcodec_close(aCodecCtx);
//	avformat_close_input(&pFormatCtx);
//
//	return 0;
//}
//
//void audio_callback(void *userdata, Uint8 *stream, int len) {
//
//	AVCodecContext *aCodecCtx = (AVCodecContext *)userdata;
//	int len1, audio_size;
//
//	AVPacket pkt;
//	AVFrame *frame=NULL;
//	int  got_frame;
//	int pk_size = 0;
//
//	frame = av_frame_alloc();
//
//	while (len > 0) {
//
//		if (audio_buf_read_index < audio_buf_size) {
//			int buf_data_len = audio_buf_size - audio_buf_read_index;
//			if (buf_data_len > len) {
//				memcpy(stream, &audio_buf[audio_buf_read_index], len);
//				audio_buf_read_index += len;
//				return;
//			}
//			else {
//				memcpy(stream, &audio_buf[audio_buf_read_index], buf_data_len);
//				audio_buf_read_index += buf_data_len;
//				len -= buf_data_len;
//				stream += buf_data_len;
//			}
//			
//		}
//		else {
//			audio_buf_read_index = 0;
//			audio_buf_size = 0;
//
//			packet_queue_get(&audioq, &pkt, 1);
//
//			pk_size = pkt.size;
//			while (pk_size > 0) {
//
//				len1 = avcodec_decode_audio4(aCodecCtx, frame, &got_frame, &pkt);
//				pk_size -= len1;
//				if (got_frame) {
//					//准备调用 swr_convert 的其他4个必须参数: out,out_samples_per_ch,in,in_samples_per_ch
//
//					uint8_t *tmp = audio_buf;
//					uint8_t **out = &tmp;
//					const uint8_t **in = (const uint8_t **)frame->extended_data;
//
//					int len2 = 0, out_data_size = 0;
//					
//					// 调用 swr_convert 进行转换
//					len2 = swr_convert(swr_ctx, out, frame->nb_samples, in, frame->nb_samples);
//					if (len2 <= 0)
//						continue;
//					// 计算转换完的 samples 总大小, 这就是给音频设备写入了多少字节的数据
//					out_data_size = len2 * 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
//					audio_buf_size += out_data_size;
//				
//				}
//
//			}
//			av_free_packet(&pkt);
//		}
//	}
//}