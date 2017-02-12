include $(SRC_PATH)/common.mak

LIBVERSION := $(lib$(NAME)_VERSION)
LIBMAJOR   := $(lib$(NAME)_VERSION_MAJOR)
LIBMINOR   := $(lib$(NAME)_VERSION_MINOR)
INCINSTDIR := $(INCDIR)/lib$(NAME)


# SUBDIR是主控 Makefile 定义好的，弄到哪个lib的时候就是哪个，比如 libavcodec/
INSTHEADERS := $(INSTHEADERS) $(HEADERS:%=$(SUBDIR)%)

# LIBNAME 在 config.mak 里定义：LIBNAME=$(LIBPREF)$(FULLNAME)$(LIBSUF)
# LIBNAME 形如：libavcodec.a， SLIBNAME 形如：avcodec.dll
# 这里定义了一个目标：all-yes 依赖于 libavcodec/libavcodec.a
# common.mak 里的默认目标 all 依赖于 all-yes
all-$(CONFIG_STATIC): $(SUBDIR)$(LIBNAME)
all-$(CONFIG_SHARED): $(SUBDIR)$(SLIBNAME)

$(SUBDIR)%-test.o: $(SUBDIR)%-test.c
	$(COMPILE_C)

$(SUBDIR)%-test.o: $(SUBDIR)%.c
	$(COMPILE_C)

$(SUBDIR)%-test.i: $(SUBDIR)%-test.c
	$(CC) $(CCFLAGS) $(CC_E) $<

$(SUBDIR)%-test.i: $(SUBDIR)%.c
	$(CC) $(CCFLAGS) $(CC_E) $<

$(SUBDIR)x86/%$(DEFAULT_YASMD).asm: $(SUBDIR)x86/%.asm
	$(DEPYASM) $(YASMFLAGS) -I $(<D)/ -M -o $@ $< > $(@:.asm=.d)
	$(YASM) $(YASMFLAGS) -I $(<D)/ -e $< | sed '/^%/d;/^$$/d;' > $@

$(SUBDIR)x86/%.o: $(SUBDIR)x86/%$(YASMD).asm
	$(DEPYASM) $(YASMFLAGS) -I $(<D)/ -M -o $@ $< > $(@:.o=.d)
	$(YASM) $(YASMFLAGS) -I $(<D)/ -o $@ $<
	-$(if $(ASMSTRIPFLAGS), $(STRIP) $(ASMSTRIPFLAGS) $@)

LIBOBJS := $(OBJS) $(SUBDIR)%.h.o $(TESTOBJS)
$(LIBOBJS) $(LIBOBJS:.o=.s) $(LIBOBJS:.o=.i):   CPPFLAGS += -DHAVE_AV_CONFIG_H
$(TESTOBJS) $(TESTOBJS:.o=.i): CPPFLAGS += -DTEST
$(TESTOBJS) $(TESTOBJS:.o=.i): CFLAGS += -Umain

