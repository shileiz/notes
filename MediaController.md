* 只要碰了一次 MediaController，它就会在3秒无操作后消失，尽管你是用 show(0) 来让它出现的。
* 你不能在 onCreate() 里调用 `MediaController.setAnchorView()`，因为在 onCreate() 退出之前，你的 AnchorView 还没有被安卓系统激活。