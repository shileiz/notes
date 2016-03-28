### 概念
参考：
http://blog.csdn.net/boyupeng/article/details/47011383  

* Binder 是用来做进程间通信用的一种机制，进程间通信简称IPC。

Binder 框架定义了四个角色：  
Server，Client，ServiceManager（以后简称SMgr）以及Binder驱动。  其中Server，Client，SMgr运行于用户空间，驱动运行于内核空间。  
这四个角色的关系和互联网类似：Server是服务器，Client是客户终端，SMgr是域名服务器（DNS），驱动是路由器。

### 以MediaService为例
* 参考：http://www.cnblogs.com/innost/archive/2011/01/09/1931456.html  
* 参考：http://blog.csdn.net/myarrow/article/details/7048488
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