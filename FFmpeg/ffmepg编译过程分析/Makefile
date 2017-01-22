MAIN_MAKEFILE=1
include config.mak


# SRC_PATH 在 config.mak 里定义：SRC_PATH=.
# 当然，config.mak 受到用户执行 configure 命令的影响，SRC_PATH 也可能是用户指定的其他值
# vpath 函数为各种类型的源文件设置搜索路径，默认都是 SRC_PATH
vpath %.c    $(SRC_PATH)
vpath %.cpp  $(SRC_PATH)
vpath %.h    $(SRC_PATH)
vpath %.m    $(SRC_PATH)
vpath %.S    $(SRC_PATH)
vpath %.asm  $(SRC_PATH)
vpath %.rc   $(SRC_PATH)
vpath %.v    $(SRC_PATH)
vpath %.texi $(SRC_PATH)
vpath %/fate_config.sh.template $(SRC_PATH)

# 如果用户执行 configure 时打开了 ffmpeg(没有加 --disable-ffmpeg)，则 CONFIG_FFMPEG=yes 会出现在 config.mak 里
# 所以这里其实是往 AVPROGS-yes 里添加用户配置了的 programs，如果用户没配 ffserver，则 ffserver 只会加到
# AVPROGS- 里，而不会加到 AVPROGS-yes 里。AVPROGS- 是个后面不会用到的变量，所以无所谓。
AVPROGS-$(CONFIG_FFMPEG)   += ffmpeg
AVPROGS-$(CONFIG_FFPLAY)   += ffplay
AVPROGS-$(CONFIG_FFPROBE)  += ffprobe
AVPROGS-$(CONFIG_FFSERVER) += ffserver

# EXESUF PROGSSUF 都是在 config.mak，根据用户配置生成的，如果用户加了 --toolchain=msvc，则 EXESUF=.exe
# PROGSSUF 默认是空
# 所以以下语句的结果是 AVPROGS = ffmpeg.exe ffplay.exe ffprobe.exe
# PROGS = ffmpeg.exe ffplay.exe ffprobe.exe
AVPROGS    := $(AVPROGS-yes:%=%$(PROGSSUF)$(EXESUF))
INSTPROGS   = $(AVPROGS-yes:%=%$(PROGSSUF)$(EXESUF))
PROGS      += $(AVPROGS)

AVBASENAMES  = ffmpeg ffplay ffprobe ffserver
ALLAVPROGS   = $(AVBASENAMES:%=%$(PROGSSUF)$(EXESUF))
ALLAVPROGS_G = $(AVBASENAMES:%=%$(PROGSSUF)_g$(EXESUF))


# OBJS-ffmpeg, OBJS-ffplay 都有对应的 OBJS-ffmpeg-yes, OBJS-ffplay-yes
# OBJS-ffmpeg, OBJS-ffplay 代表无论配不配置，都必须链接进程序的那些 obj
# OBJS-ffmpeg-yes, OBJS-ffplay-yes 代表那些只有用户在 configure 阶段配置了才需要链接进程序的 obj
# cmdutils.o 就是无论用户配没配置，都需要链接到程序 ffmpeg，ffplay，ffprobe，ffserver 里的
# cmdutils_opencl.o 会根据用户是否配置了 OPENCL 决定加到 OBJS-ffmpeg-yes 里与否。
$(foreach prog,$(AVBASENAMES),$(eval OBJS-$(prog) += cmdutils.o))
$(foreach prog,$(AVBASENAMES),$(eval OBJS-$(prog)-$(CONFIG_OPENCL) += cmdutils_opencl.o))

# OBJS-ffmpeg = ffmpeg_opt.o ffmpeg_filter.o
# OBJS-ffmpeg-yes = 
OBJS-ffmpeg                   += ffmpeg_opt.o ffmpeg_filter.o
OBJS-ffmpeg-$(HAVE_VDPAU_X11) += ffmpeg_vdpau.o
OBJS-ffmpeg-$(HAVE_DXVA2_LIB) += ffmpeg_dxva2.o
ifndef CONFIG_VIDEOTOOLBOX
OBJS-ffmpeg-$(CONFIG_VDA)     += ffmpeg_videotoolbox.o
endif
OBJS-ffmpeg-$(CONFIG_VIDEOTOOLBOX) += ffmpeg_videotoolbox.o
OBJS-ffserver                 += ffserver_config.o

# $(FFLIBS-yes) needs to be in linking order
# FFLIBS-yes = avdevice avfilter avformat ...
FFLIBS-$(CONFIG_AVDEVICE)   += avdevice
FFLIBS-$(CONFIG_AVFILTER)   += avfilter
FFLIBS-$(CONFIG_AVFORMAT)   += avformat
FFLIBS-$(CONFIG_AVCODEC)    += avcodec
FFLIBS-$(CONFIG_AVRESAMPLE) += avresample
FFLIBS-$(CONFIG_POSTPROC)   += postproc
FFLIBS-$(CONFIG_SWRESAMPLE) += swresample
FFLIBS-$(CONFIG_SWSCALE)    += swscale

