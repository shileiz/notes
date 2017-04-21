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
* 服务端 onBind() 返回一个 IBinder
* 服务端返回给客户端的东西，首先必须是一个IBinder，这样安卓OS才能把它传递给客户端
* 其次，这必须是个能调用服务端功能函数的东西，这是客户端绑定服务端的目的
* 所以，服务端一般要弄一个类，它从 IBinder 继承，并添加了服务端对外开放的业务函数
* 添加的所有业务函数，一般统一抽象出来封装到一个 interface 里，然后让返回给客户端的那个东西去实现该接口
* 以下例子是 MusicService 提供给客户端的接口类：

		:::Java
		/*MusicInterface.java， 服务端*/
		public interface MusicInterface {
			void play();
			void pause();
		}


* onBind()返回的东西，一般是 `extends Binder implements MusicInterface`。 
* 下面例子中，MusicService 的内部类 MusicController，就是用来返回给客户端的 proxy。


		:::Java
		/*MusicService.java，服务端*/
		public class MusicService extends Service{
			
			@Override
			// onBind 返回一个 MusicController 的实例
			public IBinder onBind(Intent intent) {
				return new MusicController();
			}
			
			//onBind()返回的东西, 必须继承自 Binder，并 implements MusicInterface
			// 它是 IBinder 的同时，能提供业务函数
			class MusicController extends Binder implements MusicInterface{
				// 业务函数只是调用 MusicServie 本身的业务函数
				public void play(){
					MusicService.this.play();
				}
				public void pause(){
					MusicService.this.pause();
				}
			}
			
			public void play(){
				// 真正的播放业务逻辑
				...
			}
			
			public void pause(){
				// 真正的pause业务逻辑
				...
			}
		}

* 客户端拿到这个返回的东西，会把它强转成 MusicInterface，然后调用其中的方法。

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
	* 我们这里要自己 new 一个 ServiceConnection，而且不能用匿名对象，因为这个 conn 我们解绑的时候还要用。而 ServiceConnection 是个接口，所以我们需要做一个自己的 class 来实现它。
	* 在实现 onServiceConnected() 方法的时候，要把第二个参数 IBinder 保存出来，因为这是 Service 的代理，后续调用 Service 的方法全靠它。
	* 注意：对我们客户端来说，我们从这个回调函数得到的，只是一个 IBinder，这东西并不能代表 MusicService，它并没有 play() 方法。所以我们要强转一下。

* 第三个参数 flags 一般用 `BIND_AUTO_CREATE` 即可。表示如果被绑定的Service还没启动，则安卓OS会自动启动它。
* 客户端示意代码如下:

		:::Java
		/*MainActivity.java, 客户端*/
		public class MainActivity extends Activity {
		
			MusicInterface mProxy;
		    @Override
		    protected void onCreate(Bundle savedInstanceState) {
				Intent intent = new Intent(this, MusicService.class);
				bindService(intent, new MusicServiceConn(), BIND_AUTO_CREATE);
		    }
		
		    class MusicServiceConn implements ServiceConnection{
		
				@Override
				public void onServiceConnected(ComponentName name, IBinder service) {
					mProxy = (MusicInterface) service;
				}
		
				@Override
				public void onServiceDisconnected(ComponentName name) {
				}
		    }

			onPlaybuttonClicked(){
				// 通过 proxy 调用 Service 的业务函数
				mProxy.play();
			}

		}

#### 小结
* 服务端需要提供的有：
	* 接口类 MusicInterface
	* 继承自 Binder  并实现了 MusicInterface 的 proxy 对象（即 new MusicController），通过 onBind() 函数 return 出去
	* proxy 实现的 MusicInterface 的业务函数，实际上只是调用 MusicService 的业务函数

* 客户端需要做的事情：
	* 搞个类 MusicServiceConn，实现 ServiceConnection 接口，用于接收 Service onBind() 返回的 IBinder，即 proxy
	* MusicServiceConn 实现 onServiceConnected() 方法时，把第二个参数（带出参数）数强转成 MusicInterface
	* 调用 bindService()，第二个参数传 new MusicServiceConn，来接收服务端 proxy

* 哪些东西不能跨进程？
	* 以上东西都可以跨进程，服务端提供的接口类只需要提供 class 文件即可，或者提供个 jar 包
	* 客户端只需要知道 MusicInterface 所在的包就可以 import 并使用之
	* 其他的数据传递都是由安卓 OS 完成的，完全可以跨进程
	* 即上述例子，完全可以把 MainActivity 做成一个 app，MusicService 做成另外一个 app
	* MainActivity 的 app 只需要把 MusicService 里的 MusicInterface.class 拿过来即可


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