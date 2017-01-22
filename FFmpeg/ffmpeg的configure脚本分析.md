###大面儿
* configure 脚本是个 shell 脚本，主要用来根据用户的命令行参数**以及当前的ffmpeg代码**，生成 config.h, config.mak, config.asm, config.texi
* 其中 config.h 和 config.mak 是通常我们比较关心的，因为编译阶段会用到
* 需要注意的是，configure 不光根据用户设置的命令行参数（比如 --disable-ffplay 之类），还会根据当前的ffmpeg代码来生成这几个文件。比如需要从 libavcodec/allcodecs.c 里读出当前版本的 ffmpeg 代码支持哪些 encoder 和 decoder。

###大致过程
* 检查 shell 版本等，不用细研究
* 定义函数，用到再看
* 定义变量，设置变量默认值：`XXX_LIST`，`xxx__deps`，`xxx_select` ，`xxx_default` 等等。
	* 此时 cc\_default=gcc，arch\_default=$(uname -m) 即 x86\_64，CC\_C=-c
	* 另外还夹杂着执行了一系列的 enable 函数，把默认打开的东西都 enable 了。

* 解析命令行参数：即执行 `for opt do` 语句块（一.）
	* 结果是导致一些变量被设置，比如可能导致设置了变量 toolchain=msvc  

* 根据 `$toolchain` 改变一些变量的值：即执行 `case "$toolchain" in` 语句块。 （二.）
	* 可能被改变的变量如：`cc_default`，`ld_default`，`LDFLAGS` 等等。  

* 结合用户设置、默认值、本机情况，设置编译相关的变量。（三.）
	* 结果是如下变量被正确的设置： `CC_O，CC_C，CC_E，LD_O，LD_LIB，LD_PATH，CFLAGS，LDFLAGS ....`  

* 测试一下 pkg-config 和 doxygen 是否可用
* 设置临时文件，设置完成后，代表临时文件的变量形如:

		TMPASM=/tmp/ffconf.8TjmLkMB.asm
		TMPC=/tmp/ffconf.XpKINdgl.c
		TMPH=/tmp/ffconf.YJgQgMz4.h

* 测试编译器。(四.)
	* 。。。。。。


###（一.）解析命令行参数
* configure 脚本里真正干活的代码（除了检测shell等）是从命令行解析开始的，即 `for opt do` 这一行开始。
* 这个 for 没有写 in 什么，所以它是对命令行参数循环，每次循环 opt 等于一个命令行参数，比如 --enable-ffplay。
* 这个for里是一个 case 语句，判断参数是哪种形式的，不同形状的参数不同处理。我们只分析平时常用的几种形式：
	1. `--enable-xxx=xxx / --disable-xxx=xxx`， 比如 `--enable-filter=rotate`
	2. `--enable-xxx /--disable-xxx`，比如 `--disable-ffserver`
	3. `--var=value`，比如 `--toolchain=msvc`

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

##### 2.小结
* 传入 `--enable-ffpaly` 相当于生成变量 ffplay，并赋值为 yes。
* 传入 `--disable-encoders` 相当于 把 `$ENCODER_LIST`  里的变量全部 unset。
* `$ENCODER_LIST` 里的变量是从 allcodecs.c 里读出来的，详见本文的“变量分析部分”

#### 3.`--var=value`，比如 `--toolchain=msvc`
* 该部分代码如下：

		for opt do
		    optval="${opt#*=}"
		    case "$opt" in
		    ...
	        *)
	            optname="${opt%%=*}"
	            optname="${optname#--}"
	            optname=$(echo "$optname" | sed 's/-/_/g')
	            if is_in $optname $CMDLINE_SET; then
	                eval $optname='$optval'
	            elif is_in $optname $CMDLINE_APPEND; then
	                append $optname "$optval"
	            else
	                die_unknown $opt
	            fi
	        ;;

* 比如用户设置了 `--toolchain=msvc`，则脚本中生成变量 toolchain，其值为 msvc，否则没有这个变量。
* 如果用户写的 `var` 不在 `$CMDLINE_SET` 和 `$CMDLINE_APPEND` 里，则报错退出。


