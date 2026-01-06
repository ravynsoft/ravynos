/*
 * Copyright (c) 1998-2000 Apple Computer, Inc. All rights reserved.
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

#include <sys/cdefs.h>

#include <mach/mach.h>
#include <mach/thread_switch.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <libc.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFBundlePriv.h>

#include <IOKit/IOKitLib.h>
#include <libkern/OSByteOrder.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <IOKit/graphics/IOGraphicsLibPrivate.h>
#include <IOKit/graphics/IOGraphicsTypesPrivate.h>
#include <IOKit/graphics/IOGraphicsEngine.h>
#include <IOKit/platform/IOPlatformSupportPrivate.h>

#include "IOGraphicsLibInternal.h"

#define DEBUGPARAMS             0

#define SPOOF_EDID              0

#define arrayCount(x)	(sizeof(x) / sizeof(x[0]))

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

__private_extern__ IOReturn
readFile(const char *path, vm_address_t * objAddr, vm_size_t * objSize);
__private_extern__ CFMutableDictionaryRef
readPlist( const char * path, UInt32 key );
__private_extern__ Boolean
writePlist( const char * path, CFMutableDictionaryRef dict, UInt32 key __unused );

static char gIODisplayBoardID[256] = { 0 };

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void
setDictionaryDisplayIconValue(CFMutableDictionaryRef dst, CFDictionaryRef src)
{
    CFTypeRef value = NULL;
    if (CFDictionaryGetValueIfPresent(src, CFSTR("display-icon"), (const void**)&value))
        CFDictionarySetValue(dst, CFSTR("display-icon"), value);
}

static void
setDictionaryDisplayResolutionPreviewValues(CFMutableDictionaryRef dst, CFDictionaryRef src)
{
    CFTypeRef value = NULL;
    if (CFDictionaryGetValueIfPresent(src, CFSTR("display-resolution-preview-icon"), (const void**)&value))
        CFDictionarySetValue(dst, CFSTR("display-resolution-preview-icon"), value);
    if (CFDictionaryGetValueIfPresent(src, CFSTR("resolution-preview-x"), (const void**)&value))
        CFDictionarySetValue(dst, CFSTR("resolution-preview-x"), value);
    if (CFDictionaryGetValueIfPresent(src, CFSTR("resolution-preview-y"), (const void**)&value))
        CFDictionarySetValue(dst, CFSTR("resolution-preview-y"), value);
    if (CFDictionaryGetValueIfPresent(src, CFSTR("resolution-preview-width"), (const void**)&value))
        CFDictionarySetValue(dst, CFSTR("resolution-preview-width"), value);
    if (CFDictionaryGetValueIfPresent(src, CFSTR("resolution-preview-height"), (const void**)&value))
        CFDictionarySetValue(dst, CFSTR("resolution-preview-height"), value);
}

static void
MaxTimingRangeRec( IODisplayTimingRange * range )
{
    bzero( range, sizeof( IODisplayTimingRange) );

    range->supportedSyncFlags                   = 0xffffffff;
    range->supportedSignalLevels                = 0xffffffff;
    range->supportedSignalConfigs               = 0xffffffff;

    range->maxFrameRate                         = 0xffffffff;
    range->maxLineRate                          = 0xffffffff;
    range->maxPixelClock                        = 0xffffffff;
    range->maxPixelError                        = 0xffffffff;

    range->maxHorizontalTotal                   = 0xffffffff;
    range->maxVerticalTotal                     = 0xffffffff;
    range->maxHorizontalActiveClocks            = 0xffffffff;
    range->maxHorizontalBlankingClocks          = 0xffffffff;
    range->maxHorizontalSyncOffsetClocks        = 0xffffffff;
    range->maxHorizontalPulseWidthClocks        = 0xffffffff;
    range->maxVerticalActiveClocks              = 0xffffffff;
    range->maxVerticalBlankingClocks            = 0xffffffff;
    range->maxVerticalSyncOffsetClocks          = 0xffffffff;
    range->maxVerticalPulseWidthClocks          = 0xffffffff;
    range->maxHorizontalBorderLeft              = 0xffffffff;
    range->maxHorizontalBorderRight             = 0xffffffff;
    range->maxVerticalBorderTop                 = 0xffffffff;
    range->maxVerticalBorderBottom              = 0xffffffff;

    range->charSizeHorizontalActive             = 1;
    range->charSizeHorizontalBlanking           = 1;
    range->charSizeHorizontalSyncOffset         = 1;
    range->charSizeHorizontalSyncPulse          = 1;
    range->charSizeVerticalActive               = 1;
    range->charSizeVerticalBlanking             = 1;
    range->charSizeVerticalSyncOffset           = 1;
    range->charSizeVerticalSyncPulse            = 1;
    range->charSizeHorizontalBorderLeft         = 1;
    range->charSizeHorizontalBorderRight        = 1;
    range->charSizeVerticalBorderTop            = 1;
    range->charSizeVerticalBorderBottom         = 1;
    range->charSizeHorizontalTotal              = 1;
    range->charSizeVerticalTotal                = 1;
}

static Boolean
EDIDDescToDisplayTimingRangeRec( EDID * edid, EDIDGeneralDesc * desc,
                                 IODisplayTimingRange * range )
{
    UInt8 byte;

    if( !edid || (edid->version < 1) || (edid->revision < 1))
        return( false );

    if( desc->flag1 || desc->flag2 || desc->flag3)
        return( false );
    if( 0xfd != desc->type)
        return( false );

    MaxTimingRangeRec( range );

    byte = edid->displayParams[0];
    if (!(0x80 & byte))
    {
        range->supportedSignalLevels  = 1 << ((byte >> 5) & 3);
        range->supportedSyncFlags     = ((byte & 1) ? kIORangeSupportsVSyncSerration : 0)
                                      | ((byte & 2) ? kIORangeSupportsSyncOnGreen : 0)
                                      | ((byte & 4) ? kIORangeSupportsCompositeSync : 0)
                                      | ((byte & 8) ? kIORangeSupportsSeparateSyncs : 0);
    }

    range->supportedSignalConfigs = kIORangeSupportsInterlacedCEATiming;

    range->minVerticalPulseWidthClocks   = 1;
    range->minHorizontalPulseWidthClocks = 1;

    range->minFrameRate  = desc->data[0];
    range->maxFrameRate  = desc->data[1];
    range->minLineRate   = desc->data[2] * 1000;
    range->maxLineRate   = desc->data[3] * 1000;
    range->maxPixelClock = desc->data[4] * 10000000ULL;

    range->minHorizontalActiveClocks = 640;
    range->minVerticalActiveClocks   = 480;

    if( range->minLineRate)
        range->maxHorizontalActiveClocks = range->maxPixelClock / range->minLineRate;
    if( range->minFrameRate)
        range->maxVerticalActiveClocks   = range->maxPixelClock
                                            / (range->minHorizontalActiveClocks * range->minFrameRate);

    return( true );
}


static CFMutableDictionaryRef
IODisplayCreateOverrides( io_service_t framebuffer, IOOptionBits options,
                            IODisplayVendorID vendor, IODisplayProductID product,
                            UInt32 serialNumber __unused, 
                            uint32_t manufactureYear,
                            uint32_t manufactureWeek,
                            Boolean isDigital )
{

    char                        path[256];
    CFTypeRef                   obj = 0;
    CFMutableDictionaryRef      dict = 0;

#define DISPLAY_BUNDLE_PATH 	"/System/Library/Displays"

    static const char * overridesPath1 = DISPLAY_BUNDLE_PATH "/Contents/Resources/Overrides";
    static const char * overridesPath2 = DISPLAY_BUNDLE_PATH "/Overrides";

    const char * overridesPath = overridesPath1;

    if (0 != access(overridesPath, F_OK)) overridesPath = overridesPath2;

    if( 0 == (options & kIODisplayMatchingInfo)) {

        snprintf( path, sizeof(path), "%s"
                        "/" kDisplayVendorID "-%x"
                        "/" kDisplayProductID "-%x",
                        overridesPath, (unsigned) vendor, (unsigned) product );
    
        obj = readPlist( path, ((vendor & 0xffff) << 16) | (product & 0xffff) );

	if ((!obj) && manufactureYear && manufactureWeek)
	{
	    snprintf( path, sizeof(path), "%s"
			    "/" kDisplayVendorID "-%x"
			    "/" kDisplayYearOfManufacture "-%d"
			    "-" kDisplayWeekOfManufacture "-%d",
			    overridesPath,
			    (unsigned) vendor,
			    manufactureYear, manufactureWeek );
	    obj = readPlist( path, ((vendor & 0xffff) << 16) | (product & 0xffff) );
	}
        if (obj)
        {
            if( CFDictionaryGetTypeID() == CFGetTypeID( obj ))
            {
                dict = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, obj);
            }
            else if( CFArrayGetTypeID() == CFGetTypeID( obj ))
            {
                CFArrayRef      array;
                CFIndex         count, idx;
                CFTypeRef       obj2;
                CFDictionaryRef matching, candidate;

                // look for a matching override
                array = obj;
                candidate = 0;
                count = CFArrayGetCount(array);
                for (idx = 0; idx < count; idx++, candidate = 0)
                {
                    obj2 = CFArrayGetValueAtIndex(array, idx);
                    if (CFDictionaryGetTypeID() != CFGetTypeID(obj2))
                        continue;
                    candidate = obj2;
                    matching = CFDictionaryGetValue(candidate, CFSTR(kIODisplayOverrideMatchingKey));
                    if (!matching)
                        break;
                    if (CFDictionaryGetTypeID() != CFGetTypeID(matching))
                        continue;

                    obj2 = CFDictionaryGetValue(matching, CFSTR(kIODisplayIsDigitalKey));
                    if ((obj2 == kCFBooleanTrue) && !isDigital)
                        continue;
                    if ((obj2 == kCFBooleanFalse) && isDigital)
                        continue;

                    break;
                }
                if (candidate)
                    dict = CFDictionaryCreateMutableCopy( kCFAllocatorDefault, 0, candidate);
            }
            CFRelease( obj );
        }
    }
    if( !dict)
        dict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0,
                    &kCFTypeDictionaryKeyCallBacks,
                    &kCFTypeDictionaryValueCallBacks);

    if( dict) do {
        CFStringRef string;
        CFURLRef   url;
        CFBundleRef bdl;

        if((kIODisplayMatchingInfo | kIODisplayNoProductName) & options)
            continue;

        snprintf( path, sizeof(path), DISPLAY_BUNDLE_PATH);
//                            "/" kDisplayVendorID "-%lx", vendor );
    
        string = CFStringCreateWithCString( kCFAllocatorDefault, path,
                                            kCFStringEncodingMacRoman );
        if( !string)
            continue;
        url = CFURLCreateWithFileSystemPath( kCFAllocatorDefault, string, 
        									 kCFURLPOSIXPathStyle, true );
        CFRelease(string);
        if( !url)
            continue;
        bdl = CFBundleCreate( kCFAllocatorDefault, url);
        if( bdl) {
            CFDictionarySetValue( dict, CFSTR(kDisplayBundleKey), bdl);
            CFRelease(bdl);
        }
        CFRelease(url);
    
    } while( false );

    if (dict) {
        if (gIODisplayBoardID[0] == 0) {
            io_registry_entry_t ioRegRoot = IORegistryGetRootEntry(kIOMasterPortDefault);
            if (ioRegRoot) {
                CFDataRef boardId = (CFDataRef) IORegistryEntrySearchCFProperty(ioRegRoot,
                                                                                kIOServicePlane,
                                                                                CFSTR("board-id"),
                                                                                kCFAllocatorDefault,
                                                                                kIORegistryIterateRecursively);
                IOObjectRelease(ioRegRoot);
                if (boardId) {
                	size_t len = CFDataGetLength(boardId);
                	if (len > sizeof(gIODisplayBoardID)) len = sizeof(gIODisplayBoardID);
                    strlcpy(gIODisplayBoardID, (const char *) CFDataGetBytePtr(boardId), len);
                    CFRelease(boardId);
                }
            }
        }
        
        char enclosureColor[8] = { 0 };
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        
        IOReturn ret = IOPlatformGetDeviceColor(kIOPlatformDeviceEnclosureColorKey, &r, &g, &b);
        
        if (ret == kIOReturnSuccess)
            snprintf(enclosureColor, sizeof(enclosureColor), "-%x%x%x", r, g, b);
        
        CFDataRef builtin = (CFDataRef) IORegistryEntryCreateCFProperty(framebuffer,
                                                                        CFSTR(kIOFBBuiltInKey),
                                                                        kCFAllocatorDefault, kNilOptions);
        
        CFMutableDictionaryRef iconDict = NULL;
        snprintf(path, sizeof(path), "%s"
		 "/" "Icons.plist",
		 overridesPath);
        if (access(path, F_OK) == 0) iconDict = readPlist(path, 0);
        
        CFStringRef vendorIdString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%x"), (unsigned int)vendor);
        CFStringRef productIdString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%x"), (unsigned int)product);
        CFStringRef productIdWithColorString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%x%s"), (unsigned int)product, enclosureColor);
        CFStringRef modelString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%s"), gIODisplayBoardID);
        CFStringRef modelWithColorString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%s%s"), gIODisplayBoardID, enclosureColor);
        
        snprintf(path, sizeof(path), "%s"
                 "/" kDisplayVendorID "-%x"
                 "/" kDisplayProductID "-%x-%s%s.icns",
                 overridesPath,
                 (unsigned)vendor, (unsigned)product, gIODisplayBoardID, enclosureColor);
        
        Boolean foundIcon = false;
        if (access(path, F_OK) == 0)
            foundIcon = true;
        
        if (!foundIcon) {
            snprintf(path, sizeof(path), "%s"
                     "/" kDisplayVendorID "-%x"
                     "/" kDisplayYearOfManufacture "-%d"
                     "-" kDisplayWeekOfManufacture "-%d-%s%s.icns",
                     overridesPath,
                     (unsigned)vendor,
                     manufactureYear, manufactureWeek, gIODisplayBoardID, enclosureColor);
            
            if (access(path, F_OK) == 0)
                foundIcon = true;
        }
        
        CFStringRef productModelDisplayIconFilePath = NULL;
        if (foundIcon)
            productModelDisplayIconFilePath = CFStringCreateWithFileSystemRepresentation(kCFAllocatorDefault, path);
        
        CFStringRef modelDisplayIconFilePath = NULL;
        if (builtin) {
            snprintf(path, sizeof(path), "%s/Models/%s%s.icns", overridesPath, gIODisplayBoardID, enclosureColor);
            
            if (access(path, F_OK) == 0)
                modelDisplayIconFilePath = CFStringCreateWithFileSystemRepresentation(kCFAllocatorDefault, path);
        }
        
        snprintf(path, sizeof(path), "%s"
                 "/" kDisplayVendorID "-%x"
                 "/" kDisplayProductID "-%x%s.icns",
                 overridesPath, 
                 (unsigned)vendor, (unsigned)product, enclosureColor);
        
        foundIcon = false;
        if (access(path, F_OK) == 0)
            foundIcon = true;
        
        if (!foundIcon) {
            snprintf(path, sizeof(path), "%s"
                     "/" kDisplayVendorID "-%x"
                     "/" kDisplayYearOfManufacture "-%d"
                     "-" kDisplayWeekOfManufacture "-%d%s.icns",
                     overridesPath,
                     (unsigned)vendor,
                     manufactureYear, manufactureWeek, enclosureColor);
            
            if (access(path, F_OK) == 0)
                foundIcon = true;
        }
        
        CFStringRef productDisplayIconFilePath = NULL;
        if (foundIcon)
            productDisplayIconFilePath = CFStringCreateWithFileSystemRepresentation(kCFAllocatorDefault, path);

        
        snprintf(path, sizeof(path), "%s" 
        	"/" kDisplayVendorID "-%x.icns",
                 overridesPath,
                 (unsigned)vendor);
        
        CFStringRef vendorDisplayIconFilePath = NULL;
        if (access(path, F_OK) == 0)
            vendorDisplayIconFilePath = CFStringCreateWithFileSystemRepresentation(kCFAllocatorDefault, path);
    
        CFMutableDictionaryRef displayDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

        CFMutableDictionaryRef modelIdsDict = NULL;
        if (iconDict && builtin && CFDictionaryGetValueIfPresent(iconDict, CFSTR("board-ids"), (const void**)&modelIdsDict)) {
            CFMutableDictionaryRef modelDict = NULL;
            if (CFDictionaryGetValueIfPresent(modelIdsDict, modelWithColorString, (const void**)&modelDict)
                || CFDictionaryGetValueIfPresent(modelIdsDict, modelString, (const void**)&modelDict)) {
                setDictionaryDisplayIconValue(displayDict, modelDict);
                setDictionaryDisplayResolutionPreviewValues(displayDict, modelDict);
            }
        }
        
        if (builtin && !CFDictionaryContainsKey(displayDict, CFSTR("display-icon")) && modelDisplayIconFilePath)
            CFDictionarySetValue(dict, CFSTR("display-icon"), vendorDisplayIconFilePath);

        CFMutableDictionaryRef vendorIdsDict = NULL;
        if (iconDict && CFDictionaryGetValueIfPresent(iconDict, CFSTR("vendors"), (const void**)&vendorIdsDict)) {
            CFMutableDictionaryRef vendorDict = NULL;
            if (CFDictionaryGetValueIfPresent(vendorIdsDict, vendorIdString, (const void**)&vendorDict)) {
                CFMutableDictionaryRef displayIdsDict = NULL;
                if (CFDictionaryGetValueIfPresent(vendorDict, CFSTR("products"), (const void**)&displayIdsDict)) {
                    CFMutableDictionaryRef deviceDict = NULL;
                    if (CFDictionaryGetValueIfPresent(displayIdsDict, productIdWithColorString, (const void**)&deviceDict)
                        || CFDictionaryGetValueIfPresent(displayIdsDict, productIdString, (const void**)&deviceDict)) {
                        if (builtin && CFDictionaryGetValueIfPresent(deviceDict, CFSTR("board-ids"), (const void**)&modelIdsDict)) {
                            CFMutableDictionaryRef modelDict = NULL;
                            if (CFDictionaryGetValueIfPresent(modelIdsDict, modelWithColorString, (const void**)&modelDict)
                                || CFDictionaryGetValueIfPresent(modelIdsDict, modelString, (const void**)&modelDict)) {
                                setDictionaryDisplayIconValue(displayDict, modelDict);
                                setDictionaryDisplayResolutionPreviewValues(displayDict, modelDict);
                                
                                if (!CFDictionaryContainsKey(displayDict, CFSTR("display-icon")) && productModelDisplayIconFilePath)
                                    CFDictionarySetValue(displayDict, CFSTR("display-icon"), productModelDisplayIconFilePath);
                            }
                        }
                        
                        if (!builtin) {
                            if (!CFDictionaryContainsKey(displayDict, CFSTR("display-icon")))
                                setDictionaryDisplayIconValue(displayDict, deviceDict);
                            
                            if (!CFDictionaryContainsKey(displayDict, CFSTR("display-icon")) && productDisplayIconFilePath)
                                CFDictionarySetValue(displayDict, CFSTR("display-icon"), productDisplayIconFilePath);
                        }
                        
                        if (!CFDictionaryContainsKey(displayDict, CFSTR("display-resolution-preview-icon")))
                            setDictionaryDisplayResolutionPreviewValues(displayDict, deviceDict);
                    }
                }
                
                if (!builtin) {
                    if (!CFDictionaryContainsKey(displayDict, CFSTR("display-icon")) && productDisplayIconFilePath)
                        CFDictionarySetValue(displayDict, CFSTR("display-icon"), productDisplayIconFilePath);
                    
                    if (!CFDictionaryContainsKey(displayDict, CFSTR("display-icon")))
                        setDictionaryDisplayIconValue(displayDict, vendorDict);

                    if (!CFDictionaryContainsKey(displayDict, CFSTR("display-icon")) && vendorDisplayIconFilePath)
                        CFDictionarySetValue(displayDict, CFSTR("display-icon"), vendorDisplayIconFilePath);
                }
                
                if (!CFDictionaryContainsKey(displayDict, CFSTR("display-resolution-preview-icon")))
                    setDictionaryDisplayResolutionPreviewValues(displayDict, vendorDict);
            }
            
            if (!builtin) {
                if (!CFDictionaryContainsKey(displayDict, CFSTR("display-icon")) && productDisplayIconFilePath)
                    CFDictionarySetValue(displayDict, CFSTR("display-icon"), productDisplayIconFilePath);
                
                if (!CFDictionaryContainsKey(displayDict, CFSTR("display-icon")) && vendorDisplayIconFilePath)
                    CFDictionarySetValue(displayDict, CFSTR("display-icon"), vendorDisplayIconFilePath);
                
                if (!CFDictionaryContainsKey(displayDict, CFSTR("display-icon")))
                    setDictionaryDisplayIconValue(displayDict, vendorIdsDict);
            }
        }
        
        setDictionaryDisplayIconValue(dict, displayDict);
        setDictionaryDisplayResolutionPreviewValues(dict, displayDict);

        CFRelease(displayDict);
      
        if (iconDict)
          CFRelease(iconDict);
        
        if (builtin)
            CFRelease(builtin);
            
        if (vendorDisplayIconFilePath)
            CFRelease(vendorDisplayIconFilePath);
            
        if (productDisplayIconFilePath)
            CFRelease(productDisplayIconFilePath);
            
        if (modelDisplayIconFilePath)
            CFRelease(modelDisplayIconFilePath);
        
        if (productModelDisplayIconFilePath)
            CFRelease(productModelDisplayIconFilePath);
        
        CFRelease(modelWithColorString);
        CFRelease(modelString);
        CFRelease(productIdWithColorString);
        CFRelease(productIdString);
        CFRelease(vendorIdString);
    }

    return( dict );
}

static void
EDIDInfo( struct EDID * edid,
            IODisplayVendorID * vendor, IODisplayProductID * product,
            UInt32 * serialNumber,
            uint32_t * manufactureYear, uint32_t * manufactureWeek,
            Boolean * isDigital )
{
    SInt32              sint;

    if (vendor)
        *vendor = (edid->vendorProduct[0] << 8) | edid->vendorProduct[1];
    if (product)
        *product = (edid->vendorProduct[3] << 8) | edid->vendorProduct[2];
    if (isDigital)
        *isDigital = (0 != (0x80 & edid->displayParams[0]));

    if( serialNumber) {
        sint = (edid->serialNumber[3] << 24)
             | (edid->serialNumber[2] << 16)
             | (edid->serialNumber[1] << 8)
             | (edid->serialNumber[0]);
        if( sint == 0x01010101)
            sint = 0;
        *serialNumber = sint;
    }

    if (manufactureYear) *manufactureYear = edid->yearOfManufacture + 1990;
    if (manufactureWeek) *manufactureWeek = edid->weekOfManufacture;
}

__private_extern__ Boolean
IODisplayEDIDName( EDID * edid, char * name )
{
    char *      oname = name;
    EDIDDesc *  desc;
    int         i,j;
    Boolean     ok;
    char        c;

    if( !edid || (edid->version < 1) || (edid->revision < 1))
        return( false );

    desc = edid->descriptors;
    for( i = 0; i < 4; i++, desc++) {
        if( desc->general.flag1 || desc->general.flag2 || desc->general.flag3)
            continue;
        if( 0xfc != desc->general.type)
            continue;

        for( j = 0; j < (int) sizeof(desc->general.data); j++) {
            c = desc->general.data[j];
            if( c != 0x0a)
                *oname++ = c;
            else
                break;
        }
    }
    ok = (oname != name);
    if( ok)
        *oname++ = 0;

    return( ok );
}

struct MakeOneLocalContext {
    CFBundleRef            bdl;
    CFMutableDictionaryRef dict;
    CFStringRef            key;
};

static void MakeOneLocalization( const void * item, void * context )
{
    struct MakeOneLocalContext * ctx = (struct MakeOneLocalContext *) context;
    CFStringRef         value = NULL;
    CFDictionaryRef     stringTable = NULL;
    CFURLRef            url;
    CFDataRef           tableData = NULL;
    CFStringRef         errStr;
    SInt32              errCode;

    url = CFBundleCopyResourceURLForLocalization( ctx->bdl,
                                CFSTR("Localizable"), CFSTR("strings"), NULL, item );
    if (url && CFURLCreateDataAndPropertiesFromResource( kCFAllocatorDefault,
                                url, &tableData, NULL, NULL, &errCode)) {
        stringTable = CFPropertyListCreateFromXMLData( kCFAllocatorDefault,
                                tableData, kCFPropertyListImmutable, &errStr);
        if (errStr)
            CFRelease( errStr);
        CFRelease( tableData);
    }
    if( url)
        CFRelease(url);
    if( stringTable)
        value = CFDictionaryGetValue(stringTable, ctx->key);
    if (!value)
        value = ctx->key;

    {
        SInt32           languageCode, regionCode, scriptCode;
        CFStringEncoding stringEncoding;
        if( CFBundleGetLocalizationInfoForLocalization( item, &languageCode, &regionCode,
                                                        &scriptCode, &stringEncoding )) {
            item = CFBundleCopyLocalizationForLocalizationInfo( languageCode, regionCode,
                                                        scriptCode, stringEncoding );
        } else
            item = CFRetain(item);
    }

    CFDictionarySetValue( ctx->dict, item, value );
    CFRelease( item );

    if( stringTable)
        CFRelease( stringTable );
}

static void GenerateProductName( CFMutableDictionaryRef dict,
                                        EDID * edid, SInt32 displayType, IOOptionBits options )
{
    CFStringRef         key;
    CFBundleRef         bdl;
    CFArrayRef          localizations;
    struct MakeOneLocalContext ctx;
    static const char * type2Name[] = {
        NULL,                           // 000 kUnknownConnect
        NULL,                           // 001 kUnknownConnect
        "Color LCD",                    // 002 kPanelTFTConnect
        NULL,                           // 003 kFixedModeCRTConnect
        "Multiple Scan Display",        // 004 kMultiModeCRT1Connect
        "Multiple Scan Display",        // 005 kMultiModeCRT2Connect
        "Multiple Scan Display",        // 006 kMultiModeCRT3Connect
        "Multiple Scan Display",        // 007 kMultiModeCRT4Connect
        NULL,                           // 008 kModelessConnect
        "Full-Page Display",            // 009 kFullPageConnect
        "VGA Display",                  // 010 kVGAConnect
        "Television",                   // 011 kNTSCConnect
        "Television",                   // 012 kPALConnect
        NULL,                           // 013 kHRConnect
        "Color LCD",                    // 014 kPanelFSTNConnect
        "Two-Page Display",             // 015 kMonoTwoPageConnect
        "Two-Page Display",             // 016 kColorTwoPageConnect
        NULL,                           // 017 kColor16Connect
        NULL,                           // 018 kColor19Connect
        NULL,                           // 019 kGenericCRT
        "Color LCD",                    // 020 kGenericLCD
        NULL,                           // 021 kDDCConnect
        NULL                            // 022 kNoConnect
    };

    key = CFDictionaryGetValue( dict, CFSTR(kDisplayProductName));
    if( key) {
        if( CFStringGetTypeID() != CFGetTypeID( key ))
            return;
        CFRetain(key);
    }
    bdl = (CFBundleRef) CFDictionaryGetValue( dict, CFSTR(kDisplayBundleKey));

    if( !key) {
        char sbuf[ 128 ];
        const char * name = NULL;

        if( IODisplayEDIDName(edid, sbuf))
            name = sbuf;
        else if (edid)
            name = "Unknown Display";
        else {

            if( displayType < (int) (sizeof( type2Name) / sizeof(type2Name[0])))
                name = type2Name[displayType];
            if( !name)
                name = "Unknown Display";
        }

        key = CFStringCreateWithCString( kCFAllocatorDefault, name,
                                            kCFStringEncodingMacRoman );
        if( !key)
            return;
    }

    if( bdl) {
        localizations = CFBundleCopyBundleLocalizations( bdl);
        if (localizations)
        {
            ctx.bdl = bdl;
            ctx.dict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0,
                            &kCFTypeDictionaryKeyCallBacks,
                            &kCFTypeDictionaryValueCallBacks);
            ctx.key = key;
    
            if( kIODisplayOnlyPreferredName & options) {
                CFArrayRef temp = localizations;
                localizations = CFBundleCopyPreferredLocalizationsFromArray( temp );
                CFRelease( temp );
            }

            CFArrayApplyFunction( localizations,
                                CFRangeMake(0, CFArrayGetCount(localizations)),
                                &MakeOneLocalization,
                                &ctx);
            CFDictionarySetValue( dict, CFSTR(kDisplayProductName), ctx.dict);
        
            CFRelease( localizations );
            CFRelease( ctx.dict );
        }
    }
    CFRelease( key );
}

io_service_t
IODisplayForFramebuffer(
        io_service_t            framebuffer,
        IOOptionBits    options __unused )
{
    IOReturn            kr;
    io_iterator_t       iter;
    io_service_t        service = 0;

    if( IOObjectConformsTo( framebuffer, "IODisplay"))
    {
        IOObjectRetain(framebuffer);
        return( framebuffer );
    }

    kr = IORegistryEntryCreateIterator( framebuffer, kIOServicePlane,
                                        kIORegistryIterateRecursively, &iter);
    if( kr != kIOReturnSuccess )
        return( 0 );

    do
    {
        for( ;
            (service = IOIteratorNext( iter));
            IOObjectRelease(service)) {
    
            if( IOObjectConformsTo( service, "IODisplay"))
                break;
        }
    }
    while (!service && !IOIteratorIsValid(iter) && (IOIteratorReset(iter), true));
    IOObjectRelease( iter );

    return( service );
}

enum {
    /* Used by default calibrator (should we show brightness panel) */
    kDisplayGestaltBrightnessAffectsGammaMask   = (1 << 0),
    kDisplayGestaltViewAngleAffectsGammaMask    = (1 << 1)
};

