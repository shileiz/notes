###基本
* 官方文档：[https://cmake.org/cmake/help/v3.7/](https://cmake.org/cmake/help/v3.7/)
* cmake 的所有命令都写在 CMakeLists.txt 里，或者以 .cmake 为后缀的文件里。
* 这种文件内部按照 Cmake 语法书写。 Cmake 的语法规定，每一行都是一个 Cmake 命令，Cmake 命令对大小写不敏感
* 每条命令都可以有自己的参数，参数用小括号扩起来，以空白分隔。

		# add_executable 是命令，hello 和 world.c 是参数
		add_executable(hello world.c) 

* 参数分三种类型：
	* Bracket Argument： Cmake 3.0 以上才支持，这里不展开
	* Quoted Argument： 用双引号引起来的
	* Unquoted Argument：不用双引号引起来的

###Tutorial
* 地址：[https://cmake.org/cmake-tutorial/](https://cmake.org/cmake-tutorial/)
* 