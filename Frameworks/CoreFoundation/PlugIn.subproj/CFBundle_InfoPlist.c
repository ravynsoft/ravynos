/*      CFBundle_InfoPlist.c
	Copyright (c) 2012-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
        Responsibility: Tony Parker
 */

#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFError_Private.h>
#include "CFBundle_Internal.h"
#include <CoreFoundation/CFByteOrder.h>
#include <CoreFoundation/CFURLAccess.h>

#if (TARGET_OS_MAC || TARGET_OS_LINUX || TARGET_OS_BSD) && !TARGET_OS_CYGWIN
#include <dirent.h>
#if TARGET_OS_MAC || TARGET_OS_BSD
#include <sys/sysctl.h>
#endif
#include <sys/mman.h>
#endif


#pragma mark -
#pragma mark Product and Platform Getters - Exported

CF_EXPORT void _CFSetProductName(CFStringRef str) {
    // Obsolete, does nothing
}

CF_EXPORT CFStringRef _CFGetProductName(void) {
    static CFStringRef _cfBundlePlatform = NULL;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
#if TARGET_OS_MAC
        // We only honor the classic suffix if it is one of two preset values. Otherwise we fall back to the result of sysctlbyname.
        const char *classicSuffix = __CFgetenv("CLASSIC_SUFFIX");
        if (classicSuffix && strncmp(classicSuffix, "iphone", strlen("iphone")) == 0) {
            os_log_debug(_CFBundleResourceLogger(), "Using ~iphone resources (classic)");
            _cfBundlePlatform = _CFBundleiPhoneDeviceName;
        } else if (classicSuffix && strncmp(classicSuffix, "ipad", strlen("ipad")) == 0) {
            os_log_debug(_CFBundleResourceLogger(), "Using ~ipad resources (classic)");
            _cfBundlePlatform = _CFBundleiPadDeviceName;
        } else {
#if TARGET_OS_OSX
            // Do not check the sysctl on macOS
            _cfBundlePlatform = CFSTR("");
#else
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            size_t buflen = sizeof(buffer);
            int ret = sysctlbyname("hw.machine", buffer, &buflen, NULL, 0);
            if (0 == ret || (-1 == ret && ENOMEM == errno)) {
#if TARGET_OS_IOS
                if (6 <= buflen && 0 == memcmp(buffer, "iPhone", 6)) {
                    _cfBundlePlatform = _CFBundleiPhoneDeviceName;
                } else
                    if (4 <= buflen && 0 == memcmp(buffer, "iPod", 4)) {
                        _cfBundlePlatform = _CFBundleiPodDeviceName;
                    } else
                        if (4 <= buflen && 0 == memcmp(buffer, "iPad", 4)) {
                            _cfBundlePlatform = _CFBundleiPadDeviceName;
                        }
#elif TARGET_OS_WATCH
                if (5 <= buflen && 0 == memcmp(buffer, "Watch", 5)) {
                    _cfBundlePlatform = _CFBundleAppleWatchDeviceName;
                }
#elif TARGET_OS_TV
                if (7 <= buflen && 0 == memcmp(buffer, "AppleTV", 7)) {
                    _cfBundlePlatform = _CFBundleAppleTVDeviceName;
                }
#else
                // Fallback path for other TARGET_OS_IPHONE child macros we don't know or care about
                if (false) { }
#endif
                else {
                    const char *env = __CFgetenv("SIMULATOR_LEGACY_ASSET_SUFFIX");
                    if (env) {
                        if (0 == strcmp(env, "iphone")) {
                            _cfBundlePlatform = _CFBundleiPhoneDeviceName;
                        } else if (0 == strcmp(env, "ipad")) {
                            _cfBundlePlatform = _CFBundleiPadDeviceName;
                        } else {
                            // fallback, unrecognized SIMULATOR_LEGACY_ASSET_SUFFIX
                        }
                    } else {
                        // fallback, unrecognized hw.machine and no SIMULATOR_LEGACY_ASSET_SUFFIX
                    }
                }
            }
#endif // TARGET_OS_OSX
            
            os_log_debug(_CFBundleResourceLogger(), "Using ~%@ resources", _cfBundlePlatform);
        }
#endif // TARGET_OS_MAC

        // This used to fall back to "iphone" on all unknown TARGET_OS_IPHONE platforms, but since that macro covers a wide swath of platforms, it now falls back to an empty string.
        if (!_cfBundlePlatform) {
            os_log_debug(_CFBundleResourceLogger(), "Using ~ resources");
            _cfBundlePlatform = CFSTR(""); // fallback
        }
    });
    
    return _cfBundlePlatform;
}

CF_PRIVATE CFStringRef _CFBundleGetProductNameSuffix(void) {
    static CFStringRef _cfBundlePlatformSuffix = NULL;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        CFStringRef productName = _CFGetProductName();
        if (CFEqual(productName, _CFBundleiPodDeviceName)) {
            productName = _CFBundleiPhoneDeviceName;
        }
        _cfBundlePlatformSuffix = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("~%@"), productName);
    });
    return _cfBundlePlatformSuffix;
}

CF_PRIVATE CFStringRef _CFBundleGetPlatformNameSuffix(void) {
#if TARGET_OS_OSX
    return _CFBundleMacOSXPlatformNameSuffix;
#elif TARGET_OS_IOS
    return _CFBundleiPhoneOSPlatformNameSuffix;
#elif TARGET_OS_WATCH
    return _CFBundleWatchOSPlatformNameSuffix;
#elif TARGET_OS_TV
    return _CFBundletvOSPlatformNameSuffix;
#elif TARGET_OS_IPHONE
    // Fallback path for other TARGET_OS_IPHONE targets we do not know about
    return CFSTR("");
#elif TARGET_OS_WIN32
    return _CFBundleWindowsPlatformNameSuffix;
#elif DEPLOYMENT_TARGET_SOLARIS
    return _CFBundleSolarisPlatformNameSuffix;
#elif DEPLOYMENT_TARGET_HPUX
    return _CFBundleHPUXPlatformNameSuffix;
#elif TARGET_OS_LINUX
    return _CFBundleLinuxPlatformNameSuffix;
#elif TARGET_OS_BSD
    return _CFBundleFreeBSDPlatformNameSuffix;
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
}

// All new-style bundles will have these extensions.
CF_EXPORT CFStringRef _CFGetPlatformName(void) {
#if TARGET_OS_OSX
    return _CFBundleMacOSXPlatformName;
#elif TARGET_OS_IOS
    return _CFBundleiPhoneOSPlatformName;
#elif TARGET_OS_WATCH
    return _CFBundleWatchOSPlatformName;
#elif TARGET_OS_TV
    return _CFBundletvOSPlatformName;
#elif TARGET_OS_IPHONE
    // Fallback path for other TARGET_OS_IPHONE targets we do not know about
    return CFSTR("");
#elif TARGET_OS_WIN32
    return _CFBundleWindowsPlatformName;
#elif DEPLOYMENT_TARGET_SOLARIS
    return _CFBundleSolarisPlatformName;
#elif DEPLOYMENT_TARGET_HPUX
    return _CFBundleHPUXPlatformName;
#elif TARGET_OS_LINUX
#if TARGET_OS_CYGWIN
    return _CFBundleCygwinPlatformName;
#else
    return _CFBundleLinuxPlatformName;
#endif
#elif TARGET_OS_BSD
    return _CFBundleFreeBSDPlatformName;
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
}

