### 一. gdb 调试 PC 上的程序

* 样例程序：

		:::c
		/*main.c*/
		#include <stdio.h>
		int add(int x, int y)
		{
		    int sum;
		    sum = x+ y;
		    return sum;
		}
		
		int main(int argc, char* argv[])
		{
		    int a = 10;
		    a = add(1, 2);
		    printf("a=%d\n",a);
		    return 0;
		}


* 编译时加上 -g 选项:

		gcc -g -o app main.c

* 用 gdb 启动被调试程序，进入 gdb 命令行提示符，`gdb app`：

		shilei@shilei-Ubuntu:~/temp$ gdb app
		GNU gdb (Ubuntu 7.7.1-0ubuntu5~14.04.2) 7.7.1
		Copyright (C) 2014 Free Software Foundation, Inc.
		License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
		This is free software: you are free to change and redistribute it.
		There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
		and "show warranty" for details.
		This GDB was configured as "x86_64-linux-gnu".
		Type "show configuration" for configuration details.
		For bug reporting instructions, please see:
		<http://www.gnu.org/software/gdb/bugs/>.
		Find the GDB manual and other documentation resources online at:
		<http://www.gnu.org/software/gdb/documentation/>.
		For help, type "help".
		Type "apropos word" to search for commands related to "word"...
		Reading symbols from app...done.
		(gdb) 

* 可以看到打印了一大堆提示信息，然后进入了 gdb 的命令行提示符。这一大堆信息没啥用，只有最后一行说它从 app 里读取了符号表。有了符号就可以调试了。（没有符号其实也可以调试，只不过调试时显示的全是内存地址，而不是函数名变量名，让人无法看懂。）

* l 命令，列出源代码

		(gdb) l
		2	int add(int x, int y)
		3	{
		4	    int sum;
		5	    sum = x+ y;
		6	    return sum;
		7	}
		8	
		9	int main(int argc, char* argv[])
		10	{
		11	    int a = 10;

* l 命令可以列出源代码，带行号。默认情况下一次只列10行，继续运行 l 命令将显示之后的代码。
* 在 gdb 命令行里，如果不输入任何命令直接 回车，则执行的是上一条命令（在大多数情况下）。比如我们这里 l 之后，直接回车，则会显示接下来的代码。
* l 是 list 的简写，运行 list 跟运行 l 得到同样的效果。 后续介绍的命令大多是简写。
* start 命令，让被调试程序跑起来，并在程序入口处自动插入一个断点，停在断点处：

		(gdb) start
		Temporary breakpoint 1 at 0x400556: file main.c, line 11.
		Starting program: /home/shilei/temp/app 
		
		Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe508) at main.c:11
		11	    int a = 10;

* 可以看到，gdb 自动插入了一个临时断点，在 main.c 的第 11 行，并停在了那里。此时显示的第 11 行：` int a = 10;` 是尚未执行的一行
* p 命令（print 的简写），打印变量

		(gdb) p a
		$1 = 0

* p 后面跟要打印的变量名，由于这时候 a = 10 还没执行，所以打印出来是 0。这里的 $1 是 gdb 内部的临时计数，下次调用 p 的时候就会变成 $2
* n 命令（next 的简写），执行下一语句，类似 VS 里的“逐过程”，即不进入函数内部而执行一条语句：

		(gdb) n
		12	    a = add(1, 2);
		(gdb) p a
		$2 = 10

* 可以看到，n 命令让代码运行了一行，a = 10; 被执行了，下面将要执行的是第 12 行了。 此时再次打印a，已经等于 10 了。
* s 命令（step 的简写），进入函数，类似 VS 里的“逐语句”，即单步执行。

		(gdb) s
		add (x=1, y=2) at main.c:5
		5	    sum = x+ y;

