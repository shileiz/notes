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
