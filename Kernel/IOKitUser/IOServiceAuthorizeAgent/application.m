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

#include "application.h"

#include <libproc.h>
#include <CoreFoundation/CFBundlePriv.h>
#include <Security/Security.h>

dispatch_queue_t __ApplicationGetQueue( void )
{
    static dispatch_once_t once;
    static dispatch_queue_t queue;

    dispatch_once( &once, ^
    {
        queue = dispatch_queue_create( 0, 0 );
    } );

    return queue;
}

CFBundleRef _ApplicationCopyBundle( pid_t processID )
{
    __block CFBundleRef bundle = 0;

    dispatch_sync( __ApplicationGetQueue( ), ^
    {
        int length;
        uint8_t path[ PROC_PIDPATHINFO_MAXSIZE ];

        length = proc_pidpath( processID, path, PROC_PIDPATHINFO_MAXSIZE );

        if ( length )
        {
            CFURLRef url;

            url = CFURLCreateFromFileSystemRepresentation( kCFAllocatorDefault, path, length, TRUE );

            if ( url )
            {
                bundle = _CFBundleCreateWithExecutableURLIfMightBeBundle( kCFAllocatorDefault, url );

                CFRelease( url );
            }
        }
    } );

    return bundle;
}

CFStringRef _ApplicationCopyIdentifier( pid_t processID, const audit_token_t *auditToken )
{
    __block CFStringRef identifier = 0;

    dispatch_sync( __ApplicationGetQueue( ), ^
    {
        CFMutableDictionaryRef attributes;

        attributes = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );

        if ( attributes )
        {
            CFNumberRef number;

            number = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &processID );

            if ( number )
            {
                CFDataRef auditTokenData;

                auditTokenData = CFDataCreate(NULL, (const UInt8 *) auditToken, sizeof(*auditToken));

                if ( auditTokenData )
                {
                    SecCodeRef code = 0;

                    CFDictionarySetValue( attributes, kSecGuestAttributePid, number );
                    CFDictionarySetValue( attributes, kSecGuestAttributeAudit, auditTokenData );

                    SecCodeCopyGuestWithAttributes( 0, attributes, kSecCSDefaultFlags, &code );

                    if ( code )
                    {
                        CFDictionaryRef information = 0;

                        SecCodeCopySigningInformation( code, kSecCSDefaultFlags, &information );

                        if ( information )
                        {
                            identifier = CFDictionaryGetValue( information, kSecCodeInfoIdentifier );

                            if ( identifier )
                            {
                                CFRetain( identifier );
                            }

                            CFRelease( information );
                        }

                        CFRelease( code );
                    }

                    CFRelease(auditTokenData);
                }

                CFRelease( number );
            }

            CFRelease( attributes );
        }
    } );

    return identifier;
}
