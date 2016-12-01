* 官网：[https://github.com/gperftools/gperftools](https://github.com/gperftools/gperftools)
* CPU profile 只是这个工具其中一个 feature，我们就用这个 feature。
* 其他 feature 还有 tcmalloc：一个比malloc更快的内存管理算法；heap profiler，heap checker。

###安装
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

###使用

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

####例2. 稍微复杂和耗时的程序
* 准备一个源文件 take_time.c 如下：

		:::c
		/* take_time.c */
		#include<stdio.h>
		
		int loopop()
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
		    int r =  loopop();
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
		    2937 100.0% 100.0%     2937 100.0% loopop