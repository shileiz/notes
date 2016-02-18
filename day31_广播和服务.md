## 广播
* 大多数广播都是 Android 系统发送的，我们自己发送广播的情况很少
* Android 系统几乎在所有系统状态改变时都会发送广播，为程序员提供了很大方便。
* 比如电量改变、收发短信、拨打电话、解锁屏幕、开机完成、SD卡状态改变，安装卸载软件。。。等等的时候，系统都会发送广播
### 有序广播和无序广播
* `Context.sendBroadcast(Intent intent)`  发送的是无序广播
* `Context.sendOrderedBroadcast(...)` 发送的是有序广播
* 无序广播：所有跟广播的intent匹配的广播接收者都可以收到该广播，并且是没有先后顺序（视为同时收到）
* 有序广播：按照广播接收者的优先级来决定接收的先后顺序，如果优先级高的接收者把该广播abort了，则剩下的接收者就收不到了
	* 优先级的定义：-1000~1000
	* 最终接收者：所有广播接收者都接收到广播之后，它才接收，并且一定会接收
		* 最终接收者是在发送有序广播时指定的，所以该接收者不用在清单文件中配置
		* `sendOrderedBroadcast` 的第三个参数 `BroadcastReceiver resultReceiver` 就是用来指定最终接收者的
	* abortBroadCast：阻止其他接收者接收这条广播，类似拦截，只有有序广播可以被拦截
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
	1. 对于打电话广播
		* 系统把播出的电话号码以一个String封装了起来，可以调用 BroadcastReceiver 的 `getResultData()` 方法来取得
		* 注意，只有有序广播才能通过这个方法获得数据。这个方法得到的String是上一个 BroadcastReceiver 用 `setResultData()` 设置的
	* 对于收到短信广播
		* 系统把短信数据封装到了intent里，作为Extra，以Bundle的数据结构封装进去的
		* `onReceive(Context context, Intent intent)` 的第二个参数收到的就是启动本次广播的那个intent
		* 以下代码得到封装了短信数据的Bundle
				
				Bundle bundle = intent.getExtras();
		* 短信的具体格式不是本节重点这里不记了，需要的去查原版笔记