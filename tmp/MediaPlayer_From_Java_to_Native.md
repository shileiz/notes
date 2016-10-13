# 当你在 Java 层 new 了一个 MediaPlayer，到底发生了什么？

### MediaPlayer


> MediaPlayer 类的定义在  `frameworks/base/media/java/android/media/MediaPlayer.java`
	
* 类体里，进来先是一个静态语句块（当这个类的第一个实例生成的时候，会自动调用）: 
  	
		static {
	        System.loadLibrary("media_jni");
	        native_init();
	    }
  
* `System.loadLibrary("media_jni");` 用来加载 native 库 `libmedia_jni.so`，没什么好说的
* `native_init()` 是JNI层的函数（`private static native final void native_init();`），由C/C++实现（ 位于： `frameworks/base/media/jni/android_media_MediaPlayer.cpp` ）。
* `native_init()` 主要是从 Java 层得到了：
	* 2个 MediaPlayer 的成员变量：mNativeContext、mNativeSurfaceTexture。——Java层设计这2个成员变量就是用来跟 JNI 层交互的。
	* 1个 MediaPlayer 的方法： `private static void postEventFromNative(Object mediaplayer_ref, int what, int arg1, int arg2, Object obj)` 
	* 另外，还从另外一个 Java 类 android.net.ProxyInfo 里面得到了一些方法，具体干什么用的还没研究。不过注意，
* `native_init()` 使用了全局变量 fields 来保存从 Java 层得到的这些成员变量和方法。
* `native_init()` 获得完Java层的成员后，又调了两个函数： `gPlaybackParamsFields.init(env); gSyncParamsFields.init(env);`

#### gPlaybackParamsFields.init(env)
* 定义： `static PlaybackParams::fields_t gPlaybackParamsFields;`，转去看 PlaybackParams


		:::C++
		// PlaybackParams::fields_t::init()
        void init(JNIEnv *env) {
            jclass lclazz = env->FindClass("android/media/PlaybackParams");
            if (lclazz == NULL) {
                return;
            }

            clazz = (jclass)env->NewGlobalRef(lclazz);
            if (clazz == NULL) {
                return;
            }

            constructID = env->GetMethodID(clazz, "<init>", "()V");

            speed = env->GetFieldID(clazz, "mSpeed", "F");
            pitch = env->GetFieldID(clazz, "mPitch", "F");
            audio_fallback_mode = env->GetFieldID(clazz, "mAudioFallbackMode", "I");
            audio_stretch_mode = env->GetFieldID(clazz, "mAudioStretchMode", "I");
            set = env->GetFieldID(clazz, "mSet", "I");

            set_speed =
                env->GetStaticIntField(clazz, env->GetStaticFieldID(clazz, "SET_SPEED", "I"));
            set_pitch =
                env->GetStaticIntField(clazz, env->GetStaticFieldID(clazz, "SET_PITCH", "I"));
            set_audio_fallback_mode = env->GetStaticIntField(
                    clazz, env->GetStaticFieldID(clazz, "SET_AUDIO_FALLBACK_MODE", "I"));
            set_audio_stretch_mode = env->GetStaticIntField(
                    clazz, env->GetStaticFieldID(clazz, "SET_AUDIO_STRETCH_MODE", "I"));

            env->DeleteLocalRef(lclazz);
        }

#### gSyncParamsFields.init(env)

### `libmedia_jni.so` 这个库由哪些CPP源文件编译生成？