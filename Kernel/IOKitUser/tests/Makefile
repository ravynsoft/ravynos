PROJECT := IOKitUser
TEST_DIR := tests

include $(DEVELOPER_DIR)/AppleInternal/Makefiles/darwintest/Makefile.common

OTHER_LDFLAGS += -framework IOKit

OSKext-tests: OTHER_LDFLAGS += -framework CoreFoundation

include $(DEVELOPER_DIR)/AppleInternal/Makefiles/darwintest/Makefile.targets
