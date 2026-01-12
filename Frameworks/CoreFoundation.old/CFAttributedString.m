#import <CoreFoundation/CFAttributedString.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSCFTypeID.h>

CFTypeID CFAttributedStringGetTypeID(void){
   return kNSCFTypeAttributedString;
}

CFAttributedStringRef CFAttributedStringCreate(CFAllocatorRef allocator,CFStringRef string,CFDictionaryRef attributes){
   NSUnimplementedFunction();
   return 0;
}
CFAttributedStringRef CFAttributedStringCreateWithSubstring(CFAllocatorRef allocator,CFAttributedStringRef self,CFRange range){
   NSUnimplementedFunction();
   return 0;
}

CFAttributedStringRef CFAttributedStringCreateCopy(CFAllocatorRef allocator,CFAttributedStringRef self){
   NSUnimplementedFunction();
   return 0;
}

CFIndex CFAttributedStringGetLength(CFAttributedStringRef self){
   NSUnimplementedFunction();
   return 0;
}
CFStringRef CFAttributedStringGetString(CFAttributedStringRef self){
   NSUnimplementedFunction();
   return 0;
}

CFTypeRef CFAttributedStringGetAttribute(CFAttributedStringRef self,CFIndex location,CFStringRef name,CFRange *effectiveRange){
   NSUnimplementedFunction();
   return 0;
}
CFTypeRef CFAttributedStringGetAttributeAndLongestEffectiveRange(CFAttributedStringRef self,CFIndex location,CFStringRef name,CFRange range,CFRange *longestEffectiveRange){
   NSUnimplementedFunction();
   return 0;
}
CFDictionaryRef CFAttributedStringGetAttributes(CFAttributedStringRef self,CFIndex location,CFRange *effectiveRange){
   NSUnimplementedFunction();
   return 0;
}
CFDictionaryRef CFAttributedStringGetAttributesAndLongestEffectiveRange(CFAttributedStringRef self,CFIndex location,CFRange range,CFRange *longestEffectiveRange){
   NSUnimplementedFunction();
   return 0;
}
// mutable

CFMutableAttributedStringRef CFAttributedStringCreateMutable(CFAllocatorRef allocator,CFIndex maxLength){
   NSUnimplementedFunction();
   return 0;
}

CFMutableAttributedStringRef CFAttributedStringCreateMutableCopy(CFAllocatorRef allocator,CFIndex maxLength,CFAttributedStringRef self){
   NSUnimplementedFunction();
   return 0;
}

CFMutableStringRef CFAttributedStringGetMutableString(CFMutableAttributedStringRef self){
   NSUnimplementedFunction();
   return 0;
}

void CFAttributedStringRemoveAttribute(CFMutableAttributedStringRef self,CFRange range,CFStringRef name){
   NSUnimplementedFunction();
}

void CFAttributedStringReplaceAttributedString(CFMutableAttributedStringRef self,CFRange range,CFAttributedStringRef replacement){
   NSUnimplementedFunction();
}

void CFAttributedStringReplaceString(CFMutableAttributedStringRef self,CFRange range,CFStringRef replacement){
   NSUnimplementedFunction();
}

void CFAttributedStringSetAttribute(CFMutableAttributedStringRef self,CFRange range,CFStringRef name,CFTypeRef value){
   NSUnimplementedFunction();
}

void CFAttributedStringSetAttributes(CFMutableAttributedStringRef self,CFRange range,CFDictionaryRef replacement,Boolean clearPreviousAttributes){
   NSUnimplementedFunction();
}

void CFAttributedStringBeginEditing(CFMutableAttributedStringRef self){
   NSUnimplementedFunction();
}
void CFAttributedStringEndEditing(CFMutableAttributedStringRef self){
   NSUnimplementedFunction();
}