###（二.）根据 `$toolchain` 改变一些变量的值
* 如果用户没有设置 --toolchain= 参数，则 $toolchain 是空，不会走到任何一个分支，不会有变量被改变。 
* 假如用户设置了 `--toolchain=msvc`，则：
	*  cc\_default 会变成 cl
	*  ar\_default 会变成 lib
	*  ld\_default 会变成 mslink (ffmepg 为寻找微软的 linker 写了个脚本，放在 compat/windows/mslink)
* 某些分支里会调用 `add_ldflags` 、 `add_cflags` 这种函数，而 `add_ldflags` 实际是调用 append 函数。append 函数会改变或**创造变量**。比如第一次对变量 LDFLAGS调用 `append LDFLAGS -fPIC`，则会创造变量 LDFLAGS，值为 -fPIC。第二次对 LDFLAGS 调用 append，则只是往后加东西。

###(三.) 结合用户设置、默认值、本机情况，设置编译相关的变量。
* 编译相关的变量比如 `CC，LD，CC_O，LD_O，LD_PATH，CFLAGS`，对于不同的目标平台是不一样的。
* 要以用户指定的为准，用户没指定则使用默认值。还要结合当前执行 configure 的机器上安装的编译器版本来设定。
* 一般的套路如下，举两个例子：

#### 例1. 对编译器 cc 相关的变量的处理：
* 1.`set_default cc `
	* 如果用户设置了  `--cc=xxx`，则该函数不起作用。因为在之前的命令行解析里就会生成变量cc=xxx，`set_default` 函数对已存在的变量不做操作。用户的设置最优先
	* 如果用户设置了 `--toolchain=msvc`，则该函数起作用，将 cc 设置成 `cc_default`。而因为用户设了 `--toolchain=msvc`，则 `cc_default=cl`。还是用户设置的起了作用
	* 如果用户什么都没设，则 cc 被设置成 `cc_default`，即 gcc。
	* 总之，这步结束，`$cc=gcc` 或者 `$cc=cl`
* 2.调用：`probe_cc cc "$cc" "true"`
	* 该函数的作用是，运行第二个参数 "$cc" 指定的编译器，根据输出结果设置一些变量
	* 比如 "$cc" 是 cl 的时候，会设置如下变量：
	
			_flags_filter=msvc_flags
			_cflags='-D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS'
			_cflags_speed="-O2"
			_cflags_size="-O1"
			_flags='-nologo'
			_cc_o='-Fo$@'
        	_cc_e='-P -Fi$@'
			
			...
	* 这步设置的变量都是为了一下一步使用的		

* 3.根据 `probe_cc` 的结果设置如下变量：

		cflags_filter=$_flags_filter
		cflags_speed=$_cflags_speed
		cflags_size=$_cflags_size
		cflags_noopt=$_cflags_noopt
		# add_cflags 会创建变量 CFLAGS
		add_cflags $_flags $_cflags
		cc_ldflags=$_ldflags
		# 下面的几行被集成到函数 set_ccvars() 里了
		eval CC_C=\${_cc_c-\${CC_C}}
		eval CC_E=\${_cc_e-\${CC_E}}
		eval CC_O=\${_cc_o-\${CC_O}}
        eval CCDEP=\${_DEPCMD:-\$DEPCMD}
        eval CCDEP_FLAGS=\${_DEPFLAGS:-\$DEPFLAGS}
        eval DEPCCFLAGS=\$_flags
	
	* 注意，以下几个变量在configure脚本最开始的设置变量默认值阶段已经被赋了默认值：
	
			CC_C='-c'
			CC_E='-E -o $@'
			CC_O='-o $@'
	
	* 而以下几个变量在开始阶段并未被初定义：
	
			CCDEP
			CCDEP_FLAGS
			DEPCCFLAGS	

	* 总之，这步结束之后，几个相关变量如下（以 cl 编译器为例）：
	
			cflags_filter=msvc_flags
			cflags_speed="-O2"
			cflags_size="-O1"
			cflags_noopt=
			CFLAGS=-nologo -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS
			# CPPFLAGS 到此为止还未定义
			cc_ldflags=
			CC_C='-c'
			CC_E='-P -Fi$@'
			CC_O='-Fo$@'
			CCDEP=$(DEP$(1)) $(DEP$(1)FLAGS) $($(1)DEP_FLAGS) $< 2>&1 | awk '/includub(/^.*file: */, ""); gsub(/\\/, "/"); if (!match($$0, / /)) print "$@:" > $(@:.o=.d)
			CCDEP_FLAGS=$(CPPFLAGS) $(CFLAGS) -showIncludes -Zs
			DEPCCFLAGS=-nologo

	* 如果以 gcc 为例，则几个重要的变量在这步结束后如下：
	
			CC_C=-c
			CC_E=-E -o $@
			CC_O=-o $@
			# CFLAGS 和 CPPFLAGS 到此为止还未定义

		
