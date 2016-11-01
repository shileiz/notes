###JNI 的在官方在线文档：


> [http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/jniTOC.html](http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/jniTOC.html)

### JavaVM 是进程相关的，JNIEnv 是线程相关的
* 在 Android 里，每个进程只有一个 JavaVM（DalvikVM） 的实例
* Android中每当一个Java线程第一次要调用本地C/C++代码时，Dalvik虚拟机实例会为该Java线程产生一个JNIEnv*指针;
* Java每条线程在和C/C++相互调用时，JNIEnv*是相互独立的，互不干扰，这就提升了并发执行时的安全性;
* 当本地的C/C++代码想获得当前线程所想要使用的JNIEnv时，可以使用Dalvik VM对象的JavaVM* jvm->GetEnv()方法,该方法即会返回当前线程所在的JNIEnv*。

##从上往下——Java层调用C/C++
* 这是最常见的一种方式。
* 在调用之前，肯定要加载 C/C++ 的库，即 `System.loadLibrary()`
* 在加载库的时候，JVM 会去被加载的库中寻找函数 `JNI_OnLoad(JavaVM* jvm, void* reserved)`，如果找到了就调用它。
* 注意这个函数的第一个参数是 JavaVM* 类型的。而普通的 JNI 函数的第一个参数都是 JNIEnv* 类型的。
* 这几乎是整个 JNI so 库唯一一次获得 JVM 指针的机会。
* 所以一般正经的 Native Code 的 jni 层，都会实现 `JNI_OnLoad()` 函数，并且在函数里用全局变量把 JavaVM* 保存下来，以便以后使用。

####关于Android
* 可以不准确的理解为：一个 Android app 就是一个 Android Linux 上的进程
* 在Android里，可以简单的理解为：一个进程对应一个Dalvik虚拟机。Java的dex字节码和c/c++的so库同时运行这个进程之内。
* Dalvik 虚拟机当然已经实现了JNI标准，所以在Dalvik虚拟机加载so库时，会先调用JNI_Onload()

##从下往上——Native程序主动使用Java：The Invocation API
* 目的： 一个C/C++写的可执行程序，在代码里使用已经编译好的 class 文件里的功能。 
* 参考官方文档第5章：


>http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/invocation.html

* 官方文档里上来就是一个例子，看了这个例子，基本就明白怎么从下往上调了。
* 下面介绍具体怎么把这个例子运行起来。

####1. JAVA Code

	:::java
	public class JavaApp{
	    public static void javaMethod(){
	        System.out.println("I have done a lot of things by Java!");
	    }
	}

####2. 编译 java： 
* `javac JavaApp.java` 直接生成 JavaApp.class 没什么好说的。

####3. CPP Code

	:::C++
	#include <jni.h>       
	int main(){
		JavaVM *jvm;       /* denotes a Java VM */
		JNIEnv *env;       /* pointer to native method interface */
		JavaVMInitArgs vm_args; /* JDK/JRE 6 VM initialization arguments */
		vm_args.version = JNI_VERSION_1_6;
		vm_args.ignoreUnrecognized = false;
		JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);
		jclass cls = env->FindClass("JavaApp");
		jmethodID mid = env->GetStaticMethodID(cls, "javaMethod", "()V");
		env->CallStaticVoidMethod(cls, mid);
		jvm->DestroyJavaVM();
	}

