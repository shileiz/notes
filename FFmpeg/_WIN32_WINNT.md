## `_WIN32_WINNT` 介绍
* 微软的 windows sdk 根据这个宏判断 Windows 的版本，从而决定某些 windows sdk 该使用哪个版本。
* 举个例子，在 WinSock2.h 里就有如下代码段：

		#if(_WIN32_WINNT >= 0x0600)
		
		...		
		typedef struct pollfd {
		
		    SOCKET  fd;
		    SHORT   events;
		    SHORT   revents;
		
		} WSAPOLLFD, *PWSAPOLLFD, FAR *LPWSAPOLLFD;
		
		#endif // (_WIN32_WINNT >= 0x0600)

* 以下是在 [msdn](https://msdn.microsoft.com/en-us/library/6sehtctf.aspx) 查到的，目前这个宏的值所代表的各个版本的 Windows：


		#define _WIN32_WINNT_NT4                    0x0400 // Windows NT 4.0  
		#define _WIN32_WINNT_WIN2K                  0x0500 // Windows 2000  
		#define _WIN32_WINNT_WINXP                  0x0501 // Windows XP  
		#define _WIN32_WINNT_WS03                   0x0502 // Windows Server 2003  
		#define _WIN32_WINNT_WIN6                   0x0600 // Windows Vista  
		#define _WIN32_WINNT_VISTA                  0x0600 // Windows Vista  
		#define _WIN32_WINNT_WS08                   0x0600 // Windows Server 2008  
		#define _WIN32_WINNT_LONGHORN               0x0600 // Windows Vista  
		#define _WIN32_WINNT_WIN7                   0x0601 // Windows 7  
		#define _WIN32_WINNT_WIN8                   0x0602 // Windows 8  
		#define _WIN32_WINNT_WINBLUE                0x0603 // Windows 8.1  
		#define _WIN32_WINNT_WINTHRESHOLD           0x0A00 // Windows 10  
		#define _WIN32_WINNT_WIN10                  0x0A00 // Windows 10  

## ffmpeg 里的使用
* ffmpeg 若干地方用到了这个宏，根据不同的 Windows 版本，做不同事情。
* 比如在 dxva2.h 里就有如下代码段：

		#if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0602
		#undef _WIN32_WINNT
		#define _WIN32_WINNT 0x0602
		#endif

* 这段代码还挺暴力，直接把 `_WIN32_WINNT` 强行定义成 0x0602，即 Windows 8

## Producer 里的使用
* 在 librv11enc.c 和 librv40enc.c 里用到了这个宏，相关代码段如下：

		#if defined(_WIN32)
		#define _WIN32_WINNT 0x0502
		#include <windows.h>
		#define CC __stdcall
		#else
		#define CC
		#endif

* 用法也比较暴力，如果是 WIN32 平台，则强行把 `_WIN32_WINNT` 重新定义为 0x0502，即 Windows Server 2003 
* 这样 ， window.h 里的 sdk 就只能使用那些低于这个版本的 sdk 了