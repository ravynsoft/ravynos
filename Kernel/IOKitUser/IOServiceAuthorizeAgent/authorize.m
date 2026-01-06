/*
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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

#include "authorize.h"

#include "application.h"
#include "device.h"
#include "preferences.h"

#include <CoreFoundation/CFBundlePriv.h>
#include <IOKit/IOKitLibPrivate.h>

static IOReturn __Authorize( CFDictionaryRef deviceID, CFBundleRef bundle )
{
    CFStringRef device;
    IOReturn status;

    device = _DeviceCopyName( deviceID );

    if ( device )
    {

        CFStringRef application;

        application = CFBundleGetValueForInfoDictionaryKey( bundle, _kCFBundleDisplayNameKey );

        if ( application == 0 )
        {
            application = CFBundleGetValueForInfoDictionaryKey( bundle, kCFBundleNameKey );
        }

        if ( application )
        {
            CFMutableDictionaryRef dictionary;

            dictionary = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );

            if ( dictionary )
            {
                CFStringRef string;
                CFStringRef header;

                string = CFCopyLocalizedString( CFSTR( "%@ wants to access \"%@\"." ), 0 );
                header = CFStringCreateWithFormat( kCFAllocatorDefault, 0, string, application, device );
                CFRelease( string );

                if ( header )
                {
                    CFURLRef path;

                    path = CFBundleCopyBundleURL( CFBundleGetMainBundle( ) );

                    if ( path )
                    {
                        CFUserNotificationRef notification;

                        CFDictionarySetValue( dictionary, kCFUserNotificationAlertHeaderKey, header );
                        CFDictionarySetValue( dictionary, kCFUserNotificationAlertMessageKey, CFSTR( "Do you want to allow access to this device?" ) );
                        CFDictionarySetValue( dictionary, kCFUserNotificationAlternateButtonTitleKey, CFSTR( "Always Allow" ) );
                        CFDictionarySetValue( dictionary, kCFUserNotificationDefaultButtonTitleKey, CFSTR( "Allow" ) );
                        CFDictionarySetValue( dictionary, kCFUserNotificationLocalizationURLKey, path );
                        CFDictionarySetValue( dictionary, kCFUserNotificationOtherButtonTitleKey, CFSTR( "Deny" ) );

                        notification = CFUserNotificationCreate( kCFAllocatorDefault, 0, kCFUserNotificationCautionAlertLevel, 0, dictionary );

                        if ( notification )
                        {
                            int err;
                            CFOptionFlags response;

                            err = CFUserNotificationReceiveResponse( notification, 0, &response );

                            if ( err == 0 )
                            {
                                switch ( ( response & 0x3 ) )
                                {
                                    case kCFUserNotificationAlternateResponse:
                                    {
                                        status = kIOReturnNotFound;

                                        break;
                                    }

                                    case kCFUserNotificationDefaultResponse:
                                    {
                                        status = kIOReturnSuccess;

                                        break;
                                    }

                                    default:
                                    {
                                        status = kIOReturnNotPermitted;

                                        break;
                                    }
                                }
                            }
                            else
                            {
                                status = kIOReturnNoResources;
                            }

                            CFRelease( notification );
                        }
                        else
                        {
                            status = kIOReturnNoResources;
                        }

                        CFRelease( path );
                    }
                    else
                    {
                        status = kIOReturnNoMemory;
                    }

                    CFRelease( header );
                }
                else
                {
                    status = kIOReturnNoMemory;
                }

                CFRelease( dictionary );
            }
            else
            {
                status = kIOReturnNoMemory;
            }
        }
        else
        {
            status = kIOReturnAborted;
        }

        CFRelease( device );
    }
    else
    {
        status = kIOReturnUnsupported;
    }

    return status;
}

IOReturn _Authorize( io_service_t service, uint64_t options, pid_t processID, uint64_t authorizationID, const audit_token_t *auditToken )
{
    CFDictionaryRef deviceID;
    IOReturn status;

    deviceID = _DeviceCopyIdentifier( service );

    if ( deviceID )
    {

        CFBundleRef bundle;

        bundle = _ApplicationCopyBundle( processID );

        if (bundle)
        {
            CFStringRef applicationID;

            applicationID = _ApplicationCopyIdentifier( processID, auditToken );

            if ( applicationID )
            {
                CFArrayRef deviceIDs;

                status = kIOReturnNotFound;

                deviceIDs = _PreferencesCopyValue( applicationID );

                if ( deviceIDs )
                {
                    CFIndex count;
                    CFIndex index;

                    count = CFArrayGetCount( deviceIDs );

                    for ( index = 0; index < count; index++ )
                    {
                        CFDictionaryRef compare;

                        compare = ( void * ) CFArrayGetValueAtIndex( deviceIDs, index );

                        if ( _DeviceIsEqual( deviceID, compare ) )
                        {
                            status = kIOReturnSuccess;

                            break;
                        }
                    }
                }

                if ( status )
                {
                    if ( ( options & kIOServiceInteractionAllowed ) )
                    {
                        status = __Authorize( deviceID, bundle );

                        if ( status == kIOReturnNotFound )
                        {
                            _PreferencesAppendArrayValue( applicationID, deviceID );

                            status = kIOReturnSuccess;
                        }
                    }
                    else
                    {
                        status = kIOReturnNotPermitted;
                    }
                }

                if ( status == kIOReturnSuccess )
                {
                    status = _IOServiceSetAuthorizationID( service, authorizationID );
                }

                if ( deviceIDs )
                {
                    CFRelease( deviceIDs );
                }

                CFRelease( applicationID );
            }
            else
            {
                status = kIOReturnAborted;
            }

            CFRelease( bundle );
        }
        else
        {
            status = kIOReturnAborted;
        }

        CFRelease( deviceID );
    }
    else
    {
        status = kIOReturnUnsupported;
    }

    return status;
}
