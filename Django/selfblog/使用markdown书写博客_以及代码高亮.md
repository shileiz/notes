#### 步骤一： md 转为 html

* 把 markdown 格式转成 html 格式，用 python 的库 markdown
* `pip install markdown`  即可安装
* 修改 adminx.py，在适当的位置加入如下行：

		:::python
		obj.content_html = markdown.markdown(obj.content, extensions=['codehilite',], extension_configs = {'codehilite':{'noclasses':'True'}})

* 以上的 `extensions=['codehilite',], extension_configs...` 等等，是为了代码高亮，下面说


#### 步骤二：代码高亮

* 为了代码高亮，使用了 markdown 库的 extension： codehilite
* codehilite 是基于 pygments 的，所以得装 pygments: `pip install Pygments`
* 给 markdown 传 extension 的方法是使用 keyword arg： `extensions=['codehilite',]`。具体见：[http://pythonhosted.org/Markdown/reference.html#extensions](http://pythonhosted.org/Markdown/reference.html#extensions)
* codehilite 只负责在 md 转 html 时，把代码部分给你用 `<div class='codehilite'>` 等包裹起来，但具体这个 class 是啥样式，需要用户自己去写 css。当然 pygments 有命令行工具能生成 css 文件。
* pygments 支持 300 多种语言，如果把这300多种语言的css都弄到一个.css文件里不太现实。
* 但博客里需要高亮的代码确实有各种不同的语言，所以我使用了 pygments 的另一种方式，不用 class selector 这种css，而是 inline style。这需要给 codehilite 做一个配置：`'noclasses':'True'`
* codehilite 文档：[http://pythonhosted.org/Markdown/extensions/code_hilite.html](http://pythonhosted.org/Markdown/extensions/code_hilite.html)
* 由于博客里的代码很多都是片段，靠 pygments 去猜这段代码是什么语言，有时候可能猜不准。所以用了 codehilite 的一个接口，在博文的代码段的第一行用三个冒号后面跟语言名的方式，告诉 codehilite（pygments）这是哪种语言。这么搞给写博客时带来了点麻烦，需要在每个代码块最前面多加一行，但也只能这样了。
* pygments 支持的所有语言可以在这里查到：[http://pygments.org/docs/lexers/#lexers-for-c-c-languages](http://pygments.org/docs/lexers/#lexers-for-c-c-languages)。

#### 步骤三：跟 MarkdownPad 的样式统一起来
* 因为平时书写一般是在 windows 的 MarkdownPad 上进行，习惯了所见即所得
* 为了能跟 MarkdownPad 统一起来，从 MarkdownPad 生成的 html 文件中提取了一个 css，命名为 mdpadbase.css，让模板detail.html直接引用了。
* 这样会造成很多基本元素的 css 被覆盖，比如 h1、h2 啥的，导致侧边栏都跑到最下面去了，这种方案不行。
* 正常的做法应该是自己给 markdown 写一个 extension，配合自己从 Markdownpad 抠出来的 css，给这个抠出来的 css 加上 class，这个稍微麻烦，以后再整。