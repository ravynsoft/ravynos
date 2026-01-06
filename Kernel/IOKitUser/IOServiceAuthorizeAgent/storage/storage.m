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

#include "storage.h"

#include <DiskArbitration/DiskArbitration.h>
#include <IOKit/storage/IOBlockStorageDevice.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOStorageDeviceCharacteristics.h>
#include <IOKit/usb/USB.h>

static CFDictionaryRef __CopyDescription( io_service_t service )
{
    static dispatch_once_t once;
    static DASessionRef session;
    CFDictionaryRef description = 0;

    dispatch_once( &once, ^
    {
        session = DASessionCreate( kCFAllocatorDefault );
    } );

    if ( session )
    {
        DADiskRef disk;

        disk = DADiskCreateFromIOMedia( kCFAllocatorDefault, session, service );

        if ( disk )
        {
            description = DADiskCopyDescription( disk );

            CFRelease( disk );
        }
    }

    return description;
}

static CFStringRef __CopyMediaRelativePath( io_service_t service )
{
    io_service_t parent = 0;
    CFStringRef path = 0;

    IORegistryEntryGetParentEntry( service, kIOServicePlane, &parent );

    if ( parent )
    {
        CFStringRef base;
        char separator;

        if ( IOObjectConformsTo( parent, kIOBlockStorageDeviceClass ) )
        {
            base = CFSTR( kIOServicePlane );
            separator = ':';

            CFRetain( base );
        }
        else
        {
            base = __CopyMediaRelativePath( parent );
            separator = '/';
        }

        if ( base )
        {
            IOReturn status;
            io_name_t name;

            status = IORegistryEntryGetNameInPlane( service, kIOServicePlane, name );

            if ( status == kIOReturnSuccess )
            {
                io_name_t location;

                status = IORegistryEntryGetLocationInPlane( service, kIOServicePlane, location );

                if ( status == kIOReturnSuccess )
                {
                    path = CFStringCreateWithFormat( kCFAllocatorDefault, 0, CFSTR( "%@%c%s@%s" ), base, separator, name, location );
                }
                else
                {
                    path = CFStringCreateWithFormat( kCFAllocatorDefault, 0, CFSTR( "%@%c%s" ), base, separator, name );
                }
            }

            CFRelease( base );
        }

        IOObjectRelease( parent );
    }

    return path;
}

static CFComparisonResult __CompareVolumeDescription( const void * identifier1, const void * identifier2, void * context __unused )
{
    CFTypeRef value1;
    CFTypeRef value2;
    CFComparisonResult compare;

    value1 = CFDictionaryGetValue( identifier1, kDADiskDescriptionVolumeUUIDKey );
    value2 = CFDictionaryGetValue( identifier2, kDADiskDescriptionVolumeUUIDKey );

    if ( value1 && value2 == 0 )
    {
        compare = kCFCompareLessThan;
    }
    else if ( value1 == 0 && value2 )
    {
        compare = kCFCompareGreaterThan;
    }
    else
    {
        value1 = CFDictionaryGetValue( identifier1, kDADiskDescriptionVolumeNameKey );
        value2 = CFDictionaryGetValue( identifier2, kDADiskDescriptionVolumeNameKey );

        if ( value1 && value2 == 0 )
        {
            compare = kCFCompareLessThan;
        }
        else if ( value1 == 0 && value2 )
        {
            compare = kCFCompareGreaterThan;
        }
        else
        {
            value1 = CFDictionaryGetValue( identifier1, kDADiskDescriptionMediaSizeKey );
            value2 = CFDictionaryGetValue( identifier2, kDADiskDescriptionMediaSizeKey );

            compare = CFNumberCompare( value2, value1, 0 );

            if ( compare == kCFCompareEqualTo )
            {
                value1 = CFDictionaryGetValue( identifier1, kDADiskDescriptionMediaPathKey );
                value2 = CFDictionaryGetValue( identifier2, kDADiskDescriptionMediaPathKey );

                compare = CFStringCompare( value1, value2, 0 );
            }
        }
    }

    return compare;
}

