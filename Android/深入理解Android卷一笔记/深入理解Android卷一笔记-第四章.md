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

## 第四章：深入理解 zygote 

### 4.1 概述

* 我们在编写 Activity、Service 的时候很少接触到“进程”这一概念： 这是 google 有意为之。

### 4.2 zygote 分析

* zygote 是一个 native 的应用程序，跟内核、驱动无关。
* zygote 进程的 main 函数在 App_main.cpp 里。这个文件里还定义了一个类：AppRuntime（继承自 AndroidRuntime）
* zygote 在 main 函数里，通过 AppRuntime 的 start 函数干了几件事儿： 
	* 创建Java虚拟机： `startVM(JavaVM** pJavaVM, JNIEnv** pENv)` 
		* 两个参数是用来带出返回值的
		* 把创建出来的虚拟机的两个代表：JavaVM 和 JINEnv 通过参数带出来
		* 既然得到了代表这个虚拟机的 JINEnv，我们就可以操作Java层的东西了
		* 比如动态注册 native 方法等 
	* 注册JNI函数：startReg
* 注意： 是 Native 层创建了Java虚拟机，在创建的同时，就得到了这个虚拟机的重要信息：JavaVM、JINEnv 