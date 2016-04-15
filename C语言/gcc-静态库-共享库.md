## gcc
* gcc 命令行的一般格式是： gcc 参数列表 输入文件列表
	* 即输入文件在最后。参数也是有一定顺序要求的，具体看man gcc。
	* 但也不是定死的，`gcc main.c -o app` 这种格式（先写的输入文件，后写的参数-o）也是可以的。
* gcc 默认进行四个处理步骤（按顺序）：预处理，编译，汇编，链接。
	* 预处理：生成展开了宏定义和头文件后的c语言源文件，其本质上还是一个c语言源文件。
	* 编译：生成汇编语言源代码文件，就跟你用汇编语言在文本编辑器里敲的一样，其本质上是一个汇编语言源文件。一个c源文件文件会生成一个汇编源文件。
	* 汇编：生成目标文件（二进制文件），一个汇编语言源文件生成一个目标二进制文件。
	* 链接：生成可执行程序，把生成的多个二进制目标文件加上编译系统提供的一些二进制目标文件（比如启动代码啥的）链接起来，生成可执行程序。
	* 我们平常说的编译，通常是指预处理、编译、汇编三个步骤。
## gcc 从哪步开始，到哪步结束
### 从哪步开始
* gcc 根据输入文件的后缀名判断需要对该文件从哪步开始进行处理。
	* 比如汇编语言的源文件，就不需要编译了，直接从汇编开始，然后再链接就行了。
* 常用的后缀名有：
	* .c 需要进行预处理的c语言源文件。比如 `gcc main.c` 将会进行预处理，编译，汇编，链接四个步骤。
	* .i 不需要进行预处理的c语言源文件。比如 `gcc main.i` 将会进行编译，汇编，链接三个步骤。
	* .cpp/.cc/.C 需要进行预处理的c++语言源文件。
	* .ii 不需要进行预处理的c++语言源文件。
	* .s 汇编语言源文件
	* .S/.sx 需要预处理的汇编代码
* 没有后缀名，或者不被识别的后缀名，这种文件都将被当做目标文件送给链接器进行链接。
* 另外，可以用-x参数让gcc不根据后缀名判断输入文件的类型，-x 后面直接跟输入文件的类型
	* 比如：`gcc -x c main` 将会按照 main 是一个c语言源文件来处理。
### 到哪步结束

* 可以使用“全局参数”可以让gcc在其中某个步骤后结束。
* 这类全局参数有3个：
	* -c ：在汇编后结束。
		* 输出的是目标文件。默认输出文件名是把输入文件的后缀名（.c, .i, .s等）替换成.o 
		* 比如： `gcc -c main.c` 将进行预处理，编译，汇编，生成 main.o  
	`gcc -c main.i` 将会进行编译，汇编，生成 main.o
		* 不需要进行编译或者汇编的输入文件会被忽略。
	* -S ：在编译后结束。
		* 输出的是汇编代码。默认输出文件名是把输入文件的后缀名（.c, .i等）替换成.s
		* 比如 `gcc -S main.c` 将进行预处理，编译，生成 main.s
		* 不需要进行编译的输入文件将会被忽略。
	* -E 在预处理后结束。
		* 输出的是预处理过的源文件，输出将被直接送往标准输出（默认是控制台打印）。
		* 比如 `gcc -E -o main.i main.c ` 生成main.i，就是把include和宏展开后的c文件
		* 不需要进行预处理的输入文件将会被忽略。
## gcc 其他常用全局参数:
### -o file
* 指定输出文件。无论输出文件是目标文件，汇编代码，还是可执行文件，都用-o来指定。
* 如果不指定：
	* 当输出目标文件时，其文件名将会是输入文件的后缀名替换为.o；
	* 当输出汇编代码时，其文件名将是输入文件的后缀名替换成.s；
	* 当输出可执行文件时，其文件名将固定叫做a.out，
### -x language
* 如上所述，让gcc不按后缀名而按-x参数指定的语言来处理输入文件。


## 举个例子：
前提：有两个.c文件，add.c 和 main.c 。内容如下：

	/*add.c*/
	int add(int i, int j)
	{
		return i+j;
	}

	/*main.c*/
	#include <stdio.h>
	void main()
	{
		printf("1+2=%d\n",add(1,2));
	}

* 没有 main 函数的.c文件，编译完了之后是无法链接生成可执行文件的。
	* `gcc add.c` 将会报 ld 的错
	* 这种情况应该加上 -c 参数，让gcc只编译不链接：`gcc -c add.c `
