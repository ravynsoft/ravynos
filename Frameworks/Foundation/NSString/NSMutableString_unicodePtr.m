/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSMutableString_unicodePtr.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSData.h>
#import <Foundation/NSStringHashing.h>
#import <Foundation/NSStringFormatter.h>
#import <Foundation/NSStringFileIO.h>
#import <Foundation/NSString_cString.h>
#import <Foundation/NSString_nextstep.h>
#import <Foundation/NSString_isoLatin1.h>
#import <Foundation/NSString_win1252.h>
#import <Foundation/NSStringSymbol.h>
#import <Foundation/NSStringUTF8.h>
#import <Foundation/NSUnicodeCaseMapping.h>
#import <Foundation/NSRaiseException.h>
#include <string.h>

@implementation NSMutableString_unicodePtr

-(NSUInteger)length {
   return _length;
}

-(NSUInteger)lengthOfBytesUsingEncoding:(NSStringEncoding)encoding {
    switch (encoding) {
        case NSUTF8StringEncoding:
            return NSConvertUTF16toUTF8(_unicode, _length,NULL);
        case NSUnicodeStringEncoding:
            return _length;
            
        default:
            NSUnimplementedMethod();
            NSLog(@"For encoding: %i", encoding);
            return 0;
    }
}

-(unichar)characterAtIndex:(NSUInteger)location {
   if(location>=_length){
    NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond length %d",
     location,[self length]);
   }

   return _unicode[location];
}

-(void)getCharacters:(unichar *)buffer {
    memcpy(buffer, _unicode, _length*sizeof(unichar));
}

-(void)getCharacters:(unichar *)buffer range:(NSRange)range {
   NSInteger i,loc=range.location;

   if(NSMaxRange(range)>_length){
    NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",
     NSStringFromRange(range),[self length]);
   }

    memcpy(buffer, _unicode+loc, range.length*sizeof(unichar));

}

-(void)replaceCharactersInRange:(NSRange)range withString:(NSString *)string {
   NSUInteger otherlength=[string length];
   NSUInteger i,loc=range.location;

   if(NSMaxRange(range)>_length){
    NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",
     NSStringFromRange(range),[self length]);
   }

   if(range.length<otherlength){ // make room
    NSUInteger delta=otherlength-range.length;

    _length+=delta;

    if(_length>_capacity){
     if(_capacity==0)
      _capacity=1;

     while(_length>_capacity)
      _capacity*=2;

     _unicode=NSZoneRealloc(NSZoneFromPointer(_unicode),_unicode,sizeof(unichar)*_capacity);
    }

    for(i=_length;--i>=loc+otherlength;)
     _unicode[i]=_unicode[i-delta];
   }
   else if(range.length>otherlength){ // delete some
    NSUInteger delta=range.length-otherlength;

    _length-=delta;

    for(i=loc+otherlength;i<_length;i++)
     _unicode[i]=_unicode[i+delta];
   }

   [string getCharacters:_unicode+loc range:NSMakeRange(0,otherlength)];
}

-(NSUInteger)hash {
   return NSStringHashUnicode(_unicode,MIN(_length,NSHashStringLength));
}

static inline NSUInteger roundCapacityUp(NSUInteger capacity){
   return (capacity<4)?4:capacity;
}


NSMutableString_unicodePtr *NSMutableString_unicodePtrInitWithCString(NSMutableString_unicodePtr *self, const char *cString, NSUInteger length, NSZone *zone)
{
    self->_unicode = NSCharactersFromCString(cString, length, &(self->_length), zone);
    self->_capacity = self->_length;

    return self;
}


NSMutableString_unicodePtr *NSMutableString_unicodePtrInit(NSMutableString_unicodePtr *self, const unichar *unicode, NSUInteger length, NSZone *zone)
{
    NSInteger i;

    self->_length = length;
    self->_capacity = roundCapacityUp(length);
    self->_unicode = NSZoneMalloc(zone, sizeof(unichar) * self->_capacity);
    for (i = 0; i < length; i++) {
        self->_unicode[i] = unicode[i];
    }

    return self;
}


NSMutableString_unicodePtr *NSMutableString_unicodePtrInitNoCopy(NSMutableString_unicodePtr *self, unichar *unicode, NSUInteger length, NSZone *zone)
{
    self->_length = length;
    self->_capacity = length;
    self->_unicode = unicode;

    return self;
}


NSMutableString_unicodePtr *NSMutableString_unicodePtrInitWithCapacity(NSMutableString_unicodePtr *self, NSUInteger capacity, NSZone *zone)
{
    self->_length = 0;
    self->_capacity = roundCapacityUp(capacity);
    self->_unicode = NSZoneMalloc(zone, sizeof(unichar) * self->_capacity);

    return self;
}


NSString *NSMutableString_unicodePtrNewWithCString(NSZone *zone,
 const char *cString,NSUInteger length) {
   NSMutableString_unicodePtr *self=NSAllocateObject(objc_lookUpClass("NSMutableString_unicodePtr"),0,zone);
    if (self) {
        self = NSMutableString_unicodePtrInitWithCString(self,cString,length,zone);
    }
    return self;
}

NSString *NSMutableString_unicodePtrNew(NSZone *zone,
 const unichar *unicode,NSUInteger length) {
   NSMutableString_unicodePtr *self=NSAllocateObject(objc_lookUpClass("NSMutableString_unicodePtr"),0,zone);
    if (self) {
        self = NSMutableString_unicodePtrInit(self,unicode,length,zone);
    }
    return self;
}

