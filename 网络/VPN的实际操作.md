## 总述
* 好处：客户端不需要安装软件，iOS/OSX、Windows、Android 系统，都可以用系统自带的VPN客户端连接
* 不过，不同的操作系统，对VPN类型的支持有所不同，具体可参考这篇的后半部分：[http://bo.moioi.com/2015/pptp-l2tp-ikev2/](http://bo.moioi.com/2015/pptp-l2tp-ikev2/)
* 为了选择一种尽可能各个操作系统都原生支持的VPN，最终选择了使用 IPSec VPN。因为 OpenVPN（SSL-VPN）的iphone客户端被墙了，不越狱装不上。
	* 这里的 IPSec VPN 指的是 IPsec/IKEv1 或者 IPSec/IKEv2，而不是 IPSec/L2TP。
	* IKE（Internet Key Exchange）协议是 IPSec （IP Security） 协议族的一部分，IKEv2 是 IKEV1 的改良版。
	* IPSec 要是想全搞明白估计得一学期，只是使用的话没必要弄那么透彻。

## 服务端

### 原理
1. 使用 VPN，实现PC和服务器的“伪二层”互通。这里使用的是 Strongswan，一个实现了 IPSec 协议的开源项目。
2. 配置服务器端实现转发
3. 配置服务器端的 iptables 实现 NAT。

### 安装及配置
* 参考：[https://quericy.me/blog/699/](https://quericy.me/blog/699/)
* 使用了[一键安装脚本](https://github.com/quericy/one-key-ikev2-vpn)，比自己一步一步搞方便多了
* 这个脚本里集合了 Strongswan 的下载、编译安装、配置；配置转发；配置NAT（通过iptables）


## 客户端
* 不用安装特定的客户端软件，各种操作系统都自带了 VPN Client，只需要进行一些配置，直接使用即可。
* 下面分别说明各种操作系统如何配置。

### iOS（iOS9/iOS10）
* 设置 --> 通用 --> 添加VPN，类型选择 IPSec
* 填写服务器IP、用户名、密码、秘钥即可

### Windwos（Windows7/Windows10）
* 分别在 Windows7 和 Windows10 上进行了测试

##### 一.导入证书
1. 把 ca.cert.pem 拷贝到本地，修改后缀名为 .cer
2. 打开 mmc（开始 --> 运行 --> mmc）
3. 文件 --> 添加或删除管理单元 --> 证书 --> 添加 --> 选择“计算机账户” --> 完成 --> 确定
4. 找到 “受信任的根证书颁发机构” --> 证书 --> 右键 “所有任务” --> 导入。选择从服务器拷贝来的证书文件。
5. 成功导入证书后，关闭mmc即可，问你是否保存可以选择“否”
6. **注意**，一定要用mmc方式导入证书，用证书管理器导入的证书经过验证有些电脑上不好使

##### 二.建立VPN连接
1. 打开“网络和共享中心” --> “设置新的连接或网络” --> "连接到工作区" --> “使用我的Internet连接VPN”
2. 输入服务器IP地址，确定
3. 在网络连接界面可以看到刚才新建的VPN连接，右键连接，输入用户名密码即可。
4. 注意，可以在VPN连接上右键属性，在安全选项卡里，把VPN类型选择为IKEv2，也可以不选，用默认的自动。其他选项均不改即可。

### Android
* 想进入添加VPN界面需要输入“凭证存储密码”，只要给手机设置一个PIN密码即可。如果还不行试试给手机设置个解锁屏幕密码。总之“凭证存储密码”可以尝试多次，即便失败了被Android删除了“凭证存储”也不影响啥。
* 添加VPN的时候，选择类型为“IPSec Xauth PSK”
* 填写服务器IP、用户名、密码、秘钥即可
* 但不知为何，测试用的Android连上了VPN也上不去youtube等网站，没有深入研究。