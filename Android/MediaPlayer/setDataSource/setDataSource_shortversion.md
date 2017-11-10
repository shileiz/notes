## Java层的 setDataSource 会走到哪个 native 函数

* 如果 java 层发现是 http，会走到的 native 函数是：

```java 
nativeSetDataSource(
	MediaHTTPService.createHttpServiceBinderIfNecessary(path),
	path,
	keys,
	values);
```

* 其中，path 是 http url， keys 和 values 是 headers
* 这里 java 层首先用 path 生成了一个 MediaHTTPService.createHttpServiceBinderIfNecessary() 传给了 C++。该函数如下：

```java
// frameworks/base/media/java/android/media/MediaHTTPService.java

static IBinder createHttpServiceBinderIfNecessary(
            String path) {
        if (path.startsWith("http://")
                || path.startsWith("https://")
                || path.startsWith("widevine://")) {
            return (new MediaHTTPService()).asBinder();
        }        
        return null;
    }
```

* 可以看到，该函数返回了一个 java 层的 `(new MediaHTTPService()).asBinder()`
* MediaHTTPService 的也定义在 MediaHTTPService.java，代码如下：

```java
// frameworks/base/media/java/android/media/MediaHTTPService.java

public class MediaHTTPService extends IMediaHTTPService.Stub {

	public MediaHTTPService(){
	}
	
	public IMediaHTTPConnection makeHTTPConnection(){
		return new MediaHTTPConnection();
	}

	static IBinder createHttpServiceBinderIfNecessary(
	        String path) {
	    if (path.startsWith("http://")
	            || path.startsWith("https://")
	            || path.startsWith("widevine://")) {
	            
	        // asBinder() 是  IMediaHTTPService.Stub 的方法，其实现就是 return this
	        // 这里其实就是 new 了一个 MediaHTTPService ，return 出去
	        // MediaHTTPService 继承自 IMediaHTTPService.Stub 是服务端
	        return (new MediaHTTPService()).asBinder();
	    }        
	    return null;
	}
}
```
* 可以看到它继承自 IMediaHTTPService.Stub，Stub？恩，是 AIDL 里定义的。代码如下：

```
// frameworks/base/media/java/android/media/IMediaHTTPService.aidl

package android.media;

import android.media.IMediaHTTPConnection;

/** MUST STAY IN SYNC WITH NATIVE CODE at libmedia/IMediaHTTPService.{cpp,h} */

/** @hide */
interface IMediaHTTPService
{
    IMediaHTTPConnection makeHTTPConnection();
}
```

* 这又是一个基于 Binder 的跨进程通信，这次是 java 层的。这不是本文重点，此处略过。
* 需要记住的就是，MediaHTTPService 是服务端，其业务函数是 makeHTTPConnection，以后客户端会拿到一个“指向” MediaHTTPService 的指针，并调用它的业务函数。
* 我们来看一下它的业务函数实现:

```java
public IMediaHTTPConnection makeHTTPConnection(){
	return new MediaHTTPConnection();
}
```

## native 的 setDataSource()

* 以上 native 函数在JNI层的实现是:

> 在文件 
> http://androidxref.com/5.1.0\_r1/xref/frameworks/base/media/jni/android\_media\_MediaPlayer.cpp

```c++
// frameworks/base/media/jni/android_media_MediaPlayer.cpp

static void
android_media_MediaPlayer_setDataSourceAndHeaders(
        JNIEnv *env, jobject thiz, jobject httpServiceBinderObj, jstring path,
        jobjectArray keys, jobjectArray values) {

    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
    
    ...
    // pathStr 即参数 path，headersVector 即参数 keys 和 values 组成的

    sp<IMediaHTTPService> httpService;
    if (httpServiceBinderObj != NULL) {
        sp<IBinder> binder = ibinderForJavaObject(env, httpServiceBinderObj);
        httpService = interface_cast<IMediaHTTPService>(binder);
    }

    status_t opStatus =
        mp->setDataSource(
                httpService,
                pathStr,
                headersVector.size() > 0? &headersVector : NULL); // 展开看一下

    process_media_player_call(
            env, thiz, opStatus, "java/io/IOException",
            "setDataSource failed." );
}
```

