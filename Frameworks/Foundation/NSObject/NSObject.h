/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSZone.h>

@class NSCoder, NSInvocation, NSMethodSignature, NSString;

@protocol NSObject

- (NSZone *)zone;

- self;
- (Class) class;
- (Class)superclass;

- autorelease;
- (oneway void)release;
- retain;
- (NSUInteger)retainCount;

- (NSUInteger)hash;
- (BOOL)isEqual:object;

- (BOOL)isKindOfClass:(Class)aClass;
- (BOOL)isMemberOfClass:(Class)aClass;
- (BOOL)conformsToProtocol:(Protocol *)protocol;

- (BOOL)respondsToSelector:(SEL)selector;
- performSelector:(SEL)selector;
- performSelector:(SEL)selector withObject:object0;
- performSelector:(SEL)selector withObject:object0 withObject:object1;

- (BOOL)isProxy;

- (NSString *)description;
- (NSString *)debugDescription;

@end

@protocol NSCopying
- copyWithZone:(NSZone *)zone;
@end

@protocol NSMutableCopying
- mutableCopyWithZone:(NSZone *)zone;
@end

@protocol NSCoding
- initWithCoder:(NSCoder *)coder;
- (void)encodeWithCoder:(NSCoder *)coder;
@end

NS_ROOT_CLASS @interface NSObject<NSObject> {
    Class isa;
}

+ (NSInteger)version;
+ (void)setVersion:(NSInteger)version;

+ (void)load;

+ (void)initialize;

+ (Class)superclass;
+ (Class) class;
+ (BOOL)isSubclassOfClass:(Class)cls;

+ (BOOL)instancesRespondToSelector:(SEL)selector;
+ (BOOL)conformsToProtocol:(Protocol *)protocol;

+ (IMP)instanceMethodForSelector:(SEL)selector;
+ (NSMethodSignature *)instanceMethodSignatureForSelector:(SEL)selector;
+ (BOOL)resolveClassMethod:(SEL)selector;
+ (BOOL)resolveInstanceMethod:(SEL)selector;

+ copyWithZone:(NSZone *)zone;
+ mutableCopyWithZone:(NSZone *)zone;

+ (void)poseAsClass:(Class)aClass;

+ (NSString *)description;
+ (NSString *)debugDescription;

+ alloc;
+ allocWithZone:(NSZone *)zone;

- init;
+ new;
- (void)dealloc;
- (void)finalize;

- copy;
- mutableCopy;

- (Class)classForCoder;
- (Class)classForArchiver;
- (Class)classForKeyedArchiver;

- replacementObjectForCoder:(NSCoder *)coder;
- awakeAfterUsingCoder:(NSCoder *)coder;

- (IMP)methodForSelector:(SEL)selector;

- (void)doesNotRecognizeSelector:(SEL)selector;

- (NSMethodSignature *)methodSignatureForSelector:(SEL)selector;
- (void)forwardInvocation:(NSInvocation *)invocation;

- (NSString *)className;

@end
