###cmake是干嘛的
* 正常来说，我们写好源文件（.c，.cpp）之后，会自己写 makefile 然后用 make 进行编译。如果是在 Windows 开发，则直接用 VS 新建源文件，直接编译。
* 如果你写的代码想要在不同的平台下进行调试和编译，比如说 Linux（gcc+makefile）、Windows（VisualStudio）、MacOS（Xcode），那么你可能除了代码之外，还要维护好几套配置文件。
* cmake 的作用就是用平台无关的 cmake 语法来写唯一的一套工程配置文件即可——CMakeLists.txt，然后用 cmake 工具在不同的平台上直接生成该平台的工程。
* 比如在 Linux 平台上，cmake 就根据 CMakeLists.txt 给你生成 makefile，在Windows上就给你生成 .sln 解决方案文件。
* cmake 支持的平台很全，除了上述几个最常用的，还支持 Cygwin，MinGW，Borland 等等很多编译系统。

###cmake命令行
* 一般只需运行： `cmake -G "generator-name" path-to-source`
* 参数 -G，指定 cmake 的 generateor。cmake 支持很多 generator，比如 "MSYS Makefiles", "Visual Studio 14 2015"。完整列表见这里：[https://cmake.org/cmake/help/v3.7/manual/cmake-generators.7.html#manual:cmake-generators(7)](https://cmake.org/cmake/help/v3.7/manual/cmake-generators.7.html#manual:cmake-generators(7))
* path-to-source 指定源文件的路径，这个路径里要包含一个叫 CMakeLists.txt 的文件，cmake 会去读取这个文件并执行里面的 cmake 命令。

###基本
* 官方文档：[https://cmake.org/cmake/help/v3.7/](https://cmake.org/cmake/help/v3.7/)
* cmake 的所有命令都写在 CMakeLists.txt 里，或者以 .cmake 为后缀的文件里。
* 这种文件内部按照 cmake 语法书写。 Cmake 的语法规定，每一行都是一个 cmake 命令（cmake-command），cmake 命令对大小写不敏感
* 每条命令都可以有自己的参数，参数用小括号扩起来，以空白分隔。

		# add_executable 是命令，hello 和 world.c 是参数
		add_executable(hello world.c) 

* 以上的 `add_executable` 就是一条 cmake 命令，括号里的 hello 和 world.c 是参数
* 参数分三种类型：
	* Bracket Argument： Cmake 3.0 以上才支持，这里不展开
	* Quoted Argument： 用双引号引起来的
	* Unquoted Argument：不用双引号引起来的

###Tutorial
* 地址：[https://cmake.org/cmake-tutorial/](https://cmake.org/cmake-tutorial/)

####环境准备
* msys2+mingw64(貌似 mingw64 自带 cmake，/mingw64/bin/cmake.exe )
* 源文件 tutorial.cxx，内容很简单，只是一个计算平方根的功能，内容如下：

		// A simple program that computes the square root of a number
		#include <stdio.h>
		#include <stdlib.h>
		#include <math.h>
		int main (int argc, char *argv[])
		{
		  if (argc < 2)
		    {
		    fprintf(stdout,"Usage: %s number\n",argv[0]);
		    return 1;
		    }
		  double inputValue = atof(argv[1]);
		  double outputValue = sqrt(inputValue);
		  fprintf(stdout,"The square root of %g is %g\n",
		          inputValue, outputValue);
		  return 0;
		}


* cmake 配置文件 CMakeLists.txt，内容只有一行如下：

		add_executable(Tutorial tutorial.cxx)

####实验1：生成makefile
* 打开 MinGW64 的命令行，cd 到 tutorial.cxx 所在 目录，目前该目录只有两个文件：

		.
		├── CMakeLists.txt
		└── tutorial.cxx


* 运行 `cmake -G "MSYS Makefiles" ./`，cmake 会输出很多的提示信息，运行结束后，生成了很多的文件，目录结构如下：

		.
		├── cmake_install.cmake
		├── CMakeCache.txt
		├── CMakeFiles
		├── CMakeLists.txt
		├── Makefile
		└── tutorial.cxx

* 可以看到 cmake 自动帮你生成了 Makefile，然后还生成了一个文件夹——CMakeFiles。如果展开文件 CMakeFiles，会发现里面东西很多，目前先不展开。

* 我们直接运行make，果然编译出了 Tutorial.exe，并且能运行。

####实验2：生成 VS 解决方案
* 我们把刚才生成的东西都删除，把目录恢复到最初的样子：

		.
		├── CMakeLists.txt
		└── tutorial.cxx

* 运行 `cmake -G "Visual Studio 14 2015" ./`，结束后会发现，.sln/.vcxproj 等文件全给你生成好了，目录如下：

		.
		├── ALL_BUILD.vcxproj
		├── ALL_BUILD.vcxproj.filters
		├── cmake_install.cmake
		├── CMakeCache.txt
		├── CMakeFiles
		├── CMakeLists.txt
		├── Project.sln
		├── tutorial.cxx
		├── Tutorial.vcxproj
		├── Tutorial.vcxproj.filters
		├── ZERO_CHECK.vcxproj
		└── ZERO_CHECK.vcxproj.filters

* 双击 sln 文件打开 VS2015，直接编译运行成功。