static void 
IODisplayDictAddValues(const void *key, const void *value, void *context)
{
    CFMutableDictionaryRef dict = context;
	// add if not exists
    CFDictionaryAddValue(dict, key, value);
}

static Boolean
IODisplayIsHDMISink(CFMutableDictionaryRef dict)
{
    CFDataRef   data = 0;
    EDID *      edid = 0;
    CFIndex     count;
    UInt8 *     blocks;

    if (! dict) {
        return false;
    }

    data = CFDictionaryGetValue(dict, CFSTR(kIODisplayEDIDKey));
    edid = (EDID *) CFDataGetBytePtr(data);
    if (! (data && edid)) {
        return false;
    }

    // limit to HDMI 1.3
    if ((edid->version != 1) || (edid->revision != 3)) {
        return false;
    }

    count = CFDataGetLength(data);
    if ((size_t)count <= sizeof(EDID)) {
        return false;
    }

    blocks = (UInt8 *)(edid + 1);
    count -= sizeof(EDID);

    while (count >= 128) {
        UInt8 tag = blocks[0];

        if (tag == kExtTagCEA) {
            CEA861EXT * ext = (CEA861EXT *)blocks;
            IOByteCount offset;

            offset = ext->detailedTimingsOffset;
            if (offset < 4) {
                return false;
            }
            offset -= 4;

            if (0x03 <= ext->version) {
                IOByteCount index = 0;
                while (index < offset) {
                    IOByteCount length = (ext->data[index] & 0x1f) + 1;

                    if (((ext->data[index] & 0xe0) == 0x60) && (length >= 4)) {
                        // HDMI Vendor Specific Data Block (HDMI 1.3a p.119)
                        if ((ext->data[index + 1] == 0x03) &&
                            (ext->data[index + 2] == 0x0c) &&
                            (ext->data[index + 3] == 0x00)) {
                            // 24-bit IEEE Registration Identifier (0x000C03)
                            return true;
                        }
                    }

                    index += length;
                }
            }
        }

        count  -= 128;
        blocks += 128;
    }

    return false;
}

