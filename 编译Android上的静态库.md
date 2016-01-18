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
* 因为libvpx的Android.mk用了很多相对路径，为了跟那些路径对应上，所以必须在libvpx这个文件夹的同级目录下，进行编译：
	* `cd same_path_as_libvpx`
* libvpx需要先config才能编译，config命令如下： 
	* `./libvpx/configure --target=armv7-android-gcc --disable-runtime-cpu-detect --disable-realtime-only --disable-examples --disable-docs --sdk-path=<NDK_root_path>`
	* 说明：`--disable-runtime-cpu-detect` 是为默认打开NEON优化，而不是在运行时去detect。因为libvpx默认是在运行时去检测cpu，如果支持NEON则打开，否则不打开。当我们disable了这种检测时，默认打开NEON优化。
	* 说明：`--disable-realtime-only`， 因为在目标平台是Android的时候，默认是开启 realtime-only 的，所以加上这个选项，以关闭 realtime-only。
* 自己写一个Android.mk，内容如下：

		LOCAL_PATH := $(call my-dir)
		include $(CLEAR_VARS)
		LOCAL_STATIC_LIBRARIES := libwebm
		include $(LOCAL_PATH)/libvpx/build/make/Android.mk 
* 说明：之所以要自己写一个mk，再在其中引用 `libvpx/build/make/Android.mk` 是因为这是 `libvpx/build/make/Android.mk` 的注释里写的。
* 直接使用 libvpx/build/make/Android.mk 是无法编译的，因为连 LOCAL_PATH 都没定义。它这种用法也是为了方便移植安装。
* 用NDK以及libwenbm自带的Android.mk编译libvpx：
	*  `NDK_PROJECT_PATH=. ndk-build APP_BUILD_SCRIPT=Android.mk APP_ABI=armeabi-v7a APP_STL=gnustl_static` 
* 把生成的libvpx.a拷贝出来：` cp obj/local/armeabi-v7a/libvpx.a ./  `
