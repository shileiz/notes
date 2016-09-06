### 关于libhevc代码
* 这套代码是解码h265用的
* 这套代码来自于Android 5.1.1源码，在源码的 `external/libhevc` 目录里。
* 代码是Ittiam公司开发的

### libhevc代码结构概述
* 根目录下有3个文件夹和8个mk文件：
* 3个文件夹分别是：common、decoder、test。
* 其中 common 和 decoder 中的代码是解码h265需要的所有功能
* 这两个文件夹加上根目录下的mk文件，能编译出一个解码h265的库。
* test 文件夹里的东西是教你怎么调用这个库的。

### 编译出libhevc库
* 正如上面所说，所有的解码功能的代码都在 common 和 decoder 目录下
* 而那8个mk文件是为了编译成 Android 平台上的库而提供的makefile
* 如果用这套代码编译window平台上的库，其实不用那8个makefile也行的

### 编译Android平台的静态库
* 环境信息：
	* Windows 10 
	* ndk-r10d
* 把 common、decoder、Android.mk、decoder.mk、decoder.arm.mk 拷贝到一个新的目录里
* 把这个目录重命名叫jni
* 修改 decoder.mk 把以下行注释掉：

		:::makefile
		# include $(LOCAL_PATH)/decoder.arm64.mk
		# include $(LOCAL_PATH)/decoder.x86.mk
		# include $(LOCAL_PATH)/decoder.x86_64.mk
		# include $(LOCAL_PATH)/decoder.mips.mk
		# include $(LOCAL_PATH)/decoder.mips64.mk

* 修改 decoder.mk 在最后一行的前面，加入以下内容：

		:::makefile
		LOCAL_SRC_FILES += $(LOCAL_SRC_FILES_arm)
		LOCAL_CFLAGS += $(LOCAL_CFLAGS_arm)
		LOCAL_C_INCLUDES += $(LOCAL_C_INCLUDES_arm)

* 告诉NDK你想编译的目标CPU平台：新建一个Application.mk, 里面写上 `APP_ABI := armeabi-v7a`
* 在jni目录中运行 `ndk-build.cmd`， 进行编译
* 编译结束后在跟jni同级目录会生成一个obj目录，去 `obj/local/armeabi-v7a里` 找到libhevcdec.a

### 一些说明
* 上述编译的静态库是给armv7平台使用的，并且没有开启NEON优化。
* 开启armv7 NEON优化的方法是修改decoder.arm.mk：
	* 在第一行写上： `ARCH_ARM_HAVE_NEON := true`
	* 在 `ifeq ($(ARCH_ARM_HAVE_NEON),true)` 下面一行写上： `LOCAL_ARM_NEON  := true`
* 如果要编译armv8-64bit平台，并且开启NEON优化，那么就要:
	* 把Application.mk改为：`APP_ABI := arm64-v8a`
	* 把 decoder.arm64.mk 拷贝到jni目录里
	* 把decoder.mk里 `include $(LOCAL_PATH)/decoder.arm64.mk` 这行注释打开，把 `include $(LOCAL_PATH)/decoder.arm.mk` 注释掉
	* 把decoder.mk里这三行改成arm64对应的：

			:::makefile	
			LOCAL_SRC_FILES += $(LOCAL_SRC_FILES_arm64)
			LOCAL_CFLAGS += $(LOCAL_CFLAGS_arm64)
			LOCAL_C_INCLUDES += $(LOCAL_C_INCLUDES_arm64)

### 使用Android平台的静态库
* 其实这里把详细步骤罗列下来意义不大，关键还是需要理解“如何使用提前编译好的库”，只要理解了，下面的步骤都不用看也能做出来。
* 总则：通过libhevc提供的 `test/main.c` 来使用编译出来的静态库
* 我们不直接调用libhevc.a里的函数，我们调用main.c里的main函数进行解码，而main去调用libhevc.a里的函数
* Eclipse新建一个Android项目，添加 Native Support，给自己的库命名为 mydecode
* 手动建立Application.mk，写上 `APP_ABI := armeabi-v7a`
* 把提前编译出的libhevcdec.a拷贝到项目的jni目录下
* 把libhevc提供的main.c拷贝到jni目录下
* 在项目的jni目录新建include目录，把libhevc提供的common文件夹和decoder文件夹拷贝到include里，当然，除了.h之外的文件都可以不拷。
* 把自动生成的 mydecode.cpp 重命名为 mydecode.c
* 修改自动生成的Android.mk，改后的全部内容如下：

		:::makefile
		LOCAL_PATH := $(call my-dir)
		
		include $(CLEAR_VARS)
		LOCAL_MODULE    := pre-libhevc
		LOCAL_SRC_FILES := libhevcdec.a
		LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include/common $(LOCAL_PATH)/include/decoder
		include $(PREBUILT_STATIC_LIBRARY)
		
		include $(CLEAR_VARS)
		LOCAL_MODULE    := mydecode
		LOCAL_SRC_FILES := mydecode.c main.c
		LOCAL_STATIC_LIBRARIES := pre-libhevc
		include $(BUILD_SHARED_LIBRARY)

