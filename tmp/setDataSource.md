* java 层的 setDataSource() 有5个重载，我们只看其中这一个：`setDataSource(String path)`
* 它会先走到重载的 `setDataSource(path, null, null)`, 然后在这里有个分叉

	1. 如果 path 是 file，则走到
	
			setDataSource(FileDescriptor fd_from_path);

	2. 如果 path 不是 file 而是 http/rtsp，则走到
	
			nativeSetDataSource(
				MediaHTTPService.createHttpServiceBinderIfNecessary(path), 
				path, null, null);
	
* 我们这里只看是 file 的情况，不是 file 的，我们在另外一篇单看。

##file 的 setDataSource
* java 层的 `setDataSource(FileDescriptor fd)` 最终会调用 native 方法： `private native void _setDataSource(FileDescriptor fd, long offset, long length)`
* 因为我们没有传 offset 和 length（我们只传了一个 path，java层根据这个 path 生成了 fd），java 层把 offset 设成了 0，length 设成了 0x7ffffffffffffffL —— 一个巨大的数。
* 下面来看这个 native 方法。

		static void
		android_media_MediaPlayer_setDataSourceFD(JNIEnv *env, jobject thiz, jobject fileDescriptor, jlong offset, jlong length)
		{
		    sp<MediaPlayer> mp = getMediaPlayer(env, thiz); // 1.
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
		    process_media_player_call( env, thiz, mp->setDataSource(fd, offset, length), "java/io/IOException", "setDataSourceFD failed." ); // 2.
		}

####1. mp = getMediaPlayer()
* 上来是个 getMediaPlayer()，这个函数没什么，就是从 java 的 mp 对象里拿到它的 mNativeContext 的值，强转成(MediaPlayer*)类型，返回。
* 这跟我们之前分析过的 setMediaPlayer() 呼应。 
* Java MediaPlayer 的 mNativeContext 成员，就是用来保存 C++ MediaPlayer 对象的指针的。
* C++ 把 Java **类** MediaPlayer 的成员 mNativeContext **的 ID** 存在 fields.context 里。 Java mediaplayer **对象**的 mNativeContext 的值，对应 C++ mediaplayer **对象**的指针。
* 这里我们 getMediaPlayer() 得到的就是之前在 native_setup() 时（java 层 new MediaPlayer 会走到） new 出来的那个 C++ 的 MediaPlayer。 
* 接下来有用的代码是这一行：`process_media_player_call( env, thiz, mp->setDataSource(fd, offset, length), "java/io/IOException", "setDataSourceFD failed." );`
* 外面包的这层 `process_media_player_call()` 应该是跟 java 层交互的，先不展开看，不影响主线思路。我们只看里面的 `mp->setDataSource(fd, offset, length)`

####2. mp->setDataSource(fd, offset, length)
* 我们先看 `mp->setDataSource(fd, offset, length)`，这是 C++ 类 MediaPlayer 的函数，先看其原型（frameworks/av/include/media/mediaplayer.h），并不是虚函数：

		status_t  setDataSource(int fd, int64_t offset, int64_t length); 

* 其实现如下（frameworks/av/media/libmedia/mediaplayer.cpp）：

		status_t MediaPlayer::setDataSource(int fd, int64_t offset, int64_t length)
		{
		    ALOGV("setDataSource(%d, %" PRId64 ", %" PRId64 ")", fd, offset, length);
		    status_t err = UNKNOWN_ERROR;
		    const sp<IMediaPlayerService>& service(getMediaPlayerService());  // (1).
		    if (service != 0) {
		        sp<IMediaPlayer> player(service->create(this, mAudioSessionId)); // (2).
		        if ((NO_ERROR != doSetRetransmitEndpoint(player)) ||
		            (NO_ERROR != player->setDataSource(fd, offset, length))) {
		            player.clear();
		        }
		        err = attachNewPlayer(player);
		    }
		    return err;
		}

* 这个函数虽然不长，但需要仔细分析

####2.(1). `const sp<IMediaPlayerService>& service(getMediaPlayerService())`

* 这句话就是定义了一个 `const sp<IMediaPlayerService>&` 类型的变量 service，并赋值为 getMediaPlayerService() 的返回值。
* 我们暂且不去看 getMediaPlayerService() 的实现，这个函数就是从 ServiceManager 那里获取到了 BpMediaPlayerService。(详见《深入理解Android卷1-第6章-6.4节》)
* 我们只需要记住，service 是一个 BpMediaPlayerService 即可。

####2.(2). `sp<IMediaPlayer> player(service->create(this, mAudioSessionId))`

