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


* 按照官方 README 一步一步来：
* 
		git clone https://github.com/Bilibili/ijkplayer.git ijkplayer-android
		cd ijkplayer-android
		git checkout -B latest k0.4.5.1
		./init-android.sh
* 在 `./init-android.sh` 时报错，git 说有一个参数它不认识，叫 “dissociate”
* 直接修改这个 git 命令所在的 sh 文件（tools/pull-repo-ref.sh），把这个 `--dissociate` 去掉就行了
* 重新运行 `./init-android.sh` 
* 继续按照官方文档来：
* 
		cd android/contrib
		./compile-ffmpeg.sh clean
		./compile-ffmpeg.sh armv7a
		cd ..
		./compile-ijk.sh armv7a
* 注意，以上步骤中把官方文档的 `all` 替换成了 `armv7a`，以加快速度和减少出问题的可能。
* 如果仍然使用 all，在 `./compile-ffmpeg.sh all` 的时候还是会报 SDK platform 的错误，说什么只能是 Android-12~Android-19之一之类的。（`Invalid platform name: android-21`）
* 尽管我SDK的platforms里只下载了Android-23
* 在 `compile-ijk.sh all` 的时候也会报错停止。
### 第二步，把编译好的工程倒入到 windows 的 Eclipse
* 把 `ijkplayer-android/android/ijkplayer` 这个目录拷贝到 Windows 机器上
* Windows 机器上用 Eclipse 打开已存在的 Android 代码，选择拷过来的 ijkplayer 目录
* 只勾选 ijkplayer-sample kplayer-armv7a ijkplayer-java 这三个工程就可以了
* ijkplayer-sample 依赖于两个 Library Project，下面会说到

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


### Android 的 Library Project
* library project 是作为 jar 包被其它 android 工程使用的，首先**它也是普通的android工程**。  
* library project 可以引用外部jar包，但不能引用其它library工程
* library project 不能使用aidl文件，不能引用raw、assets下资源
* library project 不能被运行，运行的话会报错。
* 如何将一个android工程配置为 library project：
	1. 右键工程选择Properties
	2. 选择Android，下拉右边的滚动条到最下面，选中”Is Library“
	3. 点击Apply，点击OK

### appcompat-v7 和 preference-v7
* 首先用 SDK 工具下载 Extras 里的 Android Support Library
* Eclipse 新建 Android 工程，从已有 code 创建，选择 `<SDK>\extras\android\support\v7`
* 勾选其中的 appcompat 和 preference
* 把他俩都搞成 Library Project。
* preference 那个工程会报错，因为它依赖于 appcompat 这个工程
* 把 appcompat 作为 Library Project 添加给 preference 工程就好了
* 最后把这两个添加给 ijkplayer-sample 即可


