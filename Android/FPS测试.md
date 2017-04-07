## FPS 测试大概有以下几种方案（百度的）：

1. 开启系统的“GPU呈现模式分析”，然后用 dumpsys gfxinfo 把 fps 信息输出到文件里。参考：[http://www.cnblogs.com/summer-sun/p/5524663.html](http://www.cnblogs.com/summer-sun/p/5524663.html)
2. 用 FPS Meter 软件，这个之前用过的，没办法自动化。
3. 用命令 `adb shell dumpsys SurfaceFlinger --latency <window_activity>`  获取 FPS,然后用脚本分析结果。 参考：[http://blog.csdn.net/itfootball/article/details/43084527](http://blog.csdn.net/itfootball/article/details/43084527)  


## FPS 能反映视频流畅度吗？
* 参考：[http://blog.csdn.net/xiaosongluo/article/details/51212296](http://blog.csdn.net/xiaosongluo/article/details/51212296)
* 参考：[http://blog.csdn.net/luoshengyang/article/details/7846923](http://blog.csdn.net/luoshengyang/article/details/7846923)