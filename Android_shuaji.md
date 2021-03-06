##编译Android源码:
* 结果是生成 boot.img，system.img，cache.img 等镜像文件。boot.img 是干嘛的，后文会有介绍。
* 生成之后把这些镜像文件刷入 Android 手机即可以使用自己定制的系统了


###编译Android源码基本流程：
####1. 准备环境
* 一台 Ubuntu，下载 Android 源码，具体方法不记了
* Ubuntu 需要安装如下软件：
* `sudo apt-get install git gnupg flex bison gperf build-essential zip curl libc6-dev libncurses5-dev:i386 x11proto-core-dev libx11-dev:i386 libreadline6-dev:i386 libgl1-mesa-dri:i386 libgl1-mesa-dev g++-multilib mingw32 tofrodos python-markdown libxml2-utils xsltproc zlib1g-dev:i386 dpkg-dev`
* 安装到 `libgl1-mesa-dri:i386` 的时候报了个奇怪的错：

		下列软件包有未满足的依赖关系：
		 unity-control-center : 依赖: libcheese-gtk23 (>= 3.4.0) 但是它将不会被安装
		                        依赖: libcheese7 (>= 3.0.1) 但是它将不会被安装

* 百度到的解决方案： `sudo aptitude install libgl1-mesa-dri:i386`
* 都安装成功后： `sudo ln -s /usr/lib/i386-linux-gnu/mesa/libGL.so.1 /usr/lib/i386-linux-gnu/libGL.so`

####2. 开始编译
* 下载并 cd 到源码根目录
* `source build/envsetup.sh`
* `lunch`，选择对应的平台，我用 Nexus9,选择 `17. aosp_flounder-userdebug`，直接输入 17 回车
* `make -j16` 参数 -j16 是使用16个线程编译，根据机器性能自行调节。
* 编译完成后，出来的东西会在：`out/target/product/flounder`

####3. 刷机
* 编译成功后进行刷机，刷机脚本如下：

		:::shell
		sudo adb reboot bootloader
		sudo fastboot flash recovery recovery.img
		sudo fastboot flash boot boot.img
		sudo fastboot flash system system.img
		#sudo ~/bin/fastboot flash vendor vendor.img 
		sudo fastboot flash cache cache.img
		sudo fastboot -w
		sudo fastboot reboot

###编译Android源码遇到的问题：
* 说 java 版本不对，编 5.1.1 的源码要求是 java 1.7，而我的是 1.8，百度一下安装一下 open jdk1.7 即可
	* 另外，已经安装了多个版本的 Ubuntu 切换java版本的方法：
	* `sudo update-alternatives --config java`
	* `sudo update-alternatives --config javac`
* 在源码根目录运行了 make clean 之后出了问题：导致再次 make 的时候说找不到某个目标文件。百度了一下，根据结果去 framework/base 目录里运行了一下 `mmm .` (注意最后有个点)
* 运行 `mmm  .` 时又说 libz.so.1 找不到，又百度了一下，`sudo apt-get install lib32z1` 搞定。


### 编译 7.0.0_r1 遇到新问题： Try increasing heap size with java option '-Xmx<size>'.

* 运行 make -j16 ，进行了很久之后报错如下

		[ 70% 22471/31781] Building with Jack: out/target/common/obj/JAVA_LIBRARIES/framework_intermediates/with-local/classes.dex
		FAILED: /bin/bash out/target/common/obj/JAVA_LIBRARIES/framework_intermediates/with-local/classes.dex.rsp
		Out of memory error (version 1.2-rc4 'Carnac' (298900 f95d7bdecfceb327f9d201a1348397ed8a843843 by android-jack-team@google.com)).
		GC overhead limit exceeded.
		Try increasing heap size with java option '-Xmx<size>'.
		Warning: This may have produced partial or corrupted output.
		[ 70% 22471/31781] Building with Jack: out/target/common/obj/APPS/VolantisLayout_intermediates/with-local/classes.dex
		ninja: build stopped: subcommand failed.
		make: *** [ninja_wrapper] 错误 1

* 百度了一下，说是内存不足，需要这样：

		export JACK_SERVER_VM_ARGUMENTS="-Dfile.encoding=UTF-8 -XX:+TieredCompilation -Xmx4g"	
		./prebuilts/sdk/tools/jack-admin kill-server 
		./prebuilts/sdk/tools/jack-admin start-server

* 重新 make -j16

###m，mm，mmm的区别
* 在任意目录下运行 m ，都是编译整个源码，生成 system.img，boot.img 等
* 在某个目录下运行 mm，只编译该目录，生成相应的模块。该路径下要有 Android.mk
* mmm 后面可以跟路径，指定编译的模块，同样该路径下要有 Android.mk

### boot.img
* boot.img 包含两部分：kernel 和 ramdisk。
* kernel 就是 Linux 内核，在 boot.img 里是一个 gz 格式的压缩文件，大小 7M 左右（Android7.0）
* ramdisk 是一个最小的文件系统映像，会在进入 Android 系统之前被映射到内存里，当做文件系统（硬盘）来使用。init.rc 在 ramdisk 的根目录下。

#### 解压 boot.img ：
1. Linux 工具： `git clone https://github.com/xiaolu/mkbootimg_tools.git`
2. 运行： `./mkboot boot.img boot` ，把 boot.img 解压缩到 boot 文件夹


## Android 6.0 之后引入的问题：enable-verity/disable-verity

