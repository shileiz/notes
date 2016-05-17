### 总揽
* init 是一个进程。是Linux用户空间的第一个进程。
	* 它负责创建系统中的几个关键进程，包括 zygote。
	* 它负责提供Android的属性服务。
* 我们将要学习：
	* init 是如何创建 zygote 的
	* init 的属性服务是如何工作的

### 第一遍阅读
* init 会解析两个配置文件："/init.rc" 和 "/init.硬件名.rc"
* 解析配置文件后，会得到一系列的 Action，Action 是分阶段的。分4个阶段：early-init，init，early-boot，boot。init 会执行这些 Action。
* init 会调用 `property_init` 和 `property_start_service` 启动属性服务
* init 最后进入一个死循环，等待跟soket、属性服务等进行交互。
* 解析配置文件时，首先找到配置文件的一个 section，不同的 section 调用不同的函数进行解析。
* 分析keywords.h的部分没有细看。觉得以下几点比较关键：
	* 关键字有三种：COMMAND、OPTION、SECTION。其中 COMMAND 有对应的处理函数。
	* 配置文件的 on 或者 service 关键字表示一个 SECTION。
	* 配置文件的 class、oneshot、onrestart、socket 等等很多关键字表示 OPTION。
	* 配置文件的 export、class_start、class_stop、restart、start、stop 等待很多关键字表示 COMMAND。
* 解析配置文件时，一旦遇到 on 或者 service，就表示一个 SECTION 开始了，直到遇到下一个 SECTION。
* 每个 SECTION 里有一些 COMMAND 和 OPTION。
* init.rc 里有 on boot 和 on init 这两个 SECTION （还有很多别的 SECTION）
* 这两个 SECTION 就对应 Action 执行的 init 和 boot 两个阶段。即： boot 阶段执行的所有 Action，都在 on boot 这个 section 里定义。init 阶段执行的所有 Action 都在 on init 这个 section 里定义。
* on section 有两种形式：
 * `on boot` 这种，表示一个SECTION，这个 SECTION 的名字是 boot
 * `on property:sys.boot_from_charger_mode=1` 这种，表示当 on 后面的条件满足时，才执行这个 SECITON 里的东西
* init.rc 里有 service zygote /system/bin/app...... 这么一个 section，这部分完整的配置代码见第42页。
* 这部分被解析之后，形成了如图3-1（第47页）所示的一个service节点，该节点位于`service_list`上。
* 之所以能形成这种节点，是因为 service 这个结构体。
* service 结构体的代码没有详细看，其中比较重要的是：
	* const char * classname; // service 所属的 class 的名字，默认是 "default"
	* 即 图3-1中，classname 应该是 "default"
* init 中如下两个函数的意义：
	* `action_for_each_trigger("boot", action_add_queue_tail);`: 把 boot section 的 command 加入到执行队列
	* `drain_action_queue();`： 执行队列里的 COMMAND
* 捋一下流程：
	* 当init执行到 `drain_action_queue();` 时，会把当前队列里的 COMMAND都执行了。
	* 而当前队列里有哪些 COMMAND 取决于上次的 `action_for_each_trigger("boot", action_add_queue_tail);` 把那个阶段（SECTION）的 COMMAND 加进来了。
	* 当执行到 `class_start default` 这个 COMMAND 的时候，就会调用它的处理函数：`do_class_start`，并把参数 default 传进去。
	* `do_class_start` 去`service_list`上找 classname 跟参数一样service，然后又通过两层函数调用，最终启动这个service
	* zygote 就是这么被搞出来的
	* 详细的流程没有细看
* 本章关于重启zygote的部分略过了没看 

### 属性服务
* 属性服务类似于 Windows 的注册表，是 Android 提供给应用程序（和系统本身）存储 key/value 对的东西
* 这一节主要讲了属性服务的服务端是怎么实现的：包括共享内存、启动过程等，没有细看
* 然后说了一下客户端通过 `property_set` 函数发送请求可以设置属性，也没具体举例子。