##adb相关

###通过 adb 命令行启动一个 app

* adb shell am  
参考：docs/tools/help/adb.html  
am ： activity manager  
开启微信的命令（不需要权限）：
  
		adb shell am start com.tencent.mm/.ui.LauncherUI  
start 后面跟的是一个 <INTENT> 参数  
这个参数要用 package\_name/activity\_name 的形式  
那么如何知道一个app的package name 呢？  

	* 在程序里得到 am start 传入的参数：
	* 首先要使用 -e 参数传入参数 （此时要使用 -n 参数来指定Activity名，而不是像上面不写参数）
	* `adb shell am start -n com.example.test/.TestActivity -e content /sdcard/rmhd/a.rmhd`
	* -e 后面连续跟两个东西，以空格分开，分别是参数的key和value
	* 在代码里这么得到这个value： 
	*  
				Intent i = getIntent();
        		String content = i.getStringExtra("content");

* adb shell pm  
pm ： package manager   
列出所有 package 的命令：  

		adb shell pm list packages
列出含有字符串 tencent 的 package( 得到结果：package:com.tencent.mm  )：  
  
		adb shell pm list packages  tencent  
找到某个 package 的 APK（得到结果：package:/data/app/com.tencent.mm-2.apk  ）：  

		adb shell pm path com.tencent.mm  
知道 package name 之后，还需要知道 activity name，才能用 adb am start 来启动这个app  
那么如何知道可启动的 activity name 呢？  
需要用SDK提供的一个工具，来分析app的APK得到答案。  
APK：上面的 adb shell pm path 命令，已经可以得到这个app的apk路径了，只要把apk文件pull出来即可。  
工具：aapt.exe ：Android Asset Packaging Tool  
这个工具在 sdk 的 build-tools 目录下  
分析apk文件得到可启动activity的命令是：  

		aapt.exe d badging ./com.tencent.mm-2.apk  
输出一大堆，其中这一行就是可启动的activity：  
launchable-activity: name='com.tencent.mm.ui.LauncherUI'  label='WeChat' icon=''  
不是 launchable-activity 的 Activity 能不能通过 am 启动呢？  
答案是不能。Permission Denial。
* 也可以通过观察logcat来得到一个app的包名和Activity名  
手机连接电脑，电脑上打开logcat，然后在手机上启动微信，就能在logcat里找到 com.tencent.mm/.ui.LauncherUI  这条log  
这就是微信的package\_name/activity\_name  

###adb connect： 不链接USB线，通过wifi链接adb
* 打开设备被 adb connect 的方法：  
  
		在手机/电视上打开命令行终端，输入如下命令（需要su到root执行）：
		setprop service.adb.tcp.port 5555
		stop adbd
		start adbd
* 设备搞好了之后，只需要在PC上输入 adb connect 192.168.x.x 即可链接。

###adb 强制刷新媒体数据库
* 发送一个 sdcard 被重新挂载的广播即可

		db shell am broadcast -a android.intent.action.MEDIA_MOUNTED -d file:///mnt/sdcard/xxxx
* 让手机不停的抽取某一个文件的缩略图
	* 给文件改名，让手机认为有新文件来了
	* 发送上述广播，让系统更新媒体数据库，把新来（其实是改了名）的媒体文件的缩略图抽出来
	* python 代码如下：

			from subprocess import Popen
			for i in range(99999):
			    cmd = ['adb', 'shell', 'mv', r'/sdcard/AdrenalineRush_720p_%s.wmv' % (str(i),), r'/sdcard/AdrenalineRush_720p_%s.wmv' % (str(i+1),),]
			    Popen(cmd).wait()
			    cmd = ['adb', 'shell', 'am', 'broadcast', '-a', 'android.intent.action.MEDIA_MOUNTED', '-d', r'file:///mnt/sdcard/',]
			    Popen(cmd).wait()