# 静态库目标 libavcodec/libavcodec.a: $(OBJS)
# OBJS 由主控 Makefile 传进来，主控 Makefile 是通过 include libavcodec/Makefile 得到的 OBJS 的
# OBJS += $(OBJS-yes) 是在 common.mak 里完成的，总之 OBJS 是 libavcodec/Makefile 里的 OBJS 和 OBJS-yes 的组合
# 即所有 libavcodec 需要的.o
$(SUBDIR)$(LIBNAME): $(OBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $(AR_O) $^
	$(RANLIB) $@

install-headers: install-lib$(NAME)-headers install-lib$(NAME)-pkgconfig

install-libs-$(CONFIG_STATIC): install-lib$(NAME)-static
install-libs-$(CONFIG_SHARED): install-lib$(NAME)-shared

define RULES
$(TOOLS):     THISLIB = $(FULLNAME:%=$(LD_LIB))
$(TESTPROGS): THISLIB = $(SUBDIR)$(LIBNAME)

$(TESTPROGS) $(TOOLS): %$(EXESUF): %.o $(EXEOBJS)
	$$(LD) $(LDFLAGS) $(LDEXEFLAGS) $$(LD_O) $$(filter %.o,$$^) $$(THISLIB) $(FFEXTRALIBS) $$(ELIBS)

# 共享库目标
$(SUBDIR)$(SLIBNAME): $(SUBDIR)$(SLIBNAME_WITH_MAJOR)
	$(Q)cd ./$(SUBDIR) && $(LN_S) $(SLIBNAME_WITH_MAJOR) $(SLIBNAME)

$(SUBDIR)$(SLIBNAME_WITH_MAJOR): $(OBJS) $(SLIBOBJS) $(SUBDIR)lib$(NAME).ver
	$(SLIB_CREATE_DEF_CMD)
	$$(LD) $(SHFLAGS) $(LDFLAGS) $(LDLIBFLAGS) $$(LD_O) $$(filter %.o,$$^) $(FFEXTRALIBS)
	$(SLIB_EXTRA_CMD)

ifdef SUBDIR
$(SUBDIR)$(SLIBNAME_WITH_MAJOR): $(DEP_LIBS)
endif

clean::
	$(RM) $(addprefix $(SUBDIR),*-test$(EXESUF) $(CLEANFILES) $(CLEANSUFFIXES) $(LIBSUFFIXES)) \
	    $(CLEANSUFFIXES:%=$(SUBDIR)$(ARCH)/%)

distclean:: clean
	$(RM) $(DISTCLEANSUFFIXES:%=$(SUBDIR)%) $(DISTCLEANSUFFIXES:%=$(SUBDIR)$(ARCH)/%)

install-lib$(NAME)-shared: $(SUBDIR)$(SLIBNAME)
	$(Q)mkdir -p "$(SHLIBDIR)"
	$$(INSTALL) -m 755 $$< "$(SHLIBDIR)/$(SLIB_INSTALL_NAME)"
	$$(STRIP) "$(SHLIBDIR)/$(SLIB_INSTALL_NAME)"
	$(Q)$(foreach F,$(SLIB_INSTALL_LINKS),(cd "$(SHLIBDIR)" && $(LN_S) $(SLIB_INSTALL_NAME) $(F));)
	$(if $(SLIB_INSTALL_EXTRA_SHLIB),$$(INSTALL) -m 644 $(SLIB_INSTALL_EXTRA_SHLIB:%=$(SUBDIR)%) "$(SHLIBDIR)")
	$(if $(SLIB_INSTALL_EXTRA_LIB),$(Q)mkdir -p "$(LIBDIR)")
	$(if $(SLIB_INSTALL_EXTRA_LIB),$$(INSTALL) -m 644 $(SLIB_INSTALL_EXTRA_LIB:%=$(SUBDIR)%) "$(LIBDIR)")

install-lib$(NAME)-static: $(SUBDIR)$(LIBNAME)
	$(Q)mkdir -p "$(LIBDIR)"
	$$(INSTALL) -m 644 $$< "$(LIBDIR)"
	$(LIB_INSTALL_EXTRA_CMD)

install-lib$(NAME)-headers: $(addprefix $(SUBDIR),$(HEADERS) $(BUILT_HEADERS))
	$(Q)mkdir -p "$(INCINSTDIR)"
	$$(INSTALL) -m 644 $$^ "$(INCINSTDIR)"

install-lib$(NAME)-pkgconfig: $(SUBDIR)lib$(FULLNAME).pc
	$(Q)mkdir -p "$(PKGCONFIGDIR)"
	$$(INSTALL) -m 644 $$^ "$(PKGCONFIGDIR)"

uninstall-libs::
	-$(RM) "$(SHLIBDIR)/$(SLIBNAME_WITH_MAJOR)" \
	       "$(SHLIBDIR)/$(SLIBNAME)"            \
	       "$(SHLIBDIR)/$(SLIBNAME_WITH_VERSION)"
	-$(RM)  $(SLIB_INSTALL_EXTRA_SHLIB:%="$(SHLIBDIR)/%")
	-$(RM)  $(SLIB_INSTALL_EXTRA_LIB:%="$(LIBDIR)/%")
	-$(RM) "$(LIBDIR)/$(LIBNAME)"

uninstall-headers::
	$(RM) $(addprefix "$(INCINSTDIR)/",$(HEADERS) $(BUILT_HEADERS))
	$(RM) "$(PKGCONFIGDIR)/lib$(FULLNAME).pc"
	-rmdir "$(INCINSTDIR)"
endef

$(eval $(RULES))

$(TOOLS):     $(DEP_LIBS) $(SUBDIR)$($(CONFIG_SHARED:yes=S)LIBNAME)
$(TESTPROGS): $(DEP_LIBS) $(SUBDIR)$(LIBNAME)

testprogs: $(TESTPROGS)