#ifndef kIODisplayIsHDMISinkKey
#define kIODisplayIsHDMISinkKey "IODisplayIsHDMISink"
#endif

static CFDictionaryRef
_IODisplayCreateInfoDictionary(
    io_service_t                framebuffer,
    IOOptionBits                options )
{
    IOReturn                    kr;
    io_service_t                service = 0;
    Boolean                     isDigital = false;
    CFDataRef                   data = 0;
    CFNumberRef                 num;
    CFMutableDictionaryRef      dict = 0;
    CFMutableDictionaryRef      regDict;
    CFMutableDictionaryRef      fbRegDict;
    CFTypeRef                   obj;
    SInt32                      sint;
    UInt8                       low;
    float                       fnum;
    EDID *                      edid = 0;
    IODisplayVendorID           vendor = 0;
    IODisplayProductID          product = 0;
    SInt32                      displayType = 0;
    UInt32                      serialNumber = 0;
    uint32_t                    manufactureYear = 0;
    uint32_t                    manufactureWeek = 0;
    io_string_t                 path;
    int                         i;
    IODisplayTimingRange        displayRange;

    if( !(service = IODisplayForFramebuffer( framebuffer, options))) {
        dict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0,
                                            &kCFTypeDictionaryKeyCallBacks,
                                            &kCFTypeDictionaryValueCallBacks );
        if( dict)
            CFDictionarySetValue( dict, CFSTR(kIODisplayLocationKey), CFSTR("unknown"));
        return( dict );
    }

    do {

        regDict = 0;
        kr = IORegistryEntryCreateCFProperties( service, &regDict,
                                                kCFAllocatorDefault, kNilOptions );
        if( kr != kIOReturnSuccess)
            continue;

        num = CFDictionaryGetValue( regDict, CFSTR(kDisplayVendorID) );
        if( num)
            CFNumberGetValue( num, kCFNumberSInt32Type, &vendor );
        num = CFDictionaryGetValue( regDict, CFSTR(kDisplayProductID) );
        if( num)
            CFNumberGetValue( num, kCFNumberSInt32Type, &product );

        num = CFDictionaryGetValue( regDict, CFSTR(kAppleDisplayTypeKey) );
        if( num) {
            CFNumberGetValue( num, kCFNumberSInt32Type, &displayType );
            if( (vendor == kDisplayVendorIDUnknown) && (displayType == 10))
                product = kDisplayProductIDGeneric;
        }

        data = CFDictionaryGetValue( regDict, CFSTR(kIODisplayEDIDKey) );

#if SPOOF_EDID
#warning             ****************
#warning             ** SPOOF_EDID **
#warning             ****************
		if (data)
		{
            EDIDInfo( (EDID *) CFDataGetBytePtr(data), &vendor, &product, NULL, NULL, NULL, NULL);
			if (0x10ac == vendor)
			{
				data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault,
												   spoofEDID, sizeof(spoofEDID), kCFAllocatorNull);
			}
			vendor = product = 0;
        }
