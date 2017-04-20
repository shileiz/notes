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
* 修改 config 加上上述选项，重新编译。这回 gprof 可以正常运行了。
* 不过新问题是，运行显示的结果，只有寥寥几个 ffmpeg.c 里的函数被统计到了，且耗时几乎都为0。真正耗时的编解码函数（libavcodec.so里的）都没有被统计到。
* 这是因为尽管编译时产生的各种 so 库，比如 libavcodec.so，都已经加了 -pg 了，但运行时实际载入内存的库，并不是这个。
* 运行时载入内存的，是机器上本来就安装好的 libavcodec.so（用 ldd ffmpeg 可以看一下），而之前安装的库，是没有加 -pg 的，所以里面的函数跟踪不到。
* 有两个解决办法：
	1. 把新编出来的加了-pg的库放到系统的库目录里，比如/usr/lib/,替换到原来的库
	2. 改用静态库编译 ffmpeg，这样运行ffmpeg时就不会去连接操作系统上原来的动态库了，新编译的带了-pg的二进制函数都被直接放到 ffmpeg 里面了。 改用静态库的配置选项是：`--disable-shared --enable-static`


####gcc的-pg参数
* You must use this option when compiling the source files you want data about, and you must also use it when linking.
* 即编译和连接时，都需要-pg参数

####检查你编出来的库/可执行文件到底有没有成功加入 -pg 选项
* -pg 的原理是： gcc 在你应用程序的每个函数中都加入了一个名为mcount ( or  “_mcount”  , or  “__mcount”)的函数，也就是说你的应用程序里的每一个函数都会调用mcount, 而mcount 会在内存中保存一张函数调用图，并通过函数调用堆栈的形式查找子函数和父函数的地址。这张调用图也保存了所有与函数相关的调用时间，调用次数等等的所有信息。
* 所以，想检查编出来的库文件或可执行文件，-pg 选项到底好使了没有，可以查看它的符号表，搜索一下关键字 mcount：

		nm xxxxxxx | grep mcount

####其他
* gcc 命令加了个 xxxx.exp 导致编出来的动态库文件符号表里并没有 mcount，虽然我也加了 -pg 选项。
* .exp 文件里只写了 VERSION 有关信息： VERSION{....}，为什么我也不知道，但去掉这个就好了。


###关于动态库的支持
* 参考：[http://www.cnblogs.com/lenolix/archive/2010/12/13/1904868.html](http://www.cnblogs.com/lenolix/archive/2010/12/13/1904868.html)
* 生成一个可执行程序的时候，gcc（实际上是ld）会把 libc.a/libc.so 连接入可执行文件，这个库负责初始化程序的栈空间等工作。即 libc 会在你业务代码之前运行。
* 加了 -pg 选项，在连接阶段，gcc 会把本应该链接到可执行文件上的 libc.a/libc.so 里的 crt0.o，改成 gcrt0.o
* gcrt0.o 会在你的main函数之前插入一个函数：`__monstartup`，该函数用来做 profile 相关的初始化工作，比如分配所需的内存
* 该函数只会对程序的 .text 段进行 profile 的初始化。
* 因为动态链接库的代码并不在主程序的 .text 段，所以是不会被 profile 到的。
* 如果想让动态库也能被 profile，只能修改 libc 的代码，这样做不太好。
* 或者按照网上有些说法，用 `libc_p.a/libc_p.so` 代替正常的 `libc.a/libc.so`，即把链接选项 `-lc` 替换为 `-lc_p`。
* 因为 `libc_p` 就是支持了 profile 的 libc
* 不过这么做也有困难，首先不是说有版本的 Linux 上都能安装到 `libc_p` 的，另外在有些系统上，比如下文的 Ubuntu，加了 `-lc_p` 也报错，编译不出来可执行程序。

####动态库的题外话
* 首先，动态库也分两种：动态链接和动态加载
* 动态链接是程序运行一开始就把库加载进内存，主程序在调用到库函数的时候才会走到库里的代码。
* 动态加载是指当程序运行到一定地方的时候，用 dlopen 来加载库，并使用其中的代码。也就是说在主程序想使用库函数之前，库文件是没有被加载进内存的。这种方式在使用完了之后，需要用 dlclose 卸载库。

---

###实验1：动态库确实不行
* 准备源文件：libtestprofiler.cpp  libtestprofiler.h main.cpp
* 其中库函数只是单纯的耗时计算，所有源文件内容如下：


		:::C++
		//libtestprofiler.h
		extern "C"{
			int loopop();
		}
		
		//libtestprofiler.cpp
		#include "libtestprofiler.h"
		extern "C"{
			int loopop()
			{
			    int n = 0;
			    for(int i = 0; i < 1000000; i++)
			        for(int j = 0; j < 10000; j++)
			        {  
			            n |= i%100 + j/100;
			        }  
			    return n;
			}
		}
		
		//main.cpp
		#include <iostream>
		#include "libtestprofiler.h"
		using namespace std;
		int main(int argc,char** argv)
		{
		    cout << "loopop: " << loopop() << endl;
		    return 1;
		}

* 编译库（加 -pg）：

		g++ --shared -fPIC -pg -o libtestprofiler.so libtestprofiler.cpp

* 编译主程序（加 -pg）

		g++ -pg -o main main.cpp -ltestprofiler -L.

* 拷贝库到 /lib/:

		sudo cp libtestprofiler.so /lib/

* 运行主程序：

		./main

* 运行结束后生成了 gmon.out，用 gprof 分析之：

		gprof main

* 发现什么也没统计到，所有函数耗时都是0
* 看一下库文件里是否加入了 mcount：

		nm libtestprofiler.so | grep mcount

* 结果是加入了的，能看到输出：`U mcount@@GLIBC_2.2.5`，不过还是采集不到动态库里函数的 profile 信息。

###实验2：让动态库也行（实验失败）
* Ubuntu 默认是没有 libc_p.a/libc_p.so 的，需要安装 libc6-prof：

		sudo apt-get install  libc6-prof

* 源文件同上，库的编译命令不变
* 在主程序的编译命令里加入 –lc_p 选项。

		g++ -pg -o main main.cpp -ltestprofiler -L. -lc_p

* 以上命令编译报错，google了半天没有找到答案，这个实验暂时不做了。