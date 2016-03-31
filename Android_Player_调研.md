## 测试环境
* 设备：小米手机3（MI 3W）
* Android版本：4.4.4
* h264视频文件：1920x1080，h264/4M，noaudio，mp4封装
* h265视频文件：1920x1080，h265/3M，noaudio，mp4封装
* rmvb视频文件：1920x1080，rv10（rv40）/5M，noaudio，rmvb封装
## MoboPlayer
* 软件版本：2.1.20,2015-9-17
* 自带解码器来自FFMpeg（在软件的关于页面里写了）
#### h264视频
* 软解：调用moboplayer自己的解码器，进程名是 com.clov4r.moboplayer.android.nil。占用CPU：50%~60%
* 硬解：调用mediaserver（/system/bin/mediaserver）。占用CPU：1%~3%
* 总结：软解真的是软件，硬解真的是硬解
#### h265视频
* 软解：调用moboplayer自己的解码器，进程名是 com.clov4r.moboplayer.android.nil。占用CPU：60%~80%
* 硬解：调用mediaserver（/system/bin/mediaserver）。占用CPU：40%~50%
* 总结：软件真的是软件，硬解本质上还是软解
#### rmvb视频
* 软解：调用moboplayer自己的解码器，进程名是 com.clov4r.moboplayer.android.nil。占用CPU：60%~80%
* 硬解：无法播放
* 总结：Mobo自带了rmvb的软解库，软解真的是软解；小米3的MediaServer无法解码rmvb
#### 总结
* MoboPlayer 软解即调用自带的解码器，肯定是软件解码。
* MoboPlayer 硬解即调用mediaserver（可能是基于android.media.MediaPlayer，或者VideoView开发的），但具体是软件解码还是硬件解码，取决于手机厂商。比如小米3上，h264就是真正的用硬件在解码，h265还是软解的，而rmvb根本就解不了。

## MXPlayer
* 软件版本：1.7.37（ARMv7 NEON）
* MXPlayer有3种解码模式选择：软解，硬解，硬解+
#### h264视频
* 软解：调用MXPlayer自己的解码器，进程名是com.mxtech.videoplayer.ad。占用CPU：50%~60%
* 硬解：调用mediaserver（/system/bin/mediaserver）。占用CPU：0%~1%
* 硬解+：调用自带硬解库，此时CPU使用率很低，com.mxtech.videoplayer.ad占用 0%~1%，mediaserver占用的CPU是为0%（排不到前三名）。
* 总结：软解调用自己的软解库，硬解调用mediaserver，硬解+调用自带的硬解码库。
#### h265视频
* 软解：调用MXPlayer自己的解码器，进程名是com.mxtech.videoplayer.ad。占用CPU：55%~70%
* 硬解：调用mediaserver（/system/bin/mediaserver）。占用CPU：40%~50%
* 硬解+：调用自带硬解库，估计发现无法硬解之后，又走到了硬解模式，此时mediaserver占用CPU：40%~50%
* 总结：软解调用自己的软解库，硬解调用mediaserver，硬解+调用自带的硬解码库，如果无法硬解还是走mediaserver。
#### rmvb视频
* 软解：调用MXPlayer自己的解码器，进程名是com.mxtech.videoplayer.ad。占用CPU：40%~50%
* 硬解：无法播放，直接弹出对话框说“不支持硬解”
* 硬解+：无法播放，直接弹出对话框说“不支持硬解+”
#### 总结
* 跟MoboPlayer一样，软件调自己的库，硬解调mediaserver
* 只不过多搞出来一个硬解+，就算不明白内部原理也不要太在意。 

## 暴风影音
* 软件版本：v6.0.08
* 不能在播放界面选择软解/硬解，只能在设置界面设置为“软解优先/硬解优先/智能解码”
* 本次不理会“智能解码”，分别测试“软解优先”和“硬解优先”
* h264和h265的测试视频用暴风影音都播放不了，画面黑屏进度条不走，所以本次不测暴风影音了

##　QQ影音
* 软件版本：3.2.0.438
* 不能在播放界面选择软解/硬解，只能在设置界面勾选“开启硬件加速”
* 暂且认为勾选“开启硬件加速”为硬解，反之为软解。
* 在说明页面说了使用了FFMpeg
### 硬解（勾选“开启硬件加速”）
* h264
	* com.tencent.research.drop占用CPU：8%~9%
	* mediaserver占用CPU：1%~2% 
* h265
	* com.tencent.research.drop占用CPU：60%~80%
	* mediaserver占用CPU：0%
* rmvb
	* com.tencent.research.drop占用CPU：50%~60%
	* mediaserver占用CPU：0%
### 硬解总结：
* QQ影音有自己硬解引擎，当能硬解的时候，它调用自己的硬解引擎，比如h264,QQ自己的硬解引擎占了最多CPU。
* 当然自己的硬解引擎最后也要借助mediaserver才能真正实现硬解，所以h264时mediaserver也占了一些CPU。
* 当无法硬解的时候，比如h265和rmvb，QQ会用自己的软解库，这跟Mobo和MX是手动选择软解是一样的。
### 软解（不勾选“开启硬件加速”）
* h264
	* com.tencent.research.drop占用CPU：50%~60%
	* mediaserver占用CPU：0% 
* h265
	* com.tencent.research.drop占用CPU：60%~80%
	* mediaserver占用CPU：0%
* rmvb
	* com.tencent.research.drop占用CPU：50%~60%
	* mediaserver占用CPU：0%
### 软解总结
* 跟mobo和mx一样，软解就跟mediaserver没有任何关系了，完全是用自己的解码库
## 总结
* 实际测试的三款主流软件，都可以设置软解或者硬解
* 只要是软解，就都使用自带的解码库（QQ和mobo说明了用的FFMp，MX不知道用的什么软解库），不会跟mediaserver有任何关系
* 只要是硬解，就都会去调用底层硬件解码，而想用硬件解码无论如何都会跟mediaserver扯上关系。
* 当然有两种使用mediaserver的方法
	* 一种就像moboplayer和mxplayer的硬解模式，纯使用mediaserver，估计这时的player是基于SDK的android.media.MediaPlayer或者VideoView开发的。
	* 还有一种就是自己带了硬解引擎，只是要借助mediaserver去调到硬件接口，比如QQ影音和mxplayer的硬解+模式。