##5.2 RefBase、sp，wp 以及 LightRefBase
* 没有详细看

##5.3 Thread类以及常用的线程同步类
* Thread 类是安卓自己封装的线程类，其底层用的也是 pthread

###5.3.1 一个变量引发的思考
* 这节主要讲了两件事儿：
	1. canCallJava 是如何控制 Thread 能不能调用 JNI 的 （到 `3. 费力而讨好` 为止）
	2. 线程函数 `_threadLoop` （从 `4. 线程函数_threadLoop介绍` 到最后）

####1. canCallJava 是如何控制 Thread 能不能调用 JNI 的
* Thread 类的构造函数有个参数：canCallJava。通过它可以控制新创建出来的线程能不能使用 JNI
* 这部分一路跟踪代码，最后发现，这个变量确实能控制该线程能否使用 JNI。如果 canCallJava 为真，则可以调用 JNI。原因如下：
	* 进入线程函数之前： `javaAttachThread(name, &env)`
	* 线程函数退出之后： `javaDetachThread();`
* 这就产生了如下疑问：
	* 怎么把一个线程 attach 到 JIN 环境，而且还是通过线程名。即 javaAttachThread() 干嘛了？
	* `&env` 看起来应该是 JNIEnv，我们知道它是线程相关的，那调用 javaAttachThread() 时传进去的 env 是从哪里来的呢？即 javaThreadShell() 函数里的 `` 是从哪来的？
	* detach 干的事儿应该很好理解了，如果 attach 干嘛了已经搞懂的话。

#####`javaAttachThread(name, &env)`
* name 是本线程的线程名。也就是本节最开始处，`Thread::run()` 的第一个参数。
* env 是带出参数，把线程 attach 到的那个 JNIEnv 用它带出
* 此函数实际上是调用了本进程 Java VM 的 AttachCurrentThread() 函数。
* AttachCurrentThread() 是 JNI 接口中的 `Invocation API Functions` 之一。详见官方文档：[http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/invocation.html#attach_current_thread](http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/invocation.html#attach_current_thread)
* **总之，canCallJava 为真的线程才能被本进程的 Java VM 看到，否则不能。**

####2. 线程函数 `_threadLoop`
* `Thread::run()` ，无论 canCallJava 是真是假，最后都会创建一个线程（用 `pthread_create` ），该线程的主函数都会是 `_threadLoop`
* 而真正用户写的线程函数是去实现 Thread 的 `threadLoop()` 函数，在 `_threadLoop` 里，`self->threadLoop()` 会放到一个 while 循环里。
* 也就是说，即便你写的线程干活函数 `threadLoop()` 已经干完活了，这个线程也可能不结束。因为外边还包着一层 while 呢。而这个 while 啥时候会跳出呢：
	* 你的  `threadLoop()` 返回 false。这属于主动正常退出。
	* mExitPending为true，这个变量可由Thread类的requestExit函数设置，这种情况属于被动退出，因为由外界强制设置了退出条件。

####小结一下2.
* Android 里用 Thread 类封装了线程而不是直接使用 pthread
* 你想新建线程干活，就得实现一个 Thread 的子类，把干活写在它的 `threadLoop()` 里
* 并且，注意，一定在正常干完活想让线程退出时 return 一个 false。 而不是 true！
