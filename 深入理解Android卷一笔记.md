### 层：Java ---> JNI ---> Native
* 分三层
	* 最上面是 Java 层
	* 中间是JIN层，C语言编写，以.so形式存在
	* 最下是Native层，C语言编写，以.so形式存在
* 举例
	* Java（MediaScanner） ---> JIN（libmedia_jin.so） ---> Native（libmedia.so） 

### JNI 层的函数名，不一定非得叫 `Java_包名_类名_函数名`	
* 如果JNI层的函数按照以上规则命名了，那么Java层直接能找到对应的函数
* 如果不按这种规则命名，也有方法，就是动态注册
* 动态注册除了有缩短函数名，不借助javah工具这两个好处
* 还有提升效率的好处。因为非动态注册的函数，在初次被Java层调用时，要去so库里按名字搜索，这样影响效率。

### 动态注册
* 在JNI层的C代码里，调用函数 `(*env)->RegisterNatives(env,clazz,gMethods,numMethods)` 即可注册
* 其中 gMethods 是一个数组，数组里放的都是 JNINativeMethod 类型的变量
* JNINativeMethod 这个结构体如下：
* 
		typedef struct{
			//Java 中native函数名，不用包含路径，比如processFile
			const char* name;
			//Java 函数的签名信息，用字符串表示，是参数类型和返回值类型的组合
			const char* signature;
			//JNI 层对应的函数的指针
			void* fnPtr;
		}JNINativeMethod
* 这个结构体就是把Java层的函数名和JNI层的函数指针对应起来的
* 所以，把实现了的Java层的函数的指针和Java层函数名对应写好，封装到这个结构体里
* 再把所有这样的结构体放到数组gMethods里
* 就可以用RegisterNatives()进行函数注册了
* 因为JNINativeMethod结构体里只写Java里的函数名，不包含路径，所以不知道这个函数是哪个类的
* 所以RegisterNatives()需要一个参数clazz，这是jclass类型的，代表着java类
* 注册过的函数就有了对应关系了，当Java层调用native函数时，直接就能找到JNI层的对应的函数指针了
* JNINativeMethod 这个结构体里之所以需要一个函数签名信息，是因为Java支持函数重载
* 只有把参数返回值都确定了，才能找到对应的到底是哪个Java层函数
### 动态注册的一般流程
* 先把实现了Java层native函数的那些函数的指针都封装到JNINativeMethod数组里
* 
		static JNINativeMethod gMethods[]={
			{
				"processFile",
				"(Ljava/lang/String;Ljava/lang/String;Landroid/media/MediaScannerClient;)V",
				(void*)android_meida_MediaScanner_processFile,
			},
		
			{
				"native_init",
				"()V",
				(vodi*)android_meida_MediaScanner_native_init
			}
		};
* 通过方便函数`AndroidRuntime::registerNativeMethods(env,"android/media/MediaScanner",gMethods,ELEN(gMethods)) `来注册
* 而不是直接使用`(*env)->RegisterNatives(env,clazz,gMethods,numMethods)` 
* 因为方便函数的第二个参数直接传字符串就可以了，不用先通过字符串生成一个jclass类型再传
* 一般把动态注册写在 `JNI_OnLoad(JavaVM* vm, void* reserved) `里
	* 这个函数会在Java层loadLibrary后被调用
	* `libmedia_jni.so`的`JNI_OnLoad`函数是在`android_media_MediaPlayer.cpp`里实现的
### 在JNI层操作Java层的东西 
#### 调用Java层的方法
1. 通过 `jclass clazz = env->FindClass("含有路径的类名");` 找到类
2. 通过 `jmethodID mid = env->GetMethodID(clazz,"方法名","方法签名信息");`找到Java层方法的ID
	* 注意jmethodID是一个专门记录Java层方法的类型
	* 类似的还有一个jfieldID
3. 通过 `env->CallxxxMethod(jobj,mid,param1,param2...);` 调用Java层的方法
	* CallxxxMethod中的xxx是Java方法的返回值类型，比如CallVoidMethod，CallIntMethod
	* 第一个参数是指调用哪个对象的方法，就是Java中`.`前面的那个对象 
	* 第二个参数Java中的MethodID
	* 后面的参数就是Java方法的参数了，其类型都要是java中能处理的类型，比如jstring，jint，jobject
#### get和setJava层的field
1. 通过 `jclass clazz = env->FindClass("含有路径的类名");` 找到类
2. 通过 `jfieldID fid = env->GetFieldID(clazz,"成员名","成员签名信息");`找到Java层成员变量的ID
3. 通过 `GetxxxField(env,obj,fid);` / `SetxxxField(env,obj,fid,value);` 来get/set相应的成员变量
### JNI层的jstring要手动释放，这和jstring内部实现有关
* 用 `char *cString = env->GetStringUTFChars(jstring javaString, NULL)` 能从jstring类型得到C语言的字符串（char*）
* 用 `jstring javaString = env->NewStringUTF(const char* cString)` 能从C语言的字符串得到jstring的类型
* 以上两种方法调用之后，都要调用 `env->ReleaseStringUTFChars(jstring javaString, char* cString) `来释放
* 否则会导致JVM内存泄露