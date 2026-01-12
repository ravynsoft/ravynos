/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

typedef struct __NSBundle *CFBundleRef;

#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFURL.h>
#import <CoreFoundation/CFArray.h>
#import <CoreFoundation/CFDictionary.h>
#import <CoreFoundation/CFError.h>
#import <CoreFoundation/CFPlugIn.h>

COREFOUNDATION_EXPORT const CFStringRef kCFBundleNameKey;
COREFOUNDATION_EXPORT const CFStringRef kCFBundleVersionKey;
COREFOUNDATION_EXPORT const CFStringRef kCFBundleIdentifierKey;
COREFOUNDATION_EXPORT const CFStringRef kCFBundleInfoDictionaryVersionKey;
COREFOUNDATION_EXPORT const CFStringRef kCFBundleLocalizationsKey;
COREFOUNDATION_EXPORT const CFStringRef kCFBundleExecutableKey;
COREFOUNDATION_EXPORT const CFStringRef kCFBundleDevelopmentRegionKey;

COREFOUNDATION_EXPORT CFTypeID CFBundleGetTypeID(void);

COREFOUNDATION_EXPORT CFBundleRef CFBundleGetMainBundle(void);
COREFOUNDATION_EXPORT CFArrayRef CFBundleGetAllBundles(void);
COREFOUNDATION_EXPORT CFStringRef CFCopyLocalizedString(CFStringRef key, const char *comment);
COREFOUNDATION_EXPORT CFStringRef CFCopyLocalizedStringFromTable(CFStringRef key, CFStringRef tableName, const char *comment);

COREFOUNDATION_EXPORT CFBundleRef CFBundleCreate(CFAllocatorRef allocator, CFURLRef bundleURL);
COREFOUNDATION_EXPORT CFArrayRef CFBundleCreateBundlesFromDirectory(CFAllocatorRef allocator, CFURLRef directoryURL, CFStringRef bundleType);

COREFOUNDATION_EXPORT CFURLRef CFBundleCopyAuxiliaryExecutableURL(CFBundleRef self, CFStringRef executableName);
COREFOUNDATION_EXPORT CFURLRef CFBundleCopyBuiltInPlugInsURL(CFBundleRef self);
COREFOUNDATION_EXPORT CFArrayRef CFBundleCopyBundleLocalizations(CFBundleRef self);
COREFOUNDATION_EXPORT CFURLRef CFBundleCopyBundleURL(CFBundleRef self);
COREFOUNDATION_EXPORT CFArrayRef CFBundleCopyExecutableArchitectures(CFBundleRef self);
COREFOUNDATION_EXPORT CFArrayRef CFBundleCopyExecutableArchitecturesForURL(CFURLRef url);
COREFOUNDATION_EXPORT CFURLRef CFBundleCopyExecutableURL(CFBundleRef self);
COREFOUNDATION_EXPORT CFDictionaryRef CFBundleCopyInfoDictionaryForURL(CFURLRef url);
COREFOUNDATION_EXPORT CFDictionaryRef CFBundleCopyInfoDictionaryInDirectory(CFURLRef bundleURL);
COREFOUNDATION_EXPORT CFArrayRef CFBundleCopyLocalizationsForPreferences(CFArrayRef locArray, CFArrayRef prefArray);
COREFOUNDATION_EXPORT CFArrayRef CFBundleCopyLocalizationsForURL(CFURLRef url);
COREFOUNDATION_EXPORT CFStringRef CFBundleCopyLocalizedString(CFBundleRef self, CFStringRef key, CFStringRef value, CFStringRef tableName);
COREFOUNDATION_EXPORT CFArrayRef CFBundleCopyPreferredLocalizationsFromArray(CFArrayRef locArray);
COREFOUNDATION_EXPORT CFURLRef CFBundleCopyPrivateFrameworksURL(CFBundleRef self);

COREFOUNDATION_EXPORT CFURLRef CFBundleCopyResourcesDirectoryURL(CFBundleRef self);
COREFOUNDATION_EXPORT CFURLRef CFBundleCopyResourceURL(CFBundleRef self, CFStringRef resourceName, CFStringRef resourceType, CFStringRef subDirName);
COREFOUNDATION_EXPORT CFURLRef CFBundleCopyResourceURLForLocalization(CFBundleRef self, CFStringRef resourceName, CFStringRef resourceType, CFStringRef subDirName, CFStringRef localizationName);
COREFOUNDATION_EXPORT CFURLRef CFBundleCopyResourceURLInDirectory(CFURLRef bundleURL, CFStringRef resourceName, CFStringRef resourceType, CFStringRef subDirName);

