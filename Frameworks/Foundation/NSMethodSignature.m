/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSMethodSignature.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSMapTable.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSStringHashing.h>
#import <Foundation/NSCoder.h>
#include <string.h>

@implementation NSMethodSignature

-initWithTypes:(const char *)typesCString {
   const char *next,*last;
   NSUInteger  size,align;
   BOOL        first=YES;
   size_t      typesCStringLength=strlen(typesCString);
   char       *types[typesCStringLength]; // at most strlen arguments
   
    // not guaranteed that typesCString is static
   _typesCString=NSZoneMalloc(NULL,typesCStringLength+1);
   strcpy(_typesCString,typesCString);
   next=last=_typesCString;
   _returnType=NULL;
   _numberOfArguments=0;

   while((next=NSGetSizeAndAlignment(next,&size,&align))!=last){
    NSUInteger length=next-last;
    char      *nextCString=NSZoneMalloc(NULL,length+1);
    
    strncpy(nextCString,last,length);
    nextCString[length]='\0';

    if(first)
     _returnType=nextCString;
    else {
     types[_numberOfArguments]=nextCString;
     _numberOfArguments++;
    }
    
    first=NO;

    while((*next>='0' && *next<='9') || *next=='+' || *next=='-' || *next=='?')
     next++; 

    if(*next=='\0')
      break;

    last=next;
   }
   
   if(_numberOfArguments){  
    _types=NSZoneMalloc(NULL,_numberOfArguments*sizeof(char *));
    
    NSInteger i;
    
    for(i=0;i<_numberOfArguments;i++)
     _types[i]=types[i];
   }
   
   return self;
}

-(void)dealloc {
   NSZoneFree(NULL,_typesCString);
   
   if(_returnType!=NULL)
    NSZoneFree(NULL,_returnType);
   
   NSInteger i;
   if(_types!=NULL){
    for(i=0;i<_numberOfArguments;i++)
     NSZoneFree(NULL,_types[i]);
    NSZoneFree(NULL,_types);
   }
   
   NSDeallocateObject(self);
   return;
   [super dealloc];
}

+(NSMethodSignature *)signatureWithObjCTypes:(const char *)typesCString {
   return [[[NSMethodSignature allocWithZone:NULL] initWithTypes:typesCString] autorelease];
}

-(NSString *)description {
   return [NSString stringWithFormat:@"<NSMethodSignature: -(%s)%s>",_returnType,_typesCString];
}

-(NSUInteger)hash {
   return NSStringHashZeroTerminatedASCII(_typesCString);
}

-(BOOL)isEqual:otherObject {
   if(self==otherObject)
    return YES;

   if([otherObject isKindOfClass:[NSMethodSignature class]]){
    NSMethodSignature *other=otherObject;

    return (strcmp(_typesCString,other->_typesCString)==0)?YES:NO;
   }

   return NO;
}

-(BOOL)isOneway {
   return (_returnType!=NULL && _returnType[0]=='V');
}

-(NSUInteger)frameLength {
   NSUInteger result=0;
   NSInteger  i;

   for(i=0;i<_numberOfArguments;i++){
    NSUInteger align;
    NSUInteger naturalSize;
    NSUInteger promotedSize;

    NSGetSizeAndAlignment(_types[i],&naturalSize,&align);
    promotedSize=((naturalSize+sizeof(long)-1)/sizeof(long))*sizeof(long);

    result+=promotedSize;
   }
   return result;
}

-(NSUInteger)methodReturnLength {
   NSUInteger size,align;

   NSGetSizeAndAlignment(_returnType,&size,&align);

   return size;
}

-(const char *)methodReturnType {
   return _returnType;
}

-(NSUInteger)numberOfArguments {
   return _numberOfArguments;
}

-(const char *)getArgumentTypeAtIndex:(NSUInteger)index {
   if(index>=_numberOfArguments){
    [NSException raise:NSInvalidArgumentException format:@"index (%d) is beyond number of arguments (%d)",index,_numberOfArguments];
    return NULL;
   }
   
   return _types[index];
}

@end