#endif
        if( !data)
            continue;
        edid = (EDID *) CFDataGetBytePtr( data );
        if( vendor && product)
            EDIDInfo( edid, 0, 0, &serialNumber, &manufactureYear, &manufactureWeek, &isDigital );
        else
            EDIDInfo( edid, &vendor, &product, &serialNumber, &manufactureYear, &manufactureWeek, &isDigital );

    } while( false );

    // <hack>
    if( !vendor && !product) {
        vendor = kDisplayVendorIDUnknown;
        product = kDisplayProductIDGeneric;
    } // </hack>

    dict = IODisplayCreateOverrides( framebuffer, options, vendor, product,
                                        serialNumber, manufactureYear, manufactureWeek, 
                                        isDigital );

#define makeInt( key, value )   \
        num = CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt32Type, &value );       \
        CFDictionaryAddValue( dict, key,  num );                                        \
        CFRelease( num );

#define addFloat( key ) \
        num = CFNumberCreate( kCFAllocatorDefault, kCFNumberFloatType, &fnum  );        \
        CFDictionaryAddValue( dict, key, num );                                         \
        CFRelease( num );

    do {
        if( !dict)
            continue;

        makeInt( CFSTR( kDisplayVendorID ), vendor );
        makeInt( CFSTR( kDisplayProductID ), product );

        if( serialNumber) {
            makeInt( CFSTR( kDisplaySerialNumber ), serialNumber );
        }

        kr = IORegistryEntryGetPath( service, kIOServicePlane, path );
        if( KERN_SUCCESS == kr) {
            CFStringRef string;
            string = CFStringCreateWithCString( kCFAllocatorDefault, path,
                                        kCFStringEncodingMacRoman );
            if( string) {
                CFDictionaryAddValue( dict, CFSTR(kIODisplayLocationKey), string);
                CFRelease(string);
            }
        }

        if ((kIODisplayNoProductName | kIODisplayMatchingInfo) & options)
            // the raw override form is not what clients expect
            CFDictionaryRemoveValue( dict, CFSTR(kDisplayProductName) );

        // -- that's all for matching --
        if( options & kIODisplayMatchingInfo)
            continue;

        if (data)
        {
			// if !exist add display edid
            CFDictionaryAddValue(dict, CFSTR(kIODisplayEDIDKey), data);
            CFDictionaryAddValue(dict, CFSTR(kIODisplayEDIDOriginalKey), data);
        }
        // get final edid
        data = CFDictionaryGetValue(dict, CFSTR(kIODisplayEDIDKey));
        if (data)
            edid = (EDID *) CFDataGetBytePtr(data);
            // no point in serial# / manufacture date from override
        else
            edid = 0;

		if (regDict)
		{
			obj = CFDictionaryGetValue( regDict, CFSTR(kIODisplayConnectFlagsKey) );
			if( obj)
				CFDictionarySetValue( dict, CFSTR(kIODisplayConnectFlagsKey), obj );
	
			obj = CFDictionaryGetValue( regDict, CFSTR(kIODisplayPrefKeyKey) );
			if( obj)
				CFDictionarySetValue( dict, CFSTR(kIODisplayPrefKeyKey), obj );

			CFDictionaryRef attrDict;
			attrDict = CFDictionaryGetValue(regDict, CFSTR(kIODisplayAttributesKey));
            if (attrDict && (CFDictionaryGetTypeID() == CFGetTypeID(attrDict)))
				CFDictionaryApplyFunction(attrDict, &IODisplayDictAddValues, dict);
		}

        if( IOObjectConformsTo( service, "IOBacklightDisplay"))
            CFDictionarySetValue( dict, CFSTR(kIODisplayHasBacklightKey), kCFBooleanTrue );

        kr = IORegistryEntryCreateCFProperties(framebuffer, &fbRegDict,
                                                kCFAllocatorDefault, kNilOptions );
        if (kIOReturnSuccess == kr)
        {
            if( (obj = CFDictionaryGetValue(fbRegDict, CFSTR(kIOFBTransformKey))))
            {
                CFNumberGetValue(obj, kCFNumberSInt32Type, &sint);
                sint = (sint & kIOFBRotateFlags) | ((sint >> 4) & kIOFBRotateFlags);
                makeInt( CFSTR(kIOFBTransformKey), sint );
            }
            if( (obj = CFDictionaryGetValue(fbRegDict, CFSTR("graphic-options"))))
                CFDictionaryAddValue(dict, CFSTR("graphic-options"), obj);
            CFRelease(fbRegDict);
        }

        data = CFDictionaryGetValue( dict, CFSTR("dmdg") );
        if( data)
            sint = OSReadBigInt32((void *) CFDataGetBytePtr(data), 0);
        else
            sint = kDisplayGestaltBrightnessAffectsGammaMask;

        if( kDisplayGestaltBrightnessAffectsGammaMask & sint)
            CFDictionaryAddValue( dict, CFSTR(kDisplayBrightnessAffectsGamma), kCFBooleanTrue );
        if( kDisplayGestaltViewAngleAffectsGammaMask & sint)
            CFDictionaryAddValue( dict, CFSTR(kDisplayViewAngleAffectsGamma), kCFBooleanTrue );

        if (!(kIODisplayNoProductName & options))
            GenerateProductName( dict, edid, displayType, options );

        if( !edid)
            continue;

        if( 0x80 & edid->displayParams[0]) {
            CFDictionarySetValue( dict, CFSTR(kIODisplayIsDigitalKey), kCFBooleanTrue );

            if( kDisplayAppleVendorID == vendor) {
                CFDictionaryAddValue( dict, CFSTR(kDisplayFixedPixelFormat), kCFBooleanTrue );
                sint = kDisplaySubPixelLayoutRGB;
                makeInt( CFSTR( kDisplaySubPixelLayout ), sint );

                CFDictionaryRemoveValue( dict, CFSTR(kDisplayBrightnessAffectsGamma) );
                CFDictionarySetValue( dict, CFSTR(kDisplayViewAngleAffectsGamma), kCFBooleanTrue );
            }
        }

        // EDID timing range
        for( i = 0; i < 4; i++ ) {
            if( EDIDDescToDisplayTimingRangeRec( edid, 
                                &edid->descriptors[i].general,
                                &displayRange )) {
                                
                if( !CFDictionaryGetValue( dict, CFSTR("drng"))) {
                    data = CFDataCreate( kCFAllocatorDefault,
                                (UInt8 *) &edid->descriptors[i].general, sizeof(EDIDGeneralDesc));
                    if( data) {
                        CFDictionarySetValue(dict, CFSTR("drng"), data);
                        CFRelease(data);
                    }
                }

                if( !CFDictionaryGetValue( dict, CFSTR("trng"))) {
                    data = CFDataCreate( kCFAllocatorDefault,
                                (UInt8 *) &displayRange, sizeof(displayRange));
                    if( data) {
                        CFDictionarySetValue(dict, CFSTR("trng"), data);
                        CFRelease(data);
                    }
                }
                break;
            }
        }

        sint = edid->weekOfManufacture;
        makeInt( CFSTR( kDisplayWeekOfManufacture ), sint );
        sint = edid->yearOfManufacture + 1990;
        makeInt( CFSTR( kDisplayYearOfManufacture ), sint );

        sint = edid->displayParams[1] * 10;
        makeInt( CFSTR( kDisplayHorizontalImageSize ), sint );
        sint = edid->displayParams[2] * 10;
        makeInt( CFSTR( kDisplayVerticalImageSize ), sint );

        // color info
        low = edid->colorCharacteristics[0];

        fnum = (edid->colorCharacteristics[2] << 2) | ((low >> 6) & 3);
        fnum /= (1 << 10);
        addFloat( CFSTR( kDisplayRedPointX ) );
        fnum = (edid->colorCharacteristics[3] << 2) | ((low >> 4) & 3);
        fnum /= (1 << 10);
        addFloat( CFSTR( kDisplayRedPointY ) );

        fnum = (edid->colorCharacteristics[4] << 2) | ((low >> 2) & 3);
        fnum /= (1 << 10);
        addFloat( CFSTR( kDisplayGreenPointX ) );
        fnum = (edid->colorCharacteristics[5] << 2) | ((low >> 0) & 3);
        fnum /= (1 << 10);
        addFloat( CFSTR( kDisplayGreenPointY ) );

        low = edid->colorCharacteristics[1];

        fnum = (edid->colorCharacteristics[6] << 2) | ((low >> 6) & 3);
        fnum /= (1 << 10);
        addFloat( CFSTR( kDisplayBluePointX ) );
        fnum = (edid->colorCharacteristics[7] << 2) | ((low >> 4) & 3);
        fnum /= (1 << 10);
        addFloat( CFSTR( kDisplayBluePointY ) );

        fnum = (edid->colorCharacteristics[8] << 2) | ((low >> 2) & 3);
        fnum /= (1 << 10);
        addFloat( CFSTR( kDisplayWhitePointX ) );
        fnum = (edid->colorCharacteristics[9] << 2) | ((low >> 0) & 3);
        fnum /= (1 << 10);
        addFloat( CFSTR( kDisplayWhitePointY ) );

        fnum = edid->displayParams[3];
        fnum = (fnum + 100.0) / 100.0;
        addFloat( CFSTR( kDisplayWhiteGamma ) );

        if (IODisplayIsHDMISink(dict)) {
            CFDictionarySetValue(dict, CFSTR(kIODisplayIsHDMISinkKey), kCFBooleanTrue);
        } else {
            CFDictionarySetValue(dict, CFSTR(kIODisplayIsHDMISinkKey), kCFBooleanFalse);
        }

    } while( false );

    if (service)
        IOObjectRelease(service);

    if( regDict)
        CFRelease( regDict );
        
    return( dict );
}

