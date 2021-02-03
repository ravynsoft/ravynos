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

CFTypeRef CFBundleGetValueForInfoDictionaryKey(CFBundleRef self,CFStringRef key) {
   return [CFBundleToNSBundle(self) objectForInfoDictionaryKey:(NSString *)key];
}

