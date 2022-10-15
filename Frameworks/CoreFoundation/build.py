# This source file is part of the Swift.org open source project
#
# Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See http://swift.org/LICENSE.txt for license information
# See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
#

script = Script()

cf = DynamicLibrary("CoreFoundation", uses_swift_runtime_object=False)

cf.GCC_PREFIX_HEADER = 'Base.subproj/CoreFoundation_Prefix.h'

if Configuration.current.target.sdk == OSType.Linux:
	cf.CFLAGS = '-D_GNU_SOURCE -DCF_CHARACTERSET_DATA_DIR="CharacterSets" '
	cf.LDFLAGS = '-Wl,-Bsymbolic '
	Configuration.current.requires_pkg_config = True
elif Configuration.current.target.sdk == OSType.FreeBSD:
	cf.CFLAGS = '-I/usr/local/include -I/usr/local/include/libxml2 -I/usr/local/include/curl '
	cf.LDFLAGS = ''
elif Configuration.current.target.sdk == OSType.MacOSX:
	cf.LDFLAGS = '-licucore -twolevel_namespace -Wl,-alias_list,Base.subproj/DarwinSymbolAliases -sectcreate __UNICODE __csbitmaps CharacterSets/CFCharacterSetBitmaps.bitmap -sectcreate __UNICODE __properties CharacterSets/CFUniCharPropertyDatabase.data -sectcreate __UNICODE __data CharacterSets/CFUnicodeData-L.mapping -segprot __UNICODE r r '
elif Configuration.current.target.sdk == OSType.Win32 and Configuration.current.target.environ == EnvironmentType.Cygnus:
	cf.CFLAGS = '-D_GNU_SOURCE -mcmodel=large '
	cf.LDFLAGS = '${SWIFT_USE_LINKER} -lswiftGlibc `icu-config --ldflags` -Wl,--allow-multiple-definition '

cf.ASFLAGS = " ".join([
        '-DCF_CHARACTERSET_BITMAP=\\"CharacterSets/CFCharacterSetBitmaps.bitmap\\"',
        '-DCF_CHARACTERSET_UNICHAR_DB=\\"CharacterSets/CFUniCharPropertyDatabase.data\\"',
        '-DCF_CHARACTERSET_UNICODE_DATA_B=\\"CharacterSets/CFUnicodeData-B.mapping\\"',
        '-DCF_CHARACTERSET_UNICODE_DATA_L=\\"CharacterSets/CFUnicodeData-L.mapping\\"',
])

cf.CFLAGS += " ".join([
    '-I${DSTROOT}${PREFIX}/include',
    '-I${DSTROOT}${PREFIX}/local/include',
]) + " "
cf.LDFLAGS += " ".join([
    '-L${DSTROOT}${PREFIX}/lib',
    '-L${DSTROOT}${PREFIX}/local/lib',
]) + " "

# For now, we do not distinguish between public and private headers (they are all private to Foundation)
# These are really part of CF, which should ultimately be a separate target
cf.ROOT_HEADERS_FOLDER_PATH = "${PREFIX}/include"
cf.PUBLIC_HEADERS_FOLDER_PATH = "${PREFIX}/include/CoreFoundation"
cf.PRIVATE_HEADERS_FOLDER_PATH = "${PREFIX}/include/CoreFoundation"
cf.PROJECT_HEADERS_FOLDER_PATH = "${PREFIX}/include/CoreFoundation"

cf.PUBLIC_MODULE_FOLDER_PATH = "${PREFIX}/include/CoreFoundation"

cf.CFLAGS += " ".join([
	'-DU_SHOW_DRAFT_API=1',
	'-DCF_BUILDING_CF=1',
	'-DDEPLOYMENT_RUNTIME_C=1',
	'-fconstant-cfstrings',
	'-fexceptions',
	'-Wno-shorten-64-to-32',
	'-Wno-deprecated-declarations',
	'-Wno-unreachable-code',
	'-Wno-conditional-uninitialized',
	'-Wno-unused-variable',
	'-Wno-int-conversion',
	'-Wno-unused-function',
	'-I./',
])

