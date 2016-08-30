##API的使用
* API函数是命令驱动的  
* 统一通过接口函数`ihevcd_cxa_api_function()`来调用API。  
* 把命令作为参数传给这个函数，调用结果通过参数带出。  
* 函数原型（ihevcd\_api.c）是： 
 
		IV_API_CALL_STATUS_T ihevcd_cxa_api_function(iv_obj_t *ps_handle, void *pv_api_ip, void *pv_api_op)

* `ps_handle` 可以是NULL或者decoder\_obj（有的命令需要decoder，有的命令不需要）
* `pv_api_ip` 是传入的命令(实际上是封装了命令的结构体)
* `pv_api_op` 是API返回的东西
* 返回值是枚举型，可能值有：成功、失败、NA。  

* 所谓命令就用一个枚举常量，比如：`IV_CMD_INIT`，`IVD_CMD_VIDEO_DECODE`  
* 所有的命令如下：  
	* Codec有关的命令定义(iv.h)（都是以IV\_CMD开头的。）:  

			/* IV_API_COMMAND_TYPE_T:API command type                                   */
			typedef enum {
			    IV_CMD_NA                           = 0x7FFFFFFF,
			    IV_CMD_GET_NUM_MEM_REC              = 0x0,
			    IV_CMD_FILL_NUM_MEM_REC             = 0x1,
			    IV_CMD_RETRIEVE_MEMREC              = 0x2,
			    IV_CMD_INIT                         = 0x3,
			    IV_CMD_DUMMY_ELEMENT                = 0x4,
			}IV_API_COMMAND_TYPE_T;

	* Decode有关的命令定义如下(ivd.h)（都是以IVD\_CMD开头的。）：  
	* 其中CONTROL\_API\_COMMAND一般作为CMD\_VIDEO\_CTL的子命令，这种命令都是以IVD\_CMD\_CTL开头的。

			/* IVD_API_COMMAND_TYPE_T:API command type                                   */
			typedef enum {
			    IVD_CMD_VIDEO_NA                          = 0x7FFFFFFF,
			    IVD_CMD_VIDEO_CTL                         = IV_CMD_DUMMY_ELEMENT + 1,
			    IVD_CMD_VIDEO_DECODE,
			    IVD_CMD_GET_DISPLAY_FRAME,
			    IVD_CMD_REL_DISPLAY_FRAME,
			    IVD_CMD_SET_DISPLAY_FRAME
			}IVD_API_COMMAND_TYPE_T;
			
			/* IVD_CONTROL_API_COMMAND_TYPE_T: Video Control API command type            */
			typedef enum {
			    IVD_CMD_NA                          = 0x7FFFFFFF,
			    IVD_CMD_CTL_GETPARAMS               = 0x0,
			    IVD_CMD_CTL_SETPARAMS               = 0x1,
			    IVD_CMD_CTL_RESET                   = 0x2,
			    IVD_CMD_CTL_SETDEFAULT              = 0x3,
			    IVD_CMD_CTL_FLUSH                   = 0x4,
			    IVD_CMD_CTL_GETBUFINFO              = 0x5,
			    IVD_CMD_CTL_GETVERSION              = 0x6,
			    IVD_CMD_CTL_CODEC_SUBCMD_START         = 0x7
			}IVD_CONTROL_API_COMMAND_TYPE_T;

* 不同的命令有不同的参数，所以为每个命令封装了一个结构体，结构体包含命令本身及其需要的参数。  
* 比如 `ivd_video_decode_ip_t` 就是封装了 `IVD_CMD_VIDEO_DECODE` 命令的结构体： 
 
		/*****************************************************************************/
		/*   Video Decode                                                            */
		/*****************************************************************************/
		
		
		/* IVD_API_COMMAND_TYPE_T::e_cmd = IVD_CMD_VIDEO_DECODE                      */
		
		
		typedef struct {
		    /**
		     * u4_size of the structure
		     */
		    UWORD32                                 u4_size;
		
		    /**
		     * e_cmd
		     */
		    IVD_API_COMMAND_TYPE_T                  e_cmd;
		
		    /**
		     * u4_ts
		     */
		    UWORD32                                 u4_ts;
		
		    /**
		     * u4_num_Bytes
		     */
		    UWORD32                                 u4_num_Bytes;
		
		    /**
		     * pv_stream_buffer
		     */
		    void                                    *pv_stream_buffer;
		
		    /**
		     * output buffer desc
		     */
		    ivd_out_bufdesc_t                       s_out_buffer;
		
		}ivd_video_decode_ip_t;


