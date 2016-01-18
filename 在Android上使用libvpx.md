libvpx 不包括webm文件的解析，只负责编解码。  
解析webm需要用到 libwebm。  
libwebm 在源码目录的：`libvpx/third_party/libwebm`  
libwebm 可以单独编译成一个库。  
libvpx 使用了库 libwebm（可选，可以在config的时候打开或者关闭wenm-io）。  
libvpx 使用 libwebm 的时候，给 libwebm 外面又封装了一层，使得用起来更加方便。封装在: `libvpx/webmenc.cc`  和 `libvpx/webmdec.cc`  
