#
#  src makefile for the GNUstep Base Library
#
#  Copyright (C) 1997 Free Software Foundation, Inc.
#
#  Written by:	Scott Christley <scottc@net-community.com>
#
#  This file is part of the GNUstep Base Library.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
#  Library General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free
#  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#  Boston, MA 02111 USA.
#

ifeq ($(GNUSTEP_MAKEFILES),)
 GNUSTEP_MAKEFILES := $(shell gnustep-config --variable=GNUSTEP_MAKEFILES 2>/dev/null)
  ifeq ($(GNUSTEP_MAKEFILES),)
    $(warning )
    $(warning Unable to obtain GNUSTEP_MAKEFILES setting from gnustep-config!)
    $(warning Perhaps gnustep-make is not properly installed,)
    $(warning so gnustep-config is not in your PATH.)
    $(warning )
    $(warning Your PATH is currently $(PATH))
    $(warning )
  endif
endif

ifeq ($(GNUSTEP_MAKEFILES),)
  $(error You need to set GNUSTEP_MAKEFILES before compiling!)
endif

PACKAGE_NAME = gnustep-base
GNUSTEP_LOCAL_ADDITIONAL_MAKEFILES=../base.make

include $(GNUSTEP_MAKEFILES)/common.make

include ../Version
include ../config.mak

# Interface version changes with each minor release
libgnustep-base_INTERFACE_VERSION=$(MAJOR_VERSION).$(MINOR_VERSION)
libgnustep-baseadd_INTERFACE_VERSION=$(MAJOR_VERSION).$(MINOR_VERSION)

PACKAGE_NAME = gnustep-base

# The library to be compiled
LIBRARY_NAME=
ifeq ($(add),yes)
LIBRARY_NAME += libgnustep-baseadd
endif

ifneq ($(base),no)
LIBRARY_NAME += libgnustep-base
endif

OBJECTIVEC2_HEADERS =

ifeq ($(OBJC2RUNTIME),0)
  libgnustep-base_SUBPROJECTS = ObjectiveC2
  OBJECTIVEC2_HEADERS += \
	Availability.h blocks_runtime.h capabilities.h runtime.h
else
  ifeq ($(HAVE_BLOCKS),0)
    libgnustep-base_SUBPROJECTS = ObjectiveC2
    OBJECTIVEC2_HEADERS += blocks_runtime.h
  endif
endif

libgnustep-base_SUBPROJECTS += Additions
libgnustep-baseadd_SUBPROJECTS += Additions

ifeq ($(GNUSTEP_TARGET_OS), mingw32)
  libgnustep-base_SUBPROJECTS += win32
else ifeq ($(GNUSTEP_TARGET_OS), mingw64)
  libgnustep-base_SUBPROJECTS += win32
else ifeq ($(GNUSTEP_TARGET_OS), windows)
  libgnustep-base_SUBPROJECTS += win32
else
  libgnustep-base_SUBPROJECTS += unix
endif

DEFS+= -DGNUSTEP_TARGET_DIR=\"$(GNUSTEP_TARGET_DIR)\" \
	-DGNUSTEP_TARGET_CPU=\"$(GNUSTEP_TARGET_CPU)\" \
	-DGNUSTEP_TARGET_OS=\"$(GNUSTEP_TARGET_OS)\" \
	-DGNUSTEP_IS_FLATTENED=\"$(GNUSTEP_IS_FLATTENED)\" \
	-DLIBRARY_COMBO=\"$(LIBRARY_COMBO)\" \
	-DGNUSTEP_BASE_INTERNAL=1

# The GNU source files

GNU_MFILES = \
GSLocale.m \
preface.m

ifeq ($(findstring openbsd, $(GNUSTEP_TARGET_OS)), openbsd)
  OBJC_LIBS += -pthread
endif

ifeq ($(GNUSTEP_TARGET_OS), mingw32)
  GNU_MFILES += libgnustep-base-entry.m
else ifeq ($(GNUSTEP_TARGET_OS), mingw64)
  GNU_MFILES += libgnustep-base-entry.m
else ifeq ($(GNUSTEP_TARGET_OS), windows)
  GNU_MFILES += libgnustep-base-entry.m
endif

GNU_OTHER_SRCFILES = \
win32-entry.m \
win32-def.top \
libgnustep-base.def