static CFDictionaryRef __CopyVolumeDescription( io_service_t service )
{
    CFMutableArrayRef volumes;
    CFDictionaryRef volume = 0;

    volumes = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );

    if ( volumes )
    {
        io_iterator_t services = 0;

        IORegistryEntryCreateIterator( service, kIOServicePlane, kIORegistryIterateRecursively, &services );

        if ( services )
        {
            for ( ; ; IOIteratorReset( services ) )
            {
                service = IOIteratorNext( services );

                if ( service )
                {
                    IOObjectRelease( service );
                }

                while ( ( service = IOIteratorNext( services ) ) )
                {
                    if ( IOObjectConformsTo( service, kIOMediaClass ) )
                    {
                        CFDictionaryRef description;

                        description = __CopyDescription( service );

                        if ( description )
                        {
                            if ( CFDictionaryGetValue( description, kDADiskDescriptionMediaWholeKey ) == kCFBooleanTrue )
                            {
                                IORegistryIteratorExitEntry( services );
                            }
                            else
                            {
                                if ( CFDictionaryGetValue( description, kDADiskDescriptionVolumeKindKey ) )
                                {
                                    CFArrayAppendValue( volumes, description );
                                }
                            }

                            CFRelease( description );
                        }
                    }

                    IOObjectRelease( service );
                }

                if ( IOIteratorIsValid( services ) )
                {
                    break;
                }

                CFArrayRemoveAllValues( volumes );
            }

            IOObjectRelease( services );
        }

        if ( CFArrayGetCount( volumes ) )
        {
            CFArraySortValues( volumes, CFRangeMake( 0, CFArrayGetCount( volumes ) ), __CompareVolumeDescription, 0 );

            volume = CFArrayGetValueAtIndex( volumes, 0 );

            CFRetain( volume );
        }

        CFRelease( volumes );
    }

    return volume;
}

static Boolean __IsEqual( CFTypeRef object1, CFTypeRef object2 )
{
    Boolean equal;

    if ( object1 == NULL || object2 == NULL )
    {
        equal = ( object1 == object2 );
    }
    else
    {
        equal = CFEqual( object1, object2 );
    }

    return equal;
}

