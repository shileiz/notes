## 基础

### `#` 开头的指令
* `#` 开头的指令有好几个，这些指令都是在**预处理阶段**被处理的。
* `#` 空指令，没有任何效果
* `#include` 包含一个源代码文件
* `#define`  定义宏
* `#undef`   取消已经定义的宏
* 其他还有  `#if`， `#ifdef`，`#ifndef` 等等    

### `#define` 命令
* `#define` 是宏定义命令，用 `#define` 定义的宏将在**预处理阶段**替换。
* `#define` 定义的宏仅作单纯的字符串替换。

### `#define` 命令有两种格式
1. 简单的宏定义： `#define  <宏名>  <字符串>`
2. 带参数的宏定义: `#define  <宏名>(<参数表>)  <宏体>`

### 简单的宏定义
* 例1：基本用法

		#define MYMAC int a = 0;
		int main()
		{
		    MYMAC
		    return 0;
		}
用 gcc 进行预处理：`gcc -E -o main.i main.c`  
查看预处理后的 main.i :

		int main()
		{
		    int a = 0;
		    return 0;
		}


* 例2：容易出错的地方

		#define N 2+2 

		int main()
		{
			int a = N*N;
			printf("a=%d\n",a); //a=8
			system("pause");
			return 0;
		}

		/*
			预处理阶段 int a = N*N; 被替换成：int a = 2+2*2+2; 
		*/

### 带参数的宏定义
* 例1：基本用法（多行可以在每一行的结尾加上续行符 \，最后一行不要加）

		#define A(x,y,z) x=1; \
		y=2;\
		z=3;
		#include <stdio.h> 
		int main()
		{
			int a, b, c;
			A(a,b,c);
			printf("%d,%d,%d\n", a, b, c); // 1,2,3
			system("pause");
			return 0;
		}

### 宏运算符：`##`、`#@`、`#` (不常用)
* `##`：预处理程序把出现在##两侧的参数合并成一个符号。

		#define NUM(a,b,c) a##b##c
		#define STR(a,b,c) a##b##c
		void main()
		{
			printf("%d\n", NUM(1, 2, 3));  // 123
			printf("%s\n", STR("aa", "bb", "cc")); // aabbcc
			system("pause");
		}
* `#@`：预处理程序把出现在`#@`右侧的参数加一个单引号。

		#define CHAR(a) #@a
		void main()
		{
			printf("%d\n", CHAR(1)); // 49, 1的 ASCII 码
			system("pause");
		}
* `#@`：预处理程序把出现在`#`右侧的参数加一个双引号。

		#define STR(a) #a
		void main()
		{
			printf("%s\n", STR(123)); // 123
			system("pause");
		}

## 条件编译
* 条件编译影响的是**预处理阶段**，即在‘编译’之前来决定哪些代码段参加编译，哪些不参加。

		#define DEBUG 0
		main()
		{
		     #if DEBUG
		     printf("Debugging\n");
		     #endif
		     printf("Running\n");
		}
用 gcc 进行预处理：`gcc -E -o main.i main.c`  
查看预处理后的 main.i :

		main()
		{
		
		
		
		     printf("Running\n");
		}