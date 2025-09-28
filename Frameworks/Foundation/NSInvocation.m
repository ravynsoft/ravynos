/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSInvocation.h>
#import <Foundation/NSMethodSignature.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSRaise.h>

#import <sys/param.h>
#include <string.h>
#import <objc/message.h>

id objc_msgSendv(id self, SEL selector, unsigned arg_size, void *arg_frame);

@implementation NSInvocation

-(void)buildFrame {
   NSInteger  i,count=[_signature numberOfArguments];
   NSUInteger align;

   NSGetSizeAndAlignment([_signature methodReturnType],&_returnSize,&align);
   _returnValue=NSZoneCalloc(NULL,MAX(_returnSize, sizeof(NSUInteger)),1);

   _argumentFrameSize=0;
   _argumentSizes=NSZoneCalloc(NULL,count,sizeof(NSUInteger));
   _argumentOffsets=NSZoneCalloc(NULL,count,sizeof(NSUInteger));

   for(i=0;i<count;i++){
    NSUInteger naturalSize;
    NSUInteger promotedSize;

    _argumentOffsets[i]=_argumentFrameSize;

    NSGetSizeAndAlignment([_signature getArgumentTypeAtIndex:i],&naturalSize,&align);
    promotedSize=((naturalSize+sizeof(NSUInteger)-1)/sizeof(NSUInteger))*sizeof(NSUInteger);

    _argumentSizes[i]=naturalSize;
    _argumentFrameSize+=promotedSize;
   }
}

-initWithMethodSignature:(NSMethodSignature *)signature {
   if(signature==nil){
    [self dealloc];
    [NSException raise:NSInvalidArgumentException format:@"nil signature in NSInvocation creation"];
    return nil;
   }

   _signature=[signature retain];

   [self buildFrame];

   _argumentFrame=NSZoneCalloc(NULL,_argumentFrameSize,1);

   return self;
}

-initWithMethodSignature:(NSMethodSignature *)signature arguments:(void *)arguments {
   unsigned       i;
   uint8_t *stackFrame=arguments;

   [self initWithMethodSignature:signature];

   for(i=0;i<_argumentFrameSize;i++)
    _argumentFrame[i]=stackFrame[i];

   return self;
}

-(void)dealloc {
    if (_retainArguments) {
        NSInteger i, count = [_signature numberOfArguments];

        for (i = 0; i < count; ++i) {
            const char *type = [_signature getArgumentTypeAtIndex:i];

            switch (type[0]) {
                case '@': {
                    id object;

                    [self getArgument:&object atIndex:i];
                    [object release];
                    break;
                }

                case '*': {
                    char *ptr;

                    [self getArgument:&ptr atIndex:i];
                    NSZoneFree(NULL, ptr);
                    break;
                }

                default:
                    break;
            }
        }
    }

   NSZoneFree(NULL,_returnValue);
   NSZoneFree(NULL,_argumentSizes);
   NSZoneFree(NULL,_argumentOffsets);
   NSZoneFree(NULL,_argumentFrame);
   [_signature release];
   NSDeallocateObject(self);
   return;
   [super dealloc];
}

static void *bufferForType(void *buffer,const char *type){
   NSUInteger size,align;

   NSGetSizeAndAlignment(type,&size,&align);
   if(buffer!=NULL)
    NSZoneFree(NULL,buffer);

   return NSZoneMalloc(NULL,size);
}

-initWithCoder:(NSCoder *)coder {
   const char *type;
   NSInteger         i,count;
   void       *buffer=NULL;

   _signature=[[coder decodeObject] retain];

   [self buildFrame];

   _argumentFrame=NSZoneCalloc(NULL,_argumentFrameSize,1);

   if([_signature methodReturnLength]>0){
    type=[_signature methodReturnType];
    buffer=bufferForType(buffer,type);
    [coder decodeValueOfObjCType:type at:buffer];
    [self setReturnValue:buffer];
   }

   count=[_signature numberOfArguments];
   for(i=0;i<count;i++){
    type=[_signature getArgumentTypeAtIndex:i];
    buffer=bufferForType(buffer,type);
    [coder decodeValueOfObjCType:type at:buffer];
    [self setArgument:buffer atIndex:i];
   }
   NSZoneFree(NULL,buffer);

   return self;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   const char *type;
   NSInteger         i,count;
   void       *buffer=NULL;

   [coder encodeObject:_signature];

   if([_signature methodReturnLength]>0){
    type=[_signature methodReturnType];
    buffer=bufferForType(buffer,type);
    [self getReturnValue:buffer];
    [coder encodeValueOfObjCType:type at:buffer];
   }

   count=[_signature numberOfArguments];
   for(i=0;i<count;i++){
    type=[_signature getArgumentTypeAtIndex:i];
    buffer=bufferForType(buffer,type);
    [self getArgument:buffer atIndex:i];
    [coder encodeValueOfObjCType:type at:buffer];
   }
}

