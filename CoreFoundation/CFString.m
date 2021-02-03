#import <CoreFoundation/CFString.h>
#import <CoreFoundation/CFStringEncodingExt.h>
#import <Foundation/NSString.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSPlatform.h>

struct __CFString {
};

#define ToNSString(object) ((NSString *)object)
#define ToCFString(object) ((CFStringRef)object)

CFStringEncoding CFStringConvertWindowsCodepageToEncoding(CFUInteger codepage)
{
    CFStringEncoding encoding = kCFStringEncodingInvalidId;
    switch (codepage) {
        case 874:
            encoding = kCFStringEncodingDOSThai;
            break;
        case 932:
            encoding = kCFStringEncodingDOSJapanese;
            break;
        case 936:
            encoding = kCFStringEncodingDOSChineseSimplif;
            break;
        case 949:
            encoding = kCFStringEncodingDOSKorean;
            break;
        case 950:
            encoding = kCFStringEncodingDOSChineseTrad;
            break;
        case 1250:
            encoding = kCFStringEncodingWindowsLatin2;
            break;
        case 1251:
            encoding = kCFStringEncodingWindowsCyrillic;
            break;
        case 1252:
            encoding = kCFStringEncodingWindowsLatin1;
            break;
        case 1253:
            encoding = kCFStringEncodingWindowsGreek;
            break;
        case 1254:
            encoding = kCFStringEncodingWindowsLatin5;
            break;
        case 1255:
            encoding = kCFStringEncodingWindowsHebrew;
            break;
        case 1256:
            encoding = kCFStringEncodingWindowsArabic;
            break;
        case 1257:
            encoding = kCFStringEncodingWindowsBalticRim;
            break;
        case 1258:
            encoding = kCFStringEncodingWindowsVietnamese;
            break;
        case 20127:
            encoding = kCFStringEncodingASCII;
            break;
        case 28591:
            encoding = kCFStringEncodingISOLatin1;
            break;
        case 28592:
            encoding = kCFStringEncodingISOLatin2;
            break;
        case 28593:
            encoding = kCFStringEncodingISOLatin3;
            break;
        case 28594:
            encoding = kCFStringEncodingISOLatin4;
            break;
        case 28595:
            encoding = kCFStringEncodingISOLatinCyrillic;
            break;
        case 28596:
            encoding = kCFStringEncodingISOLatinArabic;
            break;
        case 28597:
            encoding = kCFStringEncodingISOLatinGreek;
            break;
        case 28598:
            encoding = kCFStringEncodingISOLatinHebrew;
            break;
        case 28599:
            encoding = kCFStringEncodingISOLatin5;
            break;
        case 28600:
            encoding = kCFStringEncodingISOLatin6;
            break;
        case 28601:
            encoding = kCFStringEncodingISOLatinThai;
            break;
        case 28603:
            encoding = kCFStringEncodingISOLatin7;
            break;
        case 28604:
            encoding = kCFStringEncodingISOLatin8;
            break;
        case 28605:
            encoding = kCFStringEncodingISOLatin9;
            break;
        case 28606:
            encoding = kCFStringEncodingISOLatin10;
            break;
        default:
            encoding = kCFStringEncodingInvalidId;
            break;
    }
    return encoding;
}

 CFUInteger CFStringConvertEncodingToNSStringEncoding(CFStringEncoding encoding){
     switch(encoding){
         case kCFStringEncodingUTF8:
             return NSUTF8StringEncoding;
         case kCFStringEncodingUTF16:
             return NSUnicodeStringEncoding;
         case kCFStringEncodingUTF16BE:
             return NSUTF16BigEndianStringEncoding;
         case kCFStringEncodingUTF16LE:
             return NSUTF16LittleEndianStringEncoding;
         case kCFStringEncodingUTF32:
             return NSUTF32StringEncoding;
         case kCFStringEncodingUTF32BE:
             return NSUTF32BigEndianStringEncoding;
         case kCFStringEncodingUTF32LE:
             return NSUTF32LittleEndianStringEncoding;
             
         case kCFStringEncodingMacRoman:
             return NSMacOSRomanStringEncoding;
         case kCFStringEncodingWindowsLatin1:
             return NSWindowsCP1252StringEncoding;
         case kCFStringEncodingWindowsLatin2:
             return NSWindowsCP1250StringEncoding;
         case kCFStringEncodingISOLatin1:
             return NSISOLatin1StringEncoding;
         case kCFStringEncodingNextStepLatin:
             return NSNEXTSTEPStringEncoding;
         case kCFStringEncodingASCII:
             return NSASCIIStringEncoding;
             //    case kCFStringEncodingUnicode: same as kCFStringEncodingUTF16
         case kCFStringEncodingNonLossyASCII:
             return NSNonLossyASCIIStringEncoding;
     }
   // Cocoa is adding this bit for CF encoding that doesn't have an equivalent NS constant defined
   return encoding | 0x8000000;
}

