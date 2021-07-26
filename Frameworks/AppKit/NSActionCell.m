/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSActionCell.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSControl.h>

@implementation NSActionCell
@class NSControl;

-(void)encodeWithCoder:(NSCoder *)coder {
   [super encodeWithCoder:coder];
   [coder encodeInt:_tag forKey:@"NSActionCell tag"];
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];
   
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
 
    _tag=[keyed decodeIntForKey:@"NSTag"];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"%@ can not initWithCoder:%@",isa,[coder class]];
   }
   return self;
}

-(NSView *)controlView {
   return _controlView;
}

-target {
   return _target;
}

-(SEL)action {
   return _action;
}

-(int)tag {
   return _tag;
}


-(void)setControlView:(NSView *)value {
   _controlView=value;
}

-(void)setTarget:target {
   _target=target;
}

-(void)setAction:(SEL)action {
   _action=action;
}

-(void)setTag:(int)tag {
   _tag=tag;
}

- (void)_validateEditing
{
    if ([_controlView isKindOfClass: [NSControl class]]) {
        [(NSControl *)_controlView validateEditing];
    }
}

// Cocoa is validating the editing everytime the ask is being asked - let's do the same
// That's what will ensure the values are always properly formatted by the current formatter (if any)
/**
 * Retrieve the value of the receiver
 */
- (id) objectValue
{
    [self _validateEditing];
    return [super objectValue];
}

/**
 * Retrieve the value of the receiver as an NSAttributedString.
 */
- (NSAttributedString*) attributedStringValue
{
    [self _validateEditing];
    return [super attributedStringValue];
}

/**
 * Retrieve the value of the receiver as an NSString.
 */
- (NSString *) stringValue
{
    [self _validateEditing];
    return [super stringValue];
}

/**
 * Retrieve the value of the receiver as a double.
 */
- (double) doubleValue
{
    [self _validateEditing];
    return [super doubleValue];
}

/**
 * Retrieve the value of the receiver as a float.
 */
- (float) floatValue
{
    [self _validateEditing];
    return [super floatValue];
}

/**
 * Retrieve the value of the receiver as an int.
 */
- (int) intValue
{
    [self _validateEditing];
    return [super intValue];
}

/**
 * Retrieve the value of the receiver as an NSInteger.
 */
- (NSInteger) integerValue
{
    [self _validateEditing];
    return [super integerValue];
}

@end
