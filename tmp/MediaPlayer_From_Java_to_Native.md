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
	* 1个 MediaPlayer 的方法： `private static void postEventFromNative(Object mediaplayer_ref, int what, int arg1, int arg2, Object obj)` 
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

##### 2）gSyncParamsFields.init(env)
* gSyncParamsFields 是一个 `static SyncParams::fields_t` 类型的全局变量
* 转去看 static SyncParams （`frameworks/base/media/jni/android_media_SyncParams.h .cpp`），这里就不贴代码了。
* 跟 gPlaybackParamsFields 类似，它是把 Java 层的 android.media.SyncParams 类的各种成员保存在了自己对应的成员里。

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

#####1. Java 里的 WeakReference
* 参考： [http://blog.sina.com.cn/s/blog_4a4f9fb50100u908.html](http://blog.sina.com.cn/s/blog_4a4f9fb50100u908.html)
* WeakReference的作用：在需要取得某对象的信息，但又不想影响此对象的垃圾收集时，一般使用 WeakReferenc。所以我们的 native 函数可能需要一个本 MediaPlayer 实例的引用，但又不想因为这个引用而耽误垃圾回收。
* 问题：当调用 native 函数的时候，JVM自动会给该函数传进去前两个参数：`JNIEnv* env, jobject thiz`，这里没的 jobject 不就是对调用者的引用么？而且是强引用。如果 native 想获取关于对象的信息，用它就可以了啊，为何还要多此一举的再让 Java 层多传一个参数下来呢？
* 答案：native 给 Java 发 notify 需要用到，具体后面会分析。

#####2. JNI 的 Global and Local References
* 参考1： [http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/design.html#wp16785
](http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/design.html#wp16785)
* 参考2： [http://journals.ecs.soton.ac.uk/java/tutorial/native1.1/implementing/refs.html](http://journals.ecs.soton.ac.uk/java/tutorial/native1.1/implementing/refs.html)
* 默认，JVM 传给 native 的 Java 对象，都是 Local Reference。当 native 函数退出之后，这个引用就失效了。就不再影响对该 Java 对象的 GC 了。
* 如果 native 层想一直持有一个 Java 对象，即便在 native 函数

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
* JNIMediaPlayerListener 类在 android_media_MediaPlayer.cpp 里定义并实现。这是一个 JNI 层的类，所以写作 JNI 层源文件里。
* 它继承自 MediaPlayerListener，这是一个 native 层的类。这里就体现出了 JNI 层的意义： 他连接了 native 层和 Java 层。
* 

#####3.setMediaPlayer(env, thiz, mp);

###再看层次的划分
* Java 层、JNI层的代码都在 `frameworks/base/...` 里
* C++ 的 MediaPlayer 就不属于 JNI 层了，而是纯的 Native 层，所以它相关的代码都在 `frameworks/av/...` 里

### `libmedia_jni.so` 这个库由哪些CPP源文件编译生成？