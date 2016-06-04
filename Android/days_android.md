##日常记录
###ImageView做按钮
* 要加属性：
* 
		android:clickable="true"
* 不要加属性：
*                  
		android:focusable="true"
        android:focusableInTouchMode="true"
* 不然会导致第一下点击获得焦点，第二下点击才触发onClick事件
* 让图片周围留出空白，但不缩小图片的点击范围，即不能用margin：
*             
        android:layout_width="60dp"
        android:layout_height="45dp"
        android:scaleType="centerInside"
* 添加点击视觉效果：
	* 把 ImageView 的 android:background 属性设置成一个 selector
	* 这个 selector 在 press 的情况下为一个 shape，其他情况下为 透明：
	* 
			<?xml version="1.0" encoding="utf-8"?>
			<selector xmlns:android="http://schemas.android.com/apk/res/android">
			    <item android:drawable="@drawable/shape_btn_pressd_background" android:state_pressed="true"/>
			    <item android:drawable="@android:color/transparent" />
			</selector>
	* 其中shape也可以用一张自定义的图片代替，但个人觉得shape比较好。上面的selector用到的shape：
	* 
			<?xml version="1.0" encoding="utf-8"?>
			<shape 
				xmlns:android="http://schemas.android.com/apk/res/android"
			    android:shape="rectangle" >
			    <solid android:color="@color/grey" />
			</shape>
### 关于 Adapter (前面拷贝自 day40)
* 一般直接继承 BaseAdapter 写一个自己的 Adapter 就行了，这是最常用的。
* BaseAdapter 是抽象类，它是实现了 ListView 接口的。没人直接去 new 一个 ListAdapter 的，需要实现的方法太多了。
* 另外像 ArrayAdapter、SimpleAdapter 都是 BaseAdapter 的子类，用起来更方便，但自由度没有 BaseAdapter 高。
* 当Adapter的数据改变的时候，只要调用它的 notifyDataSetChanged() 方法就行了
* 这时 Android 系统就知道这个 Adapter 的数据变了，就会重新去重新调用 getView() 了
* 不过，由于我们继承 BaseAdapter，它的数据源是我们手动指定的，所以我们应该给它添加一个能更新数据源的方法，比如叫 updateSource(new_data)
* 我们每次通知系统数据源变了之前，先手动更新我们的数据源
* 就是，在调 notifyDataSetChanged() 之前，先调一下 updateSource(new_data)
* 以下为例子：
* 
		package com.zsl.listadapter;
		
		import java.util.ArrayList;
		
		import android.app.Activity;
		import android.content.Context;
		import android.os.Bundle;
		import android.util.Log;
		import android.view.View;
		import android.view.ViewGroup;
		import android.widget.BaseAdapter;
		import android.widget.Button;
		import android.widget.ListView;
		import android.widget.TextView;
		
		public class MainActivity extends Activity {
		   
			private ListView lv_list;
			private MyAdapter myadapter;
			private Button btn_update;
		   
			class MyAdapter extends BaseAdapter{
				
				private Context  context;
				private ArrayList<String> mydata;
				private static final String TAG = "shilei";
				
				public MyAdapter(Context ctx, ArrayList<String> data){
					Log.d(TAG, "create MyAdapter");
					this.context = ctx;
					this.mydata = data;
				}
				
				@Override
				public int getCount() {
					Log.d(TAG, "getCount(), source length: "+ mydata.size());
					return mydata.size();
				}
		
				@Override
				public Object getItem(int position) {
					return mydata.get(position);
				}
		
				@Override
				public long getItemId(int position) {
					return position;
				}
		
				@Override
				public View getView(int position, View convertView, ViewGroup parent) {
					Log.d(TAG, "getView(), position: "+position);
					if(convertView == null){
						convertView = View.inflate(context, android.R.layout.simple_list_item_1, null);
					}
					TextView text1 = (TextView) convertView.findViewById(android.R.id.text1);
					text1.setText(mydata.get(position));
					return convertView;
				}
				
				public void updateSource(ArrayList<String> data){
					this.mydata = data;
				}
				
			}
			
			@Override
		    public void onCreate(Bundle savedInstanceState) {
		        super.onCreate(savedInstanceState);
		        setContentView(R.layout.activity_main);
		        
		        lv_list = (ListView) findViewById(R.id.list);
		        ArrayList<String> data = new ArrayList<String>();
		        for (int i = 0; i < 5; i++) {
		        	data.add(new String("" + (i + 1)));
		        }
		        
		        myadapter = new MyAdapter(this, data);
		        lv_list.setAdapter(myadapter);
		        
		        btn_update = (Button) findViewById(R.id.update);
		        btn_update.setOnClickListener(new View.OnClickListener() {
					@Override
					public void onClick(View v) {
						ArrayList<String> data = new ArrayList<String>();
				        for (int i = 5; i < 100; i++) {
				        	data.add(new String("" + (i + 6)));
				        }
				        myadapter.updateSource(data);
						myadapter.notifyDataSetChanged();
					}
				});
		        
		    }
		}

