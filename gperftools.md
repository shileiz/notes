* 官网：[https://github.com/gperftools/gperftools](https://github.com/gperftools/gperftools)
* 参考：[http://www.cnblogs.com/lenolix/archive/2010/12/13/1904868.html](http://www.cnblogs.com/lenolix/archive/2010/12/13/1904868.html)
* CPU profile 只是这个工具其中一个 feature，我们就用这个 feature。
* 其他 feature 还有 tcmalloc：一个比malloc更快的内存管理算法；heap profiler，heap checker。

###一. 安装
* clone 代码：

		git clone https://github.com/gperftools/gperftools.git

* 根据 INSTALL 里面的介绍，首先需要安装 autoconf，automake，libtool。然后才能产生 configure 脚本，所以安装：

		yum install autoconf
		yum install automake
		yum install libtool

* 产生 configure 脚本：

		/autogen.sh

* 根据 INSTALL 里面的介绍，在 64 Linux 位系统上，建议先装 libunwind 然后再 config 和 make，所以安装：
	* 下载包：[http://download.savannah.gnu.org/releases/libunwind/libunwind-0.99-beta.tar.gz](http://download.savannah.gnu.org/releases/libunwind/libunwind-0.99-beta.tar.gz)
	* 解压和安装（安装到哪自定义：path/to/where/you/want，记住这个路径，一会儿装 gperftools 要用)：
			
			tar -xzvf libunwind-0.99-beta.tar.gz
			cd libunwind-0.99-beta
			./configure --prefix=path/to/where/you/want
			make
			make install

* 设置 configure 需要的环境变量：我们安装了 libunwind 之后，需要设置几个环境变量，这样做 configure gperftools 的时候，才能找到我们装的 libunwind：

		export  CPPFLAGS=-I/path/to/libunwind_install/include
		export  LDFLAGS=-L/path/to/libunwind_install/lib

* 运行 configure 脚本，只需要用 --prefix 指定安装到哪里即可：

		./configure --prefix=/path/to/gperftools_install
		make
		make install

###二. 使用

####例1. HelloWorld
* 一个helloworld源文件：hello.c

		:::c
		/* hello.c */
		#include<stdio.h>
		int main(int argc,char** argv){
		    printf("hello world\n");
		}

* 编译时加入 -lprofiler 选项：

		gcc -o hello hello.c -lprofiler -L/path/to/gperftools_install/lib

* 运行之前设置环境变量：

		export CPUPROFILE=hello.prof.out

* 运行之前把库文件拷贝到系统库目录，要不然运行时找不到 libprofiler 库：

		cp path/to/gperftools_install/lib/* /usr/lib64/

* 运行：

		./hello

* 运行结束后生成了 hello.prof.out
* 分析该文件
	* 首先把 pprof 拷贝到 /usr/bin

			cp path/to/gperftools_install/bin/pprof /usr/bin/

	* 然后用 pprof 分析刚才生成的 hello.prof.out

			pprof --text ./hello ./hello.prof.out

* 输出了两行没什么意义的东西：

		Using local file ./hello.
		Using local file ./hello.prof.out.

####例2. 稍复杂的程序
* 准备一个源文件 take_time.c 如下：

		:::c
		/* take_time.c */
		#include<stdio.h>
		
		int take()
		{
		    int n = 0;
		    int i = 0, j = 0;
		    for(i = 0; i < 1000000; i++)
		        for(j = 0; j < 10000; j++)
		        {
		            n |= i%100 + j/100;
		        }
		    return n;
		}
		
		int main(int argc,char** argv)
		{
		    int r =  take();
		    printf("%d\n",r);
		    return 1;
		}

* 编译，设置环境变量，运行都同上，就不重写了
* 分析输出文件：

		pprof --text take take.prof.out

* 得到如下结果：

		Using local file take.
		Using local file take.prof.out.
		Removing main from all stack traces.
		Removing __libc_start_main from all stack traces.
		Removing _start from all stack traces.
		Total: 2937 samples
		    2937 100.0% 100.0%     2937 100.0% take

* 可以看到，正确的统计出了函数 take 占用了 100.0% 的时间

####例3. 共享库
* 共享库的例子主要为了说明以下几点：
	1. 在编译 xxxxxx.so 时，不用加上 -lprofiler，只要在把 xxxxxx.so 连接进主程序时加上即可。库里的函数一样可以被统计到。
	2. 即使 xxxxxx.so 里有些函数没有被主程序直接使用（该库的头文件没有暴露该函数，这些函数是库自用的），即主程序的符号表里根本不存在该函数，该函数也能被成功统计到信息。本例中的 worker() 函数就将扮演这个角色。

* 准备3个源文件，库：libtake.c，主程序：main.c

		:::c
		/* libtake.h */
		int take_interface();

		/* lilbtake.c */
		#include "libtake.h"	
		int worker()
		{
		    int n = 0;
		    int i = 0, j = 0;
		    for(i = 0; i < 1000000; i++)
		        for(j = 0; j < 10000; j++)
		        {
		            n |= i%100 + j/100;
		        }
		    return n;
		}
		
		int take_interface()
		{
		    int n = 0;
		    int i = 0, j = 0;
		    for(i = 0; i < 100000; i++)
		        for(j = 0; j < 10000; j++)
		        {
		            n |= i%100 + j/100;
		        }
			int r = worker();
		    return n+r;
		}
		
		/* main.c */
		#include <stdio.h>
		#include "libtake.h"
		int main(int argc,char** argv)
		{
		    int r =  take_interface();
		    printf("%d\n",r);
		    return 1;
		}

* 编译生成库，注意，不用加 -lprofiler：

		gcc --shared  -fPIC -o libtake.so  libtake.c

* 编译生成主程序，注意，此时加入 -lprofiler：

		gcc -o main main.c -lprofiler -L/home/zsl/gperftools_build/lib -ltake -L./

* 为了能运行main，把 libtake.so 拷贝到 /usr/lib/，并进行 ldconfig:

		cp libtake.so /usr/lib/
		ldconfig

* 设置环境变量并运行：

		export CPUPROFILE=main.prof.out
		./main

* 分析结果：

		pprof --text main main.prof.out 

* 输出结果如下：

		Total: 3228 samples
		    2935  90.9%  90.9%     2935  90.9% worker
		     293   9.1% 100.0%     3228 100.0% take_interface
		       0   0.0% 100.0%     3228 100.0% __libc_start_main
		       0   0.0% 100.0%     3228 100.0% _start
		       0   0.0% 100.0%     3228 100.0% main

* 可以看到，函数 worker 占用了 90.9% 的时间，函数 `take_interface` 自身占用了 9.1% 的时间，而 `take_interface` 加上它的子函数（即 worker）一共占用了 100.0% 的时间。结果是符合预期的。

* 注意：
	* 我们编译 libtake.so 的时候并没有加 -lprofiler（并不像 gprof 要在编译库的时候也加上 -pg）
	* main.c 并不知道有 worker() 函数的存在，`nm main | grep worker `也输出空
	* 但我们确实抓到了 worker 函数消耗的时间

####例4. 动态加载(dlopen)的共享库
* 例3中我们使用共享库的方法是在编译主程序时，把共享库 libtake.so 连到了主程序上。
* 而实际开发过程中，尤其是开发插件时经常使用的，是运行时加载动态库，即使用 dlopen 的方式。
* 源文件：libtake.c 和 libtake.h 不变，甚至可以直接使用例3里编译好的库。main.c 改造成如下形式（改名为 main_dl.c）：

		:::c
		#include<stdio.h>
		#include<dlfcn.h>
		
		char LIBPATH[] = "./libtake.so";
		typedef int (*op_t) ();
		
		int main(int argc, char** argv)
		{
		    void* dl_handle;
		    op_t take_interface;
		    dl_handle = dlopen(LIBPATH, RTLD_LAZY);
		    take_interface = (op_t)dlsym(dl_handle, "take_interface");
		    printf("%d\n", (take_interface)() );
		    dlclose(dl_handle);
		    return 0;
		}

* 编译主程序，加入 -lprofiler，注意，这次不用再 -ltake 去连接自己的库了，不过要加上 -ldl 因为使用了 dlopen：

		gcc -o main_dl main_dl.c -lprofiler -L/home/zsl/gperftools_build/lib -ldl

* 设置环境变量和运行不再重复，分析出的结果如下（省略部分）：

		Total: 3230 samples
		     447  13.8%  13.8%      447  13.8% 0x00007f53418a65e7
		     278   8.6%  22.4%      278   8.6% 0x00007f53418a65d3
			   .....
		       0   0.0% 100.0%     2936  90.9% 0x00007f53418a6699
		       0   0.0% 100.0%     3230 100.0% __libc_start_main
		       0   0.0% 100.0%     3230 100.0% _start
		       0   0.0% 100.0%     3230 100.0% main

* 我们发现它找不到库 libtake.so 里的符号了，全都用内存地址代替了。
* 这个问题我们只需要把 dlclose() 去掉即可。去掉后重编 `main_dl`,分析结果正常。

####例n. 非常复杂：ffmpeg
* 我们现在用 gperftools 来对复杂的项目 ffmpeg 进行 profile
* 下载 ffmpeg 最新代码，configure 的时候加上如下选项，以使其编译时能连接 -lprofiler：

		--extra-ldflags=-lprofiler --extra-ldflags=-L/home/zsl/gperftools_build/lib

* 为了以防万一，我还加上了 `--disable-stripping`

* 其他配置选项根据个人需求，我的完整配置命令如下：

		./configure --prefix=/home/zsl/ffmpeg_build --enable-shared --disable-static \
		--disable-stripping --disable-avdevice --disable-yasm\  
		--extra-ldflags=-lprofiler --extra-ldflags=-L/home/zsl/gperftools_build/lib 

* make && make install

		make -j 16
		make install

* 因为是共享库，运行 ffmpeg 之前把lib里的东西都拷贝到 /usr/lib
* 设置 gperftools 需要的环境变量：

		export CPUPROFILE=ffmpeg.prof.out

* 运行 ffmpeg，得到转码结果的同时得到性能统计文件 ffmpeg.prof.out

		./ffmpeg -i /home/zsl/contents/input.mp4 output.mp4

* 分析性能：

		pprof --text ffmpeg ffmpeg.prof.out  > prof_mp4.txt


###三. 问题

####1. x64系统的崩溃问题
* 使用过程中，即设好了环境变量然后运行 ffmpeg 转码的过程中，会经常的崩溃，报段错误。官方文档也说了，在x64的系统上确实有这问题。
* 这种崩溃是随机发生的，多试几次，总有不崩的时候。