/*      CFBundle_SplitFileName.c
        Copyright (c) 2019, Apple Inc. All rights reserved.
*/

#include "CFBundle_SplitFileName.h"

#include <CoreFoundation/CFPriv.h>

#define _CFBundleiPadDeviceNameSuffix CFSTR("~ipad")

static Boolean _CFBundleFileVersionFoundMatchingPlatform(_CFBundleFileVersion version) {
    return version == _CFBundleFileVersionNoProductWithPlatform || version == _CFBundleFileVersionWithProductWithPlatform;
}

static Boolean _CFBundleFileVersionFoundMatchingProduct(_CFBundleFileVersion version) {
    return version == _CFBundleFileVersionWithProductNoPlatform || version == _CFBundleFileVersionWithProductWithPlatform;
}

static _CFBundleFileVersion _CFBundleVersionForFileName(CFStringRef fileName, Boolean searchProduct, CFStringRef expectedProduct, CFStringRef expectedPlatform, CFRange *outProductRange, CFRange *outPlatformRange) {
    // Search for a product name, e.g.: foo~iphone.jpg or bar~ipad
    Boolean foundProduct = false;
    Boolean foundPlatform = false;
    CFIndex fileNameLen = CFStringGetLength(fileName);
    CFRange productRange;
    CFRange platformRange;

    CFIndex dotLocation = fileNameLen;
    for (CFIndex i = fileNameLen - 1; i > 0; i--) {
        UniChar c = CFStringGetCharacterAtIndex(fileName, i);
        if (c == '.') {
            dotLocation = i;
        }

        if (searchProduct && c == '~' && !foundProduct) {
            productRange = CFRangeMake(i, dotLocation - i);
            foundProduct = (CFStringCompareWithOptions(fileName, expectedProduct, productRange, kCFCompareAnchored) == kCFCompareEqualTo);
            if (foundProduct && outProductRange) *outProductRange = productRange;
        } else if (c == '-') {
            if (foundProduct) {
                platformRange = CFRangeMake(i, productRange.location - i);
            } else {
                platformRange = CFRangeMake(i, dotLocation - i);
            }
            foundPlatform = (CFStringCompareWithOptions(fileName, expectedPlatform, platformRange, kCFCompareAnchored) == kCFCompareEqualTo);
            if (foundPlatform && outPlatformRange) *outPlatformRange = platformRange;
            break;
        }
    }

    _CFBundleFileVersion version;
    if (foundPlatform && foundProduct) {
        version = _CFBundleFileVersionWithProductWithPlatform;
    } else if (foundPlatform) {
        version = _CFBundleFileVersionNoProductWithPlatform;
    } else if (foundProduct) {
        version = _CFBundleFileVersionWithProductNoPlatform;
    } else {
        version = _CFBundleFileVersionNoProductNoPlatform;
    }
    return version;
}

// Splits up a string into its various parts. Note that the out-types must be released by the caller if they exist.
CF_PRIVATE void _CFBundleSplitFileName(CFStringRef fileName, CFStringRef *noProductOrPlatform, CFStringRef *endType, CFStringRef *startType, CFStringRef expectedProduct, CFStringRef expectedPlatform, Boolean searchForFallbackProduct, _CFBundleFileVersion *version) {
    CFIndex fileNameLen = CFStringGetLength(fileName);

    if (endType || startType) {
        // Search for the type from the end (type defined as everything after the last '.')
        // e.g., a file name like foo.jpg has a type of 'jpg'
        Boolean foundDot = false;
        uint16_t dotLocation = 0;
        for (CFIndex i = fileNameLen; i > 0; i--) {
            if (CFStringGetCharacterAtIndex(fileName, i - 1) == '.') {
                foundDot = true;
                dotLocation = i - 1;
                break;
            }
        }

        if (foundDot && dotLocation != fileNameLen - 1) {
            if (endType) *endType = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, fileName, CFRangeMake(dotLocation + 1, CFStringGetLength(fileName) - dotLocation - 1));
        }

        // Search for the type from the beginning (type defined as everything after the first '.')
        // e.g., a file name like foo.jpg.gz has a type of 'jpg.gz'
        if (startType) {
            for (CFIndex i = 0; i < fileNameLen; i++) {
                if (CFStringGetCharacterAtIndex(fileName, i) == '.') {
                    // no need to create this again if it's the same as previous
                    if (i != dotLocation) {
                        *startType = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, fileName, CFRangeMake(i + 1, CFStringGetLength(fileName) - i - 1));
                    }
                    break;
                }
            }
        }
    }

    CFRange productRange, platformRange;

    // Product names are only supported on iOS.
    // Ref docs here: "iOS Supports Device-Specific Resources" in "Resource Programming Guide"
#if TARGET_OS_IPHONE
    Boolean searchForProductName = true;
#else
    Boolean searchForProductName = false;
#endif
    *version = _CFBundleVersionForFileName(fileName, searchForProductName, expectedProduct, expectedPlatform, &productRange, &platformRange);

    Boolean foundPlatform = _CFBundleFileVersionFoundMatchingPlatform(*version);
    Boolean foundProduct = _CFBundleFileVersionFoundMatchingProduct(*version);


    // Create a string that excludes both platform and product name
    // e.g., foo-iphone~iphoneos.jpg -> foo.jpg
    if (foundPlatform || foundProduct) {
        CFMutableStringRef fileNameScratch = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, 0, fileName);
        CFIndex start, length = 0;

        // Because the platform always comes first and is immediately followed by product if it exists, we'll use the platform start location as the start of our range to delete.
        if (foundPlatform) {
            start = platformRange.location;
        } else {
            start = productRange.location;
        }

        if (foundPlatform && foundProduct) {
            length = platformRange.length + productRange.length;
        } else if (foundPlatform) {
            length = platformRange.length;
        } else if (foundProduct) {
            length = productRange.length;
        }
        CFStringDelete(fileNameScratch, CFRangeMake(start, length));
        *noProductOrPlatform = (CFStringRef)fileNameScratch;
    }
}
