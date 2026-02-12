ifeq ($(PLATFORM),)
PLATFORM=macosx
endif

ifeq ($(PLATFORM),iphoneos)
# iOS internal SDK
ARCHS=armv7
endif

ifeq ($(PLATFORM),macosx)
# Mac OS X internal SDK
ARCHS=x86_64
endif

# Mac OS X or iOS internal SDK
SDK=$(PLATFORM).internal
SYSROOT=$(shell xcodebuild -version -sdk $(SDK) Path)
CC = xcrun -sdk $(SDK) cc

all : qos-marking

qos-marking.o: qos-marking.m Makefile
	$(CC) -DOS_ACTIVITY_OBJECT_API=1 -I../../SystemConfiguration.fproj -I${SYSROOT}/System/Library/Frameworks/System.framework/PrivateHeaders -Wall -g -DMAIN -O0 -c qos-marking.m

qos-marking: qos-marking.o Makefile
	$(CC) -o qos-marking qos-marking.o -framework CoreFoundation -framework Foundation -framework SystemConfiguration -framework NetworkExtension

clean:
	rm -rf *.o qos-marking qos-marking.dSYM

