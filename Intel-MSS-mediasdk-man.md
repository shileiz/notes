文档名称：

	mediasdk-man.pdf
文档路径：

	C:\Program Files\Intel\Intel(R) Media Server Studio 2016\Software Development Kit\doc

####GOP 是在输入给 Encoder 之前就划分好的：

	Input frames usually come encoded in a repeated pattern called the Group of Picture (GOP) 
	sequence. For example, a GOP sequence can start from an I-frame, followed by a few B-
	frames, a P-frame, and so on. ENCODE uses an MPEG-2 style GOP sequence structure that can
	specify the length of the sequence and the distance between two key frames: I- or P-frames. A
	GOP sequence ensures that the segments of a bitstream do not completely depend upon each 
	other. It also enables decoding applications to reposition the bitstream. 

####可以按照显示顺序或编码顺序给 encoder 输入 frames：

	ENCODE processes input frames in two ways: 
		Display order: ENCODE receives input frames in the display order. A few GOP structure 
		parameters specify the GOP sequence during ENCODE initialization. Scene change results 
		from the video processing stage of a pipeline can alter the GOP sequence. 

		Encoded order: ENCODE receives input frames in their encoding order. The application 
		must specify the exact input frame type for encoding. ENCODE references GOP parameters 
		to determine when to insert information such as an end-of-sequence into the bitstream. 

* 如果是按照 Display order 给 encoder 输入 frames，那么 encoder 根据 GOP structure parameters 来决定先编哪帧。
* 如果是按照 Encoded order 给 encoder 输入 frames，那么应用程序必须告诉 encoder 这一帧是什么类型。encoder 根据 GOP structure parameters 来决定什么时候插入 information such as an end-of-sequence into the bitstream。

#### 关于时间戳

	An ENCODE output consists of one frame of a bitstream with the time stamp passed from the 
	input frame. The time stamp is used for multiplexing subsequent video with other associated 
	data such as audio. The SDK library provides only pure video stream encoding. The application 
	must provide its own multiplexing.
* encoder 输出的每一帧都有时间戳信息，时间戳来自于输入的frame
* 时间戳用作音视频同步

#### 关于rate control

	ENCODE supports the following bitrate control algorithms: constant bitrate, variable bitrate 
	(VBR), and constant Quantization Parameter (QP). In the constant bitrate mode, ENCODE 
	performs stuffing when the size of the least-compressed frame is smaller than what is required 
	to meet the Hypothetical Reference Decoder (HRD) buffer (or VBR) requirements. (Stuffing is a 
	process that appends zeros to the end of encoded frames.) 
* intel 的这个 encoder 支持三种码率控制模式：CBR、VBR、CQP
	* x264 和 x265 的是：目标码率模式（类似于VBR）、VBV/HRD模式（跟CBR有很大关系）、CQP
* 在 CBR 模式下，如果 encoder 按照最小压缩比压出来的一帧还是太小——比VBV/HRD需要的buffer小——则 encoder 会填写0。  