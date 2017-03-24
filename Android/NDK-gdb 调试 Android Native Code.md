#### ndk-gdb
* Android 的 native 程序本质上是运行在 Linux 上的 C/C++ 程序，所以用 gdb 调试。
* Google 在 NDK 套件里提供了一个 gdb，只不过不是很好用。（起码到目前为止，2017年3月23，NDK-r13b，Windows 版）
* Google 提供的这个 ndk-gdb，说白是在 gdb 上做了一层包装（用 python），目的是让它用起来更简单。
* 如果不看 Google 的这层包装，其实 ndk-gdb 的本质是：
	* 运行在 host 上的 gdb：host 可以是 Windows 也可以是 Linux。对于 Windows 来说，它就是 google 随 NDK 一起提供的 gdb.exe。
	* 运行在 Android 设备上的 gdbserver，用于跟 host 通信。gdbserver 是一个 google 随 NDK 提供的可以在 Android 上执行的命令行程序。
	* 运行在 Android 设备上的被 debug 的进程，大多数情况下即你做的apk
* 如果不适用 Google 提供的这层包装，我们可以手动用 gdb 调试（以 NDK r13b 为例）：
	* 编译你工程的 native 部分时，加上 NDKDEBUG=1 选项： 
		* `ndk-build NDKDEBUG=1`
	* 然后把你的工程打包成 apk，安装到手机。假设你的apk包名叫做 `com.yourpackage.xxx`
	* 启动该 app，并查询出进程号，以备后续启动 gdbserver 使用：
		* `adb shell ps | findstr "com.yourpackage"`，假设查到的 PID 为：6844，后面要用
	* 把 gdbserver push 到手机上，gdbserver 在这里： `prebuilt/android-arm64/gdbserver/gdbserver`： 
		* `adb push NDK-ROOT/prebuilt/android-arm64/gdbserver/gdbserver /data/data/com.yourpackage.xxx/gdbserver`
		* `adb shell chmod 777 /data/data/com.yourpakage.xxx/gdbserver`
	* 把手机上的某个 tcp 端口映射到一个本地文件，为启动 gdbserver / gdb 做准备：
		* `adb forward tcp:5039 localfilesystem:/data/data/com.yourpakage.xxx/debug`
		* 注意端口号可以自选，需要记住这个端口，后续启动 gdb 时就连接这个端口
		* 映射到的文件也可以自定义，不过一定要放在 `/data/data/com.yourpakage.xxx` 里面，以免有读写权限问题
	* 启动 gdbserver:
		* `adb shell run-as com.yourpackage.xxx /data/data/com.yourpackage.xxx/gdbserver +/data/data/com.yourpakage.xxx/debug --attach 6844`
	* 注意，我们使用 com.yourpackage.xxx 用户启动 gdbserver，即 run-as。以便可以读写 `/data/data/com.yourpackage.xxx` 里的东西。
	* 注意，必须让 gdbserver 监听某个端口，这个端口就是之前我们映射的 tcp:5039
	* 注意，启动时必须让 gdbserver attach 到某个进程上，就是我们之前 ps 出来的被 debug 进程。
	* 启动 gdb， gdb.exe 在这里：`android-ndk-r13b\prebuilt\windows-x86_64\bin`：
		* `gdb.exe`,进入 gdb 命令行
		* `target remote 127.0.0.1:5039` 连接设备上的 gdbserver
	* 至此，大功告成，终于可以在电脑上 gdb 手机上的程序了。不过 gdb 连上 gdbserver 后报了一大堆警告，说找不到库或者符号之类的。
	* 如果是找不到符号，肯定是你使用了现成的的库，而不是你从源码编译出来的，所以这些库在编译的时候没有加 debug 选项，自然找不到符号。
	* 如果是找不到库，可能是环境变量没有设。
* Google 为了大家使用方便，把以上步骤合并入了一个 python 程序，ndk-gdb.py，并且为该程序做了很多健壮性的工作，所以这个 python 程序也不是很小（包含了 gdbrunner 模块，adb 模块 这两个 google 自己写的模块）
* 理想情况下，你只需要跑到你的项目根目录下（即含有 AndroidManifest.xml 文件和 jni 文件夹的那个目录），运行一下 ndk-gdb 就万事大吉了。
* 但现实情况是有些坑。

#### 怎么定位 ndk-gdb 里的坑
* r10d 里的启动脚本是 ndk-gdb-py.cmd，r13b 变成了 ndk-gdb.cmd
* 运行 ndk-gdb-py.cmd/ndk-gdb.cmd 的时候，加上 --verbose 参数，能看到一些输出
* 根据这些输出去 ndk-gdb.py (r10d在NDK根目录，r13b在 `prebuilt\windows-x86_64\bin` 里) 里找到出错的点，一点一点的跟

#### Google ndk-gdb 的坑
##### r10d
* r10d，Windows x64 版本的 NDK。它提供的 gdb.exe 在 Windows 上无法运行，这是最本质错误：
	* `android-ndk-r10d\toolchains\aarch64-linux-android-4.9\prebuilt\windows-x86_64\bin\aarch64-linux-android-gdb.exe` 
	* 运行以上程序，Windows 弹框说无法启动，缺少 libpython2.7.dll。 这也正常，毕竟 google 提供的 python 脚本启动 gdb.exe 时是不会缺少这个dll的。
	* 把 libpython2.7.dll 拷贝过来再运行，libpython2.7.dll 在 `android-ndk-r10d\prebuilt\windows-x86_64\bin` 里
	* 还是运行不起来，弹框提示无法正常启动（0x000007b），所以放弃。
* 在本质错误之上，google 包的 python 脚本也有很多不妥之处，经过一系列修改，最后才发现是 gdb.exe 无法运行，让人崩溃

##### r13b
* r13b，Windows x64 版本的 NDK。google 包的 python 脚本有问题，导致 gdbserver 无法启动。修改之后可以成功使用。

#### NDK r13b 默认使用 clang 作为编译器了，而不再是 gcc
* 刚切换到 NDK r13b 的时候，可能需要修改一些代码，如果 clang 编译报错的话