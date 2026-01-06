/*
 * Copyright (c) 1998-2014 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef IOCONNECT_MAPMEMORY_10_6
#define IOCONNECT_MAPMEMORY_10_6	1
#endif

#include <IOKit/IOTypes.h>
#include <device/device_types.h>

#include <mach/mach.h>
#include <mach/mach_port.h>

#if TARGET_OS_SIMULATOR
#include <servers/bootstrap.h>
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <asl.h>
#include <dispatch/dispatch.h>
#include <dispatch/private.h>
#include <xpc/xpc.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFMachPort.h>

#include <libkern/OSAtomic.h>

#include <IOKit/IOBSD.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOKitServer.h>

#include <IOKit/IOCFSerialize.h>
#include <IOKit/IOCFUnserialize.h>

#include <IOKit/IOKitLibPrivate.h>
#include <IOKit/IOKitKeysPrivate.h>

#if __LP64__
typedef struct OSNotificationHeader64 NotificationHeader;

// XXX gvdl: Need to conditionalise this for LP64
#define mig_external __private_extern__
#include <iokitmig64.h>
#undef mig_external

#else

// To stage the IOKitUser map the 64 bit aware APIs locally
#   if EMULATE_IOCONNECT
#	define EMULATE_IOCONNECT_64		1
#	define EMULATE_IOCONNECT_ASYNC_64	1
#   endif

#   if EMULATE_IOCONNECT_64
#	define io_connect_method	em_connect_method
#   endif

#   if EMULATE_IOCONNECT_ASYNC_64
#	define io_connect_async_method	em_connect_async_method
#   endif

typedef struct OSNotificationHeader NotificationHeader;
#include <iokitmig32.h>

#endif

#if IOKIT_SERVER_VERSION >= 20190926 && !TARGET_OS_SIMULATOR
#define IOKIT_HAS_GET_PROPERTY_WITH_BUF 1
#else
#define IOKIT_HAS_GET_PROPERTY_WITH_BUF 0
#endif

uint64_t 
gIOKitLibServerVersion;
CFOptionFlags
gIOKitLibSerializeOptions = kIOCFSerializeToBinary;

/*
 * Ports
 */

extern 	mach_port_t 	mach_task_self();
const 	mach_port_t 	kIOMasterPortDefault = MACH_PORT_NULL;

static mach_port_t
__IOGetDefaultMasterPort()
{
    mach_port_t masterPort;

    kern_return_t result = IOMasterPort(MACH_PORT_NULL, &masterPort);
    if( KERN_SUCCESS != result)
	masterPort = MACH_PORT_NULL;

    return( masterPort );
}

kern_return_t
#if TARGET_OS_SIMULATOR
IOMasterPort( mach_port_t bootstrapPort, mach_port_t * masterPort )
#else
IOMasterPort( mach_port_t bootstrapPort __unused, mach_port_t * masterPort )
#endif
{
    kern_return_t result = KERN_SUCCESS;
    mach_port_t host_port = 0;

#if TARGET_OS_SIMULATOR
    /* Defaulting to bypass until <rdar://problem/13141176> is addressed */
    static boolean_t use_iokitsimd = 0;
    static dispatch_once_t once;

     dispatch_once(&once, ^{
        const char *value = getenv("IOS_SIMULATOR_IOKITSIMD");

        if (value) {
            use_iokitsimd = (*value == '1');
        }

/* Don't log about it until <rdar://problem/13141176> is addressed */
#if 0
        if (!use_iokitsimd)
            asl_log(NULL, NULL, ASL_LEVEL_NOTICE,
                    "IOKit.framework:IOMasterPort bypassing iokitsimd");
#endif
    });

    if (use_iokitsimd) {
        if (bootstrapPort == MACH_PORT_NULL)
            bootstrapPort = bootstrap_port;
        return bootstrap_look_up(bootstrapPort, "com.apple.iokitsimd", masterPort);
    }
#endif

    host_port = mach_host_self();
    result = host_get_io_master(host_port, masterPort);

    static dispatch_once_t versionOnce;
    dispatch_once(&versionOnce, ^{
#if IOKIT_SERVER_VERSION >= 20140421
        kern_return_t kr;
	kr = io_server_version(*masterPort, &gIOKitLibServerVersion);
	if (KERN_SUCCESS != kr) gIOKitLibServerVersion = 0;
#endif /* IOKIT_SERVER_VERSION >= 20140421 */
	if (gIOKitLibServerVersion < 20140421) gIOKitLibSerializeOptions &= ~kIOCFSerializeToBinary;
    });

   /* Dispose of the host port to prevent security breaches and port
    * leaks. We don't care about the kern_return_t value of this
    * call for now as there's nothing we can do if it fails.
    */
    if (host_port) mach_port_deallocate(mach_task_self(), host_port);
    return result;
}

kern_return_t
IOCreateReceivePort( uint32_t msgType, mach_port_t * recvPort )
{
    kern_return_t res;
    switch (msgType) {
        case kOSNotificationMessageID:
        case kOSAsyncCompleteMessageID:
            res = mach_port_allocate(mach_task_self(),
                                     MACH_PORT_RIGHT_RECEIVE, recvPort);

            break;

        default:
            res = kIOReturnBadArgument;
    }
    return res;
}

/*
 * IOObject
 */

kern_return_t
IOObjectRelease(
	io_object_t	object )
{
    return( mach_port_deallocate( mach_task_self(), object ));
}

kern_return_t
IOObjectRetain(
	io_object_t	object )
{
	kern_return_t ret;

    ret = mach_port_mod_refs(mach_task_self(),
                              object,
                              MACH_PORT_RIGHT_SEND,
                              1);

	if (KERN_INVALID_RIGHT == ret) {
		ret = mach_port_mod_refs(mach_task_self(),
								  object,
								  MACH_PORT_RIGHT_DEAD_NAME,
								  1);
	}

	return ret;
}

kern_return_t
IOObjectGetClass(
	io_object_t	object,
	io_name_t       className )
{
    return _IOObjectGetClass(object, 0, className);
}

kern_return_t
_IOObjectGetClass(
	io_object_t	object,
	uint64_t        options,
	io_name_t       className )
{
    CFTypeRef overrideType  = NULL;
    boolean_t override      = false;

#if !TARGET_OS_SIMULATOR
    if ( (options & kIOClassNameOverrideNone) == 0 ) {
        overrideType = IORegistryEntryCreateCFProperty(object, CFSTR(kIOClassNameOverrideKey), kCFAllocatorDefault, 0);
        
        if ( overrideType ) {
            if ( CFGetTypeID(overrideType) == CFStringGetTypeID() ) {
                override = CFStringGetCString((CFStringRef)overrideType, className, sizeof(io_name_t), kCFStringEncodingUTF8);
            }
            CFRelease(overrideType);
        }
    }
#endif /* !TARGET_OS_SIMULATOR */

    return( override ? kIOReturnSuccess : io_object_get_class( object, className ));
}


CFStringRef
IOObjectCopyClass(
	io_object_t     object)
{
    return _IOObjectCopyClass(object, 0);
}

CFStringRef
_IOObjectCopyClass(
	io_object_t     object,
	uint64_t        options)
{
    io_name_t my_name;
    CFStringRef my_str = NULL;
    
    // if there's no argument, no point going on.  Return NULL.
    if (!object)
	return my_str;
    
    _IOObjectGetClass( object, options, my_name );
    my_str = CFStringCreateWithCString (kCFAllocatorDefault, my_name, kCFStringEncodingUTF8);

    return my_str;
}

CFStringRef 
IOObjectCopySuperclassForClass(CFStringRef classname)
{
    io_name_t my_name, orig_name;
    CFStringRef my_str = NULL;
    char * my_cstr;
    kern_return_t kr; 

    // if there's no argument, no point going on.  Return NULL.
    if (classname == NULL) {
	return my_str;
    }

    my_cstr = malloc(sizeof(char) * 128);
    CFStringGetCString (classname, my_cstr, 128, kCFStringEncodingUTF8);

    strncpy(orig_name, my_cstr, sizeof(io_name_t));

    mach_port_t masterPort = __IOGetDefaultMasterPort();

    kr = io_object_get_superclass(masterPort, orig_name, my_name);

    if (masterPort != MACH_PORT_NULL)
	mach_port_deallocate(mach_task_self(), masterPort);

    if (kr == kIOReturnSuccess) {
	my_str = CFStringCreateWithCString (kCFAllocatorDefault, my_name, kCFStringEncodingUTF8);
    }
    free(my_cstr);

    return my_str;
}

CFStringRef 
IOObjectCopyBundleIdentifierForClass(CFStringRef classname)
{
    io_name_t my_name, orig_name;
    CFStringRef my_str = NULL;
    char * my_cstr;
    kern_return_t kr; 

    // if there's no argument, no point going on.  Return NULL.
    if (classname == NULL) {
	return my_str;
    }

    my_cstr = malloc(sizeof(char) * 128);
    CFStringGetCString (classname, my_cstr, 128, kCFStringEncodingUTF8);

    strncpy(orig_name, my_cstr, sizeof(io_name_t));

    mach_port_t masterPort = __IOGetDefaultMasterPort();

    kr = io_object_get_bundle_identifier(masterPort, orig_name, my_name);

    if (masterPort != MACH_PORT_NULL)
	mach_port_deallocate(mach_task_self(), masterPort);

    if (kr == kIOReturnSuccess) {
	my_str = CFStringCreateWithCString (kCFAllocatorDefault, my_name, kCFStringEncodingUTF8);
    }
    free(my_cstr);

    return my_str;
}

boolean_t
IOObjectConformsTo(
	io_object_t	object,
	const io_name_t	className )
{
    return _IOObjectConformsTo(object, className, 0);
}


boolean_t
_IOObjectConformsTo(
	io_object_t	object,
	const io_name_t	className,
	uint64_t        options)
{
    boolean_t	conforms;

    if( kIOReturnSuccess != io_object_conforms_to(
		object, (char *) className, &conforms ))
	conforms = 0;
    
#if !TARGET_OS_SIMULATOR
    if ( !conforms && ((options & kIOClassNameOverrideNone) == 0) ) {
        CFTypeRef overrideType = IORegistryEntryCreateCFProperty(object, CFSTR(kIOClassNameOverrideKey), kCFAllocatorDefault, 0);
        
        if ( overrideType ) {
            if ( CFGetTypeID(overrideType) == CFStringGetTypeID() ) {
                CFStringRef classNameString = CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, className, kCFStringEncodingUTF8, kCFAllocatorNull);
                if ( classNameString ) {
                    conforms = kCFCompareEqualTo == CFStringCompare((CFStringRef)overrideType, classNameString, 0);
                    CFRelease(classNameString);
                }
            }
            CFRelease(overrideType);
        }
    }
#endif /* !TARGET_OS_SIMULATOR */
    
    return( conforms );
}

boolean_t
IOObjectIsEqualTo(
	io_object_t	object,
	io_object_t	anObject )
{
    return( object == anObject );
}

uint32_t
IOObjectGetKernelRetainCount(
	io_object_t	object )
{
    uint32_t	count;

    if( kIOReturnSuccess != io_object_get_retain_count( object, &count))
	count = 0;

    return( count );
}

