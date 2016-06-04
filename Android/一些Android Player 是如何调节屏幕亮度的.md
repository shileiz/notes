## 基础
* 系统屏幕亮度变化时是不会发广播的
* 系统是通过 ContentProvider 来提供亮度信息的：`Settings.System.SCREEN_BRIGHTNESS`
* 也就是说你如果想实时获取亮度，要搞一个 ContentObserver
* 但是！ Android 上的 Player 貌似不通过改变这个来改变亮度。