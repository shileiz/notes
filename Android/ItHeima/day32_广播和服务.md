##同进程使用Service（一）： IBinder
* 比如，你做了一个音乐播放 app，里面有一个 MusicService 负责后台播放音乐，对外提供 play()，pause() 的接口
* 你在一个 Activity 里想调用这个 Service 的 play()，怎么搞？
* 必须在这个 Activity 里拿到刚才启动的 MusicService 的实例，但这是不可能的。Service 实例是由安卓OS维护的，你拿不到。（启动 Service 可不是 new MusicService() new 出来的，而是调用安卓系统的 startService()/bindService()，这些函数不给你返回被启动的Service实例）
* 我们可以想个办法把 Service 的实例返回去：首先要了解 Service 和客户端的回调机制，这里涉及一个重要的接口类：IBinder。

###Service机制
* 服务端都继承自 Service 类，Service 类有个 onBind() 方法。当有客户端来使用 Service 的时候，安卓OS会调用该 Service 的 onBinde()。不过因为 onBind() 的返回类型并不是 Service，所以我们不能直接把 Service 的实例返回去。
* 并且，就算你返回去了也没用，因为客户端不是 onBind() 的调用者，安卓OS才是。客户端调的是 bindService()。
* 客户端调用 bindService() 去绑定 Service，以便能够使用 Service 的功能。函数原型是：`boolean bindService(Intent intent, ServiceConnection conn, int flags)`，
* 第一个参数 intent 指定要绑定的是哪个 Service，我们这里就是: `new Intent(this, MusicService.class);`
* 第三个参数 flags 一般用 `BIND_AUTO_CREATE` 即可。表示如果被绑定的Service还没启动，则安卓OS会自动启动它。
* 重点是第二个参数 conn。 
	* 它是 ServiceConnection 类型的
	* 当 Service 绑定好了之后（Service 的 onBind() 返回了），安卓OS会回调 `conn.onServiceConnected(ComponentName name, IBinder service)`
	* 安卓OS会在这个回调里，传来一个 IBinder 类型的对象（第二个参数），它就是 Service 端的 onBind() 方法 return 的东西。
* 但这里还是没有拿到 Service 的实例，因为 onBind() 返回的并不是 Service 实例。
* 不过既然知道了服务端和客户端之间交互的流程，就可以想办法让客户端拿到 Service 实例了。

###让客户端拿到Service的实例
* Service 端通过重写 onBind() 方法，让它返回一个可以拿到 Service 实例的 IBinder。比如给这个 IBinder 添加一个方法，getService().

		public class MusicService extends Service {
		
		    // 用于让 onBind() 返回的类，它从 Binder 继承，即实现了 IBinder 接口
			// 我们给它加了一个方法，getService()，用于返回 MusicService 的实例
			public class MyBinder extends Binder {
		        MusicService getService() {
		            return MusicService.this;
		        }
		    }
		
			// 重写 onBind 方法，返回一个 MyBinder 的实例，
			// 而 MyBinder 的 getService() 可以拿到 MusicService 实例
		    @Override
		    public IBinder onBind(Intent intent) {
		        return new MyBinder();
		    }
		
		    public void play() {
		      Log.d("testing", "playing");
		    }
		}

* 客户端准备一个 ServiceConnection，用来绑定 Service 并接收 Service onBind() 返回的那个 IBinder。
* 重写这个 ServiceConnection 的 onServiceConnected() 方法，把安卓OS回传过来的 IBinder 用起来，得到 Service 的实例。

		import com.zsl.musicserver.MusicService.MyBinder;
		
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
		
				@Override
				public void onServiceDisconnected(ComponentName name) {
					// TODO Auto-generated method stub
					
				}
		    };
		
		    @Override
		    protected void onCreate(Bundle savedInstanceState) {
		        super.onCreate(savedInstanceState);
		        setContentView(R.layout.activity_main);
		        Intent intent = new Intent(this, MusicService.class);
		        bindService(intent, mConnection, BIND_AUTO_CREATE);
		    }
		
		    public void onButtonClick(View v) {
				// 调用 Service 的业务函数
				mService.play();
		    }
		}

* 以上程序写在同一个工程里可以正确运行，点击 play 按钮后 logcat 输出 playing。

