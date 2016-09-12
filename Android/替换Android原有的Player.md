### 头文件
* 自己做的 plyaer ，所有的头文件可以放到这里： `frameworks/av/include`

### 需要修改的系统源文件
* `frameworks/av/media/libmediaplayerservice/` 目录下有4个：

		MediaPlayerFactory.cpp
		MediaPlayerService.cpp
		MetadataRetrieverClient.cpp
		Android.mk

* `frameworks/base/media` 下，共4个，其中2个在java层，两个在jni层：

		:::java
		// frameworks/base/media/java/android/media，java 层的两个
		MediaFile.java 
		MediaScanner.java
		
		// rameworks/base/media/jni，jni 层的两个
		android_media_MediaScanner.cpp
		Android.mk

### 确定哪种后缀名/网络协议用哪种Player
* 这个是又 MediaPlayerFactory.cpp 决定的，里面每种 Player 有自己的 scoreFactory() 方法
* 这个方法返回值越大，该 Player 就越容易被选中