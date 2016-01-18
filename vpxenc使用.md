# vpxenc的使用 #
* 参考资料：
	* vpxenc.exe 的 help （vpxenc.exe 直接回车，打印的东西）
	* 官网的文档（http://www.webmproject.org/docs/encoder-parameters/）
	* 源码的doc（参考意义不大，这个主要是SDK文档，教你怎么使用各种函数的，而不是如何使用vpxenc.exe）
* vpxenc.exe的版本
	* 基于2016.1.11从官方git下载的代码，用vs2010编译出的win32版本。 
* 官方文档版本
	* 2016.1.11 官网上的文档: http://www.webmproject.org/docs/encoder-parameters/ 
	* 这个版本的文档很多描述都是只针对VP8，但对VP9应该同样具有参考意义

### 一些事项 ###
* 命令行格式是 vpxenc.exe 参数列表 输入文件
* 可以在参数里加上 `-v` 打印出 encoder 使用的参数。比如有些参数你没设置时，想看看默认值，就可以加上 -v
* 2-pass 不用像x26x那样分两遍运行。只需要指定参数 `--passes=2` 就会自动运行2遍编码。

### 官方文档摘录 ###
* `--fps=`
	* 设置目标帧率，必须以分数的形式给出，比如想设置成29.97，就写 `--fps=30000/1001` 
* `--target-bitrate=`  
	* 设置目标码率，单位是kbps 
	* 这个参数 is only a guideline to the encoder。最终码率跟你设置的这个数有多接近，还取决于其他参数。
* `--best、--good、--rt，--cpu-used=`
	* `--best、--good、--rt` 一般和 `--cpu-used=` 配合使用
	* 这些参数会影响目标码率控制的准确度
	* 文档说 `--good` 之后，`--cpu-used=` 可以设置为 0~5，数越大编码越快但视频质量和码率控制准确度越低。

### 关于码率控制 ###
* 码率控制方式有4种，用 `--end-usage=` 参数来指定，可取的4个值是：`vbr, cbr, cq, q`
* 官方文档里只说了前3种，估计写这篇文档的时候，还没有 `--end-usage=q` 这种模式
* 关于CQ模式
	* `--end-usage=cq` 指定为CQ模式，并且需要用一个额外的参数来指定cq值：`--cq-level=`
	* `--cq-level=` 的取值范围是0~63，类似于x26x的qp取值范围：0~51。这个数越小，视频质量越高，跟x26x也是一样的。
	* CQ模式是Constrained Quailty，而不是Constant Quality
	* the output stays within given quality **and size constraints across the set**
	* 在尽可能保证视频质量保持在设定的水平上的同时，还要保证文件大小（即码率）不超过设定的范围
	* 在CQ模式下，`--target-bitrate=` 表示的是 "target maximum rate"
	* CQ模式下必须同时设置 `--cq-level=` 和 `--target-bitrate=`
	* 如果不设置，`--cq-level=` 默认值是 10，`--target-bitrate=` 默认值是 256
	* 如果不设置`--target-bitrate=`，只设置`--cq-level=`，你会发现，无论怎么改变cq-level的值，得到的输出文件都一样大
	* 原因是，不设置`--target-bitrate=`，则使用了默认的256，而这个数太小了。无论你把cq设的多么小，都会由于码率不够用，而编出来质量较差的视频。
	* 所以在使用CQ模式的时候，如果想只通过cq值来控制视频质量，可以把`--target-bitrate=`设置成一个较大的数
	* CQ模式可以在1-pass下进行，但2-pass+CQ会有更好的效果
* 关于Q模式
	* 由于官方文档里没有Q模式，所以以下结论是基于自己的测试+参考源码doc
	* 从源码的doc可以看出来，Q模式是:Constant Quality (Q) mode，CQ模式是：Constrained Quality (CQ) mode
		* `<doc_path>/html/group__encoder.html#gaf50e74d91be4cae6f70dfeba5b7410d2 `
	* 即Q模式是恒定视频质量，只参考cq-level，不考虑码率。而CQ模式同时参考目标码率和cq-level两个值。
	* Q模式对应x26x的cqp模式。
	* Q模式同样可以在1-pass和2-pass下工作。而且也是2-pass比1-pass能得到更好的视频质量和更小的码率。
	* Q模式下 `--target-bitrate=` 是没有意义的。做了几组测试测试，使用Q模式，其他参数一样的情况下，设了`--target-bitrate=`和不设，出来的文件大小一样（但md5还是不同的）。
	* 另外，在Q模式下-v参数打印出来的target-bitrate虽然是参数`--target-bitrate=`指定的那个数，但实验结果还是说明了这个数是被codec忽略了的。