GNUSTEPBASE_HEADERS = \
GCObject.h \
GSBlocks.h \
GSVersionMacros.h \
GSObjCRuntime.h \
GSUnion.h \
GSIArray.h \
GSIMap.h \
GSFunctions.h \
GSLocale.h \
GSLock.h \
GSMime.h \
GSTLS.h \
GSXML.h \
NSArray+GNUstepBase.h \
NSAttributedString+GNUstepBase.h \
NSBundle+GNUstepBase.h \
NSCalendarDate+GNUstepBase.h \
NSData+GNUstepBase.h \
NSDebug+GNUstepBase.h \
NSFileHandle+GNUstepBase.h \
NSHashTable+GNUstepBase.h \
NSLock+GNUstepBase.h \
NSMutableString+GNUstepBase.h \
NSNetServices+GNUstepBase.h \
NSNumber+GNUstepBase.h \
NSObject+GNUstepBase.h \
NSProcessInfo+GNUstepBase.h \
NSStream+GNUstepBase.h \
NSString+GNUstepBase.h \
NSTask+GNUstepBase.h \
NSThread+GNUstepBase.h \
NSURL+GNUstepBase.h \
Unicode.h \
GNUstep.h \
Additions.h


# GNUStep source files

BASE_MFILES = \
CXXException.m\
GSArray.m \
GSAttributedString.m \
GSBlocks.m \
GSConcreteValue.m \
GSCountedSet.m \
GSDictionary.m \
GSFTPURLHandle.m \
GSFormat.m \
GSHTTPAuthentication.m \
GSHTTPURLHandle.m \
GSICUString.m \
GSOrderedSet.m \
GSPrivateHash.m \
GSQuickSort.m \
GSRunLoopWatcher.m \
GSSet.m \
GSShellSort.m \
GSSocketStream.m \
GSStream.m \
GSString.m \
GSTimSort.m \
GSTLS.m \
GSValue.m \
GSSocksParser/GSSocksParser.m \
GSSocksParser/GSSocksParserPrivate.m \
GSSocksParser/GSSocks4Parser.m \
GSSocksParser/GSSocks5Parser.m \
NSAffineTransform.m \
NSAppleEventDescriptor.m \
NSAppleEventManager.m \
NSAppleScript.m \
NSArchiver.m \
NSArray.m \
NSAssertionHandler.m \
NSAttributedString.m \
NSAutoreleasePool.m \
NSBackgroundActivityScheduler.m \
NSBundle.m \
NSByteCountFormatter.m \
NSCache.m \
NSCachedURLResponse.m \
NSCalendar.m \
NSCalendarDate.m \
NSCallBacks.m \
NSCharacterSet.m \
NSClassDescription.m \
NSCoder.m \
NSCopyObject.m \
NSCountedSet.m \
NSConcreteHashTable.m \
NSConcreteMapTable.m \
NSConnection.m \
NSData.m \
NSDate.m \
NSDateComponentsFormatter.m \
NSDateFormatter.m \
NSDateInterval.m \
NSDateIntervalFormatter.m \
NSDebug.m \
NSDecimal.m \
NSDecimalNumber.m \
NSDictionary.m \
NSDistantObject.m \
NSDistributedLock.m \
NSDistributedNotificationCenter.m \
NSEnergyFormatter.m \
NSEnumerator.m \
NSError.m \
NSException.m \
NSExtensionContext.m \
NSExtensionItem.m \
NSFileCoordinator.m \
NSFileHandle.m \
NSFileManager.m \
NSFileVersion.m \
NSFileWrapper.m \
NSFormatter.m \
NSGarbageCollector.m \
NSGeometry.m \
NSHashTable.m \
NSHFSFileTypes.m \
NSHost.m \
NSHTTPCookie.m \
NSHTTPCookieStorage.m \
NSIndexPath.m \
NSIndexSet.m \
NSInvocation.m \
NSInvocationOperation.m \
NSISO8601DateFormatter.m \
NSItemProvider.m \
NSItemProviderReadingWriting.m \
NSJSONSerialization.m \
NSKeyedArchiver.m \
NSKeyedUnarchiver.m \
NSKeyValueCoding.m \
NSKeyValueObserving.m \
NSLengthFormatter.m \
NSLinguisticTagger.m \
NSLocale.m \
NSLock.m \
NSLog.m \
NSMapTable.m \
NSMassFormatter.m \
NSMeasurementFormatter.m \
NSMeasurement.m \
NSMetadata.m \
NSMetadataAttributes.m \
NSMethodSignature.m \
NSNotification.m \
NSNotificationCenter.m \
NSNotificationQueue.m \
NSNull.m \
NSNumber.m \
NSNumberFormatter.m \
NSObjCRuntime.m \
NSObject.m \
NSObjectScripting.m \
NSObject+NSComparisonMethods.m \
NSOperation.m \
NSOrderedSet.m \
NSOrthography.m \
NSPage.m \
NSPathUtilities.m \
NSPersonNameComponents.m \
NSPersonNameComponentsFormatter.m \
NSPipe.m \
NSPointerArray.m \
NSPointerFunctions.m \
NSConcretePointerFunctions.m \
NSPort.m \
NSPortCoder.m \
NSPortMessage.m \
NSPortNameServer.m \
NSPredicate.m \
NSProcessInfo.m \
NSProgress.m \
NSPropertyList.m \
NSProtocolChecker.m \
NSProxy.m \
NSRange.m \
NSRegularExpression.m\
NSRunLoop.m \
NSScanner.m \
NSScriptClassDescription.m \
NSScriptCoercionHandler.m \
NSScriptCommand.m \
NSScriptCommandDescription.m \
NSScriptExecutionContext.m \
NSScriptKeyValueCoding.m \
NSScriptObjectSpecifiers.m \
NSScriptStandardSuiteCommands.m \
NSScriptSuiteRegistry.m \
NSUnit.m \
NSUserActivity.m \
NSUserScriptTask.m \
NSSerializer.m \
NSSet.m \
NSSocketPort.m \
NSSocketPortNameServer.m \
NSSortDescriptor.m \
NSSpellServer.m \
NSString.m \
NSTask.m \
NSThread.m \
NSTimer.m \
NSTimeZone.m \
NSUbiquitousKeyValueStore.m \
NSUnarchiver.m \
NSUndoManager.m \
NSURL.m \
NSURLAuthenticationChallenge.m \
NSURLCache.m \
NSURLCredential.m \
NSURLConnection.m \
NSURLCredentialStorage.m \
NSURLDownload.m \
NSURLProtectionSpace.m \
NSURLProtocol.m \
NSURLRequest.m \
NSURLResponse.m \
NSTextCheckingResult.m\
NSURLHandle.m \
NSUserDefaults.m \
NSUserNotification.m \
NSUUID.m \
NSValue.m \
NSValueTransformer.m \
NSXMLDocument.m \
NSXMLDTD.m \
NSXMLDTDNode.m \
NSXMLElement.m \
NSXMLNode.m \
NSXMLParser.m \
NSXPCConnection.m \
NSZone.m \
externs.m \
objc-load.m 

