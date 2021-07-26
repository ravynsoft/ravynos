/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSData_concrete.h>
#import <Foundation/NSData_mapped.h>

@implementation NSData_concrete

void *NSBytesReplicate(const void *src,NSUInteger count,NSZone *zone) {
   void    *dst=NSZoneMalloc(zone,count);

    if (dst != NULL) {
        NSByteCopy(src,dst,count);
    }
    
   return dst;
}

NSData *NSData_concreteNew(NSZone *zone,const char *bytes,NSUInteger length) {
   NSData_concrete *self=NSAllocateObject([NSData_concrete class],0,zone);
    if (self) {
       self->_length=length;
       self->_bytes=NSBytesReplicate(bytes,length,zone);
       self->_freeWhenDone=YES;
    }
    
   return self;
}

NSData *NSData_concreteNewNoCopy(NSZone *zone,void *bytes,NSUInteger length) {
   NSData_concrete *self=NSAllocateObject([NSData_concrete class],0,zone);

    if (self) {
       self->_length=length;
       self->_bytes=bytes;
       self->_freeWhenDone=YES;
    }
   return self;
}

-init {
   _length=0;
   _bytes=NULL;
   _freeWhenDone=YES;
   return self;
}

-initWithBytes:(const void *)bytes length:(NSUInteger)length {
   _length=length;
   _bytes=NSBytesReplicate(bytes,length,NSZoneFromPointer(self));
   _freeWhenDone=YES;
   return self;
}

-initWithBytesNoCopy:(void *)bytes length:(NSUInteger)length {
   _length=length;
   _bytes=bytes;
   _freeWhenDone=YES;
   return self;
}

-initWithBytesNoCopy:(void *)bytes length:(NSUInteger)length freeWhenDone:(BOOL)freeWhenDone {
   _length=length;
   _bytes=bytes;
   _freeWhenDone=freeWhenDone;
   return self;
}


- initWithContentsOfMappedFile:(NSString *)path
{
    [self dealloc];
    return (NSData_concrete *)[[NSData_mapped alloc] initWithContentsOfMappedFile:path];
}


-(void)dealloc {
   if(_bytes!=NULL && _freeWhenDone)
    NSZoneFree(NSZoneFromPointer(_bytes),_bytes);
   NSDeallocateObject(self);
   return;
   [super dealloc];
}

-(NSUInteger)length { return _length; }
-(const void *)bytes { return _bytes; }

@end
