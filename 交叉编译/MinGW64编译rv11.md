### `conflicting declaration 'typedef long int ptrdiff_t'`
* 原因分析：rvDefines.h 里面定义了一次 `ptrdiff_t`，： `'typedef long int ptrdiff_t'`。然后在 MinGW 的 `C:/msys64/mingw64/x86_64-w64-mingw32/include/crtdefs.h` 里也定义了  `ptrdiff_t`，：`#ifdef _WIN64 __MINGW_EXTENSION typedef __int64 ptrdiff_t;`
* 解决办法：把 rvDefines.h 里的注掉

### `sys/utsname.h: No such file or directory`
* 原因分析：egenc.cpp 里检测到如果是定义了宏 `_LINUX` 则会 include sys/utsname.h，而 mingw 里根本没这个头文件
* 解决办法：
	* 方案1：借助 cygwin 的头文件，然后运行时借助 cygwin1.dll。 这个方法不好，因为我们最终编出来的是个dll库，给 Producer 用的，如果这个库同时有要用到 cygwin.dll 的话肯定不太好。
	* 方案2：针对 MinGW 写一个 makefile，代码需要做相应的修改

### `_LINUX` 这个宏是在哪定义的
* `linux\centos-64bit\include` 里面有几个头文件里都定义了这个宏
	* `Datatype_rn_rm_video_codec_rvplus_rv11dec_adec4_ribodefs.h `
	* `Datatype_rn_rm_video_codec_rvplus_rv11dec_drvc_ribodefs.h`
	* `Datatype_rn_rm_video_codec_rvplus_rv11dec_rv40dec_ribodefs.h`
	* `Datatype_rn_rm_video_codec_rvplus_rv11enc_aenc4_ribodefs.h`
	* `Datatype_rn_rm_video_codec_rvplus_rv11enc_erv4_ribodefs.h`
	* `Datatype_rn_rm_video_codec_rvplus_rv11enc_rv4enc_ribodefs.h`

### 用到`_LINUX` 这个宏的地方
* 很多地方是做内存对其用的，跟 Windows 有区别，这里是否有优化空间？ 毕竟 MinGW 编出来的库是跑着 Windows 上，如果按 Linux 对其方式是否影响效率？ 需要参考 x265

		// datatype_rn\rm\video\codec\rvPlus\rv11com\x86_avx\avx_wrap.cpp
		#if defined(_LINUX) || defined(_UNIX)
		        I16 __attribute__(aligned(16)) buf[4*4];
	
		// datatype_rn\rm\video\codec\rvPlus\rv11enc\enc\ecore.cpp
		#if defined(_LINUX) || defined(_UNIX) || defined(ARM) || defined(_ARM_)
		    I16 __attribute__((aligned(16))) dequant[256];

* 这里不知道会不会对 Windows 上的可执行库有影响：

		// datatype_rn\rm\video\codec\rvPlus\rv11enc\frontend\interface.h		
		#if defined(_LINUX) || defined(_UNIX)
		#define S_OK ((U32)0L)
		#define S_FALSE ((U32)0L)
		#define E_FAIL ((U32)0x80004005L)
		#endif

* sys/utsname.h 这个头文件 MinGW 没有，需要想办法

		// egenc.cpp
		#if defined(_LINUX)
		#include <stdlib.h>
		#include <string.h>
		#include <sys/utsname.h>
		#include <unistd.h>
		#endif

* 检测 cpuid 用的函数需要包含cpuid.h，MinGW 也有这个头文件，应该没影响

		// piacpu.cpp
		#elif defined(__linux) || defined(_LINUX) || defined(_UNIX)
		#include <cpuid.h>

* 其他一些地方用到类似如下的：

		//RateControl.h, ecore.cpp
		#if defined(_LINUX) || defined(_UNIX)
		#include <math.h>
		#include <string.h>
		#endif
		
		//rc2pass.h
		#if defined(_LINUX) || defined(_UNIX)
		#ifndef _MAX_PATH
		#define _MAX_PATH 260
		#endif
		#endif
		
		//rv40.cpp
		#if defined(_LINUX) || defined(_UNIX)
		#include "stdlib.h"
		#endif


### 因为 MinGW64 会设置宏 `_WIN32`

* 参考： [http://blog.csdn.net/liangbch/article/details/36020391](http://blog.csdn.net/liangbch/article/details/36020391)

* 而我们的代码里有N多地方是只要判断 `_WIN32` 被设置了，就按照 VS 来使用库函数
* 比如使用 windows 特有的 AVI 相关的函数，MinGW是没有的，这时候就会出错
* 所以很多地方