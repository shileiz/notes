## HelloWorld 步骤
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

* 其中的 --sort 参数是用来指定结果显示哪些列，我们这里只写了一个 dso（即 dynamic shared object），所以结果只显示一列 “Shared Object”，而且按 dso 分类，结果也只有三行而已。
* 如果不加 --sort 参数，默认显示这几列：Command，Pid，Tid，Shared Object，Symbol，相当于：

		--sort comm,pid,tid,dso,symbol

* -n 参数用来显示 Sample 那列，表示该行共命中了多少个 Sample，加不加随意。
* 可以看到，百分之 88.93 的时间都耗费在了我们的被测程序上，这是预期中的。
* 看一下我们 app 内部，那些函数占的比重最大：

		adb shell /data/local/tmp/simpleperf report -i /sdcard/perf.data --dsos /data/local/rvdecApp --sort symbol

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

* 其中的  --dsos 参数是 simpleperf 的 5 个 filter 之一，意思是按照指定的 dynamic shared objects 进行过滤，只显示参数指定的 dso 里面的结果。全部 5 个 filter 是：
	* --comms： 按 command 过滤，比如：`--comm rvdecApp`
	* --pids: 按 pid 过来
	* --tids: 按 tid（线程id）过滤
	* --dsos： 按库文件/可执行文件名过滤
	* --symbols： 按函数名过滤，比如： `--symbols "RVComFunc::getPUMVPredictor(RefBlockInfo*, unsigned int, int, int, unsigned int)"`，注意函数里有空格的，需要用双引号引起来。
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

* 不覆盖被测程序也可，但是要明确告诉 simpleperf 去哪个可执行里读取符号表和debug信息，不然 simpleperf 默认是去 record 时的那个可执行文件里读的。这个参数是： `--symfs`，它后面跟一个目录。
* 比如我们刚才的被测程序是  `/data/local/rvdecApp`，我们把带有符号表的 rvdecApp 放到 /sdcard/data/local/rvdecApp，然后用 `--symfs /sdcard` 即可。 注意，必须把 rvdecApp 放到一个含有 `/data/local/` 的目录里，因为执行的时候是全路径，simpleperf 认为 `/data/local/rvdecApp` 整个是可执行文件。


## 实际使用

### 1. 用 simpleperf 启动被测进程
* 可以用 simpleperf 启动被测进程。而不必先把被测进程启动，然后 ps 出进程号再采集。其命令行格式如下：

		adb shell /data/local/tmp/simpleperf record -o /sdcard/a.log /data/local/rvdecApp /data/local/CampfireParty_2496x1404_30_300_5000_rmhd_slow2pass.rv -i w=2496,h=1404

* 其中 `/data/local/rvdecApp` 是被测 app 的可执行文件，后面跟的都是该 app 的启动参数
* 如此启动的 simpleperf，将在被测进程运行结束后停止采集并退出。

### 2. cache-miss
* 默认采集的是 event 是  cpu-cycles，所以我们默认得到的结果都是“CPU使用率”

		As the perf event is cpu-cycles, the overhead can be seen as the percentage of cpu time used in each function.

* 用 -e 参数可以指定要采集哪些 event，我们用 -e cache-misses 即可以采集 cache-miss

		adb shell /data/local/tmp/simpleperf record -o /sdcard/a.log -e cache-misses /data/local/rvdecApp /data/local/CampfireParty_2496x1404_30_300_5000_rmhd_slow2pass.rv -i w=2496,h=1404

## 其他
* 不使用 --duration 参数指定采集时间，当被采集进程退出时，simpleperf 不会自动停止采集，而是会报告如下的log：

		simpleperf I 04-19 17:39:06  6956  6956 cmd_record.cpp:1105] Cpu 1 is offlined
