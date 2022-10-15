
cmake_minimum_required(VERSION 3.12)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

project(CoreFoundation
  VERSION 1338
  LANGUAGES ASM C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED YES)

# TODO(compnerd): this should be re-enabled once CoreFoundation annotations have
# been reconciled.  Hidden visibility should ensure that internal interfaces are
# not accidentally exposed on platforms without exports lists and explicit
# annotations (i.e. ELFish and MachO targets).
if(0)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_INLINES_HIDDEN ON)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE YES)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG OFF)
find_package(Threads REQUIRED)

if(NOT CMAKE_SYSTEM_NAME STREQUAL Darwin)
  find_package(LibXml2 REQUIRED)
  find_package(CURL CONFIG)
  if(CURL_FOUND)
    set(CURL_VERSION_STRING ${CURL_VERSION})
  else()
    find_package(CURL REQUIRED)
  endif()
endif()

include(GNUInstallDirs)
include(CoreFoundationAddFramework)

option(CF_DEPLOYMENT_SWIFT "Build for swift" NO)

if("${CMAKE_C_SIMULATE_ID}" STREQUAL "MSVC")
  add_compile_options($<$<COMPILE_LANGUAGE:C>:/FI${PROJECT_SOURCE_DIR}/Base.subproj/CoreFoundation_Prefix.h>)
else()
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-include$<SEMICOLON>${PROJECT_SOURCE_DIR}/Base.subproj/CoreFoundation_Prefix.h>)
endif()

add_compile_options($<$<COMPILE_LANGUAGE:C>:-fblocks>)

if("${CMAKE_C_SIMULATE_ID}" STREQUAL "MSVC")
  add_compile_options($<$<COMPILE_LANGUAGE:C>:/EHsc>)
else()
  if(NOT CMAKE_SYSTEM_NAME STREQUAL OpenBSD)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-fexceptions>)
  endif()
endif()

if(NOT "${CMAKE_C_SIMULATE_ID}" STREQUAL "MSVC")
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-fconstant-cfstrings>)
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-fdollars-in-identifiers>)
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-fno-common>)
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-Werror=implicit-function-declaration>)
endif()

if(CF_DEPLOYMENT_SWIFT)
  add_compile_options($<$<COMPILE_LANGUAGE:C>:$<$<STREQUAL:${CMAKE_C_SIMULATE_ID},MSVC>:/clang:>-fcf-runtime-abi=swift>)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Linux OR CMAKE_SYSTEM_NAME STREQUAL Android)
  add_compile_definitions($<$<COMPILE_LANGUAGE:C>:_GNU_SOURCE>)

  if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    include(CheckSymbolExists)
    include(CheckIncludeFile)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
    check_include_file("sched.h" HAVE_SCHED_H)
    if(HAVE_SCHED_H)
      check_symbol_exists(sched_getaffinity "sched.h" HAVE_SCHED_GETAFFINITY)
      if(HAVE_SCHED_GETAFFINITY)
        add_compile_definitions($<$<COMPILE_LANGUAGE:C>:HAVE_SCHED_GETAFFINITY>)
      endif()
    endif()
  endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL Windows)
  # NOTE(compnerd) we only support building with the dynamic CRT as using the
  # static CRT causes problems for users of the library.
  add_compile_definitions($<$<COMPILE_LANGUAGE:C>:_DLL>)
  if(BUILD_SHARED_LIBS)
    add_compile_definitions($<$<COMPILE_LANGUAGE:C>:_WINDLL)
  endif()
endif()

add_compile_definitions($<$<COMPILE_LANGUAGE:C>:U_SHOW_DRAFT_API>)
add_compile_definitions($<$<COMPILE_LANGUAGE:C>:CF_BUILDING_CF>)

if(CF_DEPLOYMENT_SWIFT)
  add_compile_definitions($<$<COMPILE_LANGUAGE:C>:DEPLOYMENT_RUNTIME_SWIFT>)