CF_EXPORT CFStringRef _CFGetAlternatePlatformName(void) {
#if TARGET_OS_OSX
    return _CFBundleAlternateMacOSXPlatformName;
#elif TARGET_OS_IPHONE
    return _CFBundleMacOSXPlatformName;
#elif TARGET_OS_WIN32
    return CFSTR("");
#elif TARGET_OS_LINUX
#if TARGET_OS_CYGWIN
    return CFSTR("Cygwin");
#else
    return CFSTR("Linux");
#endif
#elif TARGET_OS_BSD
    return CFSTR("FreeBSD");
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
}

#pragma mark -
#pragma mark Product and Platform Suffix Processing - Internal

// Returns true if the searchRange of the fileName is equal to a valid platform name (e.g., macos, iphoneos).
CF_PRIVATE Boolean _CFBundleSupportedPlatformName(CFStringRef fileName, CFRange searchRange) {
#if TARGET_OS_IOS
    return CFStringFindWithOptions(fileName, _CFBundleiPhoneOSPlatformName, searchRange, kCFCompareAnchored, NULL);
#elif TARGET_OS_WATCH
    return CFStringFindWithOptions(fileName, _CFBundleWatchOSPlatformName   , searchRange, kCFCompareAnchored, NULL);
#elif TARGET_OS_TV
    return CFStringFindWithOptions(fileName, _CFBundletvOSPlatformName, searchRange, kCFCompareAnchored, NULL);
#elif TARGET_OS_OSX
    return CFStringFindWithOptions(fileName, _CFBundleMacOSXPlatformName, searchRange, kCFCompareAnchored, NULL);
#else
    // This OS supports no platform suffixes
    return false;
#endif
}

// Returns true if the searchRange of the fileName is equal to a a valid product name (e.g., ipod, ipad)
CF_PRIVATE Boolean _CFBundleSupportedProductName(CFStringRef fileName, CFRange searchRange) {
#if TARGET_OS_IOS
#define _CFBundleNumberOfPlatforms 3
    static const CFIndex numberOfPlatforms = 3;
    static const CFStringRef platforms[numberOfPlatforms] = { CFSTR("iphone"), CFSTR("ipad"), CFSTR("ipod") };
    for (CFIndex i = 0; i < numberOfPlatforms; i++) {
        if (CFStringFindWithOptions(fileName, platforms[i], searchRange, kCFCompareAnchored, NULL)) {
            return true;
        }
    }
    return false;
#elif TARGET_OS_WATCH
    return CFStringFindWithOptions(fileName, CFSTR("applewatch"), searchRange, kCFCompareAnchored, NULL);
#elif TARGET_OS_TV
    return CFStringFindWithOptions(fileName, CFSTR("appletv"), searchRange, kCFCompareAnchored, NULL);
#elif TARGET_OS_OSX
    // MacOS uses an empty string for a product name. We do not distinguish at this time between kinds of Mac products
    return false;
#else
    // This OS supports no product suffixes
    return false;
#endif
}

static Boolean _isBlacklistedKey(CFStringRef keyName) {
#if __CONSTANT_STRINGS__
#define _CFBundleNumberOfBlacklistedInfoDictionaryKeys 2
    static const CFStringRef _CFBundleBlacklistedInfoDictionaryKeys[_CFBundleNumberOfBlacklistedInfoDictionaryKeys] = { CFSTR("CFBundleExecutable"), CFSTR("CFBundleIdentifier") };
    
    for (CFIndex idx = 0; idx < _CFBundleNumberOfBlacklistedInfoDictionaryKeys; idx++) {
        if (CFEqual(keyName, _CFBundleBlacklistedInfoDictionaryKeys[idx])) return true;
    }
#endif
    return false;
}

static Boolean _isPlatformAndProductKey(CFStringRef fullKey, Boolean const useFallbackKey, CFStringRef *outBaseKey, CFStringRef *outPlatformSuffix, CFStringRef *outProductSuffix) {
    if (outBaseKey) {
        *outBaseKey = NULL;
    }
    if (outPlatformSuffix) {
        *outPlatformSuffix = NULL;
    }
    if (outProductSuffix) {
        *outProductSuffix = NULL;
    }
    if (!fullKey) return false;
    CFRange minusRange = CFStringFind(fullKey, CFSTR("-"), kCFCompareBackwards);
    CFRange tildeRange = CFStringFind(fullKey, CFSTR("~"), kCFCompareBackwards);
    if (minusRange.location == kCFNotFound && tildeRange.location == kCFNotFound) return false;
    // minus must come before tilde if both are present
    if (minusRange.location != kCFNotFound && tildeRange.location != kCFNotFound && tildeRange.location <= minusRange.location) return false;
    
    CFIndex strLen = CFStringGetLength(fullKey);
    CFRange baseKeyRange = (minusRange.location != kCFNotFound) ? CFRangeMake(0, minusRange.location) : CFRangeMake(0, tildeRange.location);
    CFRange platformRange = CFRangeMake(kCFNotFound, 0);
    CFRange productRange = CFRangeMake(kCFNotFound, 0);
    if (minusRange.location != kCFNotFound) {
        platformRange.location = minusRange.location + minusRange.length;
        platformRange.length = ((tildeRange.location != kCFNotFound) ? tildeRange.location : strLen) - platformRange.location;
    }
    if (tildeRange.location != kCFNotFound) {
        productRange.location = tildeRange.location + tildeRange.length;
        productRange.length = strLen - productRange.location;
    }
    if (baseKeyRange.length < 1) return false;
    if (platformRange.location != kCFNotFound && platformRange.length < 1) return false;
    if (productRange.location != kCFNotFound && productRange.length < 1) return false;
    
    Boolean isValidPlatformAndProduct = true;
    if (platformRange.location == kCFNotFound && productRange.location != kCFNotFound) {
        // With no platform, only check the product
        isValidPlatformAndProduct = _CFBundleSupportedProductName(fullKey, productRange);
    } else if (platformRange.location != kCFNotFound && productRange.location == kCFNotFound) {
        // With no product, check only the platform
        isValidPlatformAndProduct = _CFBundleSupportedPlatformName(fullKey, platformRange);
    } else {
        // Check both
        isValidPlatformAndProduct = _CFBundleSupportedProductName(fullKey, productRange) && _CFBundleSupportedPlatformName(fullKey, platformRange);
    }
    

    if (isValidPlatformAndProduct) {
        if (outBaseKey) {
            *outBaseKey = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, fullKey, baseKeyRange);
        }
        if (outPlatformSuffix) {
            CFStringRef platform = (platformRange.location != kCFNotFound) ? CFStringCreateWithSubstring(kCFAllocatorSystemDefault, fullKey, platformRange) : NULL;
            *outPlatformSuffix = platform;
        }
        if (outProductSuffix) {
            CFStringRef product = (productRange.location != kCFNotFound) ? CFStringCreateWithSubstring(kCFAllocatorSystemDefault, fullKey, productRange) : NULL;
            *outProductSuffix = product;
        }
    }
    return isValidPlatformAndProduct;
}


static Boolean _isValidSpecialCase(CFStringRef specialCase) {
    // NOTE: Adding any special case to this check must be paired with adding the suffix in __addSuffixesToKeys
    return false;
}

