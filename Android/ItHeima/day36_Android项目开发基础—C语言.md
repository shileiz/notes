#c的基本数据类型
* C的基本类型只有2类6种，比java少了2类2种。C语言没有byte和boolean这2个基本类型。
* C的2类6种基本类型为（跟java一样）：char类型：char，数字型：int，short，long，float，double  
* 另外C支持signed, unsigned来修饰char和整数。注意小数永远是有符号的，不能用signed/unsigned修饰
* 另外short只能修饰int，short int 简写为 short。
* long 则可以修饰 int，double，和 long。long int 简写为 long。
* 关于各种类型的长度：
>C++标准中只限制规定short int不能超过int的长度，具体长度的可以由C++编译器的实现厂商自行决定。  
>目前流行的32位C++编译器中，通常int占4字节，short int占2字节。一些16位的编译器中，short int和int都是2个字节。  
>类似地，C++标准只限制了long int不得小于int的长度，具体也没有作出限制。  
>目前流行的32位C++编译器中，通常int占4字节，long int也占4字节。一些16位的编译器中，long int占4字节，而int占2个字节。 

* 这与钱能教材的说法不太一样，钱能版教材里把short，long也算作修饰符。其实本质是一样的，只是咱们这种理解法，比较容易跟java对比。
* 另外钱能版教材没有提到 long long 这种类型，根据百度百科显示，C99标准里加入了 long double 和 long long。


##java的基本数据类型长度
* char：2
* short：2
* int：4
* long：8
* float：4
* double：8
* byte：1
* boolean：1

##c的基本数据类型长度（基于32位的windows编译器）
* char：1
* short：2
* int：4
* long：4
* float：4
* double：8

##*的三种用法
1. 乘法
2. int* p：定义一个指针变量p，p中存放一个内存地址，这个地址所存放的数据规定是int型
3. *p：取出p中保存的内存地址存放的数据

##数据传递
* 所有语言所有平台，都只有值传递，引用传递传递的值是内存地址

##栈
* 系统自动分配和释放
* 保存全局、静态、局部变量
* 在栈上分配内存叫静态分配
* 大小一般是固定的
##堆
* 程序员手动分配（malloc/new）和释放（free/java不用手动释放）
* 在堆上分配内存叫动态分配
* 一般硬件内存有多大就有多大
