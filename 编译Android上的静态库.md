* 总体思路：用NDK编译源码，生成静态库。
* 需要用到的库有两个：libwebm、libvpx。我们用NDK编译生成这两个库即可。
* libvpx依赖于libwebm，所以先编libwebm，再编libvpx。
* 用NDK编译就需要NDK的makefile，即Android.mk。
* libwebm和libvpx都有自己的Android.mk,直接使用即可。  

---
### 详细步骤 
##### 编译 libwebm：
* 新建一个文件夹，用来接收编译生成的文件：
	* `mkdir mylibwebm`
	* `cd mylibwebm`
* 用NDK以及libwenbm自带的Android.mk编译libwebm：
	* `NDK_PROJECT_PATH=. ndk-build APP_BUILD_SCRIPT=path/to/libvpx/third_party/libwebm/Android.mk APP_ABI=armeabi-v7a APP_STL=gnustl_static` 
* 把生成的libwebm.a拷贝出来: `cp obj/local/armeabi-v7a/libwebm.a  ./ `
##### 编译libvpx：
* 因为libvpx的Android.mk（libvpx/build/make/Android.mk）用了很多相对路径，为了跟那些路径对应上，所以必须在libvpx这个文件夹的同级目录下进行config和编译：
	* `cd same_path_as_libvpx`
* libvpx需要先config才能编译，config命令如下： 

		./libvpx/configure \
		--target=armv7-android-gcc \
		--disable-runtime-cpu-detect \
		--disable-realtime-only \
		--disable-examples \
		--disable-docs \
		--sdk-path=<NDK_root_path> \
		--libc=<NDK_root_path>/platforms/<android-xx>/arch-arm
	* 说明：`--disable-runtime-cpu-detect` 是为默认打开NEON优化，而不是在运行时去detect。因为libvpx默认是在运行时去检测cpu，如果支持NEON则打开，否则不打开。当我们disable了这种检测时，默认打开NEON优化。
	* 说明：`--disable-realtime-only`， 因为在目标平台是Android的时候，默认是开启 realtime-only 的，所以加上这个选项，以关闭 realtime-only。
	* 说明：`--libc=` 对于有的ndk版本，有可能config脚本找错libc的路径，所以最好手动指定一下，以免config失败。
* config之后会生成如下东西：
	* `vpx_config.c/vpx_config.h`： 这在以后使用 libvpx 的时候会用到，比如 vpxdec.c 里就用到了。
	* `libs-armv7-android-gcc.mk`： 这在用NDK编译libvpx时会用到，因为libvpx/build/make/Android.mk里面include了这个文件。
* 自己写一个Android.mk，内容如下：

		LOCAL_PATH := $(call my-dir)
		include $(CLEAR_VARS)
		include $(LOCAL_PATH)/libvpx/build/make/Android.mk 
* 说明：之所以要自己写一个mk，再在其中引用 `libvpx/build/make/Android.mk` 是因为这是 `libvpx/build/make/Android.mk` 的注释里写的。
* 说明：直接使用 libvpx/build/make/Android.mk 是无法编译的，因为连 LOCAL_PATH 都没定义。它这种用法也是为了方便移植安装。
* 用NDK编译libvpx：
	*  `NDK_PROJECT_PATH=. ndk-build APP_BUILD_SCRIPT=Android.mk APP_ABI=armeabi-v7a APP_STL=gnustl_static` 
* 把生成的libvpx.a拷贝出来：` cp obj/local/armeabi-v7a/libvpx.a ./  `
* 编译成功后还会在当前目录生成5个头文件：`vpx_dsp_rtcd.h`， `vpx_scale_rtcd.h`， `vp9_rtcd.h`， `vp8_rtcd.h`， `vpx_version.h`
* 这5个头文件具体有什么用后续再研究。
