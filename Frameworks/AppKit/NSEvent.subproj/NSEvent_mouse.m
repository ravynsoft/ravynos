/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>
                 2010 Markus Hitter <mah@jump-ing.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSEvent_mouse.h>

@implementation NSEvent_mouse

-initWithType:(NSEventType)type location:(NSPoint)location modifierFlags:(unsigned)modifierFlags window:(NSWindow *)window clickCount:(int)clickCount deltaX:(float)deltaX deltaY:(float)deltaY {
	[super initWithType:type location:location modifierFlags:modifierFlags window:window];
	
	_clickCount = clickCount;
	_deltaX = deltaX;
	_deltaY = deltaY;
	
	return self;
}

- (int)clickCount {
	return _clickCount;
}

-initWithType:(NSEventType)type location:(NSPoint)location modifierFlags:(unsigned)modifierFlags window:(NSWindow *)window deltaY:(float)deltaY {
	[super initWithType:type location:location modifierFlags:modifierFlags window:window];
	
	_deltaY = deltaY;
	
	return self;
}

-initWithType:(NSEventType)type location:(NSPoint)location modifierFlags:(NSUInteger)modifierFlags timestamp:(NSTimeInterval)timestamp windowNumber:(NSInteger)windowNumber context:(NSGraphicsContext*)context eventNumber:(NSInteger)eventNumber trackingNumber:(NSInteger)tracking userData:(void *)userData {
   self=[super initWithType:type location:location modifierFlags:modifierFlags window:(id)windowNumber];
   if(self!=nil){
    _trackingNumber=tracking;
    _userData=userData;
   }
   return self;
}

-initWithType:(NSEventType)type location:(NSPoint)location modifierFlags:(NSUInteger)modifierFlags timestamp:(NSTimeInterval)timestamp windowNumber:(NSInteger)windowNumber context:(NSGraphicsContext*)context eventNumber:(NSInteger)eventNumber clickCount:(NSInteger)clickCount pressure:(float)pressure {
   [super initWithType:type location:location modifierFlags:modifierFlags window:(id)windowNumber];
   _clickCount=clickCount;
   return self;
}

- (float)deltaX {
	return _deltaX;
}

- (float)deltaY {
	return _deltaY;
}

-(float)deltaZ {
	return 0.0;
}

// Apple's documentation is a bit confusing here. NSEvent exposes no API to pass a
// NSTrackingArea object on creation, yet they expose API to get that pointer.
// Additionally, they state -trackingArea would deliver meaningful returns only
// if the area was created with the NSTrackingArea API.
//
// On the other hand, NSTrackingArea's docs explicitely state a pointer to a
// NSTrackingArea is now substitute for NSTrackingRectTags, which are in turn
// "trackingNumbers". See "Compatibility Issues", there.
//
// Accordingly, it's unclear wether there's a difference between -trackingArea and
// -trackingNumber at all (as of 10.6 docs). Let's assume they are the same.
-(NSTrackingArea *)trackingArea {
   return (NSTrackingArea *)_trackingNumber;
}

-(NSInteger)trackingNumber {
   return _trackingNumber;
}

-(void *)userData {
   if(_type!=NSMouseEntered && _type!=NSMouseExited){
    [NSException raise:NSInternalInconsistencyException format:@"-[%@ userData] not valid for type %d",isa,_type];
    return NULL;
   }
   
   return _userData;
}

-(void)setSerialNumber:(NSInteger)serial {
    _serial = serial;
}

-(NSInteger)serialNumber {
    return _serial;
}


@end