#### 例2. 对连接器 ld 相关的变量的处理：
* 1.`set_default ld `
	* 总之，这步结束之后，ld 的值是 ld 或者 mslink

* 2.调用 `probe_cc ld "$ld"`
	* 总之，这步结束之后，几个下划线开头的变量如下（以 mslink 为例）： 

			_ld_o='-out:$@'
	        _ld_lib='lib%.a'
	        _ld_path='-libpath:'
	        _flags='-nologo'

* 3.根据 `probe_cc` 的结果设置如下变量：

		ldflags_filter=$_flags_filter
		add_ldflags $_flags $_ldflags
		# 下一句的 test 会失败，所以不会执行 && 后的语句
		# 因为 $cc_type 和 $ld_type 在 probe_cc 里，都被赋值为了 $_type
		# 只有调用 probe_cc cc cl，和 probe_cc ld mslink 时走进了不同分支才会出现不相等，这种情况只有用户单独加了 --ld=xxx 这种才有可能出现，一般不会。  
		test "$cc_type" != "$ld_type" && add_ldflags $cc_ldflags
		LD_O=${_ld_o-$LD_O}
		LD_LIB=${_ld_lib-$LD_LIB}
		LD_PATH=${_ld_path-$LD_PATH}

	* 总之，这步结束之后，几个相关变量如下（以 mslink 连接器为例）：
	
			ldflags_filter=msvc_flags
			LDFLAGS=-nologo
			LD_O=-out:$@
			LD_LIB=lib%.a
			LD_PATH=-libpath:

* 把用户指定的 `extra_xxxflags` 设置到相应的变量里去：

		add_cflags $extra_cflags
		add_cxxflags $extra_cxxflags
		add_asflags $extra_cflags

* 如果用户加了 --sysroot 选项，则给 gcc 族的编译器加上 sysroot 选项（没细看）：

		if test -n "$sysroot"; then
		...

* 做交叉编译的相关判断和设置（没细看）：

		if test "$cpu" = host; then
		...
* 设置 arch 变量

		case "$arch" in
			...
		    i[3-6]86*|i86pc|BePC|x86pc|x86_64|x86_32|amd64)
		        arch="x86"
		...
		enable $arch

* 设置 cpu 特定 flag（没细看）

###(四.) 检测编译器连接器是否能工作
* 这里涉及到几个函数和几个变量。基本可以见名知意:

		TEMPC   // 临时 .c 源文件
		TEMPO   // 临时 .o 目标文件
		TEMPE	// 临时 .exe 可执行文件

		check_cmd  //执行传入的参数
		check_cc   //编译 TMPC，生成 TMPO
		check_ld   //先调用 check_cc 生成 TMPO，再把 TMPO 链接生成 TMPE
		check_exec //先调用 check_ld 生成 TMPE，再执行 TMPE
		
* 检测是否能生成可执行程序：

		check_exec <<EOF
		int main(void){ return 0; }
		EOF
		if test "$?" != 0; then
			# 检测失败的处理

* 这段代码相当于：
	* 把 `int main(void){ return 0; }` 写入 TMPC 里。
	* 用 `gcc -c -o $TMPO $TMPC` 来编译 TMPC，生成 TMPO.(对于 msvc 来说，把 gcc 换成 cl)
	* 用 `gcc -o $TMPE $TMPO` 链接 TMPO，生成 TMPE.(对于 msvc 来说，是 `mslink -out:$TMPE $TMPO` )
	* 执行一下 TMPE，检测返回值是否为 0


