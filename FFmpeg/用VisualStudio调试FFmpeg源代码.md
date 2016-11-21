* 参考1： [http://note.youdao.com/share/?id=759383ce6d4f88a2c3387fc334aaeb1a&type=note#/](http://note.youdao.com/share/?id=759383ce6d4f88a2c3387fc334aaeb1a&type=note#/)
* 参考2：[http://www.vcmfc.com/portal.php?mod=view&aid=27](http://www.vcmfc.com/portal.php?mod=view&aid=27)

* 尽管 ffmpeg 的官方文档提供了多种方法让我们能够从 ffmpeg 的源码编译出 Windows 下可用的可执行程序以及库文件。
* 但 ffmpeg 官方并未提供使用 VS 来调试 ffmpeg 代码的方案。
* 也就是说，我们有多种方法能够编译出 libavformat，libavcodec 这种库，并可以把这些库集成到 VS 的工程中去使用它们
* 但是我们并不能够在 VS 下修改这些库的源文件然后用 VS 重新生成我们自定义后的库。

---

* 这个开源程序帮我们解决了这一问题：[https://github.com/ShiftMediaProject/FFVS-Project-Generator](https://github.com/ShiftMediaProject/FFVS-Project-Generator)
* 这是一个 VS 的工程，clone 下来直接有 sln，用VS2013打开并编译后会生成一个 `project_generated.exe`
* `project_generated.exe` 的作用就是把 ffmpeg 源码生成一个 VS 工程，直接把 sln 给你搞出来
* 而且这个 `project_generate.exe` 还能接受几乎所有的 ffmpeg 的 config 脚本的参数，简直太爽。比如：

		project_generate.exe --help

* clone 下来的代码用 VS2013 直接编译报了个错，说 `uint32_t` 未定义，只需要在 projectGenerator.cpp 最前面加个: `#include <stdint.h>` 即可编译成功，
* debug模式得到 `project_generated.exe`，release模式得到 `project_generate.exe`（在bin目录下）
* 如果用 vs2015 编译则不会报错，直接成功。
* 把这个 project_generate.exe 扔到 ffmpeg 的源码目录，跟 configure 在同一级。
* 在 ffmpeg 源码目录（跟 configure 在同一级）新建一个文件夹 SMP，这是用于 project_generated.exe 放输出文件的
* 运行如下命令生成 sln，注意，以下config参数是经过好几次失败教训的，有些东西不 disable 掉是会报错的，比如 lzma

		project_generate.exe --disable-bzlib --disable-iconv --disable-zlib --disable-lzma --disable-avdevice --disable-sdl --disable-ffplay --disable-ffprobe --disable-ffserver --toolchain=msvc

* 搞出来的 sln 用 vs2013打开后发现 libavcodec 等项目都加载失败，这是因为没有给 VS2013 安装 yasm。安装方法：
	* 下载：[http://yasm.tortall.net/Download.html](http://yasm.tortall.net/Download.html) ，下载其中的 “Win64 VS2010 .zip”
	* 解压后需要修改 vsyasm.props，把其中的 `$(Platform)` 替换成 `win$(PlatformArchitecture)`
	* 然后把解压出来的东西都扔到这里：`C:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0\V120\BuildCustomizations`
	* 另外把 exe 再拷贝一份放到这里：`C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin`
* 由于 ffmpeg 大大大量的使用了第三方库，稍微不小心就会生成 sln 失败，还是挺烦的。
* 详细的可以看这个开源库的 readme

----

* 不 disable-ffmpeg 就会在生产 sln 的最后一步出错：project_generated.exe 在打印完如下两行 log 后，崩溃退出：

		Generating solution file...
		Generating from Makefile (./) for project ffmpeg...

* 原因如下：
* 在函数 `ProjectGenerator::outputSourceFiles` 里，有一句：`if (findSourceFile(sProjectName.substr(3) + "res", ".rc", sResourceFile)) `
* 这个本意是去找 avcodecres.rc 这种 rc 文件的，其中的 `.substr(3)` 就是为了去掉 libavcodec 前面的 lib
* 但是，当我们 enable 了 ffmpeg，ffplay 这种不是 lib 的东西后，它就变成了去找 pegres.rc, layres.rc了，这显然是不对的。
* 所以我们在这外面套一层 if：`if(sProjectName.find("lib")==0){`
* 只有是 lib 开头的，我们才去试图找到 rc 文件。

