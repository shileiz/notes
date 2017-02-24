### 目标：在 Android 命令行上运行纯 c 的可执行程序

### 1. 准备 NDK
* 下载 NDK，把 ndk-build 加入 path

### 2. 准备 hello.c 源文件
* 写一个 hello world 的 c 程序

### 3. 准备 Android.mk
* Android.mk 内容如下：

		LOCAL_PATH := $(call my-dir)  
		include $(CLEAR_VARS)  
		LOCAL_MODULE := hello 
		LOCAL_SRC_FILES := hello.c 
		LOCAL_CFLAGS += -pie -fPIE
		LOCAL_LDFLAGS += -pie -fPIE 
		include $(BUILD_EXECUTABLE)

* 其中如下两行是 Android L 之后必须加的：

		LOCAL_CFLAGS += -pie -fPIE
		LOCAL_LDFLAGS += -pie -fPIE 

* 如果不加上面两行，会报错：only position independent executables (PIE) are supported.

### 4. 编译
* 目录结构： 建一个叫 jni 的文件夹，把 hello.c 和 Android.mk 都放在里面
* 在 jni 外面运行 ndk-build
* 编译成功后会生成 libs 目录，跟 jni 目录同级，libs 里面的 armeabi 里，就是 hello 这个可执行文件了。

### 5. 运行
* 把 hello 推到手机的 `/data/local/` 里，注意必须是这个目录，直接推 sdcard 是无法运行的
* adb shell ---> chmod 777 hello 
* 运行 hello，成功