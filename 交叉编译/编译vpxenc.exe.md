#### 目标 ####
* 生成windows上可用的vp9编码程序，vpxenc.exe
#### 一些说明 ####
* vp9 官网地址：http://www.webmproject.org/
* 官网上不提供任何平台的编译好的可执行程序的下载，所以要自己下载源码自己编
* 官网上提供git的clone地址：https://chromium.googlesource.com/webm/libvpx
* 官网上说了，想build clone下来的源码，需要你熟悉并且安装了一个Unix-Like Environment
* 官方代码的configure选项里，提供了很多交叉编译的选项，用`--target=`来指定目标平台
* 有了这个就好办了，我们就用这个选项来做Windows上的编译
#### 总体思路 ####
* 官方git上下载源码
* 在Linux上（可以搞个虚拟机）运行`./configure --target=x86-win32-vs10`
* 在Linux上运行`make`，生成vs2010可用的工程文件
* 把生成的vs2010的工程文件拷贝到Windows上，用vs2010打开.sln文件，在IDE里编译生成Windows可执行程序
#### 详细过程 ####
* 克隆源码　　
	* 在Windows上打开git bash，运行 `git clone https://chromium.googlesource.com/webm/libvpx`
	* 说明：因为我的Linux虚拟机连不上VPN，无法翻墙，所以只能在Windows上下载源码
	* 说明：可能是我Windows版的git配置问题，下载下来的文件都是dos格式的，导致sh、pl脚本在Linux执行不了
	* 为了解决dos文件格式的问题，在代码根目录执行 `dos2unix.exe *`，再cd到`./build/make`里执行`dos2unix.exe *`
*  在Linux上生成vs2010可用的工程文件
	* 把代码拷贝到Linux机器上  
	* 说明：可能还是windows版的git问题，代码拷贝到Linux上以后，./configure 和一些其他脚本文件都失去了可执行权限
	* 为解决这个问题，分别执行：`chmod +x ./configure`，`chmod +x ./build/make/*.sh`，`chmod +x ./build/make/*.pl`
	* 运行 `./configure --target=x86-win32-vs10`
	* 说明：vs10对应vs2010，根据你用的vs不同版本自行选择
	* 说明：`./configure --help` 可以看到所有可用的target
	* 运行 `make`，完成后会在当前目录生成 vpx.sln 等一系列vs2010的工程文件
	* 把运行完make的整个目录都拷贝回Windows上
* 用vs2010生成可执行程序vpxenc.exe
	* 双击vpx.sln，用vs2010打开工程（实际上应该叫解决方案）
	* 在项目vpxenc上右键--->属性--->配置属性--->C/C++--->附加包含目录，把原来Linux的目录改成相应的Windows的
	* 比如，把 `/root/libvpx/vp8` 改成 `D:\vpx\libvpx-aftermake\libvpx\vp8`
	* 去下载一个yasm：http://yasm.tortall.net/Download.html
	* 我下载的是：for general use on 64-bit Windows 那个 exe
	* 下载完了把文件名改成yasm.exe，扔到 C:\Windows\SysWOW64 里
	* 在项目vpxenc上右键--->生成
	* 生成成功后，会在代码目录下找到 Win32\Debug 目录，vpxenc.exe 就在里面