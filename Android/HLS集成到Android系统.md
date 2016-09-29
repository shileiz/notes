* 方案一： 从 m3u8 文件里读取 codec 格式，根据 codec 格式判断是否走我们的 Player
* 方案二： 从第一个分片里读取头部，从而判断 codec 格式，然后判断是否走我们的 Player
* 无论哪种方案，都需要使用 http 从服务端先下载文件，所以先搞定怎么下载
* 参考 NuPlayer
* NuPlayer 的源码在：`frameworks/av/media/libmediaplayerservice/nuplayer`
* NuPlayer 里的 HLS 部分是用 LiveSession 实现的，LiveSession 的源码在：`frameworks/av/media/libstagefright/httplive`
* LiveSession 工作的大概流程参考这篇：[http://blog.csdn.net/cutea/article/details/25392541](http://blog.csdn.net/cutea/article/details/25392541)
* 根据这篇文章的总结，只要你 new 了一个 LiveSession，并调用了他的 connectAsync()，就会自动的下载 m3u8 文件以及里面的分片
* 我们不需要这样，我们只要得到第一个分片就可以了


* LiveSession 是消息驱动的，一般流程（以 LiveSession::connectAsync() 为例）

		:::c
	    sp<AMessage> msg = new AMessage(kWhatConnect, this); // new 一个 message，message的类型是 kWhatConnect
	    msg->setString("url", url);  // 给 message 添加参数
		msg->post();  // 发送 message

* LiveSession::onMessageReceived() 在收到消息时，按消息类型分发给对应函数，比如本例收到 kWhatConnect 类型的 message，就分发给 onConnect(msg)
* LiveSession::onConnect() ，会从 message 里提取 url，进行 connect

		:::c
		msg->findString("url", &mMasterURL) // 从msg里拿出 url
		...
		addFetcher(mMasterURL.c_str())->fetchPlaylistAsync(); // 使用拿到的 url，进行 connect

### 主要数据结构

#### AnotherPacketSource
* 这是 mpeg2ts 的一个数据结构，用来表示一个 ts 分片
* 对应源码在： `frameworks/av/media/libstagefright/mpeg2ts/AnotherPacketSource.h`


#### LiveSession
* 构造 LiveSession 的时候是不需要 url 的，因为一个 LiveSession 可以连接多个 url。 构造的时候需要一个 IMediaHTTPService
* 调用 connectAsync() 的时候再指定 url	
* LiveSession 本身是 AHandler 的子类，所以他可以注册给一个 ALooper

#### PlaylistFetcher

#### HTTPDownloader


### NuPlayer 是怎么使用 LiveSession 的
* NuPlayer 可以设置好多种 DataSource，HTTPLiveSource 是其中一种。NuPlayer 为每一种 DataSource 封装了类。
* 其中，HTTPLiveSource 有一个 LiveSession 类型的成员： mLiveSession
* NuPlayer 在 prepareAsync() 里调用了 mSource->prepareAsync()，对于 HTTPLiveSource 类型的 DataSource 来说，就是调用了 HTTPLiveSource 的 prepareAsync()
* HTTPLiveSource 的 prepareAsync() 里 new 了一个 LiveSession 出来，保存到 HTTPLiveSource 的成员变量 mLiveSession 里
* 并且，在 new 了之后，直接调用了 mLiveSession->connectAsync()
* 一旦mLiveSession->connectAsync()，LiveSession 就会自己去下载 m3u8，然后一片一片的下载ts了

#### ABuffer
* ts 分片（m3u8可能也是）的实际数据封装在这个数据结构里
* `frameworks/av/media/libstagefright/foundation/ABuffer.cpp`
* `frameworks/av/include/media/stagefright/foundation/ABuffer.h`

### EXT-X-DISCONTINUITY
* AnotherPacketSource::getFormat()， 在自己的 mBuffers 里挨个 buffer（ABuffer类型）查看 mMeta（mMeta是ABuffer的成员，AMessage类型），如果该 buffer 的 mMeta 里有 discontinuity 属性，说明这个 buffer 里的 ts 格式可能发生变化了。
* 此时要查一下这个 buffer 的 mMeta 里的 format（MetaData 类型），并把这个 format 设置给 mFormat。
* 参考：[http://blog.csdn.net/haima1998/article/details/46791007](http://blog.csdn.net/haima1998/article/details/46791007)