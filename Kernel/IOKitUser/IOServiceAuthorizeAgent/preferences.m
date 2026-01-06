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

#include "preferences.h"

dispatch_queue_t __PreferencesGetQueue( void )
{
    static dispatch_once_t once;
    static dispatch_queue_t queue;

    dispatch_once( &once, ^
    {
        queue = dispatch_queue_create( 0, 0 );
    } );

    return queue;
}

void _PreferencesAppendArrayValue( CFStringRef key, CFPropertyListRef value )
{
    dispatch_sync( __PreferencesGetQueue( ), ^
    {
        CFArrayRef array;
        CFMutableArrayRef mutableArray;

        array = _PreferencesCopyValue( key );

        if ( array )
        {
            mutableArray = CFArrayCreateMutableCopy( kCFAllocatorDefault, 0, array );

            CFRelease( array );
        }
        else
        {
            mutableArray = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
        }

        if ( mutableArray )
        {
            CFArrayAppendValue( mutableArray, value );

            _PreferencesSetValue( key, mutableArray );

            CFRelease( mutableArray );
        }
    } );
}

CFPropertyListRef _PreferencesCopyValue( CFStringRef key )
{
    return CFPreferencesCopyAppValue( key, kCFPreferencesCurrentApplication );
}

void _PreferencesSetValue( CFStringRef key, CFPropertyListRef value )
{
    CFPreferencesSetAppValue( key, value, kCFPreferencesCurrentApplication );

    CFPreferencesAppSynchronize( kCFPreferencesCurrentApplication );
}

