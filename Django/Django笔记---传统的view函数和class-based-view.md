### 传统的 view 函数

#### view函数要跟 url 正则表达式提取出的字符串要配合使用。
* `url(r'^poll/(\d+)/$', views.detail, name='detail')`
* 这样的url，你的 detail 函数必须接受2个参数，第一个是request，第二个是 (\d+) 提取的一个都是数字的字符串。
* 定义 detail 的时候

		def detail(request) # 错了，少了一个参数
		def detail(request, q_id, q_name) # 错了，多了一个参数
		def detail(request, q_id) # 正确
		def detail(rqst, qid, qname=None) # 也正确，此函数可以接受2个参数
		def detail(request, *args) # 正确
		def detail(request, *args, **kwargs) # 正确
* 后两种写法虽然能保证无论url里怎么提取参数，都不会在调用view函数时报错，但实际上你还是要了解传过来的参数的顺序和名称才能处理好业务逻辑的。

		def detail(request, q_id):
			question = Question.objects.get(pk=q_id)
			context = {'question': question}
			return render(request, 'polls/detail.html', context)

	1. 你很清楚 url 传过来的是1个参数，所以你定义 view 的时候，除了 request 只接收1个参数
	2. 你很清楚 url 传过来的是 positionnal 参数，所以参数名字你可以随便起，你起了一个 `q_id`，你也可以起个 `question`，或者 `question_id`
	3. 你很清楚你要去查的那张表叫 Question，并且 url 传过来的参数代表着你要找的那一行的 pk，所以你用了 `pk=q_id`
	4. 你很清楚你要渲染的模板需要的 context_name 叫 question，所以你把查出来的 object 作为了 context['question'] 的 value
	5. 你很清楚你要渲染的模板的名称和路径：'polls/detail.html'

* 当完全自己写一个 view 的时候，你什么都清楚
	* 你知道怎么跟 url 配合（体现在参数名，参数个数，参数位置等）
	* 你知道怎么跟 template 配合（体现在 context 的 key 叫什么，value 是什么，template 的路径等）
* 以上只要保证你写的函数接受的参数个数能跟url匹配就可以了，还有更复杂的情况，不但个数要配合上，参数名也要跟url配合好。

		url(r'poll/(?P<question_id>\d+)/$', views.detail, name="detail")
* 如果 url 里用 name-group 提取参数，那么你函数的参数名，还要跟正则表达式的 group 名一致，因为此时传的是 keyword argument。

		def detail(request) # 错了，少了一个参数
		def detail(request, q_id, q_name) # 错了，多了一个参数
		def detail(request, q_id) # 错了，参数名不对，必须是 question_id
		def detail(request, question_id) # 正确
		def detail(request, question_id='1') # 正确
		def detail(request, *args) # 错误
		def detail(request, *args, **kwargs) # 正确

### class-based view

* 但当你复用 Django 提供给你的 class-based view 的时候，你对 view 的内部就没那么清楚了，它对 url 传进来的参数位置，参数名，参数个数有什么要求？它对 context 的 key 有什么要求？等等等等。
* 举例，把上面的 detail 改为 class-based view

		url(r'^(?P<pk>\d+)/$', views.DetailView.as_view(), name='detail')
		class DetailView(generic.DetailView):
		    model = Question
		    template_name = 'polls/detail.html'

* Django 的 DetailView 期待 url 传入 一个叫 pk 的 keyword argument （ 跟 url 的配合，必须把 url 正则表达式里的 group-name 写成 pk：(?P<pk>\d+) ）
* 它根据这个 pk 去 Question 那张表里去查出一行：即 Question.objects.get(pk=1234)，它之所以知道去 Question 表里查，是因为你写了 model = Question
* 这行作为 context 的 value
* 而 context 的 key，即 `context_name` 是表名的小写，即 'question'。（实际上是用 `obj._meta.model_name` 得到的）
* 所以，它渲染模板用的 context 是 {'question':Question.objects.get(pk=1234)}
* 它用 这个 context 去渲染 'polls/detail.html' 这个模板，然后返回 
* 它有一些限制：
	* url 提取的参必须叫 pk
	* 模板里的变量必须是表名的小写：question
* 对于 class-based view，如果你不重写 get() 等方法的话，你就不知道你传进去的 keyword argument 能不能被正确使用，或者说人家需要叫什么名的 keyword argument 你不知道。
* 我觉得这也是目前（Django 1.8）的 class-based view 的文档还不够好的地方。




### 题外话一 
* python 无重载，一个类里定义两个同名但参数个数不同的函数，后定义的会覆盖先定义的：

		>>> class P(object):
		...  def f(self,a1):
		...   print(a1)
		...  def f(self,a1,a2):
		...   print(a1,a2)
		...
		>>> p = P()
		>>> p.f(1)
		Traceback (most recent call last):
		  File "<stdin>", line 1, in <module>
		TypeError: f() takes exactly 3 arguments (2 given)
		>>> p.f(1,2)
		(1, 2)

* python 重写函数的时候可以改变函数的参数个数，重写的函数将完全覆盖父类的函数，调用完你重写的函数就返回了。除非你在重写的函数内显示的调用 super()

### 题外话二
* dir() 并不保证能把一个 object 的全部 attribute 都列出来：

		The default dir() mechanism behaves differently with different types of objects, as it attempts to produce the most relevant, rather than complete, information  
		Note：Because dir() is supplied primarily as a convenience for use at an interactive prompt, it tries to
		supply an interesting set of names more than it tries to supply a rigorously or consistently defined set of
		names, and its detailed behavior may change across releases. For example, metaclass attributes are not in
		the result list when the argument is a class.
* 所以 dir(YourClass) 返回的 list 里并没有 `__mro__` 。


