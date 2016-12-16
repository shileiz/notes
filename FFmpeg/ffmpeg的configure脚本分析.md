###大面
* configure 脚本是个 shell 脚本，主要用来根据用户的命令行参数**以及当前的ffmpeg代码**，生成 config.h, config.mak, config.asm, config.texi
* 其中 config.h 和 config.mak 是通常我们比较关心的，因为编译阶段会用到
* 需要注意的是，configure 不光根据用户设置的命令行参数（比如 --disable-ffplay 之类），还会根据当前的ffmpeg代码来生成这几个文件。比如需要从 libavcodec/allcodecs.c 里读出当前版本的 ffmpeg 代码支持哪些 encoder 和 decoder。

###一.解析命令行参数
* configure 里从 `for opt do` 段开始解析命令行参数。这个 for 没有写 in 什么，所以它是对命令行参数循环，每次循环 opt 等于一个命令行参数，比如 --enable-ffplay。
* 这个for里是一个 case 语句，判断参数是哪种形式的，不同形状的参数不同处理。我们只分析平时常用的两种形式：
	1. `--enable-xxx=xxx / --disable-xxx=xxx`， 比如 `--enable-filter=rotate`
	2. `--enable-xxx /--disable-xxx`，比如 `--disable-ffserver`

#### 1. `--enable-xxx=xxx / --disable-xxx=xxx`
* 该部分代码如下：

		for opt do
		    optval="${opt#*=}"
		    case "$opt" in
			...
	        --enable-*=*|--disable-*=*)
	            eval $(echo "${opt%%=*}" | sed 's/--/action=/;s/-/ thing=/')
	            is_in "${thing}s" $COMPONENT_LIST || die_unknown "$opt"
	            eval list=\$$(toupper $thing)_LIST
	            name=$(echo "${optval}" | sed "s/,/_${thing}|/g")_${thing}
	            list=$(filter "$name" $list)
	            [ "$list" = "" ] && warn "Option $opt did not match anything"
	            $action $list
	        ;;      



* 一句一句来分析：

		eval $(echo "${opt%%=*}" | sed 's/--/action=/;s/-/ thing=/')

* 以上这句在 opt 里提取 action 和 thing，比如 opt 是 `--disable-filter=crop`，那么处理后，action=disable，thing=filter

		is_in "${thing}s" $COMPONENT_LIST || die_unknown "$opt"


* 以上这句用来判断 filters 是否在 `$COMPONENT_LIST` 里，不在则报错退出。`$COMPONENT_LIST=encoders decoders filters...` 具体见本文的“变量分析”部分。
* 也就是说，形如 --enable-xxx=xxx 的参数，只支持对 `$COMPONENT_LIST` 里的东西使用。

		eval list=\$$(toupper $thing)_LIST

* 以上这句是定义变量 list，取值为 `$FILTER_LIST`。`$FILTER_LIST=rotate_filter scale_filter crop_filter...` 具见本文的“变量分析”部分。

		name=$(echo "${optval}" | sed "s/,/_${thing}|/g")_${thing}

* 以上这句里有个 optval 变量，这是在 case 语句之前定义的一个变量，提取的是 opt 中等号后面的部分，在本例中即 crop。
* 这句的作用是把 crop 和 filter 拼接起来，这句结束后 name 的值为：`crop_filter`
* 这种形式跟变量 `$FILTER_LIST` 的值是对应的。

		list=$(filter "$name" $list)

* 以上这句有个函数 `filter()` ，其作用就是，如果 $name 在 $list 里，则把其返回，否则返回空。需要注意，如果 $list 里有多个 $name, 则返回多个。
* 举个例子：

		:::shell
		list="rotate_filter scale_filter crop_filter crop_filter"
		name=crop_filter
		list=$(filter "$name" $list)
		echo $list
		#结果是 crop_filter crop_filter

* 总之这句执行完，list 的值是 `crop_filter`

		[ "$list" = "" ] && warn "Option $opt did not match anything"

* 以上这句的意思是：如果 list 为空，即 `crop_filter` 不在 ffmpeg 目前代码支持的范围内，则报一个 warning

		$action $list

