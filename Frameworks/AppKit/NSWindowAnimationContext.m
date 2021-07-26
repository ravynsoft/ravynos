/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSWindowAnimationContext.h>
#import <AppKit/NSWindow.h>

#define FRAMES_PER_STEP     6

@implementation NSWindowAnimationContext

+ (NSWindowAnimationContext *)contextToTransformWindow:(id)window startRect:(NSRect)startRect targetRect:(NSRect)targetRect resizeTime:(NSTimeInterval)resizeTime display:(BOOL)display
{
    NSRect deltaRect, stepRect;
    float totalPixels = 0;
    int stepCount = 0;
    
    deltaRect.origin.x = targetRect.origin.x - startRect.origin.x;
    deltaRect.origin.y = targetRect.origin.y - startRect.origin.y;
    deltaRect.size.width = targetRect.size.width - startRect.size.width;
    deltaRect.size.height = targetRect.size.height - startRect.size.height;
    
    totalPixels = (fabs(deltaRect.origin.x) + fabs(deltaRect.size.width) + 1) *
        (fabs(deltaRect.origin.y) + fabs(deltaRect.size.height + 1));
    
    stepCount = floor(totalPixels / 150.0);
    stepCount *= FRAMES_PER_STEP;

    if (stepCount > 0) {
        stepRect.origin.x = deltaRect.origin.x / stepCount;
        stepRect.origin.y = deltaRect.origin.y / stepCount;
        stepRect.size.width = deltaRect.size.width / stepCount;
        stepRect.size.height = deltaRect.size.height / stepCount;
    }
    
    return [[[NSWindowAnimationContext alloc] initWithWindow:window targetRect:targetRect stepRect:stepRect stepCount:stepCount stepInterval:(resizeTime / FRAMES_PER_STEP) display:display] autorelease];
}

- (id)initWithWindow:(id)window targetRect:(NSRect)targetRect stepRect:(NSRect)stepRect stepCount:(int)stepCount stepInterval:(NSTimeInterval)stepInterval display:(BOOL)display
{
    _window = [window retain];
    _targetRect = targetRect;
    _stepRect = stepRect;
    _stepCount = stepCount;
    _stepInterval = stepInterval;
    _display = display;
    
    return self;
}

- (void)dealloc
{
    [_window release];
    
    [super dealloc];
}

- (id)window
{
    return _window;
}

- (NSRect)targetRect
{
    return _targetRect;
}

- (NSRect)stepRect
{
    return _stepRect;
}

- (int)stepCount
{
    return _stepCount;
}

- (NSTimeInterval)stepInterval
{
    return _stepInterval;
}

- (BOOL)display
{
    return _display;
}

- (void)decrement
{
    _stepCount--;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@[0x%lx] window: %@ targetRect: %@ stepRect: %@ stepCount: %d stepInterval: %g display: %d",
        [self class], self, _window, NSStringFromRect(_targetRect), NSStringFromRect(_stepRect), _stepCount, _stepInterval, _display];
}

@end