CFDictionaryRef _IOMediaCopyIdentifier( io_service_t service )
{
    CFDictionaryRef description;
    CFMutableDictionaryRef identifier = 0;

    description = __CopyDescription( service );

    if ( description )
    {
        identifier = IOServiceMatching( kIOMediaClass );

        if ( identifier )
        {
            CFTypeRef value;

            //
            // Device
            //

            value = CFDictionaryGetValue( description, kDADiskDescriptionDeviceGUIDKey );

            if ( value )
            {
                CFDictionarySetValue( identifier, kDADiskDescriptionDeviceGUIDKey, value );
            }

            value = CFDictionaryGetValue( description, kDADiskDescriptionDeviceModelKey );

            if ( value )
            {
                CFDictionarySetValue( identifier, kDADiskDescriptionDeviceModelKey, value );
            }

            value = CFDictionaryGetValue( description, kDADiskDescriptionDevicePathKey );

            CFDictionarySetValue( identifier, kDADiskDescriptionDevicePathKey, value );

            value = CFDictionaryGetValue( description, kDADiskDescriptionDeviceRevisionKey );

            if ( value )
            {
                CFDictionarySetValue( identifier, kDADiskDescriptionDeviceRevisionKey, value );
            }

            value = IORegistryEntrySearchCFProperty( service, kIOServicePlane, CFSTR( kIOPropertyProductSerialNumberKey ), kCFAllocatorDefault, kIORegistryIterateParents | kIORegistryIterateRecursively );

            if ( value )
            {
                CFDictionarySetValue( identifier, CFSTR( "DADeviceSerial" ), value );
            }
            else
            {
                value = IORegistryEntrySearchCFProperty( service, kIOServicePlane, CFSTR( kUSBSerialNumberString ), kCFAllocatorDefault, kIORegistryIterateParents | kIORegistryIterateRecursively );

                if ( value )
                {
                    CFDictionarySetValue( identifier, CFSTR( "DADeviceSerial" ), value );
                }
            }

            value = CFDictionaryGetValue( description, kDADiskDescriptionDeviceVendorKey );

            if ( value )
            {
                CFDictionarySetValue( identifier, kDADiskDescriptionDeviceVendorKey, value );
            }

            //
            // Media
            //

            value = CFDictionaryGetValue( description, kDADiskDescriptionMediaContentKey );
            
            CFDictionarySetValue( identifier, kDADiskDescriptionMediaContentKey, value );

            value = CFDictionaryGetValue( description, kDADiskDescriptionMediaKindKey );

            CFDictionarySetValue( identifier, kDADiskDescriptionMediaKindKey, value );

            value = CFDictionaryGetValue( description, kDADiskDescriptionMediaLeafKey );

            CFDictionarySetValue( identifier, kDADiskDescriptionMediaLeafKey, value );

            value = CFDictionaryGetValue( description, kDADiskDescriptionMediaNameKey );

            CFDictionarySetValue( identifier, kDADiskDescriptionMediaNameKey, value );

            value = __CopyMediaRelativePath( service );

            if ( value )
            {
                CFDictionarySetValue( identifier, kDADiskDescriptionMediaPathKey, value );

                CFRelease( value );
            }
            else
            {
                value = CFDictionaryGetValue( description, kDADiskDescriptionMediaPathKey );

                CFDictionarySetValue( identifier, kDADiskDescriptionMediaPathKey, value );
            }

            value = CFDictionaryGetValue( description, kDADiskDescriptionMediaRemovableKey );

            CFDictionarySetValue( identifier, kDADiskDescriptionMediaRemovableKey, value );

            value = CFDictionaryGetValue( description, kDADiskDescriptionMediaSizeKey );

            CFDictionarySetValue( identifier, kDADiskDescriptionMediaSizeKey, value );

            value = CFDictionaryGetValue( description, kDADiskDescriptionMediaUUIDKey );

            if ( value )
            {
                CFStringRef string;

                string = CFUUIDCreateString( kCFAllocatorDefault, value );

                if ( string )
                {
                    CFDictionarySetValue( identifier, kDADiskDescriptionMediaUUIDKey, string );

                    CFRelease( string );
                }
            }

            value = CFDictionaryGetValue( description, kDADiskDescriptionMediaWholeKey );

            CFDictionarySetValue( identifier, kDADiskDescriptionMediaWholeKey, value );

            //
            // Volume
            //

            if ( CFDictionaryGetValue( description, kDADiskDescriptionMediaWholeKey ) == kCFBooleanTrue )
            {
                if ( CFDictionaryGetValue( description, kDADiskDescriptionVolumeKindKey ) == 0 )
                {
                    CFDictionaryRef volume;

                    volume = __CopyVolumeDescription( service );

                    if ( volume )
                    {
                        CFRelease( description );

                        description = volume;
                    }
                }
            }

            value = CFDictionaryGetValue( description, kDADiskDescriptionVolumeKindKey );

            if ( value )
            {
                CFDictionarySetValue( identifier, kDADiskDescriptionVolumeKindKey, value );
            }

            value = CFDictionaryGetValue( description, kDADiskDescriptionVolumeNameKey );

            if ( value )
            {
                CFDictionarySetValue( identifier, kDADiskDescriptionVolumeNameKey, value );
            }

            value = CFDictionaryGetValue( description, kDADiskDescriptionVolumeUUIDKey );

            if ( value )
            {
                CFStringRef string;

                string = CFUUIDCreateString( kCFAllocatorDefault, value );

                if ( string )
                {
                    CFDictionarySetValue( identifier, kDADiskDescriptionVolumeUUIDKey, string );

                    CFRelease( string );
                }
            }
        }

        CFRelease( description );
    }

    return identifier;
}

