#import <CoreFoundation/CFBundle.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSString.h>

const CFStringRef kCFBundleNameKey=(CFStringRef)@"CFBundleName";
const CFStringRef kCFBundleVersionKey=(CFStringRef)@"CFBundleVersion";
const CFStringRef kCFBundleIdentifierKey=(CFStringRef)@"CFBundleIdentifier";
const CFStringRef kCFBundleInfoDictionaryVersionKey=(CFStringRef)@"CFBundleInfoDictionaryVersion";
const CFStringRef kCFBundleLocalizationsKey=(CFStringRef)@"CFBundleLocalizations";
const CFStringRef kCFBundleExecutableKey=(CFStringRef)@"CFBundleExecutable";
const CFStringRef kCFBundleDevelopmentRegionKey=(CFStringRef)@"CFBundleDevelopmentRegion";

#define NSBundleToCFBundle(x) ((CFBundleRef)x)
#define CFBundleToNSBundle(x) ((NSBundle *)x)

CFBundleRef CFBundleGetMainBundle(void) {
   return NSBundleToCFBundle([NSBundle mainBundle]);
}

CFBundleRef CFBundleCreate(CFAllocatorRef allocator, CFURLRef bundleURL) {
    NSString *path = [((NSURL *)bundleURL) path];
    NSBundle *bundle = [[NSBundle bundleWithPath:path] retain];
    return NSBundleToCFBundle(bundle); //[NSBundle bundleWithPath:[(NSURL *)bundleURL absoluteString]]);
}

CFDictionaryRef CFBundleGetInfoDictionary(CFBundleRef self) {
    NSDictionary *info = [(NSBundle*)self infoDictionary];
    return (CFDictionaryRef)info;
}

CFTypeRef CFBundleGetValueForInfoDictionaryKey(CFBundleRef self,CFStringRef key) {
   return [CFBundleToNSBundle(self) objectForInfoDictionaryKey:(NSString *)key];
}

CFURLRef CFBundleCopyBundleURL(CFBundleRef self) {
    return [[NSURL fileURLWithPath:[CFBundleToNSBundle(self) bundlePath]] retain];
}


CFURLRef CFBundleCopyResourcesDirectoryURL(CFBundleRef self) {
    return [[NSURL fileURLWithPath:[CFBundleToNSBundle(self) resourcePath]] retain];
}