// Special case keys replace base keys in Info.plist and InfoPlist.strings files. They take the form of KeyName#SpecialCase. The special cases are checked in _isValidSpecialCase. If this function returns true then the special case key exists and the replacement behavior should be triggered, according to whatever the criteria are.
static Boolean _isSpecialCaseKey(CFStringRef fullKey, CFStringRef *outBaseKey, CFStringRef *outSpecialCase) {
    if (outBaseKey) {
        *outBaseKey = NULL;
    }
    if (outSpecialCase) {
        *outSpecialCase = NULL;
    }
    if (!fullKey) return false;
    
    CFRange hashRange = CFStringFind(fullKey, CFSTR("#"), kCFCompareBackwards);
    if (hashRange.location == kCFNotFound) return false;
    CFRange baseKeyRange = CFRangeMake(0, hashRange.location);
    if (baseKeyRange.length < 1) return false;
    CFIndex strLen = CFStringGetLength(fullKey);
    CFIndex specialCaseStart = hashRange.location + hashRange.length;
    CFRange specialCaseRange = CFRangeMake(specialCaseStart, strLen - specialCaseStart);
    CFStringRef specialCase = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, fullKey, specialCaseRange);
    Boolean result = _isValidSpecialCase(specialCase);
    
    if (result) {
        if (outBaseKey) {
            *outBaseKey = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, fullKey, baseKeyRange);
        }
        if (outSpecialCase) {
            *outSpecialCase = specialCase;
        } else if (specialCase) {
            CFRelease(specialCase);
        }
    } else if (specialCase) {
        CFRelease(specialCase);
    }
    return result;
}

static Boolean _isCurrentPlatformAndProduct(CFStringRef platform, CFStringRef product) {
    if (!platform && !product) return true;
    if (!platform) {
        return CFEqual(_CFGetProductName(), product);
    }
    if (!product) {
        return CFEqual(_CFGetPlatformName(), platform);
    }
    
    return CFEqual(_CFGetProductName(), product) && CFEqual(_CFGetPlatformName(), platform);
}

static CFArrayRef _CopySortedOverridesForBaseKey(CFStringRef keyName, CFDictionaryRef dict, Boolean const useFallbackKey) {
    CFMutableArrayRef overrides = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    CFStringRef keyNameWithBoth = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@-%@~%@"), keyName, _CFGetPlatformName(), _CFGetProductName());
    CFStringRef keyNameWithProduct = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@~%@"), keyName, _CFGetProductName());
    CFStringRef keyNameWithPlatform = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@-%@"), keyName, _CFGetPlatformName());
    
    CFIndex count = CFDictionaryGetCount(dict);
    
    if (count > 0) {
        CFTypeRef *keys = (CFTypeRef *)CFAllocatorAllocate(kCFAllocatorSystemDefault, 2 * count * sizeof(CFTypeRef), 0);
        CFTypeRef *values = &(keys[count]);
        
        CFDictionaryGetKeysAndValues(dict, keys, values);
        for (CFIndex idx = 0; idx < count; idx++) {
            if (CFEqual(keys[idx], keyNameWithBoth)) {
                CFArrayAppendValue(overrides, keys[idx]);
                break;
            }
        }
        for (CFIndex idx = 0; idx < count; idx++) {
            if (CFEqual(keys[idx], keyNameWithProduct)) {
                CFArrayAppendValue(overrides, keys[idx]);
                break;
            }
        }
        for (CFIndex idx = 0; idx < count; idx++) {
            if (CFEqual(keys[idx], keyNameWithPlatform)) {
                CFArrayAppendValue(overrides, keys[idx]);
                break;
            }
        }

        for (CFIndex idx = 0; idx < count; idx++) {
            if (CFEqual(keys[idx], keyName)) {
                CFArrayAppendValue(overrides, keys[idx]);
                break;
            }
        }
        
        CFAllocatorDeallocate(kCFAllocatorSystemDefault, keys);
    }
    
    CFRelease(keyNameWithProduct);
    CFRelease(keyNameWithPlatform);
    CFRelease(keyNameWithBoth);
    
    return overrides;
}

CF_PRIVATE void _CFBundleInfoPlistProcessInfoDictionary(CFMutableDictionaryRef dict) {
    // Defensive programming
    if (!dict) return;
    
    CFIndex count = CFDictionaryGetCount(dict);
    
    if (count > 0) {
        CFTypeRef *keys = (CFTypeRef *)CFAllocatorAllocate(kCFAllocatorSystemDefault, 2 * count * sizeof(CFTypeRef), 0);
        CFTypeRef *values = &(keys[count]);
        CFMutableArrayRef guard = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
        
        CFDictionaryGetKeysAndValues(dict, keys, values);
        for (CFIndex idx = 0; idx < count; idx++) {
            CFStringRef keyPlatformSuffix, keyProductSuffix, keySpecialCaseSuffix, keyName;
            CFStringRef key = (CFStringRef)keys[idx];

            // Non-string keys in plists aren't valid so remove them
            // if we come across one
            if (CFGetTypeID(key) != _kCFRuntimeIDCFString) {
                CFDictionaryRemoveValue(dict, key);
                continue;
            }

            Boolean const useFallbackPlatformAndProductKey = false;
            if (_isSpecialCaseKey(key, &keyName, &keySpecialCaseSuffix)) {
                // This special case key overrides the base value
                CFDictionarySetValue(dict, keyName, CFDictionaryGetValue(dict, key));
                
                // Remove the special case key
                CFDictionaryRemoveValue(dict, key);
                
                CFRelease(keyName);
                if (keySpecialCaseSuffix) CFRelease(keySpecialCaseSuffix);
                
            } else if (_isPlatformAndProductKey(key, useFallbackPlatformAndProductKey, &keyName, &keyPlatformSuffix, &keyProductSuffix)) {
                CFArrayRef keysForBaseKey = NULL;

                Boolean isSupportedPlatformAndProduct = _isCurrentPlatformAndProduct(keyPlatformSuffix, keyProductSuffix);


                if (isSupportedPlatformAndProduct && !_isBlacklistedKey(keyName) && CFDictionaryContainsKey(dict, key)) {
                    keysForBaseKey = _CopySortedOverridesForBaseKey(keyName, dict, useFallbackPlatformAndProductKey);
                    CFIndex keysForBaseKeyCount = CFArrayGetCount(keysForBaseKey);
                    
                    //make sure the other keys for this base key don't get released out from under us until we're done
                    CFArrayAppendValue(guard, keysForBaseKey); 
                    
                    //the winner for this base key will be sorted to the front, do the override with it
                    CFTypeRef highestPriorityKey = CFArrayGetValueAtIndex(keysForBaseKey, 0);
                    CFDictionarySetValue(dict, keyName, CFDictionaryGetValue(dict, highestPriorityKey));
                    
                    //remove everything except the now-overridden key; this will cause them to fail the CFDictionaryContainsKey(dict, key) check in the enclosing if() and not be reprocessed
                    for (CFIndex presentKeysIdx = 0; presentKeysIdx < keysForBaseKeyCount; presentKeysIdx++) {
                        CFStringRef currentKey = (CFStringRef)CFArrayGetValueAtIndex(keysForBaseKey, presentKeysIdx);
                        if (!CFEqual(currentKey, keyName)) {
                            CFDictionaryRemoveValue(dict, currentKey);
                        }
                    }
                } else {
                    CFDictionaryRemoveValue(dict, key);
                }
                
                
                if (keyPlatformSuffix) CFRelease(keyPlatformSuffix);
                if (keyProductSuffix) CFRelease(keyProductSuffix);
                CFRelease(keyName);
                if (keysForBaseKey) CFRelease(keysForBaseKey);
            }
        }
        
        CFAllocatorDeallocate(kCFAllocatorSystemDefault, keys);
        CFRelease(guard);
    }
}

#pragma mark -

#define DEVELOPMENT_STAGE 0x20
#define ALPHA_STAGE 0x40
#define BETA_STAGE 0x60
#define RELEASE_STAGE 0x80

#define MAX_VERS_LEN 10

CF_INLINE Boolean _isDigit(UniChar aChar) {return ((aChar >= (UniChar)'0' && aChar <= (UniChar)'9') ? true : false);}

