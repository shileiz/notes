### 第一步，在 Linux 上编译
* 环境： Ubuntu 14 x64，装好了 yasm
* 把 Ubuntu 默认的 dash 改成 bash：
* 
		# choose [No] to use bash
		sudo dpkg-reconfigure dash
* 配环境变量： 
* 
		export ANDROID_SDK=<your sdk path>
		export ANDROID_NDK=<your ndk path>
	* 注意，**NDK一定要是R9版本**的，我试了R10的，不行，有一个编译脚本会报错。
	* 注意，**SDK Platform 一定要是 Android-23**，或者也可以是Android-12~Android-19，而不是像文档里写的 9~23都可以。我用21的时候就遇到了报错。

* 按照官方 README 一步一步来：
* 
		git clone https://github.com/Bilibili/ijkplayer.git ijkplayer-android
		cd ijkplayer-android
		git checkout -B latest k0.4.5.1
		./init-android.sh
* 在 `./init-android.sh` 时报错，git 说有一个参数它不认识，叫 “dissociate”
* 直接修改这个 git 命令所在的 sh 文件，把这个 `--dissociate` 去掉就行了
* 重新运行 `./init-android.sh` 
* 继续按照官方文档来：
* 
		cd android/contrib
		./compile-ffmpeg.sh clean
		./compile-ffmpeg.sh armv7a
		cd ..
		./compile-ijk.sh armv7a
* 注意，以上步骤中把官方文档的 `all` 替换成了 `armv7a`，以加快速度和减少出问题的可能。
* 如果仍然使用 all，在 `./compile-ffmpeg.sh all` 的时候还是会报 SDK platform 的错误，说什么只能是 Android-12~Android-19之一之类的。
* 在 `compile-ijk.sh all` 的时候也会报错停止。
### 第二步，把编译好的工程倒入到 windows 的 Eclipse
* 

### Linux 用代理更新 Android-SDK
* 首先，Linux 的 SDK-Manager 是 Android SDK Tools 里面的 android 这个可执行文件
* 其次，这个东西在图像界面上没有给你留配置代理的按钮
* 配置代理的方式为：
* 在用户根目录下进入 `.android` 文件夹，在里面新建一个文件：`androidtool.cfg`
* 这个文件里写如下内容：
* 
		http.proxyPort=80
		http.proxyHost=mirrors.neusoft.edu.cn
		sdkman.force.http=true