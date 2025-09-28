#import <CoreFoundation/CFError.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSCFTypeID.h>

// FIXME: fix all string constants so they match NS* contents
const CFStringRef kCFErrorDomainPOSIX=(CFStringRef)@"FIXME";
const CFStringRef kCFErrorDomainCocoa=(CFStringRef)@"FIXME";
const CFStringRef kCFErrorDomainOSStatus=(CFStringRef)@"FIXME";
const CFStringRef kCFErrorDomainMach=(CFStringRef)@"FIXME";

const CFStringRef kCFErrorDescriptionKey=(CFStringRef)@"FIXME";
const CFStringRef kCFErrorLocalizedDescriptionKey=(CFStringRef)@"FIXME";
const CFStringRef kCFErrorUnderlyingErrorKey=(CFStringRef)@"FIXME";
const CFStringRef kCFErrorLocalizedFailureReasonKey=(CFStringRef)@"FIXME";
const CFStringRef kCFErrorLocalizedRecoverySuggestionKey=(CFStringRef)@"FIXME";

CFTypeID CFErrorGetTypeID(void) {
   return kNSCFTypeError;
}

CFErrorRef CFErrorCreate(CFAllocatorRef allocator,CFStringRef domain,CFIndex code,CFDictionaryRef userInfo){
   NSUnimplementedFunction();
   return 0;
}

CFErrorRef CFErrorCreateWithUserInfoKeysAndValues(CFAllocatorRef allocator,CFStringRef domain,CFIndex code,const void *const *userInfoKeys,const void *const *userInfoValues,CFIndex userInfoCount) {
   NSUnimplementedFunction();
   return 0;
}

CFStringRef CFErrorGetDomain(CFErrorRef self) {
   NSUnimplementedFunction();
   return 0;
}

CFIndex CFErrorGetCode(CFErrorRef self) {
   NSUnimplementedFunction();
   return 0;
}

CFDictionaryRef CFErrorCopyUserInfo(CFErrorRef self) {
   NSUnimplementedFunction();
   return 0;
}

CFStringRef CFErrorCopyFailureReason(CFErrorRef self) {
   NSUnimplementedFunction();
   return 0;
}

CFStringRef CFErrorCopyRecoverySuggestion(CFErrorRef self) {
   NSUnimplementedFunction();
   return 0;
}

CFStringRef CFErrorCopyDescription(CFErrorRef self) {
   NSUnimplementedFunction();
   return 0;
}

