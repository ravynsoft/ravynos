/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSStepper.h>
#import <AppKit/NSStepperCell.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSWindow.h>

@interface NSStepperCell(NSStepperCell_private)
-(NSImage *)_stepUpImage;
-(NSImage *)_stepDownImage;
@end

@implementation NSStepper

+(Class)cellClass {
    return [NSStepperCell class];
}

-(double)minValue {
    return [_cell minValue];
}

-(double)increment {
    return [_cell increment];
}

-(double)maxValue {
    return [_cell maxValue];
}

-(BOOL)autorepeat {
    return [_cell autorepeat];
}

-(BOOL)valueWraps {
    return [_cell valueWraps];
}

-(void)setMinValue:(double)value {
    [_cell setMinValue:value];
}

-(void)setIncrement:(double)value {
    [_cell setIncrement:value];
}

-(void)setMaxValue:(double)value {
    [_cell setMaxValue:value];
}

-(void)setAutorepeat:(BOOL)flag {
    [_cell setAutorepeat:flag];
}

-(void)setValueWraps:(BOOL)flag {
    [_cell setValueWraps:flag];
}

-(void)sizeToFit {
    NSSize size=_bounds.size;
    NSSize size1=[[_cell _stepUpImage] size];
    NSSize size2=[[_cell _stepDownImage] size];

    size.width=MAX(size1.width,size2.width);
    size.width+=12;
    size.height=size1.height+size.height;
    size.height+=4;
    [self setBoundsSize:size];
}

-(void)mouseDown:(NSEvent *)event {
    BOOL sendAction=NO;
    
    if (![self isEnabled])
        return;

    [self lockFocus];
    sendAction=[_cell trackMouse:event inRect:[self bounds] ofView:self untilMouseUp:YES];
    [self unlockFocus];

    if (sendAction == YES)
        [self sendAction:[self action] to:[self target]];

    [self setNeedsDisplay:YES];
}

- (void)keyDown:(NSEvent *)event {
    [self interpretKeyEvents:[NSArray arrayWithObject:event]];
}

- (void)moveUp:(id)sender {
    [self lockFocus];
    [[self selectedCell] moveUp:sender];
    [self unlockFocus];
    
    [self setNeedsDisplay:YES];
}

- (void)moveDown:(id)sender {
    [self lockFocus];
    [[self selectedCell] moveDown:sender];
    [self unlockFocus];
    
    [self setNeedsDisplay:YES];
}

@end