+(NSInvocation *)invocationWithMethodSignature:(NSMethodSignature *)signature {
   return [[[self allocWithZone:NULL] initWithMethodSignature:signature] autorelease];
}

+(NSInvocation *)invocationWithMethodSignature:(NSMethodSignature *)signature arguments:(void *)arguments {
   return [[[self allocWithZone:NULL] initWithMethodSignature:signature arguments:arguments] autorelease];
}

-(NSMethodSignature *)methodSignature {
   return _signature;
}

static void byteCopy(void *src,void *dst,NSUInteger length){
   NSInteger i;

   for(i=0;i<length;i++)
    ((char *)dst)[i]=((char *)src)[i];
}

-(void)getReturnValue:(void *)pointerToValue {
   byteCopy(_returnValue,pointerToValue,_returnSize);
}

-(void)setReturnValue:(void *)pointerToValue {
   byteCopy(pointerToValue,_returnValue,_returnSize);
}


-(void)getArgument:(void *)pointerToValue atIndex:(NSInteger)index
{
    NSUInteger naturalSize = _argumentSizes[index];
    NSUInteger promotedSize = ((naturalSize + sizeof(NSUInteger) - 1) / sizeof(NSUInteger)) * sizeof(NSUInteger);

    if (naturalSize == promotedSize) {
        byteCopy(_argumentFrame + _argumentOffsets[index], pointerToValue, naturalSize);
    } else if (promotedSize == sizeof(long)) {
        long promoted;

        byteCopy(_argumentFrame + _argumentOffsets[index], &promoted, promotedSize);
        if (naturalSize == 1) {
            *((char *)pointerToValue) = (char)promoted;
        } else if (naturalSize == 2) {
            *((short *)pointerToValue) = (short)promoted;
        } else if (naturalSize == 4) {
            *((int32_t *)pointerToValue) = (int32_t)promoted;
        }
    } else {
        [NSException raise:NSInvalidArgumentException format:@"Unable to convert naturalSize=%d to promotedSize=%d", naturalSize, promotedSize];
    }
}


- (void)setArgument:(void *)pointerToValue atIndex:(NSInteger)index
{
    NSUInteger naturalSize = _argumentSizes[index];
    NSUInteger promotedSize = ((naturalSize + sizeof(NSUInteger) - 1) / sizeof(NSUInteger)) * sizeof(NSUInteger);

    if (naturalSize == promotedSize) {
        byteCopy(pointerToValue, _argumentFrame + _argumentOffsets[index], naturalSize);
    } else if (promotedSize == sizeof(long)) {
        long promoted;

        if (naturalSize == 1) {
            promoted = *((char *)pointerToValue);
        } else if (naturalSize == 2) {
            promoted = *((short *)pointerToValue);
        } else if (naturalSize == 4) {
            promoted = *((int32_t *)pointerToValue);
        }

        byteCopy(&promoted, _argumentFrame + _argumentOffsets[index], promotedSize);
    } else {
        [NSException raise:NSInvalidArgumentException format:@"Unable to convert naturalSize=" NSUIntegerFormat " to promotedSize=" NSUIntegerFormat, naturalSize, promotedSize];
    }
}


-(void)retainArguments {
    if (!_retainArguments) {
        NSInteger i, count = [_signature numberOfArguments];

        _retainArguments=YES;
        for (i = 0; i < count; ++i) {
            const char *type = [_signature getArgumentTypeAtIndex:i];

            switch (type[0]) {
                case '@': {
                    id object;

                    [self getArgument:&object atIndex:i];
                    [object retain];
                    break;
                }

                case '*': {
                    char *ptr, *copy, length;

                    [self getArgument:&ptr atIndex:i];
                    length = strlen(ptr);
                    copy = NSZoneCalloc(NULL, length+1, 1);
                    byteCopy(ptr, copy, length);
                    [self setArgument:&copy atIndex:i];
                    break;
                }

                default:
                    break;
            }
        }
    }
}

