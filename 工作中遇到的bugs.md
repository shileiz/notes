### dlopen failed
* 用提前编译好的 ffmpeg 库做实验，遇到这个错误
* dlopen("/data/app-lib/com.zsl.useprebuiltffmpeg-2/libmydecode.so") failed: dlopen failed: 
* 自己的jni层的库可以编译出来，运行后立刻崩溃，报以上错误
* 原因是：编译ffmpeg库的时候（在另外一台Linux上进行的提前编译），用的 platform 是 Android-L（即Android5.0）
* 据网上说，Android5.0 的 Libc 和 Android4.3是不一样的，所以导致了这个问题
* 解决方法：换一个 Android5.0的设备进行测试，或者编译ffmpeg的时候，用Android-21之类的不要那么新的platform

### jni层读写内存卡
* 错误： `A/libc(3281): Fatal signal 11 (SIGSEGV), code 1, fault addr 0x30 in tid 3300 (Thread-119)`
* 原因： Manifest 里没有加权限
* 解决方法：加如下权限：   
* 
		<uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE"/>
		<uses-permission android:name="android.permission.MOUNT_UNMOUNT_FILESYSTEMS"/>