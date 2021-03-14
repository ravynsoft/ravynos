/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSCell.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSControl.h>
#import <AppKit/NSClipView.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSRaise.h>

#import <Foundation/NSLocale.h>
#import <Foundation/NSNumberFormatter.h>
#import "NSCellUndoManager.h"
#import <AppKit/NSTextView.h>

@implementation NSCell

#pragma mark -
#pragma mark Class Methods

+(NSFocusRingType)defaultFocusRingType {
   return NSFocusRingTypeExterior;
}

+ (NSMenu *)defaultMenu
{
	return nil;
}

+(BOOL)prefersTrackingUntilMouseUp
{
	return NO;
}

#pragma mark -

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    int                flags=[keyed decodeIntForKey:@"NSCellFlags"];
    int                flags2=[keyed decodeIntForKey:@"NSCellFlags2"];
    id                 check;

       // These flags are big endian so the first flag is over on the left of the int
       _state           = (flags & 0x80000000) ? NSOnState : NSOffState;
       _isHighlighted   = (flags & 0x40000000) ? YES : NO;
       _isEnabled       = (flags & 0x20000000) ? NO : YES;  // "disabled" in Cocoa
       _isEditable      = (flags & 0x10000000) ? YES : NO;
       _cellType        = (flags & 0x0C000000) >> 26;

       // vCentered     = (flags & 0x02000000)
       // hCentered     = (flags & 0x01000000)

       _isBordered      = (flags & 0x00800000) ? YES : NO;
       _isBezeled       = (flags & 0x00400000) ? YES : NO;
       _isSelectable    = (flags & 0x00200000) ? YES : NO;
       _isScrollable    = (flags & 0x00100000) ? YES : NO;
       // _wraps        = (flags & 0x00100000) ? NO : YES; // ! scrollable, use lineBreakMode ?

       // 0x00080000 = continuous
       // 0x00040000 = action on mouse down
       // 0x00000100 = action on mouse drag
       _isContinuous    = (flags & 0x00080100) ? YES : NO;
       
       //actOnMouseDown = (flags & 0x00040000)
       //isLeaf         = (flags & 0x00020000)

       // invObjectVal  = (flags & 0x00010000)
       _hasValidObjectValue = YES;
       
       // invalidFont   = (flags & 0x00008000)
       // cellReserved1 = (flags & 0x00001800)
       // singleLineMode= (flags & 0x00000400)
       // actOnMouseDrag= (flags & 0x00000100)
       // isLoaded      = (flags & 0x00000080)
       // truncLastLine = (flags & 0x00000040)
       // dontActOnMsUp = (flags & 0x00000020)
       // isWhite       = (flags & 0x00000010)
       // userKeyEquiv  = (flags & 0x00000008)
       // showsFirstResp= (flags & 0x00000004)
       
       _focusRingType   = (flags & 0x00000003);
       
       // Now for flags2 - the layout of these flags seems to be more
       // random. So the best way to find out which bit(s) you need is
       // to diff a xib file before and after you make the change you're
       // trying to support (i.e. check a checkbox) and convert the decimal
       // NSCellFlags2 value to hex
       // wasSelectable = (flags2 & 0x80000000)
       
       _isRichText        = (flags2 & 0x40000000) ? YES : NO;
       
       // importsGraph  = (flags2 & 0x10000000)
       
       _textAlignment   = (flags2 & 0x1c000000) >> 26;

       // layoutDirRTL  = (flags2 & 0x01000000)
       _writingDirection=NSWritingDirectionNatural;
       
       // backgrdStyle  = (flags2 & 0x00c00000) >> 21;
       // cellReserved  = (flags2 & 0x003c0000)
       _refusesFirstResponder = (flags2 & 0x02000000) ? YES : NO;
       _allowsMixedState= (flags2 & 0x01000000) ? YES : NO;

       // inMixedState  = (flags2 & 0x00800000)

       _sendsActionOnEndEditing = (flags2 &0x00400000) ? YES : NO;
       
       // Odd this isn't in flags - it's near other items in flags in the Cocoa struct from which these are derived.
       _lineBreakMode   = (flags2 & 0x00000600) >> 9 & 0x7;
       _controlSize     = (flags2 & 0x000E0000) >> 17;
       

    _objectValue=[[keyed decodeObjectForKey:@"NSContents"] retain];
    check=[keyed decodeObjectForKey:@"NSNormalImage"];
    if([check isKindOfClass:[NSImage class]])
     _image=[check retain];
    else if([check isKindOfClass:[NSFont class]])
     _font=[check retain];
     
    check=[keyed decodeObjectForKey:@"NSSupport"];
    if([check isKindOfClass:[NSFont class]])
     _font=[check retain];
	
	[self setFormatter:[keyed decodeObjectForKey:@"NSFormatter"]];

    if (_font==nil)
       _font=[[NSFont userFontOfSize:13 - _controlSize*2] retain];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"%@ can not initWithCoder:%@",isa,[coder class]];
   }
   return self;
}