CFStringRef _IOMediaCopyName( CFDictionaryRef identifier )
{
    CFStringRef name;

    name = CFDictionaryGetValue( identifier, kDADiskDescriptionVolumeNameKey );

    if ( name == 0 )
    {
        name = CFDictionaryGetValue( identifier, kDADiskDescriptionMediaNameKey );
    }

    if ( name )
    {
        CFRetain( name );
    }

    return name;
}

Boolean _IOMediaIsEqual( CFDictionaryRef identifier1, CFDictionaryRef identifier2 )
{
    Boolean equal = FALSE;

    if ( __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionVolumeKindKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionVolumeKindKey ) ) &&
         __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionVolumeNameKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionVolumeNameKey ) ) &&
         __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionVolumeUUIDKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionVolumeUUIDKey ) ) &&
         __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionMediaContentKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionMediaContentKey ) ) &&
         __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionMediaKindKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionMediaKindKey ) ) &&
         __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionMediaLeafKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionMediaLeafKey ) ) &&
         __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionMediaNameKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionMediaNameKey ) ) &&
         __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionMediaRemovableKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionMediaRemovableKey ) ) &&
         __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionMediaSizeKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionMediaSizeKey ) ) &&
         __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionMediaUUIDKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionMediaUUIDKey ) ) &&
         __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionMediaWholeKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionMediaWholeKey ) ) )
    {
        if ( CFDictionaryGetValue( identifier1, kDADiskDescriptionVolumeUUIDKey ) ||
             CFDictionaryGetValue( identifier1, kDADiskDescriptionMediaUUIDKey ) )
        {
            equal = TRUE;
        }
        else
        {
            if ( __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionDeviceGUIDKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionDeviceGUIDKey ) ) &&
                 __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionDeviceModelKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionDeviceModelKey ) ) &&
                 __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionDeviceRevisionKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionDeviceRevisionKey ) ) &&
                 __IsEqual( CFDictionaryGetValue( identifier1, CFSTR( "DADeviceSerial" ) ), CFDictionaryGetValue( identifier2, CFSTR( "DADeviceSerial" ) ) ) &&
                 __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionDeviceVendorKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionDeviceVendorKey ) ) )
            {
                if ( CFDictionaryGetValue( identifier1, kDADiskDescriptionDeviceGUIDKey ) ||
                     CFDictionaryGetValue( identifier1, CFSTR( "DADeviceSerial" ) ) )
                {
                    if ( __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionMediaPathKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionMediaPathKey ) ) )
                    {
                        equal = TRUE;
                    }
                }
                else
                {
                    if ( __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionMediaPathKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionMediaPathKey ) ) &&
                         __IsEqual( CFDictionaryGetValue( identifier1, kDADiskDescriptionDevicePathKey ), CFDictionaryGetValue( identifier2, kDADiskDescriptionDevicePathKey ) ) )
                    {
                        equal = TRUE;
                    }
                }
            }
        }
    }

    return equal;
}

Boolean _IOMediaIsValid( io_service_t service )
{
    CFDictionaryRef description;
    Boolean valid = FALSE;

    description = __CopyDescription( service );

    if ( description )
    {
        if ( CFDictionaryGetValue( description, kDADiskDescriptionDeviceInternalKey ) == kCFBooleanTrue )
        {
            if ( CFDictionaryGetValue( description, kDADiskDescriptionMediaRemovableKey ) == kCFBooleanFalse )
            {
                if ( CFDictionaryGetValue( description, kDADiskDescriptionVolumeUUIDKey ) )
                {
                    if ( CFEqual( CFDictionaryGetValue( description, kDADiskDescriptionMediaContentKey ), CFSTR( "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7" ) ) )
                    {
                        valid = TRUE;
                    }
                }
            }
            else
            {
                valid = TRUE;
            }
        }
        else
        {
            valid = TRUE;
        }

        CFRelease( description );
    }

    return valid;
}
