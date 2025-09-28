/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSColorWell.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSColorPanel.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSPasteboard.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSDragging.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSGraphicsStyle.h>
#import <AppKit/NSController.h>
#import <AppKit/NSObject+BindingSupport.h>
#import <Foundation/NSKeyValueObserving.h>
#import <AppKit/NSRaise.h>

@implementation NSColorWell

+(void)initialize
{
	[self setKeys:[NSArray arrayWithObjects:@"color", @"something", nil]
 triggerChangeNotificationsForDependentKey:@"value"];
}

-(id)_replacementKeyPathForBinding:(id)binding
{
	if([binding isEqual:@"value"])
		return @"color";
   return [super _replacementKeyPathForBinding:binding];
}

// private
NSString *_NSColorWellDidBecomeExclusiveNotification=@"_NSColorWellDidBecomeExclusiveNotification";

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
    _isEnabled=[keyed decodeBoolForKey:@"NSEnabled"];
    _isContinuous=![keyed decodeBoolForKey:@"NSIsNotContinuous"];
    _isBordered=[keyed decodeBoolForKey:@"NSIsBordered"];
    _color=[[keyed decodeObjectForKey:@"NSColor"] retain];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }
   return self;
}

-initWithFrame:(NSRect)frame {
    [super initWithFrame:frame];
    _isEnabled=YES;
    _isContinuous=YES;
    _isBordered=YES;
    _color=[[NSColor whiteColor] copy];
    
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(colorPanelWillClose:)
                                                     name:NSWindowWillCloseNotification
                                                   object:[NSColorPanel sharedColorPanel]];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(colorWellDidBecomeExclusive:)
                                                     name:_NSColorWellDidBecomeExclusiveNotification
                                                   object:nil];

   [self registerForDraggedTypes:[NSArray arrayWithObject:NSColorPboardType]];

    return self;
}

-(void)awakeFromNib {
// this should be moved the nib initWithCoder:
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(colorPanelWillClose:)
                                                 name:NSWindowWillCloseNotification
                                               object:[NSColorPanel sharedColorPanel]];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(colorWellDidBecomeExclusive:)
                                                 name:_NSColorWellDidBecomeExclusiveNotification
                                               object:nil];
   [self registerForDraggedTypes:[NSArray arrayWithObject:NSColorPboardType]];
}

-(void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [_color release];
    [super dealloc];
}

-(void)colorWellDidBecomeExclusive:(NSNotification *)note {
    if ([note object] != self)
        [self deactivate];
}

-(void)colorPanelWillClose:(NSNotification *)note {
    if ([self isActive])
        [self deactivate];
}

-(id)target {
    return _target;
}

-(void)setTarget:(id)target {
    _target = target;
}

-(SEL)action {
    return _action;
}

-(void)setAction:(SEL)action {
    _action = action;
}

-(BOOL)isEnabled {
   return _isEnabled;
}

-(void)setEnabled:(BOOL)flag {
   _isEnabled=flag;
   [self setNeedsDisplay:YES];
}

-(NSColor *)color {
   return _color;
}

-(BOOL)isBordered {
   return _isBordered;
}

-(BOOL)isActive {
    return _isActive && [self isEnabled];
}

-(void)setColor:(NSColor *)color {
	if(NSIsControllerMarker(color))
		return [self setColor:[NSColor blackColor]];

	if ([_color isEqual: color]) {
		return;
	}
	
   color=[color retain];
   [_color release];
   _color=color;

	if ([self isActive] && color != nil) {
		// Pass it on
		_notifyingColorPanel = YES;
		[[NSColorPanel sharedColorPanel] setColor: color];
		_notifyingColorPanel = NO;
	}
   [self setNeedsDisplay:YES];
}

-(void)setBordered:(BOOL)flag {
   _isBordered = flag;
}

-(void)activate:(BOOL)exclusive {

    if (_isActive == YES) {
		return;
	}
	
	if (exclusive) {
		[[NSNotificationQueue defaultQueue] enqueueNotification:[NSNotification notificationWithName:_NSColorWellDidBecomeExclusiveNotification object:self] postingStyle:NSPostNow coalesceMask:NSNotificationCoalescingOnName forModes:nil];
	}
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(changeColorWhenActive:)
												 name:NSColorPanelColorDidChangeNotification
											   object:[NSColorPanel sharedColorPanel]];

	// Update the color panel with our color
	[[NSColorPanel sharedColorPanel] setColor: [self color]];
	
	[NSApp orderFrontColorPanel: self];
		
    _isActive = YES;

	[self setNeedsDisplay:YES];
}

