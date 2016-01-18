### config ###
完整的config命令：

    ./configure --target=armv7-android-gcc \
	--extra-cflags="-mfloat-abi=softfp -mfpu=neon" \
	--sdk-path=/home/shilei/android-ndk-r10b \
	--libc=/home/shilei/android-ndk-r10b/platforms/android-L/arch-arm \
	--disable-examples \
	--disable-docs \
	--disable-vp8 \
	--disable-vp9-encoder 
说明： vpx的configure脚本把NDK叫做SDK  
说明： `--libc` 如果不配置，configure脚本会去你配置的ndk根目录下搜索`arch-arm`目录，把搜索到的第一个结果当做 `gcc --sysroot=` 参数的值。不过有可能搜错，比如我之前没配，就搜到了`<ndk>/sources/android/libportable/arch-arm`，这是不对的。所以最好手动指定一下。  
说明： 不加 `--extra-cflags="-mfloat-abi=softfp -mfpu=neon"` ，`./configure` 也会成功。但是会在 `make` 时报错说无法 include neon.h。  
说明： 不加 `--disable-examples` 也可以 `./configure` 成功，但是 `make` 的时候会报错说 `#include <new>` `#include <cstring>` 之类的有错，估计是c++的-I没有设对之类的问题，不去详细研究了，干脆不要 example，也不影响。  
说明： `--disable-docs`、`--disable-vp8`、`--disable-vp9-encoder` 这几个选项只是为了去掉用不上的功能，不影响是否成功。

* 关于 configure 的一些东西：
	* `./configure` 结束后会在当前目录生成`Makefile`和几个`.mk`文件，其中`config.mk`会被`Makefile`在第一行include
	* 其中 libs-armv7-android-gcc.mk 也会被 Makefile include

config 之后直接 make 就可以得到 libvpx.a
