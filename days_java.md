#Java的排序： Arrays类的静态方法。以及array和容器的区别。
* 对Array排序：java.util.Arrays 类有一个静态方法 sort()  
这个方法能对一个 Array 进行排序，只要你这个 Array 里的元素都实现了接口 Comparable （必须重写了抽象方法 compareTo()）  
传入的是列表的引用，所以不用返回值就能直接影响传进去的参数。
File 类是实现了 Comparable 接口的，官方API文档里：  
int	compareTo(File pathname)  Compares two abstract pathnames lexicographically.  
所以想对一个文件列表排序，只需要 Arrays.sort(file_list) 即可。  
* Arrays类的所有方法都是静态方法，这个类存在的意义就是提供一些类似排序、搜索的方法，用以操作一个Array。
* Arrays 这个类，注意，Java 里的数组，即用 [] 定义的变量，不是 Arrays 类型的, 而是一个 Array。Java 的Array被翻译成数组。
* Java的Array是Java语法上的一种‘类型’，他不是一个类的子类，起码不是一个java程序员可用的类的子类。
* Java的Array是用[]定义的变量，它在java内部实际上有自己的类，但是没有对外开放
* 不要搞混'容器类'的那些类和Array
* Arrays的静态方法是操作Array的，不是操作'容器'的
* 把容器转为Array用Collection.toArray()，把Array转为容器用Arrays.asList()
* 在使用 array-based API 和 collection-based API 之前要做好转换工作。  
* Arrays的静态方法，都是array-based API，即其接收的参数都是array，而不是容器。
  
		import java.util.Arrays;
		import java.util.ArrayList;
		public class ArrayClass{
		public static void main(String[] args){
			String str = "str";
			String[] str_array={"abc","123"};
			int[] int_array = {1,2,3};
			ArrayList array_list = new ArrayList();
			//Arrays arr = new Arrays(); // 报错，无法new 一个 Arrays 出来，这个类的文档中也没有介绍构造方法。
			System.out.println("str.getClass():"+str.getClass());  // java.lang.String
			System.out.println("str_array.getClass():"+str_array.getClass());    // [Ljava.lang.String;
			System.out.println("int_array.getClass():"+int_array.getClass());    //  [I
			System.out.println("array_list.getClass():"+array_list.getClass());    //  java.util.ArrayList
			}
		}  

#Java基础复习
###Thread
* 启动线程的两种方法：
	1. 定义一个类，继承 Thread，重写 run() 方法。然后在主函数里 new 一个这个类，调用它的 start() 方法
	2. 定义一个类，实现 Runnable 接口，重写 run() 方法。然后在主函数里 new 一个这个类，再 new 一个 Thread 类。把这个类的实例做为 Thread 类构造函数的参数传进去，然后调用 Thread 实例的 start() 方法。  
