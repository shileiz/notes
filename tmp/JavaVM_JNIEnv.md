###JavaVM：
* 这个代表java的虚拟机。所有的工作都是从获取虚拟机的接口开始的。

#####第一种方式
* 在加载 Native 库的时候，JVM 会去被加载的库中寻找函数`JNI_OnLoad(JavaVM* jvm, void* reserved)`，如果找到了就调用它。
* 注意这个函数的第一个参数是 JavaVM* 类型的。而普通的 Native 函数的第一个参数都是 JNIEnv* 类型的。这几乎是整个 Native so 库唯一一次获得 JVM 指针的机会。
* 所以一般正经的 Native Code 的 jni 层，都会实现 `JNI_OnLoad()` 函数，并且在函数里用全局变量把 JavaVM* 保存下来，以便以后使用。

#####第二种方式
* 在native code中调用JNI_CreateJavaVM(&jvm, (void)&env, &vm_args)可以得到JavaVM指针。

* 两种情况下，都可以用全局变量，比如JavaVM* g_jvm来保存获得的指针以便在任意上下文中使用。
* Android系统是利用第二种方式Invocation interface来创建JVM的。

---
* 可以不准确的理解为：一个 Android app 就是一个 Android Linux 上的进程
* 在Android里，可以简单的理解为：一个进程对应一个Dalvik虚拟机。Java的dex字节码和c/c++的so库同时运行Dalvik虚拟机之内，共同使用一个进程空间。
* Dalvik 虚拟机当然已经实现了JNI标准，所以在Dalvik虚拟机加载so库时，会先调用JNI_Onload()

当java 代码需要c/c++代码时，,此时就会把JAVA VM对象的指针存储于c层jni组件的全局环境中，在Java层调用C层的本地函数时，调用c本地函数的线程必然通过Dalvik虚拟机来调用c层的本地函数，此时，Dalvik虚拟机会为本地的C组件实例化一个JNIEnv指针，该指针指向Dalvik虚拟机的具体的函数列表，当JNI的c组件调用Java层的方法或者属性时，需要通过JNIEnv指针来进行调用。  当本地c/c++想获得当前线程所要使用的JNIEnv时，可以使用Dalvik虚拟机对象的JavaVM* jvm->GetEnv()返回当前线程所在的JNIEnv*。

---

（1）JNIEnv*内部包含一个Pointer,Pointer指向Dalvik的Java VM对象的Function Table,JNIEnv*关于程序执行环境的众多函数正是来源于Dalvik虚拟机;
（2）Android中每当一个Java线程第一次要调用本地C/C++代码时，Dalvik虚拟机实例会为该Java线程产生一个JNIEnv*指针;
（3）Java每条线程在和C/C++相互调用时，JNIEnv*是相互独立的，互不干扰，这就提升了并发执行时的安全性;
（4）当本地的C/C++代码想获得当前线程所想要使用的JNIEnv时，可以使用Dalvik VM对象的JavaVM* jvm->GetEnv()方法,该方法即会返回当前线程所在的JNIEnv*。
（5）Java的dex字节码和C/C++的*.so同时运行Dalvik VM之内，共同使用一个进程空间
当Java代码需要C/C++代码时，在Dalvik虚拟机加载进*.so库时，会先调用JNI_OnLoad()函数，此时就会把JavaVM对象的指针存储于C层JNI组件的全局环境中，在Java层调用C层的本地函数时，调用C本地函数的线程必然通过Dalvik VM来
调用C层的本地函数，此时Dalvik虚拟机会为本地的C组件实例化一个JNIEnv指针，该指针指向Dalvik虚拟机的具体的函数列表，当JNI的C组件调用Java层的方法或者属性时，需要通过JNIEnv指针来进行调用。
当C++组件主动调用Java的方法或者属性时，需要通过JNI的C组件把JNIEnv指针传递给C++组件，此后，C++组件即可通过JNIEnv指针来掌控Java层代码。