CFStringEncoding CFStringConvertNSStringEncodingToEncoding(CFUInteger encoding){
    switch(encoding){
        case NSUTF8StringEncoding:
            return kCFStringEncodingUTF8;
        case NSUnicodeStringEncoding:
            return kCFStringEncodingUTF16;
        case NSUTF16BigEndianStringEncoding:
            return kCFStringEncodingUTF16BE;
        case NSUTF16LittleEndianStringEncoding:
            return kCFStringEncodingUTF16LE;
        case NSUTF32StringEncoding:
            return kCFStringEncodingUTF32;
        case NSUTF32BigEndianStringEncoding:
            return kCFStringEncodingUTF32BE;
        case NSUTF32LittleEndianStringEncoding:
            return kCFStringEncodingUTF32LE;
            
        case NSMacOSRomanStringEncoding:
            return kCFStringEncodingMacRoman;
        case NSWindowsCP1252StringEncoding:
            return kCFStringEncodingWindowsLatin1;
        case NSWindowsCP1250StringEncoding:
            return kCFStringEncodingWindowsLatin2;
        case NSISOLatin1StringEncoding:
            return kCFStringEncodingISOLatin1;
        case NSNEXTSTEPStringEncoding:
            return kCFStringEncodingNextStepLatin;
        case NSASCIIStringEncoding:
            return kCFStringEncodingASCII;
            //    case kCFStringEncodingUnicode: same as kCFStringEncodingUTF16
        case NSNonLossyASCIIStringEncoding:
            return kCFStringEncodingNonLossyASCII;
    }
    return encoding & ~0x8000000;
}

void CFStringAppendCharacters(CFMutableStringRef mutableString, const UniChar *chars, CFIndex numChars)
{
	[(NSMutableString *)mutableString appendString:[NSString stringWithCharacters:chars length:numChars]];
}

CFStringRef CFStringMakeConstant(const char *cString) {
// FIXME: constify
   return (CFStringRef)[[[NSString allocWithZone:NULL]initWithUTF8String:cString] autorelease];
}

CFStringRef CFStringCreateByCombiningStrings(CFAllocatorRef allocator,CFArrayRef array,CFStringRef separator){
   NSUnimplementedFunction();
   return 0;
}

CFStringRef CFStringCreateCopy(CFAllocatorRef allocator,CFStringRef self){
   return ToCFString([ToNSString(self) copyWithZone:NULL]);
}

COREFOUNDATION_EXPORT CFStringRef CFStringCreateMutableCopy(CFAllocatorRef allocator,CFIndex maxLength,CFStringRef self){
	return ToCFString([ToNSString(self) mutableCopyWithZone:NULL]);
}

CFStringRef CFStringCreateWithBytes(CFAllocatorRef allocator,const uint8_t *bytes,CFIndex length,CFStringEncoding encoding,Boolean isExternalRepresentation){
    return ToCFString([[NSString allocWithZone:NULL] initWithBytes:bytes length:length encoding:CFStringConvertEncodingToNSStringEncoding(encoding)]);

}
CFStringRef CFStringCreateWithBytesNoCopy(CFAllocatorRef allocator,const uint8_t *bytes,CFIndex length,CFStringEncoding encoding,Boolean isExternalRepresentation,CFAllocatorRef contentsDeallocator){
   NSUnimplementedFunction();
   return 0;
}
CFStringRef CFStringCreateWithCharacters(CFAllocatorRef allocator,const UniChar *chars,CFIndex length){
    return ToCFString([[NSString alloc] initWithCharacters:chars length:length]);
}
CFStringRef CFStringCreateWithCharactersNoCopy(CFAllocatorRef allocator,const UniChar *chars,CFIndex length,CFAllocatorRef contentsDeallocator){
   NSUnimplementedFunction();
   return 0;
}
CFStringRef CFStringCreateWithCString(CFAllocatorRef allocator,const char *cString,CFStringEncoding encoding){
    NSInteger length = strlen(cString);
    return CFStringCreateWithBytes(allocator, (const uint8_t *)cString, length, encoding, NO);
}