* main.c 这种文件，里面引用了add这个没有声明和定义的函数，也是可以编译通过的，只是无法链接。
	* `gcc -c main.c` 可以生成 main.o
	* 但是 `gcc main.c` 报一个 ld 的错。
* 单独编译完 add.c 生成的目标文件就可以提供给链接器了。
	* `gcc -c add.c` //生成add.o
	* `gcc main.c add.o`   //编译main.c，并把编译得到的main.o和add.o链接，生成a.out
	* 生成出来的a.out可以正确运行。
* 结论：你从网上得到了一个add.o，即便人家没给你提供头文件，只要你知道了add.o里面有个函数add()可以用，你就可以用。
* 用法就是在你的程序里直接调用add()，不用include任何头文件。然后把你的程序编译成.o，再来跟add.o链接生成可执行程序即可。
* 一般网上得到的库，都提供了相应的头文件和文档，要不你也不知道咋用，连库里面有啥函数都不知道。		  
## 静态库
* 所谓静态库，只不过是好几个.o文件的集合而已，就类似一个压缩包。
* 上面的例子没有生成.a静态库，直接把二进制目标文件.o链接进了可执行程序。
* 把例子改一下，弄个静态库出来：
	* 在把add.c编译成add.o之后，用ar命令把add.o做成静态库add.a：`ar rcs add.a add.o`
	* 编译main.c同时跟静态库add.a链接生成可执行文件a.out: `gcc main.c add.a`
* ar的一般用法是： `ar rcs libmylib.a file1.o file2.o`
	* 参数r: 把目标代码file1.o和file2.o 加入到静态库 libmylib.a 中
	* 参数c：若libmylib不存在，则自动创建
	* 参数s：更新.a文件的索引，使之包含新加入的.o文件的内容

* 把上述例子加入一个sub.c就更形象了：

		/*add.c*/
		int add(int i, int j)
		{
		    return i+j;
		}

		/*sub.c*/
		int sub(int i, int j)
		{
			return i-j;
		}

		/*main.c*/
		#include <stdio.h>
		void main()
		{
			printf("1+2=%d\n",add(1,2));
			printf("3-2=%d\n",sub(3,2));
		}
	* `gcc -c add.c sub.c`
	* `ar rcs calc.a *.o`
	* `gcc main.c calc.a`
	* 以上命令生成的a.out可以正确运行。


## 共享库（动态库）
关键参数（链接参数 Options for Linking）
-shared
   Produce a shared object which can then be linked with other objects to form an executable.  Not all systems support this option.  For predictable results,
   you must also specify the same set of options used for compilation (-fpic, -fPIC, or model suboptions) when you specify this linker option.
-Wl,option
   Pass option as an option to the linker.  If option contains commas, it is split into multiple options at the commas.  You can use this syntax to pass an
   argument to the option.  For example, -Wl,-Map,output.map passes -Map output.map to the linker.  When using the GNU linker, you can also get the same effect
   with -Wl,-Map=output.map.
   NOTE: In Ubuntu 8.10 and later versions, for LDFLAGS, the option -Wl,-z,relro is used.  To disable, use -Wl,-z,norelro.


gcc -shared -Wl,-soname,libmyab.so.1 -o libmyab.so.1.0.1 a.o b.o
共享库的命名惯例：real name，soname，linker name
real name：正真的库文件名字即real name，不是符号链接，是真的文件。real name 包含完整的共享库版本号。
soname：是一个符号链接，只包含主版本号。
主版本号一致即保证函数的接口一致。因此，应用程序的.dynamic段只记录共享库的soname，只要soname一致，这个共享库就可用。
注意，libc的版本编号特殊，libc-2.8.90.so的主版本号是6而不是2。
linker name：仅在编译链接时使用，gcc的-L选项应该指定linker name所在的目录。
linker name 一般不包含任何版本号，比如就叫libab.so，它实际上也是一个real name的符号链接。
有的linker name 是一段链接脚本。libc.so就是一个linker name，它是一段链接脚本：
OUTPUT_FORMAT(elf64-x86-64)
GROUP ( /lib/x86_64-linux-gnu/libc.so.6 /usr/lib/x86_64-linux-gnu/libc_nonshared.a  AS_NEEDED ( /lib/x86_64-linux-gnu/ld-linux-x86-64.so.2 ) )

