### 官方SDK文档关于Service的例子
*  参考： docs/reference/android/app/Service.html#LocalServiceSample
*  同一进程使用 Service 时很简单，clients 只需把从 Service 得到的 IBinder 强转为 Service 暴露的接口类即可。

> When used in this way, by assuming the components are in the same process, you can greatly simplify the interaction between them:
> clients of the service can simply cast the IBinder they receive from it to a concrete class published by the service. 

* 跨进程使用 Service，官方例子里用了 Messager，这样可以避免使用 AIDL

> If you need to be able to write a Service that can perform complicated communication with clients in remote processes, then you can use the Messenger class instead of writing full AIDL files. 

* Service 把 Messager 作为 clients 的接口，在自己内部实现一个 Handler 来接收 clients 发来的 Message
* Client 同样需要暴露一个 Messager 作为接口提供给 Service 用来做回调，所以 Client 内部也要实现一个 Handler 来处理 Service 的回调命令

* 在 Service 端，onBind() 返回给 Client 的 IBinder 是：  

		return mMessenger.getBinder();
		final Messenger mMessenger = new Messenger(new IncomingHandler());

* 在 Client 端，当绑定成功后：  
		
		mService = new Messenger(service);
		public void onServiceConnected(ComponentName className, IBinder service) {
        	mService = new Messenger(service);
			...
		}



### 关于 Messenger
* Messenger 是基于 Binder 的，也是用于进程间通信的
* Messenger 的构造函数可以基于 Handler 也可以基于 IBinder：

		Messenger(Handler target)
		Messenger(IBinder target)

> Reference to a Handler, which others can use to send messages to it. This allows for the implementation of message-based communication across processes, by creating a Messenger pointing to a Handler in one process, and handing that Messenger to another process.   
  
> Note: the implementation underneath is just a simple wrapper around a Binder that is used to perform the communication. 