* 下面来看这个 native 方法调用之后的结果：

![](2.after-setDataSource.png)



## mp->setDataSource(httpService, pathStr, headersVector) 详细展开
* 首先要记住，我们现在分析的是 app 进程里的，C++ 类 MediaPlayer 对象 mp 的 setDataSource() 函数。
* 我们先看 `mp->setDataSource(httpService, pathStr, headersVector) `的原型（frameworks/av/include/media/mediaplayer.h），并不是虚函数：

```			
status_t  setDataSource(const sp<IMediaHTTPService> &httpService,
						const char *url, const KeyVector<String8, String8> *headers); 
```
* 其实现如下（frameworks/av/media/libmedia/mediaplayer.cpp）：

```
status_t MediaPlayer::setDataSource(const sp<IMediaHTTPService> &httpService,
				const char *url, const KeyVector<String8, String8> *headers)
{
    ALOGV("setDataSource(%s)", url);
    status_t err = BAD_VALUE;
    const sp<IMediaPlayerService>& service(getMediaPlayerService());  // (0).
    if(url != NULL) {
	    if (service != 0) {
	        sp<IMediaPlayer> player(service->create(this, mAudioSessionId)); // (1).
	        if ((NO_ERROR != doSetRetransmitEndpoint(player)) ||
	            (NO_ERROR != player->setDataSource(httpService, url, headers)    // (2).
				)) 
				{
		            player.clear();
		        }
	        err = attachNewPlayer(player);  // (3).
	    }
    }
    return err;
}
```		

* 其中最关键的4个点即注释中的 (0) ~ (3)，它各自的作用见下图：

![](3.setDataSource_zhankai.png)

* (0). 用 service 建立了 app 进程和 mediaserver 进程的联系
* (1). 用 service 向 mediaserver 进程发送了 create 命令，得到了一个 player。player 实际上代表着 mediaserver 进程里的一个对象。
* (2). 用 player 向 mediaserver 进程发送 setDataSource 命令，致使 mediaserver 进程干了一些 setDataSource 的活，干完之后，mediaserver 进程里才真正有了 NuPlayer。
* (3). 把 player 保存到 mp 的 mPlayer 成员里。
* 为了缕清思路，省略了基于 Binder 的跨进程通信
* 这里着重看一下第(2)点

### player->setDataSource(httpService, url, headers) 
* 这是一次跨进程调用，player 只是给远端 mediaserver 进程发了消息，让上图中的 p (MediaPlayerService::Client 类型的) 去执行 setDataSource() 
* 看一下该函数：

```C++
// frameworks/av/media/libmediaplayerservice/MediaPlayerService.cpp
status_t MediaPlayerService::Client::setDataSource(
        const sp<IMediaHTTPService> &httpService,
        const char *url,
        const KeyedVector<String8, String8> *headers)
{
        ...
        player_type playerType = MediaPlayerFactory::getPlayerType(this, url);  // (a)
        sp<MediaPlayerBase> p = setDataSource_pre(playerType);  // (b)
        if (p == NULL) {
            return NO_INIT;
        }

        setDataSource_post(p, p->setDataSource(httpService, url, headers)); // (c)
        return mStatus;
    }
}
```

* 代码中的重点是 (a), (b), (c)
* (a) 是根据 url 确定 playerType，Android 5.0.1 里有以下三种类型：

