## 步骤
* 把 simpleperf 可执行程序 push 到手机上。 simpleperf 在 NDK-r13b 的 `simpleperf/android/` 里。根据被测程序和手机的CPU选择对应版本：

		adb push simpleperf /data/local/tmp/
		adb shell chmod 777 /data/local/tmp/simpleperf

* 启动被测程序，ps 出它的进程 ID
* 用 simpleperf 进行 profile：

		adb shell /data/local/tmp/simpleperf record -p 3972 --duration 30

* 得到错误提示，说只读分区无法写入 perf.data:

		simpleperf E 04-19 15:09:29  4109  4109 record_file_writer.cpp:47] failed to open record file 'perf.data': Read-only file system

* 用 -o 参数指定输出文件解决问题：

		adb shell /data/local/tmp/simpleperf record -p 3972 --duration 30 -o /sdcard/perf.data

* 得到警告如下：

		simpleperf W 04-19 15:17:47  4299  4299 environment.cpp:504] Access to kernel symbol addresses is restricted.
		If possible, please do `echo 0 >/proc/sys/kernel/kptr_restrict` to fix this.

* 暂且不去理会这个警告，30 秒过后采集结束，得到如下输出：

		simpleperf I 04-19 15:18:19  4299  4299 cmd_record.cpp:346] Samples recorded: 125706. Samples lost: 0.

* 看起来一切正常，把结果 report 一下看看：

		adb shell /data/local/tmp/simpleperf report -i /sdcard/perf.data -n --sort dso

* 得到结果如下：

		simpleperf W 04-19 15:31:17  4564  4564 dso.cpp:274] /data/local/rvdecApp doesn't contain symbol table
		simpleperf W 04-19 15:31:17  4564  4564 dso.cpp:335] Symbol addresses in /proc/kallsyms are all zero. `echo 0 >/proc/sys/kernel/kptr_restrict` if possible.
		Cmdline: /data/local/tmp/simpleperf record -p 4281 --duration 30 -o /sdcard/perf.data
		Arch: arm64
		Event: cpu-cycles (type 0, config 0)
		Samples: 125526
		Event count: 43633757353
		
		Overhead  Sample  Shared Object
		88.93%    106529  /data/local/rvdecApp
		8.05%     10560   /system/lib/libc.so
		3.01%     8437    [kernel.kallsyms]

* 可以看到，百分之 88.93 的时间都耗费在了我们的被测程序上，这是预期中的。
* 看一下我们 app 内部，那些函数占的比重最大：

		adb shell /data/local/tmp/simpleperf report -i /sdcard/perf.data -n --dsos /data/local/rvdecApp --sort symbol

* 结果如下：

		impleperf W 04-19 15:57:34  5046  5046 dso.cpp:274] /data/local/rvdecApp doesn't contain symbol table
		simpleperf W 04-19 15:57:34  5046  5046 dso.cpp:335] Symbol addresses in /proc/kallsyms are all zero. `echo 0 >/proc/sys/kernel/kptr_restrict` if possible.
		Cmdline: /data/local/tmp/simpleperf record -p 4281 --duration 30 -o /sdcard/perf.data
		Arch: arm64
		Event: cpu-cycles (type 0, config 0)
		Samples: 106529
		Event count: 38804869540
		
		Overhead  Sample  Symbol
		5.06%     5373    rvdecApp[+24380]
		4.57%     4890    rvdecApp[+24420]
		1.43%     1588    rvdecApp[+13a44]
		1.01%     1083    rvdecApp[+21f94]
		0.94%     999     rvdecApp[+20188] 
		...

* 可以看到，结果里没有函数名字。那是因为我们的 rvdecApp 是没有符号表的版本。我们用带符号表的 app 进行分析即可。
* 带符号表的 app 可执行文件可以在 obj 目录下找到。把它 push 到手机上，覆盖原来的可执行文件。
* 注意，不用重新执行 rvdecApp 并重新采集 perf.data, 只需要在分析时使用带有符号表的 rvdecApp 即可。
* 还用刚才的命令：

		adb shell /data/local/tmp/simpleperf report -i /sdcard/perf.data -n --dsos /data/local/rvdecApp --sort symbol

* 得到如下结果：

		Overhead  Sample  Symbol
		10.45%    10993   NEON_ChromaInterpolate4_H00V00_8
		5.94%     6238    NEON_LumaInterpolate4_H03V03
		5.10%     5354    RVComFunc::DBFShiftedProcess8x8(unsigned char**, int*, unsigned char*, int, unsigned char*, int, bool, bool, bool, bool, unsigned char)
		3.55%     3722    RVComFunc::deblockCUTree(TCBDataTree*, unsigned char**, unsigned int*, int, int, RefBlockInfo*, int, unsigned char**, int*, unsigned char)
		3.49%     3675    RVComFunc::reconstructInterPredictors(TCUData*, unsigned char**, unsigned int*, TRefPicList*, RefBlockInfo*, unsigned int, unsigned int, unsigned int, unsigned int)
		3.35%     3518    RVComFunc::deriveDBFStrengthFUbyMotionInfo(unsigned char*, unsigned char*, int, int, RefBlockInfo*, int, int, unsigned char, unsigned char, bool, bool)
		2.98%     3320    Decoder::parseBitStream_FrameNew()
		2.79%     2927    NEON_DBF_EdgeFilter4_Vertical
		2.52%     2651    RVComFunc::DBFShiftedProcessFu(unsigned char**, int*, unsigned char*, int, unsigned char*, int, int, bool, bool, bool, bool, unsigned char)
		2.36%     2553    (anonymous namespace)::decode_gen_vlc(unsigned long const*, int, (anonymous namespace)::VLC*, int, int)
		...

## 其他
* 不使用 --duration 参数指定采集时间，当被采集进程退出时，simpleperf 不会自动停止采集，而是会报告如下的log：

		simpleperf I 04-19 17:39:06  6956  6956 cmd_record.cpp:1105] Cpu 1 is offlined
