## 常用：adb logcat -s -v threadtime MyTag
* adb logcat 命令行格式： `logcat [options] [filterspecs]`  
* 注意 options 一定要出现在 filterspecs 之前。
	* `log cat MyTag -s` 因为 option -s 出现在了 filterspec MyTag 之后，所以它就不好使了。
* -s：让默认的 filter 不生效
	* 默认的 filter 是 *，意思是所有的 Tag
	* 即便你加了自己的 filter，其他乱七八糟的 log 还是会打印，因为你没用 -s 让默认 filter 失效 
* -v：控制输出格式，详见下节。 
* filterspecs： filterspecs are a series of `<tag>[:priority]`
	* 比如 MyTag    //过滤 tag 是 MyTag 的log 
	* 比如 MyTag:v  //过滤 tag 是 MyTag，Verbose 级别的 log 
	* 比如 MyTag:E OtherTag  //过滤 tag 是 MyTag 级别是 Error，和 tag 是 OtherTag 的 log
## logcat 输出格式
* 默认形如：

		D/TestRunner( 1588): OnCompletion done.
* 各段的意思是：

		 priority/tag（PID）: message
* 默认格式叫 brief，可以通过 logcat -v 选项来改变格式。
* 可用的格式有：　brief， process， tag， thread， raw， time， threadtime， long
 	* threadtime（常用）： `日期（月-日） 时间（时:分：秒） PID TID priority tag： message` 例如，`04-19 08:50:30.067  1588  1604 D TestRunner: setUp()`
	* process（不常用）： `D/TestRunner( 1588): ` 这段将只显示 `D/( 1588)`
	* tag(不常用)：`D/TestRunner( 1588): ` 这段将只显示 `D/TestRunner：`
	* thread（不常用）： 除了 PID ，同时显示 TID，但不显示 tag。`priority( PID: TID) message`
	* 其他的也没用过，不写了
##`adb logcat --help` 的输出：

	Usage: logcat [options] [filterspecs]
	options include:
	  -s              Set default filter to silent.
	                  Like specifying filterspec '*:s'
	  -f <filename>   Log to file. Default to stdout
	  -r [<kbytes>]   Rotate log every kbytes. (16 if unspecified). Requires -f
	  -n <count>      Sets max number of rotated logs to <count>, default 4
	  -v <format>     Sets the log print format, where <format> is one of:
	
	                  brief process tag thread raw time threadtime long
	
	  -c              clear (flush) the entire log and exit
	  -d              dump the log and then exit (don't block)
	  -t <count>      print only the most recent <count> lines (implies -d)
	  -t '<time>'     print most recent lines since specified time (implies -d)
	  -T <count>      print only the most recent <count> lines (does not imply -d)
	  -T '<time>'     print most recent lines since specified time (not imply -d)
	                  count is pure numerical, time is 'MM-DD hh:mm:ss.mmm'
	  -g              get the size of the log's ring buffer and exit
	  -b <buffer>     Request alternate ring buffer, 'main', 'system', 'radio',
	                  'events', 'crash' or 'all'. Multiple -b parameters are
	                  allowed and results are interleaved. The default is
	                  -b main -b system -b crash.
	  -B              output the log in binary.
	  -S              output statistics.
	  -G <size>       set size of log ring buffer, may suffix with K or M.
	  -p              print prune white and ~black list. Service is specified as
	                  UID, UID/PID or /PID. Weighed for quicker pruning if prefix
	                  with ~, otherwise weighed for longevity if unadorned. All
	                  other pruning activity is oldest first. Special case ~!
	                  represents an automatic quicker pruning for the noisiest
	                  UID as determined by the current statistics.
	  -P '<list> ...' set prune white and ~black list, using same format as
	                  printed above. Must be quoted.
	
	filterspecs are a series of
	  <tag>[:priority]
	
	where <tag> is a log component tag (or * for all) and priority is:
	  V    Verbose
	  D    Debug
	  I    Info
	  W    Warn
	  E    Error
	  F    Fatal
	  S    Silent (supress all output)
	
	'*' means '*:d' and <tag> by itself means <tag>:v
	
	If not specified on the commandline, filterspec is set from ANDROID_LOG_TAGS.
	If no filterspec is found, filter defaults to '*:I'
	
	If not specified with -v, format is set from ANDROID_PRINTF_LOG
	or defaults to "brief"