### ListView Adapter的 getCount()、getView()不停的被调用
* 网上说是　ListView 的宽高设置成 match_parent 或者固定值就好了
* 试了一下还是不行
* 最后发现原因是我把 ListView 所在的 ViewGroup（是个自定义 View）放在了某个 LinearLayout 的下面造成的：
* `android:layout_below="@id/ll_top_control" `
* 无论那个 LinearLayout 的高是 wrap_content 还是固定大小，都不行
* 没有深入研究下去

### Activity的 addContentView(View view, LayoutParams params)
* Android 里所有 Activity 的根布局都是一个 FrameLayout
* 你用 setContentView() 给你的 Activity 加的布局文件，是这个 FrameLayout 的子节点
* addContentView(View view, LayoutParams params) 也是把 view 添加给这个 frameLayout 的


##Player开发相关
###关于Player开发中各种互相阻塞的问题
* 从 Activity 的 onCreate() 退出之前，SurfaceView 是不会被创建的。
   即 必须等 onCreate() 全部执行完后，才会进入 surfaceCreated()  
   以下例子，永远看不到 "surface created" 这条 log  

		public class VideoPlayerActivity extends Activity {
			private SurfaceView 	surface_view;
			private SurfaceHolder 	surface_holder;
			
			@Override
			protected void onCreate(Bundle savedInstanceState) {
				super.onCreate(savedInstanceState);
				setContentView(R.layout.video_player);
				surface_view = (SurfaceView) findViewById(R.id.video_view);
				surface_holder = surface_view.getHolder();
				surface_holder.addCallback(new Callback(){
					@Override
					public void surfaceCreated(SurfaceHolder holder) {
						Log.d("zsl","surface created");
						//openVideo();
					}
		
					@Override
					public void surfaceChanged(SurfaceHolder holder, int format,
							int width, int height) {			
					}
		
					@Override
					public void surfaceDestroyed(SurfaceHolder holder) {
					}
					
				});
				while(!surface_holder.isCreating()){
					Log.d("zsl","ont Creating surface view");
				}
			}
		}


* 必须在 surfaceCreated() 方法返回之后，player 才会准备好，其 onPrepared() 才会被调用，画面才会出现在屏幕上。  
“PreparedTest” 证明了以上观点。 

* Listener不会被主调函数阻塞。  
在某个函数里为player设置了各种Listener，不用等到这个函数返回这些Listener才会被调用，只要事件发生了，会立刻被调用。  
比如 onPrepared() 会在player准备好了的时候立刻被调用，onCompletion() 会在视频播放完成的时候立刻被调用。  
PS: 所谓“准备好了”，必须保证这个player的surfaceview的surfaceCreated()已经返回了。    
“PlayerListenersTest” 可以证明以上观点。  


* 主调函数不会被Listener阻塞  
Listener 是在单独的线程里运行的，跟主调函数并行。  
“PlayerListenersTest2” 可以证明以上观点。  
另外需注意：必须 player 准备好了之后（onPrepared() 返回了），才可能进入 onCompletion()。  


* 总之，Listener 和主调函数互不阻塞，Listener 在独立的线程中运行回调函数。
* 最重要的几点：
 - onCreate() 不返回， surfaceCreated() 不会被调用
 - surfaceCreated() 不返回， onPrepared() 不会被调用
 - onPrepared() 不返回，plyaer 就处于未准备好状态，其不可能进入其他状态，包括 seekcomplete 也不可能。  
### android.widget.VideoView
* 安卓SDK自己为我们实现了一个封装了MediaPlayer的继承自SurfaceView的类 —— VideoView  
* 做视频播放器的时候可以使用 MediaPlayer+SurfaceView 的方式，也可以使用 VideoView  
* 前者灵活但需要自己写的代码多  
* 后者使用简单但不够灵活  

##adb相关
###通过 adb 命令行启动一个 app

* adb shell am  
参考：docs/tools/help/adb.html  
am ： activity manager  
开启微信的命令（不需要权限）：
  
		adb shell am start com.tencent.mm/.ui.LauncherUI  
start 后面跟的是一个 <INTENT> 参数  
这个参数要用 package\_name/activity\_name 的形式  
那么如何知道一个app的package name 呢？  

	* 在程序里得到 am start 传入的参数：
	* 首先要使用 -e 参数传入参数 （此时要使用 -n 参数来指定Activity名，而不是像上面不写参数）
	* `adb shell am start -n com.example.test/.TestActivity -e content /sdcard/rmhd/a.rmhd`
	* -e 后面连续跟两个东西，以空格分开，分别是参数的key和value
	* 在代码里这么得到这个value： 
	*  
				Intent i = getIntent();
        		String content = i.getStringExtra("content");

