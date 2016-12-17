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