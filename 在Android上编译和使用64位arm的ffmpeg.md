>关于 arm CPU 的名字(来自百度百科)  
>ARMv8架构包含两个执行状态：AArch64和AArch32。AArch64执行状态针对64位处理技术，引入了一个全新指令集A64；而AArch32执行状态将支持现有的ARM指令集。目前的ARMv7架构的主要特性都将在ARMv8架构中得以保留或进一步拓展
### config 脚本需要修改的地方：
* `--arch=aarch64`
* `SYSROOT=$NDK/platforms/android-9/arch-arm64/`
* `CROSS_PREFIX=$NDK/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-`
* `--extra-cflags="-Os -fpic"`  # 去掉了 -marm
### Android项目需要改的地方：
* `Application.mk` 里，改成 `APP_ABI := arm64-v8a`