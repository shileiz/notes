##使用Service：同一进程
* 同一进程内的情况比较简单，先看简单的，再看复杂的
* 场景是：一个app内部，启动了一个 Service，本app的其他地方想使用这个 Service
* 比如，你做了一个音乐播放 app，里面有一个 MusicService 负责后台播放音乐，对外提供 play()，pause() 的接口
* 你在一个 Activity 里想调用这个 Service 的 play()，怎么搞？
* 必须在这个 Activity 里拿到刚才启动的 MusicService 的实例，但这是不可能的。Service 实例是由安卓OS维护的，你拿不到。（启动 Service 可不是 new MusicService() new 出来的，而是调用安卓系统的 startService(),这个函数不给你返回被启动的Service实例）
* 我们确实拿不到 MusicService 的实例，但是我们可以拿到它的“代理”——proxy。
* 如何拿到这个代理？ Service 又是如何实现这个代理的（想让别人拿到你的proxy，作为MusicService，你必须提供一个proxy才行）？
* 我们下面就分别看看客户端和服务端要怎么做。这里把使用Service的一端称为客户端，即我们的Activity；把提供Service的一端称为服务端，即MusicService。

####服务端
* 作为一个 Service，肯定要有客户端来使用我，所以我要提供一个 proxy 让客户端来使用我。
* 这个 proxy 
* 服务端onBind()返回一个IBinder
* 服务端返回给客户端的东西，首先必须是一个IBinder，这样安卓OS才能把它传递给客户端
* 其次，这必须是个能调用服务端功能函数的东西，这是客户端绑定服务端的目的
* 所以，服务端一般要弄一个类，它从IBinder继承，并添加了服务端对外开放的业务函数
* 添加的所有业务函数，一般统一抽象出来封装到一个interface里，然后让返回给客户端的那个东西去实现该接口
* 总之，onBind()返回的东西，一般是 extends IBinder implements XXXX
* 客户端拿到这个返回的东西，会把它强转成 XXXX，然后调用其中的方法。

####客户端
* 客户端要调用 `bindService(Intent intent, ServiceConnection conn, int flags)`，去绑定要使用的 Service，才能获得Service的proxy，从而使用 Service 的功能。下面看一下这个函数：
* 第一个参数 intent 指定要绑定的是哪个 Service，我们这里就是: `new Intent(this, MusicService.class);`
	* 我们这里是直接使用了.class来new这个Intent，当然也可以使用别的方式。比如new一个无参的intent，再set包名类名，或者Action和Category之类的 
	* 总之，跟启动Activity的intent一样，可以用显式的也可以用隐式的。如果用隐式的，需要在 Manifest 里给 Service 加 intent-filter。
* 第二个参数 conn 非常重要，服务端给客户的返回的proxy就通过第二个参数获取
	* 它是 ServiceConnection 类型的，ServiceConnection 是个接口
	* 当 Service 绑定好了之后（Service 的 onBind() 返回了），安卓OS会回调 `conn.onServiceConnected(ComponentName name, IBinder service)`
	* 安卓OS会在这个回调里，传来一个 IBinder 类型的对象（第二个参数）：service。它代表着 Service 提供给客户端的proxy
	* 客户端在得到这个 service 之后，就可以通过它调用 Service 里的方法了。 
	* 我们这里要自己 new 一个 ServiceConnection，而且不能用匿名对象，因为这个 conn 我们解绑的时候还要用
	* 我们可以自己做一个 class MyConn，实现 ServiceConnection 接口。我们实现 onServiceConnected() 方法的时候，要把第二个参数 IBinder 保存出来，因为这是 Service 的代理，后续调用 Service 的方法全靠它。
	* 注意：对我们客户端来说，我们从这个回调函数得到的，只是一个 IBinder，这东西并不能代表 MusicService，它并没有 play() 方法。
	* 所以，我们必须保证 MusicService 绑定好之后，回调函数传回来的这个 IBinder 不仅仅是个 IBinder，还要？？？？？？？？？
* 第三个参数 flags 一般用 `BIND_AUTO_CREATE` 即可。表示如果被绑定的Service还没启动，则安卓OS会自动启动它。

##使用Service：跨进程

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