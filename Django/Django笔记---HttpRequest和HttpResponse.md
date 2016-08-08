### 类 HttpRequest
* Attributes：HttpRequest 的 attributes 除了特殊说明都是只读的，常见的非只读如 session。

		HttpRequest.scheme ： https or http or ...
		HttpRequest.method ： GET or POST or ...
		HttpRequest.body ： 请求消息体，是一个 byte string，等价于 HttpRequest.read().
		HttpRequest.GET : A dictionary-like object, 是 django.http.QueryDict 的一个实例。用于存放 GET 的参数。
		HttpRequest.REQUEST ：Deprecated since version 1.7，用 GET 和 POST 代替。
		HttpRequest.COOKIES：标准 python 字典，用来存放 cookies，key 和 value 都是 string
		HttpRequest.META：请求头，是个标准python字典，不详细讲了。
		HttpRequest.user: django.contrib.auth.User 的一个 object（也可以用第三方的Auth中间件，在setting里用 AUTH_USER_MODEL 可配）

* 比较常见的用法是 

		if request.user.is_authenticated():
			# Do something for logged-in users.
		else:
			# Do something for anonymous users.
		