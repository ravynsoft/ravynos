/*      CFBundle_Strings.c
	Copyright (c) 1999-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
        Responsibility: Tony Parker
*/

#include "CFBundle_Internal.h"
#include "CFCollections_Internal.h"

#if TARGET_OS_OSX

#endif

#include <CoreFoundation/CFPreferences.h>
#include <CoreFoundation/CFURLAccess.h>

#pragma mark -
#pragma mark Localized Strings

static void __CFStringsDictMergeApplyFunction(const void *key, const void *value, void *context) {
    CFDictionarySetValue((CFMutableDictionaryRef)context, key, value);
}


CF_EXPORT CFStringRef CFBundleCopyLocalizedString(CFBundleRef bundle, CFStringRef key, CFStringRef value, CFStringRef tableName) {
    return CFBundleCopyLocalizedStringForLocalization(bundle, key, value, tableName, NULL);
}


/* outActualTableFile is the URL to a localization table file we're getting strings from. It may be set to NULL on return to mean that we have pulled this from the cache of the preferred language, which is fine since we want this URL to determine which localization was picked. */
static CFStringRef _copyStringFromTable(CFBundleRef bundle, CFStringRef tableName, CFStringRef key, CFStringRef localizationName, Boolean preventMarkdownParsing, CFURLRef *outActualLocalizationFile) {
    // Check the cache first. If it's not there, populate the cache and check again.
    
    __CFLock(&bundle->_lock);
    // Only consult the cache when a specific localization has not been requested. We only cache results for the preferred language as determined by normal bundle lookup rules.
    if (!localizationName && bundle->_stringTable) {
        CFDictionaryRef stringTable = (CFDictionaryRef)CFDictionaryGetValue(bundle->_stringTable, tableName);
        if (stringTable) {
            CFStringRef result = CFDictionaryGetValue(stringTable, key);
            if (result) {
                CFRetain(result);
            }
            __CFUnlock(&bundle->_lock);
            
            if (outActualLocalizationFile) {
                *outActualLocalizationFile = NULL; // Preferred localization.
            }
            return result;
        }
    }

    // Not in the local cache, so load the table. Unlock so we don't hold the lock across file system access.
    __CFUnlock(&bundle->_lock);

    CFDictionaryRef stringsTable = NULL;
    CFURLRef stringsTableURL = NULL;
    CFURLRef stringsDictTableURL = NULL;
    
    // Find the resource URL.
    if (localizationName) {
        stringsTableURL = CFBundleCopyResourceURLForLocalization(bundle, tableName, _CFBundleStringTableType, NULL, localizationName);
        stringsDictTableURL = CFBundleCopyResourceURLForLocalization(bundle, tableName, _CFBundleStringDictTableType, NULL, localizationName);
    } else {
        stringsTableURL = CFBundleCopyResourceURL(bundle, tableName, _CFBundleStringTableType, NULL);
        stringsDictTableURL = CFBundleCopyResourceURL(bundle, tableName, _CFBundleStringDictTableType, NULL);
    }

    // Next, look on disk for the regular strings file.
    if (stringsTableURL) {
        CFDataRef tableData = _CFDataCreateFromURL(stringsTableURL, NULL);
        if (tableData) {
            CFErrorRef error = NULL;
            stringsTable = (CFDictionaryRef)CFPropertyListCreateWithData(CFGetAllocator(bundle), tableData, kCFPropertyListImmutable, NULL, &error);
            CFRelease(tableData);
            
            if (stringsTable && CFDictionaryGetTypeID() != CFGetTypeID(stringsTable)) {
                os_log_error(_CFBundleLocalizedStringLogger(), "Unable to load .strings file: %@ / %@: Top-level object was not a dictionary", bundle, tableName);
                CFRelease(stringsTable);
                stringsTable = NULL;
            } else if (!stringsTable && error) {
                os_log_error(_CFBundleLocalizedStringLogger(), "Unable to load .strings file: %@ / %@: %@", bundle, tableName, error);
                CFRelease(error);
                error = NULL;
            }
        }        
    }
    
    // Check for a .stringsdict file.
    if (stringsDictTableURL) {
        CFDataRef tableData = _CFDataCreateFromURL(stringsDictTableURL, NULL);
        if (tableData) {
            CFErrorRef error = NULL;
            CFDictionaryRef stringsDictTable = (CFDictionaryRef)CFPropertyListCreateWithData(CFGetAllocator(bundle), tableData, kCFPropertyListImmutable, NULL, &error);
            CFRelease(tableData);
            
            if (!stringsDictTable && error) {
                os_log_error(_CFBundleLocalizedStringLogger(), "Unable to load .stringsdict file: %@ / %@: %@", bundle, tableName, error);
                CFRelease(error);
                error = NULL;
            } else if (stringsDictTable && CFDictionaryGetTypeID() != CFGetTypeID(stringsDictTable)) {
                os_log_error(_CFBundleLocalizedStringLogger(), "Unable to load .stringsdict file: %@ / %@: Top-level object was not a dictionary", bundle, tableName);
                CFRelease(stringsDictTable);
                stringsDictTable = NULL;
            } else if (stringsDictTable) {
                // Post-process the strings table.
                CFMutableDictionaryRef mutableStringsDictTable;
                if (stringsTable) {
                    // Any strings that are in the stringsTable that are not in the stringsDict must be added to the stringsDict.
                    // However, any entry in the stringsDictTable must override the content from stringsTable.
                    
                    // Start by copying the stringsTable.
                    mutableStringsDictTable = CFDictionaryCreateMutableCopy(NULL, 0, stringsTable);
                    
                    // Replace any stringsTable entries with entries from stringsDictTable. This will override any entries from the original stringsTable if they existed.
                    CFDictionaryApplyFunction(stringsDictTable, __CFStringsDictMergeApplyFunction, mutableStringsDictTable);
                } else {
                    // Start with a copy of the stringsDictTable on its own.
                    mutableStringsDictTable = CFDictionaryCreateMutableCopy(NULL, 0, stringsDictTable);
                }
                
                CFRelease(stringsDictTable);

                if (stringsTable) CFRelease(stringsTable);
                // The new strings table is the result of all the transforms above.
                stringsTable = mutableStringsDictTable;
                
                if (outActualLocalizationFile) {
                    *outActualLocalizationFile = CFRetain(stringsDictTableURL);
                }
            }
        }
    }
    
    if (outActualLocalizationFile && !*outActualLocalizationFile && stringsTableURL) {
        *outActualLocalizationFile = CFRetain(stringsTableURL);
    }
    
    if (stringsTableURL) CFRelease(stringsTableURL);
    if (stringsDictTableURL) CFRelease(stringsDictTableURL);
    
    // Last resort: create an empty table
    if (!stringsTable) {
        os_log_debug(_CFBundleLocalizedStringLogger(), "Hit last resort and creating empty strings table");
        stringsTable = CFDictionaryCreate(CFGetAllocator(bundle), NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
    
    // Insert the result into our local cache
    if ((!CFStringHasSuffix(tableName, CFSTR(".nocache")) || !_CFExecutableLinkedOnOrAfter(CFSystemVersionLeopard)) && localizationName == NULL) {
        // Take lock again, because this we will unlock after getting the value out of the table.
        __CFLock(&bundle->_lock);
        if (!bundle->_stringTable) bundle->_stringTable = CFDictionaryCreateMutable(CFGetAllocator(bundle), 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        
        // If another thread beat us to setting this tableName, then we'll just replace it here.
        CFDictionarySetValue(bundle->_stringTable, tableName, stringsTable);
    } else {
        // Take lock again, because this we will unlock after getting the value out of the table.
        __CFLock(&bundle->_lock);
    }
    
    // Finally, fetch the result from the table
    CFStringRef result = CFDictionaryGetValue(stringsTable, key);
    if (result) {
        CFRetain(result);
    }
    __CFUnlock(&bundle->_lock);

    CFRelease(stringsTable);
    
    return result;
}

CF_EXPORT CFStringRef _CFBundleCopyLocalizedStringForLocalizationTableURLAndMarkdownOption(CFBundleRef bundle, CFStringRef key, CFStringRef value, CFStringRef tableName, CFStringRef localizationName, Boolean preventMarkdownParsing, CFURLRef *outActualTableURL) {

    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    if (!key) { return (value ? (CFStringRef)CFRetain(value) : (CFStringRef)CFRetain(CFSTR(""))); }
    
    // Make sure to check the mixed localizations key early -- if the main bundle has not yet been cached, then we need to create the cache of the Info.plist before we start asking for resources (11172381)
    (void)CFBundleAllowMixedLocalizations();
    
    if (!tableName || CFEqual(tableName, CFSTR(""))) tableName = _CFBundleDefaultStringTableName;
    
    CFURLRef actualTableURL = NULL;
    CFStringRef result = _copyStringFromTable(bundle, tableName, key, localizationName, preventMarkdownParsing, &actualTableURL);
    
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
            os_log_error(_CFBundleLocalizedStringLogger(), "ERROR: %@ not found in table %@ of bundle %@", key, tableName, bundle);
            CFStringUppercase(capitalizedResult, NULL);
            CFRelease(result);
            result = capitalizedResult;
        }
    }
    if (outActualTableURL) {
        *outActualTableURL = actualTableURL;
    } else if (actualTableURL) {
        CFRelease(actualTableURL);
    }
    
    os_log_debug(_CFBundleLocalizedStringLogger(), "Bundle: %{private}@, key: %{public}@, value: %{public}@, table: %{public}@, localizationName: %{public}@, result: %{public}@", bundle, key, value, tableName, localizationName, result);
    return result;
}

CF_EXPORT CFStringRef _CFBundleCopyLocalizedStringForLocalizationAndTableURL(CFBundleRef bundle, CFStringRef key, CFStringRef value, CFStringRef tableName, CFStringRef localizationName, CFURLRef *outActualTableURL) {
    return _CFBundleCopyLocalizedStringForLocalizationTableURLAndMarkdownOption(bundle, key, value, tableName, localizationName, false, outActualTableURL);
}

CF_EXPORT CFStringRef CFBundleCopyLocalizedStringForLocalization(CFBundleRef bundle, CFStringRef key, CFStringRef value, CFStringRef tableName, CFStringRef localizationName) {
    return _CFBundleCopyLocalizedStringForLocalizationTableURLAndMarkdownOption(bundle, key, value, tableName, localizationName, false, NULL);
}
