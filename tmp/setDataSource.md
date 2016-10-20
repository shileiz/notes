* java 层的 setDataSource() 有5个重载，我们只看其中这一个：`setDataSource(String path)`
* 它会先走到 `setDataSource(path, null, null)`, 然后在这里有个分叉

	1. 如果 path 是 file，则走到
	
			setDataSource(FileDescriptor fd_from_path);

	2. 如果 path 不是 file 而是 http/rtsp，则走到
	
			nativeSetDataSource(
				MediaHTTPService.createHttpServiceBinderIfNecessary(path), 
				path, null, null);
	
* 我们先看是 file 的情况，然后再看不是 file 的。

### 1. file 的 setDataSource
* `setDataSource(FileDescriptor fd)` 最终会调用 native 方法： `private native void _setDataSource(FileDescriptor fd, long offset, long length)`
* 因为我们没有传 offset 和 length（我们只传了一个 path，java层根据这个 path 生成了 fd），java 层把 offset 设成了 0，length 设成了 0x7ffffffffffffffL —— 一个巨大的数。
* 下面来看这个 native 方法。

		static void
		android_media_MediaPlayer_setDataSourceFD(JNIEnv *env, jobject thiz, jobject fileDescriptor, jlong offset, jlong length)
		{
		    sp<MediaPlayer> mp = getMediaPlayer(env, thiz);
		    if (mp == NULL ) {
		        jniThrowException(env, "java/lang/IllegalStateException", NULL);
		        return;
		    }
		
		    if (fileDescriptor == NULL) {
		        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
		        return;
		    }
		    int fd = jniGetFDFromFileDescriptor(env, fileDescriptor);
		    ALOGV("setDataSourceFD: fd %d", fd);
		    process_media_player_call( env, thiz, mp->setDataSource(fd, offset, length), "java/io/IOException", "setDataSourceFD failed." );
		}

#### getMediaPlayer()
* 上来是个 getMediaPlayer()，这个函数没什么，就是从 java 的 mp 对象里拿到它的 mNativeContext 的值，强转成(MediaPlayer*)类型，返回。
* 这跟我们之前分析过的 setMediaPlayer() 呼应。 
* Java MediaPlayer 的 mNativeContext 成员，就是用来保存 C++ MediaPlayer 对象的指针的。
* C++ 把 Java **类** MediaPlayer 的成员 mNativeContext **的 ID** 存在 fields.context 里。 Java mediaplayer **对象**的 mNativeContext 的值，对应 C++ mediaplayer **对象**的指针。
* 这里我们 getMediaPlayer() 得到的就是之前在 native_setup() 时（java 层 new MediaPlayer 会走到） new 出来的那个 C++ 的 MediaPlayer。 
* 接下来有用的代码是这一行：`process_media_player_call( env, thiz, mp->setDataSource(fd, offset, length), "java/io/IOException", "setDataSourceFD failed." );`

####mp->setDataSource(fd, offset, length)
* 我们先看 `mp->setDataSource(fd, offset, length)`，这是 C++ 类 MediaPlayer 的函数，先看其原型（frameworks/av/include/media/mediaplayer.h），并不是虚函数：

		status_t  setDataSource(int fd, int64_t offset, int64_t length); 

* 其实现如下（frameworks/av/media/libmedia/mediaplayer.cpp）：

		status_t MediaPlayer::setDataSource(int fd, int64_t offset, int64_t length)
		{
		    ALOGV("setDataSource(%d, %" PRId64 ", %" PRId64 ")", fd, offset, length);
		    status_t err = UNKNOWN_ERROR;
		    const sp<IMediaPlayerService>& service(getMediaPlayerService());
		    if (service != 0) {
		        sp<IMediaPlayer> player(service->create(this, mAudioSessionId));
		        if ((NO_ERROR != doSetRetransmitEndpoint(player)) ||
		            (NO_ERROR != player->setDataSource(fd, offset, length))) {
		            player.clear();
		        }
		        err = attachNewPlayer(player);
		    }
		    return err;
		}

