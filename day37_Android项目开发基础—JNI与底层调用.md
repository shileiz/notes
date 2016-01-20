##NDK与Eclipse
* NDK下载解压后可以直接使用，主要用来编译C语言源文件生成.so库。方法是：在.c文件所在目录执行  
	
		ndk-build.cmd
* 该命令会根据当前文件夹下的 Android.mk，Application.mk，编译当前目录中的.c文件，并生成相应的.so，并自动放到对应的目录里去.
* 即工程文件夹下的libs下的armeabi-v7a，x86等文件夹，取决于你在Application.mk里配了哪几种处理器
* NDK大多数时候是配合Eclipse一起使用，即下载之后在Eclipse里配置一下，以便Eclipse自动调用NDK的各种命令,以及自动补齐代码和Ctrl跳转。
* 配置方法：Window--->Preference--->Android--->NDK，把NDK解压后的路径填进去
* 使用方法：Android Tools--->Add Native Support可以新建一个lib。部署Android项目时，会自动编译出so文件。
* 注意，一定要用 Add Natibe Support 的项目才会在部署时自动编译c语言源文件。
* 注意，一定要用 Add Natibe Support 的项目才会在右键项目属性中出现 C/C++ 选项
* 右键项目属性--->C/C++ General ---> Path and Symbols，可以把ndk/platforms/android-xx/arch-arm/usr/inlude添加到Includes里面，这样就可以ctrl跳转到jni.h了
* 注意，Eclipse关联了NDK之后，仍然无法自动调用javah命令来生成头文件，还是要自己手动执行
##Eclipse没有NDK配置项的问题
* 问题现象：  
  - Window--->Preference--->Android下面没有NDK选项
  - 项目上右键--->Android Tools下面没有Add Native Support选项  
* 问题原因：
  - 不是ADT的问题，把ADT升级到最新，仍然存在
  - 是Eclipse版本的问题，Eclipse Luna(4.4)有这个问题，改成比较老的版本Eclipse Juno Service Release 2 就好了
  - Eclipse Luna for Java Developers 也没有这个问题，只有 Luna for J2EE 有这个问题
  - 重新下载的是官方的Eclipse，然后连VPN装的ADT
  - 连VPN装ADT比较慢，这里下载了一个官方的ADT23.06.zip，可以直接用这个装ADT，比连VPN在线装快一点
  - Eclipse Juno和ADT23.0.6.zip都传到了百度云和360云盘中，NDKr10压缩包也传上去了


##使用jni
1. 在项目根目录下创建jni文件夹
2. 在jni文件中创建一个c文件
3. 在java代码中，创建一个本地方法helloFromC

		public native String helloFromC();
4. 在jni中定义函数实现这个方法，函数名必须为

		jstring Java_com_itheima_helloworld1_MainActivity_helloFromC(JNIEnv* env, jobject obj)
5. 返回一个字符串，用c定义一个字符串

		char* cstr = "hello from c";
6. 把c的字符串转换成java的字符串

		jstring jstr = (*env)->NewStringUTF(env, cstr);
		return jstr；
7. 在jni中创建Android.mk文件，这里指定了NDK编译出来的.so叫什么
	
		LOCAL_PATH := $(call my-dir)
	    include $(CLEAR_VARS)
		#编译生成的文件的类库叫什么名字，生成的so文件将叫做libxxx.so
		#xxx就是这里指定的名字，本例是hello，所以本例将生成 libhello.so
	    LOCAL_MODULE    := hello
	    #要编译的c文件
	    LOCAL_SRC_FILES := Hello.c
	    include $(BUILD_SHARED_LIBRARY)
8. 在c文件中添加<jni.h>头文件
	
		#include <jni.h>
9. 在jni文件夹下执行ndk-build.cmd指令
10. java代码中加载so类库，调用本地方法
	
		static{
			System.loadLibrary("hello");
		}
		String s = helloFromC();
		//Do something use s 

##支持AMR的同时支持x86架构
* 在jni中创建Application.mk文件
* 文件中添加一行 APP_ABI := armeabi armeabi-v7a x86
* 或者（NDK>=r7时）直接写 APP_ABI := all

##javah
* 1.7：在src目录下执行javah 包名.类名。注意，是在src目录下，即类似com文件夹所在的目录，而不是在java文件所在的目录。
* 1.6：在bin/classes目录下执行javah 包名.类名。
* javah 的作用是：根据你的java代码，总结出里面需要的C函数，生成相应的.h头文件。这样C程序员只要照着这个头文件去实现里面的函数就行了。
* javah 只起到提供方便的作用，没有javah JNI也能工作。但当你的工程量很大时，使用javah应该是很有用的。
* 不用javah也可以使用JNI，只是方法名容易写错
* javah生成的.h文件可以直接删掉
>Eclipse 可以直接删掉错误，在Problems窗口（位于屏幕下半部分），选中错误并删除，以后Eclipse就不报这个错了。 

* 从javah生成的头文件里把需要实现的函数名复制出来后，删掉那个头文件即可
* 复制过来的函数，参数都没有名字，需要自己加上。
