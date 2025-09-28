/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSButton.h>
#import <AppKit/NSButtonCell.h>
#import <AppKit/NSController.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSRaise.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSFontManager.h>

@implementation NSButton

+(Class)cellClass {
   return [NSButtonCell class];
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];
   
   if ([[self cell] controlSize] == NSMiniControlSize) {
    NSRect frame = [self frame];
    if (frame.size.height < 18) {
     frame.size.height = 18;
     frame.origin.y -= 1;
     [self setFrame:frame];
    }
   }
   
   return self;
}

-(BOOL)resignFirstResponder {
   [self setNeedsDisplay:YES];
   return [super resignFirstResponder];
}

#if 0
// This results in no button being drawn if there's no image.
-(BOOL)isFlipped {
   return YES;
}
#endif
-(BOOL)isOpaque {
   return [_cell isOpaque];
}

-(BOOL)isTransparent {
   return [_cell isTransparent];
}

-(NSString *)keyEquivalent {
   return [_cell keyEquivalent];
}

-(NSUInteger)keyEquivalentModifierMask {
   return [_cell keyEquivalentModifierMask];
}

-(NSImage *)image {
   return [_cell image];
}

-(NSCellImagePosition)imagePosition {
   return [_cell imagePosition];
}

-(NSString *)title {
   return [_cell title];
}

-(NSInteger)state {
   return [_cell state];
}

-(BOOL)allowsMixedState {
   return [_cell allowsMixedState];
}

-(NSSound *)sound {
   return [_cell sound];
}

-(NSBezelStyle)bezelStyle {
   return [_cell bezelStyle];
}

-(NSString *)alternateTitle {
   return [_cell alternateTitle];
}

-(NSImage *)alternateImage {
   return [_cell alternateImage];
}

-(NSAttributedString *)attributedTitle {
   return [_cell attributedTitle];
}

-(NSAttributedString *)attributedAlternateTitle {
   return [_cell attributedAlternateTitle];
}

-(BOOL)showsBorderOnlyWhileMouseInside {
   return [_cell showsBorderOnlyWhileMouseInside];
}

-(void)getPeriodicDelay:(float *)delay interval:(float *)interval {
   NSUnimplementedMethod();
}

-(void)setTransparent:(BOOL)value {
   [_cell setTransparent:value];
   [self setNeedsDisplay:YES];
}

-(void)setKeyEquivalent:(NSString *)value {
   [_cell setKeyEquivalent:value];
   [self setNeedsDisplay:YES];
}

-(void)setKeyEquivalentModifierMask:(NSUInteger)value {
   [_cell setKeyEquivalentModifierMask:value];
}

-(void)setImage:(NSImage *)value {
   [_cell setImage:value];
   [self setNeedsDisplay:YES];
}

-(void)setImagePosition:(NSCellImagePosition)value {
   [_cell setImagePosition:value];
   [self setNeedsDisplay:YES];
}

-(void)setTitle:(NSString *)value {
   [_cell setTitle:value];
   [self setNeedsDisplay:YES];
}

-(void)setState:(NSInteger)value {
   [_cell setState:value];
   [self setNeedsDisplay:YES];
}

-(void)setNextState {
   [_cell setNextState];
   [self setNeedsDisplay:YES];
}

-(void)setAllowsMixedState:(BOOL)flag {
   [_cell setAllowsMixedState:flag];
}

-(void)setSound:(NSSound *)sound {
   [_cell setSound:sound];
}

-(void)setBezelStyle:(NSBezelStyle)value {
   [_cell setBezelStyle:value];
   [self setNeedsDisplay:YES];
}

-(void)setAlternateTitle:(NSString *)value {
   [_cell setAlternateTitle:value];
   [self setNeedsDisplay:YES];
}

-(void)setAlternateImage:(NSImage *)value {
   [_cell setAlternateImage:value];
   [self setNeedsDisplay:YES];
}

-(void)setAttributedTitle:(NSAttributedString *)value {
   [_cell setAttributedTitle:value];
   [self setNeedsDisplay:YES];
}

-(void)setAttributedAlternateTitle:(NSAttributedString *)value {
   [_cell setAttributedAlternateTitle:value];
   [self setNeedsDisplay:YES];
}

-(void)setShowsBorderOnlyWhileMouseInside:(BOOL)value {
   [_cell setShowsBorderOnlyWhileMouseInside:value];
   [self setNeedsDisplay:YES];
}

-(void)setButtonType:(NSButtonType)value {
   [_cell setButtonType:value];
   [self setNeedsDisplay:YES];
}

-(void)setTitleWithMnemonic:(NSString *)value {
   NSUnimplementedMethod();
}

-(void)setPeriodicDelay:(float)delay interval:(float)interval {
   NSUnimplementedMethod();
}

-(void)highlight:(BOOL)value {
   [self lockFocus];
   [_cell highlight:value withFrame:[self bounds] inView:self];
   [self unlockFocus];
   [[self window] flushWindow];
}

-(BOOL)performKeyEquivalent:(NSEvent *)event {
   NSString *characters=[event charactersIgnoringModifiers];
   unsigned  modifiers=[event modifierFlags];

   if(![self isEnabled])
    return NO;

   if((modifiers&(NSCommandKeyMask|NSAlternateKeyMask))==([self keyEquivalentModifierMask]&(NSCommandKeyMask|NSAlternateKeyMask))){
    NSString *key=[self keyEquivalent];

    if([key isEqualToString:characters]){
     [self performClick:nil];
     return YES;
    }
   }

   return NO;
}

-(void)performClick:sender {
   [self highlight:YES];

   [[self cell] setState:[[self cell] nextState]];

   [self sendAction:[self action] to:[self target]];

   [self highlight:NO];
}

-(void)keyDown:(NSEvent *)event {
	// " " is a key equivalent to clicking
	if ([[event charactersIgnoringModifiers] isEqualToString:@" "]) {
		[self performClick:nil];
	} else {
		[self interpretKeyEvents:[NSArray arrayWithObject:event]];
	}
}

@end

@implementation NSButton (BindingSupport)

- (void)_setFontFamilyName:(NSString*)familyName
{
	// Bail if it's any of the objects we don't grok
	if (familyName == nil ||
		familyName == NSNoSelectionMarker ||
		familyName == NSMultipleValuesMarker ||
		familyName == NSNotApplicableMarker) {
		return;
	}
	
	NSFont* currentFont = [self font];
	CGFloat size = [currentFont pointSize];
	
	NSFont* newFont = [[NSFontManager sharedFontManager] fontWithFamily: familyName traits: 0 weight: 5 size: size];
	if (newFont != nil) {
		[self setFont: newFont];
	}
}

@end