if Configuration.current.requires_pkg_config:
    pkg_config_dependencies = [
        'libcurl',
        'libxml-2.0',
    ]
    for package_name in pkg_config_dependencies:
        try:
            package = PkgConfig(package_name)
        except PkgConfig.Error as e:
            sys.exit("pkg-config error for package {}: {}".format(package_name, e))
        cf.CFLAGS += ' {} '.format(' '.join(package.cflags))
        cf.LDFLAGS += ' {} '.format(' '.join(package.ldflags))
else:
	cf.CFLAGS += ''.join([
		'-I${SYSROOT}/usr/include/curl ',
		'-I${SYSROOT}/usr/include/libxml2 ',
	])
	cf.LDFLAGS += ''.join([
		'-lcurl ',
		'-lxml2 ',
	])

triple = Configuration.current.target.triple
if triple == "armv7-none-linux-androideabi":
	cf.LDFLAGS += '-llog '
else:
	cf.LDFLAGS += '-lpthread '

cf.LDFLAGS += '-ldl -lm '

# Configure use of Dispatch in CoreFoundation and Foundation if libdispatch is being built
cf.CFLAGS += '-DDEPLOYMENT_ENABLE_LIBDISPATCH=1 '
cf.LDFLAGS += '-ldispatch -rpath \$$ORIGIN '

if "XCTEST_BUILD_DIR" in Configuration.current.variables:
	cf.LDFLAGS += '-L${XCTEST_BUILD_DIR}'

headers = CopyHeaders(
module = 'Base.subproj/module.modulemap',
public = [
	'Stream.subproj/CFStream.h',
	'String.subproj/CFStringEncodingExt.h',
	'Base.subproj/CoreFoundation.h',
	'Base.subproj/SwiftRuntime/TargetConditionals.h',
	'RunLoop.subproj/CFMessagePort.h',
	'Collections.subproj/CFBinaryHeap.h',
	'PlugIn.subproj/CFBundle.h',
	'Locale.subproj/CFCalendar.h',
	'Collections.subproj/CFBitVector.h',
	'Base.subproj/CFAvailability.h',
	'Collections.subproj/CFTree.h',
	'NumberDate.subproj/CFTimeZone.h',
	'Error.subproj/CFError.h',
	'Collections.subproj/CFBag.h',
	'PlugIn.subproj/CFPlugIn.h',
	'Parsing.subproj/CFXMLParser.h',
	'String.subproj/CFString.h',
	'Collections.subproj/CFSet.h',
	'Base.subproj/CFUUID.h',
	'NumberDate.subproj/CFDate.h',
	'Collections.subproj/CFDictionary.h',
	'Base.subproj/CFByteOrder.h',
	'AppServices.subproj/CFUserNotification.h',
	'Base.subproj/CFBase.h',
	'Preferences.subproj/CFPreferences.h',
	'Locale.subproj/CFLocale.h',
	'RunLoop.subproj/CFSocket.h',
	'Parsing.subproj/CFPropertyList.h',
	'Collections.subproj/CFArray.h',
	'RunLoop.subproj/CFRunLoop.h',
	'URL.subproj/CFURLAccess.h',
	'URL.subproj/CFURLSessionInterface.h',
	'Locale.subproj/CFDateFormatter.h',
	'RunLoop.subproj/CFMachPort.h',
	'PlugIn.subproj/CFPlugInCOM.h',
	'Base.subproj/CFUtilities.h',
	'Parsing.subproj/CFXMLNode.h',
	'URL.subproj/CFURLComponents.h',
	'URL.subproj/CFURL.h',
	'Locale.subproj/CFNumberFormatter.h',
	'String.subproj/CFCharacterSet.h',
	'NumberDate.subproj/CFNumber.h',
	'Collections.subproj/CFData.h',
	'String.subproj/CFAttributedString.h',
	'Base.subproj/CoreFoundation_Prefix.h',
	'AppServices.subproj/CFNotificationCenter.h'
],
private = [
	'Base.subproj/ForSwiftFoundationOnly.h',
	'Base.subproj/ForFoundationOnly.h',
	'Base.subproj/CFAsmMacros.h',
	'String.subproj/CFBurstTrie.h',
	'Error.subproj/CFError_Private.h',
	'URL.subproj/CFURLPriv.h',
	'Base.subproj/CFLogUtilities.h',
	'PlugIn.subproj/CFBundlePriv.h',
	'StringEncodings.subproj/CFStringEncodingConverter.h',
	'Stream.subproj/CFStreamAbstract.h',
	'Base.subproj/CFInternal.h',
	'Parsing.subproj/CFXMLInputStream.h',
	'Parsing.subproj/CFXMLInterface.h',
	'PlugIn.subproj/CFPlugIn_Factory.h',
	'String.subproj/CFStringLocalizedFormattingInternal.h',
	'PlugIn.subproj/CFBundle_Internal.h',
	'StringEncodings.subproj/CFStringEncodingConverterPriv.h',
	'Collections.subproj/CFBasicHash.h',
	'StringEncodings.subproj/CFStringEncodingDatabase.h',
	'StringEncodings.subproj/CFUnicodeDecomposition.h',
	'Stream.subproj/CFStreamInternal.h',
	'PlugIn.subproj/CFBundle_BinaryTypes.h',
	'Locale.subproj/CFICULogging.h',
	'Locale.subproj/CFLocaleInternal.h',
	'StringEncodings.subproj/CFUnicodePrecomposition.h',
	'Base.subproj/CFPriv.h',
	'StringEncodings.subproj/CFUniCharPriv.h',
	'URL.subproj/CFURL.inc.h',
	'NumberDate.subproj/CFBigNumber.h',
	'StringEncodings.subproj/CFUniChar.h',
	'StringEncodings.subproj/CFStringEncodingConverterExt.h',
	'Collections.subproj/CFStorage.h',
	'Base.subproj/CFRuntime.h',
	'String.subproj/CFStringDefaultEncoding.h',
	'String.subproj/CFCharacterSetPriv.h',
	'Stream.subproj/CFStreamPriv.h',
	'StringEncodings.subproj/CFICUConverters.h',
	'String.subproj/CFRegularExpression.h',
	'String.subproj/CFRunArray.h',
	'Locale.subproj/CFDateFormatter_Private.h',
	'Locale.subproj/CFLocale_Private.h',
	'Parsing.subproj/CFPropertyList_Private.h',
	'Base.subproj/CFKnownLocations.h',
],
project = [
])

