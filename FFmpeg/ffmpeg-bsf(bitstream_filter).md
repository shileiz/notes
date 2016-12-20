* 官方文档：
* [http://www.ffmpeg.org/ffmpeg-bitstream-filters.html](http://www.ffmpeg.org/ffmpeg-bitstream-filters.html)
* 官方文档对bsf的基本概念并没有做太详细的阐释，在 stackoverflow 上找到了这篇：
* [http://stackoverflow.com/questions/32028437/what-are-bitstream-filters-in-ffmpeg](http://stackoverflow.com/questions/32028437/what-are-bitstream-filters-in-ffmpeg)
* 防止丢失，原文抄到下面：


* Let me explain by example. FFmpeg video decoders typically work by converting one video frame per call to avcodec_decode_video2. So the input is expected to be "one image" worth of bitstream data. Let's consider this issue of going from a file (an array of bytes of disk) to images for a second.

* For "raw" (annexb) H264 (.h264/.bin/.264 files), the individual nal unit data (sps/pps header bitstreams or cabac-encoded frame data) is concatenated in a sequence of nal units, with a start code (00 00 01 XX) in between, where XX is the nal unit type. (In order to prevent the nal data itself to have 00 00 01 data, it is RBSP escaped.) So a h264 frame parser can simply cut the file at start code markers. They search for successive packets that start with and including 00 00 01, until and excluding the next occurence of 00 00 01. Then they parse the nal unit type and slice header to find which frame each packet belongs to, and return a set of nal units making up one frame as input to the h264 decoder.

* H264 data in .mp4 files is different, though. You can imagine that the 00 00 01 start code can be considered redundant if the muxing format already has length markers in it, as is the case for mp4. So, to save 3 bytes per frame, they remove the 00 00 01 prefix. They also put the PPS/SPS in the file header instead of prepending it before the first frame, and these also miss their 00 00 01 prefixes. So, if I were to input this into the h264 decoder, which expects the prefixes for all nal units, it wouldn't work. The h264_mp4toannexb bitstream filter fixes this, by identifying the pps/sps in the extracted parts of the file header (ffmpeg calls this "extradata"), prepending this and each nal from individual frame packets with the start code, and concatenating them back together before inputting them in the h264 decoder.

* You might now feel that there's a very fine line distinction between a "parser" and a "bitstream filter". This is true. I think the official definition is that a parser takes a sequence of input data and splits it in frames without discarding any data or adding any data. The only thing a parser does is change packet boundaries. A bitstream filter, on the other hand, is allowed to actually modify the data. I'm not sure this definition is entirely true (see e.g. vp9 below), but it's the conceptual reason mp4toannexb is a BSF, not a parser (because it adds 00 00 01 prefixes).

* Other cases where such "bitstream tweaks" help keep decoders simple and uniform, but allow us to support all files variants that happen to exist in the wild:

	* mpeg4 (divx) b frame unpacking (to get B-frames sequences like IBP, which are coded as IPB, in AVI and get timestamps correct, people came up with this concept of B-frame packing where I-B-P / I-P-B is packed in frames as I-(PB)-(), i.e. the third packet is empty and the second has two frames. This means the timestamp associated with the P and B frame at the decoding phase is correct. It also means you have two frames worth of input data for one packet, which violates ffmpeg's one-frame-in-one-frame-out concept, so we wrote a bsf to split the packet back in two - along with deleting the marker that says that the packet contains two frames, hence a BSF and not a parser - before inputting it into the decoder. In practice, this solves otherwise hard problems with frame multithreading. VP9 does the same thing (called superframes), but splits frames in the parser, so the parser/BSF split isn't always theoretically perfect; maybe VP9's should be called a BSF)
	* hevc mp4 to annexb conversion (same story as above, but for hevc)
	* aac adts to asc conversion (this is basically the same as h264/hevc annexb vs. mp4, but for aac audio)

---

* bsf 是接着 parser 之后，decoder 之前的
* bsf 跟 parser 最大的区别是 bfs 会改变 stream 的内容，而 parser 不会（这是概念上，也有特殊情况，比如vp9）
* parser 只是把输入文件切开，切成一个一个的 packet
* 但有些 packet 并不适合送给 decoder，因为 ffmpeg 的 decoder 设计要求输入的数据正好够解码成一帧
* bsf 的作用就是修改这些 packet
* stackoverflow 上的帖子举了一个 h264 的例子，讲的比较明白

---

* 另外在这 stackoverflow 篇回答下面有个评论说，在用 ffmepg 进行转码的时候，bsf 是被应用到重新转出来的 stream 上的：

		note that when running ffmpeg and not doing a streamcopy (i.e. actually transcoding the video) the bitstream filters get applied after the re-encode.

* 我觉得说的是有道理的，但是没有做实验证实这一点
