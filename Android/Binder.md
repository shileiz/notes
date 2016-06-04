### 概念
	参考：http://blog.csdn.net/boyupeng/article/details/47011383  

* Binder 是用来做进程间通信用的一种机制，进程间通信简称IPC。

Binder 框架定义了四个角色：  
Server，Client，ServiceManager（以后简称SMgr）以及Binder驱动。  其中Server，Client，SMgr运行于用户空间，驱动运行于内核空间。  
这四个角色的关系和互联网类似：Server是服务器，Client是客户终端，SMgr是域名服务器（DNS），驱动是路由器。

### 以MediaService为例
	参考：http://www.cnblogs.com/innost/archive/2011/01/09/1931456.html  
	参考：http://blog.csdn.net/myarrow/article/details/7048488
* 进程 /system/bin/mediaserver ，是一个C++应用程序，运行在Android底层的Linux系统上。
* 我们来分析这个应用程序
* 这个程序的源码如下（Main_mediaserver.cpp）：

		/* framework\base\Media\MediaServer\Main_mediaserver.cpp */
		int main(int argc, char** argv)
		{
			//获得一个ProcessState实例
			sp<ProcessState> proc(ProcessState::self());
			//得到一个ServiceManager对象
		    sp<IServiceManager> sm = defaultServiceManager();
		    MediaPlayerService::instantiate();//初始化MediaPlayerService服务
		    ProcessState::self()->startThreadPool();//看名字，启动Process的线程池？
		    IPCThreadState::self()->joinThreadPool();//将自己加入到刚才的线程池？
		}


* sp<XXX>就看成是XXX*就可以了。  
* sp，究竟是smart pointer还是strong pointer呢？其实我后来发现不用太关注这个，就把它当做一个普通的指针看待，即sp<IServiceManager>======》IServiceManager*吧。sp是google搞出来的为了方便C/C++程序员管理指针的分配和释放的一套方法，类似JAVA的什么WeakReference之类的。我个人觉得，要是自己写程序的话，不用这个东西也成。
* 第一个调用的函数是ProcessState::self()，然后赋值给了proc变量，程序运行完，proc会自动delete内部的内容，所以就自动释放了先前分配的资源。

### Tips
* 阅读 Android 源码时怎么找到 native 方法在哪个cpp文件里
* native函数实现所在的文件的文件名都是如下命名的，把包名中的"."替换为"_"+类名
* 比如： android.media.MediaPlayer 这个类里的 native 方法，就在这个 cpp 文件里：
* `android_media_MediaPlayer.cpp`

### MediaPlayer的Java层怎么走到C++层的
	参考：http://www.cnblogs.com/haiming/archive/2013/03/09/2948730.html  

* new MediaPlayer()  的时候调用了native的 `native_setup(new WeakReference<MediaPlayer>(this))`
	* 这个native方法在`android_media_MediaPlayer.cpp`里实现
	* 这里最后 new 了一个 C++ 的 MediaPlayer
* setDataSource() 的时候调用了native的 `setDataSource(FileDescriptor fd, long offset, long length)`
	* 这才是重点
	* native 的 setDataSource() 里获取了一个 MediaPlayerService（用`getMediaPlayerService()`）
	* 这个过程涉及到Binder进程间通信原理，因为 MediaPlayerService 是另一个进程
	* 关于Binder的部分暂且略去，这里专注MediaPlaer相关的东西
	* 总之 Java 层 setDataSource() 的时候，会走到jin层的 setDataSource()
	* jni层的setDataSource()会搞出一个MediaPlayerService
	* 接下来的更加重要
	* 搞出MediaPlayerService之后，会通过这个service创建一个player，然后调用这个player的setDataSource()
	* 这里的重点是service是另一个进程，player是这个service创建的，即player也是另一个进程的，jni层代码只是持有这个player的引用而已。
	* 可以理解为C层有两个进程，一个是跟Java层打交道的，即native方法所在进程。另一个是service进程，service进程才是真正player所在进程。
	* 接下来还是重点
	* player的 setDataSource()中有一句：
	* 
			player_type playerType = MediaPlayerFactory::getPlayerType(this,fd,offset,length);
	* 这时候会根据fd去选择一种PlayerType，android4.4.2中有以下5种：
	* 
			enum player_type {
			    PV_PLAYER = 1,
			    SONIVOX_PLAYER = 2,
			    STAGEFRIGHT_PLAYER = 3,
			    NU_PLAYER = 4,
			    // Test players are available only in the 'test' and 'eng' builds.
			    // The shared library with the test player is passed passed as an
			    // argument to the 'test:' url in the setDataSource call.
			    TEST_PLAYER = 5,
			};
	* 其中`PV_PLAYER`从2.3开始已经弃用，`SONIVOX_PLAYER`不知道干啥的，`STAGEFRIGHT_PLAYER`是最常见的也是default的。
	* `NU_PLAYER` 可以在网上找一下，应该也有用但不知道什么时候用。
	* `STAGEFRIGHT_PLAYER`底层还会调用AwesomePlayer，如果想深入可以看看AwesomePlayer是怎么玩的
* 总之，如果你的播放器不使用Android SDK提供的MediaPlayer（或者VideoView）接口，那么就不会走到MediaServer底层了