-(void)deactivate {
    if (_isActive == NO)
        return;
    
    _isActive = NO;
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:NSColorPanelColorDidChangeNotification
                                                  object:[NSColorPanel sharedColorPanel]];
    [self setNeedsDisplay:YES];
}

-(void)changeColorWhenActive:(NSNotification *)note {
	if (_notifyingColorPanel == NO) {
	   [self setColor:[[note object] color]];
	   [self sendAction:_action to:_target];
	}
}

-(BOOL)isOpaque {
   return YES;
}

-(void)drawWellInside:(NSRect)rect {
    [_color drawSwatchInRect:rect];
}

-(void)drawRect:(NSRect)rect {
   rect=_bounds;

   rect=[[self graphicsStyle] drawColorWellBorderInRect:rect enabled:[self isEnabled] bordered:[self isBordered] active:[self isActive]];

   [self drawWellInside:rect];
}

-(void)takeColorFrom:sender {
   [self setColor:[sender color]];
}

-(void)mouseDown:(NSEvent *)event
{
	
	if(![self isEnabled]) {
		return;
	}
	
	if ([self isBordered]) {
		/*
		 * Bordered color wells have interesting logic:

		   o If the user clicks and drags in the swatch then they
				can drag a color out - and the well becomes disabled
		 
		   o If the user clicks on the border and drags then the control
				activates or not depending on whether the mouse is within the
				control or not
		 
		   o If the user simply clicks in the swatch or the border
				the well is toggled between active and inactive states
		 */
		BOOL    wasActive=[self isActive];
		NSPoint point=[self convertPoint:[event locationInWindow] fromView:nil];
		BOOL    mouseInBorder=!NSMouseInRect(point,NSInsetRect(_bounds,8,8),[self isFlipped]);
		BOOL canStartDrag = !mouseInBorder;
		if (mouseInBorder) {
			// Toggle the initial state
			if (wasActive)
				[self deactivate];
			else
				[self activate: YES];
			wasActive = !wasActive;
		}
		
		BOOL shouldStartDrag = NO;
		do {            
			event=[[self window] nextEventMatchingMask:NSLeftMouseUpMask|NSLeftMouseDraggedMask];
			point=[self convertPoint:[event locationInWindow] fromView:nil];
			BOOL mouseInBounds=NSMouseInRect(point,_bounds,[self isFlipped]);
			if ([event type] == NSLeftMouseDragged) {
				if (canStartDrag) {
					// Get dragging the color
					shouldStartDrag = YES;
				} else{
					// Toggle the state based on where the cursor is
					if (mouseInBounds) {
						if (wasActive) {
							[self activate: YES];
						} else {
							[self deactivate];
						}
					} else {
						if (wasActive) {
							[self deactivate];
						} else {
							[self activate: YES];
						}
					}
				}
			} else if (mouseInBounds) {
				// Just toggle the state
				if (wasActive) {
					[self deactivate];
				} else {
					[self activate: YES];
				}
			}
		} while ([event type] != NSLeftMouseUp && shouldStartDrag == NO);
		
		if (shouldStartDrag == NO) {
			if ([self isActive] == YES) {
				if (!([event modifierFlags] & NSShiftKeyMask)) {
					[self activate:YES];
				}
				[[NSColorPanel sharedColorPanel] setColor:[self color]];
				[NSApp orderFrontColorPanel:self];
			}
			return;
		} else {
			// We're going to drag a swatch so deactivate (like Cocoa)
			[self deactivate];
		}
	}
	
	[NSColorPanel dragColor:_color withEvent:event fromView:self];
}

-(unsigned)draggingSourceOperationMaskForLocal:(BOOL)isLocal {
   return NSDragOperationCopy;
}

-(unsigned)draggingEntered:(id <NSDraggingInfo>)sender {
   return NSDragOperationCopy;
}

-(unsigned)draggingUpdated:(id <NSDraggingInfo>)sender {
   return NSDragOperationCopy;
}

-(BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender {
   return YES;
}

-(BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
   NSPasteboard *pboard=[sender draggingPasteboard];
   NSColor      *color=[NSColor colorFromPasteboard:pboard];

   [self setColor:color];
   [self sendAction:_action to:_target];

   return YES;
}


@end