uint32_t
IOObjectGetRetainCount(
	io_object_t	object )
{
    return( IOObjectGetKernelRetainCount(object) );
}

uint32_t
IOObjectGetUserRetainCount(
	io_object_t	object )
{
    mach_port_urefs_t urefs;

    if( kIOReturnSuccess != mach_port_get_refs( mach_task_self(), object, MACH_PORT_RIGHT_SEND, &urefs))
	urefs = 0;

    return( urefs );
}

/*
 * IOIterator
 */

io_object_t
IOIteratorNext(
	io_iterator_t	iterator )
{
    io_object_t	next;

    if( kIOReturnSuccess != io_iterator_next( iterator, &next))
	next = 0;

    return( next );
}

void
IOIteratorReset(
	io_iterator_t	iterator )
{
    io_iterator_reset( iterator );
}

boolean_t
IOIteratorIsValid(
	io_iterator_t	iterator )
{
    boolean_t	valid;

    if( kIOReturnSuccess != io_iterator_is_valid( iterator, &valid ))
	valid = FALSE;

    return( valid );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * IOService
 */

io_service_t
IOServiceGetMatchingService(
	mach_port_t	_masterPort,
	CFDictionaryRef	matching )
{
    kern_return_t	kr;
    CFDataRef		data;
    CFIndex		dataLen;
    mach_port_t		masterPort;
    io_service_t        service = MACH_PORT_NULL;
    bool                ool;
    
    if( !matching)
	return( MACH_PORT_NULL);

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    data = IOCFSerialize( matching, gIOKitLibSerializeOptions );
    CFRelease( matching );
    if( !data)
	return( MACH_PORT_NULL );

    dataLen = CFDataGetLength(data);

    ool = true;
#if IOKIT_SERVER_VERSION >= 20140421
    if (kIOCFSerializeToBinary & gIOKitLibSerializeOptions)
    {
	if ((size_t) dataLen < sizeof(io_struct_inband_t))
	{
	    kr = io_service_get_matching_service_bin(masterPort,
			 (char *) CFDataGetBytePtr(data), dataLen, &service );
	    ool = false;
	}
    }
    else
#endif /* IOKIT_SERVER_VERSION >= 20140421 */
    {
	if ((size_t) dataLen < sizeof(io_string_t))
	{
	    kr = io_service_get_matching_service( masterPort,
		     (char *) CFDataGetBytePtr(data), &service );
	    ool = false;
	}
    }

    if (ool)
    {
	kern_return_t result;
	kr = io_service_get_matching_service_ool( masterPort,
		    (char *) CFDataGetBytePtr(data), dataLen, &result, &service );
	if (KERN_SUCCESS == kr)
	    kr = result;
    }

    CFRelease( data );
    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

   if (KERN_SUCCESS != kr)
	service = MACH_PORT_NULL;

    return( service );
}

kern_return_t
IOServiceGetMatchingServices(
        mach_port_t	_masterPort,
	CFDictionaryRef	matching,
	io_iterator_t * existing )
{
    kern_return_t	kr;
    CFDataRef		data;
    CFIndex		dataLen;
    mach_port_t		masterPort;
    bool                ool;
    
    if( !matching)
	return( kIOReturnBadArgument);

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    data = IOCFSerialize(matching, gIOKitLibSerializeOptions);
    CFRelease( matching );
    if( !data)
	return( kIOReturnUnsupported );

    dataLen = CFDataGetLength(data);

    ool = true;
#if IOKIT_SERVER_VERSION >= 20140421
    if (kIOCFSerializeToBinary & gIOKitLibSerializeOptions)
    {
	if ((size_t) dataLen < sizeof(io_struct_inband_t))
	{
	    kr = io_service_get_matching_services_bin(masterPort,
			(char *) CFDataGetBytePtr(data), dataLen, existing);
	    ool = false;
	}
    }
    else
#endif /* IOKIT_SERVER_VERSION >= 20140421 */
    {
	if ((size_t) dataLen < sizeof(io_string_t))
	{
	    kr = io_service_get_matching_services(masterPort,
			(char *) CFDataGetBytePtr(data), existing);
	    ool = false;
	}
    }
    if (ool)
    {
	kern_return_t result;
	kr = io_service_get_matching_services_ool( masterPort,
		    (char *) CFDataGetBytePtr(data), dataLen, &result, existing );
	if (KERN_SUCCESS == kr)
	    kr = result;
    }

    CFRelease( data );
    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

    return( kr );
}

kern_return_t
IOServiceMatchPropertyTable( io_service_t service, CFDictionaryRef matching,
                  boolean_t * matches )
{
    kern_return_t	kr;
    CFDataRef		data;
    CFIndex		dataLen;
    bool                ool;

    if( !matching)
	return( kIOReturnBadArgument);

    data = IOCFSerialize( matching, gIOKitLibSerializeOptions );
    if( !data)
	return( kIOReturnUnsupported );

    dataLen = CFDataGetLength(data);


    ool = true;
#if IOKIT_SERVER_VERSION >= 20140421
    if (kIOCFSerializeToBinary & gIOKitLibSerializeOptions)
    {
	if ((size_t) dataLen < sizeof(io_struct_inband_t))
	{
	    kr = io_service_match_property_table_bin(service,
			(char *) CFDataGetBytePtr(data), dataLen, matches);
	    ool = false;
	}
    }
    else
#endif /* IOKIT_SERVER_VERSION >= 20140421 */
    {
	if ((size_t) dataLen < sizeof(io_string_t))
	{
	    kr = io_service_match_property_table(service,
			(char *) CFDataGetBytePtr(data), matches);
	    ool = false;
	}
    }
    if (ool)
    {
	kern_return_t result;
	kr = io_service_match_property_table_ool( service,
		    (char *) CFDataGetBytePtr(data), dataLen, &result, matches );
	if (KERN_SUCCESS == kr)
	    kr = result;
    }

    CFRelease( data );

    return( kr );
}

static kern_return_t
InternalIOServiceAddNotification(
        mach_port_t	      _masterPort,
	const io_name_t	      notificationType,
	CFDictionaryRef	      matching,
	mach_port_t	      wakePort,
	io_async_ref_t        asyncRef,
	mach_msg_type_number_t referenceCnt,
	io_iterator_t 	    * notification )
{
    kern_return_t	kr;
    CFDataRef		data;
    CFIndex		dataLen;
    mach_port_t		masterPort;
    bool                ool;

    if( !matching)
	return( kIOReturnBadArgument);

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    data = IOCFSerialize( matching, gIOKitLibSerializeOptions );
    CFRelease( matching );
    if( !data)
	return( kIOReturnUnsupported );

    dataLen = CFDataGetLength(data);

    ool = true;
#if IOKIT_SERVER_VERSION >= 20140421
    if (kIOCFSerializeToBinary & gIOKitLibSerializeOptions)
    {
	if ((size_t) dataLen < sizeof(io_struct_inband_t))
	{
	    kr = io_service_add_notification_bin( masterPort, (char *) notificationType,
			(char *) CFDataGetBytePtr(data), dataLen,
			wakePort, asyncRef, referenceCnt,
			notification );
	    ool = false;
	}
    }
    else
#endif /* IOKIT_SERVER_VERSION >= 20140421 */
    {
	if ((size_t) dataLen < sizeof(io_string_t))
	{
	    kr = io_service_add_notification( masterPort, (char *) notificationType,
			(char *) CFDataGetBytePtr(data),
			wakePort, asyncRef, referenceCnt,
			notification );
	    ool = false;
	}
    }
    if (ool)
    {
	kern_return_t result;

	kr = io_service_add_notification_ool( masterPort, (char *) notificationType,
		    (char *) CFDataGetBytePtr(data), dataLen,
		    wakePort, asyncRef, referenceCnt,
		    &result, notification );
	if (KERN_SUCCESS == kr) kr = result;
    }

    CFRelease( data );
    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

    return( kr );
}

kern_return_t
IOServiceAddNotification(
        mach_port_t	 _masterPort,
	const io_name_t	 notificationType,
	CFDictionaryRef	 matching,
	mach_port_t	 wakePort,
	uintptr_t	 reference,
	io_iterator_t 	*notification )
{
    return (InternalIOServiceAddNotification(_masterPort, notificationType,
					     matching, wakePort, 
					     (io_user_reference_t *) &reference, 1,
					     notification));
}

kern_return_t
IOServiceAddMatchingNotification(
	IONotificationPortRef	notifyPort,
	const io_name_t		notificationType,
	CFDictionaryRef		matching,
        IOServiceMatchingCallback callback,
        void *			refcon,
	io_iterator_t * 	notification )
{
    io_user_reference_t	asyncRef[kIOMatchingCalloutCount];

    asyncRef[kIOMatchingCalloutFuncIndex]   = (io_user_reference_t) callback;
    asyncRef[kIOMatchingCalloutRefconIndex] = (io_user_reference_t) refcon;

    return (InternalIOServiceAddNotification(notifyPort->masterPort, notificationType,
					     matching, notifyPort->wakePort, 
					     &asyncRef[0], kIOMatchingCalloutCount,
					     notification));
}

kern_return_t
IOServiceAddInterestNotification(
	IONotificationPortRef	notifyPort,
        io_service_t		service,
	const io_name_t 	interestType,
        IOServiceInterestCallback callback,
        void *			refcon,
        io_object_t *		notification )
{
    io_user_reference_t	asyncRef[kIOInterestCalloutCount];
    kern_return_t	kr;

    asyncRef[kIOInterestCalloutFuncIndex]    = (io_user_reference_t) callback;
    asyncRef[kIOInterestCalloutRefconIndex]  = (io_user_reference_t) refcon;
    asyncRef[kIOInterestCalloutServiceIndex] = (io_user_reference_t) service;
    
    kr = io_service_add_interest_notification( service, (char *) interestType,
                            notifyPort->wakePort,
                            asyncRef, kIOInterestCalloutCount,
                            notification );

    return( kr );
}

IONotificationPortRef
IONotificationPortCreate(
	mach_port_t	masterPort )
{
    kern_return_t 	        kr;
    IONotificationPort     *notify;

    if (MACH_PORT_NULL == masterPort) {
        masterPort = __IOGetDefaultMasterPort();
    } else {
        IOObjectRetain(masterPort);
    }

    notify = calloc( 1, sizeof( IONotificationPort));
    if (!notify) {
        return( 0 );
    }
    notify->masterPort = masterPort;

    kr = IOCreateReceivePort(kOSNotificationMessageID, &notify->wakePort);
    if( kr != kIOReturnSuccess) {
        free( notify );
        return( 0 );
    }
    
    return notify;
}

static void
IONotificationPortRelease(void *ctxt)
{
	IONotificationPortRef notify = ctxt;

	if (OSAtomicDecrement32(&notify->refcount) < 0) {
		/* Note: dispatch sources require resources they are monitoring to be
		 * destroyed after their corresponding kevent has been unregistered.
		 *
		 * We hence use an internal refcount to make sure the port destruction
		 * is delayed until it is safe: the IONotificationPort and each each
		 * created dispatch source own a reference.
		 *
		 * The refcount encoding is offset by 1 so that its initial value (a
		 * single refcount owned by the IONotificationPort) can be 0. So
		 * we free resources when the refcount encoding becomes -1.
		 */
		mach_port_mod_refs(mach_task_self(), notify->wakePort,
				MACH_PORT_RIGHT_RECEIVE, -1);
		mach_port_deallocate(mach_task_self(), notify->masterPort);
		free( notify );
	}
}

void
IONotificationPortDestroy(
	IONotificationPortRef	notify )
{
	if (notify->cfmachPort) {
		CFMachPortInvalidate(notify->cfmachPort);
		CFRelease(notify->cfmachPort);
	}

	if (notify->source) {
		CFRelease(notify->source);
	}

	if (notify->dispatchSource) {
		dispatch_source_cancel(notify->dispatchSource);
		dispatch_release(notify->dispatchSource);
	}

	IONotificationPortRelease(notify);
}

CFRunLoopSourceRef
IONotificationPortGetRunLoopSource(
	IONotificationPortRef	notify )
{
    CFMachPortContext 	    context;
    Boolean		            cfReusedPort = false;

    if (notify->source)
        return (notify->source);
    
    context.version = 1;
    context.info = (void *) notify;
    context.retain = NULL;
    context.release = NULL;
    context.copyDescription = NULL;

    notify->cfmachPort = CFMachPortCreateWithPort(NULL, notify->wakePort,
        IODispatchCalloutFromCFMessage, &context, &cfReusedPort);
    if (!notify->cfmachPort)
        return NULL;
    
    if (cfReusedPort) 
    {    
        // We got a CFMachPortRef that CF re-used from its pool.
        // This is probably a race condition, and this belongs
        // to a recently dead port. 
        // We expect a new CFMachPortRef - we treat it as an error.

        CFStringRef     description = NULL;
        char            str[255];
        if (notify->cfmachPort) {
            description = CFCopyDescription(notify->cfmachPort);
            if (description) {
                CFStringGetCString(description, str, sizeof(str), kCFStringEncodingUTF8);
                CFRelease(description);
            }
        }
        
        asl_log(NULL, NULL, ASL_LEVEL_ERR, 
                "IOKit.framework:IONotificationPortGetRunLoopSource bad CFMachPort, %s\n", 
                description ? str : "No Description");

        CFRelease(notify->cfmachPort);
        notify->cfmachPort = NULL;
        goto exit;
    }

    notify->source = CFMachPortCreateRunLoopSource(NULL, notify->cfmachPort, 0);

exit:
    return (notify->source);
}

mach_port_t
IONotificationPortGetMachPort(
	IONotificationPortRef	notify )
{
    return( notify->wakePort );
}

kern_return_t
IONotificationPortSetImportanceReceiver(IONotificationPortRef notify)
{
	kern_return_t kr;

	kr = mach_port_set_attributes(mach_task_self(), notify->wakePort, MACH_PORT_IMPORTANCE_RECEIVER, (mach_port_info_t)NULL, 0);
	assert(kr == KERN_SUCCESS);

	return (kr);
}

boolean_t _IODispatchCalloutWithDispatch(mach_msg_header_t *msg, mach_msg_header_t *reply)
{
    mig_reply_setup(msg, reply);
    ((mig_reply_error_t*)reply)->RetCode = MIG_NO_REPLY;

    IODispatchCalloutFromCFMessage(NULL, msg, msg->msgh_size, dispatch_mach_msg_get_context(msg));
    return TRUE;
}

#define MAX_MSG_SIZE (8ul * 1024ul - MAX_TRAILER_SIZE)

void
IONotificationPortSetDispatchQueue(IONotificationPortRef notify, dispatch_queue_t queue)
{
	dispatch_source_t dispatchSource;

	if (notify->dispatchSource) {
		dispatch_source_cancel(notify->dispatchSource);
		dispatch_release(notify->dispatchSource);
		notify->dispatchSource = NULL;
	}

	if (!queue) return;

	OSAtomicIncrement32(&notify->refcount);
	dispatchSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, notify->wakePort, 0, queue);
	dispatch_set_context(dispatchSource, notify);
	dispatch_source_set_event_handler(dispatchSource, ^{
		dispatch_mig_server(dispatchSource, MAX_MSG_SIZE, _IODispatchCalloutWithDispatch);
	});
	dispatch_source_set_cancel_handler_f(dispatchSource, IONotificationPortRelease);

	notify->dispatchSource = dispatchSource;
	dispatch_activate(dispatchSource);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Matching creation helpers
 */

static CFMutableDictionaryRef
MakeOneStringProp(
	CFStringRef	key,
	const char *	name )
{
    CFMutableDictionaryRef	dict;
    CFStringRef			string;

    dict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0,
		&kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);

    if( !dict)
	return( dict);

    string = CFStringCreateWithCString( kCFAllocatorDefault, name,
					kCFStringEncodingMacRoman );

    if( string) {
        CFDictionarySetValue( dict, key, string );
        CFRelease( string );
    } else {
	CFRelease( dict );
	dict = 0;
    }

    return( dict );
}