* 修改MainActivity.java，load mydecode库，定义一个native方法叫mydecode()，具体细节不写了。
* 修改mydecode.c，实现native方法，修改完的mydecode.c如下：

		:::c
		#include <jni.h>
		JNIEXPORT jint JNICALL Java_com_zsl_useprebuidlibhevc_MainActivity_mydecode
		  (JNIEnv *env, jobject obj)
		{
			char* args[] = {"----", "--input", "/sdcard/test.hevc", "--num_frames", "-1"};
			int r = main(sizeof(args)/sizeof(char*), args);
			return r;
		}

* 修改main.c解决NULL没有define的问题：在最前面加一行：`#define NULL ((void *)0)`
* 把/sdcard里放一个test.hevc就可以部署测试了，解码应该是没有问题的
* 注意：不要让解码线程跑在UI线程里，这样会报一个libc的错(有时候第二次解码才会发生)，追都追不下去。

### 编译Windows平台的静态库
* 总体思路跟Android差不多，先编译一个静态库出来，然后把main.c加上静态库编成一个exe，运行这个exe来进行hevc的解码
* 直接把所有代码拿到VS里编译，难免会因为编译器的不同遇到一些问题，遇到了就对源码做一点点修改即可。
* 在VS2015中编译libhevc成静态库的具体步骤：
* 新建一个Win32项目，起名libhevcdec，下一步选静态库，取消勾选预编译头
* 项目管理面板上，源文件文件上右键--->添加--->现有项目，添加common、common\x86、decoder、decoder\x86里的所有.c文件
* 添加附加包含目录，把common、common\x86、decoder、decoder\x86都加进去
* 修改 `ihevc_platform_macros.h`，因为VS编译器和GCC的不同，需要修改以下几个地方：
	* `#define INLINE inline` 改为 `#define INLINE` 
	* 修改GNU-C特有函数 `__builtin_clz` 和 `__builtin_ctz` 对应代码段如下：   
		* 把 

				:::c	
				if(u4_word)
			        return (__builtin_clz(u4_word));
			    else
			        return 32;

		* 改为  

				:::c
				if(u4_word)
				{
					int c = 0;
					while((u4_word&0x80000000)==0)
					{
						c++;
						u4_word<<=1;
					}
					return c;
				}
				else
					return 32;	

		* 把  

				:::c
				if(0 == u4_word)
			        return 31;
			    else
			    {
			        unsigned int index;
			        index = __builtin_ctz(u4_word);
			        return (UWORD32)index;
			    }  
		
		* 改为  

				:::c	
			    if(0==u4_word)
					return 31;
				else
				{
					int c=0;
					while((u4_word&1)==0)
					{
						c++;
						u4_word>>=1;
					}
					return c;
				}

		* 说明：函数`__builtin_clz()`的作用是数一数参数最左边有多少个零的，这里由于VS没有这个函数，就自己实现一下。  函数`__builtin_ctz()`也类似，是数一数参数最右边有多少个零的。

	* 修改GNU-C特有的`__attribute__ ((attribute-list))`对应的代码段如下：  
		* 把  

				:::c
				#define MEM_ALIGN8 __attribute__ ((aligned (8)))
				#define MEM_ALIGN16 __attribute__ ((aligned (16)))
				#define MEM_ALIGN32 __attribute__ ((aligned (32)))
	
		* 改为  

				:::c
				#define MEM_ALIGN8 
				#define MEM_ALIGN16 
				#define MEM_ALIGN32 

		* 说明：`__attribute__ ((attribute-list))`的作用是通知编译器（gcc）内存对其方式。我们这里不需要，直接改掉就可以。

* 为项目添加宏（相当于gcc的-D选项），因为很多代码段需要定义了`X86_MSVC`这个宏才能到达，所以添加这个宏。
	* 右键--->属性--->C/C++--->预处理器--->预处理器定义--->编辑。把`X86_MSVC`添加上就行了 

* 生成项目，就会编译出 libhevcdec.lib

### 使用Windows平台的静态库 ###
* 上面已经编译出win32的静态库了，下面基于这个库和main.c来做一个exe。直接写详细步骤：
* 新建一个空的win32应用程序项目，在源文件里添加刚才编译出来的libhevcdec.lib
* 在源文件里添加libhevc提供的main.c
* 配置项目属性，附加包含目录添加common\x86，common，decoder
* 添加宏 `DISABLE_AVX2`
	* 右键--->属性--->C/C++--->预处理器--->预处理器定义--->编辑。把`DISABLE_AVX2`添加上就行了

* 生成项目，得到exe可执行文件。