NSString *NSMutableString_unicodePtrNewNoCopy(NSZone *zone,
 unichar *unicode,NSUInteger length) {
   NSMutableString_unicodePtr *self;

   self=NSAllocateObject(objc_lookUpClass("NSMutableString_unicodePtr"),0,zone);
    if (self) {
        self = NSMutableString_unicodePtrInitNoCopy(self,unicode,length,zone);
    }
    return self;
}

NSString *NSMutableString_unicodePtrNewWithCapacity(NSZone *zone,
 NSUInteger capacity) {
   NSMutableString_unicodePtr *self;

   self=NSAllocateObject(objc_lookUpClass("NSMutableString_unicodePtr"),0,zone);
    if (self) {
       self = NSMutableString_unicodePtrInitWithCapacity(self,capacity,zone);
    }
    return self;
}

-(void)dealloc {
   NSZoneFree(NSZoneFromPointer(self->_unicode),self->_unicode);
   NSDeallocateObject(self);
   return;
   [super dealloc];
}

- init
{
    return NSMutableString_unicodePtrInitWithCapacity(self, 0, NSZoneFromPointer(self));
}


- initWithCharactersNoCopy:(unichar *)characters length:(NSUInteger)length freeWhenDone:(BOOL)freeWhenDone
{
    NSMutableString_unicodePtr *string = NSMutableString_unicodePtrInit(self, characters, length, NSZoneFromPointer(self));

    if (freeWhenDone) {
        NSZoneFree(NSZoneFromPointer(characters), characters);
    }

    return string;
}


- initWithCharacters:(const unichar *)characters length:(NSUInteger)length
{
    return NSMutableString_unicodePtrInit(self, characters, length, NSZoneFromPointer(self));
}


- initWithCStringNoCopy:(char *)bytes length:(NSUInteger)length freeWhenDone:(BOOL)freeWhenDone
{
    NSMutableString_unicodePtr *string = NSMutableString_unicodePtrInitWithCString(self, bytes, length, NSZoneFromPointer(self));

    if (freeWhenDone) {
        NSZoneFree(NSZoneFromPointer(bytes), bytes);
    }

    return string;
}


- initWithCString:(const char *)bytes length:(NSUInteger)length
{
    return NSMutableString_unicodePtrInitWithCString(self, bytes, length, NSZoneFromPointer(self));
}


- initWithCString:(const char *)bytes
{
    NSUInteger length = strlen(bytes);

    return NSMutableString_unicodePtrInitWithCString(self, bytes, length, NSZoneFromPointer(self));
}


- initWithString:(NSString *)string
{
    NSUInteger length = [string length];
    unichar *unicode = NSZoneMalloc(NULL, sizeof(unichar)*length);
    if (unicode) {
        [string getCharacters:unicode];
        
        self = NSMutableString_unicodePtrInit(self, unicode, length, NSZoneFromPointer(self));
        free(unicode);
    } else {
        [self release];
        self = nil;
    }
    return self;
}


- initWithFormat:(NSString *)format, ...
{
    va_list arguments;
    NSUInteger length;
    unichar *unicode;

    va_start(arguments, format);

    unicode = NSCharactersNewWithFormat(format, nil, arguments, &length, NSZoneFromPointer(self));
    va_end(arguments);

    return NSMutableString_unicodePtrInitNoCopy(self, unicode, length, NSZoneFromPointer(self));
}


- initWithFormat:(NSString *)format arguments:(va_list)arguments
{
    NSUInteger length;
    unichar *unicode;

    unicode = NSCharactersNewWithFormat(format, nil, arguments, &length, NSZoneFromPointer(self));

    return NSMutableString_unicodePtrInitNoCopy(self, unicode, length, NSZoneFromPointer(self));
}


- initWithFormat:(NSString *)format locale:(NSDictionary *)locale, ...
{
    va_list arguments;
    NSUInteger length;
    unichar *unicode;

    va_start(arguments, locale);

    unicode = NSCharactersNewWithFormat(format, locale, arguments, &length, NSZoneFromPointer(self));
    va_end(arguments);

    return NSMutableString_unicodePtrInitNoCopy(self, unicode, length, NSZoneFromPointer(self));
}


- initWithBytes:(const void *)bytes length:(NSUInteger)length encoding:(NSStringEncoding)encoding
{
    NSUInteger resultLength;
    unichar *characters;

    characters = NSString_anyCStringToUnicode(encoding, bytes, length, &resultLength, NSZoneFromPointer(self));

    return NSMutableString_unicodePtrInitNoCopy(self, characters, resultLength, NSZoneFromPointer(self));
}


- initWithFormat:(NSString *)format locale:(NSDictionary *)locale arguments:(va_list)arguments
{
    NSUInteger length;
    unichar *unicode;

    unicode = NSCharactersNewWithFormat(format, locale, arguments, &length, NSZoneFromPointer(self));

    return NSMutableString_unicodePtrInitNoCopy(self, unicode, length, NSZoneFromPointer(self));
}


-initWithData:(NSData *)data encoding:(NSStringEncoding)encoding {
    return [self initWithBytes:[data bytes] length:[data length] encoding:encoding];
}


- initWithContentsOfFile:(NSString *)path
{
    NSUInteger length;
    unichar *unicode;

    if ((unicode = NSCharactersWithContentsOfFile(path, &length, NSZoneFromPointer(self))) == NULL) {
        NSDeallocateObject(self);
        return nil;
    }

    return NSMutableString_unicodePtrInitNoCopy(self, unicode, length, NSZoneFromPointer(self));
}


- initWithCapacity:(NSUInteger)capacity
{
    return NSMutableString_unicodePtrInitWithCapacity(self, capacity, NSZoneFromPointer(self));
}


@end