* 可以看到，s 命令进入了 add 函数内部，并且把传入的参数 x=1，y=2 都打印出来了。并且告诉我们下面将要执行的是第 5 行 `sum = x + y;` 
* i 命令（info 的简写），显示信息，i 命令是需要参数的，可以用 help i 列出支持的所有参数：

		(gdb) help i
		Generic command for showing things about the program being debugged.
		
		List of info subcommands:
		
		info address -- Describe where symbol SYM is stored
		info all-registers -- List of all registers and their contents
		info args -- Argument variables of current stack frame
		info auto-load -- Print current status of auto-loaded files
		info auto-load-scripts -- Print the list of automatically loaded Python scripts
		info auxv -- Display the inferior's auxiliary vector
		info bookmarks -- Status of user-settable bookmarks
		info breakpoints -- Status of specified breakpoints (all user-settable breakpoints if no argument)
		info checkpoints -- IDs of currently known checkpoints
		info classes -- All Objective-C classes
		info common -- Print out the values contained in a Fortran COMMON block
		info copying -- Conditions for redistributing copies of GDB
		info dcache -- Print information on the dcache performance
		info display -- Expressions to display when program stops
		info exceptions -- List all Ada exception names
		info extensions -- All filename extensions associated with a source language
		info files -- Names of targets and files being debugged
		info float -- Print the status of the floating point unit
		info frame -- All about selected stack frame
		info frame-filter -- List all registered Python frame-filters
		info functions -- All function names
		info handle -- What debugger does when program gets various signals
		info inferiors -- IDs of specified inferiors (all inferiors if no argument)
		info line -- Core addresses of the code for a source line
		info locals -- Local variables of current stack frame
		...

* help 后跟命令名，可以查看各种命令的帮助
* 我们用 i locals 看一下目前（在函数 add 内部）的所有局部变量：

		(gdb) i locals
		sum = 0

* 可以看到 add 函数内部只有一个局部变量 sum，目前的值是 0
* bt 命令（backtrace 的简写），显示函数调用栈帧：

		(gdb) bt
		#0  add (x=1, y=2) at main.c:5
		#1  0x000000000040056c in main (argc=1, argv=0x7fffffffe508) at main.c:12

* f 命令（frame 的简写），选择栈帧，f 跟的参数是栈帧的序号：

		(gdb) f 1
		#1  0x000000000040056c in main (argc=1, argv=0x7fffffffe508) at main.c:12
		12	    a = add(1, 2);

* 此时如果再次查看局部变量，将是 main 函数的局部变量了：

		(gdb) i locals
		a = 10

* 注意，虽然 f 命令选择了函数 main 的栈帧，但并不表示程序执行又回到了 main，现在用 n 命令，将继续在 add 内部执行：

		(gdb) n
		6	    return sum;

* finish 命令，把当前函数运行完：

		(gdb) finish
		Run till exit from #0  add (x=1, y=2) at main.c:6
		0x000000000040056c in main (argc=1, argv=0x7fffffffe508) at main.c:12
		12	    a = add(1, 2);
		Value returned is $1 = 3
		(gdb) finish
		"finish" not meaningful in the outermost frame.

* 可以看到第一次运行 finish，从 add 函数里返回了，并且显示了返回值。
* 再次执行 finish ，告诉我们在最外层的栈帧这个命令没有意义。
* c 命令（continue 的简写），运行程序到下一个断点：

		(gdb) c
		Continuing.
		a=3
		[Inferior 1 (process 15641) exited normally] 

* 可以看到程序正常退出了，因为我们并没有给程序下任何的断点。

* 我们用 q 命令退出 gdb （需要按 y 确认），然后再次启动 gdb，这次我们下个断点试试：

		shilei@shilei-Ubuntu:~/temp$ gdb app
		...
		Reading symbols from app...done.
		(gdb) b add
		Breakpoint 1 at 0x400537: file main.c, line 5.
		(gdb) start
		Temporary breakpoint 2 at 0x400556: file main.c, line 11.
		Starting program: /home/shilei/temp/app 
		
		Temporary breakpoint 2, main (argc=1, argv=0x7fffffffe508) at main.c:11
		11	    int a = 10;
		(gdb) c
		Continuing.
		
		Breakpoint 1, add (x=1, y=2) at main.c:5
		5	    sum = x+ y;
		(gdb) 

* 可以看到，我们用 b add ，在 add 函数入口处下了一个断点。 c 命令后，程序停在了 add 的入口。
* b 是 break 的简写，其参数可以跟函数名，也可以跟行号。

### 二. gdb 调试 Android 上的纯 Native 程序