static CFMutableDictionaryRef
MakeOneNumProp(
	CFStringRef	key,
	uint64_t	value )
{
    CFMutableDictionaryRef	dict;
    CFNumberRef			num;

    dict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0,
		&kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);

    if( !dict)
	return( dict);

    num = CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt64Type, &value );
    if( num) {
        CFDictionarySetValue( dict, key, num );
        CFRelease( num );
    } else {
	CFRelease( dict );
	dict = 0;
    }

    return( dict );
}


CFMutableDictionaryRef
IOServiceMatching(
	const char *	name )
{
    return( MakeOneStringProp( CFSTR(kIOProviderClassKey), name ) );
}

CFMutableDictionaryRef
IOServiceNameMatching(
	const char *	name )
{
    return( MakeOneStringProp( CFSTR(kIONameMatchKey), name ));
}

CFMutableDictionaryRef
IORegistryEntryIDMatching(
	uint64_t	entryID )
{
    return( MakeOneNumProp( CFSTR(kIORegistryEntryIDKey), entryID ));
}

CFMutableDictionaryRef
IOBSDNameMatching(
    mach_port_t		masterPort	__unused,
	uint32_t		options 	__unused,
	const char *	name )
{
    if( !name)
	return( 0 );

    return( MakeOneStringProp( CFSTR(kIOBSDNameKey), name ));
}