####4. 编译CPP
* 加上-I选项，让gcc能找到 jni.h 和 jni_md.h: `-I/usr/lib/jvm/java-8-openjdk-amd64/include -I/usr/lib/jvm/java-8-openjdk-amd64/include/linux`
* 加上连接器选项，让ld能找到 libjvm.so: `-L/usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/server CppApp.cpp -ljvm`
	* **注意**： gcc 在做链接时会严格的按照从左到右的顺序，如果 -lxxx 这个库的左边并没有任何东西需要它，那么这个 -lxxx 会被忽略。 因为我们 CppApp.cpp 编译出来的 .o 是需要 libjvm.so 的，所以我们的 -ljvm 一定要出现在 CppApp.cpp 的右边！
	* 参考： [http://stackoverflow.com/questions/16860021/undefined-reference-to-jni-createjavavm-linux](http://stackoverflow.com/questions/16860021/undefined-reference-to-jni-createjavavm-linux)
* 最后，完整的编译命令：

		gcc -I/usr/lib/jvm/java-8-openjdk-amd64/include -I/usr/lib/jvm/java-8-openjdk-amd64/include/linux -L/usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/server  -o app CppApp.cpp  -ljvm

####5. 运行
* 直接运行 app 会报错链接不上 libjvm.so 这个库。
* 因为安装完 openJDK8，操作系统并不能找到 libjvm.so。 可以运行 `sudo ldconfig -v | grep jvm` 验证一下，果然啥也没有。
* 我们需要自己配置一下，利用 ldconfig 工具。
* 具体做法是： 在目录 `/etc/ld.so.conf.d/` 里添加一个文件： openJDK.conf，这个文件里写上：`/usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/server` 。 这就是 libjvm.so 所在的目录。 然后运行一下 ldconfig 即可。因为 ldconfig 运行的时候，会去加载目录 `/etc/ld.so.conf.d/` 里的所有 .conf 文件。
* 搞定后直接运行 `./app`，会打印出 Java 层的输出，大功告成。

###6.总结
* 至此，我们已经成功运行了该例子，现在来总结一下Native程序到底是怎么调到java的功能的。
* 想使用java code实现的功能，首先我们得搞一个 JVM 出来，这就是：`JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);`
* 这个函数是在 libjvm.so 里实现的，此函数是 JNI 标准的一部分，所以随JDK发布。
* 这个函数能创建一个 JVM 对象，该对象肯定也是活在我们的 Native 进程里的。
* 这个函数中创建 JVM 的同时，把 JNIEnv 也返回给我们了，即第二个参数。
* 我们拿着 JNIEnv 就可以随便搞了，像获取 Java 类，获取 methodID，调用 java method 的什么的，都不是新鲜事了。

---

### 再看JNI

#### C 和 C++ jni 层函数的区别
* jni层函数的第一个参数都是 JNIEnv* env， 无论C还是C++
* 但是这个 JNIEnv 对于C和C++是不同的，看一下 jni.h 中的定义：

		#if defined(__cplusplus)
		/* 在C++中，JNIEnv 等同于 _JNIEnv，而_JNIEnv是一个结构体，一会儿下面会有代码 */
		typedef _JNIEnv JNIEnv;     
		typedef _JavaVM JavaVM;
		#else
		/* 在C语言中，JNIEnv 是个指向结构体的指针，指向的是 JNINativeInterface 类型的结构体 */
		typedef const struct JNINativeInterface* JNIEnv;   
		typedef const struct JNIInvokeInterface* JavaVM;
		#endif
		
		...

		struct JNINativeInterface {... // 里面定义的全是函数指针 }

		...

		struct _JNIEnv { // 里面定义的全是函数指针 }

* 即，对于C语言，env是一个二级指针，(*env)才是指向了封装了那么多个桥梁函数的结构体的结构体指针
* 对于C++而言，env是一个一级指针，env本身就是指向了封装了那么多个桥梁函数的结构体的结构体指针
* 所以，jni层，如果是.c文件，则内部用 `(*env)->xxxxx` 来引用桥梁函数
* 如果是 .cpp 文件，则内部用 `env->xxxxx` 来引用桥梁函数

##### jni层(C/C++语言)所有函数的第一个参数都是 JNIEnv* 型变量，是指向 JNIEnv 的指针
##### JNIEnv 这个结构体里，全都是函数指针，一共有300个左右。
1. jni 层的函数是被 java 层调用的，所以参数是 java 层传过来的
2. 这第一个参数 JNIEnv* 是 java 虚拟机传的
3. 也就是说是 java 虚拟机初始化了一个 JNIEnv 结构体，把300个函数指针都初始化了。
4. 但函数指针指向的都是C的函数，所以这是Java虚拟机干的事儿。Java虚拟机作为Java世界和C世界的桥梁，这也是体现之处。
5. 当你愉快的调用 (*env)->FindClass(env, "含有路径的类名") 时，虽然这是一个C函数，但它是 java 虚拟机初始化的。