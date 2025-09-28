/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSError.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSBundle.h>

NSString * const NSPOSIXErrorDomain=@"NSPOSIXErrorDomain";
NSString * const NSOSStatusErrorDomain=@"NSOSStatusErrorDomain";
NSString * const NSWINSOCKErrorDomain=@"NSWINSOCKErrorDomain";
NSString * const NSWin32ErrorDomain=@"NSWin32ErrorDomain";
NSString * const NSCocoaErrorDomain=@"NSCocoaErrorDomain";

NSString * const NSUnderlyingErrorKey=@"NSUnderlyingErrorKey";
NSString * const NSLocalizedDescriptionKey=@"NSLocalizedDescriptionKey";
NSString * const NSLocalizedFailureReasonErrorKey=@"NSLocalizedFailureReasonErrorKey";
NSString * const NSLocalizedRecoveryOptionsErrorKey=@"NSLocalizedRecoveryOptionsErrorKey";
NSString * const NSLocalizedRecoverySuggestionErrorKey=@"NSLocalizedRecoverySuggestionErrorKey";
NSString * const NSRecoveryAttempterErrorKey=@"NSRecoveryAttempterErrorKey";

@implementation NSError

-initWithDomain:(NSString *)domain code:(NSInteger)code userInfo:(NSDictionary *)userInfo {
   _domain=[domain copy];
   _code=code;
   _userInfo=[userInfo retain];
   return self;
}

-(void)dealloc {
   [_domain release];
   [_userInfo release];
   [super dealloc];
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
   return self;
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

+errorWithDomain:(NSString *)domain code:(NSInteger)code userInfo:(NSDictionary *)userInfo {
   return [[[self alloc] initWithDomain:domain code:code userInfo:userInfo] autorelease];
}

-(NSString *)domain {
   return _domain;
}

-(NSInteger)code {
   return _code;
}

-(NSDictionary *)userInfo {
   return _userInfo;
}

-(NSString *)localizedDescription {
   NSString *localizedDescription;

   localizedDescription = [_userInfo objectForKey:NSLocalizedDescriptionKey];
   if (localizedDescription != nil)
      return localizedDescription;

    localizedDescription = [self localizedFailureReason];
    if (localizedDescription) {
        return [NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"The operation could not be completed.", @"Default NSError description"), localizedDescription];        
    } else {
        return [NSString stringWithFormat:@"%@ (%@ error %d)", NSLocalizedString(@"The operation could not be completed.", @"Default NSError description"), _domain, _code];
    }
}

-(NSString *)localizedFailureReason {
   return [_userInfo objectForKey:NSLocalizedFailureReasonErrorKey];
}

-(NSArray *)localizedRecoveryOptions {
   return [_userInfo objectForKey:NSLocalizedRecoveryOptionsErrorKey];
}

-(NSString *)localizedRecoverySuggestion {
   return [_userInfo objectForKey:NSLocalizedRecoverySuggestionErrorKey];
}

-(id)recoveryAttempter {
   return [_userInfo objectForKey:NSRecoveryAttempterErrorKey];
}

-(id)description {
   return [NSString stringWithFormat:@"Error Domain=%@ Code=%d UserInfo=%p %@", _domain, _code, _userInfo, [self localizedDescription]];
}

@end
