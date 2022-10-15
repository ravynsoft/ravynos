/*      CFBundle_SplitFileName.h
        Copyright (c) 2019, Apple Inc. All rights reserved.
*/

#ifndef CFBundle_SplitFileName_h
#define CFBundle_SplitFileName_h

#include <CoreFoundation/CFString.h>

typedef enum {
    _CFBundleFileVersionNoProductNoPlatform = 1,
    _CFBundleFileVersionWithProductNoPlatform,
    _CFBundleFileVersionNoProductWithPlatform,
    _CFBundleFileVersionWithProductWithPlatform,
    _CFBundleFileVersionUnmatched
} _CFBundleFileVersion;

CF_PRIVATE void _CFBundleSplitFileName(CFStringRef fileName, CFStringRef *noProductOrPlatform, CFStringRef *endType, CFStringRef *startType, CFStringRef expectedProduct, CFStringRef expectedPlatform, Boolean searchForFallbackProduct, _CFBundleFileVersion *version);

#endif /* CFBundle_SplitFileName_h */