* 最后一句，执行 `disable crop_filter`。 其中 disable 是函数，它和 enable 类似。 `disable crop_filter` 相当于： `crop_filter="no"`， 而 `enable crop_filter` 相当于 `crop_filter="yes"`
* 小结一下：
* 当我们传入 `--disable-filter=crop` 时，相当于在脚本里弄了一个变量：`crop_filter="no"`
* 如果我们写错，传入了 `--disable-fillter=crop（filter 多打了个l）`，则会报错退出，因为 fillter 并不在 COMPONENT LIST 里。
* 如果我们写错，但错在后一个单词，比如传入了 `--disable-filter=coop（crop 拼错）`，则不会报错，只会产生一个 warning，说 `Option --disable-filter=coop did not match anything`，并不会退出。
* 当这种参数书写正确的时候，会在整个脚本里生成变量，并设置其值为 yes 或者 no。
* 这种形式的参数，只支持 `COMPONENT_LIST` 里的 11 个组件，当然，每种组件又有好多个可开关的东西，比如 encoders 就有好几十种 encoder 可开或者关。
* 默认是全部 encoders 都打开的。

#### 2. `--enable-xxx /--diable-xxx`
* 代码如下：

		for opt do
		    optval="${opt#*=}"
		    case "$opt" in
			...
	        --enable-?*|--disable-?*)
	            eval $(echo "$opt" | sed 's/--/action=/;s/-/ option=/;s/-/_/g')
	            if is_in $option $COMPONENT_LIST; then
	                test $action = disable && action=unset
	                eval $action \$$(toupper ${option%s})_LIST
	            elif is_in $option $CMDLINE_SELECT; then
	                $action $option 
	            else    
	                die_unknown $opt
	            fi
			;;

* 有了上面的经验，就不用一句一句分析了。大概看下过程：假设我们传入的opt是 `--enable-ffpaly`
* 首先，把 enable 提取为 action，把 ffplay 提取为 option。
* 然后检测 ffplay 是否在 `$COMPONENT_LIST` 里，检测结果是不在。 所以什么都不干。 
> 注意: ffplay, ffmpeg, ffprobe 这些东西不是 ffmpeg 的组件，所谓组件是 decoders，encoders，muxers 这些，共11个。 

* 接着检测 ffplay 是否在 `$CMDLINE_SELECT` 里，在。所以执行：`enable ffplay`。 相当于造了一个变量 ffplay，并赋值为 yes。
* 这里出现个新变量 `$CMDLINE_SELECT`，它的值其实是包含了 `$COMPONENT_LIST` 的，所以要放在后面检测。总之我们的 ffplay 是在这里的，所以会执行 `enable ffplay`，即生成变量 ffplay，其值为 yes。
* 注意，如果传进来的是个 COMPONENT，则会走到第一个分支里。 我们还是举个例子： `--disable-encoders`
* 因为是 disable ，所以 action 变成 unset，即取消变量的意思。
* 然后会把 `$ENCODER_LIST`  里的变量全部 unset。

###二.变量分析

####1. `COMPONENT_LIST`
* `COMPONENT_LIST` 代表了 ffmpeg 支持的组件，目前版本的ffmpeg（2016年12月），支持11种组件，包括 encoders，filters 等等
* 定义如下

		COMPONENT_LIST="
		    $AVCODEC_COMPONENTS
		    $AVDEVICE_COMPONENTS
		    $AVFILTER_COMPONENTS
		    $AVFORMAT_COMPONENTS
		    $AVRESAMPLE_COMPONENTS
		    $AVUTIL_COMPONENTS
		"
		AVCODEC_COMPONENTS="
		    bsfs
		    decoders
		    encoders
		    hwaccels
		    parsers
		"
		...

* 展开后，是这个样子：

		COMPONENT_LIST="
		    bsfs
		    decoders
		    encoders
		    hwaccels
		    parsers
			indevs
			outdevs
			filters
			demuxers
			muxers
			protocols
		"