static UInt32 _CFVersionNumberFromString(CFStringRef versStr) {
    // Parse version number from string.
    // String can begin with "." for major version number 0.  String can end at any point, but elements within the string cannot be skipped.
    UInt32 major1 = 0, major2 = 0, minor1 = 0, minor2 = 0, stage = RELEASE_STAGE, build = 0;
    UniChar versChars[MAX_VERS_LEN];
    UniChar *chars = NULL;
    CFIndex len;
    UInt32 theVers;
    Boolean digitsDone = false;
    
    if (!versStr) return 0;
    len = CFStringGetLength(versStr);
    if (len <= 0 || len > MAX_VERS_LEN) return 0;
    
    CFStringGetCharacters(versStr, CFRangeMake(0, len), versChars);
    chars = versChars;
    
    // Get major version number.
    major1 = major2 = 0;
    if (_isDigit(*chars)) {
        major2 = *chars - (UniChar)'0';
        chars++;
        len--;
        if (len > 0) {
            if (_isDigit(*chars)) {
                major1 = major2;
                major2 = *chars - (UniChar)'0';
                chars++;
                len--;
                if (len > 0) {
                    if (*chars == (UniChar)'.') {
                        chars++;
                        len--;
                    } else {
                        digitsDone = true;
                    }
                }
            } else if (*chars == (UniChar)'.') {
                chars++;
                len--;
            } else {
                digitsDone = true;
            }
        }
    } else if (*chars == (UniChar)'.') {
        chars++;
        len--;
    } else {
        digitsDone = true;
    }
    
    // Now major1 and major2 contain first and second digit of the major version number as ints.
    // Now either len is 0 or chars points at the first char beyond the first decimal point.
    
    // Get the first minor version number.
    if (len > 0 && !digitsDone) {
        if (_isDigit(*chars)) {
            minor1 = *chars - (UniChar)'0';
            chars++;
            len--;
            if (len > 0) {
                if (*chars == (UniChar)'.') {
                    chars++;
                    len--;
                } else {
                    digitsDone = true;
                }
            }
        } else {
            digitsDone = true;
        }
    }
    
    // Now minor1 contains the first minor version number as an int.
    // Now either len is 0 or chars points at the first char beyond the second decimal point.
    
    // Get the second minor version number.
    if (len > 0 && !digitsDone) {
        if (_isDigit(*chars)) {
            minor2 = *chars - (UniChar)'0';
            chars++;
            len--;
        } else {
            digitsDone = true;
        }
    }
    
    // Now minor2 contains the second minor version number as an int.
    // Now either len is 0 or chars points at the build stage letter.
    
    // Get the build stage letter.  We must find 'd', 'a', 'b', or 'f' next, if there is anything next.
    if (len > 0) {
        if (*chars == (UniChar)'d') {
            stage = DEVELOPMENT_STAGE;
        } else if (*chars == (UniChar)'a') {
            stage = ALPHA_STAGE;
        } else if (*chars == (UniChar)'b') {
            stage = BETA_STAGE;
        } else if (*chars == (UniChar)'f') {
            stage = RELEASE_STAGE;
        } else {
            return 0;
        }
        chars++;
        len--;
    }
    
    // Now stage contains the release stage.
    // Now either len is 0 or chars points at the build number.
    
    // Get the first digit of the build number.
    if (len > 0) {
        if (_isDigit(*chars)) {
            build = *chars - (UniChar)'0';
            chars++;
            len--;
        } else {
            return 0;
        }
    }
    // Get the second digit of the build number.
    if (len > 0) {
        if (_isDigit(*chars)) {
            build *= 10;
            build += *chars - (UniChar)'0';
            chars++;
            len--;
        } else {
            return 0;
        }
    }
    // Get the third digit of the build number.
    if (len > 0) {
        if (_isDigit(*chars)) {
            build *= 10;
            build += *chars - (UniChar)'0';
            chars++;
            len--;
        } else {
            return 0;
        }
    }
    
    // Range check the build number and make sure we exhausted the string.
    if (build > 0xFF || len > 0) return 0;
    
    // Build the number
    theVers = major1 << 28;
    theVers += major2 << 24;
    theVers += minor1 << 20;
    theVers += minor2 << 16;
    theVers += stage << 8;
    theVers += build;
    
    return theVers;
}

#pragma mark -
#pragma mark Info Plist Functions

// If infoPlistUrl is passed as non-null it will return retained as the out parameter; callers are responsible for releasing.
static CFDictionaryRef _CFBundleCopyInfoDictionaryInDirectoryWithVersion(CFAllocatorRef alloc, CFURLRef url, CFURLRef * infoPlistUrl, _CFBundleVersion version) {
    // We only return NULL for a bad URL, otherwise we create a dummy dictionary
    if (!url) return NULL;
    
    CFDictionaryRef result = NULL;
    
    // We're going to search for two files here - Info.plist and Info-macos.plist (platform specific). The platform-specific one takes precedence.
    // First, construct the URL to the directory we'll search by using the passed in URL as a base
    CFStringRef platformInfoURLFromBase = _CFBundlePlatformInfoURLFromBase0;
    CFStringRef infoURLFromBase = _CFBundleInfoURLFromBase0;
    CFURLRef directoryURL = NULL;
    
    if (_CFBundleVersionOldStyleResources == version) {
        directoryURL = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleResourcesURLFromBase0, url);
        platformInfoURLFromBase = _CFBundlePlatformInfoURLFromBase0;
        infoURLFromBase = _CFBundleInfoURLFromBase0;
    } else if (_CFBundleVersionOldStyleSupportFiles == version) {
        directoryURL = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleSupportFilesURLFromBase1, url);
        platformInfoURLFromBase = _CFBundlePlatformInfoURLFromBase1;
        infoURLFromBase = _CFBundleInfoURLFromBase1;
    } else if (_CFBundleVersionContentsResources == version) {
        directoryURL = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleSupportFilesURLFromBase2, url);
        platformInfoURLFromBase = _CFBundlePlatformInfoURLFromBase2;
        infoURLFromBase = _CFBundleInfoURLFromBase2;
    } else if (_CFBundleVersionWrappedContentsResources == version) {
        directoryURL = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleWrappedSupportFilesURLFromBase2, url);
        platformInfoURLFromBase = _CFBundleWrappedPlatformInfoURLFromBase2;
        infoURLFromBase = _CFBundleWrappedInfoURLFromBase2;
    } else if (_CFBundleVersionWrappedFlat == version) {
        directoryURL = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleWrappedSupportFilesURLFromBase3, url);
        platformInfoURLFromBase = _CFBundleWrappedPlatformInfoURLFromBase3;
        infoURLFromBase = _CFBundleWrappedInfoURLFromBase3;
    } else if (_CFBundleVersionFlat == version) {
        CFStringRef path = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
        // this test is necessary to exclude the case where a bundle is spuriously created from the innards of another bundle
        if (path) {
            if (!(CFStringHasSuffix(path, _CFBundleSupportFilesDirectoryName1) || CFStringHasSuffix(path, _CFBundleSupportFilesDirectoryName2) || CFStringHasSuffix(path, _CFBundleResourcesDirectoryName))) {
                directoryURL = (CFURLRef)CFRetain(url);
                platformInfoURLFromBase = _CFBundlePlatformInfoURLFromBase3;
                infoURLFromBase = _CFBundleInfoURLFromBase3;
            }
            CFRelease(path);
        }
    }
    
    CFURLRef absoluteURL;
    if (directoryURL) {
        absoluteURL = CFURLCopyAbsoluteURL(directoryURL);
        CFStringRef directoryPath = CFURLCopyFileSystemPath(absoluteURL, PLATFORM_PATH_STYLE);
        CFRelease(absoluteURL);
        
        __block CFURLRef localInfoPlistURL = NULL;
        __block CFURLRef platformInfoPlistURL = NULL;
        
        if (directoryPath) {
            CFIndex infoPlistLength = CFStringGetLength(_CFBundleInfoPlistName);
            CFIndex platformInfoPlistLength = CFStringGetLength(_CFBundlePlatformInfoPlistName);
            
            // Look inside this directory for the platform-specific and global Info.plist
            // For compatibility reasons, we support case-insensitive versions of Info.plist. That means that we must do a search of all the file names in the directory so we can compare. Otherwise, perhaps a couple of stats would be more efficient than the readdir.
            _CFIterateDirectory(directoryPath, false, NULL, ^Boolean(CFStringRef fileName, CFStringRef fileNameWithPrefix, uint8_t fileType) {
                // Only do the platform check on platforms where the string is different than the normal one
                if (_CFBundlePlatformInfoPlistName != _CFBundleInfoPlistName) {
                    if (!platformInfoPlistURL && CFStringGetLength(fileName) == platformInfoPlistLength && CFStringCompareWithOptions(fileName, _CFBundlePlatformInfoPlistName, CFRangeMake(0, platformInfoPlistLength), kCFCompareCaseInsensitive | kCFCompareAnchored) == kCFCompareEqualTo) {
                        // Make a URL out of this file
                        platformInfoPlistURL = CFURLCreateWithString(kCFAllocatorSystemDefault, platformInfoURLFromBase, url);
                    }
                }
                
                if (!localInfoPlistURL && CFStringGetLength(fileName) == infoPlistLength && CFStringCompareWithOptions(fileName, _CFBundleInfoPlistName, CFRangeMake(0, infoPlistLength), kCFCompareCaseInsensitive | kCFCompareAnchored) == kCFCompareEqualTo) {
                    // Make a URL out of this file
                    localInfoPlistURL = CFURLCreateWithString(kCFAllocatorSystemDefault, infoURLFromBase, url);
                }
                
                // If by some chance we have both URLs, just bail early (or just the localInfoPlistURL on platforms that have no platform-specific name)
                if (_CFBundlePlatformInfoPlistName != _CFBundleInfoPlistName) {
                    if (localInfoPlistURL && platformInfoPlistURL) return false;
                } else {
                    if (localInfoPlistURL) return false;
                }
                
                return true;
            });
            
            CFRelease(directoryPath);
        }
        
        CFRelease(directoryURL);
        
        // Attempt to read in the data from the Info.plist we found - first the platform-specific one.
        CFDataRef infoData = NULL;
        CFURLRef finalInfoPlistURL = NULL;
        if (platformInfoPlistURL) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
            CFURLCreateDataAndPropertiesFromResource(kCFAllocatorSystemDefault, platformInfoPlistURL, &infoData, NULL, NULL, NULL);
#pragma GCC diagnostic pop
            if (infoData) finalInfoPlistURL = platformInfoPlistURL;
        }
        
        if (!infoData && localInfoPlistURL) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
            CFURLCreateDataAndPropertiesFromResource(kCFAllocatorSystemDefault, localInfoPlistURL, &infoData, NULL, NULL, NULL);
