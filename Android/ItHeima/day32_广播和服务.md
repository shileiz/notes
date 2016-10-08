##绑定服务

####客户端
* 客户端绑定Service，调用的是 `bindService(Intent intent, ServiceConnection conn, int flags)`
* 绑的是哪个 Service 呢？ 通过第一个参数 intent 来告诉安卓OS
* 绑定是个异步过程，客户端只告诉安卓OS，我要绑定这个 Service，具体什么时候绑好，需要回调
	* 第二个参数 conn 负责回调，它是一个 ServiceConnection 类型的对象，ServiceConnection 是个接口
	* 当 Service 绑定好了之后（Service 的 onBind() 返回了），安卓OS会回调 `conn.onServiceConnected(ComponentName name, IBinder service)`
	* 安卓OS会在这个回调里，传来一个 IBinder 类型的对象（第二个参数）：service。它代表着 Service 提供给客户端的所有接口
	* 客户端在得到这个 service 之后，就可以通过它调用 Service 里的方法了。 
	* 疑问：为什么一个 IBinder 类型的对象 service 能调用到 Service 里的功能函数，不同的 Service 有不同的功能函数啊
	* 答案：每个 Service 都要自己写一个类（比如叫 BnXXXService），继承自 IBinder，给这个类添加自己想暴露给客户端的业务函数，然后让安卓OS在回调里，把这个类的对象返回给客户端。
	* 客户端在得到这个 IBinder 类型的 service 之后，首先把它强转成 Service 暴露给客户端的类型（比如，`BnXXXService bn = (BnXXXService) service`），就可以使用它了。比如 `bn.doSomething()`
* 第三个参数 flags 一般用 `BIND_AUTO_CREATE` 即可。表示如果被绑定的Service还没启动，则安卓OS会自动启动它。

####服务端
* 服务端onBind()返回一个IBinder
* 服务端返回给客户端的东西，首先必须是一个IBinder，这样安卓OS才能把它传递给客户端
* 其次，这必须是个能调用服务端功能函数的东西，这是客户端绑定服务端的目的
* 所以，服务端一般要弄一个类，它从IBinder继承，并添加了服务端对外开放的业务函数
* 添加的所有业务函数，一般统一抽象出来封装到一个interface里，然后让返回给客户端的那个东西去实现该接口
* 总之，onBind()返回的东西，一般是 extends IBinder implements XXXX
* 客户端拿到这个返回的东西，会把它强转成 XXXX，然后调用其中的方法。

####跨进程的问题
* 如果Service和客户端运行在不同的程序中，比如Service是支付宝，客户端是你写的app
* 那么你没办法把支付宝Service返回的那个 IBinder 强转成一个支付宝的接口类XXXX
* 因为你的app代码里，根本就没法定一个XXXX的对象，interface XXXX不是安卓SDK的一部分，是支付宝的一部分
* 另外，安卓OS给你的app传回来的 IBinder 对象，是从另一个进程支付宝来的。它对支付宝来说，是内存中的一个对象，但对你的进程来说，它只是一串二进制数而已。所以你真的是无法使用它啊。
* 这就涉及到进程间通信，AIDL
* 支付宝提供给你的app一个aidl文件，比如叫IAlipay.aidl。里面定义了一个interface叫 IAlipay。
* 你把IAlipay.aidl放到你自己的工程中（要放到 IAlipay 所在的包中，中aidl文件里查看），Eclipse会为你生成相应的 IAlipay.java (在gen里面)
* 这样，你的代码里就有了 IAlipay 这个支付宝的接口类了，你就可以在 conn 的回调里，把接收到的 IBinder 强转成 IAlipay 使用了
* 不过，AIDL 的用法不是让你直接强转的
* 你应该这样使用：`IAlipay.Stub.asInterface(service);` 它会返回一个 IAlipay 类型的对象。
* 注意：支付宝实际上并不对外提供 aidl 文件，而是提供 jar 包，具体接入支付宝的方式可以看[支付宝官方文档](https://doc.open.alipay.com/docs/doc.htm?spm=a219a.7629140.0.0.Z3z0Fs&treeId=59&articleId=103681&docType=1)。


##使用代码注册广播接收者
* Android四大组件都要在清单文件中注册
* 广播接收者比较特殊，既可以在清单文件中注册，也可以直接使用代码注册
* 使用清单文件注册的：广播一旦发出，系统就会去所有清单文件中寻找，哪个广播接收者的action和广播的action是匹配的，如果找到了，就把该广播接收者的进程启动起来
* 使用代码注册的：需要使用广播接收者时，执行注册的代码，不需要时，执行解除注册的代码。好处是，在不需要启动进程时，就可以不启动。要不然只要广播一发出，就启动本接收者所在的进程，太占资源。
* 有的广播接收者，必须代码注册，因为这些广播发的很频繁
	* 电量改变
	* 屏幕锁屏和解锁
* 注册广播接收者

		//创建广播接收者对象，ScreenOnOffReceiver是自己写的类，继承自 BroadcastReceiver
		receiver = new ScreenOnOffReceiver();
		
		//通过IntentFilter对象指定广播接收者接收什么类型的广播
		IntentFilter filter = new IntentFilter();
		filter.addAction(Intent.ACTION_SCREEN_OFF);
		filter.addAction(Intent.ACTION_SCREEN_ON);
		
		//注册广播接收者
		registerReceiver(receiver, filter);

* 解除注册广播接收者

		unregisterReceiver(receiver);

* 解除注册之后，广播接收者将失去作用

##启动远程服务

###服务的分类
* 本地服务：指的是服务和启动服务的activity在同一个进程中
* 远程服务：指的是服务和启动服务的activity不在同一个进程中

###启动远程服务
* startService()，bindService() 都可以启动远程服务。都是显示启动，都需要知道该远程Service的类名才行。
* 不过启动远程服务需要知道该Service的类名才行，这是显示启动。