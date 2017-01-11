环境：
Windows-64 bit


* Jenkins 的脚本（此台 Jenkins 机器运行在 Windows 上）

		---->Shell:
		git clone git@10.10.49.51:~/ParallelTranscodingForRMHD.git -b rv11encoder-plugin-support
		
		cd ParallelTranscodingForRMHD
		git branch -a
		git submodule update --init --recursive
		
		
		cd ffmpeg_module
		git branch -a
		git checkout producer
		git pull
		git branch -a
		cd ..
		
		
		cd wingui
		git branch -a
		git checkout master
		git pull
		git branch -a
		cd ..
		
		--->Windows batch command
		path=D:\MinGW_w64_32bits\msys\1.0\bin;D:\MinGW_w64_32bits\mingw32\bin;D:\MinGW_w64_32bits\bin
		cd D:\MinGW_w64_32bits\msys\1.0
		msys_release.bat

* 修改编译脚本以适合自己的环境：
	* 修改 `ParallelTranscodingForRMHD/build_MinGW_w64_32bits.sh`，把里面引用的 "/mingw/mingw32/" 替换成自己电脑上的路径，比如我的就是 "/mingw32/"
	* 这个路径里应包含： i686-w64-mingw32，bin，include 等
	* 把里面引用的 "/mingw/bin/" 替换成自己电脑上的路径，比如我的就是 "/usr/bin/"、
	* 这个路径里应包含 pkg-config.exe，yasm.exe 等
	* 把 `libgcc_s_sjlj-1.dll` 替换成自己的，比如我的就是：`libgcc_s_dw2-1.dll`

* 安装如下第三方库（避免在下一步使用使用 full 来编译 Producer 工程，那样容易遇到问题）：
	* zlib: 我的 mingw32 默认就有
	* SDL: `pacman -S  mingw-w64-i686-SDL`
	* fdk-aac: `pacman -S mingw-w64-i686-faac`
	* freetype: `pacman -S  mingw-w64-i686-freetype`
	* fribidi： `pacman -S mingw-w64-i686-fribidi`
	* libxml2: `pacman -S mingw-w64-i686-libxml2`
	* fontconfig: `pacman -S mingw-w64-i686-fontconfig`
	* libass: `pacman -S mingw-w64-i686-libass`

* 再次修改编译脚本：
	*  修改如下一行，`export PKG_CONFIG_PATH=`
	*  把里面分号分隔的，含有 `$WORKSPACE/ParallelTranscodingForRMHD/` 的部分都删除，加上一个 `/mingw32/lib/pkgconfig/`
	*  最后改完该行为： `export PKG_CONFIG_PATH=/mingw32/i686-w64-mingw32/lib/zlib/win32/lib/pkgconfig/:/mingw32/lib/pkgconfig/`

* 用 MinGW（w64-32bit 版）编译 ffmpeg。在 mingw-32 的 shell 里执行（确定已经安装了 mingw-32，敲一下 gcc 回车试试，如果没装，用 pacman 装一下）：
	
		sh path/to/ParallelTranscodingForRMHD/build_MinGW_w64_32bits.sh path/to/ParallelTranscodingForRMHD_parent out/put/path release


* 用 msbuild.exe build GUI

		cd C:\Windows\Microsoft.NET\Framework\v4.0.30319
		msbuild.exe ParallelTranscodingForRMHD\wingui\RealProducerGUI\RealProducer.sln /t:Rebuild /p:Configuration=Release /p:VisualStudioVersion=12.0


###修改代码后仅做增量编译
* MinGW32 命令行 cd 到 `ffmpeg_module` 里，直接执行 make：

		make -j 8 "PRODUCT=PRODUCER"

* 然后 

		make install

###遗留问题
* 运行时还是会去连这个dll：`libgcc_s_sjlj-1.dll`，而不是自己的 MinGW 的：`libgcc_s_dw2-1.dll`

		