cf.add_phase(headers)

sources_list = [
    '../uuid/uuid.c',
	# 'AppServices.subproj/CFUserNotification.c',
	'Base.subproj/CFBase.c',
	'Base.subproj/CFFileUtilities.c',
	'Base.subproj/CFPlatform.c',
	'Base.subproj/CFRuntime.c',
	'Base.subproj/CFSortFunctions.c',
	'Base.subproj/CFSystemDirectories.c',
	'Base.subproj/CFUtilities.c',
	'Base.subproj/CFUUID.c',
	'Collections.subproj/CFArray.c',
	'Collections.subproj/CFBag.c',
	'Collections.subproj/CFBasicHash.c',
	'Collections.subproj/CFBinaryHeap.c',
	'Collections.subproj/CFBitVector.c',
	'Collections.subproj/CFData.c',
	'Collections.subproj/CFDictionary.c',
	'Collections.subproj/CFSet.c',
	'Collections.subproj/CFStorage.c',
	'Collections.subproj/CFTree.c',
	'Error.subproj/CFError.c',
	'Locale.subproj/CFCalendar.c',
	'Locale.subproj/CFDateFormatter.c',
	'Locale.subproj/CFLocale.c',
	'Locale.subproj/CFLocaleIdentifier.c',
	'Locale.subproj/CFLocaleKeys.c',
	'Locale.subproj/CFNumberFormatter.c',
	'NumberDate.subproj/CFBigNumber.c',
	'NumberDate.subproj/CFDate.c',
	'NumberDate.subproj/CFNumber.c',
	'NumberDate.subproj/CFTimeZone.c',
	'Parsing.subproj/CFBinaryPList.c',
	'Parsing.subproj/CFOldStylePList.c',
	'Parsing.subproj/CFPropertyList.c',
	'Parsing.subproj/CFXMLInputStream.c',
	'Parsing.subproj/CFXMLNode.c',
	'Parsing.subproj/CFXMLParser.c',
	'Parsing.subproj/CFXMLTree.c',
	'Parsing.subproj/CFXMLInterface.c',
	'PlugIn.subproj/CFBundle.c',
	'PlugIn.subproj/CFBundle_Binary.c',
	'PlugIn.subproj/CFBundle_Grok.c',
	'PlugIn.subproj/CFBundle_InfoPlist.c',
	'PlugIn.subproj/CFBundle_Locale.c',
	'PlugIn.subproj/CFBundle_Resources.c',
	'PlugIn.subproj/CFBundle_Strings.c',
	'PlugIn.subproj/CFBundle_Main.c',
	'PlugIn.subproj/CFBundle_ResourceFork.c',
	'PlugIn.subproj/CFBundle_Executable.c',
	'PlugIn.subproj/CFBundle_DebugStrings.c',
	'PlugIn.subproj/CFPlugIn.c',
	'PlugIn.subproj/CFPlugIn_Factory.c',
	'PlugIn.subproj/CFPlugIn_Instance.c',
	'PlugIn.subproj/CFPlugIn_PlugIn.c',
	'Preferences.subproj/CFApplicationPreferences.c',
	'Preferences.subproj/CFPreferences.c',
	'Preferences.subproj/CFXMLPreferencesDomain.c',
	'RunLoop.subproj/CFMachPort.c',
	'RunLoop.subproj/CFMessagePort.c',
	'RunLoop.subproj/CFRunLoop.c',
	'RunLoop.subproj/CFSocket.c',
	'Stream.subproj/CFConcreteStreams.c',
	'Stream.subproj/CFSocketStream.c',
	'Stream.subproj/CFStream.c',
	'String.subproj/CFBurstTrie.c',
	'String.subproj/CFCharacterSet.c',
	'String.subproj/CFString.c',
	'String.subproj/CFStringEncodings.c',
	'String.subproj/CFStringScanner.c',
	'String.subproj/CFStringUtilities.c',
	'String.subproj/CFStringTransform.c',
	'StringEncodings.subproj/CFBuiltinConverters.c',
	'StringEncodings.subproj/CFICUConverters.c',
	'StringEncodings.subproj/CFPlatformConverters.c',
	'StringEncodings.subproj/CFStringEncodingConverter.c',
	'StringEncodings.subproj/CFStringEncodingDatabase.c',
	'StringEncodings.subproj/CFUniChar.c',
	'StringEncodings.subproj/CFUnicodeDecomposition.c',
	'StringEncodings.subproj/CFUnicodePrecomposition.c',
	'URL.subproj/CFURL.c',
	'URL.subproj/CFURLAccess.c',
	'URL.subproj/CFURLComponents.c',
	'URL.subproj/CFURLComponents_URIParser.c',
	'String.subproj/CFCharacterSetData.S',
	'String.subproj/CFUnicodeData.S',
	'String.subproj/CFUniCharPropertyDatabase.S',
	'String.subproj/CFRegularExpression.c',
	'String.subproj/CFAttributedString.c',
	'String.subproj/CFRunArray.c',
	'Base.subproj/CFKnownLocations.c',
]

