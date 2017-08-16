# 关于 MediaPlayer
* MediaPlayer 是安卓 SDK 提供的一个播放器，可以播放 mp4 等多媒体文件
* 当然一般的视频播放器（包括Youku）是不会使用这个 MediaPlayer 的，他们有自己的播放器内核
* 不过了解这个播放器的原理，对于了解其他播放器很有帮助，因为他们大同小异。
* 并且，
	1. 一般第三方播放器（比如Youku）， 会在某些策略下（比如在播放DRM视频的情况下）使用这个 MediaPlayer。并不是百分百使用自己的内核。
	2. 安卓 SDK 有良好且规范的文档，看起来不费劲。
* 所以，我们可以从此入手。

## 1. MediaPlayer 的使用
* 最基本的使用：

```
MediaPlayer mediaPlayer = new MediaPlayer();
mediaPlayer.setDataSource("http://........");
mediaPlayer.prepare(); 
mediaPlayer.start(); 
mediaPlayer.seekTo(350872);
mediaPlayer.pause();
...
```

* 用起来很简单，只需要 new 一个 MediaPlayer 出来，然后调用各种方法就可以播放、暂停、seek 等等。

* 具体参考安卓SDK文档即可: [https://developer.android.com/reference/android/media/MediaPlayer.html](https://developer.android.com/reference/android/media/MediaPlayer.html)

* 这里贴一下SDK文档里的状态机图：

![](mediaplayer_state_diagram.gif)

## 2. MediaPlayer 的原理
* 安卓SDK提供给我们一个 java 层的 MediaPlayer 类，方便我们使用。SDK文档只告诉我们怎么使用这个 Java 层的类来播视频
* 它实际的功能是在 native 层用 C++ 实现的，Android 源码里可以找到所有的实现。接下来我将用几篇文档介绍一下其底层实现
* 另外提前说一下，其实 Android 源码在底层，实现了不止一个播放器（就像 YoukuPlayer 里，有新核AliPlayer、老核UPlayer）。当你在 Java 层使用 MediaPlayer 时，底层有可能是 AwesomePlayer 也有可能是 NuPlayer，还有可能是手机厂商自己定制的 Player。