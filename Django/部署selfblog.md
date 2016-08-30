#### 安装 sqlite-devel （需要在安装python27之前，要不然还得重装python27）

	yum install sqlite-devel

#### 升级python到2.7.10（默认是2.6.6）
* 注意：yum 是依赖python的，如果你把python2.6整个删除了用2.7替代，那么yum就工作不了了
* 参考：http://blog.csdn.net/jcjc918/article/details/11022345
	* 下载： wget http://www.python.org/ftp/python/2.7.10/Python-2.7.10.tgz
	* 解压：tar -xvzf Python-2.7.10.tgz
	* 安装：

			cd Python-2.7.10
			./configure  
			make all             
			make install  
			make clean  
			make distclean 

		* 可执行程序会被安装到：/usr/local/bin/python2.7
		* 执行一下测试安装成功： /usr/local/bin/python2.7 -V
		* 注意，python27的第三方包会被安装到：/usr/local/lib/python2.7/site-packages/ 

	* 备份原来的 2.6.6：mv /usr/bin/python /usr/bin/python2.6.6  
	* 建立软连接，把2.7作为默认python：ln -s /usr/local/bin/python2.7 /usr/bin/python  
	* 解决 yum 用不了的问题：
		* vi /usr/bin/yum
		* 将文件头部的 `#!/usr/bin/python` 改成 `#!/usr/bin/python2.6.6`
	* 安装pip
		* 去 https://pip.pypa.io/en/stable/installing/ 下载 get-pip.py
		* python get-pip.py
	
#### 安装 requirements 
* 安装 git： yum install git
* 去掉不需要的组件，编辑 requirements.txt，去掉这三行：gunicorn、supervisor、MySQL-python
* pip install -r requirements.txt
		
#### 修改 settings.py
* 把 vps 的 ip 地址加入 ALLOWED_HOSTS

		ALLOWED_HOSTS = ['localhost', '45.62.98.85']

* 把 DEBUG 强制改成 Ture （如果不是 DEBUG，static 得不到）	

#### 初始化数据库
* chmod +x init_database.sh
* dos2unix init_database.sh  // 如果需要
* ./init_database.sh

#### 开启服务
* python manage.py runserver 0.0.0.0:8000 

#### 登录后台
* 打开： http://45.62.98.85:8000/xadmin/
* 用户名密码（init_database.sh时从dump-auth.json导入的）：the5fire / the5fire

#### 安装 Apache + mod_wsgi 
* 自带了 httpd2.2，不用安装
* 安装 httpd-devel，用 yum 安装即可
* 安装 mod_wsgi（不要用 yum 安装，yum 会安装 python2.6 的 mod_wsgi）
	* 下载源码并解压（本次使用 4.5.5） 
	* `./configure --with-apxs=/usr/sbin/apxs --with-python=/usr/local/bin/python `
	* `make`
	* `make install`
	* `make clean`
	* 编辑 `/etc/httpd/conf/httpd.conf`，在 load mod 的部分加入：`LoadModule wsgi_module modules/mod_wsgi.so`
	* 重启 httpd

#### 部署 django 项目到 Apache 上
* 参考： [http://blog.csdn.net/ppdouble/article/details/7718594](http://blog.csdn.net/ppdouble/article/details/7718594)
* 把selfblog 安装到Apache：
	* 把整个 selfblog 目录拷贝到 `/var/www/wsgi-app`
	* 编辑 `/etc/httpd/conf/httpd.conf` , 在最后加入： 

			WSGIScriptAlias / /var/www/wsgi-app/selfblog/selfblog/wsgi.py
			WSGIPythonPath /var/www/wsgi-app/selfblog

* log 文件
	* 修改 settings.py，指定非 DEBUG 模式下的 log 文件位置： `LOG_FILE = '/var/log/selfblog/selfblog.log'`
	* 将上述文件的用户和组，都改为 apache： `chown apache selfblog.log; chgrp apache selfblog.log`
* sqlite 文件的访问
	* 首先在 settings.py 里，把sqlite文件的路径改为绝对路径：`DB_NAME = '/var/www/wsgi-app/selfblog/mydb'`
	* 其次修改文件 mydb 的所有者和组为 apache：` chown apache mydb; chgrp apache mydb;`
	* 最后保证 mydb 所在的目录对于 apache 用户有写权限
* 开启 httpd： `service httpd start`

#### 处理 static 文件
* 暂时用 Apache 处理 static 文件，后续可以考虑装个 lighttpd
* 因为作者已经把 static 都 collect 好了，放在了 selfblog/static 里面，所以我们只需要配置 apache 就行了
* 编辑 Apache 配置文件，在 Alias 部分加入如下:

		Alias /static/ "/var/www/wsgi-app/selfblog/static/"
		
		<Directory "/var/www/wsgi-app/selfblog/static/">
		    Order deny,allow
		    Allow from all
		</Directory>

* 重启 apache，打开主页，格式就正常了

#### 处理 xadmin 的 static 文件
* cp -r /usr/local/lib/python2.7/site-packages/xadmin/static/xadmin/ /var/www/wsgi-app/selfblog/static/