FFLIBS := avutil

# common.mak 里会把 FFLIBS 扩展成：
# FFLIBS := $($(NAME)_FFLIBS) $(FFLIBS-yes) $(FFLIBS)
# NAME 由 liavxxx/Makefile 定义，例如 avcodec
# 由于到目前为止并未 include liavxxx/Makefile，所以 NAME 尚未定义
# 所以目前为止（include common.mak 之后），FFLIBS = avdevice avfilter ... avutil
include $(SRC_PATH)/common.mak

FF_EXTRALIBS := $(FFEXTRALIBS)

# DEP_LIBS 在 common.mak 里定义，它是根据 config.mak 生成的包含目录的库名
# 例如：libavdevice/libavdevice.a libavformat/libavformat.a ....
# DEP_LIBS 一方面依赖主控 Makefile 写死的 FFLIBS，另一方面依赖于 config.mak 里配置的前缀后缀名
FF_DEP_LIBS  := $(DEP_LIBS)
FF_STATIC_DEP_LIBS := $(STATIC_DEP_LIBS)

all: $(AVPROGS)


SUBDIR_VARS := CLEANFILES EXAMPLES FFLIBS HOSTPROGS TESTPROGS TOOLS      \
               HEADERS ARCH_HEADERS BUILT_HEADERS SKIPHEADERS            \
               ARMV5TE-OBJS ARMV6-OBJS ARMV8-OBJS VFP-OBJS NEON-OBJS     \
               ALTIVEC-OBJS MMX-OBJS YASM-OBJS                           \
               MIPSFPU-OBJS MIPSDSPR2-OBJS MIPSDSPR1-OBJS MSA-OBJS       \
               MMI-OBJS OBJS SLIBOBJS HOSTOBJS TESTOBJS

define RESET
$(1) :=
$(1)-yes :=
endef

define DOSUBDIR
$(foreach V,$(SUBDIR_VARS),$(eval $(call RESET,$(V))))
SUBDIR := $(1)/
include $(SRC_PATH)/$(1)/Makefile
-include $(SRC_PATH)/$(1)/$(ARCH)/Makefile
-include $(SRC_PATH)/$(1)/$(INTRINSICS)/Makefile
include $(SRC_PATH)/library.mak
endef

$(foreach D,$(FFLIBS),$(eval $(call DOSUBDIR,lib$(D))))

include $(SRC_PATH)/doc/Makefile


# $(1)$(PROGSSUF)_g$(EXESUF): LDFLAGS += $(LDFLAGS-$(1)) 的意思是
# 定义目标变量 ffmpeg_g:LDFLAGS += $(LDFLAGS-ffmpeg)
# 不过 LDFLAGS-ffmpeg 一般来说为空，没找到在哪里定义的，所以没啥用
define DOPROG
OBJS-$(1) += $(1).o $(EXEOBJS) $(OBJS-$(1)-yes)
$(1)$(PROGSSUF)_g$(EXESUF): $$(OBJS-$(1))
$$(OBJS-$(1)): CFLAGS  += $(CFLAGS-$(1))
$(1)$(PROGSSUF)_g$(EXESUF): LDFLAGS += $(LDFLAGS-$(1))
$(1)$(PROGSSUF)_g$(EXESUF): FF_EXTRALIBS += $(LIBS-$(1))
-include $$(OBJS-$(1):.o=.d)
endef

# 此时 PROGS=ffmpeg.exe ffplay.exe ffprobe.exe
# $(P:$(PROGSSUF)$(EXESUF)=)的目的是去掉后缀 .exe，相当于 $(ffmepg.exe:.exe=)
# 所以调用命令包的时候，传入的参数是 ffmpeg，ffplay，ffprobe
$(foreach P,$(PROGS),$(eval $(call DOPROG,$(P:$(PROGSSUF)$(EXESUF)=))))

ffprobe.o cmdutils.o libavcodec/utils.o libavformat/utils.o libavdevice/avdevice.o libavfilter/avfilter.o libavutil/utils.o libpostproc/postprocess.o libswresample/swresample.o libswscale/utils.o : libavutil/ffversion.h

# ffmpeg.exe 依赖于 ffmepg_g.exe
# 生成命令是：cp ffmpeg_g.exe ffmpeg.exe ; strip ffmpeg.exe
$(PROGS): %$(PROGSSUF)$(EXESUF): %$(PROGSSUF)_g$(EXESUF)
	$(CP) $< $@
	$(STRIP) $@


