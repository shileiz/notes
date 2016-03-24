###  `--throttle <milliseconds>`
* 在随机事件之间插入固定时间间隔。注意： 不加这个参数**默认是无间隔**，即尽可能快的发送随机事件。
* If not specified, there is no delay and the events are generated as rapidly as possible.

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