IOReturn
IODisplayCopyParameters(
        io_service_t      service,
        IOOptionBits      options,
        CFDictionaryRef * params )
{
    if( (service = IODisplayForFramebuffer( service, options)))
    {
        *params = IORegistryEntryCreateCFProperty( service, CFSTR(kIODisplayParametersKey),
                                                    kCFAllocatorDefault, kNilOptions );
        IOObjectRelease(service);
    }
    else
        *params = 0;

    return( *params ? kIOReturnSuccess : kIOReturnUnsupported );
}

IOReturn
IODisplayCopyFloatParameters(
        io_service_t  service __unused,
        IOOptionBits  options __unused,
        CFDictionaryRef * params __unused )
{
    return( kIOReturnUnsupported );
}

IOReturn
IODisplayGetIntegerRangeParameter(
        io_service_t    service,
        IOOptionBits    options,
        CFStringRef     parameterName,
        SInt32 *        value,
        SInt32 *        min,
        SInt32 *        max )
{
    IOReturn            err;
    CFDictionaryRef     params;
    CFDictionaryRef     param;
    CFNumberRef         num;

#if DEBUGPARAMS
    const char *        cStr = 0;
    
    if( (cStr = CFStringGetCStringPtr( parameterName, kCFStringEncodingMacRoman))
    && (cStr = getenv(cStr)))
        parameterName =  CFStringCreateWithCString( kCFAllocatorDefault, cStr,
                                                    kCFStringEncodingMacRoman );
#endif

    do {
        err = IODisplayCopyParameters( service, options, &params );
        if( err != kIOReturnSuccess)
            continue;

        param = CFDictionaryGetValue( params, parameterName );

        if( !param) {
            err = kIOReturnUnsupported;
            continue;
        }
        if( value && (num = CFDictionaryGetValue( param, CFSTR(kIODisplayValueKey))))
            CFNumberGetValue( num, kCFNumberSInt32Type, value );
        if( min && (num = CFDictionaryGetValue( param, CFSTR(kIODisplayMinValueKey))))
            CFNumberGetValue( num, kCFNumberSInt32Type, min );
        if( max && (num = CFDictionaryGetValue( param, CFSTR(kIODisplayMaxValueKey))))
            CFNumberGetValue( num, kCFNumberSInt32Type, max );

    } while( false );

    if( params)
        CFRelease(params);

#if DEBUGPARAMS
    if( cStr)
        CFRelease(parameterName);
#endif

    return( err );
}