```C++
// MediaPlayerInterface.h

enum player_type {
    STAGEFRIGHT_PLAYER = 3,
    NU_PLAYER = 4,
    // Test players are available only in the 'test' and 'eng' builds.
    // The shared library with the test player is passed passed as an
    // argument to the 'test:' url in the setDataSource call.
    TEST_PLAYER = 5,
};
```
* 具体怎么确定 player 类型这里就不展开了，最终这里会返回 `NU_PLAYER`
* (b) 展开后会 new 一个 NuPlayerDriver 赋给 p。new 的是 NuPlayerDriver 而不是 NuPlayer 本身，NuPlayerDriver 是 NuPlayer 的 wrapper，它满足  MediaPlayerBase。 NuPlayerDriver 有个成员变量 mPlayer，指向 NuPlayer。并且 NuPlayerDriver 的构造函数，会 new 一个 NuPlayer 出来，赋给自己的 mPlayer。
* (c) 首先调用 NuPlayerDriver 的 setDataSource():

```C++
//frameworks/av/media/libmediaplayerservice/nuplayer/NuPlayerDriver.cpp

status_t NuPlayerDriver::setDataSource(
        const sp<IMediaHTTPService> &httpService,
        const char *url,
        const KeyedVector<String8, String8> *headers) {
    ALOGV("setDataSource(%p) url(%s)", this, uriDebugString(url, false).c_str());
    Mutex::Autolock autoLock(mLock);

    if (mState != STATE_IDLE) {
        return INVALID_OPERATION;
    }

    mState = STATE_SET_DATASOURCE_PENDING;

    mPlayer->setDataSourceAsync(httpService, url, headers); 

    while (mState == STATE_SET_DATASOURCE_PENDING) {
        mCondition.wait(mLock);
    }

    return mAsyncResult;
}
```

* 其实就是把 NuPlayer 的 setDataSourceAsync() 给变成了一个同步的函数，NuPlayer 的 setDataSourceAsync()：

```C++
// frameworks/av/media/libmediaplayerservice/nuplayer/NuPlayer.cpp

void NuPlayer::setDataSourceAsync(
        const sp<IMediaHTTPService> &httpService,
        const char *url,
        const KeyedVector<String8, String8> *headers) {

    sp<AMessage> msg = new AMessage(kWhatSetDataSource, id());
    size_t len = strlen(url);

    sp<AMessage> notify = new AMessage(kWhatSourceNotify, id());

    sp<Source> source;
    if (IsHTTPLiveURL(url)) {
        source = new HTTPLiveSource(notify, httpService, url, headers);
    } else if (!strncasecmp(url, "rtsp://", 7)) {
        source = new RTSPSource(
                notify, httpService, url, headers, mUIDValid, mUID);
    } else if ((!strncasecmp(url, "http://", 7)
                || !strncasecmp(url, "https://", 8))
                    && ((len >= 4 && !strcasecmp(".sdp", &url[len - 4]))
                    || strstr(url, ".sdp?"))) {
        source = new RTSPSource(
                notify, httpService, url, headers, mUIDValid, mUID, true);
    } else {
        sp<GenericSource> genericSource =
                new GenericSource(notify, mUIDValid, mUID); // 形如 http://xxxx.mp4 的 url 会走到这里
        // Don't set FLAG_SECURE on mSourceFlags here for widevine.
        // The correct flags will be updated in Source::kWhatFlagsChanged
        // handler when  GenericSource is prepared.

        status_t err = genericSource->setDataSource(httpService, url, headers); // 这里也很简单

        if (err == OK) {
            source = genericSource;
        } else {
            ALOGE("Failed to set data source!");
        }
    }
    msg->setObject("source", source);
    msg->post();
}
```

* GenericSource 的 setDataSource(）

```C++
// frameworks/av/media/libmediaplayerservice/nuplayer/GenericSource.cpp

status_t NuPlayer::GenericSource::setDataSource(
        const sp<IMediaHTTPService> &httpService,
        const char *url,
        const KeyedVector<String8, String8> *headers) {
    resetDataSource();

    mHTTPService = httpService;
    mUri = url;

    if (headers) {
        mUriHeaders = *headers;
    }

    // delay data source creation to prepareAsync() to avoid blocking
    // the calling thread in setDataSource for any significant time.
    return OK;
}
```


