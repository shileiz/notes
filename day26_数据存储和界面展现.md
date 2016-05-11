---
#常见布局

##线性布局
* 必须有一个布局方向，水平或者竖直

		android:orientation="horizontal"

##### 以下是可以用于线性布局里的元素上的属性
* 指定线性布局里元素对于这个layout的对齐方式。例如跟布局的顶部对齐：

		android:layout_gravity="top"

* 注意：`android:gravity` 指的是元素内的文本和元素本身的对齐方式，跟layout_gravity不是一回事
* 注意：在竖直布局下，顶部对齐（="top"）、底部对齐（="bottom"）、竖直居中（="center_vertical"）不生效。水平布局同理。
* 权重：按比例分配屏幕的剩余宽度或者高度。例如：

		android:layout_weight="1"

* 在使用权重分配宽度/高度的时候，google推荐与0dp配合使用

##相对布局
* 组件默认位置都是左上角，组件之间可以重叠
* 相对布局里的元素，不可以使用“权重”

##### 以下是可以用于相对布局里的元素上的属性
* 相对于父元素（即本相对布局）对齐、居中。例如跟布局的右侧对齐：

		android:layout_alignParentRight="true"
		android:layout_centerInParent="true"  // 注意，在父元素居中不是 aligParentCenter，而是 centerInParent

* 相对于其他组件对齐。例如与xx组件右对齐：

		android:layout_alignRight="@id/xx"

* 放置于其他组件的上方、下方、左边、右边。例如放置于xx组件的右边：

		android:layout_toRightOf="@id/xx"
* 注意，元素内的文字相对于元素本身的对其方式，要用 `android:gravity="center"` 这种

##帧布局
* 用的比较少，仅用于有些API要求布局中必须有一个FrameLayout的情况
* 组件默认位置都是左上角，组件之间可以重叠（类似相对布局）
* 可以设置上下左右对齐，水平竖直居中；不能相对别的组件布局。（类似线性布局）

##表格布局
* 用的比较少
* 每有一个TableRow子节点表示一行，该子节点的每一个子节点都表示一列
* 表格布局中的节点可以不设置宽高，因为设置了也无效
	* 根节点 <TableLayout/\> 的子节点宽为匹配父元素，高为包裹内容
	* <TableRow/\> 节点的子节点宽为包裹内容，高为包裹内容
	* 以上默认属性无法修改
* TableRow的子节点默认宽高都是包裹内容
* 根节点中可以设置以下属性，表示让第1列拉伸填满屏幕宽度的剩余空间

		android:stretchColumns="1"

---
#Logcat

* 别按 application name 来过滤，因为有的 log 这个字段是空，可能找不到想要的
* 按 Tag 来过滤
* 其他没讲啥重要的

---
#Android的存储

###内部存储空间
* Android 要求必须有内部存储，没有内部存储 Android 系统是运行不起来的
* 一个 app 的内部存储空间在： data/data/包名 文件夹下
* 即系统为每一个 app 分配了一个专属的文件夹作为它的内部存储空间
* app 访问内部存储空间（读写）不需要在Manifest里加权限，因为每个 app 是专属空间
* 用于获取内部存储路径的 API 是：

		android.content.ContextWrapper.getFilesDir()  // 得到 data/data/包名/files/
		android.content.ContextWrapper.getCacheDir()  // 得到 data/data/包名/cache/
* app内部存储空间的文件是可以被其他app读写的，只要该文件的权限对其他用户也有rw即可
>顺带讲了 Toast 对话框
>
		Toast.makeText(this, "登录成功", Toast.LENGTH_SHORT).show();

* 复习 Java 基础：第06章\_常用类(File部分) + 第08章\_IO

>Eclipse 快捷键：把局部变量转成成员变量 Ctrl+e 

* 安卓自己的文件读写API

		FileOutputStream android.content.ContextWrapper.openFileOutput(String name, int mode) 
		FileInputStream android.content.ContextWrapper.openFileInput(String name) 
* 返回值也都是 java.io 里的类
* 其中 Output 有第二个参数，能设置生成文件的权限。具体权限是：

		MODE_PRIVATE：-rw-rw----; MODE_APPEND:-rw-rw----; MODEWORLDWRITEABLE:-rw-rw--w-; MODEWORLDREADABLE:-rw-rw-r-- 
* 当然，java API 本身也能设置文件权限，File 类有如下这些方法:

		setExecutable(boolean executable, boolean ownerOnly) 
		setWritable(boolean writable, boolean ownerOnly) 	
		setReadable(boolean readable, boolean ownerOnly) 