###问题现象
* 刷上自己编译的系统后，无法启动的手机，可以adb, 可以logcat，所以应该是进了 Android 系统了，不过系统 UI 起不来。
* 从logcat看, 应该是 data 分区没有挂载引起的系统起不来。

###问题分析
* 其实根本问题就是开机后 /data 分区挂载不上
* 因为系统第一次启动的时候，需要把一些自带 app 解压到 /data 分区里，如果你没有这个分区，那肯定启动不起来
* 网上查了一些手动挂载分区的帖子，还下载了 busybox 准备手动挂载（busybox 里有 fdisk 等工具）
* 结果为了把 busybox push 到 /system 的时候，发现不能 adb remount，所以执行了adb disable-verity
* 结果重启之后，成功进入系统了。。。。。
* 所以根本原因应该是 `adb disable-verity` 造成的

###根本原因分析
* 因为之前使用手机的时候，需要往 /system 分区推库，所以运行了 `adb disable-verity`
* 这是 Android6.0 之后的 adb 新加入的一个命令，意思是不验证 /system /cache 分区的 MD5（个人理解，实际算法应该比md5复杂和高效）
* 新刷的系统，默认是开启这个 verity 功能的，以防 /system 分区被改了导致问题
* 但我们做系统底层开发的，为了验证自己的代码，肯定是要往 /system 分区推自己的库的，所以我们就必须用 `adb disable-verity` 关闭 verity 这个功能
* 这时问题就来了
* 当我们再次刷系统的时候，新刷上的系统肯定要做 verity ，但是发现 /system 分区不太对，就报错了
* 这里还有两个小问题没有深究
	1. 新刷的系统肯定把 /system 都重写了，所以刚刷完即便做 verity，应该也能过，但为什么还出错就不知道了。
	2. 即便是 verity 的问题，为什么会导致 /data 挂不上？
* 总之，安全的方法是：

###解决方法
* 先把手机用 `adb enable-verity` 改成开启 verity 状态
* 虽然这时候 reboot 之后会导致进不了系统（毕竟 verity 会通不过么，因为我们往 /system 里推过库），但是不必理会
* 这时候刷上新系统，手机仍然进不了系统，不必理会。
* 此时是可以 adb 的。直接 `adb disable-verity`，重启
* OK 了，可以正常进系统了。

### Android分区
* 作者：Kevin
* 链接：[http://www.zhihu.com/question/20256873/answer/18421696](http://www.zhihu.com/question/20256873/answer/18421696)
* 来源：知乎
* 著作权归作者所有，转载请联系作者获得授权。

* 先说下Android的分区吧，不说清楚这个，下面都不好讲，Android一般都分这几个区:
* bootloader分区用来存储uboot，system分区存储Android，data分区用来存储用户数据，boot分区用来存储内核文件和ramdisk，recovery分区存储内核文件和recovery程序，其他还有misc和cache等等。当然各个公司会根据各自需求增加一些别的分区。
* 启动过程是先由bootloader启动uboot，然后uboot启动内核，内核会根据init.rc里的指令mount system data等这些分区，mount好以后，Android就开始启动了。
* 你应该注意到上面的分区里有两个分区都存有内核文件，就是boot和recovery，boot是正常启动流程会用到的内核文件，如果启动过程中发现有特殊按键，uboot就会读取recovery分区里的内核和ramdisk。这就是为什么你启动时按着某个组合键就会进入recovery程序。进入recovery模式以后，直接面对用户的是跑在内核上的recovery程序。这个时候除了recovery分区以外的其他分区你想写谁都可以了。recovery去读取rom文件，根据文件里的内容来更新系统，如果是img文件就直接dd到分区里，如果是文件夹就把文件里的东西copy到相应分区里(好像是这样)。更新完按正常启动过程重启机器就o了。
* 内核文件在boot.img里的，boot.img是由内核文件和ramdisk.img组成的，刷在boot分区里。你如果打开一个rom文件，如果里面如果有boot.img，哪说明刷这个rom的时候它是要更新内核的。但一般第三方定制的rom是不会去动你的内核的，只刷system。所以你在里面找不到内核文件。如果你找不到system.img，那里面应该有一个叫system的文件夹吧。是img就dd到system分区，是文件夹就copy到system分区。 

----

### 修改编译 Android 源码的默认选项：即运行 mm 时，默认被加上的编译选项
* 修改 `build/core/config.mk` 即可，mm 默认加的编译选项都在这里。
* 比如 `TARGET_ERROR_FLAGS := -Werror=return-type -Werror=non-virtual-dtor -Werror=address -Werror=sequence-point`
* 这句的意思是，把 `return-type non-virtual-dtor address sequence-point` 这些 Warning 都当成 error
* 比如我们不想把 `non-virtual-dtor` 当成 error，则可以把以上这行改成：
* `TARGET_ERROR_FLAGS := -Werror=return-type -Werror=address -Werror=sequence-point`
* 如果不想修改 Android 默认的东西，也可以不改 `build/core/config.mk`
* 只需在模块的 Android.mk 里加上相反的选项来覆盖掉默认的就可以了，比如我们在自己模块的 Android.mk 里写上：
* `LOCAL_CFLAGS += -Wno-error=non-virtual-dtor`

* 从 Android-o 开始，`build/core/config.mk` 里找不到 `TARGET_ERROR_FLAGS` 了，所以只能用第二种方法修改了，即改自己的工程的 Android.mk.