else()
  add_compile_definitions($<$<COMPILE_LANGUAGE:C>:DEPLOYMENT_RUNTIME_C>)
endif()

# TODO(compnerd) ensure that the compiler supports the warning flag
add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-shorten-64-to-32>)
add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-deprecated-declarations>)
add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-unreachable-code>)
add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-conditional-uninitialized>)
add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-unused-variable>)
add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-int-conversion>)
add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-unused-function>)
add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-microsoft-enum-forward-reference>)

if(BUILD_SHARED_LIBS)
  set(FRAMEWORK_LIBRARY_TYPE SHARED)
else()
  set(FRAMEWORK_LIBRARY_TYPE STATIC)
endif()

add_framework(CoreFoundation
                ${FRAMEWORK_LIBRARY_TYPE}
              FRAMEWORK_DIRECTORY
                CoreFoundation_FRAMEWORK_DIRECTORY
              MODULE_MAP
                Base.subproj/module.modulemap
              PRIVATE_HEADERS
                # Base
                Base.subproj/CFAsmMacros.h
                Base.subproj/CFInternal.h
                Base.subproj/CFKnownLocations.h
                Base.subproj/CFLogUtilities.h
                Base.subproj/CFPriv.h
                Base.subproj/CFOverflow.h
                Base.subproj/CFRuntime.h
                Base.subproj/CFRuntime_Internal.h
                Base.subproj/ForFoundationOnly.h
                Base.subproj/ForSwiftFoundationOnly.h
                # Collections
                Collections.subproj/CFBasicHash.h
                Collections.subproj/CFStorage.h
                Collections.subproj/CFCollections_Internal.h
                # Error
                Error.subproj/CFError_Private.h
                # Locale
                Locale.subproj/CFCalendar_Internal.h
                Locale.subproj/CFDateComponents.h
                Locale.subproj/CFDateFormatter_Private.h
                Locale.subproj/CFDateIntervalFormatter.h
                Locale.subproj/CFDateInterval.h
                Locale.subproj/CFICULogging.h
                Locale.subproj/CFListFormatter.h
                Locale.subproj/CFLocaleInternal.h
                Locale.subproj/CFLocale_Private.h
                Locale.subproj/CFRelativeDateTimeFormatter.h
                # NumberDate
                NumberDate.subproj/CFBigNumber.h
                NumberDate.subproj/CFNumber_Private.h
                # Parsing
                Parsing.subproj/CFPropertyList_Private.h
                # PlugIn
                PlugIn.subproj/CFBundlePriv.h
                PlugIn.subproj/CFBundle_BinaryTypes.h
                PlugIn.subproj/CFBundle_Internal.h
                PlugIn.subproj/CFPlugIn_Factory.h
                # RunLoop
                RunLoop.subproj/CFMachPort_Internal.h
                RunLoop.subproj/CFMachPort_Lifetime.h
                # Stream
                Stream.subproj/CFStreamAbstract.h
                Stream.subproj/CFStreamInternal.h
                Stream.subproj/CFStreamPriv.h
                # String
                String.subproj/CFAttributedStringPriv.h
                String.subproj/CFBurstTrie.h
                String.subproj/CFCharacterSetPriv.h
                String.subproj/CFRegularExpression.h
                String.subproj/CFRunArray.h
                String.subproj/CFString_Internal.h
                String.subproj/CFString_Private.h
                String.subproj/CFStringDefaultEncoding.h
                String.subproj/CFStringLocalizedFormattingInternal.h
                # StringEncodings
                StringEncodings.subproj/CFICUConverters.h
                StringEncodings.subproj/CFStringEncodingConverter.h
                StringEncodings.subproj/CFStringEncodingConverterExt.h
                StringEncodings.subproj/CFStringEncodingConverterPriv.h
                StringEncodings.subproj/CFStringEncodingDatabase.h
                StringEncodings.subproj/CFUniChar.h
                StringEncodings.subproj/CFUniCharPriv.h
                StringEncodings.subproj/CFUnicodeDecomposition.h
                StringEncodings.subproj/CFUnicodePrecomposition.h
                # URL
                URL.subproj/CFURL.inc.h
                URL.subproj/CFURLPriv.h
                URL.subproj/CFURLSessionInterface.h
              PUBLIC_HEADERS
                # FIXME: PrivateHeaders referenced by public headers
                Base.subproj/CFKnownLocations.h
                Base.subproj/CFLocking.h
                Base.subproj/CFLogUtilities.h
                Base.subproj/CFPriv.h
                Base.subproj/CFRuntime.h
                Base.subproj/ForFoundationOnly.h
                Base.subproj/ForSwiftFoundationOnly.h
                Locale.subproj/CFCalendar_Internal.h
                Locale.subproj/CFDateComponents.h
                Locale.subproj/CFDateInterval.h
                Locale.subproj/CFLocaleInternal.h
                PlugIn.subproj/CFBundlePriv.h
                Stream.subproj/CFStreamPriv.h
                String.subproj/CFCharacterSetPriv.h
                String.subproj/CFRegularExpression.h
                String.subproj/CFRunArray.h
                StringEncodings.subproj/CFStringEncodingConverter.h
                StringEncodings.subproj/CFStringEncodingConverterExt.h
                URL.subproj/CFURLPriv.h
                URL.subproj/CFURLSessionInterface.h
                Locale.subproj/CFDateIntervalFormatter.h

                # AppServices
                AppServices.subproj/CFNotificationCenter.h
                AppServices.subproj/CFUserNotification.h
                # Base
                Base.subproj/CFAvailability.h
                Base.subproj/CFBase.h
                Base.subproj/CFByteOrder.h
                Base.subproj/CFUUID.h
                Base.subproj/CFUtilities.h
                Base.subproj/SwiftRuntime/CoreFoundation.h
                Base.subproj/SwiftRuntime/TargetConditionals.h
                # Collections
                Collections.subproj/CFArray.h
                Collections.subproj/CFBag.h
                Collections.subproj/CFBinaryHeap.h
                Collections.subproj/CFBitVector.h
                Collections.subproj/CFData.h
                Collections.subproj/CFDictionary.h
                Collections.subproj/CFSet.h
                Collections.subproj/CFTree.h
                # Error
                Error.subproj/CFError.h
                # Locale
                Locale.subproj/CFCalendar.h
                Locale.subproj/CFDateFormatter.h
                Locale.subproj/CFLocale.h
                Locale.subproj/CFNumberFormatter.h
                # NumberDate
                NumberDate.subproj/CFDate.h
                NumberDate.subproj/CFNumber.h
                NumberDate.subproj/CFTimeZone.h
                # Parsing
                Parsing.subproj/CFPropertyList.h
                # PlugIn
                PlugIn.subproj/CFBundle.h
                PlugIn.subproj/CFPlugIn.h
                PlugIn.subproj/CFPlugInCOM.h
                # Preferences
                Preferences.subproj/CFPreferences.h
                # RunLoop
                RunLoop.subproj/CFMachPort.h
                RunLoop.subproj/CFMessagePort.h
                RunLoop.subproj/CFRunLoop.h
                RunLoop.subproj/CFSocket.h
                # Stream
                Stream.subproj/CFStream.h
                # String
                String.subproj/CFAttributedString.h
                String.subproj/CFCharacterSet.h
                String.subproj/CFString.h
                String.subproj/CFStringEncodingExt.h
                # URL
                URL.subproj/CFURL.h
                URL.subproj/CFURLAccess.h
                URL.subproj/CFURLComponents.h
              SOURCES
                # Base
                Base.subproj/CFBase.c
                Base.subproj/CFFileUtilities.c
                Base.subproj/CFKnownLocations.c
                Base.subproj/CFPlatform.c
                Base.subproj/CFRuntime.c
                Base.subproj/CFSortFunctions.c
                Base.subproj/CFSystemDirectories.c
                Base.subproj/CFUtilities.c
                Base.subproj/CFUUID.c
                Base.subproj/CFWindowsUtilities.c
                # Collections
                Collections.subproj/CFArray.c
                Collections.subproj/CFBag.c
                Collections.subproj/CFBasicHash.c
                Collections.subproj/CFBinaryHeap.c
                Collections.subproj/CFBitVector.c
                Collections.subproj/CFData.c
                Collections.subproj/CFDictionary.c
                Collections.subproj/CFSet.c
                Collections.subproj/CFStorage.c
                Collections.subproj/CFTree.c
                # Error
                Error.subproj/CFError.c
                # Locale
                Locale.subproj/CFCalendar.c
                Locale.subproj/CFCalendar_Enumerate.c
                Locale.subproj/CFDateComponents.c
                Locale.subproj/CFDateFormatter.c
                Locale.subproj/CFDateIntervalFormatter.c
                Locale.subproj/CFDateInterval.c
                Locale.subproj/CFListFormatter.c
                Locale.subproj/CFLocale.c
                Locale.subproj/CFLocaleIdentifier.c
                Locale.subproj/CFLocaleKeys.c
                Locale.subproj/CFNumberFormatter.c
                Locale.subproj/CFRelativeDateTimeFormatter.c
                # NumberData
                NumberDate.subproj/CFBigNumber.c
                NumberDate.subproj/CFDate.c
                NumberDate.subproj/CFNumber.c
                NumberDate.subproj/CFTimeZone.c
                # Parsing
                Parsing.subproj/CFBinaryPList.c
                Parsing.subproj/CFOldStylePList.c
                Parsing.subproj/CFPropertyList.c
                # PlugIn
                PlugIn.subproj/CFBundle_Binary.c
                PlugIn.subproj/CFBundle.c
                PlugIn.subproj/CFBundle_DebugStrings.c
                PlugIn.subproj/CFBundle_Executable.c
                PlugIn.subproj/CFBundle_Grok.c
                PlugIn.subproj/CFBundle_InfoPlist.c
                PlugIn.subproj/CFBundle_Locale.c
                PlugIn.subproj/CFBundle_Main.c
                PlugIn.subproj/CFBundle_ResourceFork.c
                PlugIn.subproj/CFBundle_Resources.c
                PlugIn.subproj/CFBundle_SplitFileName.c
                PlugIn.subproj/CFBundle_Strings.c
                PlugIn.subproj/CFBundle_Tables.c
                PlugIn.subproj/CFPlugIn.c
                # Preferences
                Preferences.subproj/CFApplicationPreferences.c
                Preferences.subproj/CFPreferences.c
                Preferences.subproj/CFXMLPreferencesDomain.c
                # RunLoop
                # TODO(compnerd) make this empty on non-Mach targets
                # RunLoop.subproj/CFMachPort.c
                # RunLoop.subproj/CFMachPort_Lifetime.c
                # RunLoop.subproj/CFMessagePort.c
                RunLoop.subproj/CFRunLoop.c
                RunLoop.subproj/CFSocket.c
                # Stream
                Stream.subproj/CFConcreteStreams.c
                Stream.subproj/CFSocketStream.c
                Stream.subproj/CFStream.c
                # String
                String.subproj/CFAttributedString.c
                String.subproj/CFBurstTrie.c
                String.subproj/CFCharacterSet.c
                String.subproj/CFCharacterSetData.S
                String.subproj/CFRegularExpression.c
                String.subproj/CFRunArray.c
                String.subproj/CFString.c
                String.subproj/CFStringEncodings.c
                String.subproj/CFStringScanner.c
                String.subproj/CFStringTransform.c
                String.subproj/CFStringUtilities.c
                String.subproj/CFUniCharPropertyDatabase.S
                String.subproj/CFUnicodeData.S
                # StringEncodings
                StringEncodings.subproj/CFBuiltinConverters.c
                StringEncodings.subproj/CFICUConverters.c
                StringEncodings.subproj/CFPlatformConverters.c
                StringEncodings.subproj/CFStringEncodingConverter.c
                StringEncodings.subproj/CFStringEncodingDatabase.c
                StringEncodings.subproj/CFUniChar.c
                StringEncodings.subproj/CFUnicodeDecomposition.c
                StringEncodings.subproj/CFUnicodePrecomposition.c
                # URL
                URL.subproj/CFURLAccess.c
                URL.subproj/CFURL.c
                URL.subproj/CFURLComponents.c
                URL.subproj/CFURLComponents_URIParser.c)