###X.变量分析

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
* `CMDLINE_SELECT` 定义如下：

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

####5. `CMDLINE_SET`
* 这个变量保存的是通过命令行设置的变量。如果命令行设置了该变量的值，则整个脚本里以设置的为准。否则会有默认值（或者未设置？）。
* 比如命令行加了 --cc=gcc，则脚本中会生成变量cc并设置其值为gcc

		CMDLINE_SET="
		    $PATHS_LIST
		    ar
		    arch
		    as
		    assert_level
		    build_suffix
		    cc
		    cpu
		    cross_prefix
		    cxx
		    dep_cc
		    doxygen
		    env
		    extra_version
		    gas
		    host_cc
		    host_cflags
		    host_ld
		    host_ldflags
		    host_libs
		    host_os
		    install
		    ld
		    logfile
		    malloc_prefix
		    nm
		    optflags
		    pkg_config
		    pkg_config_flags
		    progs_suffix
		    random_seed
		    ranlib
		    samples
		    strip
		    sws_max_filter_size
		    sysinclude
		    sysroot
		    target_exec
		    target_os
		    target_path
		    target_samples
		    tempprefix
		    toolchain
		    valgrind
		    yasmexe
		"

		PATHS_LIST="
		    bindir
		    datadir
		    docdir
		    incdir
		    libdir
		    mandir
		    pkgconfigdir
		    prefix
		    shlibdir
		"



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


### `set_default()` 函数
* 只有在被 set 的变量不存在或者为空的时候，才会进行设置。
* 比如 `set_default cc`，如果 cc 不存在，或者 cc 为空，则会把 cc 赋值为 `$cc_default`，否则啥也不干。

### `probe_cc()` 函数
* 该函数需要3个参数，第一个参数是前缀，第二个参数是编译器（如gcc，cl等），第三个参数不知道干啥的
* 最重要的是第二个参数，函数会调用该编译器，根据输出结果来做些决定。我们常用的 gcc 和 cl 为例:

		probe_cc(){
		    pfx=$1
		    _cc=$2
		    first=$3
		
		    unset _type _ident _cc_c _cc_e _cc_o _flags _cflags
		    unset _ld_o _ldflags _ld_lib _ld_path
		    unset _depflags _DEPCMD _DEPFLAGS
		    _flags_filter=echo
		
		    if $_cc --version 2>&1 | grep -q '^GNU assembler'; then
			...

			# gcc 会走到这个分支
		    elif $_cc -v 2>&1 | grep -qi ^gcc; then
		        _type=gcc
		        gcc_version=$($_cc --version | head -n1)
		        gcc_basever=$($_cc -dumpversion)
		        gcc_pkg_ver=$(expr "$gcc_version" : '[^ ]* \(([^)]*)\)')
		        gcc_ext_ver=$(expr "$gcc_version" : ".*$gcc_pkg_ver $gcc_basever \\(.*\\)")
		        _ident=$(cleanws "gcc $gcc_basever $gcc_pkg_ver $gcc_ext_ver")
		        case $gcc_basever in
		            2*) _depflags='-MMD -MF $(@:.o=.d) -MT $@' ;;
		        esac
		        if [ "$first" = true ]; then
		            case $gcc_basever in
		                4.2*)
		                warn "gcc 4.2 is outdated and may miscompile FFmpeg. Please use a newer compiler." ;;
		            esac
		        fi
		        _cflags_speed='-O3'
		        _cflags_size='-Os'
			...

			# cl 会走到这个分支
		    elif $_cc -nologo- 2>&1 | grep -q Microsoft; then
		        _type=msvc
		        _ident=$($_cc 2>&1 | head -n1)
		        _DEPCMD='$(DEP$(1)) $(DEP$(1)FLAGS) $($(1)DEP_FLAGS) $< 2>&1 | awk '\''/including/ { sub(/^.*file: */, ""); gsub(/\\/, "/"); if (!match($$0, / /)) print "$@:", $$0 }'\'' > $(@:.o=.d)'
		        _DEPFLAGS='$(CPPFLAGS) $(CFLAGS) -showIncludes -Zs'
		        _cflags_speed="-O2"
		        _cflags_size="-O1"
		        if $_cc -nologo- 2>&1 | grep -q Linker; then
		            _ld_o='-out:$@'
		        else
		            _ld_o='-Fe$@'
		        fi
		        _cc_o='-Fo$@'
		        _cc_e='-P -Fi$@'
		        _flags_filter=msvc_flags
		        _ld_lib='lib%.a'
		        _ld_path='-libpath:'
		        _flags='-nologo'
		        _cflags='-D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS'
		        disable stripping
		...
		    fi
		
		    eval ${pfx}_type=\$_type
		    eval ${pfx}_ident=\$_ident
		}

