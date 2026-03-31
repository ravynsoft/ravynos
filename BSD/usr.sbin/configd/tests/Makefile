PLATFORM=iphoneos

ifeq ($(PLATFORM),iphoneos)
ARCHS=armv7
endif

ifeq ($(PLATFORM),macosx)
ARCHS=i386 x86_64
endif

FRAMEWORKS=CoreFoundation SystemConfiguration CFNetwork
SDK=$(PLATFORM).internal
SYSROOT=$(shell xcodebuild -version -sdk $(SDK) Path)
PF_INC=-F$(SYSROOT)/System/Library/PrivateFrameworks
ARCH_FLAGS=$(foreach a,$(ARCHS),-arch $(a))
FW_FLAGS=$(foreach f,$(FRAMEWORKS),-framework $(f))
CC=xcrun -sdk $(SDK) cc

ReachabilityTester : ReachabilityTester.c
	$(CC) $(ARCH_FLAGS) -isysroot $(SYSROOT) $(PF_INC) $(FW_FLAGS) -g -o $@ $<
	tar -czf $@.tgz $@ $@.dSYM $<

clean :
	rm -rf ReachabilityTester ReachabilityTester.dSYM ReachabilityTester.tgz
