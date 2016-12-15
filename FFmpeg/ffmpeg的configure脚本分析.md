###大面
* configure 脚本是个 shell 脚本，主要用来根据用户的命令行参数**以及当前的ffmpeg代码**，生成 config.h, config.mak, config.asm, config.texi
* 其中 config.h 和 config.mak 是通常我们比较关心的，因为编译阶段会用到
* 需要注意的是，configure 不光根据用户设置的命令行参数（比如 --disable-ffplay 之类），还会根据当前的ffmpeg代码来生成这几个文件。比如需要从 libavcodec/allcodecs.c 里读出当前版本的 ffmpeg 代码支持哪些 encoder 和 decoder。

###`print_config()`函数
* configure 脚本往 config.h 等文件里写东西主要有以下两种方法：
	1. 直接通过 echo 、cat 等方式写入少量内容
	2. 通过调用 `print_config()` 函数写入内容
* 我们看看 `print_config()` 函数的用法。具体实现就不看了（实际上是通过 awk 配合一系列的字符串替换）。
* print_config 需要三个参数，调用格式如下：
* print_config prefix files options
* prefix 指的是配置的前缀，最后会被拼接到 options 前面，并写入文件
* files 是指要写入的文件，比如 config.h, config.mak，函数会根据被写入的文件后缀不同，按对应的格式写入
* options 指的是配置选项，比如 filters，muxers，doc，ffplay 等等
* print_config 会把配置选项都变成大写再写入对应文件

#####举个例子
* 调用 print_config

		print_config CONFIG_ "config.h config.mak" $CONFIG_LIST

* 这句话的意思是，往文件 config.h 和 config.mak 里写入前缀为 CONFIG_ 的配置项，需要写入的配置项由变量 CONFIG_LIST 决定

* 假设上述调用中，变量 CONFIG_LIST 的值如下：

		CONFIG_LIST="
		    filters
		    muxers
		    doc
		"
* 则调用结束后，生成的 config.h 和 config.mak 的内容如下：
		
		/* config.h */
		#define CONFIG_FILTERS 0
		#define CONFIG_MUXERS 0
		#define CONFIG_DOC 0
		
		/* config.mak */
		!CONFIG_FILTERS=yes
		!CONFIG_MUXERS=yes
		!CONFIG_DOC=yes

* 如果某个配置项在shell脚本里有非0的值，则其会被enable。上述例子里我们加上一句：

		doc=100

* 则生成的结果如下：

		/* config.h */
		#define CONFIG_FILTERS 0
		#define CONFIG_MUXERS 0
		#define CONFIG_DOC 100
		
		/* config.mak */
		!CONFIG_FILTERS=yes
		!CONFIG_MUXERS=yes
		CONFIG_DOC=yes

* 在 config.h 里， 它被 define 成了 100，在 config.mak 里，他前面的 ! 被去掉了。

###`find_things()`函数
* configure 脚本里有些变量的值不是写死则脚本里的，也不是用户通过参数传进来的，而是从 .c/.h 源文件里读出来的。
* 这个函数的作用就是从源文件里读出想要的东西。其实现自然是通过正则表达式（与sed配合）。
* 具体实现细节不去看，看看用法：

		find_things thing pattern file

* 从 file 中找到所有符合 pattern 的模式，并加上后缀 _thing 返回
* 举例：

		find_things  zsl  ENC      libavcodec/allcodecs.c

* 以上语句将把 libavcodec/allcodecs.c 里复合 ENC 这个 pattern 的行都抓取，并提取出“有用的部分”，然后加上 _zsl 后缀返回，最终的返回结果：

		a64multi_zsl a64multi5_zsl alias_pix_zsl amv_zsl apng_zsl asv1_zsl asv2_zsl avrp_zsl avui_zsl ayuv_zsl  
		bmp_zsl cinepak_zsl cljr_zsl comfortnoise_zsl dnxhd_zsl dpx_zsl dvvideo_zsl ffv1_zsl ffvhuff_zsl  
		flashsv_zsl flashsv2_zsl flv_zsl gif_zsl h261_zsl h263_zsl h263p_zsl hap_zsl huffyuv_zsl jpeg2000_zsl   
		jpegls_zsl ljpeg_zsl mjpeg_zsl mpeg1video_zsl mpeg2video_zsl mpeg4_zsl msmpeg4v2_zsl msmpeg4v3_zsl msvideo1_zsl...

* 至于该函数是如何使用 pattern 的，以及提取的是这一行的哪部分，具体可以看其实现：

		sed -n "s/^[^#]*$pattern.*([^,]*, *\([^,]*\)\(,.*\)*).*/\1_$thing/p" "$file"