target_compile_definitions(CoreFoundation
                           PRIVATE
                             $<$<COMPILE_LANGUAGE:ASM>:CF_CHARACTERSET_BITMAP="CharacterSets/CFCharacterSetBitmaps.bitmap">
                             $<$<COMPILE_LANGUAGE:ASM>:CF_CHARACTERSET_UNICHAR_DB="CharacterSets/CFUniCharPropertyDatabase.data">
                             $<$<COMPILE_LANGUAGE:ASM>:CF_CHARACTERSET_UNICODE_DATA_B="CharacterSets/CFUnicodeData-B.mapping">
                             $<$<COMPILE_LANGUAGE:ASM>:CF_CHARACTERSET_UNICODE_DATA_L="CharacterSets/CFUnicodeData-L.mapping">)
target_include_directories(CoreFoundation
                           PRIVATE
                             ${PROJECT_SOURCE_DIR})
target_link_libraries(CoreFoundation PRIVATE
  Threads::Threads
  ${CMAKE_DL_LIBS}
  BlocksRuntime
  dispatch)
if(CMAKE_SYSTEM_NAME STREQUAL Android)
  target_link_libraries(CoreFoundation PRIVATE
    log)
endif()
if(NOT CMAKE_SYSTEM_NAME STREQUAL Darwin)
  target_link_libraries(CoreFoundation PRIVATE
    ICU::uc
    ICU::i18n)
  if(ICU_DATA_FOUND)
    target_link_libraries(CoreFoundation PRIVATE
      ICU::data)
  endif()
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL Darwin)
  if((NS_CURL_ASSUME_FEATURES_MISSING) OR (CURL_VERSION_STRING VERSION_LESS "7.32.0"))
    add_compile_definitions($<$<COMPILE_LANGUAGE:C>:NS_CURL_MISSING_XFERINFOFUNCTION>)
  endif()

  if((NS_CURL_ASSUME_FEATURES_MISSING) OR (CURL_VERSION_STRING VERSION_LESS "7.30.0"))
    add_compile_definitions($<$<COMPILE_LANGUAGE:C>:NS_CURL_MISSING_MAX_HOST_CONNECTIONS>)
  endif()