###小结
* Service 重写 onBind() 方法，返回一个 IBinder 的子类。
* 客户端用 conn 的 onServiceConnected() 方法接收到这个返回的对象。
* 这样，客户端和 Service 的联系就建立起来了。

###问题
* 首先，上面这种做法很难做到跨进程，先不说原理，就从代码层面看也不行
* 上例中客户端需要拿到Service的实例，这就要求客户端和服务端必须在一个工程里，如果是两个 app 根本没法搞：

		MusicService mService; // 客户端代码里必须有 MusicService 这个类

* 理论上服务端的 MusicService 类对客户端应该是透明的，客户端只需要使用 Service 的功能就可以了。
* 所以我们稍微更进一步，把 MusicService 抽象出一个接口：MusicInterface。服务端和客户端都有接口类 MusicInterface，但分别实现，客户端是给服务端发消息，服务端真的干活。

##同进程使用Service（二）：还是 IBinder
* 这一节跟上一节没有本质区别，只是在上节基础上做了一点包装。
* 让 Service 的 onBind() 函数返回的不光是一个 IBinder，还是一个 MusicInterface。
* MusicInterface 是个接口，统一封装了 MusicService 的业务函数。客户端和服务端都通过该接口使用业务函数。

		:::Java
		package com.zsl.musicservice;
		
		public interface MusicInterface {
			void play();
			void pause();
		}