#### 1. 编译在 Android 上运行的纯 Native 程序
* Android 上常见的程序都是以 app 的形式存在，即开发者把 java 和 C/C++ 源码编译好的程序打包一个 apk 安装到手机上运行
* Android 上也可以有纯命令行的程序，仅由C/C++写成。其编译方式跟 Linux 类似，用 Android 的 makefile（Android.mk等） 和 Android 的编译工具链（NDK）制作即可。
* 我们的源程序还用之前的那个，添加如下2个 makefile: Android.mk, Application.mk
* Android.mk：

		LOCAL_PATH := $(call my-dir)
		include $(CLEAR_VARS)
		LOCAL_MODULE    := main
		LOCAL_SRC_FILES := main.c
		LOCAL_CFLAGS := -g
		include $(BUILD_EXECUTABLE)

* Application.mk：

		APP_ABI := arm64-v8a

* 注意，我们把 -g 参数加到了 Android.mk 里： `LOCAL_CFLAGS := -g`

* 把 main.c Android.mk  Application.mk 放到一个叫 jni 的文件夹里
* 在 jni 文件的外层运行 ndk-build
* 生成的可执行文件叫 main，在 libs/arm64-v8a 里，把它 push 到手机的 /data/local 下
* 给它可执行权限： `adb shell chmod 777 /data/loca/main`

#### 2. gdb 的 target remote， gdbserver
* gdbserver 是运行于手机上的一个可执行程序
* gdb 就是正常的 gdb
* 。。。。。
* 不用start。。。
* 我们会发现，gdb 虽然能连接上手机里的被调程序，但 gdb 告诉我们它无法抽取符号表：

		Reading symbols from target:/data/local/main...(no debugging symbols found)...done.
* 这是 ndk 的问题

#### 3. ndk 编译时会自动去掉符号表的问题
* 注意，nkd 在编译时，最后默认会做 strip，把符号表从可执行文件中移除，即便你加了参数 NDKDEBUG=1
* 用 V=1 参数可以看到 ndk 在编译的过程中都干了些什么：

		ndk-build V=1 NDKDEBGU=1

* 输出结果如下：

		[arm64-v8a] Compile        : main <= main.c
		D:/ProgramFiles/android-ndk-r10d/toolchains/aarch64-linux-android-4.9/prebuilt/windows-x86_64/bin/aa
		rch64-linux-android-gcc -MMD -MP -MF ./obj/local/arm64-v8a/objs/main/main.o.d -fpic -ffunction-secti
		ons -funwind-tables -fstack-protector -no-canonical-prefixes -O2 -g -DNDEBUG -fomit-frame-pointer -f
		strict-aliasing -funswitch-loops -finline-limit=300 -Ijni -DANDROID -g -Wa,--noexecstack -Wformat -W
		error=format-security -fPIE    -ID:/ProgramFiles/android-ndk-r10d/platforms/android-21/arch-arm64/us
		r/include -c  jni/main.c -o ./obj/local/arm64-v8a/objs/main/main.o
		[arm64-v8a] Executable     : main
		D:/ProgramFiles/android-ndk-r10d/toolchains/aarch64-linux-android-4.9/prebuilt/windows-x86_64/bin/aa
		rch64-linux-android-g++ -Wl,--gc-sections -Wl,-z,nocopyreloc --sysroot=D:/ProgramFiles/android-ndk-r
		10d/platforms/android-21/arch-arm64 -Wl,-rpath-link=D:/ProgramFiles/android-ndk-r10d/platforms/andro
		id-21/arch-arm64/usr/lib -Wl,-rpath-link=./obj/local/arm64-v8a ./obj/local/arm64-v8a/objs/main/main.
		o -lgcc -no-canonical-prefixes  -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -fPIE
		-pie   -lc -lm -o ./obj/local/arm64-v8a/main
		[arm64-v8a] Install        : main => libs/arm64-v8a/main
		copy /b/y ".\obj\local\arm64-v8a\main" ".\libs\arm64-v8a\main" > NUL
		D:/ProgramFiles/android-ndk-r10d/toolchains/aarch64-linux-android-4.9/prebuilt/windows-x86_64/bin/aa
		rch64-linux-android-strip --strip-unneeded  ./libs/arm64-v8a/main

