# Default platform uses the native SDK.
# To build for Mac OS X using internal SDK, use 'make PLATFORM=macosx <target>'
# To build for iOS, use 'make PLATFORM=iphoneos <target>'

ifeq ($(PLATFORM),iphoneos)
# iOS internal SDK
CORETELEPHONY=-framework CoreTelephony
ARCHS=arm64
endif

ifeq ($(PLATFORM),macosx)
# Mac OS X internal SDK
ARCHS=i386 x86_64
CORETELEPHONY=
endif

ifeq ($(PLATFORM),)
# Mac OS X native SDK
ARCHS=x86_64
CORETELEPHONY=
CC = cc
SYSROOT = /
PF_INC = -F/System/Library/PrivateFrameworks
else
# Mac OS X or iOS internal SDK
SDK=$(PLATFORM).internal
SYSROOT=$(shell xcodebuild -version -sdk $(SDK) Path)
CC = xcrun -sdk $(SDK) cc
PF_INC = -F$(SYSROOT)/System/Library/PrivateFrameworks
endif

ARCH_FLAGS=$(foreach a,$(ARCHS),-arch $(a))

if_rank_assert: IPMonitorControlServer.c IPMonitorControl.c main.c
	$(CC) -DTEST_IPMONITOR_CONTROL -I$(SYSROOT)/System/Library/Frameworks/System.framework/PrivateHeaders $(ARCH_FLAGS) -isysroot $(SYSROOT) $(PF_INC) -framework CoreFoundation -framework SystemConfiguration -Wall -g -o $@ $^

IPMonitorAWDReportTest: IPMonitorAWDReport.m
	$(CC) -isysroot $(SYSROOT) $(ARCH_FLAGS) -g  -Wall -DTEST_IPMONITOR_AWD_REPORT -framework Foundation -weak_framework WirelessDiagnostics -framework CoreFoundation -framework SystemConfiguration -framework ProtocolBuffer $(PF_INC) -I AWD AWD/AWDIPMonitorInterfaceAdvisoryReport.m -o $@ $^

clean:
	rm -rf *.dSYM *~ *.o if_rank_assert
