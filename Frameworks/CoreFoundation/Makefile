MK_WERROR = no

FRAMEWORK = CoreFoundation
FMWK_VERSION = A
INFO_PLIST = Info.plist

HEADERS = CFArray.h CFBag.h CFBase.h CFBinaryHeap.h CFBitVector.h CFBundle.h \
	CFByteOrder.h CFCalendar.h CFCharacterSet.h CFData.h CFDate.h \
	CFDateFormatter.h CFDictionary.h CFError.h CFLocale.h CFMessagePort.h \
	CFNumber.h CFNumberFormatter.h CFPlugIn.h CFPlugInCOM.h \
	CFPreferences.h CFPropertyList.h CFRunLoop.h CFSet.h CFSocket.h \
	CFStream.h CFString.h CFStringEncodingExt.h CFTimeZone.h CFTree.h \
	CFURL.h CFURLAccess.h CFUUID.h CFUserNotification.h CFXMLNode.h \
	CFXMLParser.h CFAvailability.h CFUtilities.h CoreFoundation.h

PRIVATE_HEADERS = CFBundlePriv.h CFCharacterSetPriv.h CFError_Private.h \
	CFLogUtilities.h CFPriv.h CFRuntime.h CFStorage.h CFStreamAbstract.h \
	CFStreamPriv.h CFStreamInternal.h CFStringDefaultEncoding.h \
	CFStringEncodingConverter.h CFStringEncodingConverterExt.h \
	CFUniChar.h CFUnicodeDecomposition.h CFUnicodePrecomposition.h \
	ForFoundationOnly.h CFBurstTrie.h CFICULogging.h

SLF = /System/Library/Frameworks
INSTALL_NAME_DIR = ${SLF}/${FRAMEWORK}.framework/Versions/${FMWK_VERSION}

.include "MakefileVersion"

SRCS = CFApplicationPreferences.c \
	CFArray.c \
	CFBag.c \
	CFBase.c \
	CFBasicHash.c \
	CFBigNumber.c \
	CFBinaryHeap.c \
	CFBinaryPList.c \
	CFBitVector.c \
	CFBuiltinConverters.c \
	CFBundle.c \
	CFBundle_Binary.c \
	CFBundle_Grok.c \
	CFBundle_InfoPlist.c \
	CFBundle_Locale.c \
	CFBundle_Resources.c \
	CFBundle_Strings.c \
	CFBurstTrie.c \
	CFCalendar.c \
	CFCharacterSet.c \
	CFConcreteStreams.c \
	CFData.c \
	CFDate.c \
	CFDateFormatter.c \
	CFDictionary.c \
	CFError.c \
	CFFileUtilities.c \
	CFICUConverters.c \
	CFLocale.c \
	CFLocaleIdentifier.c \
	CFLocaleKeys.c \
	CFMachPort.c \
	CFMessagePort.c \
	CFNumber.c \
	CFNumberFormatter.c \
	CFOldStylePList.c \
	CFPlatform.c \
	CFPlatformConverters.c \
	CFPlugIn.c \
	CFPlugIn_Factory.c \
	CFPlugIn_Instance.c \
	CFPlugIn_PlugIn.c \
	CFPreferences.c \
	CFPropertyList.c \
	CFRunLoop.c \
	CFRuntime.c \
	CFSet.c \
	CFSocket.c \
	CFSocketStream.c \
	CFSortFunctions.c \
	CFStorage.c \
	CFStream.c \
	CFString.c \
	CFStringEncodingConverter.c \
	CFStringEncodingDatabase.c \
	CFStringEncodings.c \
	CFStringScanner.c \
	CFStringUtilities.c \
	CFSystemDirectories.c \
	CFTimeZone.c \
	CFTree.c \
	CFURL.c \
	CFURLAccess.c \
	CFUUID.c \
	CFUniChar.c \
	CFUnicodeDecomposition.c \
	CFUnicodePrecomposition.c \
	CFUserNotification.c \
	CFUtilities.c \
	CFVersion.c \
	CFWindowsUtilities.c \
	CFXMLInputStream.c \
	CFXMLNode.c \
	CFXMLParser.c \
	CFXMLPreferencesDomain.c \
	CFXMLTree.c

#add_executable(plconvert RESOURCES plconvert.c)
#add_dependencies(plconvert CoreFoundation)

# Apple used "x86" as shorthand for "little endian" here, so extend as needed
.if "${CpuArch}" == "x86_64"
unicode_data_file_name = CFUnicodeData-L.mapping
.else
unicode_data_file_name = CFUnicodeData-B.mapping
.endif

CFLAGS = -DCF_BUILDING_CF -DU_SHOW_CPLUSPLUS_API -DU_SHOW_DRAFT_API \
	-DDEPLOYMENT_TARGET_MACOSX -D"MAC_OS_X_VERSION_MAX_ALLOWED=0x0D0100" \
	-DVERSION=${VERSION} -DTARGET_OS_BRIDGE=0 -fconstant-cfstrings \
	-fexceptions -Wno-implicit-function-declaration \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${.OBJDIR} -I${ROOT_SOURCE_DIR}/Frameworks \
	-I${ROOT_SOURCE_DIR}/Libraries/Libsystem/liblaunch \
	-I${ROOT_SOURCE_DIR}/Libraries/Libsystem/libdispatch/private \
	-I${ROOT_SOURCE_DIR}/Libraries/openbsm \
	-I${ROOT_SOURCE_DIR}/Libraries/ICU/icuSources/i18n \
	-I${ROOT_SOURCE_DIR}/Libraries/ICU/icuSources/common \
	-I${ROOT_SOURCE_DIR}/Libraries/Libsystem/removefile \
	-I${ROOT_SOURCE_DIR}/Libraries/Libsystem/private \
	-I${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_blocks \
	-I${RAVYN_SDKROOT}/usr/local/include/kernel ${CFLAGS.${.TARGET:T}} \
	-include${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk/mach/error.h \
	-include${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/mach/mach/mach.h \
	-include${.CURDIR}/CoreFoundation_Prefix.h

#-include${ROOT_SOURCE_DIR}/Libraries/Libsystem/liblaunch/bootstrap.h

LDFLAGS = -Wl,-dynamiclib -fexceptions -Wl,-init,___CFInitialize \
	-Wl,-compatibility_version,150 -Wl,-current_version,${VERSION} \
	-Wl,-alias_list,${.CURDIR}/SymbolAliases \
	-Wl,-sectcreate,__UNICODE,__csbitmaps,CFCharacterSetBitmaps.bitmap \
	-Wl,-sectcreate,__UNICODE,__properties,CFUniCharPropertyDatabase.data \
	-Wl,-sectcreate,__UNICODE,__data,${unicode_data_file_name} \
	-Wl,-segprot,__UNICODE,r,r

${.OBJDIR}/mach-o/arch.h:
	mkdir -p ${.OBJDIR}/mach-o
	cp -f ${DEVEL}/Default.xctoolchain/include/mach-o/arch.h ${.TARGET}

${FRAMEWORK}: ${.OBJDIR}/mach-o/arch.h

FRAMEWORK_LIB = ${.OBJDIR}/${FRAMEWORK}.framework/Versions/${FMWK_VERSION}/${FRAMEWORK}
${FRAMEWORK}.framework.dSYM: ${FRAMEWORK}
	${DSYMUTIL} ${FRAMEWORK_LIB} -o ${.TARGET}

.include <rvn.framework.mk>
