/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSHost.h>
#import <Foundation/NSPlatform.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSRaise.h>

@implementation NSHost

+(BOOL)isHostCacheEnabled {
   NSUnimplementedMethod();
   return NO;
}

+(void)setHostCacheEnabled:(BOOL)value {
   NSUnimplementedMethod();
}

+(void)flushHostCache {
   NSUnimplementedMethod();
}

-initWithName:(NSString *)name {
   _name=[name copy];
   _addresses=nil;
   return self;
}

-(void)dealloc {
   [_name release];
   [_addresses release];
   [super dealloc];
}

+(NSHost *)currentHost {
   return [NSHost hostWithName:[[NSPlatform currentPlatform] DNSHostName]];
}

+(NSHost *)hostWithName:(NSString *)name {
   return [[[self allocWithZone:NULL] initWithName:name] autorelease];
}

+(NSHost *)hostWithAddress:(NSString *)address {
    NSString *hostName = [[NSPlatform currentPlatform] hostNameByAddress:address];
    
    if(hostName == nil) {
        return nil;
    }
    
    return [NSHost hostWithName:hostName];
}

-(BOOL)isEqualToHost:(NSHost *)host {
    for(NSString *address in [self addresses])
    {
        if ([[host addresses] containsObject:address] == YES) {
            return YES;
}
    }
    return NO;
}

-(void)_resolveAddressesIfNeeded {
   if([_addresses count]==0){
    _addresses=[[[NSPlatform currentPlatform] addressesForDNSHostName:_name] retain];
   }
}

-(NSArray *)names {
   return [NSArray arrayWithObject:_name];
}

-(NSString *)name {
   return _name;
}

-(NSArray *)addresses {
   [self _resolveAddressesIfNeeded];
   return _addresses;
}

-(NSString *)address {
   return [[self addresses] lastObject];
}


- (NSString *)description
{
   return [NSString stringWithFormat:@"<%@[0x%lx] name: %@ addresses: %@>",
     [[self class] description], self, _name, [self addresses]];
}

@end