* 这句话定义了一个 `sp<IMediaPlayer>` 类型的变量 player，并赋值为 `service->create(this, mAudioSessionId)` 的返回值。 

#### 2.(2).1. `service->create(this, mAudioSessionId)`

* 我们研究一下这个 `service->create(this, mAudioSessionId)`，它是 BpMediaPlayerService 类的成员函数，其原型为：

		virtual sp<IMediaPlayer> create(const sp<IMediaPlayerClient>& client, int audioSessionId) 

* 第一个参数是 IMediaPlayerClient 类型的指针，我们这里传的是 this，即 C++ 的 MediaPlayer 对象 mp。 因为 MediaPlayer 继承自 BnMediaPlayerClient，而 BnMediaPlayerClient 继承自 IMediaPlayerClient。 
* 第二个参数是 int 型的 audioSessionId，我们这里传的是 mAudioSessionId。 我们当年在 new MediaPlayer 时，给它赋了初值如下（具体什么用先不用去管）：

		mAudioSessionId = AudioSystem::newAudioUniqueId();

* 返回值是 `sp<IMediaPlayer>` 类型的。 
* 接下来看一下它的实现（frameworks/av/media/libmedia/IMediaPlayerService.cpp）

		virtual sp<IMediaPlayer> create(const sp<IMediaPlayerClient>& client, int audioSessionId) {
		    Parcel data, reply;
		    data.writeInterfaceToken(IMediaPlayerService::getInterfaceDescriptor());
		    data.writeStrongBinder(IInterface::asBinder(client));
		    data.writeInt32(audioSessionId);
		
		    remote()->transact(CREATE, data, &reply);
		    return interface_cast<IMediaPlayer>(reply.readStrongBinder());
		}

