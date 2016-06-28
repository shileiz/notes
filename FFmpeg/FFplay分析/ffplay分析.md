### 说明
* 分析的 ffplay.c 的版本是 ffmpeg-release-2.8 源码里带的，从 [git](https://github.com/FFmpeg/FFmpeg/tree/release/2.8) 上下载的。


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