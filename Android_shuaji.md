###编译Android源码基本流程：
* `source build/envsetup.sh`
* `lunch`，选择对应的平台，我用 Nexus9,选择 `17. aosp_flounder-userdebug`，直接输入 17 回车
* `make systemimage -j 16` 参数 -j 16 是使用16个线程编译，根据机器性能自行调节。
* 编译完成后，出来的东西会在：`out/target/product/flounder`

###编译Android源码遇到的问题：
1. 报了个 gperf 没有的错，用 apt-get 装了一下就好了
2. 说 java 版本不对，编 5.1.1 的源码要求是 java 1.7，而我的是 1.8，百度一下安装一下 open jdk1.7 即可
3. 运行了 make clean 之后出了问题：因为在源码根目录运行了 make clean，所以删掉了很多东西。导致再次 make 的时候说找不到某个目标文件。百度了一下，根据结果去 framework/base 目录里运行了一下 mmm
4. 运行 mmm 时又说 libz.so.1 找不到，又百度了一下，`sudo apt-get install lib32z1` 搞定。

###刷机脚本：

	sudo adb reboot bootloader
	sudo fastboot flash recovery recovery.img
	sudo fastboot flash boot boot.img
	sudo fastboot flash system system.img
	#sudo ~/bin/fastboot flash vendor vendor.img 
	sudo fastboot flash cache cache.img
	sudo fastboot -w
	sudo fastboot reboot

###问题现象
* 无法启动的手机，可以adb, 可以logcat，所以应该是进了 Android 系统了，不过系统 UI 起不来。
* 从logcat看, 应该是 data 分区没有挂载引起的系统起不来。

###问题分析
* 其实根本问题就是开机后 /data 分区挂载不上
* 因为系统第一次启动的时候，需要把一些自带 app 解压到 /data 分区里，如果你没有这个分区，那肯定启动不起来
* 网上查了一些手动挂载分区的帖子，还下载了 busybox 准备手动挂载（busybox 里又 fdisk 等工具）
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

### 关于 oprofile

####关于正常的 Oprofile（Linux 下的）：
* 参考： [http://www.cnblogs.com/bangerlee/archive/2012/08/30/2659435.html](http://www.cnblogs.com/bangerlee/archive/2012/08/30/2659435.html)
* 使用方法就是在命令行运行一些命令，比如：

		opcontrol --start
		opcontrol --reset 
		等等...

####关于 Android 里的 oprofile：
1. 最早的 Android 里是没有 oprofile 的，想用的话需要自己安装。
2. 后来的 Android 里加入了 oprofile，在源码的 external 目录里有 oprofile 文件夹。并且在 5.0 之前，userdebug 版本中是默认开启的。
3. 到了 Android 5.1，把 userdebug 版本默认也不是开启的了
4. 到了 Android 6.0，干脆又把 oprofile 从 Android 源码里拿掉了

####关于 oprofile 和 kernel:
* oprofile 不光是一套软件，它的运行需要 kernel 的支持
* 无论是 Ubuntu 还是 Android，他们想使用 oprofile 都需要他们的内核开启了相关选项
* 如何开启？在编译内核时开启。


#### 安装 oprofile

* 参考：	[http://blog.csdn.net/lqc1992/article/details/48948325](http://blog.csdn.net/lqc1992/article/details/48948325)

* 基于 Android-5.1.1-r2 源码（external 里自带 oprofile）

##### 打开源码里的 oprofile：

* 修改Android.mk文件，打开后发现最后一句被注释，将最后一句前边的#号删掉即可
* 在子文件夹中存在总共12个Android.mk.bak文件，将其改为Android.mk，开启编译。


#### Android 源码与内核源码
* 正常从 google 下载的源码是不包含内核源码的
* 内核以编译好的 image 方式提供
* 比如，`/home/disk2/androidN/device/htc/flounder-kernel` 里就放着 flounder 这个设备的内核 image：Image.gz-dtb
* 当编译源码生成各种 image 的时候（make -j 16）,内核 image 是被写入 boot.img 的

想定制内核则需要下载内核源码并重新编译
1. 去这里找到 Nexus9 对应的 cpu：https://android.googlesource.com/kernel/
2. Nexus9 使用的是 tegra，所以点击左侧的 tegrea
3. 此时可以选择复制该页面的 git clone https://android.googlesource.com/kernel/tegra，把源码 clone 到 Ubuntu 上
   也可以在页面里找到跟 Android 源码版本对应的 branch，手动下载压缩包：tegra-android-tegra-flounder-3.10-marshmallow.tar.gz
4. 
git clone https://android.googlesource.com/device/htc/flounder-kernel
cd flounder-kernel
git branch -a
git checkout remotes/origin/marshmallow-release

查看Android源码的版本：
方式一，如果你编译过源码：
查看 ./out/target/product/flounder/system/build.prop
方式二，如果没编译过源码：
查看 build/core/version_defaults.mk
以上方法看到都是形如：6.0.1 这种版本号
而内核 git branch -a 出来的都是文字代号，形如：marshmallow-mr3-release
这两种版本号怎么对应起来，这里有张对应表：
https://developer.android.com/guide/topics/manifest/uses-sdk-element.html
不过表也不全，比如 Platform version 6.0 对应 version code 是 M（即 marshmallow），但 6.0.1 就没说
我们就按照 marshmallow-release 来吧

cd ../androidN
source build/envsetup.sh
lunch
17

cd ../flounder-kernel/