CFStringRef CFStringCreateWithCStringNoCopy(CFAllocatorRef allocator,const char *cString,CFStringEncoding encoding,CFAllocatorRef contentsDeallocator){
   NSUnimplementedFunction();
   return 0;
}
CFStringRef CFStringCreateWithFileSystemRepresentation(CFAllocatorRef allocator,const char *buffer){
   NSUnimplementedFunction();
   return 0;
}
CFStringRef CFStringCreateWithFormat(CFAllocatorRef allocator,CFDictionaryRef formatOptions,CFStringRef format,...){
   NSUnimplementedFunction();
   return 0;
}
CFStringRef CFStringCreateWithFormatAndArguments(CFAllocatorRef allocator,CFDictionaryRef formatOptions,CFStringRef format,va_list arguments){
   NSUnimplementedFunction();
   return 0;
}
CFStringRef CFStringCreateFromExternalRepresentation(CFAllocatorRef allocator,CFDataRef data,CFStringEncoding encoding){
   NSUnimplementedFunction();
   return 0;
}

CFStringRef CFStringCreateWithSubstring(CFAllocatorRef allocator,CFStringRef self,CFRange range) {
   NSUnimplementedFunction();
   return 0;
}


void CFShow(CFTypeRef self) {
   NSPlatformLogString([ToNSString(self) description]);
}

void CFShowStr(CFStringRef self) {
   NSUnimplementedFunction();
}


CFComparisonResult CFStringCompare(CFStringRef self,CFStringRef other,CFOptionFlags options){
   return [ToNSString(self) compare:(NSString *)other options:options];
}

CFComparisonResult CFStringCompareWithOptions(CFStringRef self,CFStringRef other,CFRange range,CFOptionFlags options) {
   NSRange nsRange={range.location,range.length};
   return [ToNSString(self) compare:(NSString *)other options:options range:nsRange];
}

CFComparisonResult CFStringCompareWithOptionsAndLocale(CFStringRef self,CFStringRef other,CFRange range,CFOptionFlags options,CFLocaleRef locale) {
   NSRange nsRange={range.location,range.length};
   return [ToNSString(self) compare:(NSString *)other options:options range:nsRange locale:(id)locale];
}


void CFStringDelete(CFMutableStringRef self,CFRange range)
{
	NSRange inrange = NSMakeRange(range.location,range.length);
	[(NSMutableString *)self deleteCharactersInRange:inrange];
}

CFIndex CFStringGetLength(CFStringRef self) {
   return [ToNSString(self) length];
}

UniChar CFStringGetCharacterAtIndex(CFStringRef self,CFIndex index) {
   return [ToNSString(self) characterAtIndex:index];
}

void CFStringGetCharacters(CFStringRef self,CFRange range,UniChar *buffer) {
   NSRange nsRange={range.location,range.length};
   [ToNSString(self) getCharacters:buffer range:nsRange];
}

Boolean CFStringGetCString(CFStringRef self,char *buffer,CFIndex bufferSize,CFStringEncoding encoding) {
   return [ToNSString(self) getCString:buffer maxLength:bufferSize encoding:CFStringConvertEncodingToNSStringEncoding(encoding)];
}

const char *CFStringGetCStringPtr(CFStringRef self,CFStringEncoding encoding) {
   return [ToNSString(self) cStringUsingEncoding:CFStringConvertEncodingToNSStringEncoding(encoding)];
}

Boolean CFStringFindCharacterFromSet(CFStringRef self,CFCharacterSetRef set,CFRange range,CFOptionFlags options,CFRange *result){
	NSRange inrange = NSMakeRange(range.location,range.length);
	NSRange outrange = [ToNSString(self) rangeOfCharacterFromSet:(NSCharacterSet*)set options:(NSStringCompareOptions)options range:inrange];
	if (result)
		*result = CFRangeMake(outrange.location, outrange.length);
	return outrange.location != NSNotFound;
}

void CFStringInsert(CFMutableStringRef self, CFIndex idx, CFStringRef insertedStr)
{
	[(NSMutableString *)self insertString:ToNSString(insertedStr) atIndex:(NSUInteger)idx];
}

