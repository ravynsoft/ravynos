/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*      CFBundle_Strings.c
        Copyright (c) 1999-2014, Apple Inc.  All rights reserved.
        Responsibility: Tony Parker
*/

#include "CFBundle_Internal.h"
#if DEPLOYMENT_TARGET_MACOSX

#endif

#include <CoreFoundation/CFPreferences.h>
#include <CoreFoundation/CFURLAccess.h>

#pragma mark -
#pragma mark Localized Strings


CF_EXPORT CFStringRef CFBundleCopyLocalizedString(CFBundleRef bundle, CFStringRef key, CFStringRef value, CFStringRef tableName) {
    return CFBundleCopyLocalizedStringForLocalization(bundle, key, value, tableName, NULL);
}

CF_EXPORT CFStringRef CFBundleCopyLocalizedStringForLocalization(CFBundleRef bundle, CFStringRef key, CFStringRef value, CFStringRef tableName, CFStringRef localizationName) {
    CFStringRef result = NULL;
    CFDictionaryRef stringTable = NULL;
    
    if (!key) return (value ? (CFStringRef)CFRetain(value) : (CFStringRef)CFRetain(CFSTR("")));
    
    // Make sure to check the mixed localizations key early -- if the main bundle has not yet been cached, then we need to create the cache of the Info.plist before we start asking for resources (11172381)
    (void)CFBundleAllowMixedLocalizations();
    
    if (!tableName || CFEqual(tableName, CFSTR(""))) tableName = _CFBundleDefaultStringTableName;
    
    __CFLock(&bundle->_lock);
    // Only consult the cache when a specific localization has not been requested. We only cache results for the preferred language as determined by normal bundle lookup rules.
    if (!localizationName && bundle->_stringTable) {
        stringTable = (CFDictionaryRef)CFDictionaryGetValue(bundle->_stringTable, tableName);
        if (stringTable) CFRetain(stringTable);
    }
    
    if (!stringTable) {
        // Go load the table. First, unlock so we don't hold the lock across file system access.
        __CFUnlock(&bundle->_lock);
        
        CFURLRef tableURL = NULL;
        if (localizationName) {
            tableURL = CFBundleCopyResourceURLForLocalization(bundle, tableName, _CFBundleStringTableType, NULL, localizationName);
        } else {
            tableURL = CFBundleCopyResourceURL(bundle, tableName, _CFBundleStringTableType, NULL);
        }
        
        if (tableURL) {
            CFStringRef nameForSharing = NULL;
            if (!stringTable) {
                CFDataRef tableData = NULL;
                SInt32 errCode;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
                if (CFURLCreateDataAndPropertiesFromResource(kCFAllocatorSystemDefault, tableURL, &tableData, NULL, NULL, &errCode)) {
#pragma GCC diagnostic pop
                    CFErrorRef error = NULL;
                    stringTable = (CFDictionaryRef)CFPropertyListCreateWithData(CFGetAllocator(bundle), tableData, kCFPropertyListImmutable, NULL, &error);
                    if (stringTable && CFDictionaryGetTypeID() != CFGetTypeID(stringTable)) {
                        CFRelease(stringTable);
                        stringTable = NULL;
                    }
                    if (!stringTable && error) {
                        CFLog(kCFLogLevelError, CFSTR("Unable to load string table file: %@ / %@: %@"), bundle, tableName, error);
                        CFRelease(error);
                        error = NULL;
                    }
                    CFRelease(tableData);
                    
                }
            }
            if (nameForSharing) CFRelease(nameForSharing);
            if (tableURL) CFRelease(tableURL);
        }
        if (!stringTable) stringTable = CFDictionaryCreate(CFGetAllocator(bundle), NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        
        if ((!CFStringHasSuffix(tableName, CFSTR(".nocache")) || !_CFExecutableLinkedOnOrAfter(CFSystemVersionLeopard)) && localizationName == NULL) {
            // Take lock again, because this we will unlock after getting the value out of the table.
            __CFLock(&bundle->_lock);
            if (!bundle->_stringTable) bundle->_stringTable = CFDictionaryCreateMutable(CFGetAllocator(bundle), 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            
            // If another thread beat us to setting this tableName, then we'll just replace it here.
            CFDictionarySetValue(bundle->_stringTable, tableName, stringTable);
        } else {
            // Take lock again, because this we will unlock after getting the value out of the table.
            __CFLock(&bundle->_lock);
        }
    }
    
    result = (CFStringRef)CFDictionaryGetValue(stringTable, key);
    if (result) {
        CFRetain(result);
    }
    
    __CFUnlock(&bundle->_lock);
    CFRelease(stringTable);
    
    if (!result) {
        if (!value) {
            result = (CFStringRef)CFRetain(key);
        } else if (CFEqual(value, CFSTR(""))) {
            result = (CFStringRef)CFRetain(key);
        } else {
            result = (CFStringRef)CFRetain(value);
        }
        static Boolean capitalize = false;
        if (capitalize) {
            CFMutableStringRef capitalizedResult = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, 0, result);
            CFLog(__kCFLogBundle, CFSTR("Localizable string \"%@\" not found in strings table \"%@\" of bundle %@."), key, tableName, bundle);
            CFStringUppercase(capitalizedResult, NULL);
            CFRelease(result);
            result = capitalizedResult;
        }
    }
    
    return result;
}

