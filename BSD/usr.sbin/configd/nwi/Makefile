ifeq ($(PLATFORM),iphoneos)
# iOS internal SDK
ARCHS=armv7
endif

ifeq ($(PLATFORM),)
PLATFORM=macosx
endif

ifeq ($(PLATFORM),macosx)
# Mac OS X internal SDK
ARCHS=x86_64
endif

# Mac OS X or iOS internal SDK
SDK=$(PLATFORM).internal
SYSROOT=$(shell xcodebuild -version -sdk $(SDK) Path)
CC = xcrun -sdk $(SDK) cc
PF_INC = -F$(SYSROOT)/System/Library/PrivateFrameworks

ARCH_FLAGS=$(foreach a,$(ARCHS),-arch $(a))
EXTRA_CFLAGS=
TEST_INCLUDE=-I. -I../dnsinfo -I../libSystemConfiguration -I../SystemConfiguration.fproj -I$(SYSROOT)/System/Library/Frameworks/System.framework/PrivateHeaders

REFERENCE_OUTPUT=../common/reference_output.sh

clean:
	rm -rf test_nwi *.o *.dSYM

test_nwi:network_information.c network_information_priv.c ../libSystemConfiguration/libSystemConfiguration_client.c
	$(CC) -DTEST_NWI $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) $(TEST_INCLUDE) $(EXTRA_CFLAGS) -Wall -O0 -g -o $@ $^

test_nwi_reference.txt: test_nwi
	sh $(REFERENCE_OUTPUT) create test_nwi test_nwi_reference.txt /bin/cat

test_nwi_test: test_nwi
	sh $(REFERENCE_OUTPUT) test test_nwi test_nwi_reference.txt /bin/cat

nwi:network_information.c network_information_priv.c ../libSystemConfiguration/libSystemConfiguration_client.c
	$(CC) -DTEST_NWI_STATE $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) $(TEST_INCLUDE) $(EXTRA_CFLAGS) -Wall -O0 -g -o $@ $^