COREFOUNDATION_EXPORT CFArrayRef CFBundleCopyResourceURLsOfType(CFBundleRef self, CFStringRef resourceType, CFStringRef subDirName);
COREFOUNDATION_EXPORT CFArrayRef CFBundleCopyResourceURLsOfTypeForLocalization(CFBundleRef self, CFStringRef resourceType, CFStringRef subDirName, CFStringRef localizationName);
COREFOUNDATION_EXPORT CFArrayRef CFBundleCopyResourceURLsOfTypeInDirectory(CFURLRef bundleURL, CFStringRef resourceType, CFStringRef subDirName);
COREFOUNDATION_EXPORT CFURLRef CFBundleCopySharedFrameworksURL(CFBundleRef self);
COREFOUNDATION_EXPORT CFURLRef CFBundleCopySharedSupportURL(CFBundleRef self);
COREFOUNDATION_EXPORT CFURLRef CFBundleCopySupportFilesDirectoryURL(CFBundleRef self);
COREFOUNDATION_EXPORT CFBundleRef CFBundleGetBundleWithIdentifier(CFStringRef bundleID);
COREFOUNDATION_EXPORT void *CFBundleGetDataPointerForName(CFBundleRef self, CFStringRef name);
COREFOUNDATION_EXPORT void CFBundleGetDataPointersForNames(CFBundleRef self, CFArrayRef names, void *symbolTable[]);
COREFOUNDATION_EXPORT CFStringRef CFBundleGetDevelopmentRegion(CFBundleRef self);
COREFOUNDATION_EXPORT void *CFBundleGetFunctionPointerForName(CFBundleRef self, CFStringRef name);
COREFOUNDATION_EXPORT void CFBundleGetFunctionPointersForNames(CFBundleRef self, CFArrayRef names, void *functionTable[]);
COREFOUNDATION_EXPORT CFStringRef CFBundleGetIdentifier(CFBundleRef self);
COREFOUNDATION_EXPORT CFDictionaryRef CFBundleGetInfoDictionary(CFBundleRef self);
COREFOUNDATION_EXPORT CFDictionaryRef CFBundleGetLocalInfoDictionary(CFBundleRef self);
COREFOUNDATION_EXPORT void CFBundleGetPackageInfo(CFBundleRef self, CFUInteger *packageType, CFUInteger *packageCreator);
COREFOUNDATION_EXPORT Boolean CFBundleGetPackageInfoInDirectory(CFURLRef url, CFUInteger *packageType, CFUInteger *packageCreator);
COREFOUNDATION_EXPORT CFPlugInRef CFBundleGetPlugIn(CFBundleRef self);
COREFOUNDATION_EXPORT CFTypeRef CFBundleGetValueForInfoDictionaryKey(CFBundleRef self, CFStringRef key);
COREFOUNDATION_EXPORT CFUInteger CFBundleGetVersionNumber(CFBundleRef self);
COREFOUNDATION_EXPORT Boolean CFBundleIsExecutableLoaded(CFBundleRef self);
COREFOUNDATION_EXPORT Boolean CFBundleLoadExecutable(CFBundleRef self);
COREFOUNDATION_EXPORT Boolean CFBundleLoadExecutableAndReturnError(CFBundleRef self, CFErrorRef *error);
COREFOUNDATION_EXPORT Boolean CFBundlePreflightExecutable(CFBundleRef self, CFErrorRef *error);
COREFOUNDATION_EXPORT void CFBundleUnloadExecutable(CFBundleRef self);
COREFOUNDATION_EXPORT CFStringRef CFCopyLocalizedStringFromTableInBundle(CFStringRef key, CFStringRef tableName, CFBundleRef self, const char *comment);
COREFOUNDATION_EXPORT CFStringRef CFCopyLocalizedStringWithDefaultValue(CFStringRef key, CFStringRef tableName, CFBundleRef self, CFStringRef value, const char *comment);
