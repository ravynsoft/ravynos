/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSButtonCell.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSStringDrawing.h>
#import <AppKit/NSGraphicsContext.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSGraphicsStyle.h>
#import <AppKit/NSStringDrawer.h>
#import <AppKit/NSControl.h>
#import <AppKit/NSMatrix.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSButtonImageSource.h>
#import <AppKit/NSComboBoxCell.h>
#import <AppKit/NSPopUpButtonCell.h>
#import <AppKit/NSSound.h>
#import <AppKit/NSRaise.h>

@implementation NSButtonCell

// Margin between an image and the Button interior borders
static const float kImageMargin = 2.;

-(void)encodeWithCoder:(NSCoder *)coder {
   [super encodeWithCoder:coder];
   [coder encodeObject:_titleOrAttributedTitle forKey:@"NSButtonCell title"];
   [coder encodeObject:_alternateTitle forKey:@"NSButtonCell alternateTitle"];
   [coder encodeInt:_imagePosition forKey:@"NSButtonCell imagePosition"];
   [coder encodeInt:_highlightsBy forKey:@"NSButtonCell highlightsBy"];
   [coder encodeInt:_showsStateBy forKey:@"NSButtonCell showsStateBy"];
   [coder encodeBool:_isTransparent forKey:@"NSButtonCell transparent"];
   [coder encodeBool:_imageDimsWhenDisabled forKey:@"NSButtonCell imageDimsWhenDisabled"];
   [coder encodeObject:_alternateImage forKey:@"NSButtonCell alternateImage"];
   [coder encodeObject:_keyEquivalent forKey:@"NSButtonCell keyEquivalent"];
   [coder encodeInt:_keyEquivalentModifierMask forKey:@"NSButtonCell keyEquivalentModifierMask"];
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    unsigned           flags=[keyed decodeIntForKey:@"NSButtonFlags"];
    unsigned           flags2=[keyed decodeIntForKey:@"NSButtonFlags2"];
    id                 check;
    
    _titleOrAttributedTitle=[[keyed decodeObjectForKey:@"NSContents"] retain];
    _alternateTitle=[[keyed decodeObjectForKey:@"NSAlternateContents"] retain];
    
    _imagePosition=NSNoImage;
    if((flags&0x00480000)==0x00400000)
     _imagePosition=NSImageOnly;
    else if((flags&0x00480000)==0x00480000)
     _imagePosition=NSImageOverlaps;
    else if((flags&0x00380000)==0x00380000)
     _imagePosition=NSImageLeft;
    else if((flags&0x00380000)==0x00280000)
     _imagePosition=NSImageRight;
    else if((flags&0x00380000)==0x00180000)
     _imagePosition=NSImageBelow;
    else if((flags&0x00380000)==0x00080000)
     _imagePosition=NSImageAbove;

    //  bits 6 and 7 in flags2, but not in order
    switch((flags2>>6)&0x3){

     case 0:
      _imageScaling=NSImageScaleNone;
      break;

     case 1:
      _imageScaling=NSImageScaleProportionallyUpOrDown;
      break;

     case 2:
      _imageScaling=NSImageScaleProportionallyDown;
      break;

     case 3:
      _imageScaling=NSImageScaleAxesIndependently;
      break;
    }
        
    _highlightsBy=0;
    _showsStateBy=0;
    
    if(flags&0x80000000)
     _highlightsBy|=NSPushInCellMask;
    if(flags&0x40000000)
     _showsStateBy|=NSContentsCellMask;
    if(flags&0x20000000)
     _showsStateBy|=NSChangeBackgroundCellMask;
    if(flags&0x10000000)
     _showsStateBy|=NSChangeGrayCellMask;
    if(flags&0x08000000)
     _highlightsBy|=NSContentsCellMask;
    if(flags&0x04000000)
     _highlightsBy|=NSChangeBackgroundCellMask;
    if(flags&0x02000000)
     _highlightsBy|=NSChangeGrayCellMask;
    
    _isBordered=(flags&0x00800000)?YES:NO; // err, this flag is in NSCell too

    _bezelStyle=(flags2&0x7)|(flags2&0x20>>2);
    
    if (_bezelStyle==0)  // this is how textured buttons are encoded by IB
     _bezelStyle=NSTexturedSquareBezelStyle;
    if (_bezelStyle==3)
     _bezelStyle=NSTexturedRoundedBezelStyle;
    if (_bezelStyle==4)
     _bezelStyle=NSRoundRectBezelStyle;
    
    _isTransparent=(flags&0x00008000)?YES:NO;
    _imageDimsWhenDisabled=(flags&0x00002000)?NO:YES;
    
    _showsBorderOnlyWhileMouseInside=(flags2&0x8)?YES:NO;

    check=[keyed decodeObjectForKey:@"NSAlternateImage"];
    if([check isKindOfClass:[NSImage class]])
     _alternateImage=[check retain];
    else if([check isKindOfClass:[NSButtonImageSource class]]){
     [_image release];
     _image=[[check normalImage] retain];
     _alternateImage=[[check alternateImage] retain];
    }
    else
     _alternateImage=nil;

/* _normalImage is a private ivar in Apple's AppKit. Third party library BGHUDAppKit uses it to 
   figure out what kind of standard button is being drawn. We emulate it for BGHUDAppKit. */
    _normalImage=[_image retain];
    
    _keyEquivalent=[[keyed decodeObjectForKey:@"NSKeyEquivalent"] retain];
    _keyEquivalentModifierMask=flags2>>8;
    [self setIntValue:_state];   // make the int value of NSButtonCell to be
                                 // in synch with the bare _state of NSCell
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"%@ can not initWithCoder:%@",isa,[coder class]];
   }
   return self;
}

