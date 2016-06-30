### 说明
* 分析的 ffplay.c 的版本是 ffmpeg-release-2.8 源码里带的，从 [git](https://github.com/FFmpeg/FFmpeg/tree/release/2.8) 上下载的。

### 大块儿
* main 进来先做了一些初始化和判断，然后进入 `stream_open`, `stream_open` 函数会开一个线程：`read_thread`
* 然后 main 函数（主线程）就进入了 event_loop , event_loop 其实就是视频的显示线程。后面会单独分析。
* `read_thread` 线程会先做一些初始化和判断，然后分别对 video/audio/subtitle 调用 `stream_component_open`
* `stream_component_open` 在做完初始化后，分别为 video/audio/subtitle 启动了各自的线程： `video_thread/audio_thread/subtitle_thread`
* `read_thread` 打开 video/audio/subtitle 各自的线程后，就进入了自己的主循环： 不停的把 packet 读入 `is->videoq/is->audioq/is->subtitleq`

### 大块儿总结
* 算上主线程一共5个线程：`main、read_thread、audio_thread、video_thread、subtitle_thread`
* main 是 video 的显示线程，从 pictq 里拿出图像，在“适当的”时间显示到屏幕上（或丢弃）。另外该线程还负责相应鼠标键盘等 event。
* read thread 是读包线程，可以理解为 demux 线程，负责从文件里读出 packet，放入对应的队列里（`is->videoq、is->audioq、is->subtitleq`）
* video thread 是视频解码线程，从 videoq 里拿出 packet，解成 frame，放入 pictq。

### video_thread

`video_thread` 在循环干这件事儿：

	从 `is->viddec->queue` 拿出一个 pkt, 并把它解码到局部变量 frame（AVFrame类型）里。
	然后把 frame (转换成 Frame 类型) 压入队列 is->pictq 。
	被压入的 frame 的 pts， duration，pos，serial 被设置好了（当然，它的 bmp 也被分配好了）：
	pts = best_effort_timestamp * time_base
	duration = 1/framerate;  // framerate = av_guess_frame_rate(is->ic, is->video_st, NULL);
	pos = av_frame_get_pkt_pos(frame)
	serial = is->viddec.pkt_serial

### FrameQueue 及其几个相关函数
* 首先要知道 Frame：Frame 是个自定义结构体，对于视频，用它来表示一个图片。音频用Frame表示Sample，Frame还能表示字幕。
* ffplay 里用 is->pictq 来存储解码出来的视频图片，这就是一个 FrameQueue
* FrameQueue 如下：

		typedef struct FrameQueue {
		    Frame queue[FRAME_QUEUE_SIZE]; /* 存放frame的数组 */
		    int rindex;                    /* 读指针，即队尾 */
		    int windex;                    /* 写指针，即队头 */
		    int size;                      /* 当前queue里有几个frame */
		    int max_size;                  /* 最多能装几个frame */
		    int keep_last;                 /* 是否保证队尾frame被显示后才能移动队尾（读）指针，视频被设为1，音频和字幕被设为0 */
		    int rindex_shown;              /* 读指针指向的那个frame是否已经被显示过了，是则等于1,否则等于0 */
		    SDL_mutex *mutex;
		    SDL_cond *cond;
		    PacketQueue *pktq;
		} FrameQueue;

* 几个重要的函数如下：

		/* 返回尚未显示的那个frame，注意函数调用后指针不动 */		
		static Frame *frame_queue_peek(FrameQueue *f)
		{
		    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
		}

		/* 返回尚未显示的那个frame的下一个frame，注意函数调用后指针不动 */	
		static Frame *frame_queue_peek_next(FrameQueue *f)
		{
		    return &f->queue[(f->rindex + f->rindex_shown + 1) % f->max_size];
		}
		
		/* 返回读指针指向的frame，注意函数调用后指针不动 */	
		static Frame *frame_queue_peek_last(FrameQueue *f)
		{
		    return &f->queue[f->rindex];
		}
	
		/* 移动写指针，size++ */
		static void frame_queue_push(FrameQueue *f)
		{
		    if (++f->windex == f->max_size)
		        f->windex = 0;
		    SDL_LockMutex(f->mutex);
		    f->size++;
		    SDL_CondSignal(f->cond);
		    SDL_UnlockMutex(f->mutex);
		}
		
		/* 
			对于视频： 
			如果读指针指向的（队尾）frame尚未显示，则把 rindex_shown 置1，读指针不用动，size不变。
			否则读指针移动一位，size--。
		 */
		static void frame_queue_next(FrameQueue *f)
		{
		    if (f->keep_last && !f->rindex_shown) {
		        f->rindex_shown = 1;
		        return;
		    }
		    frame_queue_unref_item(&f->queue[f->rindex]);
		    if (++f->rindex == f->max_size)
		        f->rindex = 0;
		    SDL_LockMutex(f->mutex);
		    f->size--;
		    SDL_CondSignal(f->cond);
		    SDL_UnlockMutex(f->mutex);
		}
		
		/* jump back to the previous frame if available by resetting rindex_shown */
		/* 把 rindex_shown 置0,返回置0之前的 rindex_shown 值 */
		static int frame_queue_prev(FrameQueue *f)
		{
		    int ret = f->rindex_shown;
		    f->rindex_shown = 0;
		    return ret;
		}

##### pictq 里图片的进
* 前面已经说了，是 `video_thread` 负责往 pictq 里写图片
* 写入的图片的 pts 是以秒为单位的（已经乘以了相应的 time base）

##### pictq 里图片的出
* 一般情况下，`rindex_shown` == 1
* 即，一般情况下，`rindex` 指向刚刚显示过的那个 pict，而 `rindex+rindex_shown` 指向尚未显示即将要显示的那个 pict
* 所以我们用 `frame_queue_peek` 得到即将要显示的 pict，用 `frame_queue_peek_last` 得到刚刚显示过的 pict
* 每当我们显示完一张图片（显示的是 `rindex+rindex_shown` 指向的图片），就把 `rindex++`，让读指针前进一步，而 `rindex_shown` 保持不变。
* 这就仍然保持了 pictq 里，`rindex` 指向刚刚显示过的那个 pict，而 `rindex+rindex_shown` 指向尚未显示即将要显示的那个 pict 的状态。

### `av_gettime_relative`
* 获取当前时间，以微妙为单位。这个函数获取的当前时间是从系统（电脑/手机）启动算起的，而不是1970年1月1日0点0分。
* `av_gettime` 获取的时间是从1970年1月1日0点0分开始的。也是以微妙为单位。
* 其实在 ffplay 里使用这两个函数哪个都行，反正是用来计算相对值的，只要能体现出两次到达同一个代码点流逝的时间就可以了。

### 主循环：`event_loop`
* main 函数的最后进入 `event_loop(is)`, 这是个死循环永不返回
* 把 `event_loop` 稍作展开是这样子的：

		for(;;){
			while(拿不到event){
				av_usleep(remaining_time 秒);
				remain_time = 0.01;
				video_refresh(is,&remaining_time);
			}
			处理event（键盘、鼠标等事件）
		} 
* 可见，主循环里一直在进行 `sleep-->video_refresh` 的循环，除非有 event 发生时，偶尔出来处理一下event，然后立刻又回到这个循环。
* 具体 sleep 多少时间：remaining_time 秒，这个数是由 `video_refresh()` 设定的。
* `video_refresh()` 干的事儿就是：显示下一幅图像，如果尚未到显示这张图像的时间，则不显示直接返回，并把需要等多久通过参数 `remaining_time` 带出来。


### avsync：音视频同步

##### 一些前提
* 先澄清一下在后续分析中的用语：
	* 当前帧 == 当前图像 == 即将显示、尚未显示的图像
	* 前一帧 == 刚刚显示过的图像
	* 下一帧 == 当前帧的下一帧 == 即将显示的图像的下一幅图像
* 按照 ffplay 默认的，以音频时钟作为主时钟，即视频向音频对其为例进行分析

##### `is->frame_timer`
* `frame_timer`: 表示的是某次准备显示一副图像时的 time，并从那时开始累加每一次的显示图像前的 delay。
* 一旦 `frame_timer` 和 time 差的太多了，就把 `frame_timer` 调整为 time。
* 理论上，`frame_timer` 应该一直等于 time。
* 因为 `frame_timer` = 上次显示图片之前得到的time + 上一张图片需要的delay ，
* 而 time 等于当前准备要显示图片之前得到的时间
* 但是整个 ffplay 运行需要一点点时间，比如计算 delay 啥的，还有循环里的其他一些判断，多多少少需要一点时间
* 这样，`frame_timer` 就渐渐跟 time 拉开了差距。这时候就需要把 `frame_timer` 更新成当前的 time。
* 可以理解为，`frame_timer` 是实际发生的所有delay的累积，所谓“实际发生”是因为delay针对音视频同步做了调整，而不仅仅是上一帧图片的 duration。并且，这个累积值归一化到了当前时间time上。

##### avsync
* 不同步是常态，同步只是某一个瞬间的偶然态
* 我们不停的调整视频的 delay，实现一个动态的音视频同步。
* 最核心的其实跟 Tutorial 里讲的一样：视频太慢了就把delay设置为0，视频太快了就把 delay 乘以2。
* 另外加入了丢帧机制：当 `time > is->frame_timer + duration` 的时候丢掉视频帧。

