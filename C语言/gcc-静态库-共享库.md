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
	* -c ：生成.o二进制目标文件（在汇编后结束）。
		* 输出的是目标文件。默认输出文件名是把输入文件的后缀名（.c, .i, .s等）替换成.o 
		* 比如： `gcc -c main.c` 将进行预处理，编译，汇编，生成 main.o  
	`gcc -c main.i` 将会进行编译，汇编，生成 main.o
		* 不需要进行编译或者汇编的输入文件会被忽略。
	* -S ：生成.s汇编语言源文件（在编译后结束）。
		* 输出的是汇编代码。默认输出文件名是把输入文件的后缀名（.c, .i等）替换成.s
		* 比如 `gcc -S main.c` 将进行预处理，编译，生成 main.s
		* 不需要进行编译的输入文件将会被忽略。
	* -E 生成.i展开宏之后的c语言源文件（在预处理后结束）。
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
	* 注意，虽然`gcc -c main.c` 可以生成 main.o，但如果在编译时加上 -Wall 参数打印所有 warning 的话，这里会有一个 warning 说：`implicit declaration of function ‘add’`
* 单独编译完 add.c 生成的目标文件就可以提供给链接器了。
	* `gcc -c add.c` //生成add.o
	* `gcc main.c add.o`   //编译main.c，并把编译得到的main.o和add.o链接，生成a.out
	* 生成出来的a.out可以正确运行。
* 结论：你从网上得到了一个add.o，即便人家没给你提供头文件，只要你知道了add.o里面有个函数add()可以用，你就可以用。
* 用法就是在你的程序里直接调用add()，不用include任何头文件。然后把你的程序编译成.o，再来跟add.o链接生成可执行程序即可。
* 一般网上得到的库，都提供了相应的头文件和文档，要不你也不知道咋用，连库里面有啥函数都不知道。	

#### 题外话，那么，头文件到底有啥用？
* 如上所述，没有头文件也可以照样使用函数 add。
* 即函数不用先声明再使用。
* gcc 确实不会对未声明就使用的函数报error，但是会报 Warning。看例子：

		/* main.c */
		int main()
		{
		    mybalabalafunc(100, 3);
		    return 0;
		}
		
		/* 编译，但不链接，输出全部 Waring*/
		gcc -c -Wall main.c

		/* 结果：成功编译出 main.o，但有如下 Warning*/
		warning: implicit declaration of function ‘mybalabalafunc’
* 所以说，头文件的作用，目前看来，就是可以消除编译器的 Warning
* 另外，如果头文件里不光负责声明函数，还定义了一些宏啊什么的，那作用就不只是消除 Warning 了。
	  
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
* 动态库本质上也是一堆.o二进制文件的集合，从这个角度看，跟静态库是一样的。
### 动态库和静态库的区别
* 区别一，从使用者的角度看：被应用程序使用的方式不同
	* 静态库会跟可执行程序绑定在一起，好处是保证可用，坏处是使得可执行程序比较大；
	* 动态库不跟可执行程序绑定在一起，好处是可复用性强，可执行程序体积小；坏处是可执行程序不一定可用，依赖于机器上是否安装了共享库。
* 区别二，从制作者的角度看：集合.o的方式不同
	* 可以简单理解为静态库只是把所有.o打包在了一起，跟rar似的。函数调用的时候，直接使用每个.o文件自己的符号表去找函数。
	* 可以简单理解为动态库除了把.o文件打包在了一起，还根据各个.o文件的符号表，重新建立了符号表。函数调用的时候，根据新建的符号表去找函数。
### 第一个例子：把静态库的例子改为动态库，先感受一下动态库
* 跟静态库一样，准备3个.c文件：add.c, sub.c, main.c
* 跟静态库一样，先把所有的.c都编译成.o: `gcc -c *.c`
* 连接生成动态库文件：`gcc -shared add.o sub.o -o calc.so` 。（这步跟静态库用 ar 生成.a文件是不一样的。）
* 连接生成可执行文件：`gcc main.o calc.so`
* 这样做出来的a.out运行时报错，说找不到共享库calc.so。把calc.so拷贝到/usr/lib/即可执行a.out了。
* 这说明使用动态库的可执行程序，依赖于操作系统是否安装了相应的动态库。（把calc.so拷贝到/usr/lib/相当于给操作系统安装了动态库calc.so）
### 从第一个例子感受到了什么？
* 感受到了动态库和静态库的区别
* 首先感受到了区别一：被应用程序使用的方式不同。用动态库做出来的应用程序想成功运行，需要首先把动态库安装到操作系统里。虽然用动态库和静态库生成可执行程序的命令是相同的，都是 `gcc 应用程序.o 库文件 `。
* 再来看看区别二：制作库文件的方式。做静态库时使用 ar 命令，做动态库使用的是 gcc -shared 命令。
	* -shared 参数： 这是一个链接参数（Options for Linking），用于把多个目标文件连接生成动态库文件。
