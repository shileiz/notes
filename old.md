#C语言相关

* strcpy()  
	    	
		char* dst=NULL;  
	    char* src="abc";  
	    strcpy(dest,src);  
这是不行的  
必须这样才行：	

		char* dst=(char*)malloc(sizeof(char)*sizeof(src));

* fprintf() 和 fopen()  

	    FILE *fp=fopen("x.txt","w+");
	    fprintf(fp,"%d",25);
	    fclose(fp);
注意：FILE* 而不是 FILE

C 语言的 FILE 和 Java 的很不一样  
C 的 FILE 在打开的时候就指定了是以什么方式打开：读/写/追加/字节 （w/r/+/b），这跟 python 是类似的  
然后直接用一个函数就往文件里写( fprintf()/fwrite() )，或者从文件里读( fread() )。而 python 则更简单，file object 本身就有 read() 和 write() 方法  
但是 Java 的 File 类，是没有读/写/追加/字节这些属性的。  
决定是写还是读这个文件，取决于你使用 Input 还是 Outp 类来操作这个文件；以字节还是字符方式来读写，取决于你使用 Reader/Writer 还是 Stream 类来操作这个文件。  
Java 是先 new 一个 File 对象，然后想读文件时再 new 一个 Reader/Input 对象来操作这个 File对象，写的话就 new 一个 Writer/Output 对象来操作。

* msize() 函数  
msize()  在标准库里（malloc.h），不用include可以直接用  
msize() 的参数是个指针，这个指针必须是 malloc() 返回的  
mszie() 能得到这个指针所指向的内存区域的大小  
注意： malloc() 在堆内存分配空间并返回一个指针

* 关于VC零星  
VC Project 配置，针对 Debug/Release 是分开的。  
\#include "stdafx.h"   
stdafx.h 里已经包含了 stdio.h 了，就不用自己再包含了  
stdafx.h 里还包含了 tchar.h，在这个头文件里微软定义了 _tmain，#define _tmain  main; 其实 _tmain 就是 main 的别名


# 视频编码相关
* 熵和无损压缩  
熵是衡量样本空间“不确定性”的一个值。一个样本空间越“不确定”，熵越大。  
熵的公式是：-Pi*log2Pi 求和，i从1到n。 其中Pi表示样本空间中第i种基本事件的概率。  
这个值当 P1=P2=...=Pn 时取得最大值：log2n。即当样本空间中所有基本事件等概率时，样本空间的熵最大。  
举个例子：实验扔硬币观察哪面朝上，其样本空间为：{正面朝上，反面朝上}。即此样本空间有2种基本事件：正面朝上，反面朝上。  
如果硬币正反面均匀，则P1=P2=1/2，此时样本空间的熵为1。  
如果硬币正反面不均匀，P1=2/3, P2=1/3, 则此样本空间的熵为0.92  
参考：http://episte.math.ntu.edu.tw/articles/mm/mm_13_3_01/  
对于图像，熵表示这幅图像的平均编码长度。即平均每个像素用几个bit来表示。  
对一幅图像来说，Pi 表示值为 i 的灰度，在图像中出现的次数。  
如果图像的每个像素出现的次数都一样，那么图像的熵达到最大值，那么每像素需要8bit来表示。  
灰度图像的熵参考这里：  
http://blog.csdn.net/tianguokaka/article/details/6544006  
熵编码属于无损压缩，哈夫曼编码是熵编码的一种。  
熵编码包括：香浓-范编码、哈夫曼编码、算数编码RLE编码。  
* 杂项  
x264.exe 的 --verbose 能打印出每一帧的信息，包括大小。  
ffmpeg 解出来的帧大小比x264.exe编码时打印的实际大小要多16bytes

#cmd命令行相关
* cmd的 > 或者 >> 只是重定向的 stdout，而不包括 stderr。 像x264.exe所有打印到屏幕的都是stderr，>是不能把其输出重定向到文件的。
* windows里2代表stderr，1代表stdout，所以要用 2>xxx.log 来把stderr重定向到文件。而1>可简写为>，所以平时只写>就可以了。
* Linux跟windows是一样的，用>重定向stdout，用2>重定向stderr。>>表示追加方式，windows和linux也是一样。
* 如果想把stdout和stderr同时重定向到文件，需要：>xxx.log 2>&1。其中 2>&1 表示把stderr重定向到stdout中。
* 对于同时重定向stdout和stderr，Linux有一种简写：>&xxx.log。注意，windows不支持这种简写。