* 实现接口的方式比较好，因为继承不支持多继承，而接口可以实现多个。  
* Thread 是一个实现了 Runnable 接口的类  
* Thread(Runnable r) 这个构造方法以一个 Runnable 对象为参数   
* Thread.start() 将创建一个新的线程，并在线程里执行这个 Thread 的 run() 方法。（直接调 run() 方法不会创建新的线程，是不行的。）  
###File类与IO类 
* File类和IO类都在 java.io 中
* File 类只是内存里一个代表文件或路径的类，仅可以创建/删除/检测存在性等等
* File类不能对文件内容进行读写。读写要用 IO 类。 IO 类是个统称，负责IO的有很多个类，其中有一部分是负责对文件进行 IO 的。
* IO 类按照功能可以分为：节点流/处理流  
* 所谓节点流，是直接把程序连接到输入/输出目标上的，比如文件，网络，控制台等等
* 所谓处理流，是把程序连接到另外的流上，对上一个流进行处理用的，比如Buffering，Filtering，把字节转换成字符等等
* IO 类还可以分为处理字节的IO类和处理字符的IO类
* InputStream/OutputStream 的子类是字节流；Reader/Writer 的子类是字符流。
* 字节流的 read()/write() 参数是字节或者字节数组，字符流的 read()/write() 参数是字符或者字符数组或者字符串
* 最后，Input/Reader 是负责读的IO类，Output/Writer是负责写的IO类
* FileOutputStream 在没有文件的时候，将会自动创建该文件
* File 和包。被打到包里的类，如果操作 File，其路径的起始点是以包的最外层所在路径开始的，而不是以编译出来的 class 文件。 
###IO类的read()，为什么可以用返回值是否-1来判断是否到了文件尾
java.io 中 InputStream.read()  
对于无参数的read()，其原型是：int read()  
官方文档的描述是：返回读到的那个字节。如果读到了尾，则返回-1。  
问题是，万一读到的那个字节本身就是-1，那岂不是让调用者误以为已经到了尾呢？  
注意：读的是字节，返回的是int。  
背景知识：  
直接把一个byte强转为int，是可能出现负数的。  
因为强转的时候，就是简单的把这个byte的最高位扩展为int的高24位。所以如果byte的最高位是1，则转成的int必然是负数。  
下面做一个实验，把一个最高位为1的byte转换成int，看看结果是啥：  
因为 java 里没办法直接给一个byte按位赋值，比如赋值成10000000这种。  
只能把一个int赋值给byte：  
byte b = -128; // 给byte型变量b赋值为 10000000，相当于 int i = -128; byte b = (byte)i;  
赋值过程相当于隐含了强转，把int转成了byte。  
java的编译器只允许把-128~127范围的整数强转为byte。因为8bit只能表示这么多数。  
byte b = 128; //编译通不过，错误: 不兼容的类型: 从int转换到byte可能会有损失  
记住：  
int强转byte的过程就是把int的高24位直接不要了，留下低8位给byte。  
byte强转int的时候，是取byte的最高位来做位扩展，把8位的byte扩展为32位的int。  
下面的程序来证明上述观点： 
 
	import java.io.*;
	public class TestByteInt{
		public static void main(String[] argv){	
			try{
				int i_128 = -128;          // -128是25个1和7个0,即十六进制的ffffff80，下面用printf()打印出来看看
				System.out.printf("int型变量-128是：%x\n",i_128);  // ffffff80。这里要用printf("%x")，而不能用println()，因为println()会把byte转成int打印。
				byte b_128 = (byte)i_128;    // int转byte，就是把前面24位去掉，留下最后8位。即去掉24个1，留下10000000，即十六进制的80。
				System.out.printf("int转成的byte是：%x\n",b_128);  // 80。	
				FileInputStream fis = new FileInputStream("./byte_11111111.txt"); 
				// 为了证明byte转int的原理，我们从文件读入一个byte，而不用从int转换而来的byte，这样更有说服力。
				// 文件byte_11111111.txt里面，我们只写了一个字节，这个字节是8位全1。用notepad++的Hex-Editor插件可以做出这种文件。
				byte[] b={1};                         		
				fis.read(b);      // read(byte[] b) 是把读入的一个byte放入b里
				byte b0 = b[0];
				System.out.printf("从文件读入的一个byte是：%x\n",b0);   // 读入的确实是8个1，即十六进制的ff
				int i0 = (int)b0;     // byte转int时，按照byte的最高位扩展，8个1被扩展成32个1，即十六进制的ffffffff，即十进制的-1。
				System.out.printf("byte转成的int是：%x\n",i0);  // ffffffff
				System.out.println(i0);        // println()直接打印这个数的十进制形式，即-1
			}
			catch(Exception e){}
		}
	}
