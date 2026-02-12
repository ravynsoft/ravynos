
ifeq ($(PLATFORM),)
PLATFORM=macosx
endif

ifeq ($(PLATFORM),iphoneos)
# iOS internal SDK
ARCHS=arm64
endif

ifeq ($(PLATFORM),macosx)
# Mac OS X internal SDK
ARCHS=x86_64
endif


ifeq ($(USER),ajn)
  ifeq ($(PLATFORM),macosx)
    PLATFORM=macosx$(OSX_VERSION)
    EXTRA=-target $(ARCHS)-apple-macos$(OSX_VERSION) -F${BUILDIT_DIR}/configd-999.roots/Shared/SDKContentRoot/osx/System/Library/Frameworks
  endif
  ifeq ($(PLATFORM),iphoneos)
    PLATFORM=ios$(EMBEDDED_VERSION)
    EXTRA=-target $(ARCHS)-apple-ios$(EMBEDDED_VERSION) -F${BUILDIT_DIR}_Embedded/configd-999.roots/Shared/SDKContentRoot/ios/System/Library/Frameworks
  endif
  EXTRA+=-DDEBUG
endif


# Mac OS X or iOS internal SDK
SDK=$(PLATFORM)internal
SYSROOT=$(shell xcodebuild -version -sdk $(SDK) Path)
CC = xcrun -sdk $(SDK) cc


all : ifnamer snapshot

# ---------- ---------- ---------- ---------- ----------

InterfaceNamerControlPrefs.o : ../../Plugins/common/InterfaceNamerControlPrefs.c ../../Plugins/common/InterfaceNamerControlPrefs.h Makefile
	$(CC) -DOS_ACTIVITY_OBJECT_API=1 -I../../SystemConfiguration.fproj -I../../IPMonitorControl -I../../Plugins/common ${EXTRA} -I${SYSROOT}/System/Library/Frameworks/System.framework/PrivateHeaders -Wall -g -O0 -c ../../Plugins/common/InterfaceNamerControlPrefs.c

# ---------- ---------- ---------- ---------- ----------

ifnamer.o: ifnamer.c Makefile
	$(CC) -DOS_ACTIVITY_OBJECT_API=1 -I../../SystemConfiguration.fproj -I../../IPMonitorControl -I../../Plugins/common ${EXTRA} -I${SYSROOT}/System/Library/Frameworks/System.framework/PrivateHeaders -Wall -g -DTEST_INTERFACE_ASSIGNMENT -O0 -c ifnamer.c

ifnamer: ifnamer.o InterfaceNamerControlPrefs.o Makefile
	$(CC) -o ifnamer ifnamer.o InterfaceNamerControlPrefs.o ${EXTRA} -framework CoreFoundation -framework SystemConfiguration -framework IOKit

# ---------- ---------- ---------- ---------- ----------

snapshot.o: ifnamer.c Makefile
	$(CC) -DOS_ACTIVITY_OBJECT_API=1 -I../../SystemConfiguration.fproj -I../../IPMonitorControl -I../../Plugins/common ${EXTRA} -I${SYSROOT}/System/Library/Frameworks/System.framework/PrivateHeaders -Wall -g -DTEST_SNAPSHOT -O0 -c -o $@ ifnamer.c

snapshot: snapshot.o InterfaceNamerControlPrefs.o Makefile
	$(CC) -o snapshot snapshot.o InterfaceNamerControlPrefs.o ${EXTRA} -framework CoreFoundation -framework SystemConfiguration -framework IOKit

# ---------- ---------- ---------- ---------- ----------

clean:
	rm -rf *.o ifnamer ifnamer.dSYM snapshot snapshot.dSYM