CFMutableDictionaryRef
IOOpenFirmwarePathMatching(
	mach_port_t	masterPort __unused,
	uint32_t	options 	__unused,
	const char *	path )
{
    io_string_t	buf;
    int		maxLen;
    char *	comp;
    int		len;

    maxLen = sizeof( io_string_t);

    do {

	len = strlen( kIODeviceTreePlane ":" );
	maxLen -= len;
	if( maxLen <= 0)
	    continue;

	strlcpy( buf, kIODeviceTreePlane ":", len + 1 );
	comp = buf + len;

	len = strlen( path );
	maxLen -= len;
	if( maxLen <= 0)
	    continue;
	strlcpy( comp, path, len + 1 );

	return( MakeOneStringProp( CFSTR(kIOPathMatchKey), buf ));

    } while( false );

    return( 0 );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

kern_return_t
OSGetNotificationFromMessage(
	mach_msg_header_t	 *msg,
	uint32_t		  index,
        uint32_t		 *type,
        uintptr_t		 *reference,
	void			**content,
        vm_size_t		 *size )
{
    // The kernel handles the downcast of the reference vector for 32bit tasks
    NotificationHeader *	header;

    if( msg->msgh_id != kOSNotificationMessageID)
	return( kIOReturnBadMessageID );

    if( msg->msgh_size < (sizeof( mach_msg_header_t)
			+ sizeof( NotificationHeader)))
	return( kIOReturnNoResources );

    if( index)
	return( kIOReturnNoResources );

    header = (NotificationHeader *) (msg + 1);
    if( type)
        *type = (kIOKitNoticationTypeMask & header->type);
    if( reference)
        *reference = (uintptr_t) header->reference[0];
    if( size)
        *size = header->size;
    if( content) {
	if( header->size)
            *content = &header->content[0];
        else
            *content = 0;
    }

    return( kIOReturnSuccess );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
IODispatchCalloutFromMessage(void *cfPort, mach_msg_header_t *msg, void *info)
{
    return( IODispatchCalloutFromCFMessage( cfPort, msg, -1, info ));
}

void
IODispatchCalloutFromCFMessage(CFMachPortRef port __unused,
			void *_msg, CFIndex size __unused, void *info __unused)
{
    struct ComplexMsg {
        mach_msg_header_t		msgHdr;
	mach_msg_body_t			msgBody;
	mach_msg_port_descriptor_t	ports[1];
    } *					complexMsg = NULL;
    mach_msg_header_t *			msg = (mach_msg_header_t *)_msg;
    NotificationHeader *		header;
    io_iterator_t			notifier = MACH_PORT_NULL;
    io_service_t			service = MACH_PORT_NULL;
    uint32_t				leftOver;
    boolean_t				deliver = TRUE;

    if( msg->msgh_id != kOSNotificationMessageID)
	return;

    if( MACH_MSGH_BITS_COMPLEX & msg->msgh_bits) {

	complexMsg = (struct ComplexMsg *)_msg;

	if( complexMsg->msgBody.msgh_descriptor_count)
	    service = complexMsg->ports[0].name;
	header = (NotificationHeader *) &complexMsg->ports[complexMsg->msgBody.msgh_descriptor_count];
    
    } else
	header = (NotificationHeader *) (msg + 1);

    leftOver = msg->msgh_size - (((vm_address_t) (header + 1)) - ((vm_address_t) msg));

    // remote port is the notification (an iterator_t) that fired
    notifier = msg->msgh_remote_port;

    if( MACH_PORT_NULL != notifier) {
	kern_return_t kr;
	mach_port_urefs_t urefs;

	kr = mach_port_get_refs( mach_task_self(), msg->msgh_remote_port, MACH_PORT_RIGHT_SEND, &urefs);
	if( (KERN_SUCCESS != kr) || (urefs < 2)) {
	    // one ref carried by the message - < 2 means owner has released the notifier
	    deliver = false;
	}
    }

    if(deliver)
    {
      leftOver -= (kIOKitNoticationMsgSizeMask & (header->type >> kIOKitNoticationTypeSizeAdjShift));

      switch( kIOKitNoticationTypeMask & header->type )
      {
	case kIOAsyncCompletionNotificationType:
	{
	    IOAsyncCompletionContent *asyncHdr;
	    
	    asyncHdr = (IOAsyncCompletionContent *)(header + 1);
	    leftOver = (leftOver - sizeof(*asyncHdr)) / sizeof(void *);
	    void *func   = (void *) header->reference[kIOAsyncCalloutFuncIndex];
	    void *refCon = (void *) header->reference[kIOAsyncCalloutRefconIndex];
	    switch (leftOver) {
	    case 0:
		((IOAsyncCallback0) func)(refCon, asyncHdr->result);
		break;
	    case 1:
		((IOAsyncCallback1) func)(refCon, asyncHdr->result,
		    asyncHdr->args[0]);
		break;
	    case 2:
		((IOAsyncCallback2) func)(refCon, asyncHdr->result,
		    asyncHdr->args[0], asyncHdr->args[1]);
		break;
	    default:
		((IOAsyncCallback) func)(refCon, asyncHdr->result,
		    asyncHdr->args, leftOver);
		break;
	    }
	    break;
	}          
	case kIOServiceMessageNotificationType:
	{    
	    IOServiceInterestContent * interestHdr;
	    void * arg;
	    
	    interestHdr = (IOServiceInterestContent *)(header + 1);
	    leftOver = (leftOver - sizeof(*interestHdr) + sizeof(interestHdr->messageArgument)) / sizeof(void *);
	    if (leftOver <= 1)
		arg = interestHdr->messageArgument[0];
	    else
		arg = &interestHdr->messageArgument[0];

	    ((IOServiceInterestCallback)header->reference[kIOInterestCalloutFuncIndex])(
		(void *) header->reference[kIOInterestCalloutRefconIndex],
		service ? service :  (io_service_t) header->reference[kIOInterestCalloutServiceIndex],
		interestHdr->messageType, arg );
	    break;
	}
	case kIOServicePublishNotificationType:
	case kIOServiceMatchedNotificationType:
	case kIOServiceTerminatedNotificationType:
	    
	    ((IOServiceMatchingCallback)header->reference[kIOMatchingCalloutFuncIndex])(
		(void *) header->reference[kIOMatchingCalloutRefconIndex],
		notifier);
	    break;
      }
    }

    if( MACH_PORT_NULL != notifier)
	mach_port_deallocate( mach_task_self(), notifier );
    if( complexMsg)
    {
	uint32_t i;
	for( i = 0; i < complexMsg->msgBody.msgh_descriptor_count; i++)
	    mach_port_deallocate( mach_task_self(), complexMsg->ports[i].name );
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

kern_return_t
IOServiceGetBusyStateAndTime(
	io_service_t    service,
	uint64_t *	state,
	uint32_t *	busy_state,
	uint64_t *	accumulated_busy_time)
{
    kern_return_t	kr;

    kr = io_service_get_state( service, state, busy_state, accumulated_busy_time );

    if (kr != KERN_SUCCESS)
    {
	*state = 0;
	*busy_state = 0;
	*accumulated_busy_time = 0;
    }

    return( kr );
}

kern_return_t
IOServiceGetBusyState(
	io_service_t    service,
	uint32_t *	busyState )
{
    uint64_t		state;
    uint64_t		accumulated_busy_time;

    return (IOServiceGetBusyStateAndTime(service, &state, busyState, &accumulated_busy_time));
}

kern_return_t
IOServiceGetState(
	io_service_t    service,
	uint64_t *	state )
{
    uint32_t		busy_state;
    uint64_t		accumulated_busy_time;

    return (IOServiceGetBusyStateAndTime(service, state, &busy_state, &accumulated_busy_time));
}

kern_return_t
IOKitGetBusyState(
        mach_port_t	_masterPort,
	uint32_t *	busyState )
{
    io_service_t 	root;
    kern_return_t	kr;
    mach_port_t		masterPort;

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    kr = io_registry_entry_from_path( masterPort,
			kIOServicePlane ":/", &root );

    if( kr == KERN_SUCCESS) {
	kr = IOServiceGetBusyState( root, busyState );
	IOObjectRelease( root );
    } else
	*busyState = 0;

    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

    return( kr );
}

kern_return_t
IOServiceWaitQuiet(
	io_service_t      service,
	mach_timespec_t * waitTime )
{
    kern_return_t	kr;
    mach_timespec_t	defaultWait = { 0, -1 };

    if( 0 == waitTime)
        waitTime = &defaultWait;

    kr = io_service_wait_quiet( service, *waitTime );

    return( kr );
}


kern_return_t
IOKitWaitQuiet(
        mach_port_t	_masterPort,
	mach_timespec_t * waitTime )
{
    io_service_t 	root;
    kern_return_t	kr;
    mach_timespec_t	defaultWait = { 0, -1 };
    mach_port_t		masterPort;

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    kr = io_registry_entry_from_path( masterPort,
			kIOServicePlane ":/", &root );

    if( kr == KERN_SUCCESS) {
	if( 0 == waitTime)
	    waitTime = &defaultWait;
	kr = io_service_wait_quiet( root, *waitTime );
	IOObjectRelease( root );
    }

    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

    return( kr );
}

kern_return_t
IOServiceOpen(
	io_service_t    service,
	task_port_t	owningTask,
	uint32_t	type,
	io_connect_t  *	connect )
{
    kern_return_t	kr;
    kern_return_t	result;

    kr = io_service_open_extended( service,
	owningTask, type, NDR_record, NULL, 0, &result, connect );

    if (KERN_SUCCESS == kr)
        kr = result;

    return (kr);
}

kern_return_t
IOServiceClose(
	io_connect_t	connect )
{
    kern_return_t	kr;

    kr = io_service_close( connect);
    IOObjectRelease( connect );

    return( kr );
}

kern_return_t
IOServiceRequestProbe(
	io_service_t    service,
	uint32_t	options )
{
    return( io_service_request_probe( service, options ));
}

kern_return_t
IOServiceAuthorize(
	io_service_t	service,
	uint32_t	options )
{
    kern_return_t status;
    uint64_t serviceID;

    status = IORegistryEntryGetRegistryEntryID( service, &serviceID );

    if ( status == kIOReturnSuccess )
    {
        xpc_object_t message;

        message = xpc_dictionary_create( NULL, NULL, 0 );

        if ( message )
        {
            xpc_connection_t connection;

            xpc_dictionary_set_uint64( message, "options", options );
            xpc_dictionary_set_uint64( message, "service", serviceID );

            connection = xpc_connection_create( "com.apple.iokit.IOServiceAuthorizeAgent", NULL );

            if ( connection )
            {
                xpc_object_t reply;

                xpc_connection_set_event_handler( connection, ^( xpc_object_t object __unused ) { } );

                xpc_connection_resume( connection );

                reply = xpc_connection_send_message_with_reply_sync( connection, message );

                if ( reply )
                {
                    xpc_type_t type;

                    type = xpc_get_type( reply );

                    if ( type == XPC_TYPE_DICTIONARY )
                    {
                        status = xpc_dictionary_get_uint64( reply, "status" );
                    }
                    else
                    {
                        status = kIOReturnBadMessageID;
                    }

                    xpc_release( reply );
                }
                else
                {
                    status = kIOReturnNotPrivileged;
                }

                xpc_release( connection );
            }
            else
            {
                status = kIOReturnUnsupported;
            }
            
            xpc_release( message );
        }
        else
        {
            status = kIOReturnNoMemory;
        }
    }
    else
    {
        status = kIOReturnBadArgument;
    }

    return status;
}

int
IOServiceOpenAsFileDescriptor(
	io_service_t	service,
	int		oflag )
{
    kern_return_t status;
    uint64_t serviceID;
    int fd = -1;

    status = IORegistryEntryGetRegistryEntryID( service, &serviceID );

    if ( status == kIOReturnSuccess )
    {
        xpc_object_t message;

        message = xpc_dictionary_create( NULL, NULL, 0 );

        if ( message )
        {
            xpc_connection_t connection;

            xpc_dictionary_set_int64( message, "oflag", oflag );
            xpc_dictionary_set_uint64( message, "service", serviceID );

            connection = xpc_connection_create( "com.apple.iokit.ioserviceauthorized", NULL );

            if ( connection )
            {
                xpc_object_t reply;

                xpc_connection_set_event_handler( connection, ^( xpc_object_t object __unused ) { } );

                xpc_connection_resume( connection );

                reply = xpc_connection_send_message_with_reply_sync( connection, message );

                if ( reply )
                {
                    xpc_type_t type;

                    type = xpc_get_type( reply );

                    if ( type == XPC_TYPE_DICTIONARY )
                    {
                        status = xpc_dictionary_get_uint64( reply, "status" );

                        if ( status == kIOReturnSuccess )
                        {
                            fd = xpc_dictionary_dup_fd( reply, "fd" );
                        }
                    }
                    else
                    {
                        status = unix_err( EBADMSG );
                    }

                    xpc_release( reply );
                }
                else
                {
                    status = unix_err( EACCES );
                }

                xpc_release( connection );
            }
            else
            {
                status = unix_err( ENOTSUP );
            }
            
            xpc_release( message );
        }
        else
        {
            status = unix_err( ENOMEM );
        }
    }
    else
    {
        status = unix_err( EINVAL );
    }

    if ( status )
    {
        if ( unix_err( err_get_code( status ) ) == status )
        {
            errno = err_get_code( status );
        }
    }

    return fd;
}

kern_return_t
_IOServiceGetAuthorizationID(
	io_service_t    service,
	uint64_t *	authorizationID )
{
    return( io_service_get_authorization_id( service, authorizationID ) );
}

kern_return_t
_IOServiceSetAuthorizationID(
	io_service_t    service,
	uint64_t	authorizationID )
{
    return( io_service_set_authorization_id( service, authorizationID ) );
}

/*
 * IOService connection
 */
kern_return_t
IOConnectAddRef(
	io_connect_t	connect )
{
    return mach_port_mod_refs(mach_task_self(),
                              connect,
                              MACH_PORT_RIGHT_SEND,
                              1);
}

kern_return_t
IOConnectRelease(
	io_connect_t	connect )
{
    // XXX gvdl: Check with Simon about last reference removal
    return mach_port_mod_refs(mach_task_self(),
                              connect,
                              MACH_PORT_RIGHT_SEND,
                              -1);
}

kern_return_t
IOConnectGetService(
	io_connect_t	connect,
	io_service_t  *	service )
{
    return( io_connect_get_service( connect, service ));
}

kern_return_t
IOConnectSetNotificationPort(
	io_connect_t	connect,
	uint32_t	type,
	mach_port_t	port,
	uintptr_t	reference )
{
    return( io_connect_set_notification_port( connect,
		type, port, reference));
}

kern_return_t IOConnectMapMemory(
	 io_connect_t	connect,
	 uint32_t		memoryType,
	 task_port_t	intoTask,
	 vm_address_t	*atAddress,
	 vm_size_t		*ofSize,
	 IOOptionBits	options )
{
#if __LP64__
    return io_connect_map_memory_into_task
		(connect, memoryType, intoTask, (mach_vm_address_t *) atAddress, (mach_vm_size_t *) ofSize, options);

#else
    return io_connect_map_memory
		(connect, memoryType, intoTask, atAddress, ofSize, options);
#endif
}

kern_return_t IOConnectMapMemory64(
	 io_connect_t		connect,
	 uint32_t		memoryType,
	 task_port_t		intoTask,
	 mach_vm_address_t	*atAddress,
	 mach_vm_size_t		*ofSize,
	 IOOptionBits		options )
{
    return io_connect_map_memory_into_task
		(connect, memoryType, intoTask, atAddress, ofSize, options);
}

kern_return_t
IOConnectUnmapMemory(
	io_connect_t		connect,
	uint32_t		memoryType,
	task_port_t		fromTask,
	vm_address_t		atAddress )
{
#if __LP64__
    return io_connect_unmap_memory_from_task
		(connect, memoryType, fromTask, atAddress);
#else
    return io_connect_unmap_memory
		(connect, memoryType, fromTask, atAddress);
#endif
}

kern_return_t IOConnectUnmapMemory64(
	 io_connect_t		connect,
	 uint32_t		memoryType,
	 task_port_t		fromTask,
	 mach_vm_address_t	atAddress)
{
    return io_connect_unmap_memory_from_task
		(connect, memoryType, fromTask, atAddress);
}

kern_return_t
IOConnectAddClient(
	io_connect_t	connect,
	io_connect_t	client )
{
    return( io_connect_add_client( connect, client));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#if USE_TRAP_TRANSPORT

#define kUseTrapTransport 1
__private_extern__ char checkBegin = 0, checkEnd = 0;
static __inline__ void checkPtrRange(void *ptr, IOByteCount cnt)
{
    checkBegin = ((uint8_t *) ptr)[0];
    checkEnd   = ((uint8_t *) ptr)[cnt];
}

#else

#define kUseTrapTransport 0

#endif /* USE_TRAP_TRANSPORT */

#define reinterpret_cast_mach_vm_address_t(p) \
    ((mach_vm_address_t) (uintptr_t) p)

kern_return_t
IOConnectCallMethod(
	mach_port_t	 connection,		// In
	uint32_t	 selector,		// In
	const uint64_t	*input,			// In
	uint32_t	 inputCnt,		// In
	const void	*inputStruct,		// In
	size_t		 inputStructCnt,	// In
	uint64_t	*output,		// Out
	uint32_t	*outputCnt,		// In/Out
	void		*outputStruct,		// Out
	size_t		*outputStructCntP)	// In/Out
{
    kern_return_t	        rtn;

    void			*inb_input       = 0;
    mach_msg_type_number_t	 inb_input_size  = 0;
    void			*inb_output      = 0;
    mach_msg_type_number_t	 inb_output_size = 0;

    mach_vm_address_t		 ool_input       = 0;
    mach_vm_size_t		 ool_input_size  = 0;
    mach_vm_address_t		 ool_output      = 0;
    mach_vm_size_t		 ool_output_size = 0;
    io_buf_ptr_t                 var_output      = NULL;
    mach_msg_type_number_t       var_output_size = 0;

    if (inputStructCnt <= sizeof(io_struct_inband_t)) {
	inb_input      = (void *) inputStruct;
	inb_input_size = (mach_msg_type_number_t) inputStructCnt;
    }
    else {
	ool_input      = reinterpret_cast_mach_vm_address_t(inputStruct);
	ool_input_size = inputStructCnt;
    }

    if (!outputCnt) {
	static uint32_t zero = 0;
	outputCnt = &zero;
    }

    if (outputStructCntP) {
	size_t size = *outputStructCntP;

	if (size == (size_t) kIOConnectMethodVarOutputSize) {

	    rtn = io_connect_method_var_output(
	    			    connection,         selector,
				    (uint64_t *) input, inputCnt,
				    inb_input,          inb_input_size,
				    ool_input,          ool_input_size,
				    inb_output,         &inb_output_size,
				    output,             outputCnt,
				    &var_output,	&var_output_size);

	    *(void **)outputStruct = var_output;
	    *outputStructCntP      = var_output_size;

	    return (rtn);

	}
	else if (size <= sizeof(io_struct_inband_t)) {
	    inb_output      = outputStruct;
	    inb_output_size = (mach_msg_type_number_t) size;
	}
	else {
	    ool_output      = reinterpret_cast_mach_vm_address_t(outputStruct);
	    ool_output_size = (mach_vm_size_t)    size;
	}
    }

    rtn = io_connect_method(connection,         selector,
			    (uint64_t *) input, inputCnt,
			    inb_input,          inb_input_size,
			    ool_input,          ool_input_size,
			    inb_output,         &inb_output_size,
			    output,             outputCnt,
			    ool_output,         &ool_output_size);

    if (outputStructCntP) {
	if (*outputStructCntP <= sizeof(io_struct_inband_t))
	    *outputStructCntP = (size_t) inb_output_size;
	else
	    *outputStructCntP = (size_t) ool_output_size;
    }

    return rtn;
}

kern_return_t
IOConnectCallAsyncMethod(
	mach_port_t	 connection,		// In
	uint32_t	 selector,		// In
	mach_port_t	 wakePort,		// In
	uint64_t	*reference,		// In
	uint32_t	 referenceCnt,		// In
	const uint64_t	*input,			// In
	uint32_t	 inputCnt,		// In
	const void	*inputStruct,		// In
	size_t		 inputStructCnt,	// In
	uint64_t	*output,		// Out
	uint32_t	*outputCnt,		// In/Out
	void		*outputStruct,		// Out
	size_t		*outputStructCntP)	// In/Out
{
    kern_return_t	        rtn;

    void			*inb_input       = 0;
    mach_msg_type_number_t	 inb_input_size  = 0;
    void			*inb_output      = 0;
    mach_msg_type_number_t	 inb_output_size = 0;

    mach_vm_address_t		 ool_input       = 0;
    mach_vm_size_t		 ool_input_size  = 0;
    mach_vm_address_t		 ool_output      = 0;
    mach_vm_size_t		 ool_output_size = 0;

    if (inputStructCnt <= sizeof(io_struct_inband_t)) {
	inb_input      = (void *) inputStruct;
	inb_input_size = (mach_msg_type_number_t) inputStructCnt;
    }
    else {
	ool_input      = reinterpret_cast_mach_vm_address_t(inputStruct);
	ool_input_size = inputStructCnt;
    }

    if (!outputCnt) {
	static uint32_t zero = 0;
	outputCnt = &zero;
    }

    if (outputStructCntP) {
	size_t size = *outputStructCntP;

	if (size <= sizeof(io_struct_inband_t)) {
	    inb_output      = outputStruct;
	    inb_output_size = (mach_msg_type_number_t) size;
	}
	else {
	    ool_output      = reinterpret_cast_mach_vm_address_t(outputStruct);
	    ool_output_size = (mach_vm_size_t)    size;
	}
    }

    rtn = io_connect_async_method(connection,         wakePort,
				  reference,          referenceCnt,
				  selector,
				  (uint64_t *) input, inputCnt,
				  inb_input,          inb_input_size,
				  ool_input,          ool_input_size,
				  inb_output,         &inb_output_size,
				  output,             outputCnt,
				  ool_output,         &ool_output_size);

    if (outputStructCntP) {
	if (*outputStructCntP <= sizeof(io_struct_inband_t))
	    *outputStructCntP = (size_t) inb_output_size;
	else
	    *outputStructCntP = (size_t) ool_output_size;
    }

    return rtn;
}

kern_return_t
IOConnectCallStructMethod(
	mach_port_t	 connection,		// In
	uint32_t	 selector,		// In
	const void	*inputStruct,		// In
	size_t		 inputStructCnt,	// In
	void		*outputStruct,		// Out
	size_t		*outputStructCnt)	// In/Out
{
    return IOConnectCallMethod(connection,   selector,
			       NULL,         0,
			       inputStruct,  inputStructCnt,
			       NULL,         NULL,
			       outputStruct, outputStructCnt);
}

kern_return_t
IOConnectCallAsyncStructMethod(
	mach_port_t	 connection,		// In
	uint32_t	 selector,		// In
	mach_port_t	 wakePort,		// In
	uint64_t	*reference,		// In
	uint32_t	 referenceCnt,		// In
	const void	*inputStruct,		// In
	size_t		 inputStructCnt,	// In
	void		*outputStruct,		// Out
	size_t		*outputStructCnt)	// In/Out
{
    return IOConnectCallAsyncMethod(connection,   selector, wakePort,
				    reference,    referenceCnt,
				    NULL,         0,
				    inputStruct,  inputStructCnt,
				    NULL,         NULL,
				    outputStruct, outputStructCnt);
}

kern_return_t
IOConnectCallScalarMethod(
	mach_port_t	 connection,		// In
	uint32_t	 selector,		// In
	const uint64_t	*input,			// In
	uint32_t	 inputCnt,		// In
	uint64_t	*output,		// Out
	uint32_t	*outputCnt)		// In/Out
{
    return IOConnectCallMethod(connection, selector,
			       input,      inputCnt,
			       NULL,       0,
			       output,     outputCnt,
			       NULL,       NULL);
}

kern_return_t
IOConnectCallAsyncScalarMethod(
	mach_port_t	 connection,		// In
	uint32_t	 selector,		// In
	mach_port_t	 wakePort,		// In
	uint64_t	*reference,		// In
	uint32_t	 referenceCnt,		// In
	const uint64_t	*input,			// In
	uint32_t	 inputCnt,		// In
	uint64_t	*output,		// Out
	uint32_t	*outputCnt)		// In/Out
{
    return IOConnectCallAsyncMethod(connection, selector, wakePort,
				    reference,  referenceCnt,
				    input,      inputCnt,
				    NULL,       0,
				    output,    outputCnt,
				    NULL,      NULL);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

kern_return_t
IOConnectTrap0(io_connect_t	connect,
	       uint32_t		index)
{
    return iokit_user_client_trap(connect, index, 0, 0, 0, 0, 0, 0);
}

kern_return_t
IOConnectTrap1(io_connect_t	connect,
	       uint32_t		index,
	       uintptr_t	p1 )
{
    return iokit_user_client_trap(connect, index, p1, 0, 0, 0, 0, 0);
}

kern_return_t
IOConnectTrap2(io_connect_t	connect,
	       uint32_t		index,
	       uintptr_t	p1,
	       uintptr_t	p2 )
{
    return iokit_user_client_trap(connect, index, p1, p2, 0, 0, 0, 0);
}

kern_return_t
IOConnectTrap3(io_connect_t	connect,
	       uint32_t		index,
	       uintptr_t	p1,
	       uintptr_t	p2,
	       uintptr_t	p3 )
{
    return iokit_user_client_trap(connect, index, p1, p2, p3, 0, 0, 0);
}

kern_return_t
IOConnectTrap4(io_connect_t	connect,
	       uint32_t		index,
	       uintptr_t	p1,
	       uintptr_t	p2,
	       uintptr_t	p3,
	       uintptr_t	p4 )
{
    return iokit_user_client_trap(connect, index, p1, p2, p3, p4, 0, 0);
}

kern_return_t
IOConnectTrap5(io_connect_t	connect,
	       uint32_t		index,
	       uintptr_t	p1,
	       uintptr_t	p2,
	       uintptr_t	p3,
	       uintptr_t	p4,
	       uintptr_t	p5 )
{
    return iokit_user_client_trap(connect, index, p1, p2, p3, p4, p5, 0);
}

kern_return_t
IOConnectTrap6(io_connect_t	connect,
	       uint32_t		index,
	       uintptr_t	p1,
	       uintptr_t	p2,
	       uintptr_t	p3,
	       uintptr_t	p4,
	       uintptr_t	p5,
	       uintptr_t	p6 )
{
    return iokit_user_client_trap(connect, index, p1, p2, p3, p4, p5, p6);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

kern_return_t
IOConnectSetCFProperties(
	io_connect_t	connect,
	CFTypeRef	properties )
{
    CFDataRef		data;
    kern_return_t	kr;
    kern_return_t	result;

    data = IOCFSerialize( properties, gIOKitLibSerializeOptions );
    if( !data)
	return( kIOReturnUnsupported );

    kr = io_connect_set_properties( connect,
            (char *) CFDataGetBytePtr(data), CFDataGetLength(data),
            &result );

    CFRelease(data);

    if( KERN_SUCCESS == kr)
        kr = result;

    return( kr );
}

kern_return_t
IOConnectSetCFProperty(
	io_connect_t	connect,
        CFStringRef	propertyName,
	CFTypeRef	property )
{
    CFDictionaryRef	dict;
    kern_return_t	kr;
 
    CFTypeRef name = propertyName;
    dict = CFDictionaryCreate( kCFAllocatorDefault,
		    &name, &property, 1,
		    &kCFTypeDictionaryKeyCallBacks,
		    &kCFTypeDictionaryValueCallBacks );
    if( !dict)
	return( kIOReturnNoMemory );

    kr = IOConnectSetCFProperties( connect, dict );
    CFRelease( dict );

    return( kr );
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * IORegistry accessors
 */

kern_return_t
IORegistryCreateIterator(
        mach_port_t	_masterPort,
	const io_name_t	plane,
	IOOptionBits	options,
	io_iterator_t * iterator )
{
    kern_return_t	kr;
    mach_port_t		masterPort;

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    kr = io_registry_create_iterator( masterPort, (char *) plane,
		options, iterator);

    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

    return( kr );
}

kern_return_t
IORegistryEntryCreateIterator(
	io_registry_entry_t	entry,
	const io_name_t		plane,
	IOOptionBits		options,
	io_iterator_t 	      * iterator )
{
    return( io_registry_entry_create_iterator( entry, (char *) plane,
		options, iterator));
}

kern_return_t
IORegistryIteratorEnterEntry(
	io_iterator_t	iterator )
{
    return( io_registry_iterator_enter_entry( iterator));
}


kern_return_t
IORegistryIteratorExitEntry(
	io_iterator_t	iterator )
{
    return( io_registry_iterator_exit_entry( iterator));
}

io_registry_entry_t
IORegistryEntryFromPath(
        mach_port_t		_masterPort,
	const io_string_t	path )
{
    kern_return_t	kr;
    io_registry_entry_t	entry;
    mach_port_t		masterPort;

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    kr = io_registry_entry_from_path( masterPort, (char *) path, &entry );
    if( kIOReturnSuccess != kr)
	entry = 0;

    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

    return( entry );
}

io_registry_entry_t
IORegistryEntryCopyFromPath(
	mach_port_t	_masterPort,
	CFStringRef	cfpath )
{
#if IOKIT_SERVER_VERSION < 20150715
    return (MACH_PORT_NULL);
#else

    kern_return_t	   kr;
    kern_return_t          result;
    io_registry_entry_t	   entry;
    mach_port_t		   masterPort;
    io_buf_ptr_t           path_ool;
    mach_msg_type_number_t path_oolCnt;
    char                 * path;
    size_t                 pathLen;
    void                 * buffer;

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    buffer      = NULL;
    pathLen     = 0;
    path_ool    = NULL;
    path_oolCnt = 0;
    path = (char *) CFStringGetCStringPtr(cfpath, kCFStringEncodingUTF8);
    if (!path)
    {
	CFIndex bufferSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfpath),
	       kCFStringEncodingUTF8) + sizeof('\0');
        buffer = malloc(bufferSize);
        if (buffer && CFStringGetCString(cfpath, buffer, bufferSize, kCFStringEncodingUTF8))
            path = buffer;
    }
    if (!path) kr = kIOReturnBadArgument;
    else
    {
 	pathLen = strlen(path);
 	if (pathLen > sizeof(io_string_inband_t))
	{
	    path_ool = path;
	    path_oolCnt = pathLen + 1;
	    path = "\0";
	}

	kr = io_registry_entry_from_path_ool(masterPort, path, path_ool, path_oolCnt, &result, &entry);
    }

    if ((kIOReturnSuccess != kr) || (kIOReturnSuccess != result)) entry = 0;

    if (buffer) free(buffer);

    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

    return( entry );
#endif /* IOKIT_SERVER_VERSION < 20150715 */
}

io_registry_entry_t
IORegistryGetRootEntry(
        mach_port_t	_masterPort )
{
    kern_return_t	kr;
    mach_port_t		masterPort;
    io_registry_entry_t	entry;

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    kr = io_registry_get_root_entry( masterPort, &entry );
    if( kIOReturnSuccess != kr)
	entry = 0;

    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

    return( entry );
}

kern_return_t
IORegistryEntryGetPath(
	io_registry_entry_t	entry,
	const io_name_t		plane,
	io_string_t		path )
{
    return( io_registry_entry_get_path( entry, (char *) plane, path ));
}

CFStringRef
IORegistryEntryCopyPath(
	io_registry_entry_t	entry,
	const io_name_t         plane)
{
#if IOKIT_SERVER_VERSION < 20150715
    return (NULL);
#else

    kern_return_t err, kr;
    io_string_inband_t path;
    io_buf_ptr_t path_ool;
    mach_msg_type_number_t path_oolCnt;
    CFStringRef result;

    err = io_registry_entry_get_path_ool(entry, (char *) plane, path, &path_ool, &path_oolCnt);
    if (kIOReturnSuccess != err) return (NULL);

    if (path_ool && path_oolCnt)
    {
        result = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *) path_ool, path_oolCnt, kCFStringEncodingUTF8, false);
	kr = vm_deallocate(mach_task_self(), path_ool, path_oolCnt);
    }
    else
    {
        result = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
    }

    return (result);

#endif /* IOKIT_SERVER_VERSION < 20150715 */
}

boolean_t
IORegistryEntryInPlane(
	io_registry_entry_t	entry,
	const io_name_t 	plane )
{
    boolean_t		inPlane;

    if( kIOReturnSuccess != io_registry_entry_in_plane(
		entry, (char *) plane, &inPlane ))
	inPlane = false;

    return( inPlane );
}

kern_return_t
IORegistryEntryGetName(
	io_registry_entry_t	entry,
	io_name_t 	        name )
{
    return( io_registry_entry_get_name( entry, name ));
}

kern_return_t
IORegistryEntryGetNameInPlane(
	io_registry_entry_t	entry,
	const io_name_t 	plane,
	io_name_t 	        name )
{
    if( NULL == plane)
        plane = "";
    return( io_registry_entry_get_name_in_plane( entry,
						(char *) plane, name ));
}

kern_return_t
IORegistryEntryGetLocationInPlane(
	io_registry_entry_t	entry,
	const io_name_t 	plane,
	io_name_t 	        location )
{
    if( NULL == plane)
        plane = "";
    return( io_registry_entry_get_location_in_plane( entry,
						(char *) plane, location ));
}

kern_return_t
IORegistryEntryGetRegistryEntryID(
	io_registry_entry_t	entry,
	uint64_t *		entryID )
{
    kern_return_t kr;

    kr =  io_registry_entry_get_registry_entry_id(entry, entryID);
    if (KERN_SUCCESS != kr)
	*entryID = 0;

    return (kr);
}

kern_return_t
IORegistryEntryCreateCFProperties(
	io_registry_entry_t	entry,
	CFMutableDictionaryRef * properties,
        CFAllocatorRef		allocator,
	IOOptionBits   options __unused )
{
    kern_return_t	kr;
    uint32_t		size;
    char *		propertiesBuffer;
    CFStringRef		errorString;
    const char *	cstr;
#if IOKIT_HAS_GET_PROPERTY_WITH_BUF
    char		sBuf[2048];
    mach_vm_size_t      sBufSize = sizeof(sBuf);
#endif // IOKIT_HAS_GET_PROPERTY_WITH_BUF

#if IOKIT_SERVER_VERSION >= 20140421
    if (kIOCFSerializeToBinary & gIOKitLibSerializeOptions)
    {
#if IOKIT_HAS_GET_PROPERTY_WITH_BUF
	kr = io_registry_entry_get_properties_bin_buf(entry,
	    (mach_vm_address_t)sBuf, &sBufSize, &propertiesBuffer, &size);
#else
	kr = io_registry_entry_get_properties_bin(entry, &propertiesBuffer, &size);
#endif // IOKIT_HAS_GET_PROPERTY_WITH_BUF
    }
    else
#endif /* IOKIT_SERVER_VERSION >= 20140421 */
    {
	kr = io_registry_entry_get_properties(entry, &propertiesBuffer, &size);
    }

    if (kr != kIOReturnSuccess) return (kr);

    if (propertiesBuffer) {
	    *properties = (CFMutableDictionaryRef) IOCFUnserializeWithSize(propertiesBuffer, size, allocator,
									  0, &errorString);
    } else {
#if IOKIT_HAS_GET_PROPERTY_WITH_BUF
        *properties = (CFMutableDictionaryRef) IOCFUnserializeWithSize(sBuf, sBufSize, allocator,
                                        0, &errorString);
#else
        *properties = NULL;
#endif // IOKIT_HAS_GET_PROPERTY_WITH_BUF
    }

    if (!(*properties) && errorString)
    {
        if ((cstr = CFStringGetCStringPtr(errorString, kCFStringEncodingMacRoman))) printf("%s\n", cstr);
	CFRelease(errorString);
    }

    if (propertiesBuffer) {
	    // free propertiesBuffer !
	    vm_deallocate(mach_task_self(), (vm_address_t)propertiesBuffer, size);
    }

    return( *properties ? kIOReturnSuccess : kIOReturnInternalError );
}

CFTypeRef
IORegistryEntryCreateCFProperty(
	io_registry_entry_t	entry,
	CFStringRef		key,
        CFAllocatorRef		allocator,
	IOOptionBits   options __unused )
{
    return (IORegistryEntrySearchCFProperty(entry, NULL, key, allocator, kNilOptions));
}

CFTypeRef
IORegistryEntrySearchCFProperty(
	io_registry_entry_t	entry,
	const io_name_t		plane,
	CFStringRef		key,
        CFAllocatorRef		allocator,
	IOOptionBits		options )
{
    IOReturn		kr;
    CFTypeRef		type = NULL;
    uint32_t		size;
    char *		propertiesBuffer;
    CFStringRef		errorString;
    const char *	cStr;
    char *		buffer = NULL;
#if IOKIT_HAS_GET_PROPERTY_WITH_BUF
    char		sBuf[2048];
    mach_vm_size_t      sBufSize = sizeof(sBuf);
#endif // IOKIT_HAS_GET_PROPERTY_WITH_BUF

    cStr = CFStringGetCStringPtr( key, kCFStringEncodingMacRoman);
    if( !cStr)
    {
	CFIndex bufferSize = CFStringGetMaximumSizeForEncoding( CFStringGetLength(key),
	       kCFStringEncodingMacRoman) + sizeof('\0');
        buffer = malloc( bufferSize);
        if( buffer && CFStringGetCString( key, buffer, bufferSize, kCFStringEncodingMacRoman))
            cStr = buffer;
    }
if (!cStr) kr = kIOReturnError;
#if IOKIT_SERVER_VERSION >= 20140421
    else if (kIOCFSerializeToBinary & gIOKitLibSerializeOptions)
    {
	if (!(kIORegistryIterateRecursively & options)) plane = "\0";
#if IOKIT_HAS_GET_PROPERTY_WITH_BUF
        kr = io_registry_entry_get_property_bin_buf(entry, (char *) plane, (char *) cStr,
            options, (mach_vm_address_t)sBuf, &sBufSize, &propertiesBuffer, &size);
#else
        kr = io_registry_entry_get_property_bin(entry, (char *) plane, (char *) cStr,
                                                options, &propertiesBuffer, &size);
#endif // IOKIT_HAS_GET_PROPERTY_WITH_BUF
    }
#endif /* IOKIT_SERVER_VERSION >= 20140421 */
    else if (kIORegistryIterateRecursively & options)
    {
        kr = io_registry_entry_get_property_recursively(entry, (char *) plane, (char *) cStr,
                                                        options, &propertiesBuffer, &size);
    }
    else
    {
        kr = io_registry_entry_get_property(entry, (char *) cStr, &propertiesBuffer, &size);
    }

    if (buffer) free(buffer);
    if (kr != kIOReturnSuccess) return (NULL);

    if (propertiesBuffer) {
        type = (CFMutableDictionaryRef) IOCFUnserializeWithSize(propertiesBuffer, size, allocator,
                                    0, &errorString);
#if IOKIT_HAS_GET_PROPERTY_WITH_BUF
    } else {
	    type = (CFMutableDictionaryRef) IOCFUnserializeWithSize(sBuf, sBufSize, allocator,
								    0, &errorString);
#endif // IOKIT_HAS_GET_PROPERTY_WITH_BUF
    }
    if (!type && errorString)
    {
        if ((cStr = CFStringGetCStringPtr(errorString, kCFStringEncodingMacRoman))) printf("%s\n", cStr);
	CFRelease(errorString);
    }

    if (propertiesBuffer) {
	    // free propertiesBuffer !
	    vm_deallocate(mach_task_self(), (vm_address_t)propertiesBuffer, size);
    }

    return( type );
}

kern_return_t
IORegistryEntryGetProperty(
	io_registry_entry_t	entry,
	const io_name_t		name,
	io_struct_inband_t	buffer,
	uint32_t	      * size )
{
    return( io_registry_entry_get_property_bytes( entry, (char *) name,
						  buffer, size ));
}

kern_return_t
IORegistryEntrySetCFProperties(
	io_registry_entry_t	entry,
	CFTypeRef	 	properties )
{
    CFDataRef		data;
    kern_return_t	kr;
    kern_return_t	result;

    data = IOCFSerialize( properties, gIOKitLibSerializeOptions );
    if( !data)
	return( kIOReturnUnsupported );

    kr = io_registry_entry_set_properties( entry,
            (char *) CFDataGetBytePtr(data), CFDataGetLength(data),
            &result );

    CFRelease(data);

    if( KERN_SUCCESS == kr)
        kr = result;

    return( kr );
}

kern_return_t
IORegistryEntrySetCFProperty(
	io_registry_entry_t	entry,
        CFStringRef		propertyName,
	CFTypeRef	 	property )
{
    CFDictionaryRef	dict;
    kern_return_t	kr;

    dict = CFDictionaryCreate( kCFAllocatorDefault,
                                (const void **) &propertyName, (const void **) &property, 1,
                                &kCFTypeDictionaryKeyCallBacks,
                                &kCFTypeDictionaryValueCallBacks );
    if( !dict)
	return( kIOReturnNoMemory );

    kr = IORegistryEntrySetCFProperties( entry, dict );
    CFRelease( dict );

    return( kr );
}

kern_return_t
IORegistryEntryGetChildIterator(
	io_registry_entry_t	entry,
	const io_name_t		plane,
	io_iterator_t	      * iterator )
{
    return( io_registry_entry_get_child_iterator( entry,
						(char *) plane, iterator));
}

kern_return_t
IORegistryEntryGetParentIterator(
	io_registry_entry_t	entry,
	const io_name_t		plane,
	io_iterator_t	      * iterator )
{
    return( io_registry_entry_get_parent_iterator( entry,
						(char *) plane, iterator));
}

kern_return_t
IORegistryEntryGetChildEntry(
	io_registry_entry_t	entry,
	const io_name_t		plane,
	io_registry_entry_t   * child )
{
    kern_return_t       kr;
    io_iterator_t	iter;

    kr = IORegistryEntryGetChildIterator( entry, plane, &iter );

    if( KERN_SUCCESS == kr) {

	*child = IOIteratorNext( iter );
	IOObjectRelease( iter );
	if( MACH_PORT_NULL == *child)
	    kr = kIOReturnNoDevice;
    }

    return( kr );
}

kern_return_t
IORegistryEntryGetParentEntry(
	io_registry_entry_t	entry,
	const io_name_t		plane,
	io_registry_entry_t   * parent )
{
    kern_return_t       kr;
    io_iterator_t	iter;

    kr = IORegistryEntryGetParentIterator( entry, plane, &iter );

    if( KERN_SUCCESS == kr) {

	*parent = IOIteratorNext( iter );
	IOObjectRelease( iter );
	if( MACH_PORT_NULL == *parent)
	    kr = kIOReturnNoDevice;
    }

    return( kr );
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

kern_return_t
IOServiceOFPathToBSDName(mach_port_t	 masterPort,
                         const io_name_t openFirmwarePath,
                         io_name_t   	 bsdName)
{
    kern_return_t       kr;
    io_registry_entry_t service;
    io_iterator_t       services;

    // Initialize return values.

    bsdName[0] = 0;

    // Find objects matching the given open firmware name.

    kr = IOServiceGetMatchingServices(
             /* mach_port_t   */ masterPort,
             /* void *        */ IOOpenFirmwarePathMatching(
                                     /* mach_port_t  */ masterPort,
                                     /* uint32_t     */ 0,
                                     /* const char * */ openFirmwarePath ),
             /* io_iterator * */ &services );

    if( KERN_SUCCESS != kr )  return( kr );

    // Obtain the first (and presumably the only) match.

    service = IOIteratorNext( services );

    if( service ) {

        // Obtain the BSD name property from this object.

        uint32_t bsdNameSize = sizeof(io_name_t);

        kr = IORegistryEntryGetProperty(
                 /* mach_port_t        */ service,
                 /* io_name_t          */ kIOBSDNameKey,
                 /* io_struct_inband_t */ bsdName,
                 /* uint32_t *         */ &bsdNameSize);

        if( KERN_SUCCESS != kr )  bsdName[0] = 0;

        IOObjectRelease( service );
    }
    else {
        kr = KERN_FAILURE;
    }

    IOObjectRelease( services );

    return kr;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

kern_return_t
IOCatalogueSendData(
        mach_port_t		_masterPort,
        uint32_t                flag,
        const char             *buffer,
        uint32_t                size )
{
    kern_return_t kr;
    kern_return_t result;
    mach_port_t   masterPort;

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    kr = io_catalog_send_data( masterPort, flag,
                            (char *) buffer, size, &result );
    if( KERN_SUCCESS == kr)
        kr = result;

    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

    return( kr );
}

kern_return_t
IOCatalogueTerminate(
        mach_port_t		_masterPort,
        uint32_t                flag,
        io_name_t		description )
{
    kern_return_t	kr;
    mach_port_t		masterPort;

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    kr = io_catalog_terminate( masterPort, flag, description );

    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

    return( kr );
}

kern_return_t
IOCatalogueGetData(
        mach_port_t		_masterPort,
        uint32_t                flag,
        char                  **buffer,
        uint32_t               *size )
{
    kern_return_t	kr;
    mach_port_t		masterPort;

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    kr = io_catalog_get_data( masterPort, flag, (char **)buffer, (unsigned *)size );

    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

    return( kr );
}

kern_return_t
IOCatlogueGetGenCount(
        mach_port_t		_masterPort,
        uint32_t               *genCount )
{
    kern_return_t	kr;
    mach_port_t		masterPort;

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    kr = io_catalog_get_gen_count( masterPort, genCount );

    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

    return( kr );
}

kern_return_t
IOCatalogueModuleLoaded(
        mach_port_t		_masterPort,
        io_name_t               name )
{
    kern_return_t	kr;
    mach_port_t		masterPort;

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    kr = io_catalog_module_loaded( masterPort, name );

    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

    return( kr );
}

kern_return_t
IOCatalogueReset(
        mach_port_t		_masterPort,
	uint32_t		flag )
{
    kern_return_t	kr;
    mach_port_t		masterPort;

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    kr = io_catalog_reset(masterPort, flag);

    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

    return( kr );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// obsolete API

kern_return_t
IORegistryCreateEnumerator(
        mach_port_t	_masterPort,
	mach_port_t *	enumerator )
{
    kern_return_t	kr;
    mach_port_t		masterPort;

    if (MACH_PORT_NULL == _masterPort)
	masterPort = __IOGetDefaultMasterPort();
    else
	masterPort = _masterPort;

    kr = io_registry_create_iterator( masterPort,
                                         "IOService", true, enumerator );

    if ((masterPort != MACH_PORT_NULL) && (masterPort != _masterPort))
	mach_port_deallocate(mach_task_self(), masterPort);

    return( kr );
}

kern_return_t
IORegistryEnumeratorReset(
	mach_port_t	enumerator )
{
    return( io_iterator_reset( enumerator));
}


static io_object_t lastRegIter;

kern_return_t
IORegistryEnumeratorNextConforming(
	mach_port_t	enumerator,
	const char *	name,
	boolean_t recursive __unused )
{
    io_object_t	next 	= 0;

    while( (next = IOIteratorNext( enumerator ))) {

	if( IOObjectConformsTo( next, (char *) name )) 
	    break;

	IOObjectRelease( next );
    }

    lastRegIter = next;

    return( next ? kIOReturnSuccess : kIOReturnNoDevice );
}


#if 0
kern_return_t
IORegistryGetProperties(
	mach_port_t	enumerator ,
	void **		properties )
{
    return( IORegistryEntryGetProperties( lastRegIter,
		(struct IOObject **)properties));
}
#endif

kern_return_t
IOOpenConnection(
	mach_port_t enumerator __unused,
	task_port_t	owningTask,
	uint32_t	type,
	mach_port_t *	connect )
{
    kern_return_t	kr;

    kr = IOServiceOpen( lastRegIter,
	owningTask, type, connect );

    IOObjectRelease( lastRegIter );

    return( kr );
}


kern_return_t
IOCloseConnection(
	mach_port_t	connect )
{
    return( io_service_close( connect));
}


kern_return_t
IOSetNotificationPort(
	mach_port_t	connect,
	uint32_t	type,
	mach_port_t	port )
{
    return( io_connect_set_notification_port( connect, type, port, 0));
}
kern_return_t
IORegisterClient(
	mach_port_t	connect,
	mach_port_t	client )
{
    return( io_connect_add_client( connect, client));
}

kern_return_t
IORegistryDisposeEnumerator(
	io_enumerator_t	enumerator )
{
    return( IOObjectRelease( enumerator ));
}

/* -------------------------- */

kern_return_t
IOCompatibiltyNumber(
	mach_port_t connect __unused,
	uint32_t *	objectNumber )
{
	*objectNumber = 1;
	return( kIOReturnSuccess);
}


void
IOInitContainerClasses()
{
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* 32bit binary compatibility routines for deprecated APIs */
#if !defined(__LP64__)

// Compatability routines with 32bit IOKitLib
kern_return_t
IOConnectMethodScalarIScalarO( 
	io_connect_t	connect,
	uint32_t	index,
	IOItemCount	inCount,
	IOItemCount	scalarOutputCount,
	... )
{
    uint64_t		inData[6], outData[6];
    kern_return_t	err;
    uint32_t		i, outCount;
    va_list		val;

    if (inCount + scalarOutputCount > 6)
	return MIG_ARRAY_TOO_LARGE;

    va_start(val, scalarOutputCount);
    for (i = 0; i < inCount; i++)
	inData[i] = va_arg(val, uint32_t);

    outCount = scalarOutputCount;
    err = IOConnectCallScalarMethod(connect, index,
	    inData, inCount, outData, &outCount);

    if( kIOReturnSuccess == err) {
	for (i = 0; i < outCount; i++) {
	    uint32_t *out = va_arg(val, uint32_t *);
	    *out = (uint32_t) outData[i];
	}
    }
    va_end(val);

    return err;
}

kern_return_t
IOConnectMethodScalarIStructureO(
	io_connect_t	connect,
	uint32_t	index,
	IOItemCount	inCount,
	IOByteCount *	outSizeP,
	... )
{
    uint64_t		inData[6];
    void               *out = NULL;
    IOItemCount		i;
    va_list		val;

    if (inCount > 6)
	return MIG_ARRAY_TOO_LARGE;

    va_start(val, outSizeP);
    for (i = 0; i < inCount; i++)
	inData[i] = va_arg(val, uint32_t);
    if (outSizeP && *outSizeP)
	out = va_arg(val, void *);

    kern_return_t err = IOConnectCallMethod(connect, index,
	    inData, inCount, NULL, 0,
	    NULL,   0,       out,  outSizeP);

    va_end(val);

    return err;
}

kern_return_t
IOConnectMethodScalarIStructureI(
	io_connect_t	connect,
	uint32_t	index,
	IOItemCount	inCount,
	IOByteCount	inSize,
	... )
{
    uint64_t		inData[6];
    uint8_t            *in = NULL;
    va_list		val;

    if (inCount > 6)
	return MIG_ARRAY_TOO_LARGE;

    va_start(val, inSize);
    for (IOItemCount i = 0; i < inCount; i++)
	inData[i] = va_arg(val, uint32_t);
    if (inSize)
	in = va_arg(val, void *);

    kern_return_t err = IOConnectCallMethod(connect, index,
	    inData, inCount, in,   inSize,
	    NULL,   NULL,    NULL, NULL);

    va_end(val);

    return err;
}

kern_return_t
IOConnectMethodStructureIStructureO(
	io_connect_t	connect,
	uint32_t	index,
	IOItemCount	inSize,
	IOByteCount *	outSizeP,
	void *		in,
	void *		out )
{
    return IOConnectCallStructMethod(connect, index, in, inSize, out, outSizeP);
}

kern_return_t
IOMapMemory(
	io_connect_t	connect,
	uint32_t	memoryType,
	task_port_t	intoTask,
	vm_address_t *	atAddress,
	vm_size_t    *	ofSize,
	uint32_t	flags )
{
    return( io_connect_map_memory( connect, memoryType, intoTask,
	atAddress, ofSize, flags));
}

#if EMULATE_IOCONNECT_64 || EMULATE_IOCONNECT_ASYNC_64

// ILP32 - need to remap up to 64 bit scalars these are helpers
#define arrayCnt(type)	(sizeof(type)/sizeof(type[0]))
static void inflate_vec(uint64_t *dp, int d, int *sp, int s)
{
    if (d > s)
	d = s;

    for (int i = 0; i < d; i++)
	dp[i] = (uint32_t) sp[i];
}

static void deflate_vec(int *dp, int d, uint64_t *sp, int s)
{
    if (d > s)
	d = s;

    for (int i = 0; i < d; i++)
	dp[i] = (int) sp[i];
}
#endif // EMULATE_IOCONNECT_64 || EMULATE_IOCONNECT_ASYNC_64

#if EMULATE_IOCONNECT_64
kern_return_t io_connect_method
(
     mach_port_t connection,
     uint32_t selector,
     io_scalar_inband64_t input,
     mach_msg_type_number_t inputCnt,
     io_struct_inband_t inband_input,
     mach_msg_type_number_t inband_inputCnt,
     mach_vm_address_t ool_input,
     mach_vm_size_t  ool_input_size __unused,
     io_scalar_inband64_t output,
     mach_msg_type_number_t *outputCnt,
     io_struct_inband_t inband_output,
     mach_msg_type_number_t *inband_outputCnt,
     mach_vm_address_t ool_output,
     mach_vm_size_t * ool_output_size __unused
)
{
    if (ool_input || ool_output)
	return MIG_ARRAY_TOO_LARGE;

    if (inband_input && inband_output) {
	// io_connect_method_structureI_structureO
	return io_connect_method_structureI_structureO(connection, selector,
		inband_input, inband_inputCnt, inband_output, inband_outputCnt);
    }

    io_scalar_inband_t inData;
    deflate_vec(inData, arrayCnt(inData), input, inputCnt);

    if (inband_input) {
	// io_connect_method_scalarI_structureI
	return io_connect_method_scalarI_structureI(connection, selector,
		inData, inputCnt, inband_input, inband_inputCnt);
    }
    else if (inband_output) {
	// io_connect_method_scalarI_structureO
	return io_connect_method_scalarI_structureO(connection, selector,
		inData, inputCnt, inband_output, inband_outputCnt);
    }

    // io_connect_method_scalarI_scalarO
    kern_return_t rtn = io_connect_method_scalarI_scalarO(connection, selector,
	    inData, inputCnt, inData, outputCnt);
    inputCnt = (rtn == KERN_SUCCESS && outputCnt)? *outputCnt : 0;
    inflate_vec(output, inputCnt, inData, arrayCnt(inData));
    return rtn;
}
#endif // EMULATE_IOCONNECT_64

#if EMULATE_IOCONNECT_ASYNC_64
kern_return_t io_connect_async_method
(
    mach_port_t connection,
    mach_port_t wake_port,
    io_async_ref64_t reference,
    mach_msg_type_number_t referenceCnt,
    uint32_t selector,
    io_scalar_inband64_t input,
    mach_msg_type_number_t inputCnt,
    io_struct_inband_t inband_input,
    mach_msg_type_number_t inband_inputCnt,
    mach_vm_address_t ool_input,
    mach_vm_size_t  ool_input_size __unused,
    io_scalar_inband64_t output,
    mach_msg_type_number_t *outputCnt,
    io_struct_inband_t inband_output,
    mach_msg_type_number_t *inband_outputCnt,
    mach_vm_address_t ool_output,
    mach_vm_size_t * ool_output_size __unused
)
{
    if (ool_input || ool_output)
	return MIG_ARRAY_TOO_LARGE;

    io_async_ref_t refData;
    deflate_vec((int*)refData, arrayCnt(refData), reference, referenceCnt);

    if (inband_input && inband_output) {
	// io_async_method_structureI_structureO
	return io_async_method_structureI_structureO(
		connection, wake_port, refData, referenceCnt, selector,
		inband_input, inband_inputCnt, inband_output, inband_outputCnt);
    }

    io_scalar_inband_t inData;
    deflate_vec(inData, arrayCnt(inData), input, inputCnt);

    if (inband_input) {
	// io_async_method_scalarI_structureI
	return io_async_method_scalarI_structureI(
		connection, wake_port, refData, referenceCnt, selector,
		inData, inputCnt, inband_input, inband_inputCnt);
    }
    else if (inband_output) {
	// io_async_method_scalarI_structureO
	return io_async_method_scalarI_structureO(
		connection, wake_port, refData, referenceCnt, selector,
		inData, inputCnt, inband_output, inband_outputCnt);
    }

    // io_async_method_scalarI_scalarO
    kern_return_t rtn = io_async_method_scalarI_scalarO(
	    connection, wake_port, refData, referenceCnt, selector,
	    inData, inputCnt, inData, outputCnt);
    inputCnt = (rtn == KERN_SUCCESS && outputCnt)? *outputCnt : 0;
    inflate_vec(output, inputCnt, inData, arrayCnt(inData));
    return rtn;
}
#endif // EMULATE_IOCONNECT_ASYNC_64


#endif /* !__LP64__ */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

__private_extern__ IOReturn
readFile(const char *path, vm_address_t * objAddr, vm_size_t * objSize)
{
    int fd;
    int err;
    struct stat stat_buf;

    *objAddr = 0;
    *objSize = 0;

    if((fd = open(path, O_RDONLY)) == -1)
	return errno;

    do {
	if(fstat(fd, &stat_buf) == -1) {
	    err = errno;
	    continue;
	}
        if (0 == (stat_buf.st_mode & S_IFREG)) 
        {
            *objAddr = 0;
            *objSize = 0;
            err = kIOReturnNotReadable;
            continue;
        }
	*objSize = stat_buf.st_size;

	*objAddr = (vm_address_t) mmap(NULL, round_page(stat_buf.st_size), PROT_READ, MAP_FILE|MAP_PRIVATE, fd, 0);
	if(!*objAddr) {
            *objSize = 0;
	    err = errno;
	    continue;
	}

	err = kIOReturnSuccess;

    } while( false );

    close(fd);

    return( err );
}

__private_extern__ CFMutableDictionaryRef
readPlist( const char * path, UInt32 key __unused )
{
    IOReturn			err;
    vm_offset_t 		bytes;
    vm_size_t			byteLen;
    CFDataRef			data;
    CFMutableDictionaryRef	obj = 0;

    err = readFile( path, &bytes, &byteLen );

    if( kIOReturnSuccess != err) 
	return (0);
    
    data = CFDataCreateWithBytesNoCopy( kCFAllocatorDefault,
				(const UInt8 *) bytes, byteLen, kCFAllocatorNull );
    if( data) {
	obj = (CFMutableDictionaryRef) CFPropertyListCreateFromXMLData( kCFAllocatorDefault, data,
					    kCFPropertyListMutableContainers,
					    (CFStringRef *) NULL );
	CFRelease( data );
    }
    vm_deallocate( mach_task_self(), bytes, byteLen );

    return (obj);
}

__private_extern__ Boolean
writePlist( const char * path, CFMutableDictionaryRef dict, UInt32 key __unused )
{
    Boolean   result = false;
    CFDataRef data;
    CFIndex   length;
    int       fd = -1;

    data = CFPropertyListCreateXMLData(kCFAllocatorDefault, dict);

    if (data)
    {
	fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH));
	result = (fd >= 0);
	if (result)
	{
	    if ((length = CFDataGetLength(data)))
		result = (length == write(fd, CFDataGetBytePtr(data), length));
	    close(fd);
	}
	CFRelease(data);
    }

    return (result);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <IOKit/IOSharedLock.h>

boolean_t ev_try_lock(OSSpinLock * l)
{
    return OSSpinLockTry(l);
}

void ev_unlock(OSSpinLock * l)
{
    OSSpinLockUnlock(l);
}
