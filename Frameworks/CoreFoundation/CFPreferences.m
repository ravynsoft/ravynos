#import <CoreFoundation/CFPreferences.h>
#import <Foundation/NSRaise.h>

const CFStringRef kCFPreferencesCurrentApplication=(CFStringRef)@"kCFPreferencesCurrentApplication";
const CFStringRef kCFPreferencesCurrentHost=(CFStringRef)@"kCFPreferencesCurrentHost";
const CFStringRef kCFPreferencesCurrentUser=(CFStringRef)@"kCFPreferencesCurrentUser";

const CFStringRef kCFPreferencesAnyApplication=(CFStringRef)@"kCFPreferencesAnyApplication";
const CFStringRef kCFPreferencesAnyHost=(CFStringRef)@"kCFPreferencesAnyHost";
const CFStringRef kCFPreferencesAnyUser=(CFStringRef)@"kCFPreferencesAnyUser";

void CFPreferencesAddSuitePreferencesToApp(CFStringRef application,CFStringRef suite) {
   NSUnimplementedFunction();
}

Boolean CFPreferencesAppSynchronize(CFStringRef application) {
   NSUnimplementedFunction();
   return NO;
}

Boolean CFPreferencesAppValueIsForced(CFStringRef key,CFStringRef application) {
   NSUnimplementedFunction();
   return NO;
}

CFArrayRef CFPreferencesCopyApplicationList(CFStringRef user,CFStringRef host) {
   NSUnimplementedFunction();
   return NULL;
}

CFPropertyListRef CFPreferencesCopyAppValue(CFStringRef key,CFStringRef application) {
   NSUnimplementedFunction();
   return nil;
}

Boolean CFPreferencesGetAppBooleanValue(CFStringRef key,CFStringRef application,Boolean *validKey) {
   NSUnimplementedFunction();
   return NO;
}

CFIndex CFPreferencesGetAppIntegerValue(CFStringRef key,CFStringRef application,Boolean *validKey) {
   NSUnimplementedFunction();
   return 0;
}

CFArrayRef CFPreferencesCopyKeyList(CFStringRef application,CFStringRef user,CFStringRef host) {
   NSUnimplementedFunction();
   return NULL;
}

CFDictionaryRef CFPreferencesCopyMultiple(CFArrayRef keysToFetch,CFStringRef application,CFStringRef user,CFStringRef host) {
   NSUnimplementedFunction();
   return NULL;
}

CFPropertyListRef CFPreferencesCopyValue(CFStringRef key,CFStringRef application,CFStringRef user,CFStringRef host) {
   NSUnimplementedFunction();
   return nil;
}

void CFPreferencesSetAppValue(CFStringRef key,CFPropertyListRef value,CFStringRef application) {
   NSUnimplementedFunction();
}

void CFPreferencesSetMultiple(CFDictionaryRef dictionary,CFArrayRef removeTheseKeys,CFStringRef application,CFStringRef user,CFStringRef host) {
   NSUnimplementedFunction();
}

void CFPreferencesSetValue(CFStringRef key,CFPropertyListRef value,CFStringRef application,CFStringRef user,CFStringRef host) {
   NSUnimplementedFunction();
}

void CFPreferencesRemoveSuitePreferencesFromApp(CFStringRef application,CFStringRef suite) {
   NSUnimplementedFunction();
}

Boolean CFPreferencesSynchronize(CFStringRef application,CFStringRef user,CFStringRef host) {
   NSUnimplementedFunction();
   return NO;
}


