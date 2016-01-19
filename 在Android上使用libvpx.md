libvpx 不包括webm文件的解析，只负责编解码。  
解析webm需要用到 libwebm。  
libwebm 在源码目录的：`libvpx/third_party/libwebm`  
libwebm 可以单独编译成一个库。  
libvpx 使用了库 libwebm（可选，可以在config的时候打开或者关闭webm-io）。  
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
* 新建Android项目，添加 Native Support，自己的库起名 mydecode。自动生成的 `mydecode.cpp` 改为 `mydecode.c`
* MainActivity 里添加 native 方法，让 native 方法跑在单独的线程里不要阻塞UI线程以免引起不必要的麻烦。  
修改后的 MainActivity.java 如下：  

		public class MainActivity extends Activity {
		
			static {System.loadLibrary("mydecode");}
			
			Handler	handler=null;
			
			public native int mydecode();
			
			@SuppressLint("HandlerLeak")
			@Override
			protected void onCreate(Bundle savedInstanceState) {
				super.onCreate(savedInstanceState);
				setContentView(R.layout.activity_main);
				handler = new Handler(){
					public void handleMessage(Message msg) {
						Toast.makeText(MainActivity.this, "return code: "+msg.arg1, Toast.LENGTH_SHORT).show();
					}
				};
			}
			
			public void click(View v){
				new Thread(){
					public void run() {
						int r = mydecode();
						Message msg = new Message();
						msg.arg1 = r;
						handler.sendMessage(msg);
					}	
				}.start();
			}
		}
* 把 `libwebm.a` 和 `webmdec.cc` 拷贝到jni目录，修改Android.mk文件，把这两个东西添加进去。  
修改后的Android.mk:

		LOCAL_PATH := $(call my-dir)
		
		include $(CLEAR_VARS)
		LOCAL_MODULE    := pre-st-libwebm
		LOCAL_SRC_FILES := libwebm.a
		include $(PREBUILT_STATIC_LIBRARY)
		
		include $(CLEAR_VARS)
		LOCAL_MODULE    := mydecode
		LOCAL_SRC_FILES := mydecode.c
		LOCAL_SRC_FILES += webmdec.cc
		LOCAL_STATIC_LIBRARIES := pre-st-libwebm
		include $(BUILD_SHARED_LIBRARY)
  
* 把以下头文件拷贝到jni目录下（保持原有目录结构，目录层级：jni相当于libvpx）：

		libvpx目录下的所有.h
		执行config时生成的vpx_config.h（见编译libvpx部分）
		libvpx/vpx 目录下的所有头文件
		libvpx/vpx_ports 目录下的所有头文件
		libvpx/third_party/libwebm 目录下的所有头文件
* native 方法的实现如下(完全仿照 vpxdec.c 写的最简单的调用 libwebm 的示例)： 

		/*  mydecode.c */
		#include <jni.h>
		#include "webmdec.h"
		
		JNIEXPORT jint JNICALL Java_com_zsl_useprebuiltstlibwebm_MainActivity_mydecode
		  (JNIEnv *env, jobject obj)
		{
		    struct VpxInputContext vpx_input_ctx;
		    struct WebmInputContext webm_ctx;
		    memset(&(webm_ctx), 0, sizeof(webm_ctx));
		    FILE    *infile;
		    infile = fopen("/sdcard/test.webm", "rb");
		    vpx_input_ctx.file = infile;
		    file_is_webm(&webm_ctx, &vpx_input_ctx);
		    if (webm_guess_framerate(&webm_ctx, &vpx_input_ctx))
		        return 0; // guess frame rate failed
		    else return 1; // guess frame rate success
		
		}


* Application.mk 如下：
		
		APP_ABI := armeabi-v7a
		APP_STL := stlport_static
 ---
# 使用 libvpx  

### 大体思路
* 提前编译好 libvpx.a， libwebm.a
* 通过 vpxdec.c 来使用该库(而 vpxdec.c 用到了同目录下的其他 .c/.cc 等文件，所以要都加入 Android 项目)

