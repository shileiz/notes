* 总思路：用NDK编译ffmpeg的源码，得到能在Android上使用的ffmpeg的共享库
* ffmpeg本身提供了用NDK（或其他ToolChain）编译自己的选项：Toolchain options
* 查看ffmpeg所有编译选项的方法： 

		./configure --help
* 我们更改如下的Toolchain options：
	* --arch=arm              
		* //目标架构为arm
	* --cross-prefix=<ndk\>/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86\_64/bin/arm-linux-androideabi-    
		* //编译工具的前缀。比如当需要用到gcc时，就会调用<ndk\>/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86\_64/bin/arm-linux-androideabi-gcc。这里的<ndk\>是指ndk的根目录。
	* --enable-cross-compile   
		* //打开交叉编译
	* --sysroot=<ndk\>/platforms/android-21/arch-arm/             
		* //usr/include和usr/lib文件夹所在的目录
	* --target-os=linux           
		* //目标OS
	* --extra-cflags="-Os -fpic -marm"   
		* //加入几个编译参数
* 除了Toolchain options，我们还要配置如下Standard options：
	* --prefix=PREFIX          
		* //编译出来的文件将被放在这里
* 还有如下的Configuration options:
	* --disable-static         
		* //do not build static libraries 
	* --enable-shared          
		* //build shared libraries
* 把以上需要做的配置做成一个脚本：

		#!/bin/bash
		NDK=/home/zsl/android-ndk-r9d
		SYSROOT=$NDK/platforms/android-9/arch-arm/
		CROSS_PREFIX=$NDK/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64/bin/arm-linux-androideabi-
		
		function do_config()
		{
		./configure \
			--arch=arm \
		    --cross-prefix=$CROSS_PREFIX \
		    --enable-cross-compile \
		    --sysroot=$SYSROOT \
		    --target-os=linux \
		    --extra-cflags="-Os -fpic -marm" \
		    --prefix=/home/zsl/ffmpeg-android \
		    --disable-static \
		    --enable-shared
		}
		do_config
* 运行以上脚本后，就可以编译了：

		make
		make install
* 编译出来的库文件在 /home/zsl/ffmpeg-android/lib 里面


##
编出来的库文件是这种文件名的：libavcodec.so.56.34.100  
NKD不识别这种格式的库文件  
CAL\_SRC\_FILES should point to a file ending with ".so"   
一种方案是直接改文件名，让NDK不报错。即改为libavcodec.so  
这样改了之后，确实app能编译出来并安装到手机了，但运行会crash。 
报的错位：dlopen failed: could not load library "libavcodec.so.56" needed by "libmyff.so";   
因为，我们之前build出来的库文件，其soname是：libavcodec.so.56（real name 是libavcodec.so.56.34.100），你基于它的代码在链接它的时候，链接器会去找名字叫libavcodec.so.56的库文件。  
为了编译出来的soname能被NDK识别，我们在编译ffmpeg之前需要修改configure文件：  
将该文件中的如下四行：  
SLIBNAME\_WITH\_MAJOR='$(SLIBNAME).$(LIBMAJOR)'  
LIB\_INSTALL\_EXTRA\_CMD='$$(RANLIB)"$(LIBDIR)/$(LIBNAME)"'  
SLIB\_INSTALL\_NAME='$(SLIBNAME\_WITH\_VERSION)'  
SLIB\_INSTALL\_LINKS='$(SLIBNAME\_WITH\_MAJOR)$(SLIBNAME)'  
替换为：  
SLIBNAME\_WITH\_MAJOR='$(SLIBPREF)$(FULLNAME)-$(LIBMAJOR)$(SLIBSUF)'  
LIB\_INSTALL\_EXTRA_CMD='$$(RANLIB)"$(LIBDIR)/$(LIBNAME)"'  
SLIB\_INSTALL\_NAME='$(SLIBNAME\_WITH\_MAJOR)'  
SLIB\_INSTALL\_LINKS='$(SLIBNAME)'   
另外，我们如果只用到ffmpeg一小部分功能（比如仅仅解码）的话，可以在config的时候关闭多余的功能，要不然编译一次也挺久的。  
我们修改do\_config.sh，添加如下的一些配置：

		
		--disable-doc \
		--disable-programs \
		--disable-encoders \
		--disable-muxers \
		--disable-filters \
虽然例子程序只用到了libavcodec-56.so，但是，这个库里的函数还依赖于libswresample-1.so，所以这个库也要加载到app里。  
然后libswresample-1.so又依赖于libavutil-54.so，所以还得接着加。  
另外，ffmpeg的所有函数之前必须调用av\_register\_all()。  
而av\_register\_all()在库libavformat中，所以这个库也要写到Android.mk里，并且，这个模块是自己的代码所依赖的，所以这个模块得加到LOCAL\_SHARED\_LIBRARIES这行。  

