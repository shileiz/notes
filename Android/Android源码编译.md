编译Android源码:
结果是生成 system.img，userdata.img，ramdisk.img	

---
### boot.img
* boot.img 包含两部分：kernel 和 ramdisk。
* kernel 就是 Linux 系统内核，在 boot.img 里是一个 gz 格式的压缩文件，大小 7M 左右（Android7.0）
* ramdisk 是文件系统，后续 system.img userdata.img 将会被解压到 ramdisk 的 system 和 data 下。init.rc 也在 ramdisk 的根目录下。

#### 解压 boot.img ：
1. Linux 工具： `git clone https://github.com/xiaolu/mkbootimg_tools.git`
2. 运行： `./mkboot boot.img boot` ，把 boot.img 解压缩到 boot 文件夹

---

1. `source build/envsetup.sh`
	1. 该脚本本身会添加很多设备类型（通过调用 `add_lunch_combo` 函数），供后续调用 lunch 时使用
	2. 还会查找 ./device 和 ./vendor 下的 vendorsetup.sh 文件(查找深度为4级目录)，找到后就执行它。它里面只有一行：`add_lunch_combo xxxx`，即把本设备加入到 lunch 可选择的列表里。
	
2. `lunch`
	1. lunch 从你选择的设备中提取出 product 和 varient，导出为环境变量，供编译时使用

			TARGET_PRODUCT=aosp_flounder
			TARGET_BUILD_VARIANT=userdebug