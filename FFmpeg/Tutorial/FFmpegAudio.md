### channel layout
* 所谓 channel layout，是用一串二进制数表示声道，其中1的个数等于声道数
* 根据 `libavutil/channel_layout.h `开始的注释：

         * A channel layout is a 64-bits integer with a bit set for every channel.
         * The number of bits set must be equal to the number of channels.
         * The value 0 means that the channel layout is not known.
         * @note this data structure is not powerful enough to handle channels
         * combinations that have the same channel multiple times, such as
         * dual-mono.


* 在`libavutil/channel_layout.h ` 里定义了一系列的宏来表示声道layout，比如：

        #define AV_CH_FRONT_LEFT             0x00000001
        #define AV_CH_FRONT_RIGHT            0x00000002
        #define AV_CH_FRONT_CENTER           0x00000004

        #define AV_CH_LAYOUT_MONO              (AV_CH_FRONT_CENTER)
        #define AV_CH_LAYOUT_STEREO            (AV_CH_FRONT_LEFT|AV_CH_FRONT_RIGHT)
        #define AV_CH_LAYOUT_5POINT0           (AV_CH_LAYOUT_SURROUND|AV_CH_SIDE_LEFT|AV_CH_SIDE_RIGHT)
        #define AV_CH_LAYOUT_5POINT1           (AV_CH_LAYOUT_5POINT0|AV_CH_LOW_FREQUENCY)

* Channel Layout 能比声道个数更加详细的描述声道信息
* 在ffmpeg里用 `int64_t` 来表示声道layout
* 在 layout 和声道个数之间转换，ffmpeg 也提供了函数：

        int av_get_channel_layout_nb_channels(uint64_t channel_layout); // 根据 layout 得到声道个数
        int64_t av_get_default_channel_layout(int nb_channels); // 根据声道个数得到 default layout

### 格式转换 
* 用 `swr_alloc_set_opts()` 得到一个`SwrContext`

        struct SwrContext *swr_alloc_set_opts(struct SwrContext *s,
                                              int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate,
                                              int64_t  in_ch_layout, enum AVSampleFormat  in_sample_fmt, int  in_sample_rate,
                                              int log_offset, void *log_ctx);

* 第一个参数传NULL，则给你生成一个 SwrContext 返回
* 接下来三个参数是设置输出格式的，可以看到，用来描述声道信息的是 layout 而不是 声道个数。
* 再接下三个参数是输入格式。最后两个参数先不用管。
* 用 `swr_init(SwrContext swr_ctx)` 初始化
* 用 `swr_convert()` 进行转换格式

        /** Convert audio.
         * 
         * @param s         allocated Swr context, with parameters set
         * @param out       output buffers, only the first one need be set in case of packed audio
         * @param out_count amount of space available for output in samples per channel
         * @param in        input buffers, only the first one need to be set in case of packed audio
         * @param in_count  number of input samples available in one channel
         *      
         * @return number of samples output per channel, negative value on error
         */
        int swr_convert(struct SwrContext *s, uint8_t **out, int out_count,
                                        const uint8_t **in , int in_count);
         

### ffplay 对 Audio 格式的处理
* 用SDL播放声音，SDL打开音频设备的主要参数:
	* format: 写死了 `wanted_spec.format = AUDIO_S16SYS;`
	* sample rate: 让用户（程序员）设定 `wanted_spec.freq = wanted_sample_rate;`
	* channels: SDL优先，用户设定其次，好几行代码就不贴在这里了，详见 `audio_open` 函数最开始的几行
* 解码出来的音频用 `swr_convert()` 进行转换格式以达到SDL设备要求，输出参数：
	* format： 写死了 `audio_hw_params->fmt = AV_SAMPLE_FMT_S16;`，以便跟SDL对应。
	* sample rate：根据SDL打开音频设备后返回的实际值设定：`audio_hw_params->freq = spec.freq;`
	* channels： 根据SDL打开音频设备后返回的实际值设定： `audio_hw_params->channels =  spec.channels;`
	* channels 稍微复杂，因为还弄了一个 `audio_hw_params->channel_layout = wanted_channel_layout;` 以保证channels的准确性