*  `const sp<IMediaPlayerService>& service(getMediaPlayerService());` 这句话就是定义了一个 `const sp<IMediaPlayerService>&` 类型的变量 service，并赋值为 getMediaPlayerService() 的返回值。
*  我们暂且不去看 getMediaPlayerService() 的实现，这个函数就是从 ServiceManager 那里获取到了 BpMediaPlayerService。(详见《深入理解Android卷1-第6章-6.4节》)
*  `sp<IMediaPlayer> player(service->create(this, mAudioSessionId));` 这句话定义了一个 sp<IMediaPlayer> 类型的变量 player，并赋值为 `service->create(this, mAudioSessionId)` 的返回值。 
* 我们研究一下这个 `service->create(this, mAudioSessionId)`，它是 BpMediaPlayerService 类的成员函数，其原型为：

		virtual sp<IMediaPlayer> create(const sp<IMediaPlayerClient>& client, int audioSessionId) 

* 第一个参数是 IMediaPlayerClient 类型的指针，我们这里传的是 this，即 C++ 的 MediaPlayer 对象 mp。 因为 MediaPlayer 继承自 BnMediaPlayerClient，而 BnMediaPlayerClient 继承自 IMediaPlayerClient。 
* 第二个参数是 int 型的 audioSessionId，我们这里传的是 mAudioSessionId。 我们当年在 new MediaPlayer 时，给它赋初值了吗？看下构造函数里：

		mAudioSessionId = AudioSystem::newAudioUniqueId();

* 确实是给了一个初值，具体什么用先不去管他。
* 返回值是 sp<IMediaPlayer> 类型的。 
* 这会被 Binder 跨进程的送给 MediaPlayerService 来处理，最终是由 MediaPlayerService 的 create() 干活

###传送门：具体怎么跨进程从app进程走到 mediaserver 进程，交给 MediaPlayerService 处理的
* 我们这里是调用 BpMediaPlayerService 的 create()，看一下它的实现（frameworks/av/media/libmedia/IMediaPlayerService.cpp）

		virtual sp<IMediaPlayer> create(
		        const sp<IMediaPlayerClient>& client, int audioSessionId) {
		    Parcel data, reply;
		    data.writeInterfaceToken(IMediaPlayerService::getInterfaceDescriptor());
		    data.writeStrongBinder(IInterface::asBinder(client));
		    data.writeInt32(audioSessionId);
		
		    remote()->transact(CREATE, data, &reply);
		    return interface_cast<IMediaPlayer>(reply.readStrongBinder());
		}


* 我们需要看一下 MediaPlayerService 的 create() 函数（位于：`frameworks/av/media/libmediaplayerservice/MediaPlayerService.cpp` ）。

		sp<IMediaPlayer> MediaPlayerService::create(const sp<IMediaPlayerClient>& client,
		        int audioSessionId)
		{
		    pid_t pid = IPCThreadState::self()->getCallingPid();
		    int32_t connId = android_atomic_inc(&mNextConnId);
		
		    sp<Client> c = new Client(
		            this, pid, connId, client, audioSessionId,
		            IPCThreadState::self()->getCallingUid());
		
		    ALOGV("Create new client(%d) from pid %d, uid %d, ", connId, pid,
		         IPCThreadState::self()->getCallingUid());
		
		    wp<Client> w = c;
		    {
		        Mutex::Autolock lock(mLock);
		        mClients.add(w);
		    }
		    return c;
		}

* 实际上就是 new 了一个 Client 并把它 return 出去了。 Client 是啥？
* Client 是 MediaPlayerService 的内部类，它继承自 BnMediaPlayer（注意：不是BnMediaPlayerClient），BnMediaPlayer 继承自 IMediaPlayer（注意，不是IMediaPlayerClient），Client 的定义也在 MediaPlayerService.h:

		class Client : public BnMediaPlayer {...}

* Client 的构造函数在 MediaPlayerServi.cpp 实现，我们来看一下：

		MediaPlayerService::Client::Client(
		        const sp<MediaPlayerService>& service, pid_t pid,
		        int32_t connId, const sp<IMediaPlayerClient>& client,
		        int audioSessionId, uid_t uid)
		{
		    ALOGV("Client(%d) constructor", connId);
		    mPid = pid;
		    mConnId = connId;
		    mService = service;
		    mClient = client;
		    mLoop = false;
		    mStatus = NO_INIT;
		    mAudioSessionId = audioSessionId;
		    mUID = uid;
		    mRetransmitEndpointValid = false;
		    mAudioAttributes = NULL;
		
		#if CALLBACK_ANTAGONIZER
		    ALOGD("create Antagonizer");
		    mAntagonizer = new Antagonizer(notify, this);
		#endif
		}