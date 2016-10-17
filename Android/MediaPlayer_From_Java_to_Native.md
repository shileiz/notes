# 当你在 Java 层 new 了一个 MediaPlayer，到底发生了什么？


> MediaPlayer 类的定义在  `frameworks/base/media/java/android/media/MediaPlayer.java`


###一. 静态语句块	
* 类体里，进来先是一个静态语句块（当这个类的第一个实例生成的时候，会自动调用）: 
  	
		static {
	        System.loadLibrary("media_jni");
	        native_init();
	    }
  
* `System.loadLibrary("media_jni");` 用来加载 native 库 `libmedia_jni.so`，没什么好说的
* `native_init()` 是JNI层的函数（`private static native final void native_init();`），由C/C++实现。
* JNI层的实现如下（省略部分代码）：（位于 `frameworks/base/media/jni/android_media_MediaPlayer.cpp`） 

		static void
		android_media_MediaPlayer_native_init(JNIEnv *env)
		{
		    jclass clazz;		
		    clazz = env->FindClass("android/media/MediaPlayer");		
		    fields.context = env->GetFieldID(clazz, "mNativeContext", "J");		
		    fields.post_event = env->GetStaticMethodID(clazz, "postEventFromNative", "(Ljava/lang/Object;IIILjava/lang/Object;)V");
		    fields.surface_texture = env->GetFieldID(clazz, "mNativeSurfaceTexture", "J");
		    env->DeleteLocalRef(clazz);
			...	
		    gPlaybackParamsFields.init(env);
		    gSyncParamsFields.init(env);
		}

* `native_init()` 主要是从 Java 层得到了：
	* 2个 MediaPlayer 的成员变量：mNativeContext、mNativeSurfaceTexture。——Java层设计这2个成员变量就是用来跟 JNI 层交互的。
	* 1个 MediaPlayer 的方法： `private static void postEventFromNative(Object mediaplayer_ref, int what, int arg1, int arg2, Object obj)` 。这个方法有意思，我们后续会详细分析。
	* 另外，还从另外一个 Java 类 android.net.ProxyInfo 里面得到了一些方法，具体干什么用的还没研究。
* **注意**：
	* 这里JNI层获取的，都是“类”的成员变量及方法，而不是“对象”的。
	* 比如 `fields.context = env->GetFieldID(clazz, "mNativeContext", "J");` 是把 MediaPlayer 这个**类**的成员变量 mNativeContext 保存到了 JNI 层的 fields.context 里。而不是把某一个 MediaPlayer 实例的 mNativeContext 保存下来。
	* **类**的东西都是**静态**的，JNI 层 init 的时候，只是把必要的 Java 层和**类**相关的东西保存起来，此时Java层还没 new 任何对象呢。
* `native_init()` 使用了全局变量 fields 来保存从 Java 层得到的这些成员变量和方法。
* 最后`native_init()` 又调了两个函数： 
	* `gPlaybackParamsFields.init(env)`
	* `gSyncParamsFields.init(env)`

##### 1） gPlaybackParamsFields.init(env)
* gPlaybackParamsFields 是一个 `PlaybackParams::fields_t` 类型的全局变量（是static的全局变量，仅在本源文件范围可见），定义： `static PlaybackParams::fields_t gPlaybackParamsFields;`
* 转去看 PlaybackParams（`frameworks/base/media/jni/android_media_PlaybackParams.h`）


		:::C++
		// PlaybackParams::fields_t，省略部分代码
		struct fields_t {
			jclass      clazz;
	        jmethodID   constructID;
			jfieldID    speed;
			jfieldID    set;
			jfieldID    audio_fallback_mode;
			jint        set_speed;
	        void init(JNIEnv *env) {
	            jclass lclazz = env->FindClass("android/media/PlaybackParams");
	            clazz = (jclass)env->NewGlobalRef(lclazz);
	            constructID = env->GetMethodID(clazz, "<init>", "()V");
	            speed = env->GetFieldID(clazz, "mSpeed", "F");
	            audio_fallback_mode = env->GetFieldID(clazz, "mAudioFallbackMode", "I");
	            set = env->GetFieldID(clazz, "mSet", "I");
	            set_speed = env->GetStaticIntField(clazz, env->GetStaticFieldID(clazz, "SET_SPEED", "I"));
	            env->DeleteLocalRef(lclazz);
	        }
		}

