#关于Activity
* 一个应用程序可以在桌面创建多个快捷图标。
* Manifest里面的Activity，只要有以下的intent-filter，就会在桌面上有一个图标

		 <intent-filter>
             <action android:name="android.intent.action.MAIN" />
             <category android:name="android.intent.category.LAUNCHER" />
         </intent-filter>

* activity的名称、图标可以和应用程序的名称、图标不相同

		android:icon="@drawable/ic_launcher"
        android:label="@string/app_name"

---
#Activity的跳转
* Activity的跳转需要创建Intent对象，通过设置intent对象的参数指定要跳转Activity
* 通过设置Activity的包名和类名实现跳转，称为显式意图
* 通过指定动作实现跳转，称为隐式意图
###显式意图：可以启动本应用的Activity和其他应用的Activity
* 显式意图启动本应用的另一个Activity，直接指定该Activity的字节码即可

		Intent intent = new Intent();
		intent.setClass(this, SecondActivity.class);
    	startActivity(intent);
* 显式意图启动其他应用的Activity，需要指定该应用的包名和该Activity的类名

		//启动拨号器
		Intent intent = new Intent();
		intent.setClassName("com.android.dialer", "com.android.dialer.DialtactsActivity");  
		startActivity(intent);  
		//启动微信  
		Intent intent = new Intent();
		intent.setClassName("com.tencent.mm", "com.tencent.mm.ui.LauncherUI");
		startActivity(intent);  

###隐式意图：也是可以启动本应用的Activity和其他应用的Activity
* 隐式意图启动本应用的另一个Activity
		
		Intent intent = new intent();
		intent.setAction("com.zsl.MyAction");
		startActivity(intent);
* 以上Activity可以被隐式启动，需要在清单文件的activity节点中设置intent-filter子节点

		<intent-filter >
            <action android:name="com.zsl.MyAction"/>
            <category android:name="android.intent.category.DEFAULT"/>
        </intent-filter>
	* action 指定动作（必须有这个节点，其值可以自定义，也可以使用系统自带的）
	* category 类别 （必须有这个节点，如果该节点写为DEFAULT，则启动这个Activity的intent无需设置category，否则必须设置）
	* 比如上面的intent-filter的category节点改为<category android:name="android.intent.category.APP_MUSIC""/>，那么启动它的intent就必须addCategory：
				
			intent.addCategory(Intent.CATEGORY_APP_MUSIC);  
	* 我们开发应用，category一般都写DEFAULT
	* 除以上两种节点外，intent-filter还可以有其他节点。只要Manifest里给intent-filter设置了子节点，则启动它的intent就必须有相应的设置才能启动
	* 比如上面的intent-filter加了一个<data android:scheme="zsl"/>子节点，要想隐式意图启动这个Activity，则意图必须setData：
				
			intent.setData(Uri.parse("zsl:balabalbal"));
	* intent-filter节点及其子节点都可以同时定义多个，隐式启动时只需与任意一个匹配即可
* 隐式意图启动其他应用的Activity（需要知道被启动的Activity的intent-filter需要什么样的Action，Category，Data等等）

		Intent intent = new Intent();
		//启动系统自带的拨号器应用
    	intent.setAction(Intent.ACTION_DIAL);
    	startActivity(intent);


###显式意图和隐式意图的应用场景
* 显式意图用于启动同一应用中的Activity
* 隐式意图用于启动不同应用中的Activity
	* 如果系统中存在多个Activity的intent-filter同时与你的intent匹配，那么系统会显示一个对话框，列出所有匹配的Activity，由用户选择启动哪一个
	