-initTextCell:(NSString *)string {
   [super initTextCell:string];
   _titleOrAttributedTitle=[string copy];
   _alternateTitle=@"";
   _imagePosition=NSNoImage;
   _highlightsBy=NSPushInCellMask;
   _showsStateBy=0;
   _isTransparent=NO;
   _imageDimsWhenDisabled=NO;
   _alternateImage=nil;
   _keyEquivalent=@"";
   _keyEquivalentModifierMask=0;
   _showsBorderOnlyWhileMouseInside=NO;

   [self setBordered:YES];
   [self setBezeled:YES];
   [self setAlignment:NSCenterTextAlignment];
   [self setObjectValue:[NSNumber numberWithBool:NO]];
   
   return self;
}

-initImageCell:(NSImage *)image {
   [super initImageCell:image];
   _titleOrAttributedTitle=@""; // empty string, not nil
   _imagePosition=NSImageOnly;
   [self setObjectValue:[NSNumber numberWithBool:NO]];
   
   return self;
}

-init {
   return [self initTextCell:@"Button"];
}

-(void)dealloc {
   [_titleOrAttributedTitle release];
   [_normalImage release];
   [_alternateTitle release];
   [_alternateImage release];
   [_keyEquivalent release];
   [_sound release];
   [_keyEquivalentFont release];
   [_backgroundColor release];
   [super dealloc];
}

-copyWithZone:(NSZone *)zone {
   NSButtonCell *result=[super copyWithZone:zone];

   result->_titleOrAttributedTitle=[_titleOrAttributedTitle copy];
   result->_alternateTitle =[_alternateTitle copy];
   result->_alternateImage=[_alternateImage retain];
   result->_keyEquivalent=[_keyEquivalent copy];
   result->_sound=[_sound retain];
   result->_keyEquivalentFont=[_keyEquivalentFont retain];
   result->_backgroundColor=[_backgroundColor retain];
   
   return result;
}

-(BOOL)isTransparent {
   return _isTransparent;
}

-(NSString *)keyEquivalent {
   return _keyEquivalent;
}

-(NSCellImagePosition)imagePosition {
   return _imagePosition;
}

-(NSString *)title {
   if([_titleOrAttributedTitle isKindOfClass:[NSAttributedString class]])
    return [_titleOrAttributedTitle string];
   else
    return _titleOrAttributedTitle;
}

-(NSString *)alternateTitle {
   return _alternateTitle;
}

-(NSImage *)alternateImage {
   return _alternateImage;
}

