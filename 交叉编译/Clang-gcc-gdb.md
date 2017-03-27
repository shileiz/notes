### 写在前面
* Clang 和 LLVM 的关系，百度了一下，也没看太明白，不是重点，先不关注了
* 本文的重点是，用 gcc 和 clang 编译出来的东西，是不是都可以用 gdb 来调试：可以。

### gdb
* 参考：[http://www.cnblogs.com/hankers/archive/2012/12/07/2806836.html](http://www.cnblogs.com/hankers/archive/2012/12/07/2806836.html)
* `gdb executable` 就可以启动 gdb 来调试这个 executable ，执行这句后进入 gdb 命令行
	* 注：被调试的 executable 可以是加了 -g 选项编出来的，也可以是没有加的
* 进入 gdb 命令行后，用 start 命令来启动被调试程序
* 其他常用命令见参考网页，如果失效了随便百度一下也可。
* 加了 -g 和没加 -g 编出来的程序虽说都能调试，但是没加 -g 的无法显示行号等信息，看起来不方便也看不懂。

### Clang 编译的程序直接可以被 gdb 调试
* 同样加不加 -g 都可以调试，同样加了 -g 的可以看到行号等。