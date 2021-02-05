/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
                 2009 Markus Hitter <mah@jump-ing.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSData.h>
#import <Foundation/NSString_cString.h>
#import <Foundation/NSData_concrete.h>
#import <Foundation/NSAutoreleasePool-private.h>
#import <Foundation/NSKeyedUnarchiver.h>
#import <Foundation/NSKeyedArchiver.h>

#import <Foundation/NSRaise.h>
#import <Foundation/NSPlatform.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSRaiseException.h>
#import <Foundation/NSURLConnection.h>
#import <Foundation/NSURLRequest.h>
#import <Foundation/NSURLError.h>
#import <Foundation/NSCFTypeID.h>
#import <Foundation/NSError.h>
#import <Foundation/NSDictionary.h>

@implementation NSData

+allocWithZone:(NSZone *)zone {
   if(self==objc_lookUpClass("NSData"))
    return NSAllocateObject([NSData_concrete class],0,zone);

   return NSAllocateObject(self,0,zone);
}

-initWithBytesNoCopy:(void *)bytes length:(NSUInteger)length freeWhenDone:(BOOL)freeWhenDone {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithBytesNoCopy:(void *)bytes length:(NSUInteger)length {
   return [self initWithBytesNoCopy:bytes length:length freeWhenDone:YES];
}

-initWithBytes:(const void *)bytes length:(NSUInteger)length {
   return [self initWithBytesNoCopy:NSBytesReplicate(bytes,length,NSZoneFromPointer(self)) length:length];
}

-initWithData:(NSData *)data {
   return [self initWithBytes:[data bytes] length:[data length]];
}

-initWithContentsOfFile:(NSString *)path {
   return [self initWithContentsOfFile:path options:0 error:NULL];
}

-initWithContentsOfMappedFile:(NSString *)path {
   return [self initWithContentsOfFile:path options:NSMappedRead error:NULL];
}

-initWithContentsOfURL:(NSURL *)url {
   return [self initWithContentsOfURL:url options:0 error:NULL];
}

-initWithContentsOfFile:(NSString *)path options:(NSUInteger)options error:(NSError **)errorp {
   NSUInteger length;
   void *bytes=NULL;

   if (options&NSUncachedRead)
    NSLog(@"-[%@ %s] option NSUncachedRead currently ignored.",[self class],sel_getName(_cmd));

   if (options&NSMappedRead)
    bytes=[[NSPlatform currentPlatform] mapContentsOfFile:path length:&length];
   else
    bytes=NSPlatformContentsOfFile(path,&length);

   if(bytes==NULL){

    if(errorp!=NULL){
     NSDictionary *userInfo=[NSDictionary dictionaryWithObject:[NSString stringWithFormat:@"Could not open file %@", path] forKey:NSLocalizedDescriptionKey];

     *errorp=[NSError errorWithDomain:NSURLErrorDomain code:NSURLErrorCannotOpenFile userInfo:userInfo];
    }

    [self dealloc];
    return nil;
   }

   return [self initWithBytesNoCopy:bytes length:length];
}

-initWithContentsOfURL:(NSURL *)url options:(NSUInteger)options error:(NSError **)errorp {

   if(![url isFileURL]){

	   if ( [[url scheme] isEqual:@"http"] || [[url scheme] isEqual:@"https"]) {
		   NSError *error=nil;
		   NSURLResponse *response=nil;

		   NSData *data = [NSURLConnection sendSynchronousRequest:[NSURLRequest requestWithURL:url] returningResponse:&response error:&error];
		   if (data == nil) {
			   if (errorp != NULL) {
				   *errorp = error;
			   }
			   [self dealloc];
			   return nil;
		   }
		   return [self initWithData:data];
	   } else {
		   [self dealloc];

           if(errorp!=NULL){
            NSDictionary *userInfo=[NSDictionary dictionaryWithObject:[NSString stringWithFormat:@"Could not open url %@", url] forKey:NSLocalizedDescriptionKey];

            *errorp=[NSError errorWithDomain:NSURLErrorDomain code:NSURLErrorBadURL userInfo:userInfo];
	   }
    return nil;
   }
   }

   return [self initWithContentsOfFile:[url path] options:options error:errorp];
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

-mutableCopyWithZone:(NSZone *)zone {
   return [[NSMutableData allocWithZone:zone] initWithData:self];
}

-(Class)classForCoder {
   return [NSData class];
}

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    NSData            *data=[keyed decodeObjectForKey:@"NS.data"];

    return [self initWithData:data];
   }
   else {
    [self dealloc];
    return [[coder decodeDataObject] retain];
   }
}

-(void)encodeWithCoder:(NSCoder *)coder {
   if([coder isKindOfClass:[NSKeyedArchiver class]]){
    NSKeyedArchiver *keyed=(NSKeyedArchiver *)coder;

    [keyed encodeObject:self forKey:@"NS.data"];
   }
   else {
    [coder encodeDataObject:self];
   }
}

+data {
   return [[[self allocWithZone:NULL] init] autorelease];
}