#pragma GCC diagnostic pop
            if (infoData) finalInfoPlistURL = localInfoPlistURL;
        }
        
        if (infoData) {
            CFErrorRef error = NULL;
            result = (CFDictionaryRef)CFPropertyListCreateWithData(alloc, infoData, kCFPropertyListMutableContainers, NULL, &error);
            if (result) {
                if (CFDictionaryGetTypeID() != CFGetTypeID(result)) {
                    CFRelease(result);
                    result = NULL;
                }
            } else if (error) {
                // Avoid calling out from CFError (which can cause infinite recursion) by grabbing some of the vital info and printing it ourselves
                CFStringRef domain = CFErrorGetDomain(error);
                CFIndex code = CFErrorGetCode(error);
                 CFLog(kCFLogLevelError, CFSTR("There was an error parsing the Info.plist for the bundle at URL <%p>: %@ - %ld"), localInfoPlistURL, domain, code);
                CFRelease(error);
            }
            
            if (!result) {
                result = CFDictionaryCreateMutable(alloc, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            }
            
            CFRelease(infoData);
        }
        
        if (infoPlistUrl && finalInfoPlistURL) {
            CFRetain(finalInfoPlistURL);
            *infoPlistUrl = finalInfoPlistURL;
        }
        
        if (platformInfoPlistURL) CFRelease(platformInfoPlistURL);
        if (localInfoPlistURL) CFRelease(localInfoPlistURL);
    }
    
    if (!result) {
        result = CFDictionaryCreateMutable(alloc, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
    
    // process ~ipad, ~iphone, etc.
    _CFBundleInfoPlistProcessInfoDictionary((CFMutableDictionaryRef)result);
    
    return result;
}

CF_PRIVATE CFDictionaryRef _CFBundleCopyInfoDictionaryInDirectory(CFAllocatorRef alloc, CFURLRef url, _CFBundleVersion *version) {
    CFDictionaryRef dict = NULL;
    unsigned char buff[CFMaxPathSize];
    _CFBundleVersion localVersion = _CFBundleVersionOldStyleResources;
    
    if (CFURLGetFileSystemRepresentation(url, true, buff, CFMaxPathSize)) {
        CFURLRef newURL = CFURLCreateFromFileSystemRepresentation(kCFAllocatorSystemDefault, buff, strlen((char *)buff), true);
        if (!newURL) newURL = (CFURLRef)CFRetain(url);
        
        localVersion = _CFBundleGetBundleVersionForURL(newURL);
        
        dict = _CFBundleCopyInfoDictionaryInDirectoryWithVersion(alloc, newURL, NULL, localVersion);
        CFRelease(newURL);
    }
    if (version) *version = localVersion;
    return dict;
}

CF_EXPORT CFDictionaryRef CFBundleCopyInfoDictionaryForURL(CFURLRef url) {
    CFDictionaryRef result = NULL;
    Boolean isDir = false;
    if (_CFIsResourceAtURL(url, &isDir)) {
        if (isDir) {
            result = _CFBundleCopyInfoDictionaryInDirectory(kCFAllocatorSystemDefault, url, NULL);
        } else {
            result = _CFBundleCopyInfoDictionaryInExecutable(url);
        }
    }
    return result;
}

static Boolean _CFBundleGetPackageInfoInDirectoryWithInfoDictionary(CFAllocatorRef alloc, CFURLRef url, CFDictionaryRef infoDict, UInt32 *packageType, UInt32 *packageCreator) {
    Boolean retVal = false, hasType = false, hasCreator = false, releaseInfoDict = false;
    CFURLRef tempURL;
    CFDataRef pkgInfoData = NULL;
    
    // Check for a "real" new bundle
    tempURL = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundlePkgInfoURLFromBase2, url);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    CFURLCreateDataAndPropertiesFromResource(kCFAllocatorSystemDefault, tempURL, &pkgInfoData, NULL, NULL, NULL);
#pragma GCC diagnostic pop
    CFRelease(tempURL);
    if (!pkgInfoData) {
        tempURL = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundlePkgInfoURLFromBase1, url);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
        CFURLCreateDataAndPropertiesFromResource(kCFAllocatorSystemDefault, tempURL, &pkgInfoData, NULL, NULL, NULL);
#pragma GCC diagnostic pop
        CFRelease(tempURL);
    }
    if (!pkgInfoData) {
        // Check for a "pseudo" new bundle
        tempURL = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundlePseudoPkgInfoURLFromBase, url);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
        CFURLCreateDataAndPropertiesFromResource(kCFAllocatorSystemDefault, tempURL, &pkgInfoData, NULL, NULL, NULL);