* 注意其中的最后一行，ndk 调用了 strip，传的参数是 --strip-unneeded，这会去掉可执行程序中的符号表，以至于 gdb 无法抽取符号表。
* 解决方式： 不用 strip 后的，去 `obj\local\arm64-v8a` 里把 strip 之前的可执行文件推到手机里运行、调试。
* 用 readelf -s 验证是否有符号表：
	* MSYS2 带了 readelf。-s 选项是查看符号表，
	* 被 strip 过的可执行文件没有 '.symtab'表，只有 '.dynsym' ，输出结果如下：

			$ readelf.exe  -s main
			
			Symbol table '.dynsym' contains 17 entries:
			   Num:    Value          Size Type    Bind   Vis      Ndx Name
			     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
			     1: 0000000000000200     0 SECTION LOCAL  DEFAULT    1
			     2: 0000000000011000     0 SECTION LOCAL  DEFAULT   17
			     3: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 _bss_end__
			     4: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 __bss_start__
			     5: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 __bss_end__
			     6: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 __bss_start
			     7: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 __end__
			     8: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 _edata
			     9: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 _end
			    10: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND printf
			    11: 0000000000010da8     8 OBJECT  GLOBAL DEFAULT   14 __FINI_ARRAY__
			    12: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_init
			    13: 0000000000010d98     8 OBJECT  GLOBAL DEFAULT   13 __INIT_ARRAY__
			    14: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __cxa_atexit
			    15: 00000000000005c0    40 FUNC    GLOBAL DEFAULT    8 main
			    16: 0000000000010d88     8 OBJECT  GLOBAL DEFAULT   12 __PREINIT_ARRAY__

	* 没有被 strip 过的可执行文件，输出结果如下：

			$ readelf.exe -s main
			
			Symbol table '.dynsym' contains 17 entries:
			   Num:    Value          Size Type    Bind   Vis      Ndx Name
			     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
			     1: 0000000000000200     0 SECTION LOCAL  DEFAULT    1
			     2: 0000000000011000     0 SECTION LOCAL  DEFAULT   17
			     3: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 _bss_end__
			     4: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 __bss_start__
			     5: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 __bss_end__
			     6: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 __bss_start
			     7: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 __end__
			     8: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 _edata
			     9: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 _end
			    10: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND printf
			    11: 0000000000010da8     8 OBJECT  GLOBAL DEFAULT   14 __FINI_ARRAY__
			    12: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_init
			    13: 0000000000010d98     8 OBJECT  GLOBAL DEFAULT   13 __INIT_ARRAY__
			    14: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __cxa_atexit
			    15: 00000000000005c0    40 FUNC    GLOBAL DEFAULT    8 main
			    16: 0000000000010d88     8 OBJECT  GLOBAL DEFAULT   12 __PREINIT_ARRAY__
			
			Symbol table '.symtab' contains 61 entries:
			   Num:    Value          Size Type    Bind   Vis      Ndx Name
			     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
			     1: 0000000000000200     0 SECTION LOCAL  DEFAULT    1
			     2: 0000000000000218     0 SECTION LOCAL  DEFAULT    2
			     3: 0000000000000270     0 SECTION LOCAL  DEFAULT    3
			     4: 0000000000000408     0 SECTION LOCAL  DEFAULT    4
			     5: 00000000000004c0     0 SECTION LOCAL  DEFAULT    5
			     6: 0000000000000520     0 SECTION LOCAL  DEFAULT    6
			     7: 0000000000000570     0 SECTION LOCAL  DEFAULT    7
			     8: 00000000000005c0     0 SECTION LOCAL  DEFAULT    8
			     9: 0000000000000660     0 SECTION LOCAL  DEFAULT    9
			    10: 0000000000000668     0 SECTION LOCAL  DEFAULT   10
			    11: 0000000000000680     0 SECTION LOCAL  DEFAULT   11
			    12: 0000000000010d88     0 SECTION LOCAL  DEFAULT   12
			    13: 0000000000010d98     0 SECTION LOCAL  DEFAULT   13
			    14: 0000000000010da8     0 SECTION LOCAL  DEFAULT   14
			    15: 0000000000010db8     0 SECTION LOCAL  DEFAULT   15
			    16: 0000000000010fa8     0 SECTION LOCAL  DEFAULT   16
			    17: 0000000000011000     0 SECTION LOCAL  DEFAULT   17
			    18: 0000000000000000     0 SECTION LOCAL  DEFAULT   18
			    19: 0000000000000000     0 SECTION LOCAL  DEFAULT   19
			    20: 0000000000000000     0 SECTION LOCAL  DEFAULT   20
			    21: 0000000000000000     0 SECTION LOCAL  DEFAULT   21
			    22: 0000000000000000     0 SECTION LOCAL  DEFAULT   22
			    23: 0000000000000000     0 SECTION LOCAL  DEFAULT   23
			    24: 0000000000000000     0 SECTION LOCAL  DEFAULT   24
			    25: 0000000000000000     0 SECTION LOCAL  DEFAULT   25
			    26: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS crtbegin.c
			    27: 00000000000005e8     0 NOTYPE  LOCAL  DEFAULT    8 $x
			    28: 0000000000010da8     0 NOTYPE  LOCAL  DEFAULT   14 $d
			    29: 0000000000010d98     0 NOTYPE  LOCAL  DEFAULT   13 $d
			    30: 0000000000010d88     0 NOTYPE  LOCAL  DEFAULT   12 $d
			    31: 0000000000011000     0 NOTYPE  LOCAL  DEFAULT   17 $d
			    32: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS crtbrand.c
			    33: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS
			    34: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS main.c
			    35: 00000000000005c0     0 NOTYPE  LOCAL  DEFAULT    8 $x
			    36: 0000000000000660     0 NOTYPE  LOCAL  DEFAULT    9 $d
			    37: 0000000000000694     0 NOTYPE  LOCAL  DEFAULT   11 $d
			    38: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS
			    39: 0000000000010db8     0 OBJECT  LOCAL  DEFAULT  ABS _DYNAMIC
			    40: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 _bss_end__
			    41: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 __bss_start__
			    42: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 __bss_end__
			    43: 0000000000000608    64 FUNC    LOCAL  DEFAULT    8 do_arm64_start
			    44: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 __bss_start
			    45: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 __end__
			    46: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 _edata
			    47: 0000000000010fd8     0 OBJECT  LOCAL  DEFAULT  ABS _GLOBAL_OFFSET_TABLE_
			    48: 0000000000011008     0 NOTYPE  LOCAL  DEFAULT   17 _end
			    49: 0000000000000590     0 NOTYPE  LOCAL  DEFAULT    7 $x
			    50: 00000000000005f0    24 FUNC    GLOBAL HIDDEN     8 __atexit_handler_wrap                         per
			    51: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND printf
			    52: 0000000000010da8     8 OBJECT  GLOBAL DEFAULT   14 __FINI_ARRAY__
			    53: 0000000000011000     8 OBJECT  GLOBAL HIDDEN    17 __dso_handle
			    54: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __libc_init
			    55: 00000000000005e8     8 FUNC    GLOBAL HIDDEN     8 _start
			    56: 0000000000010d98     8 OBJECT  GLOBAL DEFAULT   13 __INIT_ARRAY__
			    57: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND __cxa_atexit
			    58: 00000000000005c0    40 FUNC    GLOBAL DEFAULT    8 main
			    59: 0000000000000648    24 FUNC    GLOBAL HIDDEN     8 atexit
			    60: 0000000000010d88     8 OBJECT  GLOBAL DEFAULT   12 __PREINIT_ARRAY__


