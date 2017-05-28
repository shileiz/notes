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