* 对于区别二，其实不止是生成库文件的命令不同这么简单。其实在编译阶段就应该使用不同的命令。
* 如果你的目标文件（.o文件）将来是要用来生成动态库的，那么你编译时必须要加上参数 -fPIC。我们的例子就应该是： `gcc -fPIC -c *.c`
* 但是为何我们的第一个例子没加 -fPIC 也成功了呢？ 这个问题需要了解了“位置无关代码”之后再回答。
### 位置无关代码：-fPIC
* 位置无关代码/位置相关代码，指的都是编译后生成的二进制代码的不同（或者说编译后生成的汇编代码不同），而不是说C语言代码的不同。
* 同一份C语言代码既可以生成位置相关代码，也可以生成位置无关代码。取决于编译时使用的编译方式。
* 位置无关代码是指：指令跳转时使用相对地址。可以想象成执行汇编指令jmp时，jmp 的目标地址是相对当前地址的偏移量。
* 位置相关代码则相反：指令跳转使用绝对地址。
* 位置无关/相关代码的这种区别决定了，只有位置无关代码可以放到共享库里面，因为共享库不一定被载入到内存的什么位置。
* gcc 的 -fPIC 选项表示生成位置无关代码。
* -fPIC 是代码生成选项（Code Generation Option），用来控制生成的目标代码（即.o文件）的格式的。（代码生成选项都是 `-fxxx` 这种形式的）
### 第二个例子，看看位置无关代码
* 我们有一个源文件 hello.c:

		#include<stdio.h>
		void say_hello()
		{
		    printf("Hello World!");
		}
* 编译生成位置无关的二进制目标文件： `gcc -fPIC -c -o hello.fpic.o hello.c`
* 编译生成位置相关的二进制目标文件： `gcc -c -o hello.nofpic.o hello.c`
* 看一下两个目标文件 hello.fpic.o 和 hello.nofpic.o 的md5值，确实是不一样的。
* 说明加不加 -fPIC，的确影响目标文件的格式。
### 第三个例子，只有动态无关代码才能制作动态库
* 接着第二个例子
* 用位置无关代码生成动态库： `gcc -shared -o hello.so hello.fpic.o` 
	* 成功生成了 hello.so
* 试图用位置相关代码生成动态库：`gcc -shared -o hello.so hello.nofpic.o`   
	* 得到报错如下： 
	
			/usr/bin/ld: hello.nofpic.o: relocation R_X86_64_32 against `.rodata' can not be used when making a shared object; recompile with -fPIC
			hello.nofpic.o: error adding symbols: Bad value
			collect2: error: ld returned 1 exit status
* 结论：确实只有位置无关代码才能生成动态库。不然链接器 ld 会报错，并且还会提示你：`recompile with -fPIC`
### 回答问题：为什么第一个例子在编译阶段没有加 -fPIC，最后也成功制作了动态库
* 因为第一个例子太简单了，编译出来的二进制文件不涉及到指令跳转，所以也就体现不出位置相关代码和位置无关代码的区别了。
* 我们可以分别加上和不加 -fPIC 来生成 add.o/sub.o，你会发现加和不加生成出来的.o文件md5是相同的。
* 延伸思考
	* 其实加了 -fPIC，不光导致生成的.o文件不同，生成的汇编代码也会有所不同。
	* 可以用 gcc -S 来生成.s 汇编文件对比一下就知道了。我们会发现加了 -fPIC 的汇编代码，在函数调用的地方多了一个 @PLT
	* @PLT 到底是什么不必深究，想深入研究可以读 《Reverse engineering for beginners》 这本书。可以学到汇编，反编译等知识，对于深入理解位置相关位置无关代码有很大帮助。
## 动态库的名称：realname，soname，linkername
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



## gcc 其他
### toolchain
* ar 是用来创建归档文件的，即生成静态库的
* as 是汇编器
* ld 是连接器。用gcc完成链接步骤，其实是gcc调用的ld，将用户编译生成的目标文件同系统的libc启动代码链接在一起形成最终的可执行文件。
* nm 是查看目标文件中的符号表（全局变量、全局函数等）
* objcopy 复制目标文件，可以通过参数调整目标文件的格式
* objdump 生成反汇编文件，主要依赖于objcopy实现，被反汇编的目标文件编译时需要加上-g参数。`objdump -dSsx a.out > file`
### -I 参数
* 指定头文件的目录
* 在c语言源文件里，`#include "xxx.h"` 双引号表示当前目录下的头文件，双引号内也可以写上路径。比如：`#include "include/add.h"`
* 如果用-I来指定头文件的目录，那么就可以用`<>`来include并且不在c语言源文件里写路径了。比如：`#include <add.h>`
* 编译的时候：gcc -I./include main.c add.c -o app 
### include 时，双引号和尖括号的区别
* 尖括号表示去系统默认存放头文件的目录找被include的那个.h文件。（系统指的是编译系统，即编译器）
* 双引号表示去当前目录下找被include的那个.h文件，如果找不到，再去系统默认目录下找。



