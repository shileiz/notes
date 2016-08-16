##CTS相关 

#####环境准备：
1、一台Linux的PC  
2、PC上装好adb和aapt  
3、PC上装好JDK（CTS5.0+安装java7，CTS4.4-安装java6）  
4、PC上下载CTS测试包，google官网就有。包括两部分：      
1) CTS包（要与被测Android版本对应）：android-cts-5.1\_r3-linux\_x86-arm.zip   
2) 媒体文件包：android-cts-media-1.1.zip  
5、保证pc可以检测到手机  

#####手机配置：  
1、恢复出厂设置  --- [是否可以不做？]                                              --- [本测试没做]  
2、语言设置为英语  
3、打开位置服务（settings>Location）--- [如果不测GPS相关，是否可以不做？]          --- [本测试是打开的]  
4、连接到一个支持IPv6的能连入Internet的wifi --- [如果不测wifi相关，是否可以不做？] --- [本测试连入的是realnetworks]  
5、保证没有滑动解锁/图案解锁/密码解锁等设置：Settings > Security > Screen lock = 'None'  
6、打开USB调试  
7、Select: Settings > Developer options > Stay Awake  
8、Select: Settings > Developer options > Allow mock locations （Android6.0以后不需要了）  
9、Launch the browser and dismiss any startup/setup screen.                        --- [本测试打开浏览器后没有任何startup/setup screen出现，什么也没做]  
10、连接PC，允许PC调试这台手机  
11、安装CTS测试需要的apk  
  
    adb install -r android-cts/repository/testcases/CtsDeviceAdmin.apk
	安装完成后在Settings > Security > device administrators 里面，仅勾选如下两个：
	android.deviceadmin.cts.CtsDeviceAdminReceiver
	android.deviceadmin.cts.CtsDeviceAdminReceiver2
12、拷贝媒体文件到手机：  

	cd 到解压的媒体文件包路径，给copy\_media.sh加可执行权限：chmod u+x copy\_media.sh  
	运行该脚本：./copy\_media.sh all  
	注意：在Ubuntu上运行这个脚本可能会报关于[:的错误，原因是ubuntu的sh默认解释器是dash，而dash对一些shell语法不支持  
	解决办法为指定为bash执行：bash copy_media.sh all  
	或者修改默认解释器。  
	方法：  
	sudo dpkg-reconfigure dash  
	选择no即可.  

#####开始跑测试：
1、连接手机到PC  
2、手机按home键回到主屏幕  
3、手机不要跑其他程序，摆放到稳定位置防止改变横竖屏，摄像头对准一个可聚焦的物体     --- [本测试没做摄像头对准物体]  
4、测试过程中不要碰手机  
5、进入到解压的CTS包，运行这个脚本以进入cts的console：./android-cts/tools/cts-tradefed  
6、用命令 run cts --package android.media --package android.mediastress 来跑CTS测试  
7、在console观察结果。在console观察结果不太方便，可以直接在shell下运行 cts-tradefed  run cts --package android.media >> xxxx.log  

#####一些说明：
* CTS可用按plan来跑，也可以按照更细分的package来跑，还可以按照更细分的class来跑  
* 其他CTS命令：
  
		list packages  查看所有可用的测试package  
		list plans     查看所有测试计划  
* CTS的PC端要使用Linux的原因：1、谷歌提供的PC端的脚本都是shell脚本。2、官方文档关于CTS的描述中，PC端都默认为Linux系统。
* 虚拟机上搞CTS：  
虚拟机环境：VMWare+Ubuntu Kylin 14.04  
宿主实体机：Window7 64位  
CTS需要ADB和AAPT，所以先下载Linux版本的 Android SDK Tools  

		Linux版的 Android SDK Tools 被坑经历：  
		下载Android SDK Tools解压后发现没有platform-tools文件夹，本以为adb就在里面了。
		原因是我下载的东西叫 Android SDK Tools，而 platform-tools/adb 叫做 platform tools。Android SDK Tools 的作用是帮你下载 platform tools 以及 SDK 以及其他别的 tools 的。
		所以需要先 cd 到 Android SDK Tools 下的 tools 里面，然后运行 ./android
		这个东西就是 Linux 版的 Android SDK Manager，用这个东西去下载其他的 tool 和 SDK，用法跟 windows 一样。  
		adb 在 platform-tools 里面，aapt 在 build-tools 里面。  
		运行 ./adb 报错：  
		No such file or directory  
		因为在64位ubuntu系统上运行32位程序需要安装32位lib：  
		sudo apt-get install ia32-libs  
		安装这个软件包又报错：  
		Package ia32-libs is not available, but is referred to by another package.
		This may mean that the package is missing, has been obsoleted, or
		is only available from another source
		However the following packages replace it:
		  lib32z1 lib32ncurses5 lib32bz2-1.0  
		解决办法-运行以下命令即可（运行后不必再 install  ia32-libs）：  
		sudo apt-get install g++-multilib  
		这下终于可以运行 adb 了  
		但是运行 aapt 报错：error while loading shared libraries: libz.so.1: cannot open shared object  
		解决办法：
		sudo apt-get install lib32z1  
		这下终于也可以运行 aapt 了  
把adb和aapt都加入到PATH：  
在.bashrc最后加入：  
export PATH=$PATH:$HOME/android-sdk-linux/platform-tools  
export PATH=$PATH:$HOME/android-sdk-linux/build-tools/23.0.2  
然后解决虚拟机 adb 连接设备的问题  
首先保证USB插到物理机上的时候，能被VMWare截获。  
然后在虚拟机上用 lsusb 命令看一下，能不能看到设备  
能看到的话，直接adb就能调试了  
这下就能在虚拟机上搞CTS了 