背景知识结束。  
回到刚才的问题，万一读到的那个字节本身就是-1，那岂不是让调用者误以为已经到了尾呢？  
根据背景知识里的例子，如果读到的字节是8个1，那么转成int就是-1啊，这个问题就出来了。  
但实际上是不会发生这种问题的。因为这个函数返回的int是把读到的byte作为最末尾8位，高24位补0得到的。而不是像强转那样按最高位做扩展。  
所以这个方法的返回值肯定不会有负数出现。这种转换得到的int，可以随便用强转来还原成byte。  
以下程序证明了上述观点：

	import java.io.*;
	public class TestReadByte{
		public static void main(String[] argv){	
			try{
				FileInputStream fis = new FileInputStream("./byte_11111111.txt"); // 文件中写了一个字节：8位1
				int i = fis.read(); // 此 read() 返回的int并不是直接把读到的byte转换成了int，那样的话会得到-1（即32个1，ffffffff）
				System.out.printf("read()读取8个1后，返回的int为：%08x\n即十进制数：%d\n",i,i);  
				// 000000ff。结果是把byte前24位补0，即000000ff。即十进制数255。
				//但printf()会把没有意义的高位的0省略，为了不让他省略，这里加入了格式化符号%08x
				fis.close();
			}
			catch(Exception e){}
		}
	}
即：int read() 返回的int不是强转（即按最高位扩展）来的，而是高24位补0得来的，所以不可能有负数。  
另外，最高24位补0，相当于按最高位扩展后，再跟0x000000ff取与。这个 int read() 可能就是这么实现的最高位补0——强转后跟0xff取与。  


#####与C/Python不同  
java的Input类都有read()方法，其实是一系列重载的read()方法，有带参数的有不带参数的。都有返回值int。    
java的read()的返回值    
* 对于无参的read()，返回值代表着读入的数据，无论读入的是byte还是char，都被转换成了int返回了，需要用的时候还得还原成byte或者char。  
	- 这里要注意不是简单的强转，而是可还原的转换。保证转不出负数来，并且可以把转成的int的用强转还原成原来的类型。  
* 对于有参数的read(),返回值代表读入数据的个数。比如多少个字符，或者多少个字节。而数据就直接读到参数里了。  
* 无论有参无参的，当返回值为-1时，都代表着读到尾了。  
python2.x的read()，是file-like object的方法，它是用参数来指定读多少字节出来，或者不写参数就是全部读出来。  
返回值是python2.x的str，也就是byte sequence。（python2.x 的str实际上byte sequence）  
python2.x 的read()底层调用的就是C的fread()  
C语言的fread()  
头文件：#include <stdio.h>  
函数原型：size_t fread (void *buffer, size_t size, size_t count, FILE *stream);  
参数：  
buffer  用于接收数据的内存地址  
size  要读的每个数据项的字节数，单位是字节  
count  要读数据项的个数，每个数据项size个字节.  
stream  输入流的指针  
返回值：  
返回实际读取的元素个数。如果返回值与count不相同，则可能文件结尾或发生错误。  

###关于包
* 编译/使用 一个打了包的 java 源文件  
用 package 语句打了包的 java 源文件不放在包路径下，也能编译成功  
但编译出来的 class 文件必须放到包路径下，这个 class 才能被别的类使用  
被别的类使用，使用者要先 import 这个类才行  
想要 import 成功，必须保证被 import 包的最顶层所在目录位于 classpath 上  
（比方你想 import com.real.test.Cat，那么必须保证 com 所在的目录，在 classpath 上  ）
* 运行一个打在包里的 class 文件  
首先保证 Cat.class 在 .\com\real\test\ 里面了，用以下命令执行：  
C:\Users\Administrator\Desktop>java com.real.test.Cat  
在其他目录下运行这个 class 文件，还要先把被执行的 class 所在包的顶层目录所在目录放到 classpath 里  
或者用 -cp 指定 classpath  
D:\>java -cp C:\Users\Administrator\Desktop com.real.testjava.Cat   
注意，java.exe 不是以 class 文件作为参数的，你不能 java C:\Users\Administrator\Desktop\com\real\testjava\Cat.class  
* jar 包  
.jar 和 .zip 文件，当成文件夹看待就可以了，里面放的都是包的顶层目录，比如 com 啥的  
比如 C:\Users\Administrator\Desktop 里有个 com 文件夹，是 Cat.class 的包的顶层文件夹  
那么你需要把 C:\Users\Administrator\Desktop 加入 classpath  
如果 C:\Users\Administrator\Desktop\real.jar 里有个 com 文件夹，是 Cat.class 的包的顶层文件夹  
那么你需要把 C:\Users\Administrator\Desktop\real.jar 加入 classpath  
rt.jar 是 JDK 为我们提供的运行时库。java.lang, java.io, java,util 等很多常用包都在这里  
打包成 jar 包的命令行  


		jar -cvf 目标jar文件.jar *.*  // -c 创建新文件; -v  在标准输出中生成详细输出; -f  指定档案文件名