* 它会设置一些下划线开头的变量，如 `_cc_o，_cc_e，_ld_lib，_ld_path，_flags，_cflags ...`。一进函数时先把所有这些都 unset，走进每个分支再按编译器类型把必要的设置了。
* 对于 gcc 分支来说 `_cc_o，_cc_e，_ld_lib，_ld_path，_flags，_cflags` 这些都是空。
* 对于 cl 分支来说，`_cc_o，_cc_e，_ld_lib，_ld_path，_flags，_cflags ...` 都有相应的值，比如 ` _cc_o='-Fo$@'， _ld_lib='lib%.a'，_flags='-nologo'`
* 它设置的这些变量后再后续被用到。
* 另外，当走到 cl 分支的时候，它还额外执行了 disable stripping
* 另外，`probe_cc` 还会设置如下2个非下划线的变量：`cc_type=msvc, ld_type=msvc`, 或者 `cc_type=gcc, ld_type=gcc`

### `check_cc()` 函数
* 函数如下：

		check_cc(){
		    log check_cc "$@"
		    cat > $TMPC
		    log_file $TMPC
		    check_cmd $cc $CPPFLAGS $CFLAGS "$@" $CC_C $(cc_o $TMPO) $TMPC
		}

* `check_cc` 会把标准输入写入 TMPC，然后编译 TMPC。注意 `cat > $TMPC` 是把标准输入的内容写入 TMPC，而不一定是清空 TMPC。往往调用 `check_cc` 之前会重定向标准输入为一段 c 语言代码。比如 `check_exec` 会间接的调用到 `check_cc`，所以这调用 `check_exec` 时就重定向标准输入：

		check_exec <<EOF
		int main(void){ return 0; }
		EOF

* `check_cc` 调用的编译命令如下：

		$cc $CPPFLAGS $CFLAGS "$@" $CC_C $(cc_o $TMPO) $TMPC

* 其中 
	* `$cc` 是 gcc 或者 cl
	* `$CPPFLAGS` 一般情况下为空
	* `$CFLAGS` 对于 gcc 一般为空，对于 cl 为 `-nologo -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS`
	* `$@` 一般不传参数，为空
	* `$CC_C` 为 -c
	* `$(cc_o $TMPO)` 对于 gcc 来说是 -o $TMPO，对于 cl 来说是 -Fo$TMPO

---

##需要的shell知识

### 输入重定向：<<EOF
* `<<EOF ... EOF`: 把标准输入重定向成一段夹在两个 EOF 之间的文本
* 也不一定非得是 EOF，可以用任意非保留字把重定向的文本夹起来
* 注意结束的 EOF 需要顶着行首写
* 在调用函数的时候，可以重定向标准输入，这样，函数执行时的标准输入就是你重定向过的了：

		func1(){
			cat > tmp.txt
		}
		
		func1 <<EOF
		hello world !
		EOF

* 执行以上脚本文件，将生成 tmp.txt，其内容是 hello world !

### 函数或脚本的参数
* 函数或脚本传入3个参数: a b c
* 运行函数或脚本时：
* `$*` 为"a b c"（一起被引号包住）
* `$@` 为 "a" "b" "c"（分别被包住）
* `$#` 为3（参数数量）

### eval
* 忘了的话自己百度一下吧

### if

		if  [ -n $string  ]             如果string 非空(非0)
		if  [ -z $string  ]             如果string 为空

