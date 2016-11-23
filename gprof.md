* 参考：[http://blog.csdn.net/stanjiang2010/article/details/5655143](http://blog.csdn.net/stanjiang2010/article/details/5655143)

###用法
* gcc编译时带上-pg参数，运行程序时就会生成gmon.out文件
* 然后用 gprof 命令分析该文件

###具体用法

####ffmpeg
* 网上有说 ffmpeg 的config 选项支持 `--enable-gprof`，不过我再2.8的ffmpeg源码上做了实验，说不认识这个选项
* 只能手动加，ffmepg 在 config 时，加入如下三个选项：

		--extra-cflags=-pg \
		--extra-ldflags=-pg \
		--extra-cxxflags=-pg \

* 在 Windows 上用 MinGW 可以编译出出开了 gprof 的 ffmepg，运行时也能生存 gmon.out
* 不过在用 gprof 分析时报错:

		gprof ffmpeg.exe gmon.out
		C:\msys64\mingw32\bin\gprof.exe: file `ffmpeg.exe' has no symbols

* 先转战 Linux 试一下
* 在 Linux 上出现同样问题：加了三个 -pg 编出来的 ffmpeg 可以在运行的同时输出 gmon.out 文件，但是在用 gprof 进行分析的时候报错：`gprof: file ffmpeg has no symbols`
* 网上搜了一下，是因为 executable 被 strip 了，还好 ffmpeg 的 config 提供了 `--disable-stripping` 选项
* 为了保险起见，又加了 `--enable-debug=3` 选项
* 修改 config 加上上述选项，重新编译


####gcc的-pg参数
* You must use this option when compiling the source files you want data about, and you must also use it when linking.
* 即编译和连接时，都需要-pg参数