-(NSAttributedString *)attributedTitle {
   if([_titleOrAttributedTitle isKindOfClass:[NSAttributedString class]])
    return _titleOrAttributedTitle;
   else {
    NSMutableDictionary *attributes=[NSMutableDictionary dictionary];
    NSMutableParagraphStyle *paraStyle=[[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
    NSFont              *font=[self font];

    if(font!=nil)
     [attributes setObject:font forKey:NSFontAttributeName];

    [paraStyle setLineBreakMode:_lineBreakMode];
    [paraStyle setAlignment:_textAlignment];
    [attributes setObject:paraStyle forKey:NSParagraphStyleAttributeName];

    if([self isEnabled])
     [attributes setObject:[NSColor controlTextColor]
                   forKey:NSForegroundColorAttributeName];
    else
     [attributes setObject:[NSColor disabledControlTextColor]
                   forKey:NSForegroundColorAttributeName];

    return [[[NSAttributedString alloc] initWithString:[self title] attributes:attributes] autorelease];
   }
}

-(NSAttributedString *)attributedAlternateTitle {
   NSMutableDictionary *attributes=[NSMutableDictionary dictionary];
   NSMutableParagraphStyle *paraStyle=[[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
   NSFont              *font=[self font];

   if(font!=nil)
    [attributes setObject:font forKey:NSFontAttributeName];

   [paraStyle setLineBreakMode:_lineBreakMode];
   [paraStyle setAlignment:_textAlignment];
   [attributes setObject:paraStyle forKey:NSParagraphStyleAttributeName];

   if([self isEnabled])
    [attributes setObject:[NSColor controlTextColor]
                   forKey:NSForegroundColorAttributeName];
   else
    [attributes setObject:[NSColor disabledControlTextColor]
                   forKey:NSForegroundColorAttributeName];


   return [[[NSAttributedString alloc] initWithString:[self alternateTitle] attributes:attributes] autorelease];
}

-(int)highlightsBy {
   return _highlightsBy;
}

-(int)showsStateBy {
   return _showsStateBy;
}

-(BOOL)imageDimsWhenDisabled {
   return _imageDimsWhenDisabled;
}

-(unsigned)keyEquivalentModifierMask {
   return _keyEquivalentModifierMask;
}

-(NSBezelStyle)bezelStyle {
   return _bezelStyle;
}

-(BOOL)showsBorderOnlyWhileMouseInside {
   return _showsBorderOnlyWhileMouseInside;
}

-(NSSound *)sound {
   return _sound;
}

-(NSGradientType)gradientType {
   return _gradientType;
}

-(NSImageScaling)imageScaling {
   return _imageScaling;
}

-(BOOL)isOpaque {
   if(_bezelStyle==NSDisclosureBezelStyle)
    return NO;
   if(_bezelStyle==NSTexturedSquareBezelStyle)
    return NO;
   if(_bezelStyle==NSTexturedRoundedBezelStyle)
    return NO;
   if(_bezelStyle==NSShadowlessSquareBezelStyle)
    return NO;
   if(_bezelStyle==NSRecessedBezelStyle)
    return NO;
    
   return ![self isTransparent] && [self isBordered];
}

-(NSFont *)keyEquivalentFont {
   return _keyEquivalentFont;
}

-(NSColor *)backgroundColor {
   return _backgroundColor;
}

-(void)getPeriodicDelay:(float *)delay interval:(float *)interval {
   *delay=_periodicDelay;
   *interval=_periodicInterval;
}

-(int)state {
   return [self intValue];
}

-(void)setTransparent:(BOOL)flag {
   _isTransparent=flag;
}

-(void)setKeyEquivalent:(NSString *)keyEquivalent {
   keyEquivalent=[keyEquivalent copy];
   [_keyEquivalent release];
   _keyEquivalent=keyEquivalent;
}

-(void)setImagePosition:(NSCellImagePosition)position {
   _imagePosition=position;
}


-(void)setTitle:(NSString *)title {
   title=[title copy];
   [_titleOrAttributedTitle release];
   _titleOrAttributedTitle=title;
}

-(void)setAlternateTitle:(NSString *)title {
   title=[title copy];
   [_alternateTitle release];
   _alternateTitle=title;
}

-(void)setAlternateImage:(NSImage *)image {
   image=[image retain];
   [_alternateImage release];
   _alternateImage=image;
}

-(void)setAttributedTitle:(NSAttributedString *)title {
   title=[title copy];
   [_titleOrAttributedTitle release];
   _titleOrAttributedTitle=title;
}

-(void)setAttributedAlternateTitle:(NSAttributedString *)title {
   NSUnimplementedMethod();
}

-(void)setHighlightsBy:(int)type {
   _highlightsBy=type;
}

-(void)setShowsStateBy:(int)type {
   _showsStateBy=type;
}

-(void)setImageDimsWhenDisabled:(BOOL)flag {
   _imageDimsWhenDisabled=flag;
}

-(void)setKeyEquivalentModifierMask:(unsigned)mask {
   _keyEquivalentModifierMask=mask;
}

-(void)setState:(int)value {
   [self setIntValue:value];
}

-(void)setNextState {
   [self setIntValue:[self nextState]];
}

-(void)setObjectValue:(id <NSCopying>)value {
    if ([(id)value respondsToSelector:@selector(intValue)])
      [super setState:[(NSNumber *)value intValue]];
   else
      [super setState:0];

    [[self controlView] willChangeValueForKey:@"objectValue"];
   [_objectValue release];
   _objectValue = [[NSNumber numberWithInt:[super state]] retain];
   [[self controlView] didChangeValueForKey:@"objectValue"];

   if( [ [self controlView] respondsToSelector:@selector(updateCell:)] )
	[(NSControl *)[self controlView] updateCell:self];
}

-(void)setBezelStyle:(NSBezelStyle)bezelStyle {
   _bezelStyle = bezelStyle;
}

-(void)setButtonType:(NSButtonType)buttonType {
   switch (buttonType)
   {
      case NSMomentaryLightButton:
         _highlightsBy = NSChangeBackgroundCellMask;
	      _showsStateBy = NSNoCellMask;
         _imageDimsWhenDisabled = YES;
         break;

      case NSMomentaryPushInButton:
	      _highlightsBy = NSPushInCellMask|NSChangeGrayCellMask;
	      _showsStateBy = NSNoCellMask;
         _imageDimsWhenDisabled = YES;
         break;

      case NSMomentaryChangeButton:
	      _highlightsBy = NSContentsCellMask;
	      _showsStateBy = NSNoCellMask;
         _imageDimsWhenDisabled = YES;
         break;

      case NSPushOnPushOffButton:
	      _highlightsBy = NSPushInCellMask|NSChangeGrayCellMask;
	      _showsStateBy = NSChangeBackgroundCellMask;
         _imageDimsWhenDisabled = YES;
         break;

      case NSOnOffButton:
	      _highlightsBy = NSChangeBackgroundCellMask|NSChangeGrayCellMask;
	      _showsStateBy = NSChangeBackgroundCellMask|NSChangeGrayCellMask;
         _imageDimsWhenDisabled = YES;
         break;

      case NSToggleButton:
	      _highlightsBy = NSPushInCellMask|NSContentsCellMask;
	      _showsStateBy = NSContentsCellMask;
         _imageDimsWhenDisabled = YES;
         break;

      case NSSwitchButton:
	      _highlightsBy = NSContentsCellMask;
	      _showsStateBy = NSContentsCellMask;
         _imagePosition = NSImageLeft;
         _imageDimsWhenDisabled = NO;
	      [self setImage:[NSImage imageNamed:@"NSSwitch"]];
	      [self setAlternateImage:[NSImage imageNamed:@"NSHighlightedSwitch"]];
	      [self setAlignment:NSLeftTextAlignment];
	      [self setBordered:NO];
	      [self setBezeled:NO];
         break;

      case NSRadioButton:
	      _highlightsBy = NSContentsCellMask;
	      _showsStateBy = NSContentsCellMask;
         _imagePosition = NSImageLeft;
         _imageDimsWhenDisabled = NO;
	      [self setImage:[NSImage imageNamed:@"NSRadioButton"]];
	      [self setAlternateImage:[NSImage imageNamed:@"NSHighlightedRadioButton"]];
	      [self setAlignment:NSLeftTextAlignment];
	      [self setBordered:NO];
	      [self setBezeled:NO];
         break;
   }

   [(NSControl *)[self controlView] updateCell:self];
}

-(void)setShowsBorderOnlyWhileMouseInside:(BOOL)show {
   _showsBorderOnlyWhileMouseInside=show;
}

-(void)setSound:(NSSound *)sound {
   sound=[sound retain];
   [_sound release];
   _sound=sound;
}

-(void)setGradientType:(NSGradientType)value {
   _gradientType=value;
}

-(void)setBackgroundColor:(NSColor *)value {
   value=[value copy];
   [_backgroundColor release];
   _backgroundColor=value;
}

-(void)setImageScaling:(NSImageScaling)value {
   _imageScaling=value;
}

-(void)setKeyEquivalentFont:(NSFont *)value {
   value=[value retain];
   [_keyEquivalentFont release];
   _keyEquivalentFont=value;
}

-(void)setKeyEquivalentFont:(NSString *)value size:(CGFloat)size {
   NSFont *font=[NSFont fontWithName:value size:size];
   [self setKeyEquivalentFont:font];
}

-(void)setPeriodicDelay:(float)delay interval:(float)interval {
   _periodicDelay=delay;
   _periodicInterval=interval;
}

-(NSAttributedString *)titleForHighlight {
   if((([self highlightsBy]&NSContentsCellMask) && [self isHighlighted]) ||
      (([self showsStateBy]&NSContentsCellMask) && [self state])){
    NSAttributedString *result=[self attributedAlternateTitle];

    if([result length]>0)
     return result;
   }

   return [self attributedTitle];
}

-(NSImage *)imageForHighlight {
   if(_bezelStyle==NSDisclosureBezelStyle){
   
    if((([self highlightsBy]&NSContentsCellMask) && [self isHighlighted]))
     return [NSImage imageNamed:@"NSButtonCell_disclosure_highlighted"];
    else if([self state])
     return [NSImage imageNamed:@"NSButtonCell_disclosure_selected"];
    else
     return [NSImage imageNamed:@"NSButtonCell_disclosure_normal"];
     
    return nil;
   }
   else {
    if((([self highlightsBy]&NSContentsCellMask) && [self isHighlighted]) ||
       (([self showsStateBy]&NSContentsCellMask) && [self state]))
     return [self alternateImage];

    return [self image];
   }
}

-(NSRect)imageRectForBounds:(NSRect)rect {
    // Make sure we use the same image as will be drawn!
    NSImage *image = [self imageForHighlight];

    NSSize              imageSize= NSMakeSize(0,0);
    if (image != nil) {
        BOOL enabled = [self isEnabled] ? YES : ![self imageDimsWhenDisabled];
        BOOL mixed = [self state] == NSMixedState;
        imageSize = [[[self controlView] graphicsStyle] sizeOfButtonImage: image
                                                                  enabled: enabled
                                                                    mixed: mixed];
    }
    return NSMakeRect(rect.origin.x, rect.origin.y, imageSize.width, imageSize.height);
}

-(BOOL)isVisuallyHighlighted {
   return ((([self highlightsBy]&NSChangeGrayCellMask) && [self isHighlighted]) ||
           (([self showsStateBy]&NSChangeGrayCellMask) && [self state]));
}

-(NSRect)getControlSizeAdjustment: (BOOL)flipped
{
	/*
	Aqua Push Buttons actually have a frame much larger than told by IB to make room for shadows and whatnot
	So we have to compensate for this when drawing simpler buttons.
	There is probably a way to streamline this, make NSPopUpButtonCell draw itself for starters
	NSGraphicsStyle should probably do this adjustment too
	*/
	NSRect frame = { { 0, 0 }, { 0, 0 } };
	
	if ([self isKindOfClass:[NSComboBoxCell class]]) 
	{
		switch (_controlSize)
		{
			case NSRegularControlSize:
				frame.size.width  = 2;
				frame.size.height = 1;
				frame.origin.x    = 1;
				break;

			case NSSmallControlSize:
				frame.size.width  = 4;
				frame.size.height = 8;
				frame.origin.x    = 2;
				frame.origin.y    = 6;
				break;

			case NSMiniControlSize:
				frame.size.width  = 6;
				frame.size.height = 4;
				frame.origin.x    = 3;
				frame.origin.y    = 4;
				break;
		}
	}
	else if ([self isKindOfClass:[NSPopUpButtonCell class]]) 
	{
		switch (_controlSize)
		{
			case NSRegularControlSize:
				frame.size.width  = 2;
				frame.size.height = 1;
				frame.origin.x    = 1;
				break;

			case NSSmallControlSize:
				frame.size.width  = 4;
				frame.size.height = 3;
				frame.origin.x    = 2;
				frame.origin.y    = 3;
				break;

			case NSMiniControlSize:
				// Mini controls don't need adjusting they're small enough already.
				break;
		}
	}
	else if((_bezelStyle==NSRoundedBezelStyle) && (_highlightsBy&NSPushInCellMask) && (_highlightsBy&NSChangeGrayCellMask) && (_showsStateBy==NSNoCellMask)) 
	{
        switch (_controlSize) 
        {
            default:
                frame.size.width  = 10 - _controlSize*2;
                frame.size.height = 10 - _controlSize*2;
                frame.origin.x    =  5 - _controlSize;
                frame.origin.y    = flipped ? _controlSize*2 - 3 : 7 - _controlSize*2;
                break;
                
            case NSMiniControlSize:
                break;
        }
	}   
    else if(_bezelStyle==NSRegularSquareBezelStyle){

    }
    
	return frame;
}

- (void)_drawTexturedBezelWithFrame:(NSRect)frame
{
    BOOL highlighted=[self isHighlighted];
    BOOL pressed=[self state] && ([self showsStateBy] & NSChangeBackgroundCellMask);
    
    BOOL renderDarkenBg=NO, renderOutlineShadow=NO;
    //CGFloat topGray=0.76, bottomGray=0.98, strokeGray=0.4;
    CGFloat topGray=0.98, bottomGray=0.76, strokeGray=0.4;
    if (pressed) {
        topGray=0.4;
        bottomGray=0.30;
    }
    renderDarkenBg=highlighted;
    renderOutlineShadow=highlighted || pressed;
    
    CGContextRef ctx=[[NSGraphicsContext currentContext] graphicsPort];
    CGContextSaveGState(ctx);
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextSetFillColorSpace(ctx, colorSpace);
    CGContextSetStrokeColorSpace(ctx, colorSpace);
    CGColorSpaceRelease(colorSpace);
    
    frame = NSInsetRect(frame, 1.5, 1.5);
    const CGFloat rounding=1.5;
    
    const CGFloat baseY = floor(frame.origin.y);
    const CGFloat maxY = baseY + frame.size.height - 1.0;
    CGRect r = CGRectMake(floor(frame.origin.x), baseY, ceil(frame.size.width), 1.0);
    while (r.origin.y <= maxY) {
        CGFloat g = bottomGray + (topGray - bottomGray) * ((r.origin.y - baseY) / (maxY - baseY));
        CGFloat components[4] = { g, g, g, 1.0 };
        CGContextSetFillColor(ctx, components);
        
        if (r.origin.y < baseY+1.0f || r.origin.y > maxY-1.0f) {
            CGContextFillRect(ctx, CGRectMake(r.origin.x+1.0f, r.origin.y, r.size.width-2.0f, r.size.height));
        } else {
            CGContextFillRect(ctx, r);
        }
        r.origin.y += 1.0f;
    }
    
    const CGFloat lx = floor(frame.origin.x) + 0.5f;
    const CGFloat rx = floor(frame.origin.x + frame.size.width) + 0.5f;
    const CGFloat ty = floor(frame.origin.y) + 0.5f;
    const CGFloat by = floor(frame.origin.y + frame.size.height) + 0.5f;
    const CGFloat rlx = lx + rounding;
    const CGFloat rrx = rx - rounding;
    const CGFloat rty = ty + rounding;
    const CGFloat rby = by - rounding;
    CGContextBeginPath(ctx);
    CGContextMoveToPoint(ctx, rlx, ty);
    CGContextAddLineToPoint(ctx, rrx, ty);
    CGContextAddLineToPoint(ctx, rx, rty);
    CGContextAddLineToPoint(ctx, rx, rby);
    CGContextAddLineToPoint(ctx, rrx, by);
    CGContextAddLineToPoint(ctx, rlx, by);
    CGContextAddLineToPoint(ctx, lx, rby);
    CGContextAddLineToPoint(ctx, lx, rty);
    CGContextClosePath(ctx);
    
    CGFloat components[4] = { strokeGray, strokeGray, strokeGray, 1.0 };
    CGContextSetStrokeColor(ctx, components);
    CGContextSetLineWidth(ctx, 1.0);
    
    if (renderDarkenBg) {
        components[0] = components[1] = components[2] = 0.0;
        components[3] = 0.15;
        CGContextSetFillColor(ctx, components);
        CGContextDrawPath(ctx, kCGPathFillStroke);
    } else {
        CGContextStrokePath(ctx);
    }
    
    if (renderOutlineShadow) {  // a small interior shadow within the button's outline
        const CGFloat ins = 0.4f;
        CGContextBeginPath(ctx);
        CGContextMoveToPoint(ctx, lx+ins, ty+ins);
        CGContextAddLineToPoint(ctx, rx-ins, ty+ins);
        CGContextAddLineToPoint(ctx, rx-ins, by-ins);
        CGContextAddLineToPoint(ctx, lx+ins, by-ins);
        CGContextClosePath(ctx);
        
        components[0] = components[1] = components[2] = 0.0;
        components[3] = 0.3;
        CGContextSetStrokeColor(ctx, components);
        CGContextSetLineWidth(ctx, 0.9);
        CGContextStrokePath(ctx);
    }
    
    CGContextRestoreGState(ctx);
}

static void drawRoundedBezel(CGContextRef context,CGRect frame){
   CGFloat radius=frame.size.height/2;
   
   CGContextBeginPath(context);
   CGContextAddArc(context,CGRectGetMaxX(frame)-radius,CGRectGetMinY(frame)+radius,radius,M_PI_2,M_PI_2*3,YES);
   CGContextAddArc(context,CGRectGetMinX(frame)+radius,CGRectGetMinY(frame)+radius,radius,M_PI_2*3,M_PI_2,YES);
   CGContextClosePath(context);
   CGContextFillPath(context);
}

-(void)drawBezelWithFrame:(NSRect)frame inView:(NSView *)controlView {
   BOOL defaulted=([[controlView window] defaultButtonCell] == self);

   NSRect adjustment = [self getControlSizeAdjustment:[controlView isFlipped] ];
   frame.size.width -= adjustment.size.width;
   frame.size.height -= adjustment.size.height;
   frame.origin.x += adjustment.origin.x;
   frame.origin.y += adjustment.origin.y;
   
   switch(_bezelStyle){
   
    case NSDisclosureBezelStyle:
     break;
    
    case NSRegularSquareBezelStyle:
     if([self isBordered]){
      BOOL  highlighted=(([self highlightsBy]&NSPushInCellMask) && [self isHighlighted]);
      float topGray=highlighted?0.8:0.9;
      float bottomGray=highlighted?0.7:0.8;
      
      NSRect top=frame,bottom=frame;
      top.size.height=floor(frame.size.height/2);
      bottom.size.height=ceil(frame.size.height/2);
      if([controlView isFlipped])
       bottom.origin.y+=top.size.height;
      else
       top.origin.y+=bottom.size.height;
      
      [[NSColor colorWithCalibratedWhite:topGray alpha:1] set];
      NSRectFill(top);
      [[NSColor colorWithCalibratedWhite:bottomGray alpha:1] set];
      NSRectFill(bottom);
      [[NSColor lightGrayColor] set];
      NSFrameRectWithWidth(frame,1);
     }
     break;
    
    case NSTexturedSquareBezelStyle:
    case NSTexturedRoundedBezelStyle:
    case NSShadowlessSquareBezelStyle:

     if ([self isBordered]) {
      [self _drawTexturedBezelWithFrame:frame];
     }
     break;
     
    case NSRecessedBezelStyle:;
     if([self isBordered] && [self isVisuallyHighlighted]){
      CGContextRef context=[[NSGraphicsContext currentContext] graphicsPort];

      frame.size.height--;
      frame.origin.y+=[controlView isFlipped]?1:0;
      [[NSColor lightGrayColor] setFill];
      drawRoundedBezel(context,frame);

      frame.origin.y+=[controlView isFlipped]?-1:1;
      [[NSColor darkGrayColor] setFill];
      drawRoundedBezel(context,frame);

      frame.origin.y+=[controlView isFlipped]?0:-1;
      frame.size.height++;
     
      frame=CGRectInset(frame,1,1.5);
      [[NSColor grayColor] setFill];
      drawRoundedBezel(context,frame);
     }
     break;
     
    default:
     if(![self isBordered]){
      [[_controlView graphicsStyle] drawUnborderedButtonInRect:frame defaulted:defaulted];
     }
     else {
      if(([self highlightsBy]&NSPushInCellMask) && [self isHighlighted])
       [[_controlView graphicsStyle] drawPushButtonPressedInRect:frame];
      else if([self isVisuallyHighlighted])
       [[_controlView graphicsStyle] drawPushButtonHighlightedInRect:frame];
      else
       [[_controlView graphicsStyle] drawPushButtonNormalInRect:frame defaulted:defaulted];
     }
     break;
   }
}

-(void)drawImage:(NSImage *)image withFrame:(NSRect)rect inView:(NSView *)controlView {
   BOOL enabled=[self isEnabled]?YES:![self imageDimsWhenDisabled];
   BOOL mixed=([self state]==NSMixedState)?YES:NO;
   
   CGContextRef ctx=[[NSGraphicsContext currentContext] graphicsPort];
   CGContextSaveGState(ctx);
   CGContextTranslateCTM(ctx,rect.origin.x,rect.origin.y);
   if([controlView isFlipped]){
    CGContextTranslateCTM(ctx,0,rect.size.height);
    CGContextScaleCTM(ctx,1,-1);
   }
   [[controlView graphicsStyle] drawButtonImage:image inRect:NSMakeRect(0,0,rect.size.width,rect.size.height) enabled:enabled mixed:mixed];
   CGContextRestoreGState(ctx);
}

-(NSRect)drawTitle:(NSAttributedString *)title withFrame:(NSRect)titleRect inView:(NSView *)controlView {
    
    [title _clipAndDrawInRect:titleRect];

    BOOL drawDottedRect=NO;

    if([[controlView window] firstResponder]==controlView){

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
     NSDottedFrameRect(NSInsetRect(titleRect,1,1));
    
    return titleRect; //FIXME: wrong value
}

// This function is duplicated in NSImageCell, consolidate
static NSSize scaledImageSizeInFrameSize(NSSize imageSize,NSSize frameSize,NSImageScaling scaling){
      
   switch(scaling){
    case NSImageScaleProportionallyDown:{
     float xscale=frameSize.width/imageSize.width;
     float yscale=frameSize.height/imageSize.height;
     float scale=MIN(1.0,MIN(xscale,yscale));
      
     imageSize.width*=scale;
     imageSize.height*=scale;
      
     return imageSize;
     }
     
    case NSImageScaleAxesIndependently:
     return frameSize;
     
    case NSImageScaleProportionallyUpOrDown:{
     float xscale=frameSize.width/imageSize.width;
     float yscale=frameSize.height/imageSize.height;
     float scale=MIN(xscale,yscale);
      
     imageSize.width*=scale;
     imageSize.height*=scale;
      
     return imageSize;
     }
     
    default:
    case NSImageScaleNone:
     return imageSize;
   }
}

-(void)drawInteriorWithFrame:(NSRect)frame inView:(NSView *)controlView {
/* This method gets the original button frame. We have to compensate for borders.
   There is some duplication of rect calculation which can be split out
 */
   BOOL defaulted=([[controlView window] defaultButtonCell] == self);
   NSRect adjustment = [self getControlSizeAdjustment:[controlView isFlipped] ];
   frame.size.width -= adjustment.size.width;
   frame.size.height -= adjustment.size.height;
   frame.origin.x += adjustment.origin.x;
   frame.origin.y += adjustment.origin.y;
   
   if(_bezelStyle==NSDisclosureBezelStyle)
    ;
   else if(![self isBordered]){
    if(defaulted)
     frame = NSInsetRect(frame,1,1);
   }
   else {
    frame=NSInsetRect(frame,2,2);
   }


   NSAttributedString *title=[self titleForHighlight];
   NSImage            *image=[self imageForHighlight];
   BOOL                enabled=[self isEnabled]?YES:![self imageDimsWhenDisabled];
   BOOL                mixed=([self state]==NSMixedState)?YES:NO;
    NSRect             imageRect = [self imageRectForBounds: frame];
    NSSize              imageSize=imageRect.size;
   NSPoint             imageOrigin=imageRect.origin;
   NSSize              titleSize=[title size];
    NSRect              titleRect=[self titleRectForBounds:frame];
   BOOL                drawImage=YES,drawTitle=YES;
   NSCellImagePosition imagePosition=[self imagePosition];

   if([self isTransparent])
    return;

// it doesnt actually change the image pos in the button but it draws like this
   if([self bezelStyle]==NSDisclosureBezelStyle)
    imagePosition=NSImageOnly;
    
   imageSize=scaledImageSizeInFrameSize(imageSize,frame.size,[self imageScaling]);
   
   imageOrigin.x+=floor((frame.size.width-imageSize.width)/2);
   imageOrigin.y+=floor((frame.size.height-imageSize.height)/2);

   titleRect.origin.y+=floor((titleRect.size.height-titleSize.height)/2);
   titleRect.size.height=titleSize.height;

    switch(imagePosition){

    case NSNoImage:
     drawImage=NO;
     break;

    case NSImageOnly:
     drawTitle=NO;
     break;

    case NSImageLeft:
     imageOrigin.x=frame.origin.x+kImageMargin;
     titleRect.origin.x+=imageSize.width+4;
     titleRect.size.width-=imageSize.width+4;
     break;

    case NSImageRight:
     imageOrigin.x=frame.origin.x+(frame.size.width-imageSize.width)-kImageMargin;
     titleRect.size.width-=(imageSize.width+4);
     break;

    case NSImageBelow:
     imageOrigin.y=frame.origin.y;
     titleRect.origin.y+=imageSize.height;
     imageOrigin.y = MAX(frame.origin.y, imageOrigin.y);
     titleRect.origin.y = MIN(frame.origin.y + frame.size.height - titleRect.size.height, titleRect.origin.y);
     break;

    case NSImageAbove:
     imageOrigin.y=frame.origin.y+(frame.size.height-imageSize.height);
     titleRect.origin.y-=imageSize.height;
     imageOrigin.y = MIN(frame.origin.y + frame.size.height - imageSize.height, imageOrigin.y);
     titleRect.origin.y = MAX(frame.origin.y, titleRect.origin.y);
     break;

    case NSImageOverlaps:
     break;
   }

   if(![self isBordered]){
    if([self isVisuallyHighlighted]){
     [[NSColor whiteColor] setFill];
     NSRectFill(frame);
    }
   }
   
   const BOOL isTextured=(_bezelStyle == NSTexturedSquareBezelStyle || _bezelStyle == NSTexturedRoundedBezelStyle);

   if([self isBordered] && !isTextured){
    if(([self highlightsBy]&NSPushInCellMask) && [self isHighlighted]){
     imageOrigin.x+=1;
     imageOrigin.y+=[controlView isFlipped]?1:-1;
     titleRect.origin.x+=1;
     titleRect.origin.y+=[controlView isFlipped]?1:-1;
    }
   }

   if(drawImage){
    NSRect rect=NSMakeRect(imageOrigin.x,imageOrigin.y,imageSize.width,imageSize.height);
    [self drawImage:image withFrame:rect inView:controlView];
   }

   if(drawTitle){
    if (isTextured && [title length]) {
// FIXME: use shadow in attributed string and implement shadow text drawing
        const BOOL pressed=[self state] && ([self showsStateBy] & NSChangeBackgroundCellMask);
        const CGFloat fgGray = (pressed) ? 0.98 : 0.0;
        const CGFloat fgGrayDisabled = 0.5;
        const CGFloat shadowGray = (pressed) ? 0.07 : 0.93;
        const CGFloat shadowAlpha = ([self isHighlighted]) ? 0.15 : 0.25;
        NSString *baseTitle = [NSString stringWithString:[title string]];
        NSMutableDictionary *shadowAttrs = [[[title attributesAtIndex:0 effectiveRange:NULL] mutableCopy] autorelease];
        
        if (titleRect.origin.y > frame.origin.y+1) {  // only draw the shadow if it doesn't come too close to the edge
            [shadowAttrs setObject:[NSColor colorWithDeviceRed:shadowGray green:shadowGray blue:shadowGray alpha:shadowAlpha]
                        forKey:NSForegroundColorAttributeName];
        
            NSAttributedString *shadowTitle = [[[NSAttributedString alloc] initWithString:baseTitle attributes:shadowAttrs] autorelease];
            NSRect shadowRect = NSOffsetRect(titleRect, 0, 1);
        
            [shadowTitle _clipAndDrawInRect:shadowRect];
        }
        
        NSMutableDictionary *fgAttrs = [[shadowAttrs mutableCopy] autorelease];
		if ([self isEnabled])
			[fgAttrs setObject:[NSColor colorWithDeviceRed:fgGray green:fgGray blue:fgGray alpha:1.0] forKey:NSForegroundColorAttributeName];
		else
			[fgAttrs setObject:[NSColor colorWithDeviceRed:fgGrayDisabled green:fgGrayDisabled blue:fgGrayDisabled alpha:1.0] forKey:NSForegroundColorAttributeName];
        title = [[[NSAttributedString alloc] initWithString:baseTitle attributes:fgAttrs] autorelease];
    }
    
    [self drawTitle:title withFrame:titleRect inView:controlView];
     }
     }

-(NSSize)cellSize  {
   NSSize              result=NSMakeSize(0,0);
   NSAttributedString *title=[self attributedTitle];
	NSImage            *image=[self image];
	BOOL                enabled=[self isEnabled]?YES:![self imageDimsWhenDisabled];
	BOOL                mixed=([self state]==NSMixedState)?YES:NO;
   NSSize              imageSize,titleSize;
	
   if(image==nil)
    imageSize=NSMakeSize(0,0);
   else if(_controlView)
    imageSize=[[_controlView graphicsStyle] sizeOfButtonImage:image enabled:enabled mixed:mixed];
	else
    imageSize=[image size];
	
    if(title==nil) {
        titleSize=NSMakeSize(0,0);
    }else {
        titleSize=[title size];
   }
   switch([self imagePosition]){

    case NSNoImage:
     result=titleSize;
     break;
	
    case NSImageOnly:
     result=imageSize;
     break;
      
    case NSImageLeft:
    case NSImageRight:
     result.width=imageSize.width+4+titleSize.width;
     result.height=MAX(imageSize.height,titleSize.height);
     break;
      
    case NSImageBelow:
    case NSImageAbove:
     result.width=MAX(imageSize.width,titleSize.width);
     result.height=imageSize.height+4+titleSize.height;
     break;
      
    case NSImageOverlaps:
     result.width=MAX(imageSize.width,titleSize.width);
     result.height=MAX(imageSize.height,titleSize.height);
     break;
	}
	
    // Add some margin
    result.width += 4;
    if( [self isBordered] || [self isBezeled] ){
		result.width += 4;
		result.height += 4;
	}
	
	NSRect adjustment = [self getControlSizeAdjustment:NO];
	result.width += adjustment.size.width;
	result.height += adjustment.size.height;
	
	return result;
}

-(void)drawWithFrame:(NSRect)frame inView:(NSView *)control {
   _controlView=control;

   if([self isTransparent])
    return;

   [self drawBezelWithFrame:frame inView:control];
   [self drawInteriorWithFrame:frame inView:control];
}

-(void)performClick:sender {
   if([_controlView respondsToSelector:@selector(performClick:)])
    [_controlView performSelector:@selector(performClick:) withObject:sender];
}

-(void)mouseEntered:(NSEvent *)event {
   NSUnimplementedMethod();
}

-(void)mouseExited:(NSEvent *)event {
   NSUnimplementedMethod();
}

@end
