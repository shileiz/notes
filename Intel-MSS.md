## Intel Media Server Studio 

### 获取
* 官方网站： `https://software.intel.com/en-us/intel-media-server-studio`
* 在官方网站可以申请下载评估版本。注意要下载Pro版本的、Windows版的。(我下载的版本：Intel Media Server Studio 2016 Professional)
* Windows版的是个exe安装包，一路下一步安装。
* 有一个组件安装时出错：`VTune_Amplifier_XE`
* 先不管这个错误，安装完手动重启电脑

### 包括哪些东西
* 安装完成后在开始菜单里能找到它的 release-note，里面描述了它包含以下东西：

		  Graphics driver 
		  SDK 
		  OpenCL™ Code Builder 
		  Audio Decoder and Encoder： is a dedicated library for audio codec software processing. 
		  HEVC Decoder and Encoder： is of set of software development libraries (plug-ins) and tools that that expose the HEVC decode and encode acceleration capabilities of Intel® platforms. 
		  Premium Telecine Interlace Reverser ("PTIR")： is a development library that exposes the media acceleration capabilities of Intel platforms for video deinterlacing/inverse telecine processing. 
		  Video Quality Caliper：is a graphical utility for objective and visual quality inspection of encoded or uncompressed videos 
		  Intel® VTune™ Amplifier XE：is a powerful threading and performance optimization tool for developers who need to understand an application's serial and parallel behavior to improve performance and scalability. 
		  Samples 
* 我们在安装的时候，除了 `VTune™ Amplifier XE` 都装上了
* 我们其实只关注其中的 HEVC 组件，注意HEVC是以库的形式提供的。

### 使用 HEVC

#### HEVC 的 release-note：

		C:\Program Files\Intel\Intel(R) Media Server Studio 2016\HEVC Decoder & Encoder\media_server_studio_hevc_release_notes.pdf
  
* 里面没啥有用的信息，基本就是告诉你了：
	* HEVC 组件由几个库文件组成
	* 怎么用请参考 SDK 的文档

#### Samples
* 在SDK目录下，由一个samples目录，里面有一个html文件，告诉你去哪里下载samples：

		https://software.intel.com/en-us/intel-media-server-studio-support/code-samples
* 下载下来是个安装包：IntelMediaSamples.msi
* 安装过程实际上就是解压。安装后每个sample有自己的目录，里面有VisualStudio的sln文件。
* 安装目录里还有一个叫“_bin”的目录，里面有编译好的exe文件，可以直接用。
* 看了一下 sample_encode.exe ，能把 yuv 转成 h265，不过没有办法设置 preset。

