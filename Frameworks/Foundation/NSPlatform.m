/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSPlatform.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSString.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSPipe.h>
#import <Foundation/NSMutableDictionary.h>
#import <Foundation/NSNotificationCenter.h>
#import <Foundation/NSFileHandle.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSData.h>
#import <Foundation/NSRaiseException.h>

extern NSString *NSPlatformClassName;

@implementation NSPlatform

+currentPlatform {
   return NSThreadSharedInstance(NSPlatformClassName);
}

-(NSInputSource *)parentDeathInputSource {
   return nil;
}

-(Class)taskClass {
   NSInvalidAbstractInvocation();
   return Nil;
}

-(Class)socketClass {
   NSInvalidAbstractInvocation();
   return Nil;
}

-(Class)socketPortClass {
    NSInvalidAbstractInvocation();
    return Nil;
}

-(Class)pipeClass {
   NSInvalidAbstractInvocation();
   return Nil;
}

-(Class)lockClass {
   NSInvalidAbstractInvocation();
   return Nil;
}

-(Class)recursiveLockClass
{
    NSInvalidAbstractInvocation();
   return Nil;
}

-(Class)conditionLockClass {
   NSInvalidAbstractInvocation();
   return Nil;
}

-(Class)conditionClass;
{
    NSInvalidAbstractInvocation();
    return Nil;
}

-(Class)persistantDomainClass {
   NSInvalidAbstractInvocation();
   return Nil;
}

-(Class)timeZoneClass {
   NSInvalidAbstractInvocation();
   return Nil;
}

-(NSString *)userName {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSString *)fullUserName {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSString *)homeDirectory {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSString *)libraryDirectory {
	NSInvalidAbstractInvocation();
	return nil;
}

-(NSString *)temporaryDirectory {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSArray *)arguments {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSDictionary *)environment {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSString *)hostName {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSString *)DNSHostName {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSArray *)addressesForDNSHostName:(NSString *)name {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSString *)hostNameByAddress:(NSString *)address{
    NSInvalidAbstractInvocation();
    return nil;
}

-(void *)mapContentsOfFile:(NSString *)path length:(NSUInteger *)length {
   NSInvalidAbstractInvocation();
   return NULL;
}

-(void)unmapAddress:(void *)ptr length:(NSUInteger)length {
   NSInvalidAbstractInvocation();
}

-(BOOL)writeContentsOfFile:(NSString *)path bytes:(const void *)bytes length:(NSUInteger)length options:(NSUInteger)options error:(NSError **)errorp {
   NSInvalidAbstractInvocation();
   return NO;
}

-(void)checkEnvironmentKey:(NSString *)key value:(NSString *)value {
   if([key isEqualToString:@"NSZombieEnabled"]){   
    if((NSZombieEnabled=[value isEqual:@"YES"]))
     NSCLog("NSZombieEnabled=YES");
   }
}

@end
