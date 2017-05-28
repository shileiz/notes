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
* 运行 hello：  
```
adb shell /data/local/hello
```

### 6. 没有 root 的手机
* 没有 root 过的手机无法把 hello 推到  `/data/local/` 里， 有以下几种解决方案：

#### 方法一
* 尝试 `/data/local/tmp/`，这个目录非 root 用户也有权限访问，push 到这里，然后从这里运行：  
```
adb shell /data/local/tmp/hello
```

#### 方法二
* 如果还是不行，可以用 adb shell run-as ，这种方案需要做一个 apk 装到手机上，然后 run-as 的时候用 apk 的包名，比如：
```
adb shell run-as com.zsl.myapp /data/app/com.zsl.myapp-1/lib/arm/libhello.so
```
* 做的这个 apk 可以没有任何 Activity，不过需要 jni，把我们编译出的 hello 改名叫 libhello.so 放到工程的 jinLibs（AndroidStudio）目录里，这样做出来的 apk 就可以了。
* 安装这个 apk 后，libhello.so 会被放在 `/data/app/com.zsl.myapp-1/lib/arm/libhello.so`，我们可以按照上述方法运行。

#### 方法三
* 方案二的改良版，不需要把 hello 改名叫 libhello.so，也不需要把它弄到 jni 目录并打包到 apk 里。
* 我们只需要做一个空的 apk，包名是 com.zsl.myapp，给 apk 访问 sdcard 的权限
* 然后我们把 hello 推到 sdcard，然后这么搞：  

		```
		adb push hello /sdcard/
		adb shell run-as com.zsl.myapp cp /sdcard/hello .  
		adb shell run-as com.zsl.myapp chmod 755 ./hello  
		adb shell run-as com.zsl.myapp ./hello  
		```