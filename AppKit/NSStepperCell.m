/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSStepperCell.h>
#import <AppKit/NSStepper.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSButtonCell.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSGraphicsStyle.h>
#import <AppKit/NSMatrix.h>
#import <AppKit/NSRaise.h>

@implementation NSStepperCell
-(void)encodeWithCoder:(NSCoder *)coder {
	NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
	[super initWithCoder:coder];
	
	if([coder allowsKeyedCoding]){
		NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
		
		_minValue=[keyed decodeDoubleForKey:@"NSMinValue"];
		_maxValue=[keyed decodeDoubleForKey:@"NSMaxValue"];
		if ([keyed containsValueForKey: @"NSValue"]) {
			// This cell prefers NSValue to NSContents
			[_objectValue release];
			_objectValue = [[keyed decodeObjectForKey: @"NSValue"] retain];
		}
		_valueWraps=[keyed decodeBoolForKey:@"NSValueWraps"];
		_autorepeat=[keyed decodeBoolForKey:@"NSAutorepeat"];
		_increment=[keyed decodeDoubleForKey:@"NSIncrement"];
	}
	else {
		[NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
	}
	
	return self;
}

-(double)minValue {
    return _minValue;
}

-(double)increment {
    return _increment;
}

-(double)maxValue {
    return _maxValue;
}

-(BOOL)autorepeat {
    return _autorepeat;
}

-(BOOL)valueWraps {
    return _valueWraps;
}

-(void)setMinValue:(double)value {
    _minValue = value;
}

-(void)setIncrement:(double)value {
    _increment = value;
}

-(void)setMaxValue:(double)value {
    _maxValue = value;
}

-(void)setAutorepeat:(BOOL)flag {
    _autorepeat = flag;
}

-(void)setValueWraps:(BOOL)flag {
    _valueWraps = flag;
}

-(NSRect)_upArrowFrameForFrame:(NSRect)frame {
    int half = frame.size.height/2;
    if (![_controlView isFlipped])
        frame.origin.y += half;
    frame.size.height = half;
    return frame;
}

-(NSRect)_downArrowFrameForFrame:(NSRect)frame {
    int half = frame.size.height/2;
    frame.size.height = half;
    if ([_controlView isFlipped])
        frame.origin.y += half;
    return frame;
}

- (void)_incrementAndConstrainBy:(double)delta {
    double value = [self doubleValue];
    value += delta;
    if (value < _minValue) {
        if (_valueWraps)
            value = _maxValue;
        else
            value = _minValue;
    }
    if (value > _maxValue) {
        if (_valueWraps)
            value = _minValue;
        else
            value = _maxValue;
    }
    [self setDoubleValue:value];
}

// not so pretty
-(BOOL)trackMouse:(NSEvent *)event inRect:(NSRect)cellFrame ofView:(NSView *)controlView untilMouseUp:(BOOL)untilMouseUp {
    NSPoint point;
    double delta=0.0;
    NSRect trackFrame;
    BOOL sendAction=NO;

    if (_autorepeat && untilMouseUp)
        [NSEvent startPeriodicEventsAfterDelay:0.0 withPeriod:0.5];

    do {
        if ([event type] != NSPeriodic)
            point=[controlView convertPoint:[event locationInWindow] fromView:nil];

        if ([event type] == NSLeftMouseUp)
            untilMouseUp=NO;
        
        if (NSMouseInRect(point, [self _upArrowFrameForFrame:cellFrame],[controlView isFlipped])) {
            delta=[self increment];
            trackFrame=[self _upArrowFrameForFrame:cellFrame];
            _isUpHighlighted=YES;
            sendAction=YES;
        }
        else if (NSMouseInRect(point, [self _downArrowFrameForFrame:cellFrame],[controlView isFlipped])) {
            delta=0.0-[self increment];
            trackFrame=[self _downArrowFrameForFrame:cellFrame];
            _isDownHighlighted=YES;
            sendAction=YES;
        }
        else
            sendAction=NO;

        [self drawWithFrame:cellFrame inView:controlView];
        if (sendAction)
            [self _incrementAndConstrainBy:delta];
            
        if (untilMouseUp) {
            if (_autorepeat && sendAction)
                [(NSControl *)controlView sendAction:_action to:_target];
            
            event=[[controlView window] nextEventMatchingMask:NSPeriodicMask|NSLeftMouseUpMask|NSLeftMouseDraggedMask];
        }
        if ([event type] == NSLeftMouseUp)
            untilMouseUp = NO;

        _isUpHighlighted = _isDownHighlighted = NO;
    } while (untilMouseUp == YES);

    if (_autorepeat)
        [NSEvent stopPeriodicEvents];

    return sendAction;
}

// I don't believe there are < > horizontal steppers?
-(void)drawWithFrame:(NSRect)frame inView:(NSView *)controlView {
    BOOL drawDottedRect = NO;

    _controlView=controlView;
    [[_controlView graphicsStyle] drawStepperButtonInRect:[self _upArrowFrameForFrame:frame] clipRect:frame enabled:[self isEnabled] highlighted:_isUpHighlighted upNotDown:YES];
    [[_controlView graphicsStyle] drawStepperButtonInRect:[self _downArrowFrameForFrame:frame] clipRect:frame enabled:[self isEnabled] highlighted:_isDownHighlighted upNotDown:NO];
        
    if ([[controlView window] firstResponder] == controlView){
        if([controlView isKindOfClass:[NSMatrix class]]){
            NSMatrix *matrix=(NSMatrix *)controlView;

            drawDottedRect=([matrix keyCell]==self)?YES:NO;
        }
        else if([controlView isKindOfClass:[NSControl class]]){
            NSControl *control=(NSControl *)controlView;

            drawDottedRect=([control selectedCell]==self)?YES:NO;
        }
    }

    if(drawDottedRect)
        NSDottedFrameRect(NSInsetRect(frame,2,2)); // fugly
}

// *!* hmm. this is too fast! it's not visually discernible.
- (void)moveUp:(id)sender {
    [self _incrementAndConstrainBy:[self increment]];

    _isUpHighlighted=YES;
    [self drawWithFrame:[_controlView bounds] inView:_controlView];
    [[_controlView window] flushWindow];

    [(NSControl *)_controlView sendAction:[self action] to:[self target]];
    
    _isUpHighlighted=NO;
    [self drawWithFrame:[_controlView bounds] inView:_controlView];
    [[_controlView window] flushWindow];
}

- (void)moveDown:(id)sender {
    [self _incrementAndConstrainBy:0.0-[self increment]];

    _isDownHighlighted=YES;
    [self drawWithFrame:[_controlView bounds] inView:_controlView];
    [[_controlView window] flushWindow];

    [(NSControl *)_controlView sendAction:[self action] to:[self target]];

    _isDownHighlighted=NO;
    [self drawWithFrame:[_controlView bounds] inView:_controlView];
    [[_controlView window] flushWindow];
}

@end
