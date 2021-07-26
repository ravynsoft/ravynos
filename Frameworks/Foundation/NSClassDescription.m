/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSClassDescription.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSRaise.h>

NSString * const NSClassDescriptionNeededForClassNotification=@"NSClassDescriptionNeededForClassNotification";

@implementation NSClassDescription

static NSMutableDictionary* classDescriptionCache = nil;

+ (NSClassDescription*) classDescriptionForClass: (Class) class
{
   id result;
   
        @synchronized(self) {
               result = [classDescriptionCache objectForKey:
NSStringFromClass(class)];
               if (!result) {
                       [[NSNotificationCenter defaultCenter] postNotificationName:
NSClassDescriptionNeededForClassNotification object: class];
               }
               result = [classDescriptionCache objectForKey:
NSStringFromClass(class)];
       }
       return result;
}

+ (void)invalidateClassDescriptionCache
{
        @synchronized(self) {
               [classDescriptionCache release];
               classDescriptionCache = nil;
       }
}

+ (void) registerClassDescription: (NSClassDescription*)description
                                                forClass: (Class) class
{
        @synchronized(self) {
               if (!classDescriptionCache) {
                       classDescriptionCache = [[NSMutableDictionary alloc] init];
               }
               [classDescriptionCache setObject: description forKey:
NSStringFromClass(class)];
       }
}

-(NSArray *)attributeKeys {
   NSUnimplementedMethod();
   return nil;
}

-(NSString *)inverseForRelationshipKey:(NSString *)key {
   NSUnimplementedMethod();
   return nil;
}

-(NSArray *)toManyRelationshipKeys {
   NSUnimplementedMethod();
   return nil;
}

-(NSArray *)toOneRelationshipKeys {
   NSUnimplementedMethod();
   return nil;
}

@end

@implementation NSObject(NSClassDescription)

-(NSClassDescription *)classDescription {
   return [NSClassDescription classDescriptionForClass:[self class]];
}

-(NSArray *)attributeKeys {
   return [[self classDescription] attributeKeys];
}

-(NSString *)inverseForRelationshipKey:(NSString *)key {
   return [[self classDescription] inverseForRelationshipKey:key];
}

-(NSArray *)toOneRelationshipKeys {
   return [[self classDescription] toOneRelationshipKeys];
}

-(NSArray *)toManyRelationshipKeys {
   return [[self classDescription] toManyRelationshipKeys];
}

@end

