#import <CoreFoundation/CFURL.h>
#import <CoreFoundation/CFString.h>
#import <Foundation/NSCharacterSet.h>
#import <Foundation/NSRaise.h>

#define ToNSString(object) ((NSString *)object)
#define ToCFString(object) ((CFStringRef)object)

static NSString *notLegalURLCharacters(){
   unichar codes[]={
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
    0x20,
    0x22,0x23,0x25,0x3C,0x3E,0x5B,0x5C,0x5D,
    0x5E,0x60,0x7B,0x7C,0x7D
   };
   NSUInteger length=32+1+8+5;
   
   return [NSString stringWithCharacters:codes length:length];
}

CFStringRef CFURLCreateStringByAddingPercentEscapes(CFAllocatorRef allocator,CFStringRef self,CFStringRef charactersToLeaveUnescaped,CFStringRef charactersToBeEscaped,CFStringEncoding encoding) {
   NSCharacterSet *dontEscapeSet=[NSCharacterSet characterSetWithCharactersInString:charactersToLeaveUnescaped?ToNSString(charactersToLeaveUnescaped):(NSString *)@""];
   NSCharacterSet *escapeSet=[NSCharacterSet characterSetWithCharactersInString:charactersToBeEscaped?ToNSString(charactersToBeEscaped):(NSString *)@""];
   NSCharacterSet *notLegalEscapeSet=[NSCharacterSet characterSetWithCharactersInString:notLegalURLCharacters()];
   NSUInteger i,length=[ToNSString(self) length],resultLength=0;
   unichar    unicode[length];
   unichar    result[length*3];
   const char *hex="0123456789ABCDEF";
      
   [ToNSString(self) getCharacters:unicode];
   
   for(i=0;i<length;i++){
    unichar code=unicode[i];

    if(([escapeSet characterIsMember:code] || [notLegalEscapeSet characterIsMember:code]) && ![dontEscapeSet characterIsMember:code]){
     result[resultLength++]='%';
     result[resultLength++]=hex[(code>>4)&0xF];
     result[resultLength++]=hex[code&0xF];
    }
    else {
     result[resultLength++]=code;
    }
   }
   
   if(length==resultLength)
    return CFRetain(self);
    
   return ToCFString([[NSString alloc] initWithCharacters:result length:resultLength]);
}