### 详细步骤  
* 编译 libvpx.a，参考另一篇笔记《编译Android上的静态库》里面有详细的步骤可以得到 `libvpx.a`
* 新建Android项目，添加 Native Support，自己的库起名 mydecode。自动生成的 `mydecode.cpp` 改为 `mydecode.c`
* MainActivity 里添加 native 方法，让 native 方法跑在单独的线程里不要阻塞UI线程以免引起不必要的麻烦。修改后的MainActivity跟上文一样。  
* 把 `libwebm.a`， `libvpx.a` 和 libvpx 目录下的所有源文件以及子目录(其实可以有选择性的拷贝的)拷贝到jni目录。把config libvpx时生成`vpx_config.h`拷贝到jni目录。
* 修改Android.mk文件，添加 `libwebm.a`， `libvpx.a`， `vpxdec.c` 以及 vpxdec.c 用到的，没有编译到 libvpx.a 里的其他 .c 文件。  
修改后的 Android.mk 如下：

		LOCAL_PATH := $(call my-dir)
		
		include $(CLEAR_VARS)
		LOCAL_MODULE    := pre-st-libwebm
		LOCAL_SRC_FILES := libwebm.a
		include $(PREBUILT_STATIC_LIBRARY)
		
		include $(CLEAR_VARS)
		LOCAL_MODULE    := pre-st-libvpx
		LOCAL_SRC_FILES := libvpx.a
		include $(PREBUILT_STATIC_LIBRARY)
		
		include $(CLEAR_VARS)
		LOCAL_MODULE    := mydecode
		LOCAL_SRC_FILES := mydecode.c
		LOCAL_SRC_FILES += vpxdec.c
		LOCAL_SRC_FILES += args.c
		LOCAL_SRC_FILES += ivfdec.c
		LOCAL_SRC_FILES += md5_utils.c
		LOCAL_SRC_FILES += rate_hist.c
		LOCAL_SRC_FILES += tools_common.c
		LOCAL_SRC_FILES += video_reader.c
		LOCAL_SRC_FILES += webmdec.cc
		LOCAL_SRC_FILES += y4menc.c
		LOCAL_SRC_FILES += y4minput.c
		#libyuv_src_files := $(wildcard $(LOCAL_PATH)/third_party/libyuv/source/*.cc)
		#libyuv_src_files := $(libyuv_src_files:$(LOCAL_PATH)/%=%)
		#LOCAL_SRC_FILES += $(libyuv_src_files)
		LOCAL_STATIC_LIBRARIES := pre-st-libvpx pre-st-libwebm
		include $(BUILD_SHARED_LIBRARY)

* 说明，vpxdec.c 跟 libyuv 有关系，直接 Build Android 项目时会报错。关于 libyuv 有以下两种解决方案：
	* 方案一: 把libyuv的源文件放到Android项目里编译一遍。（就是以上Android.mk注释掉的部分）。比较麻烦，但也实验成功了。需要做以下工作：
		* 在Android.mk加上libyuv的源文件（即打开Android.mk注释掉的部分）
		* 修改一下libyuv的目录结构，把原来在 include 里的 libyuv 挪到 libyuv/source 里。挪完了之后的目录结构是这样：  
			`jni/third_party/libyuv/source`  
			source里面是.cc源文件和文件夹libyuv，文件夹libyuv里是头文件。
		* 修改libyuv的所有头文件，把 `#include "libyuv/xxx.h"` 全部改为 `#include "xxx.h"`
	* 方案二：把 CONFIG_LIBYUV 给干掉，有两种方式：
		* 手动把 `vpx_config.h` 里面的 `CONFIG_LIBYUV` 由1改为0。
		* 或者在编译 libvpx 的时候加上参数 `--disable-libyuv`，这样生成的 `vpx_config.h` 里面， `CONFIG_LIBYUV` 就是 0 了。
	* 说明： 在编译 libvpx 的时候，虽然有一个开关 `--eanble-libyuv/--disable-libyuv` ，但即便打开了，也不会把 libyuv编进libvpx.a里面。这个开关只影响生成的 `vpx_config.h` 里面的 `CONFIG_LIBYUV` 是0 还是1
* native 方法直接调用 vpxdec.c 里的main函数进行解码： 

		#include <jni.h>
		JNIEXPORT jint JNICALL Java_com_zsl_useprebuiltstlibvpx2_MainActivity_mydecode
		  (JNIEnv * env, jobject obj)
		{
			char* argv[] = {"nothing","--i420","-o","/sdcard/test.yuv","/sdcard/test.webm"};
			int r = main(sizeof(argv)/sizeof(char*), argv);
			return r;
		}

* 把解码生成的yuv写到sdcard上，必须在 Manifest 里添加权限：

	    <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE"/>
		<uses-permission android:name="android.permission.MOUNT_UNMOUNT_FILESYSTEMS"/>
