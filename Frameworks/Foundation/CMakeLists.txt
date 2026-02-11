add_framework_bundle(Foundation
    VERSION C
    INFO_PLIST Info.plist
    RESOURCES_DIRS English.lproj/*
    RESOURCES_FILES NSCharacterSet/*.bitmap
    HEADERS Headers/Foundation/*
    INSTALL_NAME_DIR /System/Library/Frameworks/Foundation.framework/Versions/C
)

set_target_properties(Foundation PROPERTIES CXX_STANDARD 14)
target_sources(Foundation PRIVATE
	NSFormatter.m
	NSDictionary/NSEnumerator_dictionaryObjects.m
	NSDictionary/NSMutableDictionary.m
	NSDictionary/NSEnumerator_dictionaryKeys.m
	NSDictionary/NSDictionary.m
	NSDictionary/NSMutableDictionary_mapTable.m
	NSDictionary/NSDictionary_mapTable.m
	NSArchiver.m
	NSAssertionHandler.m
	NSTimeZone/NSTimeZone.m
	NSTimeZone/NSTimeZone_absolute.m
	NSTimeZone/NSTimeZoneTransition.m
	NSTimeZone/NSTimeZoneType.m
	NSStream/NSOutputStream_socket.m
	NSStream/NSSelectInputSourceSet.m
	NSStream/NSFileHandle_stream.m
	NSStream/NSFileHandle.m
	NSStream/NSSocket.m
	NSStream/CFSSLHandler.m
	NSStream/NSInputStream.m
	NSStream/NSOutputStream_buffer.m
	NSStream/NSOutputStream_data.m
	NSStream/NSOutputStream_file.m
	NSStream/NSSelectSet.m
	NSStream/NSOutputStream.m
	NSStream/NSStream.m
	NSStream/NSInputStream_socket.m
	NSStream/NSInputStream_file.m
	NSStream/NSSelectInputSource.m
	NSStream/NSInputStream_data.m
	NSGarbageCollector.m
	NSScanner/NSScanner.m
	NSScanner/NSScanner_concrete.m
	NSNotificationCenter/NSNotificationCenter.m
	NSNotificationCenter/NSObjectToObservers.m
	NSNotificationCenter/NSNotificationObserver.m
	NSInvocation.m
	NSIndexPath.m
	NSScript/NSScriptWhoseTests.m
	NSEnumerator.m
	NSException/debugHelpers.m
	NSException/NSException.m
	NSException/NSRaise.m
	NSStringsFileFormat/NSStringsFileParser.m
	NSNetService/bonjour.m
	NSNetService/NSNetServices.m
	NSDebug.m
	NSSortDescriptor.m
	NSRange.m
	NSConcreteDirectoryEnumerator.m
	NSPointerArray.m
	NSUndoManager/NSUndoManager.m
	NSUndoManager/NSUndoGroup.m
	NSError.m
	NSKeyedArchiving/NSKeyedUnarchiver.m
	NSKeyedArchiving/NSKeyedArchiver.m
	NSSpellServer.m
	NSPipe.m
	NSByteOrder.m
	NSNumberFormatter.m
	NSHashTable.m
	NSDirectoryEnumerator.m
	NSProcessInfo.m
	NSData/NSData.m
	NSData/NSData_mapped.m
	NSData/NSData_concrete.m
	NSData/NSMutableData.m
	NSData/NSMutableData_concrete.m
	NSUnarchiver.m
	NSURL/NSURLCredential.m
	NSURL/NSHTTPURLResponse.m
	NSURL/NSURLAuthenticationChallenge.m
	NSURL/NSMutableURLRequest.m
	NSURL/NSURLCredentialStorage.m
	NSURL/NSURLDownload.m
	NSURL/NSURLError.m
	NSURL/NSURL.m
	NSURL/NSURLRequest.m
	NSURL/NSURLHandle.m
	NSURL/NSURLProtectionSpace.m
	NSURL/NSURLConnectionState.m
	NSURL/NSURLConnection.m
	NSURL/NSURLResponse.m
	NSURL/NSURLCache.m
	NSURL/NSURLProtocol_http.m
	NSURL/NSCachedURLResponse.m
	NSURL/NSURLProtocol.m
	xml/NSXMLDocument.m
	xml/NSXMLParser.m
	xml/NSXMLNode.m
	xml/NSXMLDTD.m
	xml/NSXMLDTDNode.m
	xml/NSXMLElement.m
	NSKeyValueCoding/NSDictionary+KVC.m
	NSKeyValueCoding/NSKVCMutableArray.m
	NSKeyValueCoding/NSKVOInfoPerObject.m
	NSKeyValueCoding/NSString+KVCAdditions.m
	NSKeyValueCoding/NSKeyValueCoding.m
	NSKeyValueCoding/NSKeyValueObserving.m
	NSKeyValueCoding/NSKeyObserver.m
	NSKeyValueCoding/NSArray+KVC.m
	NSKeyValueCoding/NSKeyPathObserver.m
	NSPointerFunctions.m
	NSOrthography.m
	NSString/NSDarwinString.m
	NSString/NSStringUTF8.m
	NSString/NSString_placeholder.m
	NSString/NSMutableString.m
	NSString/NSString_unicode.m
	NSString/NSPathUtilities.m
	NSString/NSString_nextstep.m
	NSString/unibuffer.m
	NSString/NSString_win1252.m
	NSString/NSString_nextstepCString.m
	NSString/NSUnicodeCaseMapping.m
	NSString/NSString.m
	NSString/NSStringSymbol.m
	NSString/NSStringFormatter.m
	NSString/NSConstantString-Impl.m
	NSString/NSConstantString_tiny-Impl.m
	NSString/NSMutableString_unicodePtr.m
	NSString/NSString_isoLatin1.m
	NSString/NSString_macOSRoman.m
	NSString/NSString_unicodePtr.m
	NSString/NSString_cString.m
	NSString/NSStringFileIO.m
	NSString/NSStringEncoder.m
	NSString/NSString_isoLatin2.m
	NSSpellEngine.m
	NSConnection/NSConnection.m
	NSConnection/NSDistantObject.m
	NSConnection/NSSocketPort.m
	NSConnection/NSPortCoder.m
	NSConnection/NSProtocolChecker.m
	NSConnection/NSPortMessage.m
	NSConnection/NSMessagePort.m
	NSConnection/NSPort.m
	NSNull.m
	NSHTTPCookie/NSHTTPCookieStorage.m
	NSHTTPCookie/NSHTTPCookie.m
	NSLocale.m
	NSNotificationQueue/NSNotificationQueue.m
	NSNotificationQueue/NSNotificationAndModes.m
	NSMapTable.m
	NSAutoreleasePool/NSAutoreleasePool.m
	NSValueTransformer/NSValueTransformer_IsNotNil.m
	NSValueTransformer/NSValueTransformer.m
	NSValueTransformer/NSValueTransformer_IsNil.m
	NSValueTransformer/NSValueTransformer_KeyedUnarchiveFromData.m
	NSValueTransformer/NSValueTransformer_UnarchiveFromData.m
	NSValueTransformer/NSValueTransformer_NegateBoolean.m
	NSArray/NSArray_placeholder.m
	NSArray/NSEnumerator_array.m
	NSArray/NSMutableArray_concrete.m
	NSArray/NSArray_concrete.m
	NSArray/NSMutableArray.m
	NSArray/NSEnumerator_arrayReverse.m
	NSArray/NSArray.m
	NSThread/NSThread.m
	NSOperation/NSOperationQueue.m
	NSOperation/NSAtomicList.m
	NSOperation/NSOperation.m
	NSObjCRuntime.m
	NSDistributedNotificationCenter.m
	NSMethodSignature.m
	NSPredicate/NSExpression_variable.m
	NSPredicate/NSCompoundPredicate.m
	NSPredicate/NSExpression_operator.m
	NSPredicate/NSExpression_constant.m
	NSPredicate/NSExpression_array.m
	NSPredicate/NSPredicate_BOOL.m
	NSPredicate/NSExpression_keypath.m
	NSPredicate/NSExpression_assignment.m
	NSPredicate/NSExpression_function.m
	NSPredicate/NSPredicate.m
	NSPredicate/NSComparisonPredicate.m
	NSPredicate/NSExpression.m
	NSPredicate/NSExpression_self.m
	NSCoder.m
	NSUserDefaults/NSUserDefaults.m
	NSUserDefaults/NSPersistantDomain.m
	NSIndexSet/NSMutableIndexSet.m
	NSIndexSet/NSIndexSet.m
	NSCharacterSet/NSMutableCharacterSet_bitmap.m
	NSCharacterSet/NSCharacterSet_range.m
	NSCharacterSet/NSCharacterSet_string.m
	NSCharacterSet/NSCharacterSet_bitmap.m
	NSCharacterSet/NSCharacterSet_rangeInverted.m
	NSCharacterSet/NSCharacterSet.m
	NSCharacterSet/NSMutableCharacterSet.m
	NSProxy.m
	NSGeometry.m
	NSLock/NSRecursiveLock.m
	NSLock/NSConditionLock.m
	NSLock/NSCondition.m
	NSLock/NSLock.m
	NSLock/NSSynchronization.m
	NSTextCheckingResult.m
	NSValue/NSValue.m
	NSValue/NSValue_placeholder.m
	NSValue/NSValue_nonRetainedObject.m
	NSValue/NSValue_concrete.m
	NSValue/NSValue_pointer.m
	NSPortNameServer/NSMessagePortNameServer.m
	NSPortNameServer/NSPortNameServer.m
	NSPortNameServer/NSSocketPortNameServer.m
	NSDecimal/NSDecimal.m
	NSDecimal/NSDecimalNumber.m
	NSDecimal/NSDecimalNumberPlaceholder.m
	NSNumber/NSNumber_unsignedInt.m
	NSNumber/NSNumber_unsignedShort.m
	NSNumber/NSNumber_unsignedLong.m
	NSNumber/NSNumber_char.m
	NSNumber/NSNumber_longLong.m
	NSNumber/NSNumber_int.m
	NSNumber/NSNumber_BOOL_const.m
	NSNumber/NSNumber_CF.m
	NSNumber/NSNumber.m
	NSNumber/NSNumber_float.m
	NSNumber/NSNumber_short.m
	NSNumber/NSNumber_unsignedLongLong.m
	NSNumber/NSNumber_placeholder.m
	NSNumber/NSNumber_BOOL.m
	NSNumber/NSNumber_double.m
	NSNumber/NSNumber_unsignedChar.m
	NSNumber/NSNumber_long.m
	NSTimer/NSTimer_targetAction.m
	NSTimer/NSTimer_concrete.m
	NSTimer/NSTimer_invocation.m
	NSTimer/NSTimer.m
	NSClassDescription.m
	NSTask.m
	NSSet/NSSet_placeholder.m
	NSSet/NSMutableSet_concrete.m
	NSSet/NSSet_concrete.m
	NSSet/NSEnumerator_set.m
	NSSet/NSCountedSet.m
	NSSet/NSInlineSetTable.m
	NSSet/NSMutableSet.m
	NSSet/NSSet.m
	NSAttributedString/NSRangeEntries.m
	NSAttributedString/NSAttributedString_manyAttributes.m
	NSAttributedString/NSMutableAttributedString_concrete.m
	NSAttributedString/NSAttributedString_nilAttributes.m
	NSAttributedString/NSAttributedString.m
	NSAttributedString/NSAttributedString_placeholder.m
	NSAttributedString/NSAttributedString_oneAttribute.m
	NSAttributedString/NSMutableString_proxyToMutableAttributedString.m
	NSAttributedString/NSMutableAttributedString.m
	NSObject/NSObject.m
	NSObject/objc_forward_ffi.m
	NSObject/objc_size_alignment.m
	NSObject/NSZombieObject.m
	NSObject/forwarding.m
	NSDistributedLock.m
	NSMetadata/NSMetadataQuery.m
	NSMetadata/NSMetadataQueryResultGroup.m
	NSMetadata/NSMetadataItem.m
	NSMetadata/NSMetadataQueryAttributeValueTuple.m
	NSPlatform.m
	NSPropertyList/NSOldXMLAttribute.m
	NSPropertyList/NSOldXMLElement.m
	NSPropertyList/NSPropertyListReader_binary1.m
	NSPropertyList/NSOldXMLReader.m
	NSPropertyList/CFUID.m
	NSPropertyList/NSPropertyListWriter_xml1.m
	NSPropertyList/NSPropertyList.m
	NSPropertyList/NSPropertyListWriter_vintage.m
	NSPropertyList/NSOldXMLDocument.m
	NSPropertyList/NSPropertyListReader.m
	NSPropertyList/NSPropertyListReader_xml1.m
	NSPropertyList/NSPropertyListReader_vintage.m
	NSHost.m
	NSBundle.m
	NSRunLoop/NSRunLoop.m
	NSRunLoop/NSInputSource.m
	NSRunLoop/NSRunLoopState.m
	NSRunLoop/NSOrderedPerform.m
	NSRunLoop/NSInputSourceSet.m
	NSRunLoop/NSDelayedPerform.m
	NSFileManager.m
	NSAffineTransform.m
	NSNotification/NSNotification_concrete.m
	NSNotification/NSNotification.m
	NSZone/NSVirtualMemory.m
	NSZone/NSZone.m
	NSDate/NSCalendar.m
	NSDate/NSDate_timeInterval.m
	NSDate/NSCalendarDate.m
	NSDate/NSDate.m
	NSDate/NSDateComponents.m
	NSDate/NSDateFormatter.m 
)

if(CMAKE_SYSTEM_NAME MATCHES Darwin)
    target_sources(Foundation PRIVATE
        platform_darwin/DarwinMemoryFunctions.m
        platform_darwin/NSPlatform_darwin.m
        platform_darwin/NSString_defaultEncoding_darwin.m
        platform_darwin/libmain.m)
endif()

if(CMAKE_SYSTEM_NAME MATCHES Linux)
    target_sources(Foundation PRIVATE
        platform_linux/NSMemoryFunctions_linux.m
        platform_linux/NSPlatform_linux.m
        platform_linux/libmain.m)

    if(CpuArch MATCHES "aarch64")
        target_compile_definitions(Foundation PRIVATE OBJC_DISALLOW_FORWARDING=1)
        target_sources(Foundation PRIVATE NSObject/platform_linux/msgSendv-aarch64.s)
    else()
        target_sources(Foundation PRIVATE NSObject/platform_linux/msgSendv-x64.s)
    endif()
endif()

if((CMAKE_SYSTEM_NAME MATCHES Darwin) OR (CMAKE_SYSTEM_NAME MATCHES Linux))
    target_sources(Foundation PRIVATE
        platform_posix/NSCancelInputSource_posix.m
        platform_posix/NSCondition_posix.m
        platform_posix/NSConditionLock_posix.m
        platform_posix/NSConstantString.m
        platform_posix/NSFileHandle_posix.m
        platform_posix/NSFileManager_posix.m
        platform_posix/NSLocale_posix.m
        platform_posix/NSLock_posix.m
        platform_posix/NSMemoryFunctions_posix.m
        platform_posix/NSPersistantDomain_posix.m
        platform_posix/NSPipe_posix.m
        platform_posix/NSPlatform_posix.m
        platform_posix/NSRecursiveLock_posix.m
        platform_posix/NSRunLoopState_posix.m
        platform_posix/NSSelectSet_bsd.m
        platform_posix/NSSocket_bsd.m
        platform_posix/NSSocketPort_posix.m
        platform_posix/NSString_defaultEncoding_posix.m
        platform_posix/NSTask_posix.m
        platform_posix/NSTimeZone_posix.m)
endif()

# FIXME: we should build with HAVE_FFI=1
target_compile_definitions(Foundation PRIVATE
    APPLE_RUNTIME_4 DARWIN NULL=0 TARGET_OS_IOSMAC=0 PLATFORM_IS_POSIX PLATFORM_USES_BSD_SOCKETS)
target_compile_options(Foundation PRIVATE -isysroot${RAVYN_SDKROOT} -Wno-implicit-function-declaration)

target_include_directories(Foundation PRIVATE
    ${RAVYN_SDKROOT}/usr/local/include/kernel
    Headers .. ${ROOT_SOURCE_DIR}/Libraries/objc4
    ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_info/lookup.subproj)

target_link_options(Foundation PRIVATE
    -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib
    -Wl,-current_version,1677.104.0 -Wl,-compatibility_version,300.0.0
    -L${RAVYN_SDKROOT}/usr/lib -L${RAVYN_SDKROOT}/usr/lib/system -lobjc
)
set_source_files_properties(NSProcessInfo.m NSStringsFileFormat/NSStringsFileParser.m NSData/NSData.m NSNumberFormatter.m TARGET_DIRECTORY Foundation PROPERTIES COMPILE_OPTIONS "-I${CMAKE_CURRENT_SOURCE_DIR}/NSString")
set_source_files_properties(NSAutoreleasePool/NSAutoreleasePool.m TARGET_DIRECTORY Foundation PROPERTIES COMPILE_OPTIONS "-I${ROOT_SOURCE_DIR}/Libraries/objc4")
set_source_files_properties(NSObject/forwarding.m TARGET_DIRECTORY Foundation PROPERTIES
    LANGUAGE OBJCXX
    COMPILE_OPTIONS "-I${ROOT_SOURCE_DIR}/Libraries/objc4;-I${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders;-I/nest/ravynos/Libraries/Libsystem/private"
)