ifneq ($(GNUSTEP_TARGET_OS), mingw32)
  ifneq ($(GNUSTEP_TARGET_OS), mingw64)
    ifneq ($(GNUSTEP_TARGET_OS), windows)
      BASE_MFILES += \
	GSFileHandle.m \
	NSMessagePort.m \
	NSMessagePortNameServer.m
    endif
  endif
endif

ifeq ($(HAVE_BLOCKS), 1)
ifeq ($(GNUSTEP_BASE_HAVE_LIBDISPATCH), 1)
ifeq ($(GNUSTEP_BASE_HAVE_LIBCURL), 1)
  BASE_MFILES += \
	GSEasyHandle.m \
	GSHTTPURLProtocol.m \
	GSMultiHandle.m \
	GSNativeProtocol.m \
	GSTaskRegistry.m \
	GSTimeoutSource.m \
	GSTransferState.m \
	GSURLSessionTaskBody.m \
	GSURLSessionTaskBodySource.m \
	NSURLSession.m
endif
endif
endif

ifeq ($(GNUSTEP_BASE_HAVE_MDNS), 1)
  BASE_MFILES += \
	GSMDNSNetServices.m \
	NSNetServices.m
endif

ifeq ($(GNUSTEP_BASE_HAVE_AVAHI), 1)
  BASE_MFILES += \
	GSAvahiNetService.m \
	GSAvahiNetServiceBrowser.m \
	GSAvahiClient.m \
	GSAvahiRunLoopIntegration.m \
	NSNetServices.m
