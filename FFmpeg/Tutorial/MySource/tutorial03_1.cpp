extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}
#include <sdl/SDL.h>

#undef main

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
	pkt1 = (AVPacketList *)av_malloc(sizeof(AVPacketList));
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

PacketQueue audioq;
SwrContext *swr_ctx;
uint8_t *audio_buffer;
unsigned int read_index;
unsigned int audio_buffer_size;

void audio_callback(void *userdata, Uint8 *stream, int len);

int main(int argc, char** argv)
{
	AVFormatContext *pFormatCtx = NULL;
	int             i, videoStream, audioStream;
	AVPacket        packet;
	AVCodecContext  *aCodecCtx = NULL;
	AVCodec         *aCodec = NULL;

	audio_buffer = (uint8_t*)av_malloc(192000);
	packet_queue_init(&audioq);

	av_register_all();

	SDL_Init(SDL_INIT_AUDIO);

	avformat_open_input(&pFormatCtx, argv[1], NULL, NULL);
	avformat_find_stream_info(pFormatCtx, NULL);

	audioStream = -1;
	for (i = 0; i<pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioStream = i;
			break;
		}
	}

	aCodecCtx = pFormatCtx->streams[audioStream]->codec;
	aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
	avcodec_open2(aCodecCtx, aCodec, NULL);

	SDL_AudioSpec   wanted_spec, spec;
	wanted_spec.freq = aCodecCtx->sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = aCodecCtx->channels;
	wanted_spec.samples = 1024;
	wanted_spec.callback = audio_callback;
	wanted_spec.userdata = aCodecCtx;
	swr_ctx = swr_alloc_set_opts(NULL,
		AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, aCodecCtx->sample_rate,
		av_get_default_channel_layout(aCodecCtx->channels), aCodecCtx->sample_fmt, aCodecCtx->sample_rate,
		0, NULL);
	swr_init(swr_ctx);
	SDL_OpenAudio(&wanted_spec, &spec);
	SDL_PauseAudio(0);

	i = 0;
	while (av_read_frame(pFormatCtx, &packet) >= 0) {
		if (packet.stream_index == audioStream) {
			packet_queue_put(&audioq, &packet);
		}
		else {
			av_free_packet(&packet);
		}
	}

	getchar();
	return 0;
}

void audio_callback(void *userdata, Uint8 *stream, int len)
{
	AVPacket packet;
	AVCodecContext *aCodecCtx = (AVCodecContext*) userdata;
	AVFrame *frame = NULL;
	int got_frame, pkt_size, consumed_size, nu_samples;

	frame = av_frame_alloc();

	while (len > 0)
	{
		if (read_index < audio_buffer_size)
		{
			int available_data = audio_buffer_size - read_index;
			if (available_data > len)
			{
				memcpy(stream, &audio_buffer[read_index], len);
				read_index += len;
				return;
			}
			else
			{
				memcpy(stream, &audio_buffer[read_index], available_data);
				read_index += available_data;
				len -= available_data;
				stream += available_data;
			}
		}
		else
		{
			audio_buffer_size = 0;
			read_index = 0;
			packet_queue_get(&audioq, &packet, 1);
			pkt_size = packet.size;
			while (pkt_size > 0)
			{
				consumed_size = avcodec_decode_audio4(aCodecCtx, frame, &got_frame, &packet);
				pkt_size -= consumed_size;
				if (got_frame)
				{
					const uint8_t ** in = (const uint8_t **)frame->extended_data;
					nu_samples = swr_convert(swr_ctx, &audio_buffer, frame->nb_samples, in, frame->nb_samples);
					if (nu_samples <= 0)
						continue;
					audio_buffer_size += nu_samples * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) * 2;
				}
			}
			av_free_packet(&packet);
		}
	}
}