endif()

add_framework(CFURLSessionInterface
                ${FRAMEWORK_LIBRARY_TYPE}
              FRAMEWORK_DIRECTORY
                CFURLSessionInterface_FRAMEWORK_DIRECTORY
              MODULE_MAP
                URL.subproj/module.modulemap
              PRIVATE_HEADERS
                URL.subproj/CFURLSessionInterface.h
              PUBLIC_HEADERS
                URL.subproj/CFURLSessionInterface.h
              SOURCES
                URL.subproj/CFURLSessionInterface.c)
add_dependencies(CFURLSessionInterface CoreFoundation)
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  target_compile_definitions(CFURLSessionInterface
                             PRIVATE
                               CURL_STATICLIB)
endif()
if(NOT CMAKE_SYSTEM_NAME STREQUAL Darwin)
  target_link_libraries(CFURLSessionInterface PRIVATE
    CURL::libcurl)
endif()

add_framework(CFXMLInterface
                ${FRAMEWORK_LIBRARY_TYPE}
              FRAMEWORK_DIRECTORY
                CFXMLInterface_FRAMEWORK_DIRECTORY
              MODULE_MAP
                Parsing.subproj/module.modulemap
              PRIVATE_HEADERS
                Parsing.subproj/CFXMLInterface.h
              PUBLIC_HEADERS
                Parsing.subproj/CFXMLInterface.h
              SOURCES
                Parsing.subproj/CFXMLInterface.c)
