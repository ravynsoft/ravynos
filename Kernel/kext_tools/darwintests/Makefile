PROJECT := kext_tools
TEST_DIR := darwintests/

ifeq ($(DEVELOPER_DIR),)
	DEVELOPER_DIR := $(shell xcode-select -p)
endif

include $(DEVELOPER_DIR)/AppleInternal/Makefiles/darwintest/Makefile.common

BATS_PLISTS= $(SRCROOT)/../tests/kext_tools.plist ${BUILT_PRODUCTS_DIR}/kextaudit_darwintest.plist

include $(DEVELOPER_DIR)/AppleInternal/Makefiles/darwintest/Makefile.targets
