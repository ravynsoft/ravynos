.PHONY: all installhdrs install $(DEVELOPER_DIR)/AppleInternal/Makefiles/darwintest/Makefile.targets $(DEVELOPER_DIR)/AppleInternal/Makefiles/darwintest/Makefile.common Makefile

PROJECT := Libnotify
TEST_DIR := tests/

DEVELOPER_DIR ?= /Applications/Xcode.app/Contents/Developer/

ifneq ($(wildcard $(DEVELOPER_DIR)/AppleInternal/Makefiles/darwintest/Makefile.common),)

include $(DEVELOPER_DIR)/AppleInternal/Makefiles/darwintest/Makefile.common

TARGETS := notify_benchmark notify_disable_test notify_many_dups dispatch_cancel_in_block notify_register_signal notify_qos parallel_register_cancel notify_regenerate notify_sigusr notify_pathwatch
OTHER_TARGETS :=
OTHER_LDFLAGS := -ldarwintest

include $(DEVELOPER_DIR)/AppleInternal/Makefiles/darwintest/Makefile.targets


# fall back if we can't find the Makefile to avoid an error in that case
else
all:
installhdrs:
install:
	mkdir -p $(DSTROOT)/AppleInternal/CoreOS/tests/$(PROJECT)
endif