add_dependencies(CFXMLInterface CoreFoundation)
if(NOT CMAKE_SYSTEM_NAME STREQUAL Darwin)
  target_link_libraries(CFXMLInterface PRIVATE
    LibXml2::LibXml2)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  add_library(CoreFoundationResources OBJECT
    CoreFoundation.rc)
  if(BUILD_SHARED_LIBS)
    target_link_libraries(CoreFoundation PRIVATE CoreFoundationResources)
  else()
    target_link_libraries(CoreFoundation INTERFACE CoreFoundationResources)
  endif()

  # NOTE(compnerd) the WindowsOlsonMapping.plist and OlsonWindowsMapping.plist
  # are embedded Windows resources that we use to map the Windows timezone to
  # the Olson name.  Ensure that we rebuild on regeneration of the files.
  add_custom_target(CoreFoundationWindowsTimeZonesPLists DEPENDS
    WindowsOlsonMapping.plist
    OlsonWindowsMapping.plist)
  add_dependencies(CoreFoundation CoreFoundationWindowsTimeZonesPLists)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  target_link_libraries(CoreFoundation
                        PRIVATE
                          AdvAPI32
                          Secur32
                          User32
                          mincore
                          pathcch)
  target_link_libraries(CFURLSessionInterface
                        PRIVATE
                          AdvAPI32
                          Secur32
                          User32
                          mincore)
  target_link_libraries(CFXMLInterface
                        PRIVATE
                          AdvAPI32
                          Secur32
                          User32
                          mincore)
