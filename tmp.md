### Eclipse 反编译插件： JadClipse
* 原理：
	* 把 Java 反编译器 jad.exe 通过一个 eclipse plugin jar 集成到 eclipse 里面
	* jad.exe 可以把 .class 文件反编译成 .java 源代码
* 步骤： 
 	* 下载 jad.exe: http://varaneckas.com/jad/ 
	* 把 jad.exe 放在电脑某个目标，比如：D:\ProgramFiles\jad\jad.exe
	* 下载 jadclipse 的 jar 包(net.sf.jadclipse_3.3.0.jar)：sourceforge.net/projects/jadclipse/
	* 把 net.sf.jadclipse_3.3.0.jar 拷贝到 eclipse 的 plugins 目录下，重启 Eclipse
	* 打开 eclipse 的 Preference： Window ---> Preference
	* 在右侧导航的 Java 下，会发现 JadClipes，把 path to decompiler 处填写: D:\ProgramFiles\jad\jad.exe
	* 让 Eclipse 打开 .class 文件时使用 jad
	* General--->Editors--->File Associations
	* 把 .class 和 .class without source 都改成默认用 JadClipse 打开即可

### Ubuntu 切换 jdk 版本
* `sudo update-alternatives --config java`
* `sudo update-alternatives --config javac`
* 以上两条命令是以交互模式来更改 java 和 javac 的默认路径
* 如果要以静默方式，用：`sudo update-alternatives --set java /usr/lib/jvm/java-8-openjdk-amd64/jre/bin/java`
* 以 set 代替 config，并且手写要设置的路径
* 另外，sudo 要提示密码，还是需要交互，避免 sudo 密码引起的交互：
* `echo "yourpassword" | sudo -S update-alternatives --set java /usr/lib/jvm/java-8-openjdk-amd64/jre/bin/java`
* 给 sudo 加个 -S 参数，使其静默。把密码通过 echo 管道传递给 sudo。