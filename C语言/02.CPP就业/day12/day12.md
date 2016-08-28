#### 成员函数在不在对象“里面”
* C++ 的对象是不持有成员函数的函数指针的，除非该函数被声明为 virtual 的。 可以自己写个 sizeof 小程序验证一下，这里就不写了。
* Java 的对象是持有成员函数的指针的
* C++ 编译器靠类型来判断该调用哪个函数，Java 靠对象内部的函数指针

#### 举例，先看代码，后面有内存分析图

#####C++

	#include <stdio.h>
	
	class Person {
	protected:
		char* name;
	public:
		Person(char *name) 
		{
			this->name = name;
		}
		void say() 
		{ 
			printf("i am a person,my name is %s\n",name); 
		}
	};
	
	class Singer : public Person {
	public:
		Singer(char *name):Person(name) 
		{}
		void say() {
			printf("i am a singer,my name is %s\n", name);
		}
	};
	
	void main()
	{
		/* 情形1，对象在堆上 */
		Person *p = new Person("XiaoMing");
		Singer *s = new Singer("XueYou");
		p = s;
		p->say(); // i am a person,my name is XueYou
	
		/* 情形2，对象在栈上，结果和对象在堆上是一样的 */
		//Person p("XiaoMing");
		//Singer s("XueYou");
		//p = s;
		//p.say(); // i am a person,my name is XueYou
	
		getchar();
	}


#####Java

	public class JavaTempTest {
		public static void main(String[] args) {
			Person p = new Person("XiaoMing");
			Singer s = new Singer("XueYou");
			p = s;
			p.say(); //i am a singer, my name is XueYou
		}
	}
	
	class Person {
		protected String name;
	
		Person(String name) {
			this.name = name;
		}
	
		public void say() {
			System.out.println("i am a person, my name is " + name);
		}
	}
	
	class Singer extends Person {
		Singer(String name) {
			super(name);
		}
	
		public void say() {
			System.out.println("i am a singer, my name is " + name);
		}
	}

#####内存分析图

![](cpp-java.jpg)