-(BOOL)argumentsRetained {
   return _retainArguments;
}

-(SEL)selector {
   SEL result;

   [self getArgument:&result atIndex:1];

   return result;
}

-(void)setSelector:(SEL)selector {
   [self setArgument:&selector atIndex:1];
}

-target {
   id result;

   [self getArgument:&result atIndex:0];

   return result;
}

-(void)setTarget:target {
   [self setArgument:&target atIndex:0];
}


- (void)invokeWithTarget:target
{
    [self setTarget: target];
    [self invoke];
}


- (void)invoke
{
    const char *returnType = [_signature methodReturnType];
    void *msgSendv = objc_msgSendv;

    switch (returnType[0]) {
        case 'r':
        case 'n':
        case 'N':
        case 'o':
        case 'O':
        case 'R':
        case 'V':
            returnType++;
            break;
    }

    switch (returnType[0]) {
        case _C_CHR:
        case _C_UCHR:
            {
                char (*function)() = msgSendv;
                char value = function([self target], [self selector], _argumentFrameSize, _argumentFrame);

                [self setReturnValue:&value];
            }
            break;

        case _C_SHT:
        case _C_USHT:
            {
                short (*function)() = msgSendv;
                short value = function([self target], [self selector], _argumentFrameSize, _argumentFrame);

                [self setReturnValue:&value];
            }
            break;

        case _C_INT:
        case _C_UINT:
            {
                int (*function)() = msgSendv;
                int value = function([self target], [self selector], _argumentFrameSize, _argumentFrame);

                [self setReturnValue:&value];
            }
            break;

        case _C_LNG:
        case _C_ULNG:
            {
                long (*function)() = msgSendv;
                long value = function([self target], [self selector], _argumentFrameSize, _argumentFrame);

                [self setReturnValue:&value];
            }
            break;

        case _C_LNG_LNG:
        case _C_ULNG_LNG:
            {
                long long (*function)() = msgSendv;
                long long value = function([self target], [self selector], _argumentFrameSize, _argumentFrame);

                [self setReturnValue:&value];
            }
            break;

        case _C_FLT:
            {
                float (*function)() = msgSendv;
                float value = function([self target], [self selector], _argumentFrameSize, _argumentFrame);

                [self setReturnValue:&value];
            }
            break;

        case _C_DBL:
            {
                double (*function)() = msgSendv;
                double value = function([self target], [self selector], _argumentFrameSize, _argumentFrame);

                [self setReturnValue:&value];
            }
            break;

        case _C_VOID:
            {
                void (*function)() = msgSendv;

                function([self target], [self selector], _argumentFrameSize, _argumentFrame);
            }
            break;

        case _C_CHARPTR:
        case _C_ID:
        case _C_CLASS:
        case _C_SEL:
            {
                void *(*function)() = msgSendv;
                void *value=function([self target], [self selector], _argumentFrameSize, _argumentFrame);

                [self setReturnValue:&value];
            }
            break;

        default:
            {
                NSUInteger size, alignment;

                NSGetSizeAndAlignment(returnType, &size, &alignment);
                if (size <= sizeof(long)) {
                    long (*function)() = msgSendv;
                    long value = function([self target], [self selector], _argumentFrameSize, _argumentFrame);

                    [self setReturnValue:&value];
                } else if (size <= sizeof(long long)) {
                    long long (*function)() = msgSendv;
                    long long value = function([self target], [self selector], _argumentFrameSize, _argumentFrame);

                    [self setReturnValue:&value];
                } else {
#ifdef __clang__
// see http://llvm.org/bugs/show_bug.cgi?id=9254
                    @throw @"NSInvocation: current implementation of struct returning invocation is not supported by Clang.";
#else
                    struct structReturn {
                        char *result;;
                    } (*function)() = (struct structReturn (*)())msgSendv; // should be msgSend_stret
                    struct structReturn value;

			value.result = calloc(size, sizeof(char));

// FIX internal compiler error on windows/linux/bsd
#if !defined(WIN32) && !defined(BSD) && !defined(LINUX)
                    value = function([self target], [self selector], _argumentFrameSize, _argumentFrame);
#else
                    if (function) {/*avoid compiler warning*/}
#endif

                    [self setReturnValue:&value];
			free(value.result);
#endif
                }
            }
            break;
    }
}


-(id)description
{
   return [NSString stringWithFormat:@"<%@ with signature %@>", [super description], [_signature description]];
}

@end
