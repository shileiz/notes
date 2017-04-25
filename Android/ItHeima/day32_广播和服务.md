##使用Service: 同进程（一）
* 比如，你做了一个音乐播放 app，里面有一个 MusicService 负责后台播放音乐，对外提供 play()，pause() 的接口
* 你在一个 Activity 里想调用这个 Service 的 play()，怎么搞？
* 必须在这个 Activity 里拿到刚才启动的 MusicService 的实例，但这是不可能的。Service 实例是由安卓OS维护的，你拿不到。（启动 Service 可不是 new MusicService() new 出来的，而是调用安卓系统的 startService()/bindService()，这些函数不给你返回被启动的Service实例）
* 我们可以想个办法把 Service 的实例返回去：首先要了解 Service 和客户端的回调机制，这里涉及一个重要的接口类：IBinder。

###Service机制
* 服务端都继承自 Service 类，Service 类有个 onBind() 方法。当有客户端来使用 Service 的时候，安卓OS会调用该 Service 的 onBinde()。因为 onBind() 的返回类型并不是 Service，所以我们不能直接把 Service 的实例返回去。
* 并且，就算你返回去了也没用，因为客户端不是 onBind() 的调用者，安卓OS才是。客户端调的是 bindService()。
* 客户端调用 `bindService(Intent intent, ServiceConnection conn, int flags)`，去绑定 Service，以便能够使用 Service 的功能。
* 安卓OS是这么玩的： 
	* 客户端调用 `bindService(intent, conn, BIND_AUTO_CREATE)` 的时候，传入的 conn 对象是用于回调的；
	* 此时安卓OS调用 Service 的 `onBind(intent)`；
	* 当 onBind() 返回的时候，安卓OS把返回值传给conn的 ： `conn.onServiceConnected(ComponentName name, IBinder service)` ，其中第二个参数就是 onBind() 的返回值。

####捋一下 bindService() ：
* `bindService(Intent intent, ServiceConnection conn, int flags)`
* 第一个参数 intent 指定要绑定的是哪个 Service，我们这里就是: `new Intent(this, MusicService.class);`
* 第三个参数 flags 一般用 `BIND_AUTO_CREATE` 即可。表示如果被绑定的Service还没启动，则安卓OS会自动启动它。
* 重点是第二个参数。 
	* 它是 ServiceConnection 类型的，ServiceConnection 是个接口
	* 当 Service 绑定好了之后（Service 的 onBind() 返回了），安卓OS会回调 `conn.onServiceConnected(ComponentName name, IBinder service)`
	* 安卓OS会在这个回调里，传来一个 IBinder 类型的对象（第二个参数）：service。
	* 这个 service 是 Service 端的 onBind() 方法 return 的东西。
* 知道了这个机制，就可以想办法让客户端拿到 Service 的实例了。

###让客户端拿到Service的实例
* Service 端通过重写 onBind() 方法，让它返回一个可以拿到 Service 实例的 IBinder。比如给这个 IBinder 添加一个方法，getService().

		public class MusicService extends Service {
		
		    // 用于让 onBind() 返回的类，它从 Binder 继承，即实现了 IBinder 接口
			// 我们给它加了一个方法，getService()，用于返回 MusicService 的实例
			public class MyBinder extends Binder {
		        LocalService getService() {
		            return MusicService.this;
		        }
		    }
		
			// 重写 onBind 方法，返回一个 MyBinder 的实例，
			// 而 MyBinder 的 getService() 可以拿到 MusicService 实例
		    @Override
		    public IBinder onBind(Intent intent) {
		        return new MyBinder();
		    }
		
		    public int play() {
		      Log.d("testing", "playing")
		    }
		}

* 客户端准备一个 ServiceConnection，用来绑定 Service 并接收 Service onBind() 返回的那个 IBinder。
* 重写这个 ServiceConnection 的 onServiceConnected() 方法，把安卓OS回传过来的 IBinder 用起来，得到 Service 的实例。

		public class MainActivity extends Activity {
		    MusicService mService;

			// 准备一个 ServiceConnection，用来绑定 Service 并接收 Service onBind() 返回的那个 IBinder。
		    private ServiceConnection mConnection = new ServiceConnection() {
		
		        @Override
		        public void onServiceConnected(ComponentName className,IBinder service) {
		            // 注意 ！！！！ 这里要进行强转，才能使用 getService() 方法 ！！！！
					MyBinder binder = (MyBinder) service;
		            mService = binder.getService();
		        }
		    };
		
		    @Override
		    protected void onCreate(Bundle savedInstanceState) {
		        super.onCreate(savedInstanceState);
		        setContentView(R.layout.main);
		        Intent intent = new Intent(this, MusicService.class);
		        bindService(intent, mConnection, BIND_AUTO_CREATE);
		    }
		
		    public void onButtonClick(View v) {
				// 调用 Service 的业务函数
				mService.play();
		    }
		}

* 注意，我们这里把安卓OS传过来的 IBinder 强转成了 MyBinder。这很关键，因为如果是跨进程的话，这步强转是无法进行的。后面会有例子。

###小结
* Service 重写 onBind() 方法，返回一个 IBinder 的子类。
* 客户端用 conn 的 onServiceConnected() 方法接收到这个返回的对象。
* 这样，客户端和 Service 的联系就建立起来了。


