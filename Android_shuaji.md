无法启动的手机，可以adb, 可以logcat，所以应该是进了 Android 系统了，不过系统 UI 起不来。
从logcat看, 应该是 data 分区没有挂载引起的系统起不来。

插播一下网上找的文章，介绍 Android 分区的：
作者：Kevin
链接：http://www.zhihu.com/question/20256873/answer/18421696
来源：知乎
著作权归作者所有，转载请联系作者获得授权。

先说下Android的分区吧，不说清楚这个，下面都不好讲，Android一般都分这几个区:
bootloader分区用来存储uboot，system分区存储Android，data分区用来存储用户数据，boot分区用来存储内核文件和ramdisk，recovery分区存储内核文件和recovery程序，其他还有misc和cache等等。当然各个公司会根据各自需求增加一些别的分区。
启动过程是先由bootloader启动uboot，然后uboot启动内核，内核会根据init.rc里的指令mount system data等这些分区，mount好以后，Android就开始启动了。
你应该注意到上面的分区里有两个分区都存有内核文件，就是boot和recovery，boot是正常启动流程会用到的内核文件，如果启动过程中发现有特殊按键，uboot就会读取recovery分区里的内核和ramdisk。这就是为什么你启动时按着某个组合键就会进入recovery程序。进入recovery模式以后，直接面对用户的是跑在内核上的recovery程序。这个时候除了recovery分区以外的其他分区你想写谁都可以了。recovery去读取rom文件，根据文件里的内容来更新系统，如果是img文件就直接dd到分区里，如果是文件夹就把文件里的东西copy到相应分区里(好像是这样)。更新完按正常启动过程重启机器就o了。

内核文件在boot.img里的，boot.img是由内核文件和ramdisk.img组成的，刷在boot分区里。你如果打开一个rom文件，如果里面如果有boot.img，哪说明刷这个rom的时候它是要更新内核的。但一般第三方定制的rom是不会去动你的内核的，只刷system。所以你在里面找不到内核文件。如果你找不到system.img，那里面应该有一个叫system的文件夹吧。是img就dd到system分区，是文件夹就copy到system分区。 


在系统起不来的手机上（还好能进adb） ，用 df 看了一下分区，结果如下：
flounder:/ # df
Filesystem            1K-blocks   Used Available Use% Mounted on
tmpfs                    939876    420    939456   1% /dev
none                     939876     16    939860   1% /sys/fs/cgroup
tmpfs                    939876      0    939876   0% /mnt
/dev/block/dm-0         2591420 506056   2085364  20% /system
/dev/block/mmcblk0p30    253920    140    253780   1% /cache

在一台正常的手机上，用 df 看来一下分区，结果如下：
flounder:/ $ df
Filesystem            1K-blocks    Used Available Use% Mounted on
tmpfs                    939876     416    939460   1% /dev
none                     939876      16    939860   1% /sys/fs/cgroup
tmpfs                    939876       0    939876   0% /mnt
/dev/block/mmcblk0p29   2591420  507624   2067412  20% /system
/dev/block/mmcblk0p24    249856  115724    128972  48% /vendor
/dev/block/mmcblk0p30    253920     248    248432   1% /cache
/dev/block/dm-0        11585536 1493084  10008484  13% /data
/dev/fuse              11585536 1493084  10008484  13% /storage/emulated

主要区别如下：
1. 坏手机上没有 /vendor 分区
2. 坏手机上没有 /data 分区
3. 坏手机上没有 /storage/emulated 分区
4. 坏手机的 /system 挂的是 /dev/block/dm-0， 而好手机的 /system 挂的是 /dev/block/mmcblk0p29
5. /dev/block/dm-0 在好手机上是挂给了 /data，而坏手机挂给了 /system

其实根本问题就是开机后 /data 分区挂载不上
因为系统第一次启动的时候，需要把一些自带 app 解压到 /data 分区里，如果你没有这个分区，那肯定启动不起来
网上查了一些手动挂载分区的帖子，还下载了 busybox 准备手动挂载（busybox 里又 fdisk 等工具）
结果为了把 busybox push 到 /system 的时候，发现不能 adb remount，所以执行了adb disable-verity
结果重启之后，成功进入系统了。。。。。

以后再出现这种情况，直接 adb disable-verity 就好了啊！！！


目前看来，出现问题的原因可能是：
1. 在 disable-verity 的状态刷了原版系统
2. 这时候刷成修改过的系统
3. 这时候再次 disable-verity 就出问题了