###String类
* Java 中，除了 boolean，char，byte 和 5种数字（int float long short double）是基本数据类型外(即四类八种)，其余全是类，包括 String
* String 类比较特殊，String变量可以 new 出来，也可以用字符串常量初始化；其他类只能 new 出来。
* new 出来的 String 对象在堆区，字符串常量在 data 区 
 
		String s1 = "hello"; //用字符串常量初始化
		String s2 = new String("hello"); // new 出来
* String 的不可变性  
虽然 s1 在初始化之后可以改变，但实际上是让 s1 重新指向了其他的内存区域，而不是把 s1 原本指向的内存区域的内容改变了。  
String 的不可变性并不影响对 String变量进行诸如 s1+=";" 等操作，只是说这些操作实际上都进行了内存重新分配，s1 重新指向的操作，相对来说比较低效而已。  
而 StringBuffer 是可变的字符序列，对它进行重新赋值不用重分内存，相对高效。StringBuffer 有 append() 方法，比 String+="xxxx" 高效。  
并且 StringBuffer 有 insert(int index，String s) 方法
#Java遍历文件夹
Java7 之前， 没有像 Python 中的 walk() 这种方法，直接遍历文件夹  
自己用递归实现一下也并不复杂  
Java7 提供了一个方法实现对文件夹的遍历：java.nio.file.Files.walkFileTree(Path start,FileVisitor<> visitor)  
参考：  
http://www.java3z.com/cwbwebhome/article/article8/83562.html?id=4655  
http://www.cnblogs.com/kgdxpr/archive/2013/07/13/3187575.html  
#字符编码问题
* 字符编码的问题在 第02章\_基础语法/01\_标识符\_关键字\_数据类型\_1 的最后，讲 char 类型的时候说到了。
* 只说了 Java 内部采用 UTF-16 编码，一个 char 变量占用2个字节，并且 char 型变量可以用 '\u534e' 这种 utf-16 编码直接赋值
* Java 把字符串写到文件的时候，采用的是什么编码？  
首先，写字符串要用字符流（而不是字节流），即 Writer 的子类。  
OutputStreamWriter 就是 Writer 的子类（是个抽象类），API 文档里有如下描述：  
Characters written to it are encoded into bytes using a specified charset.   
The charset that it uses may be specified by name or may be given explicitly, or the platform's default charset may be accepted.  
其构造方法就要求第二个参数指定 charset，如果不指定，则使用 platform 的默认 charset  
总之，你把一个字符串按什么编码格式写到文件里，是需要在代码里手动指定的。在不指定的情况下，使用默认的编码格式。

		import java.io.*;
		public class TestWrite{
		
			// 字符流的输出，字符流输入/输出类都有一个属性charset：字符编码 
			// 在 new 的时候可以传给其构造函数
			// 下面这个测试函数,你可以更改构造函数中的 "GBK",比如变成"UTF-8"，就可以以其他编码格式写文件了
			private void write_by_OutputStreamWriter(){
				try{
				OutputStreamWriter writer = new OutputStreamWriter(new FileOutputStream("OutputStreamWriter.txt"),"UTF-8");
				writer.write("中文测试");
				writer.close();
				}
				catch(IOException e){}
			}		
			
			// FileWriter 是一个方便类，它简化了 OutputStreamWriter，它是 OutputStreamWriter 的子类
			// FileWriter 使用默认的字符编码，不能让程序员手动设置了
			// 所谓默认编码，是指运行这个java程序时所在的环境。比如在 console 中用 java xxxx 来运行时，默认编码就是 console 的编码格式
			// 注意，默认编码与 java 源文件使用什么编码格式无关。TestCharset.java 可以证明这一点。
			private void write_by_FileWriter(){
				try{
				FileWriter writer = new FileWriter("FileWriter.txt");
				writer.write("中文测试");
				writer.close();
				}
				catch(IOException e){}
			}
			
			// 字节流的输出，字节流不能按字符输出，只能按字节
			// 你想把一个字符串用字节流输出，必须在输出之前手动把字符串转成字节数组
			// String 有一个方法：  byte[] getBytes(Charset charset) ，能把自己编码成字节串
			private void write_by_FileOutputStream(){
				try{
				FileOutputStream writer = new FileOutputStream("FileOutputStream.txt");
				writer.write("中文测试".getBytes("UTF-8"));
				writer.flush();
				writer.close();
				}
				catch(IOException e){}
			}	
			
			public static void main(String[] args){
				TestWrite tw = new TestWrite();
				//tw.write_by_OutputStreamWriter();
				//tw.write_by_FileWriter();
				tw.write_by_FileOutputStream();
			}	
		}
