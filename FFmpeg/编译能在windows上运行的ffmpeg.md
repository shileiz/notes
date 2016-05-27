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
* 这种方法比较麻烦，需要做  C99-to-C89 的转换
* 这种方法没有实践过，不做记录

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