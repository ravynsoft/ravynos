
include MakefileVersion

MIN_MACOSX_VERSION=10.10
MAX_MACOSX_VERSION=MAC_OS_X_VERSION_10_10

OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))
OBJECTS += CFBasicHash.o
HFILES = $(wildcard *.h)
INTERMEDIATE_HFILES = $(addprefix $(OBJBASE)/CoreFoundation/,$(HFILES))

PUBLIC_HEADERS=CFArray.h CFBag.h CFBase.h CFBinaryHeap.h CFBitVector.h CFBundle.h CFByteOrder.h CFCalendar.h CFCharacterSet.h CFData.h CFDate.h CFDateFormatter.h CFDictionary.h CFError.h CFLocale.h CFMessagePort.h CFNumber.h CFNumberFormatter.h CFPlugIn.h CFPlugInCOM.h CFPreferences.h CFPropertyList.h CFRunLoop.h CFSet.h CFSocket.h CFStream.h CFString.h CFStringEncodingExt.h CFTimeZone.h CFTree.h CFURL.h CFURLAccess.h CFUUID.h CFUserNotification.h CFXMLNode.h CFXMLParser.h CFAvailability.h CFUtilities.h CoreFoundation.h

PRIVATE_HEADERS=CFBundlePriv.h CFCharacterSetPriv.h CFError_Private.h CFLogUtilities.h CFPriv.h CFRuntime.h CFStorage.h CFStreamAbstract.h CFStreamPriv.h CFStreamInternal.h CFStringDefaultEncoding.h CFStringEncodingConverter.h CFStringEncodingConverterExt.h CFUniChar.h CFUnicodeDecomposition.h CFUnicodePrecomposition.h ForFoundationOnly.h CFBurstTrie.h CFICULogging.h

MACHINE_TYPE := $(shell uname -p)
unicode_data_file_name = $(if $(or $(findstring i386,$(1)),$(findstring i686,$(1)),$(findstring x86_64,$(1))),CFUnicodeData-L.mapping,CFUnicodeData-B.mapping)

OBJBASE_ROOT = CF-Objects
OBJBASE = $(OBJBASE_ROOT)/$(STYLE)
DSTBASE = $(if $(DSTROOT),$(DSTROOT)/System/Library/Frameworks,../CF-Root)

STYLE=normal
STYLE_CFLAGS=-O2
STYLE_LFLAGS=
ARCHFLAGS=-arch i386 -arch x86_64
INSTALLNAME=/System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation_$(STYLE)

CC = /usr/bin/clang

CFLAGS=-c -x c -pipe -std=gnu99 -Wmost -Wno-trigraphs -Wno-deprecated -mmacosx-version-min=$(MIN_MACOSX_VERSION) -fconstant-cfstrings -fexceptions -DCF_BUILDING_CF=1 -DDEPLOYMENT_TARGET_MACOSX=1 -DMAC_OS_X_VERSION_MAX_ALLOWED=$(MAX_MACOSX_VERSION) -DU_SHOW_DRAFT_API=1 -DU_SHOW_CPLUSPLUS_API=0 -I$(OBJBASE) -DVERSION=$(VERSION) -include CoreFoundation_Prefix.h

LFLAGS=-dynamiclib -mmacosx-version-min=$(MIN_MACOSX_VERSION) -twolevel_namespace -fexceptions -init ___CFInitialize -compatibility_version 150 -current_version $(VERSION) -Wl,-alias_list,SymbolAliases -sectcreate __UNICODE __csbitmaps CFCharacterSetBitmaps.bitmap -sectcreate __UNICODE __properties CFUniCharPropertyDatabase.data -sectcreate __UNICODE __data $(call unicode_data_file_name,$(MACHINE_TYPE)) -segprot __UNICODE r r


.PHONY: all install clean
.PRECIOUS: $(OBJBASE)/CoreFoundation/%.h

all: install

clean:
	-/bin/rm -rf $(OBJBASE_ROOT)

$(OBJBASE)/CoreFoundation:
	/bin/mkdir -p $(OBJBASE)/CoreFoundation

$(OBJBASE)/CoreFoundation/%.h: %.h $(OBJBASE)/CoreFoundation
	/bin/cp $< $@

$(OBJBASE)/%.o: %.c $(INTERMEDIATE_HFILES)
	$(CC) $(STYLE_CFLAGS) $(ARCHFLAGS) $(CFLAGS) $< -o $@

$(OBJBASE)/%.o: %.m $(INTERMEDIATE_HFILES)
	$(CC) $(STYLE_CFLAGS) $(ARCHFLAGS) $(CFLAGS) $< -o $@

$(OBJBASE)/CoreFoundation_$(STYLE): $(addprefix $(OBJBASE)/,$(OBJECTS))
	$(CC) $(STYLE_LFLAGS) -install_name $(INSTALLNAME) $(ARCHFLAGS) $(LFLAGS) $^ -licucore.A -o $(OBJBASE)/CoreFoundation_$(STYLE)

install: $(OBJBASE)/CoreFoundation_$(STYLE)
	/bin/rm -rf $(DSTBASE)/CoreFoundation.framework
	/bin/mkdir -p $(DSTBASE)/CoreFoundation.framework/Versions/A/Resources
	/bin/mkdir -p $(DSTBASE)/CoreFoundation.framework/Versions/A/Headers
	/bin/mkdir -p $(DSTBASE)/CoreFoundation.framework/Versions/A/PrivateHeaders
	/bin/ln -sf A $(DSTBASE)/CoreFoundation.framework/Versions/Current
	/bin/ln -sf Versions/Current/Resources $(DSTBASE)/CoreFoundation.framework/Resources
	/bin/ln -sf Versions/Current/Headers $(DSTBASE)/CoreFoundation.framework/Headers
	/bin/ln -sf Versions/Current/PrivateHeaders $(DSTBASE)/CoreFoundation.framework/PrivateHeaders
	/bin/ln -sf Versions/Current/CoreFoundation $(DSTBASE)/CoreFoundation.framework/CoreFoundation
	/bin/cp Info.plist $(DSTBASE)/CoreFoundation.framework/Versions/A/Resources
	/bin/mkdir -p $(DSTBASE)/CoreFoundation.framework/Versions/A/Resources/en.lproj
	/bin/cp $(PUBLIC_HEADERS) $(DSTBASE)/CoreFoundation.framework/Versions/A/Headers
	/bin/cp $(PRIVATE_HEADERS) $(DSTBASE)/CoreFoundation.framework/Versions/A/PrivateHeaders
	#/usr/bin/strip -S -o $(DSTBASE)/CoreFoundation.framework/Versions/A/CoreFoundation $(OBJBASE)/CoreFoundation_$(STYLE)
	/bin/cp $(OBJBASE)/CoreFoundation_$(STYLE) $(DSTBASE)/CoreFoundation.framework/Versions/A/CoreFoundation
	/usr/bin/dsymutil $(DSTBASE)/CoreFoundation.framework/Versions/A/CoreFoundation -o $(DSTBASE)/CoreFoundation.framework.dSYM
	/usr/sbin/chown -RH -f root:wheel $(DSTBASE)/CoreFoundation.framework
	/bin/chmod -RH a-w,a+rX $(DSTBASE)/CoreFoundation.framework
	/bin/chmod -RH u+w $(DSTBASE)
	install_name_tool -id /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation $(DSTBASE)/CoreFoundation.framework/Versions/A/CoreFoundation
	@echo "Installing done.  The framework is in $(DSTBASE)"

