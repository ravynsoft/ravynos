/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2DataProvider.h>
#import <Foundation/NSData.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSStream.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSValue.h>
#import <string.h>

@implementation O2DataProvider

-initWithData:(NSData *)data {
   _inputStream=[[NSInputStream inputStreamWithData:data] retain];
   [_inputStream open];
   _data=[data retain];
   _isDirectAccess=YES;
   _bytes=[data bytes];
   _length=[data length];
   return self;
}

-initWithBytes:(const void *)bytes length:(size_t)length {
   NSData *data=[NSData dataWithBytesNoCopy:(void *)bytes length:length freeWhenDone:NO];
   return [self initWithData:data];
}

-initWithFilename:(const char *)pathCString {
// why doesn't O2DataProvider use CFString's, ugh
   NSUInteger len=strlen(pathCString);
   _path=[[[NSFileManager defaultManager] stringWithFileSystemRepresentation:pathCString length:len] copy];
   _inputStream=[[NSInputStream inputStreamWithFileAtPath:_path] retain];
   [_inputStream open];
   _data=nil;
   _isDirectAccess=NO;
   _bytes=nil;
   _length=0;
   return self;
}

-initWithURL:(NSURL *)url {
   NSData *data=[[NSData alloc] initWithContentsOfURL:url];
   id      result=[self initWithData:data];
   
   [data release];
   
   return result;
}

O2DataProviderRef O2DataProviderCreateWithData(void *info,const void *data,size_t size,O2DataProviderReleaseDataCallback releaseCallback) {
   return [[O2DataProvider alloc] initWithBytes:data length:size];
}

O2DataProviderRef O2DataProviderCreateWithCFData(CFDataRef data) {
   return [[O2DataProvider alloc] initWithData:(NSData *)data];
}

O2DataProviderRef O2DataProviderCreateWithURL(NSURL *url) {
   return [[O2DataProvider alloc] initWithURL:url];
}

O2DataProviderRef O2DataProviderCreateWithFilename(const char *pathCString) {
   return [[O2DataProvider alloc] initWithFilename:pathCString];
}

O2DataProviderRef O2DataProviderRetain(O2DataProviderRef self) {
   return (self!=NULL)?(O2DataProviderRef)CFRetain(self):NULL;
}

void O2DataProviderRelease(O2DataProviderRef self) {
   if(self!=NULL)
    CFRelease(self);
}

CFDataRef O2DataProviderCopyData(O2DataProviderRef self) {
   if(self->_data!=nil)
    return (CFDataRef)[self->_data copy];
   else
    return (CFDataRef)[[NSData alloc] initWithContentsOfFile:self->_path]; 
}

-(void)dealloc {
   [_inputStream close];
   [_inputStream release];
   [_data release];
   [_path release];
   [super dealloc];
}

-(NSString *)path {
   return _path;
}

-(NSInputStream *)inputStream {
   return _inputStream;
}

-(BOOL)isDirectAccess {
   return _isDirectAccess;
}

-(NSData *)data {
   return _data;
}

-(const void *)bytes {
   return _bytes;
}

-(size_t)length {
   return _length;
}

-(void)rewind {
   NSNumber *number=[[NSNumber alloc] initWithInt:0];
   
   [_inputStream setProperty:number forKey:NSStreamFileCurrentOffsetKey];

   [number release];
}

-(NSInteger)getBytes:(void *)bytes range:(NSRange)range {
   if(_data!=nil){
    NSUInteger length=[_data length];
    
    if(NSMaxRange(range)<=length){
     [_data getBytes:bytes range:range];
     return range.length;
    }
    else if(range.location<length){
     range.length=length-range.location;
     [_data getBytes:bytes range:range];
     return range.length;
    }
    else
     return -1;
   }
   else {
    NSInteger check=-1;
    NSNumber *number=[[NSNumber alloc] initWithInt:range.location];

   if([_inputStream setProperty:number forKey:NSStreamFileCurrentOffsetKey])
    check=[_inputStream read:bytes maxLength:range.length];

    [number release];
    
    return check;
   }
}

@end
