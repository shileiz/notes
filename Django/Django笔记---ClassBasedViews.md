### 复习一下 url 分发
* `url(r'url正则表达式', YourViewClass.as_view(init_kwargs), {key:value}, name='url_name')`
* `r'url正则表达式'`
	* 这个不多说了，记住从这里匹配到的 group 们会作为参数传递给 view 函数。
    * 只要这个正则表达式里有一个 named-group ，则忽略其他没有命名的 group ，把所有 named-group 作为 keyword argument 传递给 view 函数。
	* 如果这个正则表达式里所有的 group 都没有命名，那么这些 group 将作为 positional argument 传递给 view 函数。
* `YourViewClass.as_view(init_kwargs)`
	* 这个东西返回的是一个函数，就是 view 函数。此函数接受 request，返回 response。
	* 这个函数还接受其他参数，比如从正则表达式里提取的 keyword args 或者 positional args。
	* 这个函数就是 YourViewClass 里定义的 get() 或者 post()，或者 head() 之类的，具体调用哪个取决于用户是用哪个方法访问的这个 url 。
	* 当然了，YourViewClass 之所以这么“智能”是因为它继承自 View， View 实现了以上说的这些功能。View 具体怎么干的，下文会说。
	* 另外这个 `init_kwargs` 是干嘛用的也会在下文说，总之别搞混了：传给 `as_view()` 的 `init_kwargs` 可不是传给 view 函数的。
* `{key:value}`
	* 这个地方的字典会作为 keyword argument 传递给 view 函数，此参数是可选的。
* `name='url_name'`
	* 给这个url起个名字，以便以后反解。

* 总结： view 函数可能接收到的参数有：
	1. 从 url 正则表达式里提取的 positional args
	2. 从 url 正则表达式里提取的 keyword args
	3. 写在 url() 第三个参数上的字典

### 这里科普一下：
* 所谓 keyword argument 别跟 default value argument 搞混了，完全两码事。
* keyword argument 和 positional argument 是在函数调用的时候的两个名词。
* default value argument 和 non default value argument 是在函数定义时的两个名词。

		def f(arg1, arg2=100):
			pass
* arg2 是有 default value 的 argument，但 arg1 和 arg2 都能成为 keyword argument

		f(arg1=100,arg2=200)  # keyword argument, arg1=100,arg2=20
		f(3)         # positional argument,arg1=3,arg2=100 
		f(4,5)       # positional argument,arg1=4,arg2=5
		f(50,arg2=80)# positional argument and keyword argument, arg1=50,arg2=80
		f(arg2=300)  # 错误！arg1 是必须的参数，不能省略。
* 有 default value 的参数才是 optional 的参数，在调用时才可以省略。

### ClassBasedViews

* 记住 view 函数是要跟 url正则表达式密切配合使用的，因为 view 函数的参数（主要）都是 url正则表达式里提取出来的。

####View.as_view(**initkwargs)

1. 先检查传进来的 keyword args 的 key 是不是本类的 attr，如果有不是的，就报异常
2. 返回一个叫 view 的 method。
	* view 接受的参数为 request（以及从 url 提取的参数），就像传统的 view 函数那样：`view(request,*args,**kwargs)`
	* view 的返回值是一个 httpresponse
	* 这就实现了传统 view 函数的功能，接受一个reques，返回一个response
3. view() 具体干了什么：
	1. 实例化本类，生成的实例叫 self。实例化的时候会把 **initkwargs 设置上。
	2. self.request = request; self.args = args; self.kwargs = kwargs
	3. 调用 `self.get(request,*args,**kwargs)` 或者 `self.post(request,*args,**kwargs)`，并且把调用结果（必须是一个httpresponse）返回。
		（注：具体调用 get() 还是 post() 还是 put() 什么的，是根据 request.method.lower() 决定的，这个判断逻辑封装在了 self.dispatch() 里）

####所有的 class-based View 都直接或间接继承自 View
####View 本身并没有实现 get() 等方法
		
####总结：
* 真正干了传统 view 函数干的事儿的，是具体的 get(), post() 等函数
* 这些函数要你自己实现，或者从一个已经实现了的类继承

### Django 提供的 ClassBasedView

* 我们平时自己写的 YourViewClass，都会从Django提供的几个Class-Based View 来继承（还会加上自己的或者Django的Mixin）
* 较常用的是从这几个类继承：TemplateView，DetailView，ListView
* 我们来看看他们的 get 方法都是怎么实现的

####TemplateView 的 get 实现

	def get(self, request, *args, **kwargs):
	    context = self.get_context_data(**kwargs)  # 把 url 里捕获的 kwargs 以及写在 url() 第三个参数上的字典作为 context
	    return self.render_to_response(context)    # 用这个 context 去渲染 template_name 指明的那个模板，如果没有设置 template_name 这个类变量，则会有异常抛出

* TemplateView 实现的 get() 很简单，只有两行
* 这两行也是最基本的一个传统 view 函数该干的事儿：根据业务逻辑生成一个 context，用这个 context 渲染模板并返回。
* 那么，这个 context 是根据什么生成的呢？ 是调用了一个 `get_context_data()`，这是来自类 ContextMixin	

####ContextMixin