# ffmpeg_g.exe 依赖于 ffmpeg.o 和 FF_DEP_LIBS
# FF_DEP_LIBS := $(DEP_LIBS) 
# DEP_LIBS 在 common.mak 里定义，它包含多少个库是写死的，取决于主控 Makefile 写死的 FFLIBS 和 FFLIBS-yes 变量
# 而每个库的库名的格式是根据用户配置的 config.mak 生成的，可以是 libavcodec/libavcodec.a，也可以是 libavcodec/libavcodec.so，windows上还可能是 .dll
# 生成 ffmpeg_g.exe 的命令是：$(LD) $(LDFLAGS) $(LDEXEFLAGS) $(LD_O) $(OBJS-$*) $(FF_EXTRALIBS)
# 其中的 $(OBJS-$*) 即 $(OBJS-ffmpeg)，这是这主控 Makefile 写死并经命令包 DOPROG 处理过的
# OBJS-ffmpeg += ffmpeg.o $(OBJS-ffmpeg-yes)，最终值形如： OBJS-ffmpeg = ffmpeg.o cmdutils.o ffmpeg_opt.o
# 其中的 $(LD_O) 是用户配置的，在 config.mak 里。Linux 平台时 LD_O=-o $@，windows 平台时 LD_O=-out:$@
# 其中的 $(LDEXEFLAGS) 在 config.mak 里，用户配置，默认是空
# 其中的 $(LDFLAGS) 在 common.mak 里定义，由写死的 ALLFFLIBS 拼接用户定义的 LDFLAGS 而成
# 另外，虽然命令包 DOPROG 里定义了目标变量 ffmepg_g:LDFLAGS，但实际上并没改变它的值
# $(FF_EXTRALIBS) 是在主控里定义的，它取 common.mak 里的 $(FFEXTRALIBS)
# 最终其展开为 -lavdevice -lavfilter ... -lm -lz -pthread
# 其中前半部分是写死的，写死在主控里的 FFLIBS/FFLIBS-yes 造就了 -lavdevice -lavfilter ...
# 后半部分是根据用户配置生成在 config.mak 里的
%$(PROGSSUF)_g$(EXESUF): %.o $(FF_DEP_LIBS)
	$(LD) $(LDFLAGS) $(LDEXEFLAGS) $(LD_O) $(OBJS-$*) $(FF_EXTRALIBS)

OBJDIRS += tools

-include $(wildcard tools/*.d)

VERSION_SH  = $(SRC_PATH)/version.sh
GIT_LOG     = $(SRC_PATH)/.git/logs/HEAD

.version: $(wildcard $(GIT_LOG)) $(VERSION_SH) config.mak
.version: M=@

libavutil/ffversion.h .version:
	$(M)$(VERSION_SH) $(SRC_PATH) libavutil/ffversion.h $(EXTRA_VERSION)
	$(Q)touch .version

# force version.sh to run whenever version might have changed
-include .version

ifdef AVPROGS
install: install-progs install-data
endif

install: install-libs install-headers

install-libs: install-libs-yes

install-progs-yes:
install-progs-$(CONFIG_SHARED): install-libs

install-progs: install-progs-yes $(AVPROGS)
	$(Q)mkdir -p "$(BINDIR)"
	$(INSTALL) -c -m 755 $(INSTPROGS) "$(BINDIR)"

install-data: $(DATA_FILES) $(EXAMPLES_FILES)
	$(Q)mkdir -p "$(DATADIR)/examples"
	$(INSTALL) -m 644 $(DATA_FILES) "$(DATADIR)"
	$(INSTALL) -m 644 $(EXAMPLES_FILES) "$(DATADIR)/examples"

uninstall: uninstall-libs uninstall-headers uninstall-progs uninstall-data

uninstall-progs:
	$(RM) $(addprefix "$(BINDIR)/", $(ALLAVPROGS))

uninstall-data:
	$(RM) -r "$(DATADIR)"

clean::
	$(RM) $(ALLAVPROGS) $(ALLAVPROGS_G)
	$(RM) $(CLEANSUFFIXES)
	$(RM) $(CLEANSUFFIXES:%=tools/%)
	$(RM) -r coverage-html
	$(RM) -rf coverage.info lcov

distclean::
	$(RM) $(DISTCLEANSUFFIXES)
	$(RM) config.* .config libavutil/avconfig.h .version avversion.h version.h libavutil/ffversion.h libavcodec/codec_names.h

config:
	$(SRC_PATH)/configure $(value FFMPEG_CONFIGURATION)

check: all alltools examples testprogs fate

include $(SRC_PATH)/tests/Makefile

$(sort $(OBJDIRS)):
	$(Q)mkdir -p $@

# Dummy rule to stop make trying to rebuild removed or renamed headers
%.h:
	@:

# Disable suffix rules.  Most of the builtin rules are suffix rules,
# so this saves some time on slow systems.
.SUFFIXES:

.PHONY: all all-yes alltools check *clean config install*
.PHONY: testprogs uninstall*