-initTextCell:(NSString *)string {
   _focusRingType=[isa defaultFocusRingType];
   _state=NSOffState;
   _font=[[NSFont userFontOfSize:0] retain];
   _objectValue=[string copy];
   _image=nil;
   _cellType=NSTextCellType;
   _isEnabled=YES;
   _isEditable=NO;
   _isSelectable=NO;
   _isBordered=NO;
   _isBezeled=NO;
   _isHighlighted=NO;
   _refusesFirstResponder=NO;
   _lineBreakMode=NSLineBreakByWordWrapping;
    _hasValidObjectValue = YES;
   return self;
}

-initImageCell:(NSImage *)image {
   _focusRingType=[isa defaultFocusRingType];
   _state=NSOffState;
   _font=nil;
   _objectValue=nil;
   _image=[image retain];
   _cellType=NSImageCellType;
   _isEnabled=YES;
   _isEditable=NO;
   _isSelectable=NO;
   _isBordered=NO;
   _isBezeled=NO;
   _isHighlighted=NO;
   _refusesFirstResponder=NO;
   _lineBreakMode=NSLineBreakByWordWrapping;
    _hasValidObjectValue = YES;
   return self;
}

-init {
   return [self initImageCell:nil];
}

-(void)dealloc {
   [_font release];
   [_objectValue release];
   [_image release];
   [_formatter release];
   [_representedObject release];
   [super dealloc];
}

-copyWithZone:(NSZone *)zone {
   NSCell *copy=NSCopyObject(self,0,zone);

   copy->_font=[_font retain];
   copy->_objectValue=[_objectValue copy];
   copy->_image=[_image retain];
   copy->_formatter=[_formatter retain];
   copy->_representedObject=[_representedObject retain];

   return copy;
}

-(NSView *)controlView {
   return nil;
}

-(NSCellType)type {
   return _cellType;
}

-(int)state {
   if (_allowsMixedState) {
      if (_state < 0)
         return -1;
      else if (_state > 0)
         return 1;
      else
         return 0;
   }
   else
      return (abs(_state) > 0) ? 1 : 0;
}

-target {
   return nil;
}

-(SEL)action {
   return NULL;
}

-(int)tag {
   return -1;
}

-(int)entryType {
   return _entryType;
}

-(id)formatter {
    return _formatter;
}

-(NSFont *)font {
   return _font;
}

-(NSImage *)image {
   return _image;
}

-(NSTextAlignment)alignment {
   return _textAlignment;
}

-(NSLineBreakMode)lineBreakMode {
   return _lineBreakMode;
}

-(NSWritingDirection)baseWritingDirection {
   return _writingDirection;
}

-(BOOL)wraps {
   return (_lineBreakMode==NSLineBreakByWordWrapping || _lineBreakMode==NSLineBreakByCharWrapping)?YES:NO;
}

-(NSString *)title {
    return [self stringValue];
}

-(BOOL)isEnabled {
   return _isEnabled;
}

-(BOOL)isEditable {
   return _isEditable;
}

-(BOOL)isSelectable {
   return _isSelectable;
}

-(BOOL)isScrollable {
   return _isScrollable;
}

-(BOOL)isBordered {
   return _isBordered;
}

-(BOOL)isBezeled {
   return _isBezeled;
}

-(BOOL)isContinuous {
   return _isContinuous;
}

-(BOOL)showsFirstResponder {
   return _showsFirstResponder;
}

