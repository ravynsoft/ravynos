#ifndef CF_BASE_SHIM_H
#define CF_BASE_SHIM_H

#import <Foundation/NSObjCRuntime.h>

#import <CoreFoundation/CFBase.h>

FOUNDATION_EXPORT CFTypeRef CFRetainShim(CFTypeRef self);
FOUNDATION_EXPORT void CFReleaseShim(CFTypeRef self);

FOUNDATION_EXPORT CFHashCode CFHashShim(CFTypeRef self);
FOUNDATION_EXPORT Boolean CFEqualShim(CFTypeRef self, CFTypeRef other);
FOUNDATION_EXPORT CFStringRef CFCopyDescriptionShim(CFTypeRef self);

#endif
