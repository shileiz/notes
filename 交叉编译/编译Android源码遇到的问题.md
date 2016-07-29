先 source build/envsetup.sh  
再 lunch 

=======

* 问题1：  

	    Out of memory error
	    GC overhead limit exceeded.
	    Try increasing heap size with java option '-Xmx<size>'.


* 解决方法：
  
		export JACK_SERVER_VM_ARGUMENTS="-Dfile.encoding=UTF-8 -XX:+TieredCompilation -Xmx4g"  
		./prebuilts/sdk/tools/jack-admin kill-server  
		./prebuilts/sdk/tools/jack-admin start-server  

		或者修改文件 /prebuilts/sdk/tools/jack-admin  
		把 JACK_SERVER_VM_ARGUMENTS 最后加上 -Xmx4g 

* 问题2：

		mediaserver依赖于external里的librmhdplayer.so，而rmhdplayer的Android.mk里设定了 32bitonly
		导致 mediaserver编译时报错

* 解决办法：去掉 rmhdplayer 里的 32bitonly

* 问题3:  
在修改了framework里的东西集成了rmhd之后，再次 make systemimage 报错如下

		ninja: error: 'out/target/product/flounder/obj/SHARED_LIBRARIES/librmhdplayer_intermediates/export_includes', needed by 'out/target/product/flounder/obj/SHARED_LIBRARIES/libmedia_jni_intermediates/import_includes', missing and no known rule to make it

* 解决方法，创建缺少的文件即可：

		mkdir -p out/target/product/flounder/obj/SHARED_LIBRARIES/librmhdplayer_intermediates/
		touch out/target/product/flounder/obj/SHARED_LIBRARIES/librmhdplayer_intermediates/export_includes

* 问题4：

		ninja: error: 'out/target/product/flounder/obj/lib/librmhdplayer.so.toc', needed by 'out/target/product/flounder/obj/SHARED_LIBRARIES/libmedia_jni_intermediates/LINKED/libmedia_jni.so', missing and no known rule to make it

* 解决方法：

		去原来好使的Android源码目录下，搜索：
		find out/target/product/flounder -name librmhdplayer.so.toc
		把找到的 librmhdplayer.so.toc 都拷贝到现在要 build 的Android源码的对应目录下
		再搜 librmhdplayer.so：
		find out/target/product/flounder -name librmhdplayer.so
		把找到的都考过去

		