### 三. gdb 调试 Android 上的 app


---
### ndk-gdb
* Android 的 native 程序本质上是运行在 Linux 上的 C/C++ 程序，所以用 gdb 调试。
* Google 在 NDK 套件里提供了一个 gdb，只不过不是很好用。（起码到目前为止，2017年3月23，NDK-r13b，Windows 版）
* Google 提供的这个 ndk-gdb，说白是在 gdb 上做了一层包装（用 python），目的是让它用起来更简单。
* 如果不看 Google 的这层包装，其实 ndk-gdb 的本质是：
	* 运行在 host 上的 gdb：host 可以是 Windows 也可以是 Linux。对于 Windows 来说，它就是 google 随 NDK 一起提供的 gdb.exe。
	* 运行在 Android 设备上的 gdbserver，用于跟 host 通信。gdbserver 是一个 google 随 NDK 提供的可以在 Android 上执行的命令行程序。
	* 运行在 Android 设备上的被 debug 的进程，大多数情况下即你做的apk
* 如果不适用 Google 提供的这层包装，我们可以手动用 gdb 调试（以 NDK r13b 为例）：
	* 编译你工程的 native 部分时，加上 NDKDEBUG=1 选项： 
		* `ndk-build NDKDEBUG=1`
	* 然后把你的工程打包成 apk，安装到手机。假设你的apk包名叫做 `com.yourpackage.xxx`
	* 启动该 app，并查询出进程号，以备后续启动 gdbserver 使用：
		* `adb shell ps | findstr "com.yourpackage"`，假设查到的 PID 为：6844，后面要用
	* 把 gdbserver push 到手机上，gdbserver 在 NDK 的这里： `prebuilt/android-arm64/gdbserver/gdbserver`： 
		* `adb push NDK-ROOT/prebuilt/android-arm64/gdbserver/gdbserver /data/data/com.yourpackage.xxx/gdbserver`
		* `adb shell chmod 777 /data/data/com.yourpakage.xxx/gdbserver`
	* 把PC上的 TCP 端口映射到手机上的 TCP 端口，为 gdb 和 gdbserver 通信做准备：
		* `adb forward tcp:6669 tcp:5039`
		* 注意：端口号可以自选，写在前面的是 PC 上的端口号，写在后面的是手机上的。需要记住这2个端口号，后续启动 gdbserver 时需要指定手机上的端口号，启动 gdb 时需要指定PC上的端口号。
	* 启动 gdbserver:
		* `adb shell /data/data/com.yourpackage.xxx/gdbserver ：5039  --attach 6844`
	* 注意，必须让 gdbserver 监听某个端口，这个端口就是之前我们映射的 tcp:5039
	* 注意，启动时必须让 gdbserver attach 到某个进程上，就是我们之前 ps 出来的被 debug 进程。
	* 启动 gdb:
		*  gdb.exe 在这里：`android-ndk-r13b\prebuilt\windows-x86_64\bin`
		* `gdb.exe`,进入 gdb 命令行
		* `target remote 127.0.0.1:5039` 连接设备上的 gdbserver
	* 至此，大功告成，终于可以在电脑上 gdb 手机上的程序了。不过 gdb 连上 gdbserver 后报了一大堆警告，说找不到库或者符号之类的。
	* 如果是找不到符号，肯定是你使用了现成的的库，而不是你从源码编译出来的，所以这些库在编译的时候没有加 debug 选项，自然找不到符号。
	* 如果是找不到库，可能是环境变量没有设。
