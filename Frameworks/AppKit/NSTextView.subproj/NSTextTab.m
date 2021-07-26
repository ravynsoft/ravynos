/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <Foundation/NSKeyedArchiver.h>

#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSRaise.h>
#import <AppKit/NSTextTab.h>

NSString *NSTabColumnTerminatorsAttributeName = @"NSTabColumnTerminatorsAttributeName";

@implementation NSTextTab

- (id)initWithTextAlignment:(NSTextAlignment)alignment location:(CGFloat)location options:(NSDictionary *)options
{
    NSTextTabType type = NSLeftTabStopType;
    switch (alignment) {
        case NSLeftTextAlignment:
        case NSJustifiedTextAlignment:
            type = NSLeftTabStopType;
            break;
        
        case NSRightTextAlignment:
            type = NSRightTabStopType;
            break;
            
        case NSCenterTextAlignment:
            type = NSCenterTabStopType;
            break;
    
        case NSNaturalTextAlignment:
            if ([[NSParagraphStyle defaultParagraphStyle] baseWritingDirection] == NSWritingDirectionRightToLeft) {
                type = NSRightTabStopType;
            }
            break;
    }
    return [self initWithType: type location: location];
}

-initWithType:(NSTextTabType)type location:(float)location {
   _type=type;
   _location=location;
   return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
	if ((self = [super init])) {
		if ([aDecoder isKindOfClass: [NSKeyedUnarchiver class]]) {
			_type = [aDecoder decodeIntForKey: @"Type"];
			_location = [aDecoder decodeFloatForKey: @"Location"];
		} else {
			NSUnimplementedMethod();
			[self release];
			self = nil;
		}
	}
	return self;
}

- (void)encodeWithCoder:(NSCoder *)aCoder
{
	if ([aCoder isKindOfClass: [NSKeyedArchiver class]]) {
		[aCoder encodeInt: _type forKey: @"Type"];
		[aCoder encodeFloat: _location forKey: @"Location"];
	} else {
		NSUnimplementedMethod();
	}
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

- (NSTextAlignment)alignment {
    
    NSTextAlignment alignment = NSLeftTextAlignment;

    switch (_type) {
        case NSLeftTabStopType:
            alignment = NSLeftTextAlignment;
            break;
        case NSRightTabStopType:
            alignment = NSRightTextAlignment;
            break;
        case NSCenterTabStopType:
            alignment = NSCenterTextAlignment;
            break;
        case NSDecimalTabStopType:
            alignment = NSRightTextAlignment;
            break;
    }
    
    return alignment;
}

-(NSDictionary *)options {
    return [NSDictionary dictionary];
}

-(NSTextTabType)tabStopType {
   return _type;
}

-(float)location {
   return _location;
}

- (BOOL)isEqual:(id)object
{
    if (self == object)  {
        return YES;
    }
    if (![object isKindOfClass:[NSTextTab class]]) {
        return NO;
    }
    NSTextTab *other = (NSTextTab *)object;
    return self.location == other.location && self.tabStopType == other.tabStopType;
}
@end
