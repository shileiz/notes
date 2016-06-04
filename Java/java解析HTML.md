### 目标
* 在Java里解析html文件，找到想要的节点
* 就像在python里用lxml+xpath那样
### 思路
1. 用html解析器处理
	1. 好处：html解析器比较多，好找。搜了一圈儿决定使用 org.htmlparser，用着还行。
	2. 坏处：不能使用xpath语法，处理起来比较麻烦。在使用中遇到了性能问题。
2. 用xpath解析器处理
	1. 搜了半天没有能直接解析html的xpath解析器，只有能解析xml的（javax.xml.xpath）
	2. 只能先找个第三方工具，把html转成xml（比如 Jtidy），再用xpath解析，也比较麻烦

### 用htmlparser找节点的低效率
* 以下例子是在一个充满了.ts分片文件和一个.m3u8文件的页面寻找那个.m3u8链接的代码：
* 								 
* 								 
		Parser parser = new Parser("http://192.168.8.177/HLS/007_Spectre/");
		NodeList list = parser.extractAllNodesThatMatch(new LinkRegexFilter(".*m3u8", false));
		LinkTag m3u8Node = (LinkTag) list.elementAt(0);
* 页面类似这样：
* 
		<html>
			<head>
			  <title>Index of /HLS/007_Spectre</title>
			</head>
			<body>
				<table>
					<tr><th><img src="/icons/blank.gif" alt="[ICO]"></th><th><a href="?C=N;O=D">Name</a></th><th><a href="?C=M;O=A">Last modified</a></th><th><a href="?C=S;O=A">Size</a></th><th><a href="?C=D;O=A">Description</a></th></tr><tr><th colspan="5"><hr></th></tr>
					<tr><td valign="top"><img src="/icons/back.gif" alt="[DIR]"></td><td><a href="/HLS/">Parent Directory</a></td><td>&nbsp;</td><td align="right">  - </td><td>&nbsp;</td></tr>
					<tr><td valign="top"><img src="/icons/unknown.gif" alt="[   ]"></td><td><a href="007_1080P_5Mbps.m3u8">007_1080P_5Mbps.m3u8</a></td><td align="right">04-Mar-2016 08:45  </td><td align="right"> 35K</td><td>&nbsp;</td></tr>
					<tr><td valign="top"><img src="/icons/text.gif" alt="[TXT]"></td><td><a href="007_1080P_5Mbps0.ts">007_1080P_5Mbps0.ts</a></td><td align="right">04-Mar-2016 08:45  </td><td align="right">4.2M</td><td>&nbsp;</td></tr>
					<tr><td valign="top"><img src="/icons/text.gif" alt="[TXT]"></td><td><a href="007_1080P_5Mbps1.ts">007_1080P_5Mbps1.ts</a></td><td align="right">04-Mar-2016 08:45  </td><td align="right">2.3M</td><td>&nbsp;</td></tr>
					<tr><td valign="top"><img src="/icons/text.gif" alt="[TXT]"></td><td><a href="007_1080P_5Mbps2.ts">007_1080P_5Mbps2.ts</a></td><td align="right">04-Mar-2016 08:45  </td><td align="right">2.0M</td><td>&nbsp;</td></tr>
					<tr><td valign="top"><img src="/icons/text.gif" alt="[TXT]"></td><td><a href="007_1080P_5Mbps3.ts">007_1080P_5Mbps3.ts</a></td><td align="right">04-Mar-2016 08:45  </td><td align="right">239K</td><td>&nbsp;</td></tr>
					<tr><td valign="top"><img src="/icons/text.gif" alt="[TXT]"></td><td><a href="007_1080P_5Mbps4.ts">007_1080P_5Mbps4.ts</a></td><td align="right">04-Mar-2016 08:45  </td><td align="right">1.8M</td><td>&nbsp;</td></tr>
					<tr><td valign="top"><img src="/icons/text.gif" alt="[TXT]"></td><td><a href="007_1080P_5Mbps5.ts">007_1080P_5Mbps5.ts</a></td><td align="right">04-Mar-2016 08:45  </td><td align="right">4.6M</td><td>&nbsp;</td></tr>
					<tr><td valign="top"><img src="/icons/text.gif" alt="[TXT]"></td><td><a href="007_1080P_5Mbps6.ts">007_1080P_5Mbps6.ts</a></td><td align="right">04-Mar-2016 08:45  </td><td align="right"> 17M</td><td>&nbsp;</td></tr>
					<tr><td valign="top"><img src="/icons/text.gif" alt="[TXT]"></td><td><a href="007_1080P_5Mbps7.ts">007_1080P_5Mbps7.ts</a></td><td align="right">04-Mar-2016 08:45  </td><td align="right"> 13M</td><td>&nbsp;</td></tr>
					<tr><td valign="top"><img src="/icons/text.gif" alt="[TXT]"></td><td><a href="007_1080P_5Mbps8.ts">007_1080P_5Mbps8.ts</a></td><td align="right">04-Mar-2016 08:45  </td><td align="right"> 16M</td><td>&nbsp;</td></tr>
					<tr><td valign="top"><img src="/icons/text.gif" alt="[TXT]"></td><td><a href="007_1080P_5Mbps9.ts">007_1080P_5Mbps9.ts</a></td><td align="right">04-Mar-2016 08:45  </td><td align="right">8.6M</td><td>&nbsp;</td></tr>
					<tr><td valign="top"><img src="/icons/text.gif" alt="[TXT]"></td><td><a href="007_1080P_5Mbps10.ts">007_1080P_5Mbps10.ts</a></td><td align="right">04-Mar-2016 08:45  </td><td align="right">7.3M</td><td>&nbsp;</td></tr>
				</table>
			</body>
		</html>
* 影响效率的是这句话： `parser.extractAllNodesThatMatch(new LinkRegexFilter(".*m3u8", false))`
* 我们想要找到m3u8就结束，但extractAllNodesThatMatch会把所有的Node都比对一遍才结束，浪费了很多时间。
* 根据测试，当某个页面的node很多的时候（比如一个含有上千行.ts的页面），执行以上那行代码需要将近1秒的时间
* 而parser又没有提供一个“找到就结束”的方法
### 提高效率的一种方法：改为递归
* 把以上程序改下，深度优先递归查找，找到就结束：
* 
		Node root = parser.parse(new TagNameFilter("html")).elementAt(0);
		LinkTag m3u8Node = (LinkTag) findM3u8Node(root);
		private Node findM3u8Node(Node node) {
			if (isM3u8Node(node)) {
				return node;
			} else {
				NodeList list = node.getChildren();
				if (list != null) {
					for (Node n : list.toNodeArray()) {
						Node result = findM3u8Node(n);
						if (result != null) {
							return result;
						}
					}
				}
				return null;
			}
		}
	
		private boolean isM3u8Node(Node node) {
			return new LinkRegexFilter(".*m3u8", false).accept(node);
		}
* 根据测试，以上方法在上千个分片的页面也会在10毫秒只能结束。

### getLink() 得到的 String 是被 URLEncode 后的
* getLink() 得到的 String 是被 URLEncode 后的，里面的汉字变成了 %3e 这种形状
* 想恢复成汉字要 `URLDecoder.decode(node.getLink(),"UTF-8")`