##使用Service: 同进程（二）
* 这一节跟上一节没有本质区别，只是在上节基础上做了一点包装。
* 让 Service 的 onBind() 函数返回的不光是一个 IBinder，还是一个 MusicInterface。
* MusicInterface 是个接口，统一封装了 MusicService 的业务函数。客户端和服务端都通过该接口使用业务函数。

		:::Java
		package com.zsl.musicplayer0;
		
		public interface MusicInterface {
			void play(String f);
			void pause();
		}

####服务端
* onBind() 返回的东西，一般是 `extends Binder implements MusicInterface`。 
* 下面例子中，MusicService 的内部类 MusicServiceProxy，就是用来返回给客户端的 proxy。


		:::Java
		package com.zsl.musicplayer0;
		
		import android.app.Service;
		import android.content.Intent;
		import android.os.Binder;
		import android.os.IBinder;
		import android.util.Log;
		
		public class MusicService extends Service {
		
			private String mFile;
		
			// MusicServiceProxy 类的作用是 proxy。 proxy只负责分发，把活交给 Service 的业务函数干
			class MusicServiceProxy extends Binder implements MusicInterface {
		
				@Override
				public void play(String f) {
					MusicService.this.play(f);
				}
		
				@Override
				public void pause() {
					MusicService.this.pause();
				}
			}
		
			// 模拟播放的业务逻辑
			public void play(String f) {
				mFile = f;
				Log.d("zsl", "Play: " + mFile);
			}
		
			// 模拟暂停的业务逻辑
			public void pause() {
				Log.d("zsl", "Paused");
			}
		
			@Override
			public IBinder onBind(Intent intent) {
				return new MusicServiceProxy();
			}
		}

####客户端
* 客户端示意代码如下:

		:::Java
		package com.zsl.musicplayer0;
		
		import android.app.Activity;
		import android.content.ComponentName;
		import android.content.Intent;
		import android.content.ServiceConnection;
		import android.os.Bundle;
		import android.os.IBinder;
		import android.util.Log;
		import android.view.View;
		
		public class PlayerActivity extends Activity {
		
			private String mFile;
			private MusicInterface mMusicServiceProxy;
		
			@Override
			protected void onCreate(Bundle savedInstanceState) {
				super.onCreate(savedInstanceState);
				setContentView(R.layout.activity_player);
				mFile = new String("sdcard/a.mp3");	
				Intent intent = new Intent(this, MusicService.class);
				bindService(intent, new MusicServiceConn(), BIND_AUTO_CREATE);
			}
			
		    class MusicServiceConn implements ServiceConnection{
				
				@Override
				public void onServiceConnected(ComponentName name, IBinder service) {
					mMusicServiceProxy = (MusicInterface) service;
				}
		
				@Override
				public void onServiceDisconnected(ComponentName name) {
				}
		    }
			
			public void btn_play_clicked(View v){
				mMusicServiceProxy.play(mFile);
			}
			
			public void btn_pause_clicked(View v){
				mMusicServiceProxy.pause();
			}
			
		}

#### 小结
* 服务端需要提供的有：
	* 接口类 MusicInterface
	* 继承自 Binder  并实现了 MusicInterface 的 proxy 类，即 MusicServiceProxy，它的业务函数，实际上只是调用 MusicService 的业务函数
	* 通过 onBind() 函数 return proxy 对象，即 new MusicController()

* 客户端需要做的事情：
	* 搞个类 MusicServiceConn，实现 ServiceConnection 接口，用于接收 Service onBind() 返回的 IBinder，即 proxy
	* MusicServiceConn 实现 onServiceConnected() 方法时，把第二个参数（带出参数）数强转成 MusicInterface
	* 调用 bindService()，第二个参数传 new MusicServiceConn，来接收服务端 proxy

* 为何不能跨进程？
	* 客户端拿到 proxy 后，强转那句，不能跨进程。即：
	
			mMusicServiceProxy = (MusicInterface) service;
	* 这个 service 是安卓OS传过来的，它的本质是Server进程里的一个对象，即一堆二进制数据。安卓OS拿他当 IBinder 类型看待。
	* 如果你的客户端是另一个进程，你没办法成功进行强转的。哪怕你把 MusicInterface.java 拷贝到了客户端进程，并且包名也是使用服务端的包名。
	* 虽然这样客户端可以 import MusicInterface，并且使用它，但强转的时候还是会报错如下：

			java.lang.ClassCastException: 
			android.os.BinderProxy cannot be cast to com.zsl.musicserver.MusicInterface
			
	* 其根本原因就是一个进程里的对象，是不可能被另一个进程认识。

#### 如果你想做实验，见证真的不能跨进程
* 拆分成两个工程，一个 Sever（com.zsl.musicserver）， 一个 Client（com.zsl.musicclient）
* Server 工程包括如下源文件： MusicInterface.java， MusicService.java
* Client 工程包括如下源文件： MusicInterface.java， MainActivity.java
* Client 需要知道 Server 的包名和 Service 的类名：

		Intent intent = new Intent();
		intent.setClassName("com.zsl.musicserver", "com.zsl.musicserver.MusicService");  

* 问题：Client 绑定 Service 时报错，不让绑定

		java.lang.SecurityException: Not allowed to bind to service Intent

* 解决方式是在 Client 里先 startService(intent) 然后再 bindService(intent, ...)
* 最后，你会遇到不让强转的问题


##使用Service: 跨进程


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