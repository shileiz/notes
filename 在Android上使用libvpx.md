libvpx 不包括webm文件的解析，只负责编解码。  
解析webm需要用到 libwebm。  
libwebm 在源码目录的：`libvpx/third_party/libwebm`  
libwebm 可以单独编译成一个库。  
libvpx 使用了库 libwebm（可选，可以在config的时候打开或者关闭wenm-io）。  
libvpx 使用 libwebm 的时候，给 libwebm 外面又封装了一层，使得用起来更加方便。封装在: `libvpx/webmenc.cc`  和 `libvpx/webmdec.cc`。相应的头文件也在同文件夹下。  
类似的封装还有其他几个，c++源文件和头文件也都在libvpx根目录下。  
我们使用 libvpx 解码是参考 `libvpx/vpxdec.c`。  
它在处理webm文件时，就是直接使用 `libvpx/webmenc.cc` 中的函数。  
然后它在解码时，使用的是 libvpx 中的函数。  
 ---
# 使用 libwebm

### 大体思路
* 把 libwebm.a 提前编译出来
* 在自己的 Android 项目中使 libwebm.a，但不是直接调用库里面的函数，而是通过 webmdec.cc 间接地使用
* 所以要把 webmdec.cc 放到自己的 Android 项目里。
* 仿照 vpxdec.c，调用一下 webmdec.cc 里的东西，如果成功了，则说明使用成功。

### 详细步骤  

* 先把 libwebm 编成库文件，参考另一篇笔记《编译Android上的静态库》里面有详细的步骤可以得到 `libwebm.a`
* 新建Android项目，添加 Native Support，自己的库起名 mydecode。把 `libwebm.a` 和 `webmdec.cc` 拷贝到jni目录  
* 把 libvpx 下所有的目录和文件（只要头文件即可）拷贝到jni目录下
* MainActivity 里添加 native 方法，让 native 方法跑在单独的线程里不要阻塞UI线程以免引起不必要的麻烦。
* 该 native 方法的实现如下(完全仿照 vpxdec.c 写的最简单的调用 libwebm 的示例)： 

		/*  mydecode.c */
		#include <jni.h>
		#include "webmdec.h"
		
		struct VpxDecInputContext {
			  struct VpxInputContext *vpx_input_ctx;
			  struct WebmInputContext *webm_ctx;
		};
		
		JNIEXPORT jint JNICALL Java_com_real_useprebuildstlibvpx_MainActivity_mydecode
		  (JNIEnv *env, jobject obj)
		{
			struct VpxDecInputContext input = {NULL, NULL};
			struct VpxInputContext vpx_input_ctx;
			struct WebmInputContext webm_ctx;
			memset(&(webm_ctx), 0, sizeof(webm_ctx));
			input.webm_ctx = &webm_ctx;
			input.vpx_input_ctx = &vpx_input_ctx;
			FILE	*infile;
			infile = fopen("/sdcard/test.webm", "rb");
			input.vpx_input_ctx->file = infile;
			file_is_webm(input.webm_ctx, input.vpx_input_ctx);
			if (webm_guess_framerate(input.webm_ctx, input.vpx_input_ctx))
				return 0; // guess frame rate failed
			else return 1; // guess frame rate success
		
		}
* Android.mk 如下：

		LOCAL_PATH := $(call my-dir)
		
		include $(CLEAR_VARS)
		LOCAL_MODULE    := pre-st-libwebm
		LOCAL_SRC_FILES := libwebm.a
		LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
		include $(PREBUILT_STATIC_LIBRARY)
		
		include $(CLEAR_VARS)
		LOCAL_MODULE    := mydecode
		LOCAL_SRC_FILES := mydecode.c
		LOCAL_SRC_FILES += webmdec.cpp
		LOCAL_STATIC_LIBRARIES := pre-st-libwebm
		include $(BUILD_SHARED_LIBRARY)
* Application.mk 如下：
		
		APP_ABI := armeabi-v7a
		APP_STL := stlport_static
 ---
# 使用 libvpx  

### 大体思路
* 提前编译好 libvpx.a
* 通过 vpxdec.c 来使用该库