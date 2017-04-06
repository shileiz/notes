* Java 的继承不分 public protected 之类的
* C++ 继承时 public protected private 的区别在于，父类成员到子类里变成了什么权限。
	* public 继承复制父类的权限：public 的还是 public，protected 还是 protected，private 子类不可见。
	* protected 继承把父类的public和protected成员在子类里都变成 protected，private 子类不可见。
	* private 继承把父类的public和protected成员在子类里都变成 private，private 子类不可见。
* C++ 无论哪种继承，父类的 private 成员子类都不可见。