export MakeInc_cmd=${SRCROOT}/makedefs/MakeInc.cmd
export MakeInc_def=${SRCROOT}/makedefs/MakeInc.def
export MakeInc_rule=${SRCROOT}/makedefs/MakeInc.rule
export MakeInc_dir=${SRCROOT}/makedefs/MakeInc.dir

include $(MakeInc_cmd)
include $(MakeInc_def)

OPSYS := $(shell uname -s)

OBJS = kextsymboltool.o

CFLAGS = -g -O0 -Wno-implicit-function-declaration \
	-I$(SRCROOT)/../../Developer/Default.xctoolchain/include \
	-I$(SOURCE) -I. 
LDFLAGS = -lstdc++

ifeq ($(OPSYS),Darwin)
CFLAGS += -isysroot $(HOST_SDKROOT) -mmacosx-version-min=$(HOST_OS_VERSION)
LDFLAGS += -isysroot $(HOST_SDKROOT) -mmacosx-version-min=$(HOST_OS_VERSION)
endif

ifeq ($(OPSYS),Linux)
# The linux linker needs these to come after object files :(
export LIBS = -L../../../../Developer/Default.xctoolchain/cctools/libmacho \
	-L../../../../Developer/Default.xctoolchain/cctools/libstuff \
	-luuid -lmacho -lstuff
endif

WARNFLAGS = -Wall -Wno-int-conversion

kextsymboltool: $(OBJS)
	$(call makelog,$(ColorH)HOST_LD$(Color0)       $(ColorF)$@$(Color0))
	$(_v)$(HOST_CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(call makelog,$(ColorH)HOST_CODESIGN$(Color0) $(ColorF)$@$(Color0))
	$(_v)env CODESIGN_ALLOCATE=$(HOST_CODESIGN_ALLOCATE) $(HOST_CODESIGN) -s - $@

%.o: %.c
	$(call makelog,$(ColorH)HOST_CC$(Color0)       $(ColorF)$@$(Color0))
	$(_v)$(HOST_CC) $(WARNFLAGS) $(CFLAGS) -c -o $@ $<

do_build_setup:: kextsymboltool

include $(MakeInc_rule)
include $(MakeInc_dir)
