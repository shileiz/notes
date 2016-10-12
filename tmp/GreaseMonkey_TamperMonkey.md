## GreaseMonkey 和 TamperMonkey
* Firefox 叫 GreaseMonkey，Chrome 叫 TamperMonkey
* 下载安装即可，去官网
* 每个URL可以指定一个js脚本，当你打开这个URL时就自动执行

## localStorage
* 每个域对应一个 localStorage，关闭浏览器再打开也依然有效
	* 这样的问题是，只能在同一台电脑的chrome上好使，换一台电脑就不行了
	* 等有时间了，想个办法把 localStorage 里的东西导出成文件，现在只在一个电脑上玩，没需求
* 不过 localStorage 只能存字符串，不能直接存 javascript 的对象（包括Array）。所以要用 JASON 做中介

## 代码
* 最后完整的代码是这样的：

		:::javascript
		// ==UserScript==
		// @name         Zhilian SearchResult
		// @namespace    http://tampermonkey.net/
		// @version      0.1
		// @description  隐藏已经投过的公司
		// @author       zsl
		// @match        http://sou.zhaopin.com/jobs/searchresult.ashx*
		// @grant        none
		// ==/UserScript==
		
		(function() {
		    'use strict';
		
		    // 全局变量，已经投过的公司 array
		    var deliveredList;
		
		    // 从 localStorage 读出已经投过的公司列表
		    function loadDeliveredCompnayFromLocalStorage()
		    {
		        deliveredList = JSON.parse(localStorage.deliveredList);
		    }
		
		    function saveDeliveredCompnayToLocalStorage()
		    {
		        localStorage.deliveredList = JSON.stringify(deliveredList);
		    }
		
		    function hideDeliveredCompnay()
		    {
		        // gsmc = 公司名称...
		        var gsmcList = document.getElementsByClassName("gsmc");
		        // 从 1 开始循环即可，gsmcList[0] 是一个 th 元素，不是我们想要的，其他的都是这个 th 内部的 td 元素，这些才是我们想要的
		        for(var i=1;i<gsmcList.length;i++)
		        {
		            if(isDelivered(gsmcList[i].firstChild.firstChild.nodeValue))
		            {
		                gsmcList[i].parentNode.style.backgroundColor  = " #000 ";
		                gsmcList[i].parentNode.style.color=" #000 ";
		            }
		        }
		    }
		
		    function isDelivered(company_name)
		    {
		        for(var i=0;i<deliveredList.length;i++)
		        {
		            if(company_name == deliveredList[i]) return true;
		        }
		        return false;
		    }
		
		    // 点击申请职位之前，需要记住勾选了哪些公司，把他们加入 localStorage
		    function beforeApply()
		    {
		        var check_box_list = document.getElementsByName("vacancyid");
		        for(var i in check_box_list)
		        {
		            if(check_box_list[i].checked)
		            {
		                var checked_compnay = check_box_list[i].parentNode.parentNode.getElementsByClassName("gsmc")[0].firstChild.firstChild.nodeValue;
		                if(deliveredList.indexOf(checked_compnay) == -1)
		                    deliveredList.push(checked_compnay);
		            }
		        }
		        saveDeliveredCompnayToLocalStorage();
		    }
		
		    function main()
		    {
		        // 从 localStorage 载入已经投递过的公司列表
		        loadDeliveredCompnayFromLocalStorage();
		
		        // 隐藏掉已经投递过的公司
		        hideDeliveredCompnay();
		
		        //  给“申请职位” 按钮加监听，把选中的公司加到 localStorage 里
		        // "申请职位按钮，有两个，分别位于页面顶部和底部"
		        var btn_sqzw_list = document.getElementsByClassName("newlist_sqzw");
		        for(var i=0;i<btn_sqzw_list.length;i++)
		        {
		            btn_sqzw_list[i].onclick = function(){beforeApply();};
		        }
		    }
		    main();
		})();

## TODO (maybe)
* 过滤一下不想看到的职位，比如“游戏”
* 上面提到的把 localStorage 里的东西固化到硬盘上