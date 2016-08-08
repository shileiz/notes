### The Django template language Overview

####Django template language 的4个基本组件
1. Variables ：{{ first_name }}
2. Tags : {% csrf_token %}， {% if %}{% endif %} 等等
3. Filters ： {{ django|title }}，{{ my_date|date:"Y-m-d" }}
4. Comments ： {# this won't be rendered #}，多行注释用 {% comment %} tag 

#####Filters
* Use a pipe (|) to apply a filter.
* {{ name|lower }} 会把 name 变成小写之后再显示。lower 是 django 的 built-in filter, django 一共有 60 多个 built-in filter
* 再举几个例子：
* {{ value|length }} If value is ['a', 'b', 'c', 'd'], the output will be 4.
* {{ value|filesizeformat }} If value is 123456789, the output would be 117.7 MB.
* filter 可以带参数：
* {{ value|default:"nothing" }} If value isn’t provided or is empty, the above will display “nothing”.

#####Custom template tags and filters（ [https://docs.djangoproject.com/en/1.8/howto/custom-template-tags/](http://10.10.49.50:8080/view/GrailPlayer/job/GrailPlayerOEM_MM_android_N/24/?) ）    
* 如果 built-in 的 tags/filters 不能满足你的要求，可以自定义
* 自定义的 filters are just Python functions that take one or two arguments
* 第一个参数是 被 filter 的变量，第二个参数是这个 filter 的参数
* {{ var|foo:"bar" }} 传给 filter foo 的第一个参数将是 var，第二个参数将是 “bar”

		def foo(value, param):
			# do something 

* 自定义的 filter 需要注册
* 一般用装饰器装饰你的函数，即可在定义时完成注册

		from django import template
		register = template.Library()
		@register.filter(name='foo')
		def foo(value, param):
			# do something 

* 上例也可以不写 name='foo'，不写参数时，将自动把你的函数名注册为 filter 名
* 你写的函数必须放在 `your_app/templatetags` 目录里，记得在这个目录里放上 `__init__.py`
* 比如上述 foo 函数写在了 `your_app/templatetags/my_filter.py` 里
* 则在 template 里要使用 `{% load my_filter %}` 之后，采用使用 foo 这个 filter

