* 锁屏的时候 Activity 自动走了 onDestroy，onCreate （参考 day30）
	* 只有 PlayerActivity 会这样， 文件浏览 Activity 就不会
	* 网上查了是因为我的 PlayerActivity 设置成了横屏（清单文件里配置的），所以锁屏时会触发Activity重走整个生命周期
	* 为了避免重走整个生命周期，需要在清单文件里为 PlayerActivity 再配置如下属性：
	* `android:configChanges="keyboardHidden|orientation|screenSize" `
	* 这个意思是，让本 Activity 能捕捉到哦设备的 `keyboardHidden|orientation|screenSize` 这三个状态变化
	* 这样 Android 系统就不用因为这几种设备状态改变而重新走一遍 Activity 的生命周期了
	* 另外，正常的 Activity （比如文件浏览的），没有设置强制横屏的，在锁屏的时候只会走如下生命周期：`onPause，onStop`
	* 延伸1： 被 `android:configChanges` 捕获的动作到哪去了？ 
	* 答：会被 Activity 的 onConfigurationChanged() 处理，你可以重写这个方法做想要的动作。
	* 延伸2：为啥横屏 Activity 锁屏会触发设备的这三个状态改变：`keyboardHidden|orientation|screenSize` 
	* 答：没为啥，记住就可以。Android 系统就是这么干的。注意其中的 `screenSize` 是在 API-13之后才会触发的。