####服务端
* onBind() 返回的东西是 `extends Binder implements MusicInterface`。 
* 下面例子中，MusicService 的内部类 MusicServiceProxy，就是用来返回给客户端的 proxy。


		:::Java
		package com.zsl.musicservice;
		
		import android.app.Service;
		import android.content.Intent;
		import android.os.Binder;
		import android.os.IBinder;
		import android.util.Log;
		
		public class MusicService extends Service {
		
			// proxy只负责分发，把活交给 Service 的业务函数干
			class MusicServiceProxy extends Binder implements MusicInterface {
		
				@Override
				public void play() {
					MusicService.this.play();
				}
		
				@Override
				public void pause() {
					MusicService.this.pause();
				}
			}
		
			// 模拟播放的业务逻辑
			public void play() {
				Log.d("zsl", "Playing");
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
* 客户端代码如下:

		:::Java
		package com.zsl.musicclient;
		
		import com.zsl.musicplayer0.R;
		import com.zsl.musicservice.MusicInterface;
		import com.zsl.musicservice.MusicService;
		
		import android.app.Activity;
		import android.content.ComponentName;
		import android.content.Intent;
		import android.content.ServiceConnection;
		import android.os.Bundle;
		import android.os.IBinder;
		import android.view.View;
		
		public class PlayerActivity extends Activity {
		
			private MusicInterface mMusicServiceProxy;
		
			@Override
			protected void onCreate(Bundle savedInstanceState) {
				super.onCreate(savedInstanceState);
				setContentView(R.layout.activity_player);
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
				mMusicServiceProxy.play();
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

* 为何不能跨进程？
	* 客户端拿到 proxy 后，强转那句，不能跨进程。即：
	
			mMusicServiceProxy = (MusicInterface) service;
	* 这个 service 是安卓OS传过来的，它的本质是Server进程里的一个对象，即一堆二进制数据。安卓OS拿他当 IBinder 类型看待。
	* 如果你的客户端是另一个进程，你没办法成功进行强转的。哪怕你把 MusicInterface.java 拷贝到了客户端进程。
	* 虽然这样客户端可以 import MusicInterface，并且使用它，编译的时候也没问题。但运行时，强转的时候还是会报错如下：

			java.lang.ClassCastException: 
			android.os.BinderProxy cannot be cast to com.zsl.musicclient.MusicInterface
	
	* 如果还是不服，把 MusicInterface 类的包也改成跟服务端一样的，还是报错：

			java.lang.ClassCastException: 
			android.os.BinderProxy cannot be cast to com.zsl.musicserver.MusicInterface
			
	* 其根本原因就是一个进程里的对象，是不可能被另一个进程认识。

#### 如果你想做实验，见证真的不能跨进程
* 拆分成两个工程，一个 Sever（com.zsl.musicserver）， 一个 Client（com.zsl.musicclient）
* Server 工程包括如下源文件： MusicService.java，MusicInterface.java
* Client 工程包括如下源文件： MainActivity.java，MusicInterface.java
* Client 需要知道 Server 的包名和 Service 的类名：

		Intent intent = new Intent();
		intent.setClassName("com.zsl.musicserver", "com.zsl.musicserver.MusicService");  

* 问题：Client 绑定 Service 时报错，不让绑定

		java.lang.SecurityException: Not allowed to bind to service Intent

* 解决方式是在 Client 里先 startService(intent) 然后再 bindService(intent, ...)
* 最后，你会遇到不让强转的问题



##跨进程使用Service（一）：AIDL

###一、AIDL 怎么用

####1.书写 AIDL 文件，定义接口

* 新建一个 Android 工程，作为服务端。
* 新建一个 Interface，起名叫 MusicInterface。MusicInterface.java 内容如下：

		package com.zsl.musicservice2aidl;
		
		public interface MusicInterface {
		    void play();
		    void pause();
		}

* 直接把 MusicInterface.java 改成 MusicInterface.aidl，并且把 public 去掉，因为 AIDL 语法不支持 public。MusicInterface.aidl 内容如下：

		package com.zsl.musicservice2aidl;
		
		interface MusicInterface {
		    void play(String f);
		    void pause();
		}


####2. 实现服务端
* 新建一个 MusicService。这次我们让 onBind() 返回的类不用自己搞一个 `extends Binder implements MusicInterface` 的内部类了，AIDL 已经为我们搞定了。我们只需要搞一个内部类，让它 `extends Stub` 即可。Stub 是 AIDL 为我们生成的一个类，它已经 `extends Binder implements MusicInterface` 了。
* 服务端代码如下：


		package com.zsl.musicservice2aidl;
		
		// 把 AIDL 生成的 Stub 类 import 进来
		import com.zsl.musicservice2aidl.MusicInterface.Stub;
		
		import android.app.Service;
		import android.content.Intent;
		import android.os.IBinder;
		import android.util.Log;
		
		public class MusicService extends Service {
		
			// 用于让 onBind() 返回的类，只需要从 Stub 继承即可，比原来简单了些
			 class MusicServiceProxy extends Stub {
		
			        @Override
			        public void play() {
			            MusicService.this.play();
			        }
		
			        @Override
			        public void pause() {
			            MusicService.this.pause();
			        }
			    }
		
			    // 模拟播放的业务逻辑
			    public void play() {
			        Log.d("zsl", "Playing");
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

* 记得在项目的 Manifest 文件里写上这个 Service。
* 另外为了能被远程启动和绑定，最好把该 Service 的 export 搞成 true。

####2. 实现客户端
* 新建一个 Android 工程，作为客户端。
* 客户端也需要 MusicInterface 这个接口，把 MusicInterface.aidl 拷贝到客户端项目。注意！！ 一定要在客户端建一个跟服务端一样的包，把 MusicInterface.aidl 放到这个包里。
* 客户端的代码如下：

		package com.zsl.musicclient2aidl;
		
		// 注意包名，跟服务端是一致的
		import com.zsl.musicservice2aidl.MusicInterface;
		import com.zsl.musicservice2aidl.MusicInterface.Stub;
		
		import android.app.Activity;
		import android.content.ComponentName;
		import android.content.Intent;
		import android.content.ServiceConnection;
		import android.os.Bundle;
		import android.os.IBinder;
		import android.os.RemoteException;
		import android.view.View;
		
		public class MainActivity extends Activity {
		
		    private MusicInterface mMusicServiceProxy;
		
		    @Override
		    protected void onCreate(Bundle savedInstanceState) {
		        super.onCreate(savedInstanceState);
		        setContentView(R.layout.activity_main);
		        Intent intent = new Intent();
		        intent.setClassName("com.zsl.musicservice2aidl", "com.zsl.musicservice2aidl.MusicService");
		        startService(intent);
		        bindService(intent, new MusicServiceConn(), BIND_AUTO_CREATE);
		    }
		
		    class MusicServiceConn implements ServiceConnection{
		
		        @Override
		        public void onServiceConnected(ComponentName name, IBinder service) {
		        	// 这里不直接强转了，而是使用 Stub的asInterface()方法
		            mMusicServiceProxy = Stub.asInterface(service);
		        }
		
		        @Override
		        public void onServiceDisconnected(ComponentName name) {
		        }
		    }
		
		    public void btn_play_clicked(View v){
		        try {
					mMusicServiceProxy.play();
				} catch (RemoteException e) {
					e.printStackTrace();
				}
		    }
		}

* 可以看到客户端代码跟原来区别很小，最关键的一点就是： 在把 IBinder 强转成 MusicInterface 时，不再直接强转，而是使用 Stub 的 asInterface() 方法。
* 部署服务端再部署客户端，实验，可以跨进程调用。


###二、AIDL的原理

* MusicInterface.aidl 文件写好后，会在 gen 目录下生成 MusicInterface.java，其内容如下（经过精简）：


		package com.zsl.musicservice2aidl;
		
		public interface MusicInterface extends android.os.IInterface {

			public static abstract class Stub extends Binder implements MusicInterface {

				private static final String DESCRIPTOR = "com.zsl.musicservice2aidl.MusicInterface";
		
				public Stub() {
					this.attachInterface(this, DESCRIPTOR);
				}
		

				public static MusicInterface asInterface(IBinder obj) {
					if ((obj == null)) {
						return null;
					}
					IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
					if (((iin != null) && (iin instanceof MusicInterface))) {
						return ((MusicInterface) iin);
					}
					return new MusicInterface.Stub.Proxy(obj);
				}
		
				@Override
				public IBinder asBinder() {
					return this;
				}
		
				@Override
				public boolean onTransact(int code, Parcel data, Parcel reply, int flags) throws RemoteException {
					switch (code) {
					case INTERFACE_TRANSACTION: {
						reply.writeString(DESCRIPTOR);
						return true;
					}
					case TRANSACTION_play: {
						data.enforceInterface(DESCRIPTOR);
						this.play();
						reply.writeNoException();
						return true;
					}
					case TRANSACTION_pause: {
						data.enforceInterface(DESCRIPTOR);
						this.pause();
						reply.writeNoException();
						return true;
					}
					}
					return super.onTransact(code, data, reply, flags);
				}
		
				private static class Proxy implements MusicInterface {
					private IBinder mRemote;
		
					Proxy(IBinder remote) {
						mRemote = remote;
					}
		
					@Override
					public IBinder asBinder() {
						return mRemote;
					}
		
					public String getInterfaceDescriptor() {
						return DESCRIPTOR;
					}
		
					@Override
					public void play() throws android.os.RemoteException {
						android.os.Parcel _data = android.os.Parcel.obtain();
						android.os.Parcel _reply = android.os.Parcel.obtain();
						try {
							_data.writeInterfaceToken(DESCRIPTOR);
							mRemote.transact(Stub.TRANSACTION_play, _data, _reply, 0);
							_reply.readException();
						} finally {
							_reply.recycle();
							_data.recycle();
						}
					}
		
					@Override
					public void pause() throws android.os.RemoteException {
						android.os.Parcel _data = android.os.Parcel.obtain();
						android.os.Parcel _reply = android.os.Parcel.obtain();
						try {
							_data.writeInterfaceToken(DESCRIPTOR);
							mRemote.transact(Stub.TRANSACTION_pause, _data, _reply, 0);
							_reply.readException();
						} finally {
							_reply.recycle();
							_data.recycle();
						}
					}
				}
		
				static final int TRANSACTION_play = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
				static final int TRANSACTION_pause = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
			}
		
			public void play() throws RemoteException;
		
			public void pause() throws RemoteException;
		}


###扩展：传递复杂数据类型：用 AIDL 定义类型


##跨进程使用Service（二）：Message

### 1. 回忆一下 Handler 和 Message
* 在UI编程时，我们经常用 Handler 和 Message 处理高耗时任务。
* 具体方法：
	* Activity 里弄个 Handler 类型的成员 mHandler。 
	* mHandler 的 handleMessage() 负责处理 Message
	* 在 Activity 里起一个线程去做高耗时任务，该线程通过 mHandler.sendMessage() 给 mHander 发送 Message。
* 要点： Activity 里的工作线程能拿到 mHander。这很容易，内部类可以通过类名访问外部类。


### 2. Messenger
* UI 里只使用了两个类： Handler、Message。 UI 里都是调用 Handler 的 sendMessage(Message msg) 方法，发送一个 Message。然后 Handler 的 handleMessage() 会收到这个 Message 参数。
* 说白了，这种传递 Message 的方式，是用同一个对象（mHandler）的两个方法（sedMessage(),handleMessage()）在传递 Message。
* 而同一个对象不可能横跨两个进程，所以我们需要一个新类：Messenger。
* Messager 是基于 Binder 构建的，所以可以跨进程。 Messenger 的构造函数需要一个 IBinder 参数：

		Messenger(IBinder target)
		// Create a Messenger from a raw IBinder, which had previously been retrieved with getBinder(). 

* 以上是 API 文档里对该构造函数的描述，他说那个 IBinder 参数必须是之前用 getBinder() 拿到的。这是啥意思？
* Messenger 类有个 getBinder() 方法，能返回这个 Messenger 与对应的 Handler 通信用的 IBinder。 
* 对应的 Handler 在哪里？ Messager 还有一个构造函数，以 Handler 为参数。

	 	Messenger(Handler target)
		//Create a new Messenger pointing to the given Handler. 

* 用 Messenger 发送 Message 的时候，直接发给该 Handler。
* 要点：
	1. Messenger 有两个构造函数，一个以 Handler 为参数，一个以 IBinder 为参数。
	2. Messenger 有个 getBinder() 方法，能返回与它对应的 Handler 通信用的 IBinder。
* 这样就可在服务端弄一个 Handler，用来处理客户端发的 Message。然后基于它构建一个 Messenger，用其 getBinder() 方法得到 Messenger 与 Handler 通信的 IBinder。
* 然后服务端的 onBind() 返回这个 IBinder。
* 客户端得到这个 IBinder 后，再基于它构建一个自己的 Messenger，这个 Messenger 发的消息，就会被服务端收到了。
* 注意： 服务端和客户端各有自己的 Messenger 对象，而不是同一个对象。服务端的 Messenger 是基于 Handler 构建，客户端的是基于 IBinder 构建。

### 3. 利用 Messenger 跨进程使用 Service

#### 3.1服务端
* 服务端代码如下：

		package com.zsl.musicserver1message;
		
		public class MusicServer extends Service {
		
		    static final int MSG_PLAY = 1;
		
		    class ServiceHandler extends Handler  {
		        @Override
		        public void handleMessage(Message msg) {
		            switch (msg.what) {
		                case MSG_PLAY:
		                	MusicServer.this.play();
		                    break;
		                default:
		                    super.handleMessage(msg);
		            }
		        }
		    }
		
		    final Messenger mMessenger = new Messenger(new ServiceHandler());
		
		    @Override
		    public IBinder onBind(Intent intent) {
		        //返回给客户端一个 IBinder 实例,客户端基于它构建自己的 Messager，然后发消息过来
		        return mMessenger.getBinder();
		    }
		    
		    public void play(){
		    	Log.d("Messenger Test", "Playing");
		    }
		}

* Service 的 Manifest 里要加上这个： `android:exported="true" `

#### 3.2 客户端
* 客户端代码如下：

		package com.zsl.musicclient1message;
		
		public class MainActivity extends Activity {
			
		    static final int MSG_PLAY = 1;
		
		    Messenger mService = null;
		
		    private ServiceConnection mConnection = new ServiceConnection() {
		        public void onServiceConnected(ComponentName className, IBinder service) {
		            //接收onBind()传回来的IBinder，并用它构造Messenger
		            mService = new Messenger(service);
		        }
		
				@Override
				public void onServiceDisconnected(ComponentName name) {
				}
		
		    };
		
		    public void play() {
		        Message msg = Message.obtain(null, MSG_PLAY, 0, 0);
		        try {
		            mService.send(msg);
		        } catch (RemoteException e) {
		            e.printStackTrace();
		        }
		    }
		    
			@Override
			protected void onCreate(Bundle savedInstanceState) {
				super.onCreate(savedInstanceState);
				setContentView(R.layout.activity_main);
				Intent intent = new Intent();
				intent.setClassName("com.zsl.musicserver1message", "com.zsl.musicserver1message.MusicServer");
				startService(intent);
				bindService(intent, mConnection, BIND_AUTO_CREATE);
			}
			
		    public void btn_play_clicked(View v){
		        play();
		    }
		}



## Messenger 和 AIDL 哪个好？
* 本质都是 Binder


---

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