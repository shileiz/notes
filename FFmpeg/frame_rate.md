### AVStream 里有如下几个成员跟 framerate 有关：
* `avg_frame_rate`，`r_frame_rate`，`time_base`,`codec->time_base` 

####1. `avg_frame_rate`：
* 平均 framerate

		:::c
	    /**
	     * Average framerate
	     *
	     * - demuxing: May be set by libavformat when creating the stream or in
	     *             avformat_find_stream_info().
	     * - muxing: May be set by the caller before avformat_write_header().
	     */
		AVRational avg_frame_rate;

* 如果 demuxer 没有获得这个数，则在 `avformat_find_stream_info()` 里计算
* 计算方式也是根据 `r_frame_rate` “猜”

####2. `r_frame_rate`
* real base framerate

		:::c
	    /**
	     * Real base framerate of the stream.
	     * This is the lowest framerate with which all timestamps can be
	     * represented accurately (it is the least common multiple of all
	     * framerates in the stream). Note, this value is just a guess!
	     * For example, if the time base is 1/90000 and all frames have either
	     * approximately 3600 or 1800 timer ticks, then r_frame_rate will be 50/1.
	     */
	    AVRational r_frame_rate;

* 在 `avformat_find_stream_info()` 里根据 `time_base` “猜” 出来的值
* 猜的方式是调用：`av_reduce()`，这个函数没看太懂，是一堆数学计算。总之这个函数前两个参数是带出参数，后两个是输入参数，根据这两个参数来“猜”。

####3. `time_base`
* time base 是一个以秒为单位的分数，比如 1/44100 秒。
* 在 Tutorial 05 里解释过了
* `AVStream->time_base` 可以从文件里读出来

####4. `codec->time_base` 
* codec 是 AVStream 的成员，是 AVCodecContext 类型的
* 在 Tutorial 05 里解释过了，见 `AVStream.time_base 和 AVCodecContext.time_base` 一节
* 目前暂不考虑这个数

### 一个 ts 文件的 framerate 取得过程：
* 实验了一个 ts 文件： MEPG2_Main_Main_720x480_29.970fps_No_Audio.ts
* 跟踪了一下 ffmpeg 的代码，发现 ffmpeg.exe 在转码是，打印在屏幕上的 fps 是这么算出来的：
	* `avformat_open_input()` 读出了 video stream 的 `time_base`：{1, 90000}
	* 在 `avformat_find_stream_info()` 里，根据 `time_base` 猜出了 `r_frame_rate`: {30000, 1001}
	* 在 `avformat_find_stream_info()` 里，根据 `r_frame_rate` 猜出了 `avg_frame_rate`: {30000, 1001}

* 对于 ts 文件来说，是通过 `mpegts_read_header()` 读到的 `time_base`，是从文件里读到的
* 另外，AVCodecContext 里有个 framerate 成员，是根据 ts 里的 ticks 和 scale 算出来的 xxxxx
* 猜测 `r_frame_rate` 根据 如下两个值（按照注释里说的）：
	1. time base： For example, if the time base is 1/90000
	2. ticks：  and all frames have either approximately 3600 or 1800 timer ticks

### 数据断点好使