sources = CompileSources(sources_list)
sources.add_dependency(headers)
cf.add_phase(sources)

script.add_product(cf)

LIBS_DIRS = ""
if "XCTEST_BUILD_DIR" in Configuration.current.variables:
    LIBS_DIRS += "${XCTEST_BUILD_DIR}:"
if "PREFIX" in Configuration.current.variables:
    LIBS_DIRS += Configuration.current.variables["PREFIX"]+"/lib:"

Configuration.current.variables["LIBS_DIRS"] = LIBS_DIRS

extra_script = """
rule InstallCoreFoundation
    command = mkdir -p "${DSTROOT}/${PREFIX}/lib"; $
    mkdir -p "${DSTROOT}/${PREFIX}/include"; $
    rsync -a "${BUILD_DIR}/CoreFoundation/${PREFIX}/include/CoreFoundation" "${DSTROOT}/${PREFIX}/include/"; $
    cp "${BUILD_DIR}/CoreFoundation/${DYLIB_PREFIX}CoreFoundation${DYLIB_SUFFIX}" "${DSTROOT}/${PREFIX}/lib"

build ${BUILD_DIR}/.install: InstallCoreFoundation ${BUILD_DIR}/CoreFoundation/${DYLIB_PREFIX}CoreFoundation${DYLIB_SUFFIX}

build install: phony | ${BUILD_DIR}/.install

"""

script.add_text(extra_script)

script.generate()