endif

ifeq ($(WITH_FFI),libffi)
  GNU_MFILES += cifframe.m
  BASE_MFILES += GSFFIInvocation.m
endif
ifeq ($(WITH_FFI),ffcall)
  GNU_MFILES += callframe.m
  BASE_MFILES += GSFFCallInvocation.m
endif

BASE_OTHER_SRCFILES = \
GSConcreteValueTemplate.m \
GSTemplateValue.m \
dld-load.h \
hpux-load.h \
null-load.h \
simple-load.h \
win32-load.h \
NSCallBacks.h \
tzfile.h

FOUNDATION_HEADERS = \
Foundation.h \
FoundationErrors.h \
FoundationLegacySwiftCompatibility.h \
NSAffineTransform.h \
NSAppleEventDescriptor.h \
NSAppleEventManager.h \
NSAppleScript.h \
NSArchiver.h \
NSArray.h \
NSAttributedString.h \
NSAutoreleasePool.h \
NSBackgroundActivityScheduler.h \
NSBundle.h \
NSByteCountFormatter.h \
NSByteOrder.h \
NSCache.h\
NSCalendar.h \
NSCalendarDate.h \
NSCharacterSet.h \
NSClassDescription.h \
NSCoder.h \
NSComparisonPredicate.h \
NSCompoundPredicate.h \
NSConnection.h \
NSData.h \
NSDateComponentsFormatter.h \
NSDateFormatter.h \
NSDateInterval.h \
NSDateIntervalFormatter.h \
NSDate.h \
NSDebug.h \
NSDecimal.h \
NSDecimalNumber.h \
NSDictionary.h \
NSDistantObject.h \
NSDistributedLock.h \
NSDistributedNotificationCenter.h \
NSEnergyFormatter.h \
NSEnumerator.h \
NSError.h \
NSErrorRecoveryAttempting.h \
NSException.h \
NSExtensionContext.h \
NSExtensionItem.h \
NSExtensionRequestHandling.h \
NSExpression.h \
NSFileCoordinator.h \
NSFileHandle.h \
NSFileManager.h \
NSFilePresenter.h \
NSFileVersion.h \
NSFileWrapper.h \
NSFormatter.h \
NSGarbageCollector.h \
NSGeometry.h \
NSHashTable.h \
NSHFSFileTypes.h \
NSHost.h \
NSHTTPCookie.h \
NSHTTPCookieStorage.h \
NSIndexPath.h \
NSIndexSet.h \
NSInvocation.h \
NSInvocationOperation.h \
NSISO8601DateFormatter.h \
NSItemProvider.h \
NSItemProviderReadingWriting.h \
NSJSONSerialization.h \
NSKeyedArchiver.h \
NSKeyValueCoding.h \
NSKeyValueObserving.h \
NSLengthFormatter.h \
NSLinguisticTagger.h \
NSLocale.h \
NSLock.h \
NSMapTable.h \
NSMassFormatter.h \
NSMeasurementFormatter.h \
NSMeasurement.h \
NSMetadata.h \
NSMetadataAttributes.h \
NSMethodSignature.h \
NSNetServices.h \
NSNotification.h \
NSNotificationQueue.h \
NSNull.h \
NSNumberFormatter.h \
NSObjCRuntime.h \
NSObject.h \
NSObjectScripting.h \
NSOperation.h \
NSOrderedSet.h \
NSOrthography.h \
NSPathUtilities.h \
NSPersonNameComponents.h \
NSPersonNameComponentsFormatter.h \
NSPointerArray.h \
NSPointerFunctions.h \
NSPortCoder.h \
NSPort.h \
NSPortMessage.h \
NSPortNameServer.h \
NSPredicate.h \
NSProcessInfo.h \
NSProgress.h \
NSPropertyList.h \
NSProtocolChecker.h \
NSProxy.h \
NSRange.h \
NSRegularExpression.h\
NSRunLoop.h \
NSScanner.h \
NSScriptClassDescription.h \
NSScriptCoercionHandler.h \
NSScriptCommand.h \
NSScriptCommandDescription.h \
NSScriptExecutionContext.h \
NSScriptKeyValueCoding.h \
NSScriptObjectSpecifiers.h \
NSScriptStandardSuiteCommands.h \
NSScriptSuiteRegistry.h \
NSUnit.h \
NSUserActivity.h \
NSUserScriptTask.h \
NSScriptWhoseTests.h \
NSSerialization.h \
NSSet.h \
NSSortDescriptor.h \
NSSpellServer.h \
NSStream.h \
NSString.h \
NSTask.h \
NSTextCheckingResult.h\
NSThread.h \
NSTimer.h \
NSTimeZone.h \
NSUbiquitousKeyValueStore.h \
NSUndoManager.h \
NSURLAuthenticationChallenge.h \
NSURLCache.h \
NSURLConnection.h \
NSURLCredential.h \
NSURLCredentialStorage.h \
NSURLDownload.h \
NSURLError.h \
NSURL.h \
NSURLHandle.h \
NSURLProtectionSpace.h \
NSURLProtocol.h \
NSURLRequest.h \
NSURLResponse.h \
NSURLSession.h \
NSUserDefaults.h \
NSUserNotification.h \
NSUtilities.h \
NSUUID.h \
NSValue.h \
NSValueTransformer.h \
NSXMLDocument.h \
NSXMLDTD.h \
NSXMLDTDNode.h \
NSXMLElement.h \
NSXMLNode.h \
NSXMLNodeOptions.h \
NSXMLParser.h \
NSXPCConnection.h \
NSZone.h