IOReturn
IODisplayGetFloatParameter(
        io_service_t    service,
        IOOptionBits    options,
        CFStringRef     parameterName,
        float *         value )
{
    IOReturn    err;
    SInt32      ivalue, min, max;

    err = IODisplayGetIntegerRangeParameter( service, options, parameterName,
                                             &ivalue, &min, &max );
    if( err)
        return( err);

    if( min == max)
        *value = 0;
    else
        *value = (((float) ivalue) - ((float) min)) / (((float) max) - ((float) min));

    return( err );
}


IOReturn
IODisplaySetParameters(
        io_service_t    service,
        IOOptionBits    options,
        CFDictionaryRef params )
{
    IOReturn err;

    if( !(service = IODisplayForFramebuffer( service, options)))
        return( kIOReturnUnsupported );

    err = IORegistryEntrySetCFProperties( service, params );

    IOObjectRelease(service);

    return( err );
}

IOReturn
IODisplaySetIntegerParameter(
        io_service_t    service,
        IOOptionBits options __unused,
        CFStringRef     parameterName,
        SInt32          value )
{
    IOReturn            err;
    CFDictionaryRef     dict;
    CFNumberRef         num;

#if DEBUGPARAMS
    const char *        cStr;

    if( (cStr = CFStringGetCStringPtr( parameterName, kCFStringEncodingMacRoman))
     && (cStr = getenv(cStr)))
        parameterName =  CFStringCreateWithCString( kCFAllocatorDefault, cStr,
                                                    kCFStringEncodingMacRoman );
#endif

    num = CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt32Type, &value );
    if( !num)
        return( kIOReturnNoMemory );

    dict = CFDictionaryCreate( kCFAllocatorDefault,
                                (const void **) &parameterName, (const void **) &num, 1,
                                &kCFTypeDictionaryKeyCallBacks,
                                &kCFTypeDictionaryValueCallBacks );
    CFRelease(num);
    if( !dict)
        return( kIOReturnNoMemory );

    err = IODisplaySetParameters( service, kNilOptions, dict );

    CFRelease(dict);