#pragma GCC diagnostic pop
        CFRelease(tempURL);
    }
    
    // Now, either we have a pkgInfoData or not.  If not, then is it because this is a new bundle without one (do we allow this?), or is it dbecause it is an old bundle.
    // If we allow new bundles to not have a PkgInfo (because they already have the same data in the Info.plist), then we have to go read the info plist which makes failure expensive.
    // drd: So we assume that a new bundle _must_ have a PkgInfo if they have this data at all, otherwise we manufacture it from the extension.
    
    if (pkgInfoData && CFDataGetLength(pkgInfoData) >= (int)(sizeof(UInt32) * 2)) {
        UInt32 *pkgInfo = (UInt32 *)CFDataGetBytePtr(pkgInfoData);
        if (packageType) *packageType = CFSwapInt32BigToHost(pkgInfo[0]);
        if (packageCreator) *packageCreator = CFSwapInt32BigToHost(pkgInfo[1]);
        retVal = hasType = hasCreator = true;
    }
    if (pkgInfoData) CFRelease(pkgInfoData);
    if (!retVal) {
        if (!infoDict) {
            infoDict = _CFBundleCopyInfoDictionaryInDirectory(kCFAllocatorSystemDefault, url, NULL);
            releaseInfoDict = true;
        }
        if (infoDict) {
            CFStringRef typeString = (CFStringRef)CFDictionaryGetValue(infoDict, _kCFBundlePackageTypeKey), creatorString = (CFStringRef)CFDictionaryGetValue(infoDict, _kCFBundleSignatureKey);
            UInt32 tmp;
            CFIndex usedBufLen = 0;
            if (typeString && CFGetTypeID(typeString) == CFStringGetTypeID() && CFStringGetLength(typeString) == 4 && 4 == CFStringGetBytes(typeString, CFRangeMake(0, 4), kCFStringEncodingMacRoman, 0, false, (UInt8 *)&tmp, 4, &usedBufLen) && 4 == usedBufLen) {
                if (packageType) *packageType = CFSwapInt32BigToHost(tmp);
                retVal = hasType = true;
            }
            if (creatorString && CFGetTypeID(creatorString) == CFStringGetTypeID() && CFStringGetLength(creatorString) == 4 && 4 == CFStringGetBytes(creatorString, CFRangeMake(0, 4), kCFStringEncodingMacRoman, 0, false, (UInt8 *)&tmp, 4, &usedBufLen) && 4 == usedBufLen) {
                if (packageCreator) *packageCreator = CFSwapInt32BigToHost(tmp);
                retVal = hasCreator = true;
            }
            if (releaseInfoDict) CFRelease(infoDict);
        }
    }
    if (!hasType || !hasCreator) {
        // If this looks like a bundle then manufacture the type and creator.
        if (retVal || _CFBundleURLLooksLikeBundle(url)) {
            if (packageCreator && !hasCreator) *packageCreator = 0x3f3f3f3f;  // '????'
            if (packageType && !hasType) {
                // Detect "app", "debug", "profile", or "framework" extensions
                CFURLRef absoluteURL = CFURLCopyAbsoluteURL(url);
                CFStringRef urlStr = CFURLCopyFileSystemPath(absoluteURL, PLATFORM_PATH_STYLE);
                CFRelease(absoluteURL);
                
                if (urlStr) {
                    UniChar buff[CFMaxPathSize];
                    CFIndex strLen, startOfExtension;
                    
                    strLen = CFStringGetLength(urlStr);
                    if (strLen > CFMaxPathSize) strLen = CFMaxPathSize;
                    CFStringGetCharacters(urlStr, CFRangeMake(0, strLen), buff);
                    CFRelease(urlStr);
                    startOfExtension = _CFStartOfPathExtension(buff, strLen);
                    if ((strLen - startOfExtension == 4 || strLen - startOfExtension == 5) && buff[startOfExtension] == (UniChar)'.' && buff[startOfExtension+1] == (UniChar)'a' && buff[startOfExtension+2] == (UniChar)'p' && buff[startOfExtension+3] == (UniChar)'p' && (strLen - startOfExtension == 4 || buff[startOfExtension+4] == (UniChar)PATH_SEP)) {
                        // This is an app
                        *packageType = 0x4150504c;  // 'APPL'
                    } else if ((strLen - startOfExtension == 6 || strLen - startOfExtension == 7) && buff[startOfExtension] == (UniChar)'.' && buff[startOfExtension+1] == (UniChar)'d' && buff[startOfExtension+2] == (UniChar)'e' && buff[startOfExtension+3] == (UniChar)'b' && buff[startOfExtension+4] == (UniChar)'u' && buff[startOfExtension+5] == (UniChar)'g' && (strLen - startOfExtension == 6 || buff[startOfExtension+6] == (UniChar)PATH_SEP)) {
                        // This is an app (debug version)
                        *packageType = 0x4150504c;  // 'APPL'
                    } else if ((strLen - startOfExtension == 8 || strLen - startOfExtension == 9) && buff[startOfExtension] == (UniChar)'.' && buff[startOfExtension+1] == (UniChar)'p' && buff[startOfExtension+2] == (UniChar)'r' && buff[startOfExtension+3] == (UniChar)'o' && buff[startOfExtension+4] == (UniChar)'f' && buff[startOfExtension+5] == (UniChar)'i' && buff[startOfExtension+6] == (UniChar)'l' && buff[startOfExtension+7] == (UniChar)'e' && (strLen - startOfExtension == 8 || buff[startOfExtension+8] == (UniChar)PATH_SEP)) {
                        // This is an app (profile version)
                        *packageType = 0x4150504c;  // 'APPL'
                    } else if ((strLen - startOfExtension == 8 || strLen - startOfExtension == 9) && buff[startOfExtension] == (UniChar)'.' && buff[startOfExtension+1] == (UniChar)'s' && buff[startOfExtension+2] == (UniChar)'e' && buff[startOfExtension+3] == (UniChar)'r' && buff[startOfExtension+4] == (UniChar)'v' && buff[startOfExtension+5] == (UniChar)'i' && buff[startOfExtension+6] == (UniChar)'c' && buff[startOfExtension+7] == (UniChar)'e' && (strLen - startOfExtension == 8 || buff[startOfExtension+8] == (UniChar)PATH_SEP)) {
                        // This is a service
                        *packageType = 0x4150504c;  // 'APPL'
                    } else if ((strLen - startOfExtension == 10 || strLen - startOfExtension == 11) && buff[startOfExtension] == (UniChar)'.' && buff[startOfExtension+1] == (UniChar)'f' && buff[startOfExtension+2] == (UniChar)'r' && buff[startOfExtension+3] == (UniChar)'a' && buff[startOfExtension+4] == (UniChar)'m' && buff[startOfExtension+5] == (UniChar)'e' && buff[startOfExtension+6] == (UniChar)'w' && buff[startOfExtension+7] == (UniChar)'o' && buff[startOfExtension+8] == (UniChar)'r' && buff[startOfExtension+9] == (UniChar)'k' && (strLen - startOfExtension == 10 || buff[startOfExtension+10] == (UniChar)PATH_SEP)) {
                        // This is a framework
                        *packageType = 0x464d574b;  // 'FMWK'
                    } else {
                        // Default to BNDL for generic bundle
                        *packageType = 0x424e444c;  // 'BNDL'
                    }
                } else {
                    // Default to BNDL for generic bundle
                    *packageType = 0x424e444c;  // 'BNDL'
                }
            }
            retVal = true;
        }
    }
    return retVal;
}

