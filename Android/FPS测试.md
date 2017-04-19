## FPS 测试大概有以下几种方案（百度的）：

1. 开启系统的“GPU呈现模式分析”，然后用 dumpsys gfxinfo 把 fps 信息输出到文件里。参考：[http://www.cnblogs.com/summer-sun/p/5524663.html](http://www.cnblogs.com/summer-sun/p/5524663.html)
2. 用 FPS Meter 软件，这个之前用过的，没办法自动化。
3. 用命令 `adb shell dumpsys SurfaceFlinger --latency <window_activity>`  获取 FPS,然后用脚本分析结果。 参考：[http://blog.csdn.net/itfootball/article/details/43084527](http://blog.csdn.net/itfootball/article/details/43084527)  


## FPS 能反映视频流畅度吗？
* 参考：[http://blog.csdn.net/xiaosongluo/article/details/51212296](http://blog.csdn.net/xiaosongluo/article/details/51212296)
* 参考：[http://blog.csdn.net/luoshengyang/article/details/7846923](http://blog.csdn.net/luoshengyang/article/details/7846923)

## 使用方案3进行FPS测试
* Google 已经有了现成的一套 python 脚本，使用方案3进行FPS测试，这套脚本随 Android 源码发布。
* 上述参考文档里使用的，也是这套脚本。
* 下面主要介绍该套脚本的使用方法，然后分析一下其原理。

### Google 脚本的使用方法
* 以 Android 源码 5.1 为例，这套脚本在这里：

		external/chromium_org/build/android

* 我们只需要它的一小部分，如下：

		external/chromium_org/build/android/pylib 整个目录
		external/chromium_org/build/android/surface_stats.py 单个文件

* 运行 surface_stats.py 即可进行 FPS 的测试，前提是已经 adb 连接了被测设备。
* 第一次运行会报错，说：

		ImportError: No module named errors
		ImportError: No module named adb_interface

* 这是因为我们还需要 Google 的另外一套 python 库，在 Android 源码的这里：

		external/chromium_org/third_party/android_testrunner

* 把该目录放到跟 `surface_stats.py` 平级，在该目录里加个 `__init__.py` 使之成为模块。把 import errors 和 `adb_interface` 的地方修改一下，改成：

		from android_testrunner import errors
		from android_testrunner import adb_interface

* 另外，该脚本需要在 Linux 上运行，因为 Windows 不支持 `preexec_fn`： 
 
		preexec_fn is not supported on Windows platforms

* 运行成功后，每隔一秒会打印一行数据，格式如下，其中最后一列就是 fps：

		max_frame_delay (vsyncs)  frame_lengths (vsyncs)  jank_count (janks)  avg_surface_fps (fps) 
		------------------------  ----------------------  ------------------  --------------------- 
		                       7                   4.889                   5                  12.00 
		                      12                   7.167                   2                   8.00

* 注意，如果测试过程中屏幕没有任何刷新，则不会有数据打印出来。
* 另外，运行脚本时可以加上 -d 参数，改变默认为1秒的时间间隔。

### 修改脚本
* 这套脚本有个地方无法满足测试需求，就是，当屏幕不刷新的时候，不显示 fps。
* 比如播放视频过程中，视频卡住不动，这时的 fps 应该算作是0。但如果脚本不报告这个 fps，我们收集不到，那么最后的统计结果就是不准的。
* 另外，我们只关注 fps，其他三个指标暂时不用收集。所以把脚本改成只打印 fps。
* 以上两点修改都很简单，只需修改 `surface_stats.py` 的 main():
* 在 `if not results:` 里，加上 `print 0`，这样当屏幕不刷新时，就会打印 0.
* 把如下用于打印表头的语句去掉：

		terminal_height = _GetTerminalHeight()
		if row_count is None or (terminal_height and
		  row_count >= terminal_height - 3):
		_PrintColumnTitles(results)
		row_count = 0

		row_count += 1

* 把 `_PrintResults(results)` 改为 `print results['avg_surface_fps (fps)']`，因为我们只关注 fps，所以每次只打印 fps

### 原理简单分析

#### 命令：dumpsys SurfaceFlinger --latency SurfaceView
* 这套脚本的根本原理是根据命令： `dumpsys SurfaceFlinger --latency SurfaceView` 的结果来计算 fps
* 该命令会打印 128 行数据，格式如下：

	    16954612
	    7657467895508   7657482691352   7657493499756
	    7657484466553   7657499645964   7657511077881
	    7657500793457   7657516600576   7657527404785

* 其中第一行是刷新间隔，这里就是 16.95 ms
* 接下来的每一行有3个时间戳，他们的意义是：

		A) when the app started to draw
		B) the vsync immediately preceding SF submitting the frame to the h/w
		C) timestamp immediately after SF submitted that frame to the h/w

* 我们将取中间那列来计算 fps

#### 主控脚本
* 主控脚本 `surface_stats.py` 里用了一个 collector 来控制整个数据收集过程：

		collector = surface_stats_collector.SurfaceStatsCollector(device)

* 主控里首先调用了 `collector.Start()`，然后就进入死循环，每隔一秒调用一次 `collector.SampleResults()`，他它的返回结果整理一下，打印到屏幕上。

##### 1. Start()
* `collector.Start()` 的主要作用是启动 collector 的收集数据线程，该线程跑的函数是：`_CollectorThread()`。这条线程在一刻不停的用如下命令抓取 timestamp (取结果里的第2列)：

		adb shell dumpsys SurfaceFlinger --latency SurfaceView

* 然后把抓到的 timestamp 按照单调递增的序列存储在 `self._data_queue` 里
* 因为是连续不停的运行 dumpsys 命令，所以难免会有重复的 timestamp，做成单调递增的意义就是去掉重复的部分。

##### 2. SampleResults()
* `collector.SampleResults()` 的作用是让 collector ：
	1. 从 `self._data_queue` 里把收集线程写入的数据拿出来，然后用拿到的数据进行计算，计算结果存储到 `self._result` 里。
	2. 返回 `self._result`
	3. 清空 `self._result`
* 其中第一步是通过函数 `_StorePerfResults()` 完成的。
* 注意，该函数只有**不**走进函数一开始那个分支：`if self._use_legacy_method` 才会按照我们所说的，以 dumpsys 的数据为基础来计算 fps。不然的话，就是另外一种方式了，这不在本文讨论范围内。

#### 计算fps
* 计算 fps 是通过函数 `_CalculateResults()` 完成的
* 具体计算过程就不分析了，自己看代码吧