* 复习 Linux 权限管理
* 对于路径来说，x权限代表进入该路径的权限。如果一个文件放在了没有x权限的路径下，该文件是不能被读写的。
* 安卓自动为每个app（一个app即一个用户）创建的目录，该目录对所有用户都有x权限（rwxr-x--x）
* 安卓里的文件是手动用API创建的，也就说文件的权限是程序员设置的，但内部路径的权限是安卓系统设定的。
* 由于路径对任何人都有x权限，所以只要文件有rw权限，其他人就可以读写。

>复制一个 Android 工程：  
需要改package name： Manifest 文件的 <manifest> 节点的 package 属性。  
修改package name之后java文件会报错，这是因为类 R 需要重新import一下。import 新包名.R    
需要改application name（显示在图标下面的名字）：strings.xml 里的 app\_name。  
package name  / application name / 代码的package：  
package name 对于一个 app 来说必须是唯一的，用它来识别同一个 app 的不同版本，以确定他们是“同一个 app”。  
package name 在 Manifest 文件的 <manifest> 节点的 package 属性处配置。  
eclipse 自动把代码打包到了 package name 的包里，即MainActivity.java啥的都在 package name 这个包里。  
当然你可以改变你代码的 package，随便改。（但改了以后会因为R.java报错）  
eclipse 自动生成的代码R.java，被打包在了 package name 包里，所以当你改变了MainActivity所在的包之后，要重新import R。  
当你修改了package name之后（在Manifest里改），R.java会自动被eclipse重新放到新的包名里去，所以你原来代码里的R就失效了，要手动重新import。  
application name 是显示在 Play Store 里的名字，还有在 setting 里显示的名字。  
application name 在strings.xml 里的 app_name 处配置。


###外部存储空间
* 原生系统SD卡路径：2.2之前：sdcard; 4.3之前：mnt/sdcard;  4.3开始：storage/sdcard  
* 各个版本用 sdcard 均可访问，因为新版本都做了链接  
* 但各个厂商定制的安卓系统，一般都把外部存储的位置给改了  
* 所以一般都用 android.os.Environment.getExternalStorageDirectory() 来取得外部存储路径
* 另外讲了 sd 卡的 mounted 等状态，没有细看	

>查看Android源码的方法：  
安卓源码不是SDK源码。安卓系统源码虽然也是开源的，但需要单独下载，不随SDK发布。
查看安卓2.3的源码就可以了，业务代码差不多。4.x之后的源码太大了，2.3就已经5G多了。
安卓系统源码下载后，可以看到 dalvik 虚拟机的源码、ndk 的源码等等很多底层的源码。
也可以看到系统自带app的源码。
自带app的源码在：packages/apps/ 里面
查看这些自带 app 的源码，可以用 eclipse 新建一个安卓工程，从已存在代码新建。
虽然这样看源码，eclipse还是会有报错，但已经不影响ctrl+左键跳转了。  

* 所有存储设备，都会被划分成若干个 block，每个block有固定的大小
* 存储设备的总大小 = block大小 x block数量

>提了一下 preference，类似 layout的东西，已经弃用了，不用学了，知道是这么个东西就行，看系统代码看到的时候得认识。  

* Android 获取文件系统状态的 API：Android.os.StatFs 类
* 获取 block 大小和数量等，都通过 new StatFs 对象出来，调用其 getBlockSize()等方法
>Build.VERSION.SDK\_INT 获取当前系统等级（比如18, 19, 21 啥的）
>Build.VERSION\_CODES.xx 用系统名字代表系统等级（比如 JELLY_BEAN 代表4.1,即 18）
>Build 这个类位于 Android.os，这个类就是用来获取当前系统版本信息的

----
#SharedPreference
* SharedPreference 特别适合用于存储零散的数据
* SharedPreference 实质上也是一个文件，是存放于 data/data/包名/shared_prefs/ 下的一个 xml 文件。
* SharedPreference 要比文件读写方便的多

* 往SharedPreference里写数据。以下例子把数据保存在 data/data/包名/shared_prefs/config.xml 中

		//拿到一个SharedPreference对象
		SharedPreferences sp = getSharedPreferences("config", MODE_PRIVATE);
		//拿到编辑器
		Editor ed = sp.edit();
		//写数据
		ed.putString("name", name);
		ed.putBoolean("checked", true);
		ed.commit();

* 从SharedPreference里取数据

		SharedPreferences sp = getSharedPreferences("config", MODE_PRIVATE);
		//从SharedPreference里取数据
		String name = sp.getString("name", "");
* 百分之 95 以上场景，都用 MODE_PRIVATE 模式来使用 SharedPreference