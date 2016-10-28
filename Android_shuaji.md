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
