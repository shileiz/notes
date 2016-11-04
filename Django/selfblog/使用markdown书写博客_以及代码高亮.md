#### 步骤一： md 转为 html

* 把 markdown 格式转成 html 格式，用 python 的库 markdown
* `pip install markdown`  即可安装
* 修改 adminx.py，在适当的位置加入如下行：

		:::python
		obj.content_html = markdown.markdown(obj.content, extensions=['codehilite', ],
                                             extension_configs={'codehilite':
                                                                {'noclasses': 'True', 'pygments_style': 'emacs'}})

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
* codehilite 还有一个参数 `pygments_style` 用来设置 pygments 的style。pygments 的 builtin sytle 有：

		:::python
		['default', 'emacs', 'friendly', 'colorful']

#### 步骤三：跟 MarkdownPad 的样式统一起来
* 因为平时书写一般是在 windows 的 MarkdownPad2 上进行，习惯了所见即所得
* 为了能跟 MarkdownPad2 统一起来，从 MarkdownPad 生成的 html 文件中提取了一个 css，命名为 mdpadbase.css，让模板detail.html直接引用了。
* 从 MarkdownPad2 里提取出来的 css，实际上来自这里： [https://github.com/nicolashery/markdownpad-github/blob/master/markdownpad-github.css](https://github.com/nicolashery/markdownpad-github/blob/master/markdownpad-github.css)
* 这个 css 要稍作修改，把 html，body 相关的样式去掉，然后在所有选择器前面加上类前缀 .content
* 因为博客正文的 div 的 class 就是 content
* 因为作者原来的css会跟 MarkdownPad2 的css稍有冲突，所以改了一下 `static/blog/css/style.css` 。把这段注掉了：

		:::css
		.detail h2 {
		    margin:10px 0 0 0;
		    font-size: 16px;
		    height: 24px;
		    padding-bottom: 10px;
		}