### 要点一： include 
* 把预编译ffmpeg时（执行 `make install` 时）生成的include目录整个拷贝到 jin 目录下
* 为每个预编译的库加上： `LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include`
	> 关于 `LOCAL_EXPORT_C_INCLUDES` 的作用：  
	> 比如 ffmpeg 的头文件里，有大量这种语句： `#include "libavutil/samplefmt.h"`  
	> 而 `samplefmt.h` 并不在当前文件目录下的 `libavutil/` 里  
	> 但只要它在 `LOCAL_EXPORT_C_INCLUDES` 目录下的 `libavutil/` 里就可以了。
### 要点二： 别忘了 `Application.mk` 
### 要点三： 别忘了 `System.loadLibrary` 
* 只要 load 自己那个库就行了，不用把 ffmpeg 的库也 load 进来
### 完整的 Android.mk:
	LOCAL_PATH := $(call my-dir)
	
	include $(CLEAR_VARS)
	LOCAL_MODULE    := pre-ff-avcodec
	LOCAL_SRC_FILES := libavcodec-57.so
	LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
	include $(PREBUILT_SHARED_LIBRARY)
	
	include $(CLEAR_VARS)
	LOCAL_MODULE    := pre-ff-avdevice
	LOCAL_SRC_FILES := libavdevice-57.so
	LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
	include $(PREBUILT_SHARED_LIBRARY)
	
	include $(CLEAR_VARS)
	LOCAL_MODULE    := pre-ff-avfilter
	LOCAL_SRC_FILES := libavfilter-6.so
	LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
	include $(PREBUILT_SHARED_LIBRARY)
	
	include $(CLEAR_VARS)
	LOCAL_MODULE    := pre-ff-avformat
	LOCAL_SRC_FILES := libavformat-57.so
	LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
	include $(PREBUILT_SHARED_LIBRARY)
	
	include $(CLEAR_VARS)
	LOCAL_MODULE    := pre-ff-avutil
	LOCAL_SRC_FILES := libavutil-55.so 
	LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
	include $(PREBUILT_SHARED_LIBRARY)
	
	include $(CLEAR_VARS)
	LOCAL_MODULE    := pre-ff-swresample
	LOCAL_SRC_FILES := libswresample-2.so 
	LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
	include $(PREBUILT_SHARED_LIBRARY)
	
	include $(CLEAR_VARS)
	LOCAL_MODULE    := pre-ff-swscale
	LOCAL_SRC_FILES := libswscale-4.so
	LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
	include $(PREBUILT_SHARED_LIBRARY)
	
	include $(CLEAR_VARS)
	LOCAL_MODULE    := mydecode
	LOCAL_SRC_FILES := mydecode.c
	LOCAL_SHARED_LIBRARIES := pre-ff-avcodec pre-ff-avdevice pre-ff-avfilter pre-ff-avformat pre-ff-avutil pre-ff-swresample pre-ff-swscale
	include $(BUILD_SHARED_LIBRARY)