HEADERS_INSTALL = \
	$(OBJECTIVEC2_HEADERS) \
	$(GNUSTEPBASE_HEADERS) \
	$(FOUNDATION_HEADERS)

GENERATED_HFILES = \
dynamic-load.h \
$(GNUSTEP_TARGET_DIR)/config.h \
$(GNUSTEP_TARGET_DIR)/GNUstepBase/GSConfig.h


ifeq ($(HAVE_INET_PTON), no)
GNU_MFILES += inet_pton.m
endif
ifeq ($(HAVE_INET_NTOP), no)
GNU_MFILES += inet_ntop.m
endif

# The Objective-C source files to be compiled
libgnustep-base_OBJC_FILES = $(GNU_MFILES) \
	$(BASE_MFILES)
libgnustep-base_C_FILES = $(GNU_CFILES)

# Extra DLL exports file
libgnustep-base_DLL_DEF = libgnustep-base.def

libgnustep-base_HEADER_FILES_DIR = $(HEADER_DIR_FND)
libgnustep-base_HEADER_FILES_INSTALL_DIR = /Foundation

libgnustep-base_HEADER_FILES = $(FOUNDATION_HEADERS)

# Resources
RESOURCE_SET_NAME = libbase-resources

# This is for gnustep-make >= 14-02-2007
libbase-resources_INSTALL_DIR = $(GNUSTEP_LIBRARY)/Libraries/gnustep-base/Versions/$(libgnustep-base_INTERFACE_VERSION)/Resources
# This is kept temporarily for gnustep-make < 14-02-2007
libbase-resources_RESOURCE_FILES_INSTALL_DIR = /Library/Libraries/Resources/gnustep-base

libbase-resources_LANGUAGES =
libbase-resources_LOCALIZED_RESOURCE_FILES =
libbase-resources_RESOURCE_DIRS =
libbase-resources_RESOURCE_FILES = Info-gnustep.plist

libgnustep-base_NEEDS_GUI = NO
libgnustep-baseadd_NEEDS_GUI = NO

# Build the Additions subproject first.  It can then be used in by
# both gnustep-base and gnustep-baseadd (otherwise, if we ever build
# gnustep-base and gnustep-baseadd in parallel, they'd both try to
# build Additions as a subproject, causing concurrency issues).  If it
# can be guaranteed that they'll never be built together, this could
# be removed.
ifeq ($(OBJC2RUNTIME),0)
SUBPROJECTS = ObjectiveC2
endif
SUBPROJECTS += Additions

-include Makefile.preamble

include $(GNUSTEP_MAKEFILES)/aggregate.make
include $(GNUSTEP_MAKEFILES)/library.make
include $(GNUSTEP_MAKEFILES)/resource-set.make

-include Makefile.postamble

