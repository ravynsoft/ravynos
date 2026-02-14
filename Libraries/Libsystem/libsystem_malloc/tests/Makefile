PROJECT := libmalloc
TEST_DIR := tests/

DEVELOPER_DIR ?= /Applications/Xcode.app/Contents/Developer/

include $(DEVELOPER_DIR)/AppleInternal/Makefiles/darwintest/Makefile.common

TRACE_FILES := \
	$(notdir $(wildcard $(SRCROOT)/../traces/*.mtrace))

# add trace files without the traces/ prefix, fex:
#   exclude_this_file.mtrace
EXCLUDED_TRACE_FILES :=

CUSTOM_TARGETS = \
	MALLOC_BENCH

OTHER_TEST_TARGETS = \
	$(addprefix nano-trace-replay_, $(basename $(filter-out $(EXCLUDED_TRACE_FILES), $(TRACE_FILES)))) 

MALLOCBENCHTEST_DIR = $(SRCROOT)/MallocBenchTest
BATS_PLISTS_EXTRACT = \
	$(patsubst %,$(SYMROOT)/%.plist,$(OTHER_TEST_TARGETS))

EXCLUDED_SOURCES := \
	nano_trace_replay.c
CXX := $(shell $(XCRUN) -sdk "$(TARGETSDK)" -find clang++)

WARNING_CFLAGS := -Wno-format-invalid-specifier -Wno-format-extra-args
OTHER_CFLAGS += \
	-DDARWINTEST \
	-DOS_UNFAIR_LOCK_INLINE=1 \
	-lCrashReporterClient \
	-I$(SDKROOT)/System/Library/Frameworks/System.framework/PrivateHeaders \
	-I$(SRCROOT)/../private \
	-I$(SRCROOT)/../include \
	$(WARNING_CFLAGS)

PRIVATE_FRAMEWORKS = $(SDKROOT)/System/Library/PrivateFrameworks
PERFDATA_FRAMEWORK = perfdata

nano-trace-replay_%: $(SRCROOT)/nano_trace_replay.c
	$(CC) \
		$(CFLAGS) $(DT_CFLAGS) $(OTHER_CFLAGS) \
		$(LDFLAGS) $(DT_LDFLAGS) $(OTHER_LDFLAGS) \
		-DTRACE_NAME="$(patsubst nano-trace-replay_%,%,$(notdir $@))" \
		-o $@ \
		$(SRCROOT)/nano_trace_replay.c

# For now, we only build and run MallocBench on iOS. macOS requires
# us to adopt installAPI.
LOWER_SDKROOT  = $(shell echo $(SDKROOT) | tr '[:upper:]' '[:lower:]')
ifneq (, $(findstring iphoneos,$(LOWER_SDKROOT)))
include MallocBenchTest/Makefile.malloc-bench
BATS_PLISTS = $(BATS_PLISTS_EXTRACT) $(MALLOCBENCHTEST_DIR)/MallocBench.plist
else
include MallocBenchTest/Makefile.dummy
BATS_PLISTS = $(BATS_PLISTS_EXTRACT)
endif

$(BATS_PLISTS_EXTRACT): %.plist : %
	$(EXTRACTMETA) extract -i /$(INSTALLPATH)/$(notdir $<) -b $(SYMROOT)/$(notdir $<) -o $@
	@plutil -convert binary1 $@

SANITIZER_DYLIB_PATH := /usr/local/lib/sanitizers/
asan: OTHER_CFLAGS += -fsanitize=address
asan: OTHER_LDFLAGS += -Wl,-rpath -Wl,$(SANITIZER_DYLIB_PATH)

ifeq ($(Embedded),NO)
tsan: CFLAGS := $(filter-out $(ARCH_FLAGS),$(CFLAGS)) -arch x86_64 -fsanitize=thread
tsan: OTHER_LDFLAGS += -Wl,-rpath -Wl,$(SANITIZER_DYLIB_PATH)
else
EXCLUDED_SOURCES += tsan.c
endif

madvise: OTHER_CFLAGS += -I../src

.DEFAULT_GOAL := all

include $(DEVELOPER_DIR)/AppleInternal/Makefiles/darwintest/Makefile.targets
