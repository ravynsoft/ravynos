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

#include "usb.h"

#include <Foundation/Foundation.h>
#include <IOKit/usb/IOUSBLib.h>

@interface __USB : NSObject

@end

@implementation __USB

/*
 * pathForResource:ofType:inBundle
 * Stolen from UserNotificationCenter-30:localizedInfo.m
 *
 * Get the path to the localized resource of the right type
 */
- (NSString *)pathForResource:(NSString *)resource ofType:(NSString *)type inBundle:(NSBundle *)bundle
{
    NSString *userName = NSUserName(), *result = nil;
    NSArray *preferredLanguages = nil;
    
    if (!bundle) bundle = [NSBundle mainBundle];
    if (userName && ![userName isEqualToString:@""])
    {
        CFPropertyListRef languages = CFPreferencesCopyValue(CFSTR("AppleLanguages"), kCFPreferencesAnyApplication, (__bridge CFStringRef)userName, kCFPreferencesAnyHost);
        if (languages && CFGetTypeID(languages) != CFArrayGetTypeID()) {
            CFRelease(languages);
            languages = nil;
        }
        preferredLanguages = (__bridge NSArray *)languages;
    }
    if (bundle)
    {
        NSArray *bundleLocalizations = [bundle localizations], *preferredLocalizations = [NSBundle preferredLocalizationsFromArray:bundleLocalizations forPreferences:preferredLanguages];
        unsigned i;
        NSUInteger count = [preferredLocalizations count];
        for (i = 0; !result && i < count; i++)
        {
            result = [bundle pathForResource:resource ofType:type inDirectory:nil forLocalization:[preferredLocalizations objectAtIndex:i]];
        }
        if (!result)
        {
            NSString *developmentLocalization = [bundle developmentLocalization];
            if (developmentLocalization) {
                result = [bundle pathForResource:resource ofType:type inDirectory:nil forLocalization:developmentLocalization];
            }
        }
    }
    
    return result;
}

- (NSDictionary *)localizationDictForTable:(NSString *)table inBundle:(NSBundle *)bundle
{
    NSDictionary *result = nil;
    NSString *localizedStringsPath = [self pathForResource:(table ? table : @"Localizable") ofType:@"strings" inBundle:bundle];
    if (localizedStringsPath && ![localizedStringsPath isEqualToString:@""]) {
        result = [NSDictionary dictionaryWithContentsOfFile:localizedStringsPath];
    }
    
    return result;
}

- (NSString *)getLocalizedStringFromTableInBundle:(NSString *)string inBundle:(NSBundle *)bundle
{
	NSString *localizedString;
	NSString *tableName = @"Localizable";
	
	NSDictionary *dict = [self localizationDictForTable:tableName inBundle:bundle];
	localizedString = [dict objectForKey:string];
	
	if (localizedString == nil)
    {
		localizedString = [bundle localizedStringForKey: string
                                                  value: @"" // Blank is better than some weird error string
                                                  table: tableName];
    }
    
    return localizedString ? localizedString : string;
}

- (NSString *) getLocalizedNameForDeviceName:(NSString *)deviceName
{
    NSBundle            *usbFamilyBundle = [NSBundle bundleWithURL:[NSURL fileURLWithPath:@"/System/Library/Extensions/IOUSBFamily.kext"]];
    NSBundle            *spUSBReporterBundle = [NSBundle bundleWithURL:[NSURL fileURLWithPath:@"/System/Library/SystemProfiler/SPUSBReporter.spreporter"]];
    NSMutableString     *localizedName = nil;
    
    if (deviceName)
    {
        // SPUSBReporter has localization names for Apple's devices
        localizedName = (NSMutableString *)[self getLocalizedStringFromTableInBundle:deviceName inBundle:spUSBReporterBundle];
    }
    else
    {
        localizedName = (NSMutableString *)[self getLocalizedStringFromTableInBundle:@"kUSBDefaultUSBDeviceName" inBundle:usbFamilyBundle];
    }
    
    return localizedName;
}

- (BOOL) isDeviceAuthorizable:(io_service_t) usbDevice
{
    NSNumber *  bDeviceClass;
    NSNumber *  locationID;
    NSNumber *  builtIn = nil;
    BOOL        isRootHub = NO;
    
    // NOTE:  We will not allow the following devices to be authorized:
    //
    //        1.  Built-in devices (they will have a "Built-In" property. This is a temporary workaround for root hubs: locationID integerValue]) & 0x00ffffff)
    //        2.  USB Hub Devices
    
    builtIn = CFBridgingRelease(IORegistryEntryCreateCFProperty(usbDevice, CFSTR("Built-In"), kCFAllocatorDefault, 0));
    locationID = CFBridgingRelease(IORegistryEntryCreateCFProperty(usbDevice, CFSTR(kUSBDevicePropertyLocationID), kCFAllocatorDefault, 0));
    bDeviceClass = CFBridgingRelease(IORegistryEntryCreateCFProperty(usbDevice, CFSTR(kUSBDeviceClass), kCFAllocatorDefault, 0));

    isRootHub = ((((int)[locationID integerValue]) & 0x00ffffff) == 0);
    
    if ( ([builtIn isEqual: @(YES)])        ||
         (isRootHub)                        ||
         ([bDeviceClass integerValue] == kUSBHubClass)
        )
        return NO;
    else
        return YES;
}

