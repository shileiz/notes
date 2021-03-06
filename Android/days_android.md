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


### 用 ant 构建 Android 工程

#### 1. 生成 build.xml
* 首先要用 `android update project -p .` 来生成 build.xml， ant 需要这个 xml
* 其中 android 命令是 SDK 的一部分，需要把 SDK 的 tools 加入 PATH 才能使用
* -p 参数指定 project，可以先 cd 到工程目录，即 AndroidManifest.xml 所在目录，再以 . 为 -p 参数的值。

#### 2. 安装 ant
* 绿色安装，下载加入 path 即可
* 下载地址：[http://ant.apache.org/bindownload.cgi](http://ant.apache.org/bindownload.cgi)

#### 3. 运行 ant 进行 build
* 用 ant 编译一个 debug 版的 apk：`ant debug`
* 成功后会在 bin 目录下生成 XXX-debug.apk 和 XXX-debug-unaligned.apk 两个 apk，安装 debug 那个即可。

#### 4. 出现 invalid resource directory name: xxxx\xxx\res crunch 错误
* 删除 bin\res\crunch 目录，重新 `ant debug` 即可

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