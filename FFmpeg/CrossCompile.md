## 目标
* 编译能在 windows 上运行的 ffmpeg

### 参考文档

* http://ffmpeg.org/platform.html#Windows

### 方式一：在 Windows 上使用 MinGW-w64 编译 ffmpeg

#### MSYS2 和 MinGW-w64
* 简单理解：
* MSYS2 是一个在 Windows 上运行的命令行窗口，让你可以在里面运行一些 Linux 命令，比如 ls、pwd 之类的
* MinGW 是一个能编译出 windows 下可执行程序的 gcc

#### 安装 MSYS2 和 MinGW-w64
* 先装 MSYS2,通过 MSYS2 的 pacman 来安装 MinGW-w64
* MSYS2 在这里下载：

		http://msys2.github.io/
* 下载安装后，在安装目录下运行 mingw64_shell.bat 启动 MSYS2 的窗口
* 在窗口里用 pacman 命令安装 MinGW-w64：

		pacman -S mingw-w64-x86_64-gcc

#### 编译 ffmpeg

##### 安装 yasm
* 在 MSYS2 里运行

		pacman -S yasm

##### config
* 在 MSYS2 里运行

		./configure --target-os=mingw32

##### make
* 在 MSYS2 里运行 make


### 方式二：在 Linux 上使用 mingw-w64 编译 ffmpeg
* 环境信息：
* Ubuntu 14.04 64bit
* ffmpeg 源码是从官方 git 上 克隆的 2.8 release 这个 branch

#### 安装 mingw
* 直接用 apt-get 进行安装：

		sudo apt-get install mingw-w64

#### config：配置交叉编译
* 必须的三个参数如下

		./configure --target-os=mingw32 --cross-prefix=x86_64-w64-mingw32- --arch=x86_64
* config 时报的 warning：`x86_64-w64-mingw32-pkg-config` 找不到，做个软连接就可以了：

		sudo ln -s "/usr/bin/pkg-config" /usr/bin/x86_64-w64-mingw32-pkg-config

#### 编译：make
* 配置成功后直接 make 即可
* 注意，如果之前 make 过，要 make uninstall， make clean 一下，然后重新 config 再重新 make

#### 运行
* 把 make 得到的 ffmpeg.exe， ffprobe.exe 拷贝到 Windows7-64bit 的电脑上运行
* 需要把 `/usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll` 一起拷贝到 Windows 电脑上才能正确运行

### 方式三：用 Microsoft Visual C++ 在 Windows 上编译 ffmpeg
* 这种方法比较麻烦，需要做  C99-to-C89 的转换
* 这种方法没有实践过，不做记录

### 方式四：Cygwin
* 没有实践过，不做记录