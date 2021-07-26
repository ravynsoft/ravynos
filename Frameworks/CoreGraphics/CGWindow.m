/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreGraphics/CGWindow.h>
#import <Onyx2D/O2Exceptions.h>

@implementation CGWindow

-(void)setDelegate:delegate {
   O2InvalidAbstractInvocation();
}

-delegate {
   O2InvalidAbstractInvocation();
   return nil;
}

-(void)invalidate {
   O2InvalidAbstractInvocation();
}


-(O2Context *)cgContext {
   O2InvalidAbstractInvocation();
   return nil;
}

-(unsigned)styleMask {
   O2InvalidAbstractInvocation();
   return 0;
}

-(void)setLevel:(int)value {
   O2InvalidAbstractInvocation();
}

-(void)setStyleMask:(unsigned)mask {
   O2InvalidAbstractInvocation();
}

-(void)setTitle:(NSString *)title {
   O2InvalidAbstractInvocation();
}

-(void)setFrame:(CGRect)frame {
   O2InvalidAbstractInvocation();
}

-(void)setOpaque:(BOOL)value {
   O2InvalidAbstractInvocation();
}

-(void)setAlphaValue:(CGFloat)value {
   O2InvalidAbstractInvocation();
}

-(void)setHasShadow:(BOOL)value {
   O2InvalidAbstractInvocation();
}

-(void)sheetOrderFrontFromFrame:(NSRect)frame aboveWindow:(CGWindow *)aboveWindow {
   O2InvalidAbstractInvocation();
}

-(void)sheetOrderOutToFrame:(NSRect)frame {
   O2InvalidAbstractInvocation();
}

-(void)showWindowForAppActivation:(NSRect)frame {
   O2InvalidAbstractInvocation();
}

-(void)hideWindowForAppDeactivation:(NSRect)frame {
   O2InvalidAbstractInvocation();
}

-(void)showWindowWithoutActivation {
   O2InvalidAbstractInvocation();
}

-(void)hideWindow {
   O2InvalidAbstractInvocation();
}

// these suck
+windowWithWindowNumber:(int)windowNumber {
   return (id)windowNumber;
}

-(int)windowNumber {
   return (int)self;
}

-(void)placeAboveWindow:(int)other {
   O2InvalidAbstractInvocation();
}

-(void)placeBelowWindow:(int)other {
   O2InvalidAbstractInvocation();
}


-(void)makeKey {
   O2InvalidAbstractInvocation();
}

-(void)makeMain {
   O2InvalidAbstractInvocation();
}

-(void)captureEvents {
   O2InvalidAbstractInvocation();
}

-(void)miniaturize {
   O2InvalidAbstractInvocation();
}

-(void)deminiaturize {
   O2InvalidAbstractInvocation();
}

-(BOOL)isMiniaturized {
   O2InvalidAbstractInvocation();
   return NO;
}

-(void)disableFlushWindow {
}

-(void)enableFlushWindow {
}

-(void)flushBuffer {
   O2InvalidAbstractInvocation();
}


-(void)dirtyRect:(CGRect)rect
{
    // Default does nothing - that can be used for optimized redraws
}

-(NSPoint)mouseLocationOutsideOfEventStream {
   O2InvalidAbstractInvocation();
   return NSZeroPoint;
}


-(void)sendEvent:(CGEvent *)event {
   O2InvalidAbstractInvocation();
}

-(void)addEntriesToDeviceDictionary:(NSDictionary *)entries {
   O2InvalidAbstractInvocation();
}

-(void)flashWindow {
   O2InvalidAbstractInvocation();
}

-(void)addCGLContext:(CGLContextObj)cglContext {
}

-(void)removeCGLContext:(CGLContextObj)cglContext {
}

-(void)flushCGLContext:(CGLContextObj)cglContext {
}

@end