* 编译 java 源文件时遇到的字符编码问题  
1- 运行 javac 的 console 使用的是 GBK 编码 （中文windows系统默认）  
2- 被编译的 java 源文件用的是 UTF-8 编码 （NotePad++ 默认）  
3- java 源文件里写了挺多中文注释，其中有一些字符无法在 GBK 编码中被理解成任何字符  
4- 这时你运行 javac xxxx.java 的时候就会遇到 “错误: 编码GBK的不可映射字符”   
5- 解决方法，把你的 java 源文件转换成跟 GBK，或者把你的 console 转换成 utf-8  
* 默认编码的问题  
默认编码取决于 JVM，简体中文windows上的JVM，默认编码就是 GBK；Linux默认编码就是 UTF-8  
以下程序证明了这一点  

		/*
			TestCharset.java
			This java source file is encoded by "utf-8"
			But we will run this class in a Chinese-Windows7 Environment
			The output will be "GBK"
			Even you use chcp 65001 to change the console's charset to utf-8
			The output will still be "GBK"
			But when you copy the TestCharset.class file to a Linux
			then the output will be "UTF8"
		*/
		
		import java.io.*;
		public class TestCharset{
		
			public static void main(String[] args) throws IOException{
				FileWriter fwt = new FileWriter("null.text");
				System.out.println(fwt.getEncoding());
			}
		
		}  
1- 默认编码跟编辑 java 源文件使用什么编码无关  
2- 默认编码跟编译 java 源文件时使用的 JVM 和 console 无关  
3- 默认编码只跟运行 java 程序时的 JVM 有关  
ps1- Linux 安装JDK实际上只需要解压和设置环境变量。  
ps2- Linxu 安装JDK如果搞错了 64位 和 x86 版本，当你运行 java 的时候，将会提示 no such file or directory  
ps3- 低版本(1.7)JVM编译出来的class文件可以直接在高版本(1.8)JVM里运行，反之可能有问题  
ps4- JVM 使用的默认编码是可以通过 Java API 获得的：System.getProperty("file.encoding")  
可参考：http://wulinhaoxia5.iteye.com/blog/1450456  
* 类型转换问题  
把一个 char 强制转成 int，无法指定 charset，得到的是 utf-16 对应的数字  
把一个 int 强制转成 char，无法指定 charset，得到的是 utf-16 对应的字符  
这也说明了 java 内部是按照 utf-16 存储 char 型的  

		public class TestConvert{
			public static void main(String[] args){
				// utf-16/utf-32 编码里，‘中’ 的编码为 0x4e2d
				int i = 0x4e2d; 
				char c = '中';
				// 无论 JVM 的默认编码是 GBK 还是 UTF8，都将按照 UTF-16 进行类型转换
				System.out.printf("%x",(int)c);
				System.out.println();
				System.out.printf("%c",(char)i);
			}
		}
#Java杂项
* Java 的格式化输出：Java 也有跟 C 语言一样的 printf：

		System.out.printf("%x",i)