-(BOOL)refusesFirstResponder {
   return _refusesFirstResponder;
}

-(BOOL)isHighlighted {
   return _isHighlighted;
}

-(BOOL)hasValidObjectValue
{
    return _hasValidObjectValue;
}

/*
 "Returns the receiver's object value, or nil if a valid object has not been associated with the receiver."
 */
-objectValue {
    if (_hasValidObjectValue) {
        return _objectValue;
    } else {
        return nil;
    }
}

-(NSString *)stringValue {
    NSString *formatted;
    if (_formatter != nil)
        if ((formatted = [_formatter stringForObjectValue:_objectValue])!=nil)
          return formatted;
    if([_objectValue isKindOfClass:[NSAttributedString class]])
     return [_objectValue string];
    else if([_objectValue isKindOfClass:[NSString class]])
     return _objectValue;

    if([_objectValue respondsToSelector:@selector(descriptionWithLocale:)])
        return [_objectValue descriptionWithLocale:[NSLocale currentLocale]];
    else if([_objectValue respondsToSelector:@selector(description)])
        return [_objectValue description];
    else
        return @"";
}

-(int)intValue {
   NSString *objString = ([_objectValue isKindOfClass:[NSAttributedString class]]) ? [_objectValue string] : (NSString *)_objectValue;
   if([objString isKindOfClass:[NSString class]])
   {
      int i = 0;
      [[NSScanner localizedScannerWithString:objString] scanInt:&i];
      return i;
   }
   else
      return [_objectValue intValue];
}

-(float)floatValue {
	if (_objectValue == nil) {
		// [nil someFloatMethod] doesn't return 0.f on Cocotron - tmp fix until the runtime is fixed
		return 0.f;
	}
	NSString *objString = ([_objectValue isKindOfClass:[NSAttributedString class]]) ? [_objectValue string] : (NSString *)_objectValue;
   if([objString isKindOfClass:[NSString class]])
   {
      float f = 0.0;
      [[NSScanner localizedScannerWithString:objString] scanFloat:&f];
      return f;
   }
   else {
      return [_objectValue floatValue];
   }
}

-(double)doubleValue {
	if (_objectValue == nil) {
		// [nil someDoubleMethod] doesn't return 0. on Cocotron - tmp fix until the runtime is fixed
		return 0.;
	}
	
   NSString *objString = ([_objectValue isKindOfClass:[NSAttributedString class]]) ? [_objectValue string] : (NSString *)_objectValue;
   if([objString isKindOfClass:[NSString class]])
   {
      double d = 0.0;
      [[NSScanner localizedScannerWithString:objString] scanDouble:&d];
      return d;
   }
   else
      return [_objectValue doubleValue];
}

-(NSInteger)integerValue {
   NSString *objString = ([_objectValue isKindOfClass:[NSAttributedString class]]) ? [_objectValue string] : (NSString *)_objectValue;
   if([objString isKindOfClass:[NSString class]])
   {
      NSInteger i = 0;
      [[NSScanner localizedScannerWithString:objString] scanInteger:&i];
      return i;
   }
   else
      return [_objectValue integerValue];
}