* ContextMixin 的 `get_context_data(**kwargs)` 方法很简单，只是把 kwargs 这个字典(来自url())给返回了，唯一做的就是在返回这个字典之前，往里添加了一个键值对：`kwargs['view'] = self`

####TemplateResponseMixin

* `render_to_response(context)` 来自于类 TemplateResponseMixin
* 它返回的是一个 TemplateResponse 的实例，而 TemplateResponse 是 HttpResponse 的子类。
* `render_to_response(context)` 干的事儿就是用 context 去渲染 `self.template_name`，然后返回一个 TemplateResponse 的实例。
* 使用 TemplateView 时必须要设置 `template_name`，即把 `template_name = "xxx"` 传给 `as_view()` 或者在你定义的子类里写上 `template_name = "xxx"`
* TemplateView 使用比较简单，只需要设置这一个类变量即可。
* 如果 url里捕获的参数是 version=1.0 则用 {'version':'1.0'} 去渲染 `template_name` 然后返回 response



####DetailView 的 get 实现

	def get(self, request, *args, **kwargs):
	    self.object = self.get_object()
	    context = self.get_context_data(object=self.object)
	    return self.render_to_response(context)
	
* DetailView 的总体思路是这样的，这是一个显示数据库某个表中某一行的详细信息的 view 。
* 它用这样一个 context 渲染 `template_name` 这个模板，然后返回。
* 这样一个 context 是指至少要包含 {'object_name':object} 的context，即你要显示的那行的名字作为 key，从数据库里查出来的代表这行的object作为 value 的这么一对儿。
* `'object_name'` 可以用类变量 `context_object_name` 来设置，如果不设置则自动的通过查出来的那个 object 的 `object._meta.model_name`。使用` ._meta.model_name` 的前提是，知道 object 是从哪个表里查的
* 表用类变量 model 来指定。
* 那么具体是哪个表的哪一行呢？可以设置类变量 model 来指定是哪个表，kwargs里设置 `pk=xxxx` 来指定是哪一行，注意，一定要是pk。

* 它的 context 只负责 `context['object'] = self.object` 和 `context[context_object_name] = self.object`
* 所以它有个方法叫 `get_object()`，能返回本 view 要显示的是哪一行。

* 总结：
	* DetailView 需要设置 model，`template_name` 两个类变量
	* 如果不重写 get_object 则需要跟 url 配合，传入 key 为 pk 或 slug 的 keyword argument
	
####ListView 的 get 实现( 如果不考虑 allow_empty)

	def get(self, request, *args, **kwargs):
	    self.object_list = self.get_queryset()
	    context = self.get_context_data()
	    return self.render_to_response(context)	
	
* ListView 的 get() 方法继承自 BaseListView 
* BaseListView 的 get() 是这样实现的，分了三步：
	1. 成了一个实例变量 `self.object_list`

			self.object_list = self.get_queryset()
	
		* 这个变量是实例的，不是类的。只有调用到了 BaseListView 的get()才有这个变量。
		* 这个变量很有用，我们自己写的继承自 ListView 的类，经常要用到这个变量。

	2. 判断了一下 self.allow_empty() 是否为 True，不是则处理一下，抛个异常之类的，这里先不关注。
	3. 完成最基本的：
		
			context = self.get_context_data(**kwargs)
			return self.render_to_response(context)

* 个人认为其中的第一步是最重要的，虽然只有一行。
* 这就必须说说 `get_queryset()` 这个方法了。这是 MultipleObjectMixin 的方法，这个类也是 ListView 的父类 
* 这个类有个类变量 叫 queryset，如果用户设置了这个类变量（用了kwarg：queryset=xxxx），那么 `get_queryset()` 直接返回这个 queryset
* 如果用户没有设置，则会返回 `self.model._default_manager.all()`。model 也是类变量，是可以被用户设置的，默认是 None。

	    def get_queryset(self):
	        """
	        Get the list of items for this view. This must be an iterable, and may
	        be a queryset (in which qs-specific behavior will be enabled).
	        """
	        if self.queryset is not None:
	            queryset = self.queryset
	            if hasattr(queryset, '_clone'):
	                queryset = queryset._clone()
	        elif self.model is not None:
	            queryset = self.model._default_manager.all()
	        else:
	            raise ImproperlyConfigured("'%s' must define 'queryset' or 'model'"
	                                       % self.__class__.__name__)
	        return queryset

* 如果用户既没设置 queryset 也没设置 model，则会抛异常
* 当然这里的用户就是你，一个写 View 的人。
* 那么如果你继承了 ListView，你有没有设置 queryset 或者 model 这两个类变量，那么你就必须重写 `get_queryset()` 这个方法，并且让这个方法能返回一个 `object_list`，设置给你 self
* 说白了，对于一个继承了 MultipleObjectMixin 的类来说，它肯定是一个要处理多个 object 的类，所以它有一个 `get_queryset()` 方法来拿到装着这些 object 的 iterable。
* 这个 iterable 当然可以是一个 queryset。所谓 queryset 就是一个表里若干行。一个表的 all(),filter(),exclude()，返回的都是 queryset。
* 关于 queryset 可以看“Django笔记---Mode”。
* 话说回来，，，，，

### Mixin

