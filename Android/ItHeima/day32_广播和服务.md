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