* adb shell pm  
pm ： package manager   
列出所有 package 的命令：  

		adb shell pm list packages
列出含有字符串 tencent 的 package( 得到结果：package:com.tencent.mm  )：  
  
		adb shell pm list packages  tencent  
找到某个 package 的 APK（得到结果：package:/data/app/com.tencent.mm-2.apk  ）：  

		adb shell pm path com.tencent.mm  
知道 package name 之后，还需要知道 activity name，才能用 adb am start 来启动这个app  
那么如何知道可启动的 activity name 呢？  
需要用SDK提供的一个工具，来分析app的APK得到答案。  
APK：上面的 adb shell pm path 命令，已经可以得到这个app的apk路径了，只要把apk文件pull出来即可。  
工具：aapt.exe ：Android Asset Packaging Tool  
这个工具在 sdk 的 build-tools 目录下  
分析apk文件得到可启动activity的命令是：  

		aapt.exe d badging ./com.tencent.mm-2.apk  
输出一大堆，其中这一行就是可启动的activity：  
launchable-activity: name='com.tencent.mm.ui.LauncherUI'  label='WeChat' icon=''  
不是 launchable-activity 的 Activity 能不能通过 am 启动呢？  
答案是不能。Permission Denial。
* 也可以通过观察logcat来得到一个app的包名和Activity名  
手机连接电脑，电脑上打开logcat，然后在手机上启动微信，就能在logcat里找到 com.tencent.mm/.ui.LauncherUI  这条log  
这就是微信的package\_name/activity\_name  
###adb connect： 不链接USB线，通过wifi链接adb
* 打开设备被 adb connect 的方法：  
  
		在手机/电视上打开命令行终端，输入如下命令（需要su到root执行）：
		setprop service.adb.tcp.port 5555
		stop adbd
		start adbd
* 设备搞好了之后，只需要在PC上输入 adb connect 192.168.x.x 即可链接。

##CTS相关 
#####环境准备：
1、一台Linux的PC  
2、PC上装好adb和aapt  
3、PC上装好JDK（CTS5.0+安装java7，CTS4.4-安装java6）  
4、PC上下载CTS测试包，google官网就有。包括两部分：      
1) CTS包（要与被测Android版本对应）：android-cts-5.1\_r3-linux\_x86-arm.zip   
2) 媒体文件包：android-cts-media-1.1.zip  
5、保证pc可以检测到手机  
#####手机配置：  
1、恢复出厂设置  --- [是否可以不做？]                                              --- [本测试没做]  
2、语言设置为英语  
3、打开位置服务（settings>Location）--- [如果不测GPS相关，是否可以不做？]          --- [本测试是打开的]  
4、连接到一个支持IPv6的能连入Internet的wifi --- [如果不测wifi相关，是否可以不做？] --- [本测试连入的是realnetworks]  
5、保证没有滑动解锁/图案解锁/密码解锁等设置：Settings > Security > Screen lock = 'None'  
6、打开USB调试  
7、Select: Settings > Developer options > Stay Awake  
8、Select: Settings > Developer options > Allow mock locations （Android6.0以后不需要了）  
9、Launch the browser and dismiss any startup/setup screen.                        --- [本测试打开浏览器后没有任何startup/setup screen出现，什么也没做]  
10、连接PC，允许PC调试这台手机  
11、安装CTS测试需要的apk  
  
    adb install -r android-cts/repository/testcases/CtsDeviceAdmin.apk
	安装完成后在Settings > Security > device administrators 里面，仅勾选如下两个：
	android.deviceadmin.cts.CtsDeviceAdminReceiver
	android.deviceadmin.cts.CtsDeviceAdminReceiver2