#if DEBUGPARAMS
    if( cStr)
        CFRelease(parameterName);
#endif

    return( err );
}

IOReturn
IODisplaySetFloatParameter(
        io_service_t    service,
        IOOptionBits    options,
        CFStringRef     parameterName,
        float           value )
{
    IOReturn    err;
    SInt32      ivalue, min, max;

    err = IODisplayGetIntegerRangeParameter( service, options, parameterName,
                                             NULL, &min, &max );
    if( err)
        return( err);

    ivalue = roundf((value * (((float) max) - ((float) min)) + ((float) min)));

    err = IODisplaySetIntegerParameter( service, options, parameterName, ivalue );

    return( err );
}

IOReturn
IODisplayCommitParameters(
        io_service_t    service,
        IOOptionBits    options )
{
    return( IODisplaySetIntegerParameter( service, options,
                CFSTR(kIODisplayParametersCommitKey), 1));
}

#undef IOCreateDisplayInfoDictionary
CFDictionaryRef
IOCreateDisplayInfoDictionary(
        io_service_t            framebuffer,
        IOOptionBits            options )
{
    return( _IODisplayCreateInfoDictionary(framebuffer, options));
}

CFDictionaryRef
IODisplayCreateInfoDictionary(
        io_service_t            framebuffer,
        IOOptionBits            options )
{
    return( _IODisplayCreateInfoDictionary(framebuffer, options));
}