* 核心的一句话是 `remote()->transact(CREATE, data, &reply);`，这会给 BnMediaplayerService 的 onTransact() 函数发去 CREATE 命令，并把需要得到的东西通过 replay 拿出来。这是一次跨进程通信。最后该函数返回的是 `reply.readStrongBinder()` cast 成的 interface。
* 我们先转去看 BnMediaPlayerService 的 onTransact() 实现（IMediaPlayerService.cpp），看看他把什么东西写到了 reply 的 StrongBinder 里:

		status_t BnMediaPlayerService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
		{
		    switch (code) {
		        case CREATE: {
		            CHECK_INTERFACE(IMediaPlayerService, data, reply);
		            sp<IMediaPlayerClient> client = interface_cast<IMediaPlayerClient>(data.readStrongBinder());
		            audio_session_t audioSessionId = (audio_session_t) data.readInt32();
		            sp<IMediaPlayer> player = create(client, audioSessionId);
		            reply->writeStrongBinder(IInterface::asBinder(player));
		            return NO_ERROR;
		        } break;
				...
		}

* 所以，BpMediaPlayerService 的 creat() 最终返回的是从 BnMediaPlayerService 处得到的  sp<IMediaPlayer> 类型的一个 player，该 player 是由 MediaPlayerService 的 create() 函数返回的:`sp<IMediaPlayer> player = create(client, audioSessionId)`,这句话调用的 create()。

####再看Binder
* app 进程给 mediaserver 进程发业务请求：`BpMediaPlayerService.create(client, audioSessionId)`，为了把 client 这个对象发到 mediaserver 进程，把 client 转成了 Binder：`IInterface::asBinder(client)`，然后写到 data 里：`data.writeStrongBinder(IInterface::asBinder(client))`，最后把 data 发给远端进程 mediaserver：`remote()->transact(CREATE, data, &reply);`
* mediaserver 进程收到业务请求，从 data 里读出需要的参数 —— client —— 是个 Binder，强转成 IMediaPlayerClient 类型：`interface_cast<IMediaPlayerClient>(data.readStrongBinder())`，使用它干完事情之后，把想要返回给 app 进程的对象 player，也是先转换成 Binder：`IInterface::asBinder(player)`，然后写到 reply 里:` reply->writeStrongBinder(IInterface::asBinder(player))`
* app 进程会从 reply 里拿到 mediaserver 进程返回的 player，当然还是先读出 Binder 再强转：`interface_cast<IMediaPlayer>(reply.readStrongBinder())`

* app 进程想从 mediaserver 进程拿到的是一个 `sp<IMediaPlayer>` ，但由于是跨进程的，操作系统只能给 app 进程一个 IBinder。app 进程拿到 IBinder 后需要自己转。用 `interface_cast<IMediaPlayer>(IBinder remote)` 转。
* `interface_cast<IMediaPlayer>(IBinder remote)`  实际上就是 new 了一个 BpMediaPlayer 出来，并用 IBinder remote 构造的这个 BpMediaPlayer。这个 remote 最终会成为其成员变量 mRemote 的值。


####真正干活的 create()
* 由上面的分析可知，app 进程最终通过 BpMediaPlayerService 的 create() 得到的东西，是由 MediaPlayerService 的 create() 函数返回的一个 `sp<IMediaPlayer>` 类型的对象。并且这是跨进程完成的。
* 下面我们就看看在远端进程 mediaserver 里，MediaPlayerService 的 create() 函数到底干了什么活。
* 函数位于：`frameworks/av/media/libmediaplayerservice/MediaPlayerService.cpp` ，其实现如下：

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

* Client 的构造函数在 MediaPlayerService.cpp 实现，我们来看一下：

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

####回到mp->setDataSource(fd, offset, length)
* 到此为止，我们才只分析到了`mp->setDataSource(fd, offset, length)`的开头，我们知道这个函数里，从远端进程 mediaserver 拿到了一个真正能干活的 player。
* 接下来的几行是：

        if ((NO_ERROR != doSetRetransmitEndpoint(player)) ||
            (NO_ERROR != player->setDataSource(fd, offset, length))) {
            player.clear();
        }
        err = attachNewPlayer(player);

* 其中的 `doSetRetransmitEndpoint(player)` 我们先不看（这是 MediaPlayer 类的成员函数，别忘了，我们目前还在 `mp->setDataSource(fd, offset, length)` 里面，它直接调用的都是 mp 的成员函数。），不看它并不影响我们的主线思路。

#####1.player->setDataSource(fd, offset, length)
* 首先，这个 setDataSource(fd, offset, length) 是 BpMediaPlayer 的方法，因为 player 是一个 BpMediaPlayer。其实现如下（IMediaPlayer.cpp）：

		status_t setDataSource(int fd, int64_t offset, int64_t length) {
		    Parcel data, reply;
		    data.writeInterfaceToken(IMediaPlayer::getInterfaceDescriptor());
		    data.writeFileDescriptor(fd);
		    data.writeInt64(offset);
		    data.writeInt64(length);
		    remote()->transact(SET_DATA_SOURCE_FD, data, &reply);
		    return reply.readInt32();
		}

* 果然，是给远端进程发了个 `SET_DATA_SOURCE_FD` 消息。看看 BnMediaPlayer 的 onTransact() 吧，不出意料的话应该是分发给 BnMediaPlayer::Client 的 setDataSource() 处理：

        case SET_DATA_SOURCE_FD: {
            CHECK_INTERFACE(IMediaPlayer, data, reply);
            int fd = data.readFileDescriptor();
            int64_t offset = data.readInt64();
            int64_t length = data.readInt64();
            reply->writeInt32(setDataSource(fd, offset, length)); //BnMediaPlayer 没有实现setDataSource()方法，是其子类Client实现的。
            return NO_ERROR;
        }

* BnMediaPlayer::Client setDataSource() 的实现。 注意 Client 虽然是 BnMediaPlayer 的子类，但它是在 MediaPlayerService.cpp 里实现的，并非跟 BnMediaPlayer 一起。

		status_t MediaPlayerService::Client::setDataSource(int fd, int64_t offset, int64_t length)
		{
			...
		    player_type playerType = MediaPlayerFactory::getPlayerType(this,fd,offset,length);  // 1). 
		    sp<MediaPlayerBase> p = setDataSource_pre(playerType);  // 2). 
		    if (p == NULL) {
		        return NO_INIT;
		    }
		
		    // now set data source
		    setDataSource_post(p, p->setDataSource(fd, offset, length)); // 3).
		    return mStatus;
		}

##### 1). MediaPlayerFactory::getPlayerType(this,fd,offset,length)
* 文件位置：`av/media/libmediaplayerservice/MediaPlayerFactory.h`， `av/media/libmediaplayerservice/MediaPlayerFactory.cpp`
* 首先返回值 `player_type` 是一个枚举，在 MediaPlayerInterface.h 定义如下：

		enum player_type {
		    STAGEFRIGHT_PLAYER = 3,
		    NU_PLAYER = 4,
		    // Test players are available only in the 'test' and 'eng' builds.
		    // The shared library with the test player is passed passed as an
		    // argument to the 'test:' url in the setDataSource call.
		    TEST_PLAYER = 5,
		};

* 函数的实现如下：

		player_type MediaPlayerFactory::getPlayerType(const sp<IMediaPlayer>& client,int fd, int64_t offset,int64_t length) 
		{
		    GET_PLAYER_TYPE_IMPL(client, fd, offset, length);
		}

* 宏替换之后：

		player_type MediaPlayerFactory::getPlayerType(const sp<IMediaPlayer>& client,int fd, int64_t offset,int64_t length) 
		{
		    Mutex::Autolock lock_(&sLock);                      
		                                                        
		    player_type ret = STAGEFRIGHT_PLAYER;               
		    float bestScore = 0.0;                              
		                                                        
		    for (size_t i = 0; i < sFactoryMap.size(); ++i) {   
		                                                        
		        IFactory* v = sFactoryMap.valueAt(i);           
		        float thisScore;                                
		        CHECK(v != NULL);                               
		        thisScore = v->scoreFactory(client, fd, offset, length, bestScore);      
		        if (thisScore > bestScore) {                    
		            ret = sFactoryMap.keyAt(i);                 
		            bestScore = thisScore;                      
		        }                                               
		    }                                                   
		                                                        
		    if (0.0 == bestScore) {                             
		        ret = getDefaultPlayerType();                   
		    }                                                   
		                                                        
		    return ret;
		}

* 这里能看出来是怎么决定 `player_type` 的， 到底是 NuPlayer 呢 还是 Stagefright 呢 还是 `test_player` 呢？
* sFactoryMap 是 tFactoryMap 类型的变量，在 MediaPlayerFactory.cpp 里定义，是个**全局变量**。
* tFactoryMap 是 `typedef KeyedVector<player_type, IFactory*> tFactoryMap;`
* sFactoryMap 只有在函数 `registerFactory_l()` 里才增加元素，而 `registerFactory_l() `只在 MediaPlayerFactory.cpp 里被调用过2次：`registerFactory_l(new NuPlayerFactory(), NU_PLAYER)、registerFactory_l(new TestPlayerFactory(), TEST_PLAYER)`，这里只加了 NuPlayer 和 TestPlayer，没有加 StageFright。看上面的代码可以看到，默认是 StageFright——0分，如果任何一个 player 的评分比 StageFright 高了，则使用高分的那个。所以没必要把 StageFright 加进来。
* 每一种 player 想给自己评分，必须自己去实现 scoreFactory() 方法。
* 如果不实现，则默认返回0.0。见 MediaPlayerFactory.h。
* scoreFactory() 可能的参数有，因为 scoreFactory() 的参数跟 getPlayerType() 是一模一样的，宏替换来的。所以 getPlayerType() 有几种可能的重载，scoreFactory() 就有几种。而 getPlayerType() 又跟 setDataSource() 一致。所以，理论上每种 player 都应该实现所有 setDataSource() 可能的参数列表。如果不实现该参数列表，则说明该 player 不支持这种类型的 DataSource，返回 0.0，不会选到该 player。
* 我们看到 NuPlayerFactory（在 MediaPlayerFactory.cpp 里定义） 并没有重写 `scoreFactory(const sp<IMediaPlayer>&,int,int64_t,int64_t,float)`，所以这里最后会走到 `getDefaultPlayerType()`，还是会 `return NU_PLAYER`。看来 Android 已经彻底放弃 StageFright 了。
* 具体怎么打分的策略之类的就不展开看了
* 总之到此为止，`getPlayerType(this,fd,offset,length)` 得到了一个枚举值： `NU_PLAYER`

##### 2). sp<MediaPlayerBase\> p = setDataSource_pre(playerType)
* 该函数返回的是 MediaPlayerBase
* 该函数是 MediaPlayerService::Client 的成员函数， 在 MediaPlayerService.cpp 里实现：

		sp<MediaPlayerBase> MediaPlayerService::Client::setDataSource_pre(player_type playerType)
		{
		    // create the right type of player
		    sp<MediaPlayerBase> p = createPlayer(playerType); // 2).1
			...
		    return p;
		}

* 这个 `createPlayer(playerType)` 是 MediaPlayerService::Client 的成员函数，如下：

		sp<MediaPlayerBase> MediaPlayerService::Client::createPlayer(player_type playerType)
		{
		    // determine if we have the right player type
		    sp<MediaPlayerBase> p = mPlayer;
		    if ((p != NULL) && (p->playerType() != playerType)) {
		        ALOGV("delete player");
		        p.clear();
		    }
		    if (p == NULL) {
		        p = MediaPlayerFactory::createPlayer(playerType, this, notify, mPid);
		    }
		
		    if (p != NULL) {
		        p->setUID(mUID);
		    }
		
		    return p;
		}

* Client 有个成员变量，叫 mPlayer，是 MediaPlayerBase 类型的。

##### 3). setDataSource_post(p, p->setDataSource(fd, offset, length))
#####2.player.clear();
#####3.attachNewPlayer(player)
