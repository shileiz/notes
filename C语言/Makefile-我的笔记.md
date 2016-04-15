## makefile基本语法：
	目标:依赖条件
		命令

## 阶段一
* all是目标，为了达到目标all，需要 add.c sub.c dive.c mul.c main.c 这些条件
* gcc是命令，命令执行完目标all就达到了

		all:add.c sub.c dive.c mul.c main.c
			gcc add.c sub.c dive.c mul.c main.c -o app
* 注意，要保证执行make的目录必须存在上述所有.c文件，不然会报错说条件add.c无法满足
	
## 阶段二
* 完成目标app需要add.o等，而add.o作为目标，要完成它需要add.c
* gcc -c add.c 将生成 add.o
* makefile的工作过程：先分析目标和条件的关系，根据这些关系自顶向下建立关系树；再沿着树自底向上执行命令
 
		app:add.o sub.o dive.o mul.o main.o
			gcc add.o sub.o dive.o mul.o main.o -o app
		add.o:add.c
			gcc -c add.c
		sub.o:sub.c
			gcc -c sub.c
		dive.o:dive.c
			gcc -c dive.c
		mul.o:mul.c
			gcc -c mul.c
		main.o:main.c
			gcc -c main.c
	
运行make时，make会先对比目标和依赖的修改时间，如果依赖比目标新，则说明需要重新生成目标，否则说明不需要重新生成目标。
make只会执行那些需要更新的目标的命令
如果目标不依赖任何条件，则会执行命令

make 命令的语法：
make 目标
比如上例： make app
一个makefile可以有多棵关系树，比如上例最后加入
clean:
	rm *.o
	rm app
该makefile就有两棵关系树，执行的时候需要用 make clean 来执行 clean
不加参数只会执行第一棵关系树，上例中即app
或者可以 make add.o 来只执行 add.o 这个目标
当执行 make xxx 时，如果当前目录中恰巧有一个文件叫xxx，而xxx这个目标又是没有依赖的，那么make会提示 make: `xxx' is up to date.
解决方案是，在makefile里加一行：
.PHONY:xxx
注意 .PHONY 是关键字，换成别的是不好使的。

命令前面加上 - 表示该命令即使执行失败了，make也会继续运行
命令前面加上 @ 表示不显示命令本身，只显示命令的结果
这俩符号可以连着用，例如
just:
    @-rm nobody.txt
    @echo "just do it!"

#阶段三
makefile里可以定义变量(变量名=值)，然后用$(变量名)来引用变量，比如
src=add.c sub.c dive.c mul.c main.c
app:$(src)
	gcc $(src) -o app
特殊的变量：$@表示目标，$^表示所有依赖，$<表示依赖中的第一个
例如：
src=add.c sub.c dive.c mul.c main.c
app:$(src)
	gcc $^ -o $@
注意：上例中不能用$<来引用依赖，因为“依赖中的第一个”表示的是把变量展开后的第一个，所以$<只能引用到add.c
makefile有内建的一系列变量和规则，用命令 make -p 可以看到，一共有一千多行。
比如：
CC = cc
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
OUTPUT_OPTION = -o $@
%.o: %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<
注意：默认没有定义 CFLAGS，没定义也可以引用，引用的结果就是空，比如：
test:
	echo $(notDefine)
以上是可以的，make不会报错，会echo一个空
为了通用性，我们一般在自己的makefile里重新实现需要用到的内建规则
比如：
%.o:%.c
	gcc -c $< -o $@
makefile有自己的函数，比如 wildcard，patsubst
了解了这些，我们阶段三的Makefile是如下这样的：
src = $(wildcard *.c)
obj = $(patsubst %.c,%.o,$(src))
target = app
$(target):$(obj)
	gcc $^ -o $@
%.o:%.c
	gcc -c $< -o $@
.PHONY:clean
clean:
	-rm -f *.o
	-rm -f app
	
#阶段四
CPPFLAGS= -Iinclude
CFLAGS= -g -Wall
LDFLAGS= 
CC=gcc
#CC=arm-linux-gcc

src = $(wildcard *.c)
obj = $(patsubst %.c,%.o,$(src))
target = app

$(target):$(obj)
	$(CC) $^ $(LDFLAGS) -o $@

%.o:%.c
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) -o $@

.PHONY:clean

#彻底清除生生过程文件
clean:
	-rm -f *.o
	-rm -f app

#彻底清除生生过程文件和生成配置文件
distclean:
	rm /usr/bin/app
install:
	cp app  /usr/bin	










