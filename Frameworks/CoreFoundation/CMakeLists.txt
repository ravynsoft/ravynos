add_framework_bundle(CoreFoundation
    VERSION A
    INFO_PLIST Info.plist
    HEADERS CFArray.h CFBag.h CFBase.h CFBinaryHeap.h CFBitVector.h CFBundle.h
            CFByteOrder.h CFCalendar.h CFCharacterSet.h CFData.h CFDate.h
            CFDateFormatter.h CFDictionary.h CFError.h CFLocale.h CFMessagePort.h
            CFNumber.h CFNumberFormatter.h CFPlugIn.h CFPlugInCOM.h CFPreferences.h
            CFPropertyList.h CFRunLoop.h CFSet.h CFSocket.h CFStream.h CFString.h
            CFStringEncodingExt.h CFTimeZone.h CFTree.h CFURL.h CFURLAccess.h
            CFUUID.h CFUserNotification.h CFXMLNode.h CFXMLParser.h CFAvailability.h
            CFUtilities.h CoreFoundation.h
    PRIVATE_HEADERS CFBundlePriv.h CFCharacterSetPriv.h CFError_Private.h
                    CFLogUtilities.h CFPriv.h CFRuntime.h CFStorage.h
                    CFStreamAbstract.h CFStreamPriv.h CFStreamInternal.h
                    CFStringDefaultEncoding.h CFStringEncodingConverter.h
                    CFStringEncodingConverterExt.h CFUniChar.h
                    CFUnicodeDecomposition.h CFUnicodePrecomposition.h
                    ForFoundationOnly.h CFBurstTrie.h CFICULogging.h
    INSTALL_NAME /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation
)

set_target_properties(CoreFoundation PROPERTIES COMPILE_LANGUAGE CXX)
set(VERSION 1153.18)

target_sources(CoreFoundation PRIVATE
    CFApplicationPreferences.c
    CFArray.c
    CFBag.c
    CFBase.c
    CFBasicHash.c
    CFBigNumber.c
    CFBinaryHeap.c
    CFBinaryPList.c
    CFBitVector.c
    CFBuiltinConverters.c
    CFBundle.c
    CFBundle_Binary.c
    CFBundle_Grok.c
    CFBundle_InfoPlist.c
    CFBundle_Locale.c
    CFBundle_Resources.c
    CFBundle_Strings.c
    CFBurstTrie.c
    CFCalendar.c
    CFCharacterSet.c
    CFConcreteStreams.c
    CFData.c
    CFDate.c
    CFDateFormatter.c
    CFDictionary.c
    CFError.c
    CFFileUtilities.c
    CFICUConverters.c
    CFLocale.c
    CFLocaleIdentifier.c
    CFLocaleKeys.c
    CFMachPort.c
    CFMessagePort.c
    CFNumber.c
    CFNumberFormatter.c
    CFOldStylePList.c
    CFPlatform.c
    CFPlatformConverters.c
    CFPlugIn.c
    CFPlugIn_Factory.c
    CFPlugIn_Instance.c
    CFPlugIn_PlugIn.c
    CFPreferences.c
    CFPropertyList.c
    CFRunLoop.c
    CFRuntime.c
    CFSet.c
    CFSocket.c
    CFSocketStream.c
    CFSortFunctions.c
    CFStorage.c
    CFStream.c
    CFString.c
    CFStringEncodingConverter.c
    CFStringEncodingDatabase.c
    CFStringEncodings.c
    CFStringScanner.c
    CFStringUtilities.c
    CFSystemDirectories.c
    CFTimeZone.c
    CFTree.c
    CFURL.c
    CFURLAccess.c
    CFUUID.c
    CFUniChar.c
    CFUnicodeDecomposition.c
    CFUnicodePrecomposition.c
    CFUserNotification.c
    CFUtilities.c
    CFVersion.c
    CFWindowsUtilities.c
    CFXMLInputStream.c
    CFXMLNode.c
    CFXMLParser.c
    CFXMLPreferencesDomain.c
    CFXMLTree.c
)

#add_executable(plconvert RESOURCES plconvert.c)
#add_dependencies(plconvert CoreFoundation)

if(CpuArch MATCHES x86_64)  # Apple used "x86" as shorthand for "little endian" here, so extend as needed
    set(unicode_data_file_name CFUnicodeData-L.mapping)
else()
    set(unicode_data_file_name CFUnicodeData-B.mapping)
endif()

target_compile_definitions(CoreFoundation PRIVATE
    CF_BUILDING_CF U_SHOW_CPLUSPLUS_API U_SHOW_DRAFT_API DEPLOYMENT_TARGET_MACOSX
    "MAC_OS_X_VERSION_MAX_ALLOWED=0x0D0100" VERSION=${VERSION})

target_include_directories(CoreFoundation PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR} ${ROOT_SOURCE_DIR}/Frameworks
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/liblaunch
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libdispatch/private
    ${ROOT_SOURCE_DIR}/Libraries/openbsm
    ${ROOT_SOURCE_DIR}/Libraries/ICU/icuSources/i18n
    ${ROOT_SOURCE_DIR}/Libraries/ICU/icuSources/common
    ${RAVYN_SDKROOT}/usr/local/include/kernel)

target_compile_options(CoreFoundation PRIVATE
     -mmacos-version-min=${MACOS_VERSION_MIN} -isysroot${RAVYN_SDKROOT}
    -fconstant-cfstrings -fexceptions
    -include${CMAKE_CURRENT_SOURCE_DIR}/CoreFoundation_Prefix.h)

target_link_options(CoreFoundation PRIVATE
    -Wl,-dynamiclib -mmacosx-version-min=${MACOSX_VERSION_MIN} -fexceptions
    -Wl,-init,___CFInitialize -Wl,-compatibility_version,150
    -Wl,-current_version,${VERSION} -Wl,-alias_list,${CMAKE_CURRENT_SOURCE_DIR}/SymbolAliases
    -Wl,-sectcreate,__UNICODE,__csbitmaps,CFCharacterSetBitmaps.bitmap
    -Wl,-sectcreate,__UNICODE,__properties,CFUniCharPropertyDatabase.data
    -Wl,-sectcreate,__UNICODE,__data,${unicode_data_file_name}
    -Wl,-segprot,__UNICODE,r,r)


add_custom_command(TARGET CoreFoundation POST_BUILD
    COMMAND ${DSYMUTIL} ${CMAKE_CURRENT_BINARY_DIR}/CoreFoundation.framework/Versions/A/CoreFoundation
        -o ${CMAKE_CURRENT_BINARY_DIR}/CoreFoundation.framework.dSYM)
