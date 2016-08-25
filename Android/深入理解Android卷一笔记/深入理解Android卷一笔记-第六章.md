### 业务层整体逻辑
* 有 3 个东西： Client、Server、ServiceManager
* Server 向 ServiceManager 注册 Service，Client 向 ServiceManager 查询 Service，Client 使用 Server 的 Service
* Server 向 ServiceManager 注册 Service 的过程中，Server 相当于 Client，ServiceManager 相当于 Server

#### Client 和 Server 交互的一般流程
* Client 先调用 defaultServiceManager() 获取一个 IServiceManager 的指针（实际上是其子类 BpServiceManager 的指针）: `sp<IServiceManager> sm = defaultServiceManager();`
	* 注：函数 defaultServiceManager() 是 Android binder 组件的一部分，在 IServiceManager.cpp 中实现的。
* 然后 Client 利用这个指针，向 ServiceManager 查询 Service，得到这个 Service 的 IXXXService 指针。查询 Service 分为两小步，以查询 MediaPlayerService 为例：
	1. `sp<IBinder> binder = sm->getService(String16("media.player");`： 得到 binder
	2. `sp<IMediaPlayerService> service = interface_cast<IMediaPlayerService>(binder);` ： 把 binder cast 成 IMediaPlayerService（实际上是其子类 BpMediaPlayerService）。 
* 最后 Client 用得到的 IMediaPlayerService 指针，调用该 Service 提供的业务函数。 `service->create(this, mAudioSessionId);`

#### 再捋一次
* 一个 Service，要提供一个接口类来供 Client 调用其业务函数，一般这个接口类叫做 IXXXService。
* 比如上面的 IMediaPlayerService 就是 MediaPlayerService 的接口类，它的 create() 函数就是提供给 Client 的业务函数之一。
* ServiceManager 本身也是一个 Service，它提供给 Client 的业务函数就是 getService()，addService() 等等
* 所以 ServiceManager 也有一个接口类，IServiceManager，上面的过程中 Client 就调用了这个接口里的 getService() 函数来查询 Service。
* 得到普通 Server 的接口类的指针，需要向 ServiceManager 查询，方法是调用 IServiceManager 的 getService();
* 得到 ServiceManager 的接口类指针，binder 系统为我们提供了方便函数： defaultServiceManager()

#### Server 向 ServiceManager 注册 Service
* 上面说过，Server 向 ServiceManager 注册 Service 的过程中，Server 相当于 Client，ServiceManager 相当于 Server
* 此时的流程跟 Client 使用 Service 是一个道理
* Server 先调用 defaultServiceManager() 获取一个 IServiceManager 的指针（实际上是其子类 BpServiceManager 的指针）:`sp<IServiceManager> sm = defaultServiceManager();`
* Server 得到这个指针时候，调用 ServiceManager 的业务函数 addService() 来注册自己的 Service：`sm->addService(String16("media.player"), new MediaPlayerService());`

#### 业务层小结
* 其实抛开底层 binder 具体是怎么实现的，业务层已经很简单清晰了
* 业务层使用起来流程也相对固定，没什么难理解的
* 接下来我们稍微深入一点（但还没深到binder那么深），看看作为一个 Service，是怎么提供服务的，Client 是怎么使用这个服务的。