12、拷贝媒体文件到手机：  

	cd 到解压的媒体文件包路径，给copy\_media.sh加可执行权限：chmod u+x copy\_media.sh  
	运行该脚本：./copy\_media.sh all  
	注意：在Ubuntu上运行这个脚本可能会报关于[:的错误，原因是ubuntu的sh默认解释器是dash，而dash对一些shell语法不支持  
	解决办法为指定为bash执行：bash copy_media.sh all  
	或者修改默认解释器。  
	方法：  
	sudo dpkg-reconfigure dash  
	选择no即可.  
#####开始跑测试：
1、连接手机到PC  
2、手机按home键回到主屏幕  
3、手机不要跑其他程序，摆放到稳定位置防止改变横竖屏，摄像头对准一个可聚焦的物体     --- [本测试没做摄像头对准物体]  
4、测试过程中不要碰手机  
5、进入到解压的CTS包，运行这个脚本以进入cts的console：./android-cts/tools/cts-tradefed  
6、用命令 run cts --package android.media --package android.mediastress 来跑CTS测试  
7、在console观察结果。在console观察结果不太方便，可以直接在shell下运行 cts-tradefed  run cts --package android.media >> xxxx.log  
#####一些说明：
* CTS可用按plan来跑，也可以按照更细分的package来跑，还可以按照更细分的class来跑  
* 其他CTS命令：
  
		list packages  查看所有可用的测试package  
		list plans     查看所有测试计划  
* CTS的PC端要使用Linux的原因：1、谷歌提供的PC端的脚本都是shell脚本。2、官方文档关于CTS的描述中，PC端都默认为Linux系统。
* 虚拟机上搞CTS：  
虚拟机环境：VMWare+Ubuntu Kylin 14.04  
宿主实体机：Window7 64位  
CTS需要ADB和AAPT，所以先下载Linux版本的 Android SDK Tools  

		Linux版的 Android SDK Tools 被坑经历：  
		下载Android SDK Tools解压后发现没有platform-tools文件夹，本以为adb就在里面了。
		原因是我下载的东西叫 Android SDK Tools，而 platform-tools/adb 叫做 platform tools。Android SDK Tools 的作用是帮你下载 platform tools 以及 SDK 以及其他别的 tools 的。
		所以需要先 cd 到 Android SDK Tools 下的 tools 里面，然后运行 ./android
		这个东西就是 Linux 版的 Android SDK Manager，用这个东西去下载其他的 tool 和 SDK，用法跟 windows 一样。  
		adb 在 platform-tools 里面，aapt 在 build-tools 里面。  
		运行 ./adb 报错：  
		No such file or directory  
		因为在64位ubuntu系统上运行32位程序需要安装32位lib：  
		sudo apt-get install ia32-libs  
		安装这个软件包又报错：  
		Package ia32-libs is not available, but is referred to by another package.
		This may mean that the package is missing, has been obsoleted, or
		is only available from another source
		However the following packages replace it:
		  lib32z1 lib32ncurses5 lib32bz2-1.0  
		解决办法-运行以下命令即可（运行后不必再 install  ia32-libs）：  
		sudo apt-get install g++-multilib  
		这下终于可以运行 adb 了  
		但是运行 aapt 报错：error while loading shared libraries: libz.so.1: cannot open shared object  
		解决办法：
		sudo apt-get install lib32z1  
		这下终于也可以运行 aapt 了  
把adb和aapt都加入到PATH：  
在.bashrc最后加入：  
export PATH=$PATH:$HOME/android-sdk-linux/platform-tools  
export PATH=$PATH:$HOME/android-sdk-linux/build-tools/23.0.2  
然后解决虚拟机 adb 连接设备的问题  
首先保证USB插到物理机上的时候，能被VMWare截获。  
然后在虚拟机上用 lsusb 命令看一下，能不能看到设备  
能看到的话，直接adb就能调试了  
这下就能在虚拟机上搞CTS了 
 
##杂项
###在子线程里更新UI
* 参考day28_Android应用开发-网络编程  
* 关键点：子线程要拿到主线程（即UI线程）的handler，才能给主线程发消息：handler.sendMessage()  
* 一般来说子线程是UI线程的内部类，内部类可以通过类名直接访问外部类。
###Manifest及布局文件里的相对路径
* Manifest里注册组件，可以写相对路径也可以写全路径。相对路径相对的是<manifest>的package属性  
* 全路径就是包名.类名。比如<Activity>的 android:name="com.zsl.justtest.MainActivity"  
* 相对路径要保证<manifest>的package属性和你的Activity所在的包一样，不然运行时会崩溃。  
* 比如<manifest>的package="com.zsl.justtest"，然后你的Activity也在这个包里，则可以写android:name=".MainActivity"  
* 布局文件里的View也是一样。  
* 如果你使用Android系统提供的View，则只写类名即可。比如 <TextView\> 
* 但如果要在布局文件里使用自己写的类，则要写全路径。比如<com.zsl.justtest.MyTextView>  
###TextView的singleLine 和 lines 属性
* 可以指定这个TextView只显示单行或者几行的文本，如果文本过长了，则会自动转换成...
###两个SDCard的手机，getExternalStorageDirectory()
* Environment.getExternalStorageDirectory() 只能得到第一个外部存储的路径：  
* Return the primary external storage directory.   
* 现在的手机出场时都内置了SD卡，所以这个函数返回的都是内置存储的路径。  
* 如果有两个SD卡，即又插了一块存储卡，则想得到其路径比较麻烦  。
* 不过貌似现在的手机厂商在出厂时，都做了工作，让 Environment.getExternalStorageDirectory() 得到的是后插上去的SD卡路径。  
* 这种能插SD卡同时又有内置SD卡的手机，应该可以在setting里设置那个卡是主存储路径。  
* 如果非要在程序里把两个SD卡路径都找出来，还得再google一下。  