// This method will check to see that all the fields in the first dicionary match the ones in the second dictionary.  It assumes the dictionaries
// are the identifier dictionaries

- (BOOL) areDevicesEqual:(NSDictionary *)firstDevice secondDevice:(NSDictionary *)secondDevice
{
    BOOL    areEqual = NO;
    
    // Both are USB devices, now check to see if the USB vid/pid/release are the same
    if ([[firstDevice objectForKey:@kUSBVendorID] isEqualToNumber:[secondDevice objectForKey:@kUSBVendorID]] &&
        [[firstDevice objectForKey:@kUSBProductID] isEqualToNumber:[secondDevice objectForKey:@kUSBProductID]] &&
        [[firstDevice objectForKey:@kUSBDeviceReleaseNumber] isEqualToNumber:[secondDevice objectForKey:@kUSBDeviceReleaseNumber]]
        )
    {
        // We need to check and see if we have a serial number.  If we don't, then look at the locationID and if the same, then assume the device is the same.
        if ( ([firstDevice objectForKey:@kUSBSerialNumberString] == nil) && ([secondDevice objectForKey:@kUSBSerialNumberString] == nil) )
        {
            if ( [[firstDevice objectForKey:@kUSBDevicePropertyLocationID] isEqualToNumber:[secondDevice objectForKey:@kUSBDevicePropertyLocationID]] )
            {
                // Same locationIDs
                areEqual = YES;
            }
        }
        else
        {
            // We have a serial number.  If we do, we will assume that it's the same device, regardless of the locationID.  This
            // will not catch devices where we have the same serial number.  If we want to ALWAYS include the locationID in the test, then we will need to modify
            // this testing
            if ( ([firstDevice objectForKey:@kUSBSerialNumberString] != nil) &&
                [[firstDevice objectForKey:@kUSBSerialNumberString] isEqualToString:[secondDevice objectForKey:@kUSBSerialNumberString]]
                )
            {
                // Same serial #'s
                areEqual = YES;
            }
        }
    }
    
    return areEqual;
}

@end

CFDictionaryRef _IOUSBDeviceCopyIdentifier( io_service_t service )
{
    CFMutableDictionaryRef properties = 0;
    CFMutableDictionaryRef identifier = 0;

    IORegistryEntryCreateCFProperties( service, &properties, kCFAllocatorDefault, 0 );

    if ( properties )
    {
        identifier = IOServiceMatching( kIOUSBDeviceClassName );

        if ( identifier )
        {
            CFTypeRef value;

            value = CFDictionaryGetValue( properties, CFSTR( kUSBDevicePropertyLocationID ) );

            CFDictionarySetValue( identifier, CFSTR( kUSBDevicePropertyLocationID ), value );

            value = CFDictionaryGetValue( properties, CFSTR( kUSBProductID ) );

            CFDictionarySetValue( identifier, CFSTR( kUSBProductID ), value );

            value = CFDictionaryGetValue( properties, CFSTR( kUSBProductString ) );

            if ( value )
            {
                CFDictionarySetValue( identifier, CFSTR( kUSBProductString ), value );
            }

            value = CFDictionaryGetValue( properties, CFSTR( kUSBDeviceReleaseNumber ) );

            CFDictionarySetValue( identifier, CFSTR( kUSBDeviceReleaseNumber ), value );

            value = CFDictionaryGetValue( properties, CFSTR( kUSBSerialNumberString ) );

            if ( value )
            {
                CFDictionarySetValue( identifier, CFSTR( kUSBSerialNumberString ), value );
            }

            value = CFDictionaryGetValue( properties, CFSTR( kUSBVendorID ) );

            CFDictionarySetValue( identifier, CFSTR( kUSBVendorID ), value );
        }

        CFRelease( properties );
    }

    return identifier;
}

CFStringRef _IOUSBDeviceCopyName( CFDictionaryRef identifier )
{
    CFStringRef name;

    name = CFDictionaryGetValue( identifier, CFSTR( kUSBProductString ) );

    return CFBridgingRetain( [ [ [ __USB alloc ] init ] getLocalizedNameForDeviceName: ( __bridge id ) name ] );
}

Boolean _IOUSBDeviceIsEqual( CFDictionaryRef identifier1, CFDictionaryRef identifier2 )
{
    return [ [ [ __USB alloc ] init ] areDevicesEqual: ( __bridge id ) identifier1 secondDevice: ( __bridge id ) identifier2 ];
}

Boolean _IOUSBDeviceIsValid( io_service_t service )
{
    return [ [ [ __USB alloc ] init ] isDeviceAuthorizable: service ];
}
