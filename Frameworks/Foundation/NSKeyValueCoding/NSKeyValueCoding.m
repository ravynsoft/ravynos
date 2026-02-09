/* Copyright (c) 2006-2007 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSNull.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSString.h>
#import <Foundation/NSInvocation.h>
#import <Foundation/NSMethodSignature.h>
#import <Foundation/NSKeyValueObserving.h>
#include <objc/runtime.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>

#import "NSKVCMutableArray.h"
#import "NSString+KVCAdditions.h"
#import "NSKeyValueObserving-Private.h"

NSString *const NSUndefinedKeyException = @"NSUnknownKeyException";


@implementation NSObject (KeyValueCoding)

-(void)_demangleTypeEncoding:(const char*)type to:(char*)cleanType
{
	while(*type)
	{
		if(*type=='"')
		{
			type++;
			while(*type && *type!='"')
				type++;
			type++;
		}
		while(isdigit(*type))
			type++;
		*cleanType=*type;
		type++; cleanType++;
		*cleanType=0;
	}
}

-(id)_wrapValue:(void*)value ofType:(const char*)type
{
   char* cleanType=__builtin_alloca(strlen(type)+1);
   // strip offsets & quotes from type
   [self _demangleTypeEncoding:type to:cleanType];

	if(type[0]!='@' && strlen(cleanType)>1)
	{
		return [NSValue valueWithBytes:value objCType:cleanType];
	}

	switch(type[0])
	{
		case '@':
		case '#':
			return *(id*)value;
		case 'i':
			return [NSNumber numberWithInt:*(int*)value];
		case 'I':
			return [NSNumber numberWithUnsignedInt:*(int*)value];
		case 'f':
			return [NSNumber numberWithFloat:*(float*)value];
		case 'd':
			return [NSNumber numberWithDouble:*(double*)value];
		case 's':
			return [NSNumber numberWithShort:*(short*)value];
		case 'S':
			return [NSNumber numberWithUnsignedShort:*(unsigned short*)value];
		case 'c':
			return [NSNumber numberWithChar:*(char*)value];
		case 'C':
			return [NSNumber numberWithUnsignedChar:*(unsigned char*)value];
		case 'q':
			return [NSNumber numberWithLongLong:*(long long*)value];
		case 'Q':
			return [NSNumber numberWithUnsignedLongLong:*(unsigned long long*)value];
		default:
// FIX #warning some wrapping types unimplemented
            [NSException raise:NSInvalidArgumentException format:@"FIXME: wrap value of type %s unimplemented for get", type];
            return nil;
	}
}

-(BOOL)_setValue:(id)value toBuffer:(void*)buffer ofType:(const char*)type shouldRetain:(BOOL)shouldRetain
{
	char* cleanType=__builtin_alloca(strlen(type)+1);
	[self _demangleTypeEncoding:type to:cleanType];

	if(cleanType[0]!='@' && cleanType[0]!='#' && strlen(cleanType)>1)
	{
		if(strcmp([value objCType], cleanType))
		{
			NSLog(@"trying to set value of type %s for type %@", cleanType, [value objCType]);
			return NO;
		}
		[value getValue:buffer];
		return YES;
	}

	switch(cleanType[0])
	{
		case '#':
			shouldRetain = NO; // no need to retain classes
		case '@':
         if(shouldRetain) {
            if((*(id*)buffer)!=value) {
               [(*(id*)buffer) release];
               *(id*)buffer = [value retain];
            }
         }
         else {
            *(id*)buffer = value;
         }

			return YES;
		case 'i':
			*(int*)buffer = [value intValue];
			return YES;
		case 'I':
			*(unsigned int*)buffer = [value unsignedIntValue];
			return YES;
		case 'f':
			*(float*)buffer = [value floatValue];
			return YES;
		case 'd':
			*(double*)buffer = [value doubleValue];
			return YES;

		case 'c':
			*(char*)buffer = [value charValue];
			return YES;
		case 'C':
			*(unsigned char*)buffer = [value unsignedCharValue];
			return YES;

        case 'q':
            *(long long*)buffer = [value longLongValue];
            return YES;

        case 'Q':
            *(unsigned long long*)buffer = [value unsignedLongLongValue];
            return YES;
		default:
// FIX #warning some wrapping types unimplemented
            [NSException raise:NSInvalidArgumentException format:@"FIXME: wrap value of type %s unimplemented for set", type];
			return NO;
	}
}

-(id)_wrapReturnValueForSelector:(SEL)sel
{
	id sig=[self methodSignatureForSelector:sel];
	const char* type=[sig methodReturnType];
	if(strcmp(type, "@") && strcmp(type, "#")) // neither object or class
	{
		id inv=[NSInvocation invocationWithMethodSignature:sig];
		[inv setSelector:sel];
		[inv setTarget:self];
		[inv invoke];

		NSUInteger returnLength=[sig methodReturnLength];
		void *returnValue=__builtin_alloca(returnLength);
		[inv getReturnValue:returnValue];

		return [self _wrapValue:returnValue ofType:type];
	}
	return [self performSelector:sel];
}

-(void)_setValue:(id)value withSelector:(SEL)sel fromKey:(id)key
{
	id sig=[self methodSignatureForSelector:sel];
	const char* type=[sig getArgumentTypeAtIndex:2];
	if(strcmp(type, "@") && strcmp(type, "#")) // neither object or class
	{
		if(!value)
		{
			// value is nil and accessor doesn't take object type
			return [self setNilValueForKey:key];
		}
		NSUInteger size, align;
		NSInvocation* inv=[NSInvocation invocationWithMethodSignature:sig];
		[inv setSelector:sel];
		[inv setTarget:self];

		NSGetSizeAndAlignment(type, &size, &align);
		void *buffer=__builtin_alloca(size);
      memset(buffer, 0, size);
		[self _setValue:value toBuffer:buffer ofType:type shouldRetain:NO];

		[inv setArgument:buffer atIndex:2];

		[inv invoke];
		return;
	}
	[self performSelector:sel withObject:value];
}

#pragma mark -
#pragma mark Primary methods


- (id)valueForKey: (NSString*)key
{
    if (!key) {
        id value = [self valueForUndefinedKey:nil];
        return value;
    }

    const char *keyCString = [key UTF8String];
    SEL sel = sel_getUid(keyCString);

    // FIXME: getKey, _getKey, isKey, _isKey are missing

    if ([self respondsToSelector:sel]) {
        id value = [self _wrapReturnValueForSelector:sel];
        return value;
    }

    size_t keyCStringLength = strlen(keyCString);
    char *selBuffer = __builtin_alloca(keyCStringLength + 5);

    char *keyname = __builtin_alloca(keyCStringLength + 1);
    strcpy(keyname, keyCString);

    #define TRY_FORMAT(format)\
            sprintf(selBuffer, format, keyname);\
            sel = sel_getUid(selBuffer);\
            if ([self respondsToSelector:sel]) {\
                id value = [self _wrapReturnValueForSelector:sel];\
                return value;\
            }
    TRY_FORMAT("_%s");
    keyname[0] = toupper(keyname[0]);
    TRY_FORMAT("is%s");
    TRY_FORMAT("_is%s");
//    TRY_FORMAT("get%s");
//    TRY_FORMAT("_get%s");
    #undef TRY_FORMAT

    if ([object_getClass(self) accessInstanceVariablesDirectly]) {
        sprintf(selBuffer, "_%s", keyCString);
        sel = sel_getUid(selBuffer);

        if ([self respondsToSelector:sel]) {
            id value = [self _wrapReturnValueForSelector:sel];
            return value;
        }


        Ivar ivar = class_getInstanceVariable(object_getClass(self), selBuffer);
        if (!ivar) {
            ivar = class_getInstanceVariable(object_getClass(self), keyCString);
        }

        if (ivar) {
            id value = [self _wrapValue:(void*)self + ivar_getOffset(ivar) ofType:ivar_getTypeEncoding(ivar)];
            return value;
        }

    }

    id value = [self valueForUndefinedKey:key];
    return value;
}


-(void)setValue:(id)value forKey:(NSString *)key
{
    NSUInteger cStringLength = [key length];
    char keyCString[cStringLength + 1];
    char uppercaseKeyCString[cStringLength + 1];
    char check[cStringLength + 10]; // needs to accomodate key and set/_set: stuff

    [key getCString:keyCString];
    strcpy(uppercaseKeyCString, keyCString);
    uppercaseKeyCString[0] = toupper(uppercaseKeyCString[0]);

    strcpy(check,"set"); strcat(check, uppercaseKeyCString); strcat(check, ":");

    SEL sel = sel_getUid(check);
    if([self respondsToSelector:sel]) {
        return [self _setValue:value withSelector:sel fromKey:key];
    }

	BOOL shouldNotify=[object_getClass(self) automaticallyNotifiesObserversForKey:key] && [self _hasObserverForKey: key] ;
	if (shouldNotify == YES) {
	}
	if([object_getClass(self) accessInstanceVariablesDirectly])
	{
        // Check the _setXXX: method
        strcpy(check,"_set");strcat(check,uppercaseKeyCString);strcat(check,":");
        sel = sel_getUid(check);

        if ([self respondsToSelector:sel]) {
            return [self _setValue:value withSelector:sel fromKey:key];
        }

        strcpy(check, "_"); strcat(check, keyCString);
        Ivar ivar = class_getInstanceVariable(object_getClass(self), check);
        if (!ivar) {
            strcpy(check,"_is"); strcat(check, uppercaseKeyCString);
            ivar = class_getInstanceVariable(object_getClass(self), check);
        }
        if (!ivar) {
            ivar = class_getInstanceVariable(object_getClass(self), keyCString);
        }
        if (!ivar) {
            strcpy(check, "is"); strcat(check, uppercaseKeyCString);
            ivar = class_getInstanceVariable(object_getClass(self), check);
        }

        if (ivar) {
            if (shouldNotify) {
                [self willChangeValueForKey:key];
            }
            // if value is nil and ivar is not an object type
            if (!value && ivar_getTypeEncoding(ivar)[0] != '@') {
                [self setNilValueForKey:key];
            } else {
                [self _setValue:value toBuffer:(void*)self + ivar_getOffset(ivar) ofType:ivar_getTypeEncoding(ivar) shouldRetain:YES];
            }
            if (shouldNotify) {
                [self didChangeValueForKey:key];
            }
            return;
        }
    }

    // Path of last resort - but still assume we're letting people know about changes
    if (shouldNotify) {
        [self willChangeValueForKey:key];
    }
    [self setValue:value forUndefinedKey:key];
    if (shouldNotify) {
        [self didChangeValueForKey:key];
    }
}


- (BOOL)validateValue:(id *)ioValue forKey:(NSString *)key error:(NSError **)outError
{
	SEL sel=NSSelectorFromString([NSString stringWithFormat:@"validate%@:error:", [key capitalizedString]]);
	if([self respondsToSelector:sel])
	{
		id inv=[NSInvocation invocationWithMethodSignature:[self methodSignatureForSelector:sel]];
		[inv setSelector:sel];
		[inv setTarget:self];
		[inv setArgument:ioValue atIndex:2];
		[inv setArgument:outError atIndex:3];
		[inv invoke];
		BOOL ret;
		[inv getReturnValue:&ret];
		return ret;
	}
	return YES;
}

+(BOOL)accessInstanceVariablesDirectly {
   return YES;
}

-valueForUndefinedKey:(NSString *)key {
   [NSException raise:NSUndefinedKeyException format:@"%@: trying to get undefined key '%@'", [self className], key];
   return nil;
}

-(void)setValue:(id)value forUndefinedKey:(NSString *)key {
   [NSException raise:NSUndefinedKeyException format:@"%@: trying to set undefined key '%@'", [self className], key];
}

-(void)setNilValueForKey:key {
   [NSException raise:@"NSInvalidArgumentException"  format:@"%@: trying to set nil value for key '%@'", [self className], key];
}

- (id)valueForKeyPath:(NSString*)keyPath {

   NSString* firstPart, *rest;
   [keyPath _KVC_partBeforeDot:&firstPart afterDot:&rest];

	if(rest) {
		return [[self valueForKeyPath:firstPart] valueForKeyPath:rest];
	}
	else {
		return [self valueForKey:firstPart];
	}
}

-(void)setValue:(id)value forKeyPath:(NSString *)keyPath {

   NSString* firstPart, *rest;
   [keyPath _KVC_partBeforeDot:&firstPart afterDot:&rest];

	if(rest) {
		id firstPartObj = [self valueForKey:firstPart];
		[firstPartObj setValue:value forKeyPath:rest];
	} else {
		[self setValue:value  forKey:firstPart];
   }
}

- (BOOL)validateValue:(id *)ioValue forKeyPath:(NSString *)keyPath error:(NSError **)outError
{
	id array=[[[keyPath componentsSeparatedByString:@"."] mutableCopy] autorelease];
	id lastPathComponent=[array lastObject];
	[array removeObject:lastPathComponent];
	id en=[array objectEnumerator];
	id pathComponent;
	id ret=self;
	while((pathComponent = [en nextObject]) && ret)
	{
		ret = [ret valueForKey:pathComponent];
	}

	BOOL valid = [self validateValue:ioValue forKey:lastPathComponent error:outError];

	return valid;
}


-(NSDictionary *)dictionaryWithValuesForKeys:(NSArray *)keys
{
	id en=[keys objectEnumerator];
	id ret=[NSMutableDictionary dictionary];
	id key;
	while((key=[en nextObject]))
	{
		id value=[self valueForKey:key];
		[ret setObject:value ? value : (id)[NSNull null] forKey:key];
	}
	return ret;
}


- (void)setValuesForKeysWithDictionary:(NSDictionary *)keyedValues
{
	id en=[keyedValues keyEnumerator];
	NSString* key;
	NSNull* null=[NSNull null];
	while((key=[en nextObject]))
	{
		id value=[keyedValues objectForKey:key];
		[self setValue:value == null ? nil : value forKey:key];
	}
}

- (void) takeValuesFromDictionary: (NSDictionary*)aDictionary
{
	for(id key in aDictionary)
	{
		[self setValue:[aDictionary objectForKey:key] forKey:key];
	}
}

-(id)mutableArrayValueForKey:(id)key
{
	return [[[NSKVCMutableArray alloc] initWithKey:key forProxyObject:self] autorelease];
}

-(id)mutableArrayValueForKeyPath:(id)keyPath
{
	NSString* firstPart, *rest;
	[keyPath _KVC_partBeforeDot:&firstPart afterDot:&rest];
	if(rest)
		return [[self valueForKeyPath:firstPart] valueForKeyPath:rest];
	else
		return [[[NSKVCMutableArray alloc] initWithKey:firstPart forProxyObject:self] autorelease];
}
@end