+dataWithBytesNoCopy:(void *)bytes length:(NSUInteger)length freeWhenDone:(BOOL)freeWhenDone{
   return [[[self allocWithZone:NULL] initWithBytesNoCopy:bytes length:length freeWhenDone:freeWhenDone] autorelease];
}

+dataWithBytesNoCopy:(void *)bytes length:(NSUInteger)length {
   return [[[self allocWithZone:NULL] initWithBytesNoCopy:bytes length:length] autorelease];
}

+dataWithBytes:(const void *)bytes length:(NSUInteger)length {
   return [[[self allocWithZone:NULL] initWithBytes:bytes length:length] autorelease];
}

+dataWithData:(NSData *)data {
   return [[[self allocWithZone:NULL] initWithBytes:[data bytes] length:[data length]] autorelease];
}

+dataWithContentsOfFile:(NSString *)path {
   return [[[self allocWithZone:NULL] initWithContentsOfFile:path] autorelease];
}

+dataWithContentsOfMappedFile:(NSString *)path {
   return [[[self allocWithZone:NULL] initWithContentsOfMappedFile:path] autorelease];
}

+dataWithContentsOfURL:(NSURL *)url {
   return [[[self allocWithZone:NULL] initWithContentsOfURL:url] autorelease];
}

+dataWithContentsOfFile:(NSString *)path options:(NSUInteger)options error:(NSError **)errorp {
   return [[[self alloc] initWithContentsOfFile:path options:options error:errorp] autorelease];
}

+dataWithContentsOfURL:(NSURL *)url options:(NSUInteger)options error:(NSError **)errorp {
   return [[[self alloc] initWithContentsOfURL:url options:options error:errorp] autorelease];
}

-(const void *)bytes {
   NSInvalidAbstractInvocation();
   return NULL;
}

-(NSUInteger)length {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSUInteger)hash {
	return [self length];
}

-(BOOL)isEqual:other {
   if(self==other)
    return YES;

   if(![other isKindOfClass:objc_lookUpClass("NSData")])
    return NO;

   return [self isEqualToData:other];
}

-(BOOL)isEqualToData:(NSData *)other {
   NSUInteger length;

   if(self==other)
    return YES;

   length=[self length];
   if(length!=[other length])
    return NO;

   return NSBytesEqual([self bytes],[other bytes],length);
}

-(void)getBytes:(void *)result range:(NSRange)range {
   const char *bytes=[self bytes];
   NSUInteger    i;

   if(NSMaxRange(range)>[self length]){
    NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",
     NSStringFromRange(range),[self length]);
   }

   for(i=0;i<range.length;i++)
    ((char *)result)[i]=bytes[range.location+i];
}

-(void)getBytes:(void *)result {
   NSRange range={0,[self length]};
   [self getBytes:result range:range];
}

-(void)getBytes:(void *)result length:(NSUInteger)length {
   NSRange range={0,length};
   [self getBytes:result range:range];
}

-(NSData *)subdataWithRange:(NSRange)range {
   void *buffer;

   if(NSMaxRange(range)>[self length]){
    NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",
     NSStringFromRange(range),[self length]);
   }

   buffer=NSZoneCalloc(NSZoneFromPointer(self),range.length,sizeof(char));

   [self getBytes:buffer range:range];

   return NSAutorelease(NSData_concreteNewNoCopy(NULL,buffer,range.length));
}

-(BOOL)writeToFile:(NSString *)path atomically:(BOOL)atomically {
   NSUInteger options=0;
   if (atomically) options=NSAtomicWrite;
   return [self writeToFile:path options:options error:(NSError **)0];
}

-(BOOL)writeToURL:(NSURL *)url atomically:(BOOL)atomically {
   NSUInteger options=0;
   if (atomically) options=NSAtomicWrite;
   return [self writeToURL:url options:options error:(NSError **)0];
}

-(BOOL)writeToFile:(NSString *)path options:(NSUInteger)options error:(NSError **)errorp {
   return [[NSPlatform currentPlatform] writeContentsOfFile:path bytes:[self bytes] length:[self length] options:options error:errorp];
}

-(BOOL)writeToURL:(NSURL *)url options:(NSUInteger)options error:(NSError **)errorp {
  if(![url isFileURL]){
   NSLog(@"-[%@ %s]: Only file: URLs are supported so far.",[self class],sel_getName(_cmd));
   return NO;
  }
  return [self writeToFile:[url path] options:options error:errorp];
}

-(NSString *)description {
   const char *hex="0123456789ABCDEF";
   const char *bytes=[self bytes];
   NSUInteger    length=[self length];
   NSUInteger    pos=0,i;
   char       *cString;
   NSString   *string=NSAutorelease(NSString_cStringNewWithCapacity(NULL,
     1+length*2+(length/4)+1,&cString));

   cString[pos++]='<';
   for(i=0;i<length;){
    uint8_t byte=bytes[i];

    cString[pos++]=hex[byte>>4];
    cString[pos++]=hex[byte&0x0F];
    i++;

    if((i%4)==0 && i<length)
     cString[pos++]=' ';
   }
   cString[pos++]='>';

   return string;
}

- (unsigned) _cfTypeID {
   return kNSCFTypeData;
}

@end
