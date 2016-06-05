/**
* ��򵥵�SDL2������Ƶ�����ӣ�SDL2����PCM��
* Simplest Audio Play SDL2 (SDL2 play PCM)
*
* ������ Lei Xiaohua
* leixiaohua1020@126.com
* �й���ý��ѧ/���ֵ��Ӽ���
* Communication University of China / Digital TV Technology
* http://blog.csdn.net/leixiaohua1020
*
* ������ʹ��SDL2����PCM��Ƶ�������ݡ�SDLʵ�����ǶԵײ��ͼ
* API��Direct3D��OpenGL���ķ�װ��ʹ���������Լ���ֱ�ӵ��õײ�
* API��
*
* �������ò�������:
*
* [��ʼ��]
* SDL_Init(): ��ʼ��SDL��
* SDL_OpenAudio(): ���ݲ������洢��SDL_AudioSpec������Ƶ�豸��
* SDL_PauseAudio(): ������Ƶ���ݡ�
*
* [ѭ����������]
* SDL_Delay(): ��ʱ�ȴ�������ɡ�
*
* This software plays PCM raw audio data using SDL2.
* SDL is a wrapper of low-level API (DirectSound).
* Use SDL is much easier than directly call these low-level API.
*
* The process is shown as follows:
*
* [Init]
* SDL_Init(): Init SDL.
* SDL_OpenAudio(): Opens the audio device with the desired
*					parameters (In SDL_AudioSpec).
* SDL_PauseAudio(): Play Audio.
*
* [Loop to play data]
* SDL_Delay(): Wait for completetion of playback.
*/


#include <SDL.h>
#include <SDL_thread.h>


#include <stdio.h>
#include <tchar.h>

#undef main 

//Buffer:
//|-----------|-------------|
//chunk-------pos---len-----|
static  Uint8  *audio_chunk;
static  Uint32  audio_len;
static  Uint8  *audio_pos;

/* Audio Callback
* The audio function callback takes the following parameters:
* stream: A pointer to the audio buffer to be filled
* len: The length (in bytes) of the audio buffer
*
*/
void  fill_audio(void *udata, Uint8 *stream, int len) {
	if (audio_len == 0)
		return;
	len = (len>audio_len ? audio_len : len);

	memcpy(stream, audio_pos, len);
	audio_pos += len;
	audio_len -= len;
}

int main(int argc, char* argv[])
{
	//Init
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}
	//SDL_AudioSpec
	SDL_AudioSpec wanted_spec;
	wanted_spec.freq = 44100;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = 2;
	wanted_spec.silence = 0;
	wanted_spec.samples = 1024;
	wanted_spec.callback = fill_audio;

	if (SDL_OpenAudio(&wanted_spec, NULL)<0) {
		printf("can't open audio.\n");
		return -1;
	}

	FILE *fp = fopen("D:\\CppWorkSpace\\FFmpegTutorial\\x64\\Debug\\chimei_416x240_42sec.pcm", "rb+");
	if (fp == NULL) {
		printf("cannot open this file\n");
		return -1;
	}
	int pcm_buffer_size = 4096;
	char *pcm_buffer = (char *)malloc(pcm_buffer_size);

	//Play
	SDL_PauseAudio(0);

	while (fread(pcm_buffer, 1, pcm_buffer_size, fp)) {
		//Set audio buffer (PCM data)
		audio_chunk = (Uint8 *)pcm_buffer;
		//Audio buffer length
		audio_len = pcm_buffer_size;
		audio_pos = audio_chunk;

		while (audio_len>0)//Wait until finish
			SDL_Delay(1);
	}
	free(pcm_buffer);
	SDL_Quit();

	return 0;
}