endif()
if(NOT CMAKE_SYSTEM_NAME STREQUAL Windows AND NOT CMAKE_SYSTEM_NAME STREQUAL Darwin)
  target_link_libraries(CoreFoundation
                        PRIVATE
                          m)
  target_link_libraries(CFURLSessionInterface
                        PRIVATE
                          m)
  target_link_libraries(CFXMLInterface
                        PRIVATE
                          m)
endif()
target_link_libraries(CoreFoundation
                      PRIVATE
                        dispatch)
target_link_libraries(CFURLSessionInterface
                      PRIVATE
                        dispatch)
target_link_libraries(CFXMLInterface
                      PRIVATE
                        dispatch)
if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
  target_link_libraries(CoreFoundation
                        PRIVATE
                          icucore)
  target_link_libraries(CFURLSessionInterface
                        PRIVATE
                          icucore)
  target_link_libraries(CFXMLInterface
                        PRIVATE
                          icucore)
  set_target_properties(CoreFoundation
                        PROPERTIES LINK_FLAGS
                          -Xlinker;-alias_list;-Xlinker;Base.subproj/DarwinSymbolAliases;-twolevel_namespace;-sectcreate;__UNICODE;__csbitmaps;CharacterSets/CFCharacterSetBitmaps.bitmap;-sectcreate;__UNICODE;__properties;CharacterSets/CFUniCharPropertyDatabase.data;-sectcreate;__UNICODE;__data;CharacterSets/CFUnicodeData-L.mapping;-segprot;__UNICODE;r;r)
endif()

install(TARGETS
          CoreFoundation
          CFURLSessionInterface
          CFXMLInterface
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBDIR}")

# Needed to avoid double slash "//" when CMAKE_INSTALL_PREFIX set to "/" and DESTDIR used to relocate whole installation.
# Double slash raise CMake error "file called with network path DESTINATION //System/Library/Frameworks".
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

install(DIRECTORY
          ${CoreFoundation_FRAMEWORK_DIRECTORY}
        DESTINATION
          ${CMAKE_INSTALL_PREFIX}/System/Library/Frameworks
        USE_SOURCE_PERMISSIONS
        PATTERN PrivateHeaders EXCLUDE)
