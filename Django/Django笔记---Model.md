## Making queries

[file:///C:/Users/Administrator/Desktop/django-docs-1.6-en/topics/db/queries.html ](file:///C:/Users/Administrator/Desktop/django-docs-1.6-en/topics/db/queries.html ) 

### objects：表的 Manager
* 每个表都有一个Manager，叫做objects
* 通过表的Manager可以得到QuerySet，所谓QuerySet就是表里面的若干行。
* Blog.objects 就是 Blog 表的 Manager； 
* Blog.objects.filter(name__contains='jia') 通过 Blog 表的 Manager 得到了此表的一个 QuerySet
* Manager 是表级别的，不能通过行来访问：

		>>> Blog.objects
		<django.db.models.manager.Manager object at ...>
		>>> b = Blog(name='Foo', tagline='Bar')
		>>> b.objects
		Traceback:
	    ...
		AttributeError: "Manager isn't accessible via Blog instances."

### queryset：行的集合
* QuerySet 是行的集合
* all(),filter(),exclude()，返回的都是 QuerySet
* get() 返回的不是 QuerySet，是单独的一行
* QuerySet 可以继续被查询：

		Blog.objects.filter(name__contains='jia').exclude(tagline__contains='music')

* 返回的结果还是一个 QuerySet
* 相当于：

		q1 = Blog.objects.filter(name__contains='jia')
		q2 = q1.exclude(tagline__contains='music')

* 注意，q1是q1，q2是q2，第二次查询仅仅生成了q2，q1还是q1。我们可以在以后的任何时候再次引用q1。

### Field lookups
* filter(),exclude(),get() 的 keyword arguments：Field lookups
* 格式：`field__lookuptype=value`

#### 常用的 lookuptype：
* exact：可以省略不写

		>>> Blog.objects.get(id__exact=14)  # Explicit form
		>>> Blog.objects.get(id=14)         # __exact is implied

* iexact：大小写无关的exact

		>>> Blog.objects.get(name__iexact="beatles blog")
		Would match a Blog titled “Beatles Blog”, “beatles blog”, or even “BeAtlES blOG”.

* contains / icontains : 包含/大小写无关的包含
* startswith, endswith / istartswith, iendswith  以xx开始，以xx结束 / 大小写无关的以xx开始，大小写无关的以xx结束
* in 在list里

		Entry.objects.filter(id__in=[1, 3, 4])

* gt 大于， gte 大于等于，lt 小于，lte 小于等于


#### 多对一和多对多关系中的特殊情况：
1. Blog 表中没有 entry 这个 field，还是可以直接使用`entry__`做过滤

		b1 = Blog.objects.get(pk=1); 
	
	* b1.entry 是不存在的（b1.name存在）——尽管Blog有一个属性叫 `entry_set`（因为Entry的外键指向了它），但他确实没有一个叫 entry的属性。
	* 不过 查询Blog表的时候，还是可以直接使用`entry__`做过滤（而不是`entry_set__`）：
	
			Blog.objects.filter(entry__headline__contains='Lennon')
2. 有这么一个问题：

		Blog.objects.filter(entry__headline__contains='good',entry__rating=1)
		和
		Blog.objects.filter(entry__headline__contains='good').filter(entry__rating=1)
	* 的区别是：
	* 如果有某个人的blog它里面有一些文章的题目包含"good"，它另外一些文章的rating=1，那么它会被第二种方式查出来，而不会被第一种。
	* 第一种查的是，这个blog里至少有一篇文章同时满足了题目包含"good"和rating=1
	* 举个例子说比较明朗：
	* 按照第二种查法，首先在 Blog 表里找到写过 带有 "good" 题目的 blogs，结果找到了 韩寒的博客，老徐的博客 还有 老罗的博客。再接着在这三个人的博客里找 rating = 1 的，结果找到了老罗的博客和韩寒的博客。不过老罗的博客里rating=1那篇文章的题目里并不含有"good"
	* 出问题的关键就在于 entry__ 实际上是 entry__set，是一系列的文章而不是一篇。
	* 记住这个查找查的是Blog，返回的是一个Blog的集合而不是一个 Entry的集合。



### 截取 QuerySets
* 用python的array-slicing语法来截取

		>>> Entry.objects.all()[:5]
		>>> Entry.objects.all()[5:10]
		>>> Entry.objects.all()[0]

* 注意，截取 QuerySet 不支持 负数索引：

		>>> Entry.objects.all()[-1] 
		AssertionError: Negative indexing is not supported.

### F objects ( django.db.models.F )
* 行内的比较用 F()
* 一个 F() object 代表着某一列的值
* 在 filter(),exlude(),get()中使用F(x)表达式，意思是跟本行的x那一列进行比较

		>>> from django.db.models import F
		>>> Entry.objects.filter(n_comments__gt=F('n_pingbacks'))

* 以上是过滤出那些评论数大于pingback数的文章

### Q objects ( django.db.models.Q )
* `filter(name__contains='jia',tagline__contains='music')` 这两个查询条件是 and 的关系
* 如果我们想使用其他复杂的查询，比如说 or/~ 之类的，就需要用到 Q objects
* Q object 用来包裹 lookup 
* 例如 Q(name__contains='jia') 就生成了一个 Q object
* 多个 Q objet 可以用 &、| 、~ 连接组合，组合后会返回一个新的 Q object，这就能组合成任意的复杂查询条件
* 例如 `Q(name__contains='jia') | ~Q(tagline__contains='music')`
* filter(), exclude(), get() 都可以接收一个或多个 Q object 作为参数，多个 Q object 之间是 and 的关系
* 例如：

		Poll.objects.get(
		    Q(question__startswith='Who'),
		    Q(pub_date=date(2005, 5, 2)) | Q(pub_date=date(2005, 5, 6))
		)
* 相当于：

		SELECT * from polls WHERE question LIKE 'Who%'
		    AND (pub_date = '2005-05-02' OR pub_date = '2005-05-06')

* 注意，Q object 对于这些函数来说是 positional argument，因为传递给他们的时候没用 xxxx=Q() 这种格式	
* 而 `name__contains='jia'` 这种是 keyword argument。所以，如果在一个函数里混用这两种的话，需要把 positionnal 的（也就是 Q）放在前面，把 keyword 的放在后面


### 其他杂项
* 当找不到的时候, get() 和 filter()[0] 的表现是不一样的：
* get() 会抛出一个 DoesNotExist 异常
* 而 filter()[0] 只会报 IndexError: list index out of range
* 单个行和 QuerySet 都有 delete() 方法，调用后行将被删除，没有返回值，不用save()
* 比较两行是否相等，直接用==即可，当这两行的 primary key 相等的时候，Django认为这两行相等
* 复制行：

		old_instance.pk=None
		old_instance.save()    # 这时 old_instance 的pk会被自动设为表的最后一行相当于复制了一份 old_instance




看到了：


=================================


Models
A model is the single, definitive source of information about your data. It contains the essential fields and behaviors of the data you’re storing. Generally, each model maps to a single database table.

The basics:

	Each model is a Python class that subclasses django.db.models.Model.
	Each attribute of the model represents a database field.
	With all of this, Django gives you an automatically-generated database-access API; see Making queries.

	
file:///C:/Users/Administrator/Desktop/django-docs-1.6-en/topics/db/models.html#fields


===================================

明天计划：auth
auth 是 Django 自带的一个 app，用于鉴权。
auth 的核心类是 User。User 也是 django.db.models.Model 的子类，也是一个 Model 。
具体继承关系为：User <--- AbstractUser <--- AbstractBaseUser <--- Model ，这一点不用在意。

file:///C:/Users/Administrator/Desktop/django-docs-1.6-en/topics/auth/default.html#topic-authorization