SInt32
IODisplayMatchDictionaries(
        CFDictionaryRef         matching1,
        CFDictionaryRef         matching2,
        IOOptionBits    options __unused )
{
    CFNumberRef         num1, num2;
    CFStringRef         str1, str2;
    SInt32              matches = 0;

    if( !matching1 || !matching2)
        return( -1 );

    do {
        num1 = CFDictionaryGetValue( matching1, CFSTR(kDisplayVendorID) );
        num2 = CFDictionaryGetValue( matching2, CFSTR(kDisplayVendorID) );
        if( !num1 || !num2)
            continue;
        if( !CFEqual( num1, num2))
            continue;

        num1 = CFDictionaryGetValue( matching1, CFSTR(kDisplayProductID) );
        num2 = CFDictionaryGetValue( matching2, CFSTR(kDisplayProductID) );
        if( !num1 || !num2)
            continue;
        if( !CFEqual( num1, num2))
            continue;

        num1 = CFDictionaryGetValue( matching1, CFSTR(kDisplaySerialNumber) );
        num2 = CFDictionaryGetValue( matching2, CFSTR(kDisplaySerialNumber) );
        if( num1 && num2 && (!CFEqual( num1, num2)))
            continue;

        str1 = CFDictionaryGetValue( matching1, CFSTR(kIODisplayLocationKey) );
        str2 = CFDictionaryGetValue( matching2, CFSTR(kIODisplayLocationKey) );
        if( str1 && str2 && (!CFEqual( str1, str2)))
            continue;

        matches = 1000;

    } while( false );

    return( matches );
}