CF_EXPORT Boolean _CFBundleGetPackageInfoInDirectory(CFAllocatorRef alloc, CFURLRef url, UInt32 *packageType, UInt32 *packageCreator) {
    return _CFBundleGetPackageInfoInDirectoryWithInfoDictionary(alloc, url, NULL, packageType, packageCreator);
}

CF_EXPORT void CFBundleGetPackageInfo(CFBundleRef bundle, UInt32 *packageType, UInt32 *packageCreator) {
    CFURLRef bundleURL = CFBundleCopyBundleURL(bundle);
    if (!_CFBundleGetPackageInfoInDirectoryWithInfoDictionary(kCFAllocatorSystemDefault, bundleURL, CFBundleGetInfoDictionary(bundle), packageType, packageCreator)) {
        if (packageType) *packageType = 0x424e444c;  // 'BNDL'
        if (packageCreator) *packageCreator = 0x3f3f3f3f;  // '????'
    }
    if (bundleURL) CFRelease(bundleURL);
}

CF_EXPORT Boolean CFBundleGetPackageInfoInDirectory(CFURLRef url, UInt32 *packageType, UInt32 *packageCreator) {
    return _CFBundleGetPackageInfoInDirectory(kCFAllocatorSystemDefault, url, packageType, packageCreator);
}

CFDictionaryRef CFBundleCopyInfoDictionaryInDirectory(CFURLRef url) {
    CFDictionaryRef dict = _CFBundleCopyInfoDictionaryInDirectory(kCFAllocatorSystemDefault, url, NULL);
    return dict;
}

// The Info.plist should NOT be mutated after being created. If there is any fixing up of the info dictionary to do, do it here.
// Call with bundle lock
static void _CFBundleInfoPlistFixupInfoDictionary(CFBundleRef bundle, CFMutableDictionaryRef infoDict) {
    // Version number
    CFTypeRef unknownVersionValue = CFDictionaryGetValue(infoDict, _kCFBundleNumericVersionKey);
    CFNumberRef versNum;
    UInt32 vers = 0;
    
    if (!unknownVersionValue) unknownVersionValue = CFDictionaryGetValue(infoDict, kCFBundleVersionKey);
    if (unknownVersionValue) {
        if (CFGetTypeID(unknownVersionValue) == CFStringGetTypeID()) {
            // Convert a string version number into a numeric one.
            vers = _CFVersionNumberFromString((CFStringRef)unknownVersionValue);
            
            versNum = CFNumberCreate(CFGetAllocator(bundle), kCFNumberSInt32Type, &vers);
            CFDictionarySetValue(infoDict, _kCFBundleNumericVersionKey, versNum);
            CFRelease(versNum);
        } else if (CFGetTypeID(unknownVersionValue) == CFNumberGetTypeID()) {
            // Nothing to do here
        } else {
            CFDictionaryRemoveValue((CFMutableDictionaryRef)infoDict, _kCFBundleNumericVersionKey);
        }
    }    
}

CF_PRIVATE void _CFBundleRefreshInfoDictionaryAlreadyLocked(CFBundleRef bundle) {
    if (!bundle->_infoDict) {
        CFURLRef infoPlistUrl = NULL;
        bundle->_infoDict = _CFBundleCopyInfoDictionaryInDirectoryWithVersion(kCFAllocatorSystemDefault, bundle->_url, &infoPlistUrl, bundle->_version);
        if (bundle->_infoPlistUrl) {
            CFRelease(bundle->_infoPlistUrl);
        }
        bundle->_infoPlistUrl = infoPlistUrl; // transfered as retained

        // Add or fixup any keys that will be expected later
        if (bundle->_infoDict) _CFBundleInfoPlistFixupInfoDictionary(bundle, (CFMutableDictionaryRef)bundle->_infoDict);
    }
}

CFDictionaryRef CFBundleGetInfoDictionary(CFBundleRef bundle) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    __CFLock(&bundle->_lock);
    _CFBundleRefreshInfoDictionaryAlreadyLocked(bundle);
    __CFUnlock(&bundle->_lock);
    return bundle->_infoDict;
}

CFDictionaryRef _CFBundleGetLocalInfoDictionary(CFBundleRef bundle) {
    return CFBundleGetLocalInfoDictionary(bundle);
}

CFDictionaryRef CFBundleGetLocalInfoDictionary(CFBundleRef bundle) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    CFDictionaryRef localInfoDict = NULL;
    __CFLock(&bundle->_lock);
    localInfoDict = bundle->_localInfoDict;    
    if (!localInfoDict) {
        // To avoid keeping the spin lock for too long, let go of it here while we create a new dictionary. We'll relock later to set the value. If it turns out that we have already created another local info dictionary in the meantime, then we'll take care of it then.
        __CFUnlock(&bundle->_lock);
        CFURLRef url = CFBundleCopyResourceURL(bundle, _CFBundleLocalInfoName, _CFBundleStringTableType, NULL);
        if (url) {
            CFDataRef data;
            SInt32 errCode;
            CFStringRef errStr = NULL;
            
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
            if (CFURLCreateDataAndPropertiesFromResource(kCFAllocatorSystemDefault, url, &data, NULL, NULL, &errCode)) {
                localInfoDict = (CFDictionaryRef)CFPropertyListCreateFromXMLData(kCFAllocatorSystemDefault, data, kCFPropertyListMutableContainers, &errStr);
                if (errStr) CFRelease(errStr);
                if (localInfoDict && CFDictionaryGetTypeID() != CFGetTypeID(localInfoDict)) {
                    CFRelease(localInfoDict);
                    localInfoDict = NULL;
                }
                CFRelease(data);
            }
#pragma GCC diagnostic pop
            CFRelease(url);
        }
        if (localInfoDict) _CFBundleInfoPlistProcessInfoDictionary((CFMutableDictionaryRef)localInfoDict);
        // remain locked here until we exit the if statement.
        __CFLock(&bundle->_lock);
        if (!bundle->_localInfoDict) {
            // Still have no info dictionary, so set it
            bundle->_localInfoDict = localInfoDict;
        } else {
            // Oops, some other thread created an info dictionary too. We'll just release this one and use that one.
            if (localInfoDict) CFRelease(localInfoDict);
            localInfoDict = bundle->_localInfoDict;
        }
    }
    __CFUnlock(&bundle->_lock);

    return localInfoDict;
}

CFPropertyListRef _CFBundleGetValueForInfoKey(CFBundleRef bundle, CFStringRef key) {
    return (CFPropertyListRef)CFBundleGetValueForInfoDictionaryKey(bundle, key);
}

CFTypeRef CFBundleGetValueForInfoDictionaryKey(CFBundleRef bundle, CFStringRef key) {
    // Look in InfoPlist.strings first.  Then look in Info.plist
    CFTypeRef result = NULL;
    if (bundle && key) {
        CFDictionaryRef dict = CFBundleGetLocalInfoDictionary(bundle);
        if (dict) result = CFDictionaryGetValue(dict, key);
        if (!result) {
            dict = CFBundleGetInfoDictionary(bundle);
            if (dict) result = CFDictionaryGetValue(dict, key);
        }
    }
    return result;
}

CFStringRef CFBundleGetIdentifier(CFBundleRef bundle) {
    CFStringRef bundleID = NULL;
    CFDictionaryRef infoDict = CFBundleGetInfoDictionary(bundle);
    if (infoDict) bundleID = (CFStringRef)CFDictionaryGetValue(infoDict, kCFBundleIdentifierKey);
    return bundleID;
}

