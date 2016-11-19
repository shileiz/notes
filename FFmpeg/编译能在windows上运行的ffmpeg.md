## 目标
* 编译能在 windows 上运行的 ffmpeg

### 参考文档

* [http://ffmpeg.org/platform.html#Windows](http://ffmpeg.org/platform.html#Windows)

### 方式一：在 Windows 上使用 MinGW-w64 编译 ffmpeg （本地编译）
* 环境信息
* windows10 64bit
* msys2 x86-64版，于 2016.05.26 下载于 http://msys2.github.io/
* ffmpeg 源码: 用 msys2 的 git 从官方 github 克隆，选择的 branch 是 2.8/release

#### MSYS2 和 MinGW-w64
* 简单理解：
* MSYS2 是一个在 Windows 上运行的命令行窗口，让你可以在里面运行一些 Linux 命令，比如 ls、pwd 之类的
* MinGW 是一个能编译出 windows 下可执行程序的 gcc
* 在安装了 mingw 的 msys2 里，直接敲 gcc，就能调用到 mingw（默认的 msys2 是不带 gcc 的，你敲 gcc 回车，bash 会报找不到命令的错）
* 即简单理解为：msys2 提供了一个假的 linux 命令行环境，mingw 就是在这个环境里的gcc，并且这个gcc能编译出 window 下的程序。
* 注意，在 windows 用 mingw 编译C代码，属于本地（Native）编译，不属于交叉编译

#### 安装 MSYS2 和 MinGW-w64
* 先装 MSYS2,通过 MSYS2 的 pacman 来安装 MinGW-w64
* MSYS2 在这里下载： [http://msys2.github.io/](http://msys2.github.io/)
* 下载安装后，在安装目录下运行 mingw64_shell.bat 启动 MSYS2 的窗口
* 注意，其实以下这几步在 `msys2_shell.bat`（或者msys2.exe）里完成也一样，因为都是在用 pacman 装软件。
* 不过因为编译 ffmpeg 必须在64位的shell里完成（因为我们是编译给64位windows使用的ffmpeg），所以后面的编译相关的命令，要在 `mingw64_shell.bat`（或者 msys264.exe）打开的窗口里完成。
* 所以我们干脆在这步就直接通过 `mingw64_shell.bat`（或者 msys264.exe） 来启动命令行窗口，以免后面忘记了。
* 在窗口里用 pacman 命令安装 MinGW-w64：

		pacman -S mingw-w64-x86_64-gcc

>这里说一下怎么找到 mingw-w64-x86_64-gcc 这个软件包的名字的，可以用如下命令： pacman -Sl | grep gcc
>如果想装32位的MinGW-w64，则使用： pacman -S mingw-w64-i686-gcc

* 安装 make，pkgconfig，yasm 和 diffutils

		pacman -S make pkgconfig diffutils yasm

* （如果需要通过 MSYS2 下载 ffmpeg 源码）安装 git  

		pacman -S git

* （如果需要） 用 git 从 ffmpeg 的官方克隆一份源码，本例使用的是 2016年5月26号的 2.8/release branch

#### 编译 ffmpeg
* 再次提醒，一切命令行要在 `mingw64_shell.bat`（或者 msys264.exe）打开的窗口里完成，而不是 `msys2_shell.bat`（或者 msys2.exe）里

##### config
* 在 MSYS2 里，cd 到 ffmpeg 的源码目录，然后运行 configure：

		./configure --target-os=mingw32

* 注意，在 msys2 的环境下（安装了 mingw-w64 的），gcc 就是 mingw-w64。所以这不是交叉编译，所以不用在 config 的时候加上 --target-os、--cross-prefix、--arch 参数

##### make
* 在 MSYS2 里运行 make

##### 运行
* 找到编译出来的 ffmpeg.exe （在 `E:\msys64\home\Administrator\FFmpeg\` 里）
* 其中前半段 `E:\msys64\home\Administrator\` 是 msys2 的安装目录
* 后面是克隆 ffmpeg 源码的目录
* 把 ffmpeg.exe 拷贝到其他目录，比如拷贝到 `D:\temp\`
* 把 `E:\msys64\mingw64\bin` 下的所有 dll 文件也拷贝到 `D:\temp\`
* 在 `D:\temp\` 启动 cmd，在 cmd 里运行 ffmpeg.exe

### 方式二：在 Linux 上使用 mingw-w64 编译 ffmpeg （交叉编译）
* 环境信息：
* Ubuntu 14.04 64bit
* ffmpeg 源码是从官方 git 上 克隆的 2.8 release 这个 branch

#### 原理
* 在 Linux 下编译 window 的可执行程序（或者库），是交叉编译
* 交叉编译一般都要用到目标平台（这里就是 windows）的特有 tool-chain
* Linux 下的给 window 目标平台用的 tool-chian ，就是 mingw 了
* 我们利用 mingw 这个 tool-chian 和 ffmpeg 提供的 config 选项（中的 tool-chian 选项），来在 Linux 上进行交叉编译

#### 安装 mingw
* 直接用 apt-get 进行安装：

		sudo apt-get install mingw-w64

* 注意，不要傻了吧唧在 Ubuntu 的图形化管理界面里，把软件更新相关的功能都关闭，这会导致 apt-get install 出错。
* 因为这时候一旦查到依赖关系需要升级某个软件时，相当于你禁止它去升级。

#### 编译 ffmpeg

##### config：配置交叉编译
* 必须的三个参数如下

		./configure --target-os=mingw32 --cross-prefix=x86_64-w64-mingw32- --arch=x86_64

* config 时报的 warning：`x86_64-w64-mingw32-pkg-config` 找不到，做个软连接就可以了：

		sudo ln -s "/usr/bin/pkg-config" /usr/bin/x86_64-w64-mingw32-pkg-config

##### 编译：make
* 配置成功后直接 make 即可
* 注意，如果之前 make 过，要 make uninstall， make clean 一下，然后重新 config 再重新 make

##### 运行
* 把 make 得到的 ffmpeg.exe， ffprobe.exe 拷贝到 Windows7-64bit 的电脑上运行
* 需要把 `/usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll` 一起拷贝到 Windows 电脑上才能正确运行

### 方式三：用 Microsoft Visual C++ 在 Windows 上编译 ffmpeg
* 如果使用VS2012及以前的版本，则这种方法比较麻烦，需要做  C99-to-C89 的转换
* 这里只记录使用 VS2013 编译的过程

#### 原理
* 同样需要用到 msys2，在 msys2 里调用 ffmpeg 的 config 脚本进行配置
* config 的时候指定目标是 msvc：`./configure --toolchain=msvc`
* 然后再 msys2 里用 make 命令进行编译，此时 make 命令调用的编译器不再是 gcc/mingw，而是微软的 cl
* 为了能让 msys2 找到微软的编译器，必须要引入 vs2013 的环境变量，具体方法下一节细说
* 注意1：连接器也要用微软的 link.exe，而不能用 msys2 的 link.exe，这两个文件是同名的。msys2 的 link.exe 在这里：`C:\msys64\usr\bin`，所以网上的教程一般建议大家编译之前把这个 link.exe 改名。
* 注意2：ffmpeg 官方文档里说了，微软的汇编编译器不支持 ffmpeg 里的汇编代码，所以我们必须要用 yasm，在 msys2 里安装即可：`pacman -S yasm`

####操作步骤1：准备工作
* 从 VS2013 的命令行启动 msys2，以保证 msys2 能正确引入 VS2013 的环境变量，这样才能找到微软的编译器连接器等等。
* 首先修改 msys2 的启动脚本，让 msys2 启动后能继承 parent 的环境变量：修改 `C:\msys64\msys2_shell.bat`，去掉这一行的注释：`set MSYS2_PATH_TYPE=inherit`
* 然后打开 vs2013的命令行，VS2013 的命令行在：`C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\Shortcuts`，双击 “VS2013 开发人员命令提示”打开命令提示符即可
* 在这个命令提示符里运行 `C:\msys64\msys2_shell.bat`，会弹出 msys2 的命令提示符，在里面敲一下 cl 回车，测试一下是否有输出。
* 在 msys2 的命令行里，cd 到 ffmpeg 的源码目录

####操作步骤2：config
* 运行以下命令（需要1到2分钟）：

		./configure  --enable-shared   --prefix=./VS2013  --toolchain=msvc 

* 下一步进行 make

####操作步骤3：make，make install
* 直接 make 即可
* make 结束后 make install
* 最终的产出物是一些动态库 dll 和头文件，并不生成 sln 解决方案文件
* 这种方式虽然从 ffmpeg 的源代码编译出了可执行文件和库文件，但并没有生成 VS 的解决方案，**我们还是无法用VS调试 ffmpeg 内部的代码**，比如 h264.c


### 方式四：Cygwin
* 没有实践过，不做记录

### 写在后面（其实应该写在前面？）

#### 费这么大劲别再没必要

* 如果只是想使用 ffmpeg.exe、ffplay.exe、ffprobe.exe，
* 或者只是想使用 libavformat.dll 等在 windows 平台做基于 ffmpeg 的开发，
* 完全没有必要自己动手编译。
* 网上有人专门给 windows 编译 ffmpeg 的可执行文件和库：`https://ffmpeg.zeranoe.com/builds/`
* 这个网站的链接在 ffmpeg 的官网也有

#### 例子都很简单，且功能不全

* 这里的例子都是尽可能少的改变 ffmpeg 的编译配置，即 ./configure 后面尽量少跟参数
* 这样做只是为了起到演示的效果，可以根据实际需要多加参数，比如 --disable-server 之类的
* 为了简单，没有编译 ffmpeg 需要依赖的第三方库，比如 libx264 等等非常常用的库都没编
* 这样会导致编译出来的 ffmpeg.exe 虽然能运行，但连h264的目标文件都转不出来
* 不过解码器都是 ffmpeg 自己的，所以按照上述步骤编出来的 ffmpeg 是可以解码视频的
* 另外也没有下载和编译 SDL，这导致没有编出来 ffplay（依赖于 SDL）