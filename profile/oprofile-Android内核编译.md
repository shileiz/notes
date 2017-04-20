## 关于 oprofile

####关于正常的 Oprofile（Linux 下的）：
* 参考： [http://www.cnblogs.com/bangerlee/archive/2012/08/30/2659435.html](http://www.cnblogs.com/bangerlee/archive/2012/08/30/2659435.html)
* 使用方法就是在命令行运行一些命令，比如：

		opcontrol --start
		opcontrol --reset 
		等等...

####关于 Android 里的 oprofile：
1. 最早的 Android 里是没有 oprofile 的，想用的话需要自己安装。
2. 后来的 Android 里加入了 oprofile，在源码的 external 目录里有 oprofile 文件夹。并且在 5.0 之前，userdebug 版本中是默认开启的。
3. 到了 Android 5.1，userdebug 版本默认不是开启的了
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
* 在子文件夹中存在总共12个Android.mk.bak文件，将其改为Android.mk。
* 重新编译 Android 源码，make -j16
* 遇到报错如下：

		external/oprofile/opcontrol/opcontrol.cpp:141:21: error: 'MAX_EVENTS' was not declared in this scope
		 int selected_events[MAX_EVENTS];
		                     ^
		external/oprofile/opcontrol/opcontrol.cpp:142:21: error: 'MAX_EVENTS' was not declared in this scope
		 int selected_counts[MAX_EVENTS];
		                     ^
		external/oprofile/opcontrol/opcontrol.cpp: In function 'int setup_device()':
		external/oprofile/opcontrol/opcontrol.cpp:255:18: error: 'MAX_EVENTS' was not declared in this scope
		     max_events = MAX_EVENTS;
		                  ^
		external/oprofile/opcontrol/opcontrol.cpp: In function 'int process_event(const char*)':
		external/oprofile/opcontrol/opcontrol.cpp:405:21: error: 'min_count' was not declared in this scope
		         count_val = min_count[0];
		                     ^
		external/oprofile/libpopt/popt.c: In function 'poptAddAlias':
		external/oprofile/opcontrol/opcontrol.cpp:410:5: error: 'selected_events' was not declared in this scope
		     selected_events[num_events] = event_idx;
		     ^
		external/oprofile/libpopt/popt.c:1107:20: warning: unused parameter 'flags' [-Wunused-parameter]
		   /*@unused@*/ int flags)
		                    ^
		external/oprofile/opcontrol/opcontrol.cpp:411:5: error: 'selected_counts' was not declared in this scope
		     selected_counts[num_events++] = count_val;
		     ^
		external/oprofile/opcontrol/opcontrol.cpp: In function 'int main(int, char* const*)':
		external/oprofile/opcontrol/opcontrol.cpp:583:35: error: 'MAX_EVENTS' was not declared in this scope
		                 if (num_events == MAX_EVENTS) {
		                                   ^
		external/oprofile/opcontrol/opcontrol.cpp:644:23: error: 'default_event' was not declared in this scope
		         process_event(default_event);
		                       ^
		external/oprofile/opcontrol/opcontrol.cpp:716:29: error: 'selected_events' was not declared in this scope
		             int event_idx = selected_events[i];
		                             ^
		external/oprofile/opcontrol/opcontrol.cpp:733:22: error: 'selected_counts' was not declared in this scope
		                      selected_counts[i],

* 解决方法： `vi external/oprofile/opcontrol/Android.mk`,加一行：`LOCAL_CFLAGS += -D__arm__`
* 编译成功后刷机，刷机成功后可以在 adb shell 里运行 oprofile 相关命令行，不过遇到以下问题：

		adb root
		adb remount
		adb shell
		127|root@flounder:/ # opcontrol                                                
		mount: No such device

* 报了个奇奇怪怪的 mount：No such device，怀疑是因为内核没有开启 oprofile，开启内核的 oprofile 后再试。


### Android 源码与内核源码
* 正常从 google 下载的源码是不包含内核源码的
* 内核以编译好的 image 方式提供
* 比如，`/home/disk2/androidN/device/htc/flounder-kernel` 里就放着 flounder 这个设备的内核 image：Image.gz-dtb
* 当编译源码生成各种 image 的时候（make -j 16）,内核 image 是被写入 boot.img 的

#### 定制内核（需要下载内核源码并重新编译）

#####编译内核的官方文档：
* 官方文档： [http://source.android.com/source/building-kernels.html](http://source.android.com/source/building-kernels.html)

#####编译内核的步骤：

#####1. 下载源码
1. 地址：[https://android.googlesource.com/kernel/](https://android.googlesource.com/kernel/)
2. Nexus9 的 CPU 是 tegra，所以点击左侧的 tegrea
3. 此时复制该页面的 `git clone https://android.googlesource.com/kernel/tegra`，把源码 clone 到 Ubuntu 上。然后根据 `git branch -a` 的输出，checkout 合适的版本，比如： `git checkout remotes/origin/android-tegra-flounder-3.10-marshmallow-mr2`

#####2. 编译
5. 设置环境变量：

		export ARCH=arm64
		export CROSS_COMPILE=aarch64-linux-android-
		//注意，如果不是64位的CPU则这里的设置不一样。

6. 去 Android 源码目录进行一下 `source build/envsetup.sh`，目的是把 `aarch64-linux-android-gcc` 等编译工具所在目录加入 PATH
7. 回到内核目录，执行 `make flounder_defconfig`  （`make help` 能看到所有芯片的 `xxx_defconfig`）
	* 注意1：如果没有设置环境变量则这步会失败。
	* 注意2：如果环境变量设置成了32位CPU的，则 make help 出来的可选 Architecture 全是 32位 CPU 的，并没有 `flounder_defconfig`
	* 注意3：make 后面那个 `flounder_defconfig` 是怎么来的？
	* 从这里可以查询设备和内核的对应关系：[http://source.android.com/source/building-kernels.html#figuring-out-which-kernel-to-build](http://source.android.com/source/building-kernels.html#figuring-out-which-kernel-to-build)

8. 然后执行 `make menuconfig` 跳出配置界面进行配置。
	* 这步遇到了问题，无法弹出配置界面，报了一大堆错误。解决方法：
	
			sudo apt-get install libncursesw5-dev

9. 在界面上开启如下2个选项：

		General setup->Profiling support
		General setup->OProfile system profiling

* 遇到问题，flounder 无法配置 `General setup->OProfile system profiling`，在 menuconfig 里搜索 oprofile 后发现（跟 vi 的搜索一样，按‘/’）：
 	
		Symbol: OPROFILE [=n]
		Symbol: OPROFILE_NMI_TIMER [=n]
		Symbol: HAVE_OPROFILE [=n]

* 根据结果里的 depends on，可以推断出，如下三个为最根本的未开启选项：

		HAVE_OPROFILE [=n]
		RING_BUFFER_ALLOW_SWAP [=n]
		HAVE_PERF_EVENTS_NMI [=n]  

* 如搜索结果会告诉你这些配置项是在哪个配置文件的那一行定义的，比如 HAVE_OPROFILE： Defined at arch/Kconfig

* 尝试暴力修改（一下行号和文件名都是从 menuconfig 的搜索结果查到的）：
	* `vi arch/Kconfig`，把第 30 行的 bool 改为： `def_bool y`(修改 `HAVE_OPROFILE` 为 y)
	* `vi arch/Kconfig`，把第 263 行的 bool 改为： `def_bool y`(修改 `HAVE_PERF_EVENTS_NMI` 为 y)
	* `vi kernel/trace/Kconfig`， 把第 91 行的 bool 改为： `def_bool y`(修改 `RING_BUFFER_ALLOW_SWAP` 为 y) 
* 以上文件修改好以后，再进 menuconfig 就有  `General setup->OProfile system profiling` 了， 开启之。
* 结果开启之后编译无法通过（也是预料之中，人家本来不支持，你非得暴力开启肯定不成啊），所以此处只能回退。

10. 编译：`make -j4`，遇到问题如下:

		drivers/net/wireless/bcmdhd/dhd_linux.c:3240:47: error: 'MAX_RT_PRIO' undeclared (first use in this function)
		   param.sched_priority = (dhd_watchdog_prio < MAX_RT_PRIO)?
		                                               ^
		drivers/net/wireless/bcmdhd/dhd_linux.c:3240:47: note: each undeclared identifier is reported only once for each function it appears in
		drivers/net/wireless/bcmdhd/dhd_linux.c: In function 'dhd_dpc_thread':
		drivers/net/wireless/bcmdhd/dhd_linux.c:3357:42: error: 'MAX_RT_PRIO' undeclared (first use in this function)
		   param.sched_priority = (dhd_dpc_prio < MAX_RT_PRIO)?dhd_dpc_prio:(MAX_RT_PRIO-1);
		                                          ^
		drivers/net/wireless/bcmdhd/dhd_linux.c: In function 'dhd_rxf_thread':
		drivers/net/wireless/bcmdhd/dhd_linux.c:3418:42: error: 'MAX_RT_PRIO' undeclared (first use in this function)
		   param.sched_priority = (dhd_rxf_prio < MAX_RT_PRIO)?dhd_rxf_prio:(MAX_RT_PRIO-1);
		                                          ^
		drivers/net/wireless/bcmdhd/dhd_linux.c:3422:2: error: implicit declaration of function 'daemonize' [-Werror=implicit-function-declaration]
		  DAEMONIZE("dhd_rxf");
		  ^
		drivers/net/wireless/bcmdhd/dhd_linux.c: At top level:
		drivers/net/wireless/bcmdhd/dhd_linux.c:4573:2: error: unknown field 'ndo_set_multicast_list' specified in initializer
		  .ndo_set_multicast_list = dhd_set_multicast_list,

	* 没找到好方法解决，进 menuconfig( `make menuconfig` ) 里把 Device Drivers ---> Network device support 给关了
	* 重新 `make -j4` 编译成功

#####3. 拷贝内核，生成 boot.img, system.img 重新刷机
* 编出来的内核 image 是 `arch/arm64/boot/Image.gz-dtb`，把它拷贝到 Android 源码对应目录：
* `cp /home/disk2/android/tegra/arch/arm64/boot/Image.gz-dtb /home/disk2/android/android-5.1.1_r2.BAK/device/htc/flounder-kernel/`
* 重新编译源码并且刷机


#####查看Android源码的版本：
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