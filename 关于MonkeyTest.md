## 常用参数

### `-s seed`
* seed 是个数，最为随机生成动作序列的种子。比如500，1000，都可以。
* 相同 seed 产生的动作序列是一样的。如果想每次跑 Monkey 都产生同样事件序列，则每次的 -s 参数都设成一样即可。

### `-p package`
* 允许被打开的 app
* 比如 `adb shell monkey -p com.android.gallery3d`
* 如果要同时跑多个 app，要用多个 `-p`
* 比如 `adb shell monkey -p com.android.gallery3d -p com.android.music`
* 当然写太多了也烦，可以用 `--pkg-blacklist-file` 或者 `--pkg-whitelist-file` 代替，后面会讲。

### `--ignore-crashes --ignore-timeouts --ignore-security-exceptions --ignore-native-crashes --monitor-native-crashes`
* 这5个参数一般都用上，防止因为 crash 和 timeout（比如 ANR） 导致monkey停下来；
* `--monitor-native-crashes` 用来记录底层库的 crash。

### `-v -v -v `
* 把 Monkey 的 log 级别设置到最详细级别
* 注意要连着写3个 `-v`

###  `--throttle <milliseconds>`
* 在随机事件之间插入固定时间间隔。注意： 不加这个参数**默认是无间隔**，即尽可能快的发送随机事件。
* If not specified, there is no delay and the events are generated as rapidly as possible.
* 一般可以把这个参数设置成 1000，即每隔1秒产生下一个动作
* 根据经验，如果不设间隔，即让它尽可能快的跑，平均每秒钟能产生60~70个随机事件（基于双核64bit的Nexus9，针对图库app）


## Monkey Log  分析
* 一般用3个 -v 来输出日志
* 在日志里搜 ANR、NOT RESPONS、CRASH 可以确定是否有 ANR 和 CRASH 发生
* 在日志里搜 system_uptime 可以确定时间：
	* 可以粗略的视第一个出现 system uptime 的地方为开始跑 Monkey 的时间
	* 然后在发生 ANR 或 CRASH 的紧挨着的上面一条为发生错误的时间
	* 两个时间差就是错误大概在开跑多久之后出现的，然后可以根据这个时间去logcat的输出中找到更详细的log（logcat 记得加上参数 `-v threadtime`）。
	* 注意这个时间是毫秒
* 例子：

		SET MONKEY_OUTPUT=monkey.log
		SET REPORT_NAME=MonkeyScreenLog
		
		 "会在本批处理文件目录下生成6个报告："
		echo "Error_List_%REPORT_NAME%.txt 为两种应用层错误（Crash和ANR）按发生时间排列的列表。"
		echo "Error_Crash_%REPORT_NAME%.txt 为Crash错误的列表和错误原因。"
		echo "Error_NotRes_%REPORT_NAME%.txt 为Not Responding错误的列表和错误原因。"
		echo "Error_Details_%REPORT_NAME%.txt 为两种错误按发生时间排列的总的错误列表和原因。"
		echo "时间戳_%REPORT_NAME%.txt 为发生错误的时间戳（毫秒数）。
		echo "实际执行次数_%REPORT_NAME%.txt 为实际执行了多少次随机事件。
		
		findstr /n /c:"CRASH:" /c:"NOT RESPONDING"  %MONKEY_OUTPUT% > Error_List_%REPORT_NAME%.txt
		findstr /n /c:"CRASH:" /c:"Short Msg:" /c:"Long Msg:" %MONKEY_OUTPUT% > Error_Crash_%REPORT_NAME%.txt
		findstr /n /c:"NOT RESPONDING" /c:"ANR " /c:"Reason:" %MONKEY_OUTPUT% > Error_NotRes_%REPORT_NAME%.txt
		findstr /n /c:"CRASH:" /c:"Short Msg:" /c:"Long Msg:" /c:"NOT RESPONDING" /c:"ANR " /c:"Reason:" %MONKEY_OUTPUT% > Error_Details_%REPORT_NAME%.txt
		findstr /n /c:"system_uptime" /c:"crash" /c:"CRASH:" /c:"echoNOT RESPONDING"  %MONKEY_OUTPUT% >时间戳_%REPORT_NAME%.txt
		findstr /n /c:":Sending Trackball"  %MONKEY_OUTPUT% >实际执行次数_%REPORT_NAME%.txt


## 不太常用参数 

### `--pkg-blacklist-file PACKAGE_BLACKLIST_FILE`
* 参数 PACKAGE_BLACKLIST_FILE 是个文本文件，里面写上 package 的名字，一个 package 占一行即可，比如：

		com.android.nfc
		com.android.wallpapercropper
		com.android.galaxy4
		com.tencent.research.drop
* 然后把这个文件起名叫 blacklist.txt，并 push 到手机的 /data
* 运行 `adb shell monkey --pkg-blacklist-file /data/blacklist.txt` 即可
* `--pkg-whitelist-file` 跟这个参数一样，见名知意不再赘述。

### `--pct-xxxxx <percent>`
* 官方文档里只给出8中事件的百分比设置：
* `--pct-touch，--pct-motion，--pct-trackball，--pct-nav，--pct-majornav，--pct-syskeys，--pct-appswitch，--pct-anyevent` 
* 但在Monkey的日志文件里，却能看到11种事件：
* 
		// Event percentages:
		//   0: 15.0%
		//   1: 10.0%
		//   2: 2.0%
		//   3: 15.0%
		//   4: -0.0%
		//   5: 25.0%
		//   6: 15.0%
		//   7: 2.0%
		//   8: 2.0%
		//   9: 1.0%
		//   10: 13.0%
* 最坑的是，日志里只写了0,1,2...这些各占多少百分比，但是没说0代表啥，1代表啥
* 更加坑的是，不同SDK版本里，0,1,2这些数字代表的事件是不同的
* 想知道哪些事件代表啥，只能查看对应版本SDK的源码：MonkeySourceRandom.java
* 对于Android4.4.2来说，对应关系如下：
* 
		0：触摸事件（touch）百分比，即参数--pct-touch
		1：滑动事件（motion）百分比，即参数--pct-motion
		2：缩放事件（pinchzoom）百分比，
		3：轨迹球事件（trackball）百分比，即参数--pct-trackball
		4：屏幕旋转事件（rotation）百分比，
		5：基本导航事件（nav）百分比，即参数--pct-nav
		6：主要导航事件（majornav）百分比，即参数--pct-majornav
		7：系统事件（syskeys）百分比，即参数--pct-syskeys
		8：Activity启动事件（appswitch）百分比，即参数--pct-appswitch
		9：翻转事件（flip）百分比，
		10：其他事件（anyevent）百分比，即参数--pct-anyevent
* 可以看到，缩放（pinchzoom）、旋转（rotation）、翻转（flip），这三种事件是不能通过Monkey的参数指定百分比的