* 每次调用API之前，需要定义一个输入命令结构体、一个返回值接收结构体，然后把这两个结构体传入接口函数。  比如：

	    ivd_video_decode_ip_t s_video_decode_ip;  //定义输入命令结构体
        ivd_video_decode_op_t s_video_decode_op;  //定义接收API调用结果的结构体
       	 
		/* 设置命令参数  */
		s_video_decode_ip.e_cmd = IVD_CMD_VIDEO_DECODE;  //设置命令为IVD_CMD_VIDEO_DECODE
        s_video_decode_ip.u4_ts = u4_ip_frm_ts;   
        s_video_decode_ip.pv_stream_buffer = pu1_bs_buf;
        s_video_decode_ip.u4_num_Bytes = u4_bytes_remaining;
        s_video_decode_ip.u4_size = sizeof(ivd_video_decode_ip_t);
        s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[0] =
                        ps_out_buf->u4_min_out_buf_size[0];
        s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[1] =
                        ps_out_buf->u4_min_out_buf_size[1];
        s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[2] =
                        ps_out_buf->u4_min_out_buf_size[2];

        s_video_decode_ip.s_out_buffer.pu1_bufs[0] =
                        ps_out_buf->pu1_bufs[0];
        s_video_decode_ip.s_out_buffer.pu1_bufs[1] =
                        ps_out_buf->pu1_bufs[1];
        s_video_decode_ip.s_out_buffer.pu1_bufs[2] =
                        ps_out_buf->pu1_bufs[2];
        s_video_decode_ip.s_out_buffer.u4_num_bufs =
                        ps_out_buf->u4_num_bufs;

        s_video_decode_op.u4_size = sizeof(ivd_video_decode_op_t);

        /*****************************************************************************/
        /*   API Call: Video Decode   
		/*   调用API                                               */
        /*****************************************************************************/
        ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_video_decode_ip,
                                   (void *)&s_video_decode_op);

        /*  使用API调用结果  */
		if(1 == s_video_decode_op.u4_output_present)
        {
            dump_output(ps_app_ctx, &(s_video_decode_op.s_disp_frm_buf),
                        s_video_decode_op.u4_disp_buf_id, ps_op_file,
                        ps_op_chksum_file,
                        *pu4_op_frm_ts, ps_app_ctx->u4_file_save_flag,
                        ps_app_ctx->u4_chksum_save_flag);

            (*pu4_op_frm_ts)++;
        }  
  
##关于main.c的参数
* 必须传`--num_frames`, 不传的话只解0帧，就是不解码。可以传一个-1，相当于传了一个很大的正数，这样会把整个文件解完。  
* 必须传`--input`，不然不知道输入文件是什么。  
* 其他参数都可选，几个常用的如下：  
	* `--save_output` 后面跟0或者1,0表示不保存输出文件。默认是0。  
	* `--output` 后面跟输出yuv文件名，当`--save_output` 设为1的时候，这里要设一下。  
	* `--chroma_format` 后面一般跟 "YUV\_420P", 指定输出YUV文件的格式。当需要保存输出文件时，这里应该设置一下。不然默认是YUV\_420SP\_UV  
	* `--display` 开启display，默认是不开启。  
  
##关于main.c的用法
* **解码主循环**  
* 最主要的循环是在：  
  
		while(u4_op_frm_ts < (s_app_ctx.u4_max_frm_ts + s_app_ctx.disp_delay))

* `u4_op_frm_ts` 表示当前是第几帧，从0开始，每次循环加1。  
* `s_app_ctx.u4_max_frm_ts` 表示总共需要解多少帧，是用户通过参数设的。当然用户可以设个巨大的数，循环会在整个input文件消耗光的时候退出的。  
* `s_app_ctx.disp_delay` 只有在开启了宏`APP_EXTRA_BUFS`的时候才有意义，一般情况下是0。  
* 这个循环每进行一次，解码一帧。  

* **关于解码时间统计**  
* main.c本身提供了这个功能，不用自己写代码做了。只需要打开一个宏即可：`PROFILE_ENABLE`  
* 打开这个宏之后，main.c会把解码每一帧用的时间，每一帧的字节数等等信息，以及一个summary信息，打印到控制台。  
* 不过Android上看不到printf()出来的这些信息，所以需要修改一下，把相关的地方改成fprintf()，写文件里就行了。  
* main.c统计的方式是：每次以Decode命令调用API函数前记录下时间，函数返回后再记录一下时间。两个时间差就认为是解码这帧的时间。  

* **在主循环之前**  
* 在进入解码主循环之前，通过不同的命令调用了若干次API，主要做了初始化、内存分配等工作。  
* 还在正式解码之前，把decoder设置为“解码文件头”的模式，解析了一下输入文件的头部，获得了分辨率信息。  
* 详细的也没有太细看。