### ${}
* 正常情况下，${} 用来取出变量的值, 跟直接用$取变量值没区别：

		var=zsl
		echo ${var}
		# 以上将输出 zsl

* 取变量时用 `${var=defaultvalue}/${var:=defaultvale}` 的形式，可以设置默认值：

		unset var
		echo ${var=value1} 
		# 以上语句将输出 value1，因为取 var 时，并没有 var 这个变量，所以将其设置为 value1 这个默认值
		echo ${var=value2}
		# 将输出 value1，因为这时候 var 变量已经存在了，所以不再设置其值。
		unset var
		var=""
		echo ${var=value1}
		# 输出空，因为 var 变量已经存在(虽然var是空，但存在)，不再设置默认值
		echo ${var:=value1}	
		# 输出 value1, := 跟 = 的区别在于，:= 对于 var 是空时也设置默认值。

* 一个较为完整的用法列表：

		${file-my.file.txt} ：假如 $file 为空值，则使用 my.file.txt 作默认值。(保留没设定及非空值) 
		${file:-my.file.txt} ：假如 $file 没有设定或为空值，则使用 my.file.txt 作默认值。 (保留非空值) 
		${file+my.file.txt} ：不管 $file 为何值，均使用 my.file.txt 作默认值。 (不保留任何值) 
		${file:+my.file.txt} ：除非 $file 为空值，否则使用 my.file.txt 作默认值。 (保留空值) 
		${file=my.file.txt} ：若 $file 没设定，则使用 my.file.txt 作默认值，同时将 $file 定义为非空值。 (保留空值及非空值) 
		${file:=my.file.txt} ：若 $file 没设定或为空值，则使用 my.file.txt 作默认值，同时将 $file 定义为非空值。 (保留非空值) 
		${file?my.file.txt} ：若 $file 没设定，则将 my.file.txt 输出至 STDERR。 (保留空值及非空值)) 
		${file:?my.file.txt} ：若 $file 没设定或为空值，则将 my.file.txt 输出至 STDERR。 (保留非空值) 
		
### 冒号：
* 冒号是一个 linux 命令，它啥也不干，有一点类似 python 的 pass。不过它可以带参数。
* 冒号干的唯一一件事就是把参数展开和执行用户指定的输入输出重定向。一下是 man page 的描述：

		: [arguments]
		    No effect; the command does nothing beyond expanding arguments and performing any specified redirections. A zero exit code is returned.

* 不带参数的冒号可以用作语句块的占位符，跟 python 的 pass 是一个意思
* 带参数的冒号可以跟 ${var:=default} 配合，执行变量的赋值。
* 因为直接写 `${var:=default}` 相当于把 var 的值取出来并执行，我们并不想执行，只是想给 var 赋值。所以我们利用冒号会展开参数这个特性来处理：

		: ${var:=default}

* 如果 var 不存在或者为空，则var被赋值为 default，否则var不变。

### shift

* 参数左移，移出去的参数就被销毁了
* 参考：[http://blog.csdn.net/zhu_xun/article/details/24796235](http://blog.csdn.net/zhu_xun/article/details/24796235)

		#!/bin/bash  
		echo "参数个数为：$#,其中："  
		for i in $(seq 1 $#)  
		do  
		  eval j=\$$i  
		  echo "第$i个参数($"$i")：$j"  
		done  
		  
		shift 3  
		  
		echo "执行shift 3操作后："  
		echo "参数个数为：$#,其中："  
		for i in $(seq 1 $#)  
		do  
		  #通过eval把i变量的值($i)作为变量j的名字  
		  eval j=\$$i  
		  echo "第$i个参数($"$i")：$j"  
		done  

* 以参数 a b c d e 调用上述脚本，输出结果为：

		参数个数为：5,其中：
		第1个参数($1)：a
		第2个参数($2)：b
		第3个参数($3)：c
		第4个参数($4)：d
		第5个参数($5)：e
		执行shift 3操作后：
		参数个数为：2,其中：
		第1个参数($1)：d
		第2个参数($2)：e

* 不加参数直接 shift 相当于 shift 1