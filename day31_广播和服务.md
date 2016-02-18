## 广播
* 大多数广播都是 Android 系统发送的，我们自己发送广播的情况很少
* Android 系统几乎在所有系统状态改变时都会发送广播，为程序员提供了很大方便。
* 比如电量改变、收发短信、拨打电话、解锁屏幕、开机完成、SD卡状态改变，安装卸载软件。。。等等的时候，系统都会发送广播
### 广播接收者
* 即使广播接收者的进程没有启动，当系统发送的广播可以被该接收者接收时，系统会自动启动该接收者所在的进程 
* 启动该接收者所在的进程，就是启动你的安卓项目：com.xxx.xxx
* 但是不会把你的主 Activity 弄到前台来
##### 在清单文件中配置广播接收者
* 需要在清单文件中配置（四大组建都需要）
* 广播接收者是跟 activity 同级的，在 application 节点下，叫做 `<receiver android:name="com.xxx.xxx.xxxReceiver">`
* 广播接收者必须添加 `<intent-filter >` 子节点，来告诉安卓系统你这个接收者是接收什么广播的
* `<intent-filter >` 子节点必须添加 ` <action>` 子节点来过滤广播的类型
* 一个广播接收者在清单文件里至少是以下这样的：
* 
		<receiver android:name="com.xxx.xxx.xxxReceiver">
            <intent-filter >
                <action android:name="android.intent.action.XXXXX"/>
            </intent-filter>
        </receiver>
* 广播接收者也是需要权限的，比如接收播出电话的广播，需要以下权限：
* 
		<uses-permission android:name="android.permission.PROCESS_OUTGOING_CALLS"/>
##### 在程序中添加广播接收者
* 搞一个类，继承 BroadcastReceiver， 重写 onReceive() 方法
##### 怎么从广播中拿到数据
* 不同的广播，封装数据的方法不一样，所以从广播中得到数据的方法也不一样
* 对于系统广播，我们只能查文档去看广播发送时怎么封装数据的
* 下面举两个例子：
* 对于打电话广播
	* 系统把播出的电话号码以一个String封装了起来，可以调用 BroadcastReceiver 的 `getResultData()` 方法来取得

* 对于收到短信广播
	* 打发打发
* 根据官方API，拿到的是上个广播接收者 `setResultData(String data)` 的结果
* 我们也不知道上一个收到打电话广播的是谁，应该是系统的某个app
* 但我们 `setResultData()` 之后，打电话app再收到这个广播，再 `getResultData()` 到的号码，就是我们设置的新号码了
* 总之 Android 系统往外打电话那个app，是在我们的app之后才收到 `NEW_OUTGOING_CALL `这个广播的
* 这才能保证我们改号码成功了