* 可以看到，它是从 Java 层的类 PlaybackParams （android.media.PlaybackParams） 里获取了一些成员变量和方法，保存在了 gPlaybackParamsFields 里。
* 这里有一个地方需要重点看一下：

        jclass lclazz = env->FindClass("android/media/PlaybackParams");
        clazz = (jclass)env->NewGlobalRef(lclazz);
		...
		env->DeleteLocalRef(lclazz);

* 这段代码从一个 lclazz 生成了一个 NewGlobalRef，保存在了结构体 fields_t 的成员变量 clazz 里。 然后 DeleteLocalRef 了 lclazz。
* 什么是 GlobalRef 和 LocalRef 呢？ 我们下面来看一下。

#####Global and Local References
* 参考1： [http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/design.html#wp16785
](http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/design.html#wp16785)
* 参考2： [http://journals.ecs.soton.ac.uk/java/tutorial/native1.1/implementing/refs.html](http://journals.ecs.soton.ac.uk/java/tutorial/native1.1/implementing/refs.html)
* 当 native 函数里拿到一个 java 对象之后，JVM 要记住这件事，以免 native 函数还没使用完该对象，就被 GC 给回收了。 
* 比如上面代码里，`jclass lclazz = env->FindClass("android/media/PlaybackParams");` 这个 lclazz 就是一个 java 层的**类对象**。 在 native 函数使用完这个类对象之前，JVM 不应该把这个类从虚拟机里 unload 掉。

#####Local References
* 所以，默认的，native 函数里得到的任何 java 对象的引用（包括 JVM 传进来的，和 native 代码自己从 env->xxx 获取的），在 native 函数退出之前都不会被GC。而当 native 函数退出之后，就可以GC了。这种引用，就叫 Local Reference。
	* 因为 Local Reference 会在 native 函数退出之后被自动释放，不需要 native 程序员管理，所以大多数时候，native 程序员不去手动释放 Local Reference，交给 JVM 去管理。 不过 JNI 标准还是提供了接口，让程序员手动释放掉 LocalReference： `env->DeleteLocalRef(local_ref)`
	* 官方文档里列举了两种情况有必要手动释放 LocalReference 的，1. 用完了该对象之后，native 函数还要做大量计算才能退出。 2. native 函数里有个大循环，每次循环要引用一个比较大的 java 对象。
	* 总之，在你确定了你不再需要使用该对象时，手动释放掉也是个好习惯。 比如上面 google 的代码里，就手动释放了 lclazz。

#####Global References
* 如果 native 层想一直持有一个 Java 对象，即便在 native 函数 return 之后，还能使用该对象，则需要用Global Reference。
	* Global Reference 只有在 native 程序员显示的释放之后，才可能被 JVM GC 掉。
	* JNI标准规定，可以用一个 Local Reference 生成一个 Global Reference。 `env->NewGlobalRef(local_ref)` 返回一个 Global Reference。
* 什么样的 java 对象需要在 native 函数退出之后还要被 native 函数持有呢？最常见的就是**类对象**。
* 比如上面 google 的代码，native 为了把 java 层PlaybackParams类的类对象保存下来，就建了一个 Global Reference。

##### 2）gSyncParamsFields.init(env)
* gSyncParamsFields 是一个 `static SyncParams::fields_t` 类型的全局变量
* 转去看 static SyncParams （`frameworks/base/media/jni/android_media_SyncParams.h .cpp`），这里就不贴代码了。
* 跟 gPlaybackParamsFields 类似，它是把 Java 层的 android.media.SyncParams 类以及类的各种成员保存在了自己对应的成员里。

####小结
* 到目前为止，都是一些静态的东西，在 Java 层刚要创建第一个 MediaPlayer 实例的时候，JNI 层需要把 Java 层的几个必要的类跟自己结构体对应起来。
* 对于 MediaPlayer 这个 Java 类，JNI 层直接在 `libmedia_jni.so` 这个库里搞了个全局变量 fields，来存储其需要 JNI 层持有的成员变量和方法，包括 mNativeContext、mNativeSurfaceTexture。另外，C++ 也有一个 MediaPlayer 类，不过那不是在 JNI 层实现的，是在 native 层，我们后面会看到。
* 对于 PlaybackParams 和 SyncParams 这两个类，JNI 层也封装了对应的结构体，`struct PlaybackParams` 和 `struct SyncParams`，并在他们内部又封装了个结构体 `struct fields_t` 用来保存Java层对应的类成员和方法。

###二. 构造函数
* MediaPlayer 的构造函数如下：

	    public MediaPlayer() {
			...

	        /* Native setup requires a weak reference to our object.
	         * It's easier to create it here than in C++.
	         */
	        native_setup(new WeakReference<MediaPlayer>(this));
	    }

* 调用了 native 函数 `native_setup()`： `private native final void native_setup(Object mediaplayer_this);`
* 我们看到，这个 native 函数要 java 传一个 WeakReference<MediaPlayer> 的对象下去。在分析这个函数都干了什么之前，我们看看这个参数干嘛的。

####WeakReference
* 参考： [http://blog.sina.com.cn/s/blog_4a4f9fb50100u908.html](http://blog.sina.com.cn/s/blog_4a4f9fb50100u908.html)
* WeakReference的作用：在需要取得某对象的信息，但又不想影响此对象的垃圾收集时，一般使用 WeakReferenc。所以我们的 native 函数可能需要一个本 MediaPlayer 实例的引用，但又不想因为这个引用而耽误垃圾回收。
* 问题：当调用 native 函数的时候，JVM自动会给该函数传进去前两个参数：`JNIEnv* env, jobject thiz`，这里没的 jobject 不就是对调用者的引用么？而且是强引用。如果 native 想获取关于对象的信息，用它就可以了啊，为何还要多此一举的再让 Java 层多传一个参数下来呢？
* 答案：native 给 Java 发 notify 需要用到，具体后面会分析。

####native_setup 实现
 
* JNI 层对应的实现如下（位于：`frameworks/base/media/jni/android_media_MediaPlayer.cpp`）

		static void
		android_media_MediaPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this)
		{
		    ALOGV("native_setup");
		    sp<MediaPlayer> mp = new MediaPlayer();
		    if (mp == NULL) {
		        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
		        return;
		    }
		
		    // create new listener and give it to MediaPlayer
		    sp<JNIMediaPlayerListener> listener = new JNIMediaPlayerListener(env, thiz, weak_this);
		    mp->setListener(listener);
		
		    // Stow our new C++ MediaPlayer in an opaque field in the Java object.
		    setMediaPlayer(env, thiz, mp);
		}

#####1.new 了一个 C++ 的 MediaPlayer
* C++ 的 MediaPlayer 类定义在这里： `frameworks/av/include/media/mediaplayer.h`，实现这这里: `frameworks/av/media/libmedia/mediaplayer.cpp`
* C++ 的 MediaPlayer 只不过是个壳（所以Java 的MediaPlayer也只是个壳），它里面有一个成员变量 `sp<IMediaPlayer>  mPlayer;`，我猜这才是真正干活的 player，它可能是 NuPlayer 也可能是其他的 Player。
* 这一步没什么实际东西，只需要记住，C++ 的 MediaPlayer 类继承自 BnMediaPlayerClient 和 IMediaDeathNotifier

		class MediaPlayer : public BnMediaPlayerClient,
		                    public virtual IMediaDeathNotifier
		{...}

* 注意 C++ 的 MediaPlayer 类是 native 层的东西，它完全不知道有 Java 层的存在。

#####2.new 了一个 JNIMediaPlayerListener，set 给了 mp
* JNIMediaPlayerListener 类在 android_media_MediaPlayer.cpp 里定义并实现。这是一个 JNI 层的类，所以写在 JNI 层源文件里。
* 它继承自 MediaPlayerListener，这是一个 native 层的类。这里就体现出了 JNI 层的意义： 他连接了 native 层和 Java 层。
* 我们要开始分析前面留下的坑了，Java 层的 postEventFromNative() 函数以及 WeakReference 的使用。

#####postEventFromNative()、WeakReference、 Global Reference
* 先看一下 JNIMediaPlayerListener 的构造函数:

		JNIMediaPlayerListener::JNIMediaPlayerListener(JNIEnv* env, jobject thiz, jobject weak_thiz)
		{
		
		    // Hold onto the MediaPlayer class for use in calling the static method
		    // that posts events to the application thread.
		    jclass clazz = env->GetObjectClass(thiz);
		    if (clazz == NULL) {
		        ALOGE("Can't find android/media/MediaPlayer");
		        jniThrowException(env, "java/lang/Exception", NULL);
		        return;
		    }
		    mClass = (jclass)env->NewGlobalRef(clazz);
		
		    // We use a weak reference so the MediaPlayer object can be garbage collected.
		    // The reference is only used as a proxy for callbacks.
		    mObject  = env->NewGlobalRef(weak_thiz);
		}

* 首先，把 java 类 MediaPlayer 得到，并转成了 Global Reference，存储到成员变量 mClass 中。 保存 MediaPlayer 这个类是因为 JNIMediaPlayerListener 需要调用它的静态函数 postEventFromNative() 给它的实例发消息。
* 然后从 weak_thiz 生成了一个 Global Reference，存储到成员变量 mObject 里。这里之所以没有用 thiz 来做 Global Reference，而是让 java 层额外传一个 weak reference 下来来做 Global Reference，是因为不能让某个 Java 层的 MediaPlayer 对象因为在 native 层有一个 Listener 就不会被 GC。
* JNIMediaPlayerListener 在给 java 层发 notify 时，调用的是自己的 `notify(int msg, int ext1, int ext2, const Parcel *obj)` 。 它真正发消息的代码是：`env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, jParcel);`。 即调用了 java 层 MediaPlayer 的静态方法  postEventFromNative()，把消息发给了 mObject。 如果这时候 mObject 对应的 java 层对象已经被GC了怎么办？ 没事，postEventFromNative() 里做了处理了，只会安静的返回，因为这是正常现象。

#####mp->setListener(listener)
* JNIMediaPlayerListener new 好了之后，就 mp->setListener(listener)，把它 set 给了 C++ 的 MediaPlayer 实例。
* 在 new JNIMediaPlayerListener 的时候，我们通过 weak_thiz ，让这个 listener 和 java 层的 MediaPlayer 对象对应了起来。
* 现在又把这个 listener set 给了 C++ 的 MediaPlayer 实例 mp。
* 这样，当底层的 mp 想给 Java 层的 MediaPlayer 发消息的时候，只要调用这个 listener 的 notify() 就可以了。
* 通过上面的分析我们知道，就算底层 mp 发消息的时候，java 层的 mp 对象已经不存在了也没关系。


#####3.setMediaPlayer(env, thiz, mp);
* 这个函数的实现：

		static sp<MediaPlayer> setMediaPlayer(JNIEnv* env, jobject thiz, const sp<MediaPlayer>& player)
		{
		    Mutex::Autolock l(sLock);
		    sp<MediaPlayer> old = (MediaPlayer*)env->GetLongField(thiz, fields.context);
		    if (player.get()) {
		        player->incStrong((void*)setMediaPlayer);
		    }
		    if (old != 0) {
		        old->decStrong((void*)setMediaPlayer);
		    }
		    env->SetLongField(thiz, fields.context, (jlong)player.get());
		    return old;
		}

* 除了引用计数的更改，实际上做的事儿就是把 java mp 对象的 mNativeContext 成员设置成了 C++ 的 mp
* 注：player.get() 中的 get() 是类 sp 的方法，不用太纠结这个

##总结
* 静态语句块：如果是本进程第一次 new MediaPlayer，则会把 native 库 load 进来，并且进行一下 `native_init()`
	* `native_init()` 就 native 层把需要的 Java 类都保存起来，并且做成 Global Reference 
* 构造函数：会在 C++ 层也 new 一个 MediaPlayer，并把 java 的 mp.mNativeContext 设置成 C++ 的这个 MediaPlayer。并且做 C++ 层 new 了一个 JNIMediaPlayerListener，用于底层给 java 层发消息。

###再看层次的划分
* Java 层、JNI层的代码都在 `frameworks/base/...` 里
* C++ 的 MediaPlayer 就不属于 JNI 层了，而是纯的 Native 层，所以它相关的代码都在 `frameworks/av/...` 里