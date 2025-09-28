/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>

@interface NSWindowAnimationContext : NSObject {
    id _window;
    NSRect _targetRect;
    NSRect _stepRect;
    int _stepCount;
    NSTimeInterval _stepInterval;
    BOOL _display;
}

+ (NSWindowAnimationContext *)contextToTransformWindow:(id)window startRect:(NSRect)startRect targetRect:(NSRect)targetRect resizeTime:(NSTimeInterval)resizeTime display:(BOOL)display;

- (id)initWithWindow:(id)window targetRect:(NSRect)targetRect stepRect:(NSRect)stepRect stepCount:(int)stepCount stepInterval:(NSTimeInterval)stepInterval display:(BOOL)display;

- (id)window;

- (NSRect)targetRect;
- (NSRect)stepRect;

- (int)stepCount;
- (NSTimeInterval)stepInterval;

- (BOOL)display;

- (void)decrement;

@end

@interface NSObject (NSWindowAnimationContext)
- (NSRect)frame;
- (void)setFrame:(NSRect)frame display:(BOOL)display;
@end
