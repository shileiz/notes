* 首先第一步：

		ProjectGenerator.ConfigGenerator.passConfig(argc, argv)

* passConfig() 里比较重点的几步：
	1.  passConfigureFile()

####1. passConfigureFile() 分析

#####1.1
* 调用 `loadFromFile()`：把 configure 脚本的**全部文本**读入到 `m_sConfigureFile` 里面

#####1.2
* 找到 configure 里的 `#define FFMPEG_CONIG_H` 到 `EOF` 之间的文本，按逻辑存入 `m_vFixedConfigValues` 里面。
* 因为 configure 脚本的这部分内容是要写入 config.h 的（用 `cat > $TMPH <<EOF`），所以我们的程序也要把它保存起来。
* 这步结束后`m_vFixedConfigValues` 形如:

		----------------------------------------
		| m_sOption: FFMPEG_CONFIGURATION       |
		| m_sValue:  ""                         |
		| m_sPrefix:                            |
		| m_bLock:   false                      |
		----------------------------------------
		----------------------------------------
		| m_sOption: FFMPEG_LICENSE             |
		| m_sValue:  "lgpl"                     |
		| m_sPrefix:                            |
		| m_bLock:   false                      |
		----------------------------------------
		.
		.
		.
		----------------------------------------
		| m_sOption: SWS_MAX_FILTER_SIZE        |
		| m_sValue:  256                        |
		| m_sPrefix:                            |
		| m_bLock:   false                      |
		----------------------------------------

* `m_vFixedConfigValues` 是个装着 ConfigPair 的 vector，即 ValuesList：

		:::C++
		typedef vector<ConfigPair> ValuesList;
		ValuesList m_vFixedConfigValues;


#####1.3
* 找到 configure 里紧随上面那部分之后的 `print_config` 部分，并把需要写入文件的值按逻辑存入 `m_vConfigValues` 里
* 因为 configure 里 `print_config` 函数的意思就是往文件（config.h, config.mak...）里写东西，所以我们必须保存这些值。
* 关于 configure 的 `print_config` 函数，可以看这里：《ffmpeg的configure脚本分析.md》
* 我们的程序需要模拟 configure 这个 shell 脚本的工作，把 `print_config` 函数干的事情做一遍。我们要将其参数变量进行展开。展开变量是一个递归的过程，并且，变量还有可能是由 shell 函数得到的，所以这部分稍微复杂。
* 这部分的实际工作是调用函数 `passConfigList(sPrefix, "", sList)` 完成的。passConfigList() 又调用了递归展开变量的 getConfigList() 函数。
* 注意：这一步需要从一些 c 文件里读取内容，这一做法跟 configure 脚本是一致的。configure 的 `find_things` 函数就是这么干的。我们的程序有个函数：passFindThings()，模拟了 configure 的`find_things` 函数。
* 总之，这一步结束之后，`m_vConfigValues` 形如：

		----------------------------------------
		| m_sOption: X86_64                     |
		| m_sValue:                             |
		| m_sPrefix: ARCH_                      |
		| m_bLock:   false                      |
		----------------------------------------
		----------------------------------------
		| m_sOption: ARM                        |
		| m_sValue:                             |
		| m_sPrefix: ARCH_                      |
		| m_bLock:   false                      |
		----------------------------------------
		.
		.
		.
		----------------------------------------
		| m_sOption: SSE42                      |
		| m_sValue:                             |
		| m_sPrefix: HAVE_                      |
		| m_bLock:   false                      |
		----------------------------------------
		.
		.
		.
		----------------------------------------
		| m_sOption: LIBVPX                     |
		| m_sValue:                             |
		| m_sPrefix: CONFIG_                    |
		| m_bLock:   false                      |
		----------------------------------------
		----------------------------------------
		| m_sOption: ENCODERS                   |
		| m_sValue:                             |
		| m_sPrefix: CONFIG_                    |
		| m_bLock:   false                      |
		----------------------------------------

* 这个变量超大，有2000多个节点


###若干函数分析

####1.getConfigList()
* 该函数在 configure 脚本里展开“`XXXX_LIST`”变量。第一个参数是要找的变量名，第二个参数用来带出返回值，是一个 `vector<string>`。
* 该函数在展开变量时可以理解 configure 脚本里的 `find_things`，`find_things_extern`，`add_suffix`，`filter_out` 函数，并模拟这些函数，对变量做相应处理。
* 举例：

	    vector<string> vList;
	    getConfigList("PROGRAM_LIST", vList);

* 以上代码将在 configure 脚本里解析出 `$PROGRAM_LIST` 的最终值，把结果存到 vList 里。vList 的结果将是："ffplay","ffprobe","ffserver","ffmpeg"
* 以上变量 `PROGRAM_LIST` 在 configure 脚本里是直接定义的，所以展开也比较容易。 getConfigList() 只是递归的去找 `PROGRAM_LIST=` 这种语句就能解决。
* 还有的变量是通过函数返回值获取的，比如 `ENCODER_LIST`。当 getConfigList() 去找 `ENCODER_LIST=` 时，最终会找到这句：

		ENCODER_LIST=$(find_things  encoder  ENC      libavcodec/allcodecs.c)

* 因为这是通过调用 `find_things` 函数得到的，所以此时 getConfigList() 就开始模拟 `find_things` 函数做事了。也会去打开 `libavcodec/allcodecs.c` 文件，找到匹配的行，提取想要的文本，加上后缀。
* 注： configure 脚本的 find_things 函数在 《ffmpeg的configure脚本分析.md》 里有分析。 

####2.toggleConfigValue()
* 更改 `m_vConfigValues` 里元素的值，把元素的 `m_sValue` 置1或者0。
* 举例：

		toggleConfigValue("ffplay", true);

* 将把 `m_vConfigValues` 里 `m_sOption==FFPLAY` 那个元素的 `m_sValue` 置1:

		----------------------------------------
		| m_sOption: FFPLAY                     |
		| m_sValue:  1                          |
		| m_sPrefix: CONFIG_                    |
		| m_bLock:   false                      |
		----------------------------------------