####2. `ENCODER_LIST，DECODER_LIST，...，FILTER_LIST，...`
* `$COMPONENT_LIST` 里的11个组件，都有自己的具体列表。 比如 encoders 支持哪些，就由 `$ENCODER_LIST` 来表示。
* 这 11 个组件的 LIST 都是用函数找到的，如下：

		ENCODER_LIST=$(find_things  encoder  ENC      libavcodec/allcodecs.c)
		DECODER_LIST=$(find_things  decoder  DEC      libavcodec/allcodecs.c)
		HWACCEL_LIST=$(find_things  hwaccel  HWACCEL  libavcodec/allcodecs.c)
		PARSER_LIST=$(find_things   parser   PARSER   libavcodec/allcodecs.c)
		MUXER_LIST=$(find_things    muxer    _MUX     libavformat/allformats.c)
		DEMUXER_LIST=$(find_things  demuxer  DEMUX    libavformat/allformats.c)
		OUTDEV_LIST=$(find_things   outdev   OUTDEV   libavdevice/alldevices.c)
		INDEV_LIST=$(find_things    indev    _IN      libavdevice/alldevices.c)
		FILTER_LIST=$(find_things   filter   FILTER   libavfilter/allfilters.c)
		BSF_LIST=$(find_things_extern bsf AVBitStreamFilter libavcodec/bitstream_filters.c)
		PROTOCOL_LIST=$(find_things_extern protocol URLProtocol libavformat/protocols.c)

* 其中涉及到两个函数：`find_things()` 和 `find_things_extern()`，他俩的实现大同小异。原理都是去对应的 .c 源文件里寻找想要的字符串并稍微处理一下然后返回。
* 比如 `find_things  encoder  ENC      libavcodec/allcodecs.cc` 就会返回形如这个字符串：

		mpeg1video_encoder mpeg2video_encoder mpeg4_encoder msmpeg4v2_encoder msmpeg4v3_encoder msvideo1_encoder

* 函数具体分析见本文的“函数分析”部分
* 总之:
* `$ENCODER_LIST` 表示了当前 ffmpeg 的代码里支持的 encoder 列表，每种 encoder 以 `_encoder` 为后缀。`$ENCODER_LIST` 形如：`mpeg1video_encoder mpeg2video_encoder ...`
* `$DEMUXER_LIST` 表示了当前 ffmpeg 的代码里支持的 demuxer 列表，每种 demuxer 以 `_demuxer` 为后缀。`$DEMUXER_LIST`形如：`mov_demuxer mp3_demuxer ....`

* ...

####3.`CONFIG_LIST`
* 表示可以通过命令行配置的东西。
* 定义如下：

		CONFIG_LIST="
		    $COMPONENT_LIST
		    $DOCUMENT_LIST
		    $EXAMPLE_LIST
		    $EXTERNAL_LIBRARY_LIST
		    $HWACCEL_LIBRARY_LIST
		    $FEATURE_LIST
		    $LICENSE_LIST
		    $LIBRARY_LIST
		    $PROGRAM_LIST
		    $SUBSYSTEM_LIST
		    fontconfig
		    memalign_hack
		    memory_poisoning
		    neon_clobber_test
		    pic
		    pod2man
		    raise_major
		    thumb
		    valgrind_backtrace
		    xmm_clobber_test
		"
* 注意，`$COMPONENT_LIST` 是它的子集。

####4. `CMDLINE_SELECT`
* 这个变量保存能直接被命令行参数开关的东西，比如 ffplay，debug，asm 之类的。
* `COMPONENT_LIST` 定义如下：

		CMDLINE_SELECT="
		    $ARCH_EXT_LIST
		    $CONFIG_LIST
		    $HAVE_LIST_CMDLINE
		    $THREADS_LIST
		    asm
		    cross_compile
		    debug
		    extra_warnings
		    logging
		    lto
		    optimizations
		    rpath
		    stripping
		"

* 全部展开之后比较长，就不在这儿贴代码了，总之这些东西都是在 configure 脚本里写死的。
* 注意` $CONFIG_LIST` 是它的子集。




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