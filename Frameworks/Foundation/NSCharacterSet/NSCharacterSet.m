/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSCharacterSet.h>
#import <Foundation/NSData.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSMapTable.h>
#import <Foundation/NSCharacterSet_range.h>
#import <Foundation/NSCharacterSet_bitmap.h>
#import <Foundation/NSMutableCharacterSet_bitmap.h>
#import <Foundation/NSCharacterSet_string.h>
#import <Foundation/bitmapRepresentation.h>
#import <Foundation/NSAutoreleasePool-private.h>
#import <Foundation/NSRaise.h>

@implementation NSCharacterSet

static NSMapTable *nameToSet=NULL;

+(void)initialize {
   if(self==[NSCharacterSet class]){
    nameToSet=NSCreateMapTable(NSObjectMapKeyCallBacks,NSObjectMapValueCallBacks,0);
   }
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

-mutableCopyWithZone:(NSZone *)zone {
   return [[NSMutableCharacterSet_bitmap allocWithZone:NULL] initWithCharacterSet:self];
}

-(Class)classForCoder {
   NSUnimplementedMethod();
   return Nil;
}

-initWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
   return self;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

+characterSetWithBitmapRepresentation:(NSData *)data {
   return NSAutorelease(NSCharacterSet_bitmapNewWithBitmap(NULL,data));
}

+characterSetWithCharactersInString:(NSString *)string {
   return NSAutorelease([[NSCharacterSet_string allocWithZone:NULL] initWithString:string inverted:NO]);
}

+characterSetWithContentsOfFile:(NSString *)path {
   NSData *data=[NSData dataWithContentsOfFile:path];

   if(data==nil)
    return nil;

   return [self characterSetWithBitmapRepresentation:data];
}

+characterSetWithRange:(NSRange)range {
   return NSAutorelease([[NSCharacterSet_range allocWithZone:NULL] initWithRange:range]);
}

static NSString *pathForCharacterSet(NSString *name){
   NSBundle *bundle=[NSBundle bundleForClass:[NSCharacterSet class]];
   NSString *path=[bundle pathForResource:name ofType:@"bitmap"];

   if(path==nil)
    [NSException raise:@"NSCharacterSetFailedException" format:@"NSCharacterSet unable to find bitmap for %@",name];

   return path;
}

static NSCharacterSet *sharedSetWithName(Class cls,NSString *name){
   NSCharacterSet *result;

   if(cls!=[NSCharacterSet class])
    result=[cls characterSetWithContentsOfFile:pathForCharacterSet(name)];
   else {
    if((result=NSMapGet(nameToSet,name))==nil){
     if((result=[NSCharacterSet characterSetWithContentsOfFile:pathForCharacterSet(name)])!=nil)
      NSMapInsert(nameToSet,name,result);
    }
   }
   
   return result;
}

+alphanumericCharacterSet {
   return sharedSetWithName(self,@"alphanumericCharacterSet");
}

+controlCharacterSet {
   return sharedSetWithName(self,@"controlCharacterSet");
}

+decimalDigitCharacterSet {
   return sharedSetWithName(self,@"decimalDigitCharacterSet");
}

+decomposableCharacterSet {
   return sharedSetWithName(self,@"decomposableCharacterSet");
}

+illegalCharacterSet {
   return sharedSetWithName(self,@"illegalCharacterSet");
}

+letterCharacterSet {
   return sharedSetWithName(self,@"letterCharacterSet");
}

+lowercaseLetterCharacterSet {
   return sharedSetWithName(self,@"lowercaseLetterCharacterSet");
}

+nonBaseCharacterSet {
   return sharedSetWithName(self,@"nonBaseCharacterSet");
}

+punctuationCharacterSet {
   return sharedSetWithName(self,@"punctuationCharacterSet");
}

+uppercaseLetterCharacterSet {
   return sharedSetWithName(self,@"uppercaseLetterCharacterSet");
}

+newlineCharacterSet {
    static NSString *setName = @"newlineCharacterSet";
    id set;
    if ( !(set = NSMapGet(nameToSet,setName)) || self != [NSCharacterSet class]) {
        unichar chars[] = { 0x0A, 0x0B, 0x0C, 0x0D,  0x85, 0x2028, 0x2029 };
        set = [self characterSetWithCharactersInString:[NSString stringWithCharacters:chars length:
                                                                  sizeof(chars)/sizeof(unichar)]];
        if (self == [NSCharacterSet class]) NSMapInsert(nameToSet,setName,set);
    }
    return set;   
}

+whitespaceAndNewlineCharacterSet {
    static NSString *setName = @"whitespaceAndNewlineCharacterSet";
    id set;
    if ( !(set = NSMapGet(nameToSet,setName)) || self != [NSCharacterSet class]) {
    // Doc.s do not mention 0xA0 but it is implemented as a member
        unichar chars[] = { 0x20, 0x09,  0x0A, 0x0B, 0x0C, 0x0D,  0x85, 0xA0, 0x2028, 0x2029 };
        set = [self characterSetWithCharactersInString:[NSString stringWithCharacters:chars length:
                                                                  sizeof(chars)/sizeof(unichar)]];
        if (self == [NSCharacterSet class]) NSMapInsert(nameToSet,setName,set);
    }
    return set;
}

+whitespaceCharacterSet {
    static NSString *setName = @"whitespaceCharacterSet";
    id set;
    if ( !(set = NSMapGet(nameToSet,setName)) || self != [NSCharacterSet class]) {
    // Doc.s do not mention 0xA0 but it is implemented as a member
        unichar chars[3] = { 0x20, 0x09, 0xA0 };
        set = [self characterSetWithCharactersInString:[NSString stringWithCharacters:chars length:3]];
        if (self == [NSCharacterSet class]) NSMapInsert(nameToSet,setName,set);
    }
    return set;
}

-(BOOL)characterIsMember:(unichar)character {
   NSInvalidAbstractInvocation();
   return NO;
}

-(NSCharacterSet *)invertedSet {
   uint8_t *bitmap=bitmapBytes(self);
   NSUInteger       i;

   for(i=0;i<NSBitmapCharacterSetSize;i++)
    bitmap[i]=~bitmap[i];

   return NSAutorelease(NSCharacterSet_bitmapNewWithBitmap(NULL,
     [NSData dataWithBytesNoCopy:bitmap length:NSBitmapCharacterSetSize]));
}

-(NSData *)bitmapRepresentation {
   return [NSData dataWithBytesNoCopy:bitmapBytes(self)
                               length:NSBitmapCharacterSetSize];
}

// yea this is terrible
-(BOOL)isSupersetOfSet:(NSCharacterSet *)other {
   NSUInteger i;
   
   for(i=0;i<=0xFFFF;i++){
    if([other characterIsMember:i] && ![self characterIsMember:i])
     return NO;
   }
   
   return YES;
}

@end
