Out of memory error
GC overhead limit exceeded.
Try increasing heap size with java option '-Xmx<size>'.


解决方法：
export JACK_SERVER_VM_ARGUMENTS="-Dfile.encoding=UTF-8 -XX:+TieredCompilation -Xmx4g"
./prebuilts/sdk/tools/jack-admin kill-server
./prebuilts/sdk/tools/jack-admin start-server

或者修改文件 /prebuilts/sdk/tools/jack-admin
把 JACK_SERVER_VM_ARGUMENTS 最后加上 -Xmx4g 