static void __addSuffixesToKeys(const void *value, void *context) {
    CFMutableSetRef newKeys = (CFMutableSetRef)context;
    CFStringRef key = (CFStringRef)value;
    CFStringRef firstPartOfKey = NULL;
    CFStringRef restOfKey = NULL;
    
    // Find the first ':'
    CFRange range;
    Boolean success = CFStringFindWithOptions(key, CFSTR(":"), CFRangeMake(0, CFStringGetLength(key)), 0, &range);
    if (success) {
        firstPartOfKey = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, key, CFRangeMake(0, range.location));
        restOfKey = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, key, CFRangeMake(range.location + 1, CFStringGetLength(key) - range.location - 1));
    } else {
        firstPartOfKey = (CFStringRef)CFRetain(key);
    }
    
    // only apply product and platform to top-level key
    CFStringRef newKeyWithPlatform = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@-%@%@%@"), firstPartOfKey, _CFGetPlatformName(), restOfKey ? CFSTR(":") : CFSTR(""), restOfKey ? restOfKey : CFSTR(""));
    CFStringRef newKeyWithProduct = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@~%@%@%@"), firstPartOfKey, _CFGetProductName(), restOfKey ? CFSTR(":") : CFSTR(""), restOfKey ? restOfKey : CFSTR(""));
    CFStringRef newKeyWithProductAndPlatform = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@-%@~%@%@%@"), firstPartOfKey, _CFGetPlatformName(), _CFGetProductName(), restOfKey ? CFSTR(":") : CFSTR(""), restOfKey ? restOfKey : CFSTR(""));
    
    CFSetAddValue(newKeys, key);
    CFSetAddValue(newKeys, newKeyWithPlatform);
    CFSetAddValue(newKeys, newKeyWithProduct);
    CFSetAddValue(newKeys, newKeyWithProductAndPlatform);
    
    CFRelease(newKeyWithPlatform);
    CFRelease(newKeyWithProduct);
    CFRelease(newKeyWithProductAndPlatform);

    // Add special case keys
    CFStringRef overrideSpecialCase = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@#override%@%@"), firstPartOfKey, restOfKey ? CFSTR(":") : CFSTR(""), restOfKey ? restOfKey : CFSTR(""));
    CFSetAddValue(newKeys, overrideSpecialCase);
    CFRelease(overrideSpecialCase);
    
    if (firstPartOfKey) CFRelease(firstPartOfKey);
    if (restOfKey) CFRelease(restOfKey);
}

// from CFUtilities.c
CF_PRIVATE Boolean _CFReadMappedFromFile(CFStringRef path, Boolean map, Boolean uncached, void **outBytes, CFIndex *outLength, CFErrorRef *errorPtr);

// Ensure keyPaths are actually `CFString`
static void __validPlistKeys(const void *value, void *context) {
    CFStringRef key = (CFStringRef)value;
    if (CFGetTypeID(key) != _kCFRuntimeIDCFString) {
        HALT_MSG("Property lists must have string keys!");
    }
}

// implementation of below functions - takes URL as parameter
static CFPropertyListRef _CFBundleCreateFilteredInfoPlistWithURL(CFURLRef infoPlistURL, CFSetRef keyPaths, _CFBundleFilteredPlistOptions options) {
    CFPropertyListRef result = NULL;
    
    if (!infoPlistURL) return CFDictionaryCreate(kCFAllocatorSystemDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    CFURLRef absoluteURL = CFURLCopyAbsoluteURL(infoPlistURL);
    CFStringRef filePath = CFURLCopyFileSystemPath(absoluteURL, PLATFORM_PATH_STYLE);
    CFRelease(absoluteURL);
    
    if (!filePath) return CFDictionaryCreate(kCFAllocatorSystemDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    void *bytes = NULL;
    CFIndex length = 0;
#if TARGET_OS_MAC
    Boolean mapped = options & _CFBundleFilteredPlistMemoryMapped ? true : false;
#else
    Boolean mapped = false;
#endif
    Boolean success = _CFReadMappedFromFile(filePath, mapped, false, &bytes, &length, NULL);
    CFRelease(filePath);
    if (!success) return CFDictionaryCreate(kCFAllocatorSystemDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    CFDataRef infoPlistData = CFDataCreateWithBytesNoCopy(kCFAllocatorSystemDefault, (const UInt8 *)bytes, length, kCFAllocatorNull);

    // Ensure `keyPaths` are all actually `CFString`s
    // if not `HALT` on first invalid keyPath
    CFSetApplyFunction(keyPaths, __validPlistKeys, NULL);

    // We need to include all possible variants of the platform/product combo as possible keys.
    CFMutableSetRef newKeyPaths = CFSetCreateMutable(kCFAllocatorSystemDefault, CFSetGetCount(keyPaths), &kCFTypeSetCallBacks);
    CFSetApplyFunction(keyPaths, __addSuffixesToKeys, newKeyPaths);
    
    success = _CFPropertyListCreateFiltered(kCFAllocatorSystemDefault, infoPlistData, kCFPropertyListMutableContainers, newKeyPaths, &result, NULL);
    
    if (!success || !result) {
        result = CFDictionaryCreate(kCFAllocatorSystemDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    } else if (CFDictionaryGetTypeID() == CFGetTypeID(result)) {
        _CFBundleInfoPlistProcessInfoDictionary((CFMutableDictionaryRef)result);
    } else {
        CFRelease(result);
        CFLog(kCFLogLevelError, CFSTR("A filtered Info.plist result was not a dictionary at URL %@ (for key paths %@)"), infoPlistURL, keyPaths);
        result = CFDictionaryCreate(kCFAllocatorSystemDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
    
    CFRelease(newKeyPaths);
    CFRelease(infoPlistData);
    if (mapped) {
#if TARGET_OS_MAC
        munmap(bytes, length);
#endif
    } else {
        free(bytes);
    }
    
    return result;
}

// Returns a subset of the bundle's property list, only including the keyPaths in the CFSet. If the top level object is not a dictionary, you will get back an empty dictionary as the result. If the Info.plist does not exist or could not be parsed, you will get back an empty dictionary.
CF_EXPORT CFPropertyListRef _CFBundleCreateFilteredInfoPlist(CFBundleRef bundle, CFSetRef keyPaths, _CFBundleFilteredPlistOptions options) {
    CFURLRef infoPlistURL = _CFBundleCopyInfoPlistURL(bundle);
    CFPropertyListRef result = _CFBundleCreateFilteredInfoPlistWithURL(infoPlistURL, keyPaths, options);
    if (infoPlistURL) CFRelease(infoPlistURL);
    return result;
}

CF_EXPORT CFPropertyListRef _CFBundleCreateFilteredLocalizedInfoPlist(CFBundleRef bundle, CFSetRef keyPaths, CFStringRef localizationName, _CFBundleFilteredPlistOptions options) {
    CFURLRef infoPlistURL = CFBundleCopyResourceURLForLocalization(bundle, _CFBundleLocalInfoName, _CFBundleStringTableType, NULL, localizationName);
    CFPropertyListRef result = _CFBundleCreateFilteredInfoPlistWithURL(infoPlistURL, keyPaths, options);
    if (infoPlistURL) CFRelease(infoPlistURL);
    return result;
}

CF_EXPORT CFURLRef _CFBundleCopyInfoPlistURL(CFBundleRef bundle) {
    __CFLock(&bundle->_lock);
    CFURLRef url = bundle->_infoPlistUrl;
    CFURLRef result = (url ? (CFURLRef) CFRetain(url) : NULL);
    __CFUnlock(&bundle->_lock);
    return result;
}