* Google 为了大家使用方便，把以上步骤合并入了一个 python 程序，ndk-gdb.py，并且为该程序做了很多健壮性的工作，所以这个 python 程序也不是很小（包含了 gdbrunner 模块，adb 模块 这两个 google 自己写的模块）
* 理想情况下，你只需要跑到你的项目根目录下（即含有 AndroidManifest.xml 文件和 jni 文件夹的那个目录），运行一下 ndk-gdb 就万事大吉了。
* 但现实情况是有些坑。

#### 怎么定位 ndk-gdb 里的坑
* r10d 里的启动脚本是 ndk-gdb-py.cmd，r13b 变成了 ndk-gdb.cmd
* 运行 ndk-gdb-py.cmd/ndk-gdb.cmd 的时候，加上 --verbose 参数，能看到一些输出
* 根据这些输出去 ndk-gdb.py (r10d在NDK根目录，r13b在 `prebuilt\windows-x86_64\bin` 里) 里找到出错的点，一点一点的跟

#### Google ndk-gdb 的坑
##### r10d
* r10d，Windows x64 版本的 NDK。它提供的 gdb.exe 在 Windows 上无法运行，这是最本质错误：
	* `android-ndk-r10d\toolchains\aarch64-linux-android-4.9\prebuilt\windows-x86_64\bin\aarch64-linux-android-gdb.exe` 
	* 运行以上程序，Windows 弹框说无法启动，缺少 libpython2.7.dll。 这也正常，毕竟 google 提供的 python 脚本启动 gdb.exe 时是不会缺少这个dll的。
	* 把 libpython2.7.dll 拷贝过来再运行，libpython2.7.dll 在 `android-ndk-r10d\prebuilt\windows-x86_64\bin` 里
	* 还是运行不起来，弹框提示无法正常启动（0x000007b），所以放弃。
* 在本质错误之上，google 包的 python 脚本也有很多不妥之处，经过一系列修改，最后才发现是 gdb.exe 无法运行，让人崩溃

##### r13b
* r13b，Windows x64 版本的 NDK。google 包的 python 脚本有问题，导致 gdbserver 无法启动。修改之后可以成功使用。

#### NDK r13b 默认使用 clang 作为编译器了，而不再是 gcc
* 刚切换到 NDK r13b 的时候，可能需要修改一些代码，如果 clang 编译报错的话