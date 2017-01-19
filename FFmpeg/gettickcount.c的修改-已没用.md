##gettickcount.c  

###1.SHANDLE_PTR  重定义；不同的基类型

####根本原因： 
* gettickcount.c 里 同时 include 了 unistd.h 和 hxtypes.h
* unistd.h 是 project generator 生成的，而非 Linux 标准的。（这也是为了能成功把 ffmepg port 到 Windows。）
* project generator 生成的这个 unistd.h 里， include 了 BaseTsd.h 这个 VisualStudio 的头文件，BaseTsd.h 里定义了 `SHANDLE_PTR`。
* 而 helix 本身也定义了 `SHANDLE_PTR`，在 hxbastsd.h 里。 Helix 这么做，正是为了避免 include 微软的 BaseTsd.h，因为 Helix 很多定义的东西跟微软本身的不兼容。
* 说白了，根本原因是 project generator 不知道 Helix 跟 BaseTsd.h 不兼容，所以在它生成的通用头文件 unistd.h 里无条件的 Include 了它。而 Helix 又不知道只要 Include 了 unistd.h 就会 Include BaseTsd.h。

####解决方法：
* 让 gettickcount.c 不去 Include project generator 生成的这个 unistd.h，而去 Include 自己的  unistd.h。
* Helix 自己的 unistd.h 首先会检测是否有 _UNIX 这个宏，然后再去 include <unistd.h>
* 修改方法是，把gettickcount.c里的， `include <unistd.h>` 改为 `include "include/hlxclib/unistd.h" `

####其他
* ffmpeg 自己的代码也很多地方直接 `include <unistd.h>`（并没有判断是否 define 了 _UNIX）
* 之所以没有问题，是因为 ffmpeg 的代码跟微软的 BaseTsd.h 并不冲突   

###2.无法打开包括文件: “hlxosstr.h”

####根本原因
* hxtick.h include 了 hlxclib/windows.h, hlxclib/windows.h include 了 hlxosstr.h，而 hlxosstr.h 根本不存在
* hlxclib/windows.h 在 include hlxosstr.h 时，是通过宏 _WINDOWS 判断的。 为何 MinGW 就没事儿呢？
* project generator 的 cl 命令行，确实加了这个宏：`/D"_WINDOWS"`

####解决方法：
* 我们不能去掉 `/D"_WINDOWS"` 来规避这个问题，因为日后生成 Windows 可执行程序时，没准儿 VS 也会加这个宏
* 所以只能让 Helix 无论如何都不 include hlxosstr.h 这个不存在的文件了
* 修改方法是，把 libavcodec/include/hxtick.h 里的这一行注释掉： `#include "hlxclib/windows.h"`

###3.“CLOCK_MONOTONIC”: 未声明的标识符

####根本原因：
* `CLOCK_MONOTONIC` 是 Linux 的 glibc 实现的，Windows 根本没有。虽然 `CLOCK_MONOTONIC` 是 time.h 里的，虽说 time.h 是标准c 的一部分，但 time.h 在不同的操作系统有不同的实现的。
* 尝试用 Helix 自己的 time.h（`#include "include/hlxclib/time.h"`） 替换微软的 time.h（`#include <time.h>`） ，还是不行，仍然有同样的错误

####解决方法：
* 学习 ffmpeg 的代码，用到 `CLOCK_MONOTONIC` 的地方都加宏保护起来
* 修改 gettickcount.c中的两处 `#ifdef HELIX_CONFIG_USE_CLOCK_GETTIME` 为 `#if defined(HELIX_CONFIG_USE_CLOCK_GETTIME) && defined(CLOCK_MONOTONIC)`
* 这样搞以后又会说： 使用未定义的 struct“timeval”
* ？？？？？？？？？？？？？？？？？？？？

####其他：
* ffmpeg 也有用到 `CLOCK_MONOTONIC` 的地方，在 libavutil 里的 time.c 和 libavdevice 的 v4l2.c
* 它都用宏 `#if HAVE_CLOCK_GETTIME && defined(CLOCK_MONOTONIC)` 保护起来了。

###4.根本原因

* gettickcount.c 这个文件根本就没考虑过用 native 的 Windows 来编译，像 sys/time.h 这种头文件，没有任何宏保护，直接就 include。
* 而 ffmpeg 都是有所保护的，比如 sys/time.h 就是如此保护的：

		#if HAVE_GETTIMEOFDAY
		#include <sys/time.h>
		#endif
* 如果 configure ffmpeg 时选择的 toolchain 是 msvc，则 生成的 config.h 里，`HAVE_GETTIMEOFDAY` 为 0
* 根据 gettickcount.c 的代码看出，它主要是定义了 GetTickCount() 和 GetTickCountInUSec() 这两个函数。
* 对于 GetTickCount()， windows.h 就定义了该函数，且作用跟 gettickcount.c 实现的一样，返回一个毫秒数。gettickcount.c 实现时首选 `clock_gettime()` 这个 Linux 的函数，如果没有这个函数（用宏 `HELIX_CONFIG_USE_CLOCK_GETTIME` 控制），则再想别的办法。
* 因为 Windows 上直接就有 GetTickCount()，所以我们不再实现一遍了。只 include windows.h 即可。
* 对于 GetTickCountInUSec()，Windows 上也没有这个函数，我们简单实现一下，返回一个微妙数，简单的把毫秒乘以1000。这当然是不准确的，但只是为了意思一下。
* 遗留问题： gettickcount.c 实现的 GetTickCount() 返回类型是 ULONG32，而 Windows 的返回类型是 DWORD。然后 Helix 本来很多类型定义就跟 Windows 不兼容。所以可能后续会有问题。