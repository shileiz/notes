## makefile 的函数

####vpath
* 它的使用方法有三种：
	1. `vpath <pattern> <directories>` 为符合模式 `<pattern>` 的文件指定搜索目录 `<directories>`。
	2. `vpath <pattern>` 清除符合模式 `<pattern>` 的文件的搜索目录。
	3. `vpath` 清除所有已被设置好了的文件搜索目录。

## 变量高级用法

####结尾替换
* 替换变量的结尾部分，其格式是
	
		$(var:a=b)
		${var:a=b}

* 意思是，把变量“ var”中所有以“ a”字串“结尾”的“ a”替换成“ b”字串。这里的“结尾”意思是“空格”或是“结束符”。
* 举例：

		foo := a.o b.o c.o
		bar := $(foo:.o=.c)
		# $(bar) 的值是 a.c b.c c.c 

####模式替换
* 替换掉变量中复合模式的部分，注意 makefile 的 pattern 是含有 % 的字符串，而非传统意义上的正则表达式
* 举例：

		foo := a.o b.o c.o
		bar := $(foo:%.o=%.c)
		# $(bar) 的值是 a.c b.c c.c 

## 其他地自动变量

#### `$*`
* 这个变量表示目标模式中“ %”及其之前的部分。如果目标是 `dir/a.foo.b`，并且目标的模式是 `a.%.b`，那么， `$*` 的值就是 `dir/a.foo`。

## 目标变量

* 形如：

		target: var = xx
		target: var := xx
		target: var += xx

* 举例：

		ffmpeg_g: LDFLAGS += $(LDFLAGS-ffmpeg)

* 当我们设置了这样一个变量，这个变量会作用到由这个目标所引发的所有的规则中去
* 它可以和“全局变量”同名，因为它的作用范围只在这条规则以及连带规则中，所以其值也只在作用范围内有效。而不会影响规则链以外的全局变量的值。

## override commands for target

### 同名 target 的处理
* 如果一个 Makefile 里有两个同名的 target，则后写的 target 的命令，将覆盖先前的。
* 但是，先前的 target 的依赖依然会被检查，如果依赖无法满足，则 make 也会报错退出。
* 看如下例子：

		mytarget: a.o
			@echo first rule applied !
		
		mytarget: b.o
			@echo sencond rule applied !

* 如果 a.o 不存在，则 make 报错退出：

		make: *** No rule to make target `a.o', needed by `mytarget'.  Stop.

* 如果 b.o 不存在，也会报错退出。
* 当 a.o 和 b.o 都存在时，只会执行第二条命令，第一条命令被 override，make 会给出相应的 warning：

		Makefile:4: warning: overriding commands for target `mytarget'
		Makefile:2: warning: ignoring old commands for target `mytarget'
		sencond rule applied !

* 注意，如果第二个 target 是用 pattern 写的，则不算做 override，看例子：

		mytarget: a.o
			@echo first rule applied !
		
		%: b.o
			@echo sencond rule applied !

* 如果 a.o 不存在，则报错退出。
* 如果 b.o 不存在，则正常执行第一条命令，打印 “first rule applied !”，make 正常结束。

## 没有命令的 target
* 没有命令的 target，也会被检查依赖

		mytarget: a.o
		
		%: b.o
			@echo sencond rule applied !

* 如果 a.o 不存在，则 make 报错退出。
* 如果 a.o 存在，但 b.o 不存在，则 make 会正常结束，同时报告 “Nothing to be done for `mytarget'.”
* 如果 a.o 和 b.o 都存在，则 make 会执行第二条命令，并正常结束。