-(NSAttributedString *)attributedStringValue {
   if([_objectValue isKindOfClass:[NSAttributedString class]])
    return _objectValue;
   else {
    NSMutableDictionary *attributes=[NSMutableDictionary dictionary];
    NSMutableParagraphStyle *paraStyle=[[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
    NSFont              *font=[self font];

    if(font!=nil)
     [attributes setObject:font forKey:NSFontAttributeName];

    if([self isEnabled]){
     if([self isHighlighted] || [self state])
      [attributes setObject:[NSColor whiteColor]
                     forKey:NSForegroundColorAttributeName];
     else
      [attributes setObject:[NSColor controlTextColor]
                     forKey:NSForegroundColorAttributeName];
    }
    else {
     [attributes setObject:[NSColor disabledControlTextColor]
                     forKey:NSForegroundColorAttributeName];
    }

    [paraStyle setLineBreakMode:_lineBreakMode];
    [paraStyle setAlignment:_textAlignment];
    [attributes setObject:paraStyle forKey:NSParagraphStyleAttributeName];

    return [[[NSAttributedString alloc] initWithString:[self stringValue] attributes:attributes] autorelease];
   }
}

-(id)representedObject {
    return _representedObject;
}

-(NSControlSize)controlSize {
    return _controlSize;
}

-(NSFocusRingType)focusRingType {
    return _focusRingType;
}

-(NSBackgroundStyle)backgroundStyle {
   return _backgroundStyle;
}

-(void)setControlView:(NSView *)view {
// Do nothing or raise?
}

-(void)setType:(NSCellType)type {
   if(_cellType!=type){
    _cellType = type;
    if (type == NSTextCellType) {
// FIX, localization
       [self setTitle:@"Cell"];				// mostly clarified in setEntryType dox
       [self setFont:[NSFont systemFontOfSize:12.0]];
    }
    [[[self controlView] window] invalidateCursorRectsForView:[self controlView]];
   }
}

-(void)setState:(int)value {
   if (_allowsMixedState) {
      if (value < 0)
         _state = -1;
      else if (value > 0)
         _state = 1;
      else
         _state = 0;
   }
   else
      _state = (abs(value) > 0) ? 1 : 0;
}

-(int)nextState {
   if (_allowsMixedState) {
      int value = [self state];
      return value - ((value == -1) ? -2 : 1);
   }
   else
      return 1 - [self state];
}

-(void)setNextState {
   _state = [self nextState];
}

-(BOOL)allowsMixedState; {
   return _allowsMixedState;
}

-(void)setAllowsMixedState:(BOOL)allow {
   _allowsMixedState = allow;
}

-(void)setTarget:target {
   [NSException raise:NSInternalInconsistencyException
               format:@"-[%@ %s] Unimplemented",isa,sel_getName(_cmd)];
}


-(void)setAction:(SEL)action {
   [NSException raise:NSInternalInconsistencyException
               format:@"-[%@ %s] Unimplemented",isa,sel_getName(_cmd)];
}

-(void)setTag:(int)tag {
   [NSException raise:NSInternalInconsistencyException
               format:@"-[%@ %s] Unimplemented",isa,sel_getName(_cmd)];
}

-(void)setEntryType:(int)type {
   _entryType=type;
   [self setType:NSTextCellType];
}

-(void)setFormatter:(NSFormatter *)formatter {
    formatter=[formatter retain];
    [_formatter release];
    _formatter=formatter;
}

-(void)setFont:(NSFont *)font {
   font=[font retain];
   [_font release];
   _font=font;
}

-(void)setImage:(NSImage *)image {
   if(image!=nil)
   [self setType:NSImageCellType];
    
   image=[image retain];
   [_image release];
   _image=image;
   [(NSControl *)[self controlView] updateCell:self];
}

-(void)setAlignment:(NSTextAlignment)alignment {
   _textAlignment=alignment;
}

-(void)setLineBreakMode:(NSLineBreakMode)value {
   _lineBreakMode=value;
}

-(void)setBaseWritingDirection:(NSWritingDirection)value {
   _writingDirection=value;
}

-(void)setWraps:(BOOL)wraps {
   _lineBreakMode=wraps?NSLineBreakByWordWrapping:NSLineBreakByClipping;
}

-(void)setTitle:(NSString *)title {
    [self setStringValue:title];
}

-(void)setEnabled:(BOOL)flag {
   if(_isEnabled!=flag){
    _isEnabled=flag;
    [[[self controlView] window] invalidateCursorRectsForView:[self controlView]];
   }
}

-(void)setEditable:(BOOL)flag {
   if(_isEditable!=flag){
    _isEditable=flag;
    [[[self controlView] window] invalidateCursorRectsForView:[self controlView]];
   }
}

-(void)setSelectable:(BOOL)flag {
   if(_isSelectable!=flag){
    _isSelectable=flag;
    [[[self controlView] window] invalidateCursorRectsForView:[self controlView]];
   }
}

-(void)setScrollable:(BOOL)flag {
   _isScrollable=flag;
}

-(void)setBordered:(BOOL)flag {
   _isBordered=flag;
   _isBezeled=NO;
}

-(void)setBezeled:(BOOL)flag {
   _isBezeled=flag;
}

-(void)setContinuous:(BOOL)flag {
   _isContinuous=flag;
}

-(void)setShowsFirstResponder:(BOOL)value {
   _showsFirstResponder=value;
}

-(void)setRefusesFirstResponder:(BOOL)flag {
   _refusesFirstResponder=flag;
}

-(void)setHighlighted:(BOOL)flag {
   _isHighlighted = flag;
}

// the problem with this method is that the dox specify that if autorange is YES, then the field
// becomes one big floating-point entry, but NSNumberFormatter doesn't work that way. - dwy
-(void)setFloatingPointFormat:(BOOL)fpp left:(unsigned)left right:(unsigned)right {
    NSMutableString *format = [NSMutableString string];
    
    [self setFormatter:[[[NSNumberFormatter alloc] init] autorelease]];
    if (fpp == YES) { // autorange
        unsigned fieldWidth = left + right;
        while(fieldWidth--)
            [format appendString:@"#"];
    }
    else {
        while(left--)
            [format appendString:@"#"];
        [format appendString:@"."];
        while(right--)
            [format appendString:@"0"];
    }
    [(NSNumberFormatter *)_formatter setFormat:format];
}

-(void)setObjectValue:(id <NSCopying>)value {
    value=[value copyWithZone:NULL];
    
    [[self controlView] willChangeValueForKey:@"objectValue"];
    [_objectValue release];
    _objectValue=value;
    _hasValidObjectValue = YES;
    [[self controlView] didChangeValueForKey:@"objectValue"];

    [(NSControl *)[self controlView] updateCell:self];
}

-(void)setStringValue:(NSString *)value {
   if(value==nil){
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] value==nil",isa,sel_getName(_cmd)];
    return;
   }

   [self setType:NSTextCellType];

   if (_formatter != nil) {
       id formattedValue = nil;
       if ([_formatter getObjectValue:&formattedValue forString:value errorDescription:NULL]) {
           value=formattedValue;
       } else {
           // Invalid format
           _hasValidObjectValue = NO;
           return;
       }
   }
   [self setObjectValue:value];
}

-(void)setIntValue:(int)value {
   [self setObjectValue:[NSNumber numberWithInt:value]];
}

-(void)setFloatValue:(float)value {
   [self setObjectValue:[NSNumber numberWithFloat:value]];
}

-(void)setDoubleValue:(double)value {
   [self setObjectValue:[NSNumber numberWithDouble:value]];
}

-(void)setIntegerValue:(NSInteger)value {
   [self setObjectValue:[NSNumber numberWithInteger:value]];
}


-(void)setAttributedStringValue:(NSAttributedString *)value {
   value=[value copy];
   [_objectValue release];
   _objectValue=value;
}

-(void)setRepresentedObject:(id)object {
    object = [object retain];
    [_representedObject release];
    _representedObject = object;
}

-(void)setControlSize:(NSControlSize)size {
   _controlSize = size;
   [_font release];
   _font = [[NSFont userFontOfSize:13 - _controlSize*2] retain];
   [(NSControl *)[self controlView] updateCell:self];
}

-(void)setFocusRingType:(NSFocusRingType)focusRingType {
   _focusRingType = focusRingType;
}

-(void)setBackgroundStyle:(NSBackgroundStyle)value {
   _backgroundStyle=value;
}

-(void)takeObjectValueFrom:sender {
   [self setObjectValue:[sender objectValue]];
}

-(void)takeStringValueFrom:sender {
   [self setStringValue:[sender stringValue]];
}

-(void)takeIntValueFrom:sender {
   [self setIntValue:[sender intValue]];
}

-(void)takeFloatValueFrom:sender {
   [self setFloatValue:[sender floatValue]];
}

-(void)takeDoubleValueFrom:sender {
   [self setDoubleValue:[sender doubleValue]];
}

-(void)takeIntegerValueFrom:sender {
   [self setIntegerValue:[sender integerValue]];
}

-(NSSize)cellSize {
   return NSMakeSize(10000,10000);
}

-(NSSize)cellSizeForBounds:(NSRect)rect {
   NSSize result=[self cellSize];
   
   return NSMakeSize(MIN(rect.size.width,result.width),MIN(rect.size.height,result.height));
}

-(NSRect)imageRectForBounds:(NSRect)rect {
   return rect;
}

-(NSRect)titleRectForBounds:(NSRect)rect {
   return rect;
}

-(NSRect)drawingRectForBounds:(NSRect)rect {
   return rect;
}

-(void)drawInteriorWithFrame:(NSRect)frame inView:(NSView *)view {
}

-(void)drawWithFrame:(NSRect)frame inView:(NSView *)view {

   if([self type]==NSTextCellType){
    if([self isBezeled])
     NSDrawWhiteBezel(frame,frame);
   }

   [self drawInteriorWithFrame:[self drawingRectForBounds:frame] inView:view];
}

-(void)highlight:(BOOL)highlight withFrame:(NSRect)frame inView:(NSView *)view {
   if(_isHighlighted!=highlight){
    _isHighlighted=highlight;
   }
}

-(BOOL)startTrackingAt:(NSPoint)startPoint inView:(NSView *)view {
   return YES;
}

-(BOOL)continueTracking:(NSPoint)lastPoint at:(NSPoint)currentPoint inView:(NSView *)view {
   return YES;
}

-(void)stopTracking:(NSPoint)lastPoint at:(NSPoint)stopPoint inView:(NSView *)view mouseIsUp:(BOOL)flag {
}

-(BOOL)trackMouse:(NSEvent *)event inRect:(NSRect)frame ofView:(NSView *)view untilMouseUp:(BOOL)untilMouseUp {
   NSPoint lastPoint;
   BOOL    result=NO;

   if(![self startTrackingAt:[event locationInWindow] inView:view])
    return NO;

   do{
    NSPoint currentPoint;
    BOOL isWithinCellFrame;

    lastPoint=[event locationInWindow];
    currentPoint=[view convertPoint:[event locationInWindow] fromView:nil];
    isWithinCellFrame=NSMouseInRect(currentPoint,frame,[view isFlipped]);

    if(untilMouseUp){
     if([event type]==NSLeftMouseUp){
      [self stopTracking:lastPoint at:[event locationInWindow] inView:view mouseIsUp:YES];
      result=YES;
      break;
     }
    }
    else if(isWithinCellFrame){
     if([event type]==NSLeftMouseUp){
      [self stopTracking:lastPoint at:[event locationInWindow] inView:view mouseIsUp:YES];
      result=YES;
      break;
     }
    }
    else {
     [self stopTracking:lastPoint at:[event locationInWindow] inView:view mouseIsUp:NO];
     result=NO;
     break;
    }

    if(untilMouseUp || isWithinCellFrame) {
     if(![self continueTracking:lastPoint at:[event locationInWindow] inView:view])
      break;

     if([self isContinuous])
      [(NSControl *)view sendAction:[(NSControl *)view action] to:[(NSControl *)view target]];
    }

    [[view window] flushWindow];

    event=[[view window] nextEventMatchingMask:NSLeftMouseUpMask|NSLeftMouseDraggedMask];

   }while(YES);

   return result;
}

-(NSText *)setUpFieldEditorAttributes:(NSText *)editor {
   [editor setEditable:[self isEditable]];
   [editor setSelectable:[self isSelectable]];
    NSString* strValue = nil;
    if (_formatter) {
        // Prioritize the editing string value over everything else
        strValue = [_formatter editingStringForObjectValue: [self objectValue]];
    }
    if (strValue == nil) {
        strValue = [self stringValue];
    }
    [editor setRichText: _isRichText];
	[editor setString: strValue];
   [editor setFont:[self font]];
   [editor setAlignment:[self alignment]];
   if([self respondsToSelector:@selector(drawsBackground)])
    [editor setDrawsBackground:(BOOL)(int)[self performSelector:@selector(drawsBackground)]];
   if([self respondsToSelector:@selector(backgroundColor)])
    [editor setBackgroundColor:[self performSelector:@selector(backgroundColor)]];

   return editor;
}

-(void)_setupFieldEditorWithFrame:(NSRect)frame controlView:(NSView *)view editor:(NSText *)editor delegate:delegate {
/* There is some funkiness here where the editor is already in the control and it is moving to
   a different cell or the same cell is being edited after a makeFirstResponder
   This needs to be straightened out
 */
   if([self isScrollable] || [self wraps]){
    NSClipView *clipView;

    if([[editor superview] isKindOfClass:[NSClipView class]] && [[editor superview] superview] == view){
     clipView=(NSClipView *)[editor superview];
     [clipView setFrame:frame];
    }
    else {
     clipView=[[[NSClipView alloc] initWithFrame:frame] autorelease];
	 [editor setFrameOrigin:NSZeroPoint];
	 [editor setFrameSize:frame.size];
     [clipView setDocumentView:editor];
     [view addSubview:clipView];
    }

    [clipView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [editor setAutoresizingMask:0];
    [editor setHorizontallyResizable:[self isScrollable]];
    [editor setVerticallyResizable:YES];
    [editor sizeToFit];
    [editor setNeedsDisplay:YES];
   }
   else {
	   [editor setHorizontallyResizable:NO];
	   [editor setVerticallyResizable:NO];
	   [editor setFrame:frame];
	   [view addSubview:editor];
   }
   [[view window] makeFirstResponder:editor];
   [editor setDelegate:delegate];
  
   if ([editor isKindOfClass:[NSTextView class]]) {
    NSCellUndoManager * undoManager = [[NSCellUndoManager alloc] init];
    [undoManager setNextUndoManager:[[view window] undoManager]];
    [(NSTextView *)editor _setFieldEditorUndoManager:undoManager];
    [undoManager release];
    [(NSTextView *)editor setAllowsUndo:YES];
   }
}

-(void)editWithFrame:(NSRect)frame inView:(NSView *)view editor:(NSText *)editor delegate:(id)delegate event:(NSEvent *)event {

   if(![self isEditable] && ![self isSelectable])
    return;

   if(view == nil || editor == nil || [self font] == nil || _cellType != NSTextCellType)
    return;

   [self _setupFieldEditorWithFrame:frame controlView:view editor:editor delegate:delegate];
    [editor mouseDown:event];
}

-(void)selectWithFrame:(NSRect)frame inView:(NSView *)view editor:(NSText *)editor delegate:(id)delegate start:(int)location length:(int)length {
   if(![self isEditable] && ![self isSelectable])
    return;

   if(view == nil || editor == nil || [self font] == nil || _cellType != NSTextCellType)
    return;

   [self _setupFieldEditorWithFrame:frame controlView:view editor:editor delegate:delegate];
    NSRange range = NSMakeRange(location, length);
    NSRange maxRange = NSMakeRange(0, [[editor string] length]);
   [editor setSelectedRange:NSIntersectionRange(range, maxRange)];
}

-(void)endEditing:(NSText *)editor {
    [self setStringValue: [editor string]];
}

-(void)resetCursorRect:(NSRect)rect inView:(NSView *)view {
   if(([self type]==NSTextCellType) && [self isEnabled]){
    if([self isEditable] || [self isSelectable]){
     NSRect titleRect=[self titleRectForBounds:rect];

     titleRect=NSIntersectionRect(titleRect,[view visibleRect]);

     if(!NSIsEmptyRect(titleRect))
      [view addCursorRect:titleRect cursor:[NSCursor IBeamCursor]];
    }
   }
}

- (void)setSendsActionOnEndEditing:(BOOL)flag {
   _sendsActionOnEndEditing=flag;
}

- (BOOL)sendsActionOnEndEditing {
   return _sendsActionOnEndEditing;
}

static NSString *NSStringFromCellType(NSCellType type) {
    switch (type) {
        case NSNullCellType:
            return @"NSNullCellType";
        case NSImageCellType:
            return @"NSImageCellType";
        case NSTextCellType:
            return @"NSTextCellType";
        default:
            break;
    }
    return [NSString stringWithFormat: @"Unknown: %d", type];
}

- (NSString *)description
{
    return [NSString stringWithFormat: @"%@\n\ttype: %@\n\tfont: %@\n\tobjectValue: %@",
            [super description],
            NSStringFromCellType(_cellType),
            _font,
            _objectValue];
}
@end

void NSDrawThreePartImage(NSRect frame,NSImage *startCap,NSImage *centerFill,NSImage *endCap,BOOL vertical,NSCompositingOperation operation,CGFloat alpha,BOOL flipped) {
   NSUnimplementedFunction();
}