举个例子：
跟静态库准备的文件一样，3个.c文件：add.c sub.c main.c
跟静态库一样，先把所有的.c都编译成.o: gcc -c *.c
链接生成动态库文件：gcc -shared add.o sub.o -o calc.so
链接生成可执行文件：gcc main.o calc.so
这样做出来的a.out运行时报错，说找不到共享库calc.so
把calc.so拷贝到/usr/lib/即可执行a.out了
注意1：生成add.o和sub.o时，都没加-fPIC，结果一样可运行
注意2：本例没有按惯例区分real name，soname，linker name，一样可以运行

下面说一下共享库的3种name在实际例子中的应用：
我们从网上下载了calc.so.1.0.1，注意，人家提供的库文件肯定是含有详细版本号的。
你想基于这个库开发一个main.c，比如你的main.c调用了这个库的add()函数。
开发好main.c之后，你要编译链接生成可执行程序。
你把main.c和库文件放在同一个目录下，然后运行 gcc main.c calc.so.1.0.1 -o app
为了能让生成的app可以运行，你还得把下载的库文件calc.so.1.0.1拷贝到/usr/lib/里
这样你的开发就完成了，app可以运行。
当发布库文件的团体更新calc.so到新版本：calc.so.1.0.2的时候，你想升级库，你去下载新的库文件，然后拷贝到/usr/lib/，并删除原来旧的库文件。
这时你需要重新编译main.c并跟新的库文件进行链接：gcc main.c calc.so.1.0.2 -o app
这样的问题是，每次需要升级库的时候，都太麻烦。


gcc main.c calc.so.1.0.1 -o app
和
gcc main.c calc.so.1.0.1 -o app
的区别：
以上两条命令完全一样，有什么区别呢？
用ldd分别查看两个app，确实还是有区别：
第一个的结果是：calc.so.1.0.1 => not found
第二个的结果是：calc.so.1 => not found
造成这种区别的原因是，两条命令中使用的calc.so.1.0.1不同
第一个是用：gcc -shared add.o sub.o -o calc.so.1.0.1 生成的
第二个是用：gcc -shared -Wl,-soname,calc.so.1 add.o sub.o -o calc.so.1.0.1 生成的
第二条命令多了个 -Wl,-soname,calc.so.1参数，导致生成的so文件不一样（确实不一样，md5值不同）
gcc 的-Wl命令是直接传给链接器的参数，因为传给链接器的多个参数不能用空格分隔了（如果用空格分隔可能被理解成gcc的多个参数了），所以用逗号分隔。
也就是，我们在用链接器生成动态库的时候，如果给链接器指定了-soname xxxx 那么无论这库文件的文件名是什么，当你把他链接到应用程序里时，也只记录他的soname。



ldd 命令：
ldd 可执行程序名
用来查看该程序依赖哪些共享库
ldd a.out
	linux-vdso.so.1 =>  (0x00007fff527fc000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f3451b43000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f3451f20000)




-I 指定头文件的目录
#include "" 双引号表示当前目录下的头文件，双引号内也可以写上路径。
比如：
#include "include/add.h"
然后把头文件放在include目录里，是可以编译的。
如果用-I来指定头文件的目录，那么就可以用<>来include了。
比如：
#include <add.h>
编译的时候：gcc -I./include main.c add.c -o app 
-g 包含调试信息
-On n=0~3 编译优化，n越大优化的越多
-Wall 提示更多警告信息
-D 编译时定义宏，注意-D和参数之间没有空格

-M 生成.c文件与头文件依赖关系用于Makefile，包括系统库的头文件
-MM 生成.c文件与头文件依赖关系用于Makefile，不包括系统库的头文件
-fPIC 生成与位置无关的代码


toolchain
ar 是用来创建归档文件的，即生成静态库的
as 是汇编器
ld 是连接器。用gcc完成链接步骤，其实是gcc调用的ld，将用户编译生成的目标文件同系统的libc启动代码链接在一起形成最终的可执行文件。
nm 是查看目标文件中的符号表（全局变量、全局函数等）
objcopy 复制目标文件，可以通过参数调整目标文件的格式
objdump 生成反汇编文件，主要依赖于objcopy实现，被反汇编的目标文件编译时需要加上-g参数
objdump -dSsx a.out > file
objdump -dSsx app > file
glibc 是Linux的一个C语言库，实现了open，read等，也实现标准C语言库，比如printf，fopen等等
几乎所有应用程序都需要与glibc链接






