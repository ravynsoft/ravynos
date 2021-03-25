/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSTextView.h>
#import <AppKit/NSTextContainer.h>
#import <AppKit/NSTextStorage.h>
#import <AppKit/NSLayoutManager.h>
#import <AppKit/NSRulerView.h>
#import <AppKit/NSRulerMarker.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSStringDrawing.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSClipView.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSPasteboard.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSMenuItem.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSTextTab.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSRichTextReader.h>
#import <AppKit/NSRichTextWriter.h>
#import <AppKit/NSDisplay.h>
#import <AppKit/NSFontManager.h>
#import <AppKit/NSScrollView.h>
#import <AppKit/NSDragging.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSGraphicsStyle.h>
#import <AppKit/NSGraphicsContext.h>
#import "NSTextViewSharedData.h"
#import <AppKit/NSRaise.h>
#import <AppKit/NSController.h>
#import <AppKit/NSSpellChecker.h>
#import <AppKit/NSControl.h>

#import "NSUndoTextOperation.h"
#import "NSRulerMarker+NSTextExtensions.h"

NSString * const NSTextViewDidChangeSelectionNotification=@"NSTextViewDidChangeSelectionNotification";
NSString * const NSOldSelectedCharacterRange=@"NSOldSelectedCharacterRange";

@interface NSLayoutManager(NSLayoutManager_visualKeyboardMovement)
- (NSRange)_softLineRangeForCharacterAtIndex:(unsigned)location;
@end

@interface NSTextView(NSTextView_textCompletion)
- (void)endUserCompletion;
@end

@interface NSTextView()
-(void)_updateTypingAttributes;
-(void)_replaceCharactersInRange:(NSRange)range
					  withString:(id)string
			 useTypingAttributes:(BOOL)useTypingAttributes
          allowsTypingCoalescing:(BOOL)allowsTypingCoalescing;
// Same as above, with allowsTypingCoalescing = YES
-(void)_replaceCharactersInRange:(NSRange)range withString:(id)string useTypingAttributes:(BOOL)useTypingAttributes;
// Same as above, with useTypingAttributes = YES
-(void)_replaceCharactersInRange:(NSRange)range withString:(id)string;
-(void)_replaceCharactersInRange:(NSRange)range withString:(id)string allowsTypingCoalescing:(BOOL)allowsTypingCoalescing;

- (BOOL) _delegateChangeTextInRange: (NSRange)    range
                  replacementString: (NSString *) string;

-(NSArray *)_delegateChangeSelectionFromRanges:(NSArray *)from toRanges:(NSArray *)to;

-(void)_continuousSpellCheckWithInvalidatedRange:(NSRange)range;
-(void)_continuousSpellCheck;
@end

@implementation NSTextView
-(void)configureMenu {
    static NSMenu *menu=nil;
    
    if(menu==nil){
        menu=[[NSMenu alloc] initWithTitle:@""];
        [menu addItemWithTitle: NSLocalizedStringFromTableInBundle(@"Cut", nil, [NSBundle bundleForClass: [NSTextView class]], @"Cut the selection")
                        action:@selector(cut:) keyEquivalent:@""];
        [menu addItemWithTitle: NSLocalizedStringFromTableInBundle(@"Copy", nil, [NSBundle bundleForClass: [NSTextView class]], @"Copy the selection")
                        action:@selector(copy:) keyEquivalent:@""];
        [menu addItemWithTitle: NSLocalizedStringFromTableInBundle(@"Paste", nil, [NSBundle bundleForClass: [NSTextView class]], @"Paste the selection")
                        action:@selector(paste:) keyEquivalent:@""];
        [menu addItem:[NSMenuItem separatorItem]];
        [menu addItemWithTitle: NSLocalizedStringFromTableInBundle(@"Select All", nil, [NSBundle bundleForClass: [NSTextView class]], @"Select all the content")
                        action:@selector(selectAll:) keyEquivalent:@""];
    }
    [self setMenu:menu];
}

-(void)encodeWithCoder:(NSCoder *)coder {
    NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
    [super initWithCoder:coder];
    
    if([coder allowsKeyedCoding]){
        NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
        unsigned              flags=[keyed decodeIntForKey:@"NSTVFlags"];
        NSTextViewSharedData *sharedData=[keyed decodeObjectForKey:@"NSSharedData"];
        
        _textContainer=[[keyed decodeObjectForKey:@"NSTextContainer"] retain];
        _textContainerInset=NSMakeSize(4,4);
        
        _textStorage=[[[_textContainer layoutManager] textStorage] retain];
        [_textStorage addLayoutManager:[_textContainer layoutManager]];
        _ownsTextStorage=YES;
        
        _delegate=[keyed decodeObjectForKey:@"NSDelegate"];
        
        _isEditable=[sharedData isEditable];
        _isSelectable=[sharedData isSelectable];
        _isRichText=[sharedData isRichText];
        _allowsUndo=[sharedData allowsUndo];
        
        _backgroundColor=[[sharedData backgroundColor] retain];
        _drawsBackground=[sharedData drawsBackground];
        _font=[[NSFont userFontOfSize:0] retain];
        _textColor=[[NSColor textColor] copy];
        
        _textAlignment=[[sharedData defaultParagraphStyle] alignment];
        _insertionPointColor=[[sharedData insertionColor] retain];
        
        _usesFontPanel=YES;
        
        _isFieldEditor=NO;
        _maxSize=[self bounds].size;
        _minSize=NSMakeSize(0,0);

        _isHorizontallyResizable = ((flags & 0x01) != 0);
        _isVerticallyResizable = ((flags & 0x02) != 0);

        _selectedRanges=[[NSMutableArray alloc] init];
        [_selectedRanges addObject:[NSValue valueWithRange:NSMakeRange(0,0)]];
        
        _rangeForUserCompletion=NSMakeRange(NSNotFound, 0);
        _selectedTextAttributes=[[NSDictionary dictionaryWithObjectsAndKeys:
                                  [NSColor selectedTextColor],NSForegroundColorAttributeName,
                                  [NSColor selectedTextBackgroundColor],NSBackgroundColorAttributeName,
                                  nil] retain];
        
        [_textStorage addAttribute:NSFontAttributeName value:_font range:NSMakeRange(0,[[self textStorage] length])];
        [_textStorage addAttribute:NSForegroundColorAttributeName value:_textColor range:NSMakeRange(0,[[self textStorage] length])];
        
        NSMutableDictionary *typingAttributes=[[_textStorage attributesAtIndex:0 effectiveRange:NULL] mutableCopy];
        if (![typingAttributes objectForKey:NSFontAttributeName]) {
            [typingAttributes setObject:_font forKey: NSFontAttributeName];
        }
        if (![typingAttributes objectForKey:NSForegroundColorAttributeName]) {
            [typingAttributes setObject:_textColor forKey: NSForegroundColorAttributeName];
        }
        _typingAttributes = typingAttributes;
        
        if ([typingAttributes objectForKey: NSParagraphStyleAttributeName]) {
            _defaultParagraphStyle = [[typingAttributes objectForKey: NSParagraphStyleAttributeName] copy];
        } else {
            _defaultParagraphStyle = [[NSParagraphStyle defaultParagraphStyle] copy];
        }
    }
    else {
        [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
    }
    [self registerForDraggedTypes:[NSArray arrayWithObjects:NSRTFPboardType, NSStringPboardType, nil]];
    
    return self;
}


-initWithFrame:(NSRect)frame textContainer:(NSTextContainer *)container {
    [super initWithFrame:frame];
    
    _textStorage=[[container layoutManager] textStorage];
    _ownsTextStorage=NO;
    _textContainer=[container retain];
    [_textContainer setTextView:self];
    _textContainerInset=NSMakeSize(0,0);
    
    _isEditable=YES;
    _isSelectable=YES;
    _isRichText=YES;
	_usesFontPanel=YES;
    _backgroundColor=[[NSColor whiteColor] copy];
    _drawsBackground=YES;
    _font=[[NSFont userFontOfSize:0] retain];
    _textColor=[[NSColor textColor] copy];
    _textAlignment=NSLeftTextAlignment;
    _insertionPointColor=[[NSColor blackColor] copy];
    _isFieldEditor=NO;
    _maxSize=[self bounds].size;
    _isHorizontallyResizable=NO;
    _isVerticallyResizable=YES;
    _selectedRanges=[[NSMutableArray alloc] init];
    [_selectedRanges addObject:[NSValue valueWithRange:NSMakeRange(0,0)]];
    
	NSMutableDictionary *typingAttributes=[[_textStorage attributesAtIndex:0 effectiveRange:NULL] mutableCopy];
	if (![typingAttributes objectForKey:NSFontAttributeName]) {
		[typingAttributes setObject:_font forKey: NSFontAttributeName];
	}
	if (![typingAttributes objectForKey:NSForegroundColorAttributeName]) {
		[typingAttributes setObject:_textColor forKey: NSForegroundColorAttributeName];
	}
	_typingAttributes = typingAttributes;
	
    if ([typingAttributes objectForKey: NSParagraphStyleAttributeName]) {
        _defaultParagraphStyle = [[typingAttributes objectForKey: NSParagraphStyleAttributeName] copy];
    } else {
        _defaultParagraphStyle = [[NSParagraphStyle defaultParagraphStyle] copy];
    }

    _rangeForUserCompletion=NSMakeRange(NSNotFound, 0);
    _selectedTextAttributes=[[NSDictionary dictionaryWithObjectsAndKeys:
                              [NSColor selectedTextColor],NSForegroundColorAttributeName,
                              [NSColor selectedTextBackgroundColor],NSBackgroundColorAttributeName,
                              nil] retain];
    
    [self setBoundsOrigin:NSMakePoint(-_textContainerInset.width,-_textContainerInset.height)];
    [self configureMenu];
    [self registerForDraggedTypes:[NSArray arrayWithObjects:NSRTFPboardType,NSStringPboardType,nil]];
    
    return self;
}

-initWithFrame:(NSRect)frame {
    NSTextStorage   *storage=[[NSTextStorage new] autorelease];
    NSLayoutManager *layout=[[NSLayoutManager new] autorelease];
    NSTextContainer *container=[[[NSTextContainer alloc] initWithContainerSize:frame.size] autorelease];
    
    [storage addLayoutManager:layout];
    [layout addTextContainer:container];
    
    self=[self initWithFrame:frame textContainer:container];
    if(self==nil)
        return nil;
    
    [_textStorage retain];
    _ownsTextStorage=YES;
    
    return self;
}

-(void)awakeFromNib {
    [self configureMenu];
}

-(void)dealloc {
    if (_delegate) {
        // Be sure to remove our delegate as an observer before we die
        [self setDelegate:nil];
    }
	if(_ownsTextStorage) {
		[_textStorage release];
	}
	[_textContainer setTextView: nil];
    [_textContainer release];
    [_typingAttributes release];
    [_defaultParagraphStyle release];
    [_backgroundColor release];
    [_font release];
    [_textColor release];
    [_insertionPointColor release];
    [_insertionPointTimer invalidate];
    [_insertionPointTimer release];
    [_selectedRanges release];
    [_initialRanges release];
    [_selectedTextAttributes release];
    [_fieldEditorUndoManager release];
    [_undoTyping release];
    [super dealloc];
}

-(BOOL)isFlipped {
    return YES;
}

-(BOOL)isOpaque {
    return [self drawsBackground];
}

-(BOOL)acceptsFirstResponder {
    return YES;
}

-(BOOL)canBecomeKeyView {
    if(![self isEditable])
        return NO;
    
    return [super canBecomeKeyView];
}

-(BOOL)needsPanelToBecomeKey {
    return YES;
}

-(void)resetCursorRects {
    [self addCursorRect:[self visibleRect] cursor:[NSCursor IBeamCursor]];
}

-(NSTextContainer *)textContainer {
    return _textContainer;
}

-(NSSize)textContainerInset {
    return _textContainerInset;
}

-(NSPoint)textContainerOrigin {
    return NSMakePoint(_textContainerInset.width,_textContainerInset.height);
}

-(NSLayoutManager *)layoutManager {
    return [_textContainer layoutManager];
}

-(NSTextStorage *)textStorage {
    return _textStorage;
}

-(BOOL)usesRuler {
    return _usesRuler;
}

-(BOOL)isRulerVisible {
    return _rulerVisible;
}

-(BOOL)allowsUndo {
    return _allowsUndo;
}

-(NSColor *)insertionPointColor {
    return _insertionPointColor;
}

-(NSDictionary *)typingAttributes {
    return [[_typingAttributes retain] autorelease];
}

-(NSParagraphStyle *)defaultParagraphStyle {
    return _defaultParagraphStyle;
}

-(NSDictionary *)selectedTextAttributes {
    return _selectedTextAttributes;
}

-(NSArray *)selectedRanges {
    return _selectedRanges;
}

-(void)setTextContainer:(NSTextContainer *)container {
    if(container!=_textContainer){
        container=[container retain];
        [_textContainer release];
        _textContainer=container;
        
        if (_ownsTextStorage) {
            [_textStorage release];
            _ownsTextStorage=NO;
        }
        _textStorage = [[_textContainer layoutManager] textStorage];
    }
}

-(void)setTextContainerInset:(NSSize)size {
    _textContainerInset=size;
}

-(void)setUsesRuler:(BOOL)flag {
    _usesRuler=flag;
    [[self enclosingScrollView] setHasHorizontalRuler:_usesRuler];
    
    [self updateRuler];
}

-(void)setRulerVisible:(BOOL)flag {
    _rulerVisible=flag;
    [[self enclosingScrollView] setRulersVisible:_rulerVisible];
    
    [self updateRuler];
}

-(void)setUsesFontPanel:(BOOL)flag {
	_usesFontPanel = flag;
}

-(BOOL)usesFontPanel
{
	return _usesFontPanel;
}

-(void)setAllowsUndo:(BOOL)flag {
    _allowsUndo = flag;
}


-(void)setInsertionPointColor:(NSColor *)color {
    color=[color copy];
    [_insertionPointColor release];
    _insertionPointColor=color;
}

-(void)setTypingAttributes:(NSDictionary *)attributes {
    attributes=[attributes retain];
    [_typingAttributes release];
    _typingAttributes=attributes;
    
    [self updateRuler];
}

-(void)setDefaultParagraphStyle:(NSParagraphStyle *)paragraphStyle {
    paragraphStyle = [paragraphStyle copy];
    [_defaultParagraphStyle release];
    _defaultParagraphStyle = paragraphStyle;
}

-(void)setSelectedTextAttributes:(NSDictionary *)attributes {
    [_selectedTextAttributes autorelease];
    _selectedTextAttributes=[attributes retain];
}

- (NSRange)selectionRangeForProposedRange:(NSRange)range granularity:(NSSelectionGranularity)granularity {
    switch (granularity) {
        case NSSelectByCharacter:
            return range;
            
        case NSSelectByWord:
            return [_textStorage doubleClickAtIndex: range.location];
            
        case NSSelectByParagraph:
            return [[_textStorage string] lineRangeForRange:range];
    }
    
    return NSMakeRange(NSNotFound, 0);
}

-(void)setSelectedRange:(NSRange)range affinity:(NSSelectionAffinity)affinity stillSelecting:(BOOL)stillSelecting {
    NSArray *ranges=[NSArray arrayWithObject:[NSValue valueWithRange:range]];
    
    [self setSelectedRanges:ranges affinity:affinity stillSelecting:stillSelecting];
}

-(void)setSelectedRanges:(NSArray *)ranges affinity:(NSSelectionAffinity)affinity stillSelecting:(BOOL)stillSelecting {
    if([ranges count]==0)
        [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] ranges should not be empty",isa,_cmd];
    
	// Save the initial selected ranges so we can use it when we're done with the selection change
	if (_initialRanges == nil) {
		_initialRanges = [[self selectedRanges] copy];
	}
	
    if (stillSelecting == NO && [[ranges objectAtIndex:0] rangeValue].length==0)
        _selectionOrigin = [[ranges objectAtIndex:0] rangeValue].length;
    
    NSArray *oldRanges=[[_selectedRanges copy] autorelease];
    
	// Tell the world the selection will change when the selection change is done
	if (!stillSelecting) {
		ranges = [self _delegateChangeSelectionFromRanges:_initialRanges toRanges:ranges];
		[_initialRanges release];
		_initialRanges = nil;
	}
    [_selectedRanges setArray:ranges];
    
    _selectionAffinity=affinity;
    _selectionGranularity=NSSelectByCharacter;
    _insertionPointOn=[self shouldDrawInsertionPoint];
	
	if (![oldRanges isEqual:[self selectedRanges]]) {
		[self setNeedsDisplay:YES];
		[self _updateTypingAttributes];
	}
	
	// Tell the world the selection changed when the selection change is done
	if (!stillSelecting) {
		[[NSNotificationCenter defaultCenter] postNotificationName: NSTextViewDidChangeSelectionNotification  object: self userInfo:[NSDictionary dictionaryWithObject: [oldRanges objectAtIndex:0] forKey: NSOldSelectedCharacterRange]];
	}
	
	
    if(!_isContinuousSpellCheckingEnabled)
        [[self layoutManager] removeTemporaryAttribute:NSSpellingStateAttributeName forCharacterRange:NSMakeRange(0,[[self string] length])];
	
}

-(void)setSelectedRanges:(NSArray *)ranges {
    [self setSelectedRanges:ranges affinity:_selectionAffinity stillSelecting:NO];
}

- (NSRange)rangeForUserCompletion {
    NSRange range = [self selectedRange];
    unsigned index = [_textStorage nextWordFromIndex:range.location forward:NO];
    
    if (range.length != 0)
        return NSMakeRange(NSNotFound, 0);
    
    range.length = range.location - index;
    range.location = index;
    
    return range;
}

- (NSArray *)completionsForPartialWordRange:(NSRange)range indexOfSelectedItem:(int *)index {
    NSArray *result;
    // replace this with a real completion list source...
    NSArray *source = [NSArray arrayWithObjects:
                       @"selector", @"Class", @"Classic", @"object", @"objects d'art", @"objection", @"objectivism", @"objective-c", @"NSString", nil];
    NSMutableArray *completions = [NSMutableArray new];
    NSString *string = [[_textStorage string] substringWithRange:range];
    int i, count = [source count];
    
    *index = NSNotFound;
    
    // we will want to cache this if we are going to use a large list which changes infrequently.
    source = [source sortedArrayUsingSelector:@selector(compare:)];
    for (i = 0; i < count; ++i)
        if ([[source objectAtIndex:i] hasPrefix:string] == YES)
            [completions addObject:[source objectAtIndex:i]];
    
    if ([completions count] > 0)
        *index = 0;
    
    result=completions;
    
    if ([_delegate respondsToSelector:@selector(textView:completions:forPartialWordRange:indexOfSelectedItem:)])
        result = [_delegate textView:self completions:result forPartialWordRange:range indexOfSelectedItem:index];
    
    return result;
}

- (void)insertCompletion:(NSString *)string forPartialWordRange:(NSRange)range movement:(int)movement isFinal:(BOOL)isFinal {
    [self _replaceCharactersInRange:range withString:string];
    
    // is this proper behavior? i dunno
    _rangeForUserCompletion = NSMakeRange(range.location, [string length]);
    [self setSelectedRange:_rangeForUserCompletion];
    
    if (isFinal) {
        // i think that doing something "else", e.g, (just typing, clicking elsewhere) implies acceptance
        // of the user completion currently selected.
        if (movement == NSReturnTextMovement || movement == NSOtherTextMovement) {
            [self setSelectedRange:NSMakeRange(range.location + [string length], 0)];
            [self endUserCompletion];
        }
        else if (movement == NSCancelTextMovement)
            [self endUserCompletion];
    }
}

-(NSArray *)writablePasteboardTypes {
    return [NSArray arrayWithObjects:NSRTFPboardType,NSStringPboardType,nil];
}

-(BOOL)writeSelectionToPasteboard:(NSPasteboard *)pasteboard type:(NSString *)type {
    if([type isEqualToString:NSStringPboardType]) {
        [pasteboard setString:[[self string] substringWithRange:[self selectedRange]] forType:type];
        return YES;
    } else if ([type isEqualToString:NSRTFPboardType]) {
        NSData *rtfdata = [[self textStorage] RTFFromRange:[self selectedRange] documentAttributes:nil];
        [pasteboard setData:rtfdata forType:type];
        return YES;
    }
    return NO;
}

-(BOOL)writeSelectionToPasteboard:(NSPasteboard *)pasteboard types:(NSArray *)types {
    int i,count=[types count];
    
    [pasteboard declareTypes:types owner:nil];
    for(i=0;i<count;i++)
        if(![self writeSelectionToPasteboard:pasteboard type:[types objectAtIndex:i]])
            return NO;
    
    return YES;
}

-(NSRange)rangeForUserTextChange {
    if (_isEditable == NO)
        return NSMakeRange(NSNotFound, 0);
    
    return [self selectedRange];
}

-(NSRange)rangeForUserCharacterAttributeChange {
    if (_isEditable == NO)
        return NSMakeRange(NSNotFound, 0);
    
    if (_isRichText)
        return [self selectedRange];
    else
        return NSMakeRange(0, [[_textStorage string] length]);
}

-(NSRange)rangeForUserParagraphAttributeChange {
    if (_isEditable == NO)
        return NSMakeRange(NSNotFound, 0);
    
    if (_isRichText)
        return [[_textStorage string] lineRangeForRange:[self selectedRange]];
    else
        return NSMakeRange(0, [[_textStorage string] length]);
}

-(BOOL)shouldChangeTextInRange:(NSRange)changeInRange replacementString:(NSString *)replacementString {
    if(![self isEditable])
        return NO;
    
    return [self _delegateChangeTextInRange: changeInRange
                          replacementString: replacementString];
}

-(void)didChangeText {
    [[NSNotificationCenter defaultCenter] postNotificationName:NSTextDidChangeNotification object:self];
}

-(BOOL)shouldDrawInsertionPoint {
    if(![self isEditable])
        return NO;
    
    if([[self window] isKeyWindow]){
        if([[self window] firstResponder]==self)
            return YES;
    }
    
    return NO;
}

-(void)drawInsertionPointInRect:(NSRect)rect color:(NSColor *)color turnedOn:(BOOL)turnedOn {
    if(![[NSGraphicsContext currentContext] isDrawingToScreen])
        return;

    if(NSIsEmptyRect(rect))
        return;

    CGContextRef context=[[NSGraphicsContext currentContext] graphicsPort];

    CGContextSaveGState(context);
    rect.origin.x = roundf(rect.origin.x);
    rect.origin.y = roundf(rect.origin.y);
    CGContextClipToRect(context,rect);
    
    NSLayoutManager *layoutManager=[self layoutManager];
    NSPoint          origin=[self textContainerOrigin];
    
    NSRange          glyphRange;
    NSRect           glyphRect=rect;
    
    [_backgroundColor setFill];
    NSRectFill(rect);
    
    glyphRect.origin.x-=_textContainerInset.width;
    glyphRect.origin.y-=_textContainerInset.height;
    
    if(NSIntersectsRect(glyphRect,[[self layoutManager] extraLineFragmentRect])){
        glyphRange.location=[layoutManager numberOfGlyphs]-1;
        glyphRange.length=1;
    }
    else {
        glyphRange=[layoutManager glyphRangeForBoundingRect:glyphRect inTextContainer:[self textContainer]];
    }
    [layoutManager drawBackgroundForGlyphRange:glyphRange atPoint:origin];
    [layoutManager drawGlyphsForGlyphRange:glyphRange atPoint:origin];
    if(turnedOn)
        [[self graphicsStyle] drawTextViewInsertionPointInRect:rect color:color];
    
    CGContextRestoreGState(context);
}

-(NSRect)_viewRectForCharacterRange:(NSRange)range {
    NSPoint origin=[self textContainerOrigin];
    NSRect  result;
    
	if(range.length==0){
		if(range.location>=[_textStorage length]) {
			result=[[self layoutManager] extraLineFragmentRect];
			if (NSIsEmptyRect(result) && [_textStorage length]) {
				unsigned    rectCount=0;
				// Get the last used fragment rect
				range = NSMakeRange([_textStorage length]-1, 1);
                range = [[self layoutManager] glyphRangeForCharacterRange:range actualCharacterRange:NULL];
                result = [[self layoutManager] lineFragmentUsedRectForGlyphAtIndex:range.location effectiveRange:NULL];
                
                // Check the direction for that location
                uint8_t level;
                [[self layoutManager] getGlyphsInRange:range glyphs:NULL characterIndexes:NULL glyphInscriptions:NULL elasticBits:NULL bidiLevels:&level];
                if (level & 1) {
                    // Right to Left fragment
                    result.origin.x = NSMinX(result);
                } else {
                    // Left to right fragment
                    result.origin.x = NSMaxX(result);
                }
			}
			result.size.width=1;
		} else {
			unsigned    rectCount=0;
			NSRect * rectArray=[[self layoutManager] rectArrayForCharacterRange:range withinSelectedCharacterRange:range inTextContainer:[self textContainer] rectCount:&rectCount];
			result=rectArray[0];
		}
		result.size.width=1;
	} else {
		if(range.location>=[_textStorage length]) {
			result = [[self layoutManager] extraLineFragmentRect];
		} else {
			NSRange glyphRange=[[self layoutManager] glyphRangeForCharacterRange:range actualCharacterRange:NULL];
			
			result=[[self layoutManager] boundingRectForGlyphRange:glyphRange inTextContainer:[self textContainer]];
		}
	}
	
    result.origin.x+=origin.x;
    result.origin.y+=origin.y;
    
    return result;
}

-(void)_displayInsertionPointWithState:(BOOL)ison {
    _insertionPointOn=ison;
    
    [self lockFocus];
    [self drawInsertionPointInRect:_insertionPointRect color:_insertionPointColor turnedOn:_insertionPointOn];
    [self unlockFocus];
    [[self window] flushWindow];
}

-(void)_insertionPointTimerFired:(NSTimer *)timer {
    if([self shouldDrawInsertionPoint])
        [self _displayInsertionPointWithState:!_insertionPointOn];
}

-(void)updateInsertionPointStateAndRestartTimer:(BOOL)restartFlag {
    NSRange range=[self selectedRange];
    
    if(range.length>0)
        _insertionPointRect=NSZeroRect;
    else
        _insertionPointRect=[self _viewRectForCharacterRange:[self selectedRange]];
    
    if(restartFlag){
        float interval=[[NSDisplay currentDisplay] textCaretBlinkInterval];
        
        _insertionPointOn=[self shouldDrawInsertionPoint];
        [_insertionPointTimer invalidate];
        [_insertionPointTimer release];
        _insertionPointTimer=nil;
        
        _insertionPointTimer=[[NSTimer scheduledTimerWithTimeInterval:interval target:self selector:@selector(_insertionPointTimerFired:) userInfo:nil repeats:YES] retain];
        [[NSRunLoop currentRunLoop] addTimer:_insertionPointTimer forMode:NSModalPanelRunLoopMode];
    }
}

-(void)updateRuler {
    NSRulerView *ruler = [[self enclosingScrollView] horizontalRulerView];
    
    if(ruler!=nil){
        [ruler setOriginOffset:self.textContainerOrigin.x + self.textContainer.lineFragmentPadding];
        
        NSDictionary *typingAttributes = [self typingAttributes];
        NSParagraphStyle  *style=[typingAttributes objectForKey:NSParagraphStyleAttributeName];
        if(style==nil) {
            // This should be the NSTextView defaultParagraphStyle but it's not supported yet in Cocoton
            style=[NSParagraphStyle defaultParagraphStyle];
        }
        
        NSArray *markers = [[self layoutManager] rulerMarkersForTextView:self paragraphStyle:style ruler:ruler];
        [ruler setMarkers:markers];
    }
}

-(void)cut:sender {
    if([self selectedRange].length>0){
        if (! [self _delegateChangeTextInRange: [self selectedRange]
                             replacementString: @""])
            return;
        
        [self writeSelectionToPasteboard:[NSPasteboard generalPasteboard] types:[self writablePasteboardTypes]];
        
        [self _replaceCharactersInRange:[self selectedRange] withString:@""];
        [self didChangeText];
    }
}

-(void)copy:sender {
    if([self selectedRange].length>0){
        [self writeSelectionToPasteboard:[NSPasteboard generalPasteboard] types:[self writablePasteboardTypes]];
    }
}

-(void)paste:sender {
    id string = nil;
    
    NSPasteboard *pboard = [NSPasteboard generalPasteboard];
    if (_isRichText) {
        NSData *data = [pboard dataForType:NSRTFPboardType];
        if (data && data.length > 0) {
            string = [[[NSAttributedString alloc] initWithRTF:data documentAttributes:nil] autorelease];
        }
    }
    if (string == nil) {
        string = [pboard stringForType: NSStringPboardType];
    }
    if([string length]>0){
        if (! [self _delegateChangeTextInRange: [self selectedRange]
                             replacementString: string])
            return;
        
        [self _replaceCharactersInRange:[self selectedRange] withString:string useTypingAttributes:NO];
        [self didChangeText];
    }
}

-(void)selectAll:sender {
    [self setSelectedRange:NSMakeRange(0,[[self string] length])];
}

-(void)_textDidEndWithMovement:(NSInteger)movement {
    if ([_delegate respondsToSelector:@selector(textShouldEndEditing:)]) {
        if ([_delegate textShouldEndEditing:self] == NO) {
            return;
        }
    }
    [_insertionPointTimer invalidate];
    [_insertionPointTimer release];
    _insertionPointTimer=nil;
    
    NSDictionary *userInfo=[NSDictionary dictionaryWithObject:[NSNumber numberWithInt:movement] forKey:@"NSTextMovement"];
    
    NSNotification *note=[NSNotification notificationWithName:NSTextDidEndEditingNotification object:self userInfo:userInfo];
    
    _didSendTextDidEndNotification=YES;
    
    [[NSNotificationCenter defaultCenter] postNotification:note];
    
    _didSendTextDidEndNotification=NO;
}

- (void)insertTab:sender {
    if(_isFieldEditor){
        [self _textDidEndWithMovement:NSTabTextMovement];
        return;
    }
    
    if (_rangeForUserCompletion.location != NSNotFound) {
        _userCompletionSelectedItem++;
        if (_userCompletionSelectedItem >= [_userCompletions count])
            _userCompletionSelectedItem = 0;
        
        [self insertCompletion:[_userCompletions objectAtIndex:_userCompletionSelectedItem]
           forPartialWordRange:_rangeForUserCompletion
                      movement:NSTabTextMovement
                       isFinal:NO];
        return;
    }
    
    [self insertTabIgnoringFieldEditor:sender];
}

- (void)insertTabIgnoringFieldEditor:sender {
    [self insertText:@"\t"];
}

- (void)insertBacktab:sender {
    if (_isFieldEditor) {
        [self _textDidEndWithMovement:NSBacktabTextMovement];
    }
}

-(void)insertNewline:sender {
    if(_isFieldEditor){
        [self _textDidEndWithMovement:NSReturnTextMovement];
        return;
    }
    
    if (_rangeForUserCompletion.location != NSNotFound) {
        [self insertCompletion:[_userCompletions objectAtIndex:_userCompletionSelectedItem]
           forPartialWordRange:_rangeForUserCompletion
                      movement:NSReturnTextMovement
                       isFinal:YES];
        return;
    }
    
    [self insertNewlineIgnoringFieldEditor:sender];
}

- (void)insertNewlineIgnoringFieldEditor:sender {
    [self insertText:@"\n"];
}

- (void)cancel:sender {
    if (_rangeForUserCompletion.location != NSNotFound) {
        [self insertCompletion:_userCompletionHint forPartialWordRange:_rangeForUserCompletion
                      movement:NSCancelTextMovement isFinal:YES];
        return;
    }
}

-(void)_setAndScrollToRange:(NSRange)range upstream:(BOOL)upstream {
    [self setSelectedRange:range];
    if (upstream)
        [self scrollRangeToVisible:NSMakeRange(range.location, 0)];
    else
        [self scrollRangeToVisible:NSMakeRange(NSMaxRange(range), 0)];
}

-(void)_setAndScrollToRange:(NSRange)range {
    [self _setAndScrollToRange:range upstream:YES];
}

-(void)moveForward:sender {
    NSRange range=[self selectedRange];
    
    if(range.length>0){
        range.location=NSMaxRange(range);
        range.length=0;
    }
    else {
        unsigned length=[[self string] length];
        
        range.location++;
        if(range.location>length)
            range.location= length;
    }
    
    [self _setAndScrollToRange:range];
}

- (void)moveForwardAndModifySelection:sender {
    NSString *string = [_textStorage string];
    unsigned length = [string length];
    NSRange range = [self selectedRange];
    BOOL downstream = (range.location >= _selectionOrigin);
    
    if (downstream) {
        if (NSMaxRange(range) < length) {
            range.length++;
        }
    }
    else {
        range.location++;
        range.length--;
    }
    
    [self _setAndScrollToRange:range upstream:(downstream == NO)];
}

- (void)moveWordForward:sender {
    NSRange range = [self selectedRange];
    
    if (range.location < [[_textStorage string] length]) {
        range.location = [_textStorage nextWordFromIndex:range.location forward:YES];
        range.length = 0;
        
        [self _setAndScrollToRange:range];
    }
}

- (void)moveWordForwardAndModifySelection:sender {
    NSRange range = [self selectedRange];
    BOOL downstream = (range.location >= _selectionOrigin);
    
    if (downstream) {
        unsigned location = [_textStorage nextWordFromIndex:NSMaxRange(range) forward:YES];
        unsigned delta = location - NSMaxRange(range);
        
        range.length += delta;
    }
    else {
        unsigned location = [_textStorage nextWordFromIndex:range.location forward:YES];
        unsigned delta = location - range.location;
        
        range.location += delta;
        range.length -= delta;
        if (range.location >= _selectionOrigin) {
            range.location = _selectionOrigin;
            range.length = 0;
        }
    }
    
    [self _setAndScrollToRange:range upstream:(downstream == NO)];
}

-(void)moveDown:sender {
    NSString *string=[_textStorage string];
    unsigned  length=[string length];
    NSRange   range=[self selectedRange];
    
    if (_rangeForUserCompletion.location != NSNotFound) {
        if (_userCompletionSelectedItem < [_userCompletions count]-1)
            _userCompletionSelectedItem++;
        
        [self insertCompletion:[_userCompletions objectAtIndex:_userCompletionSelectedItem]
           forPartialWordRange:_rangeForUserCompletion
                      movement:NSDownTextMovement
                       isFinal:NO];
        return;
    }
    
    range.location=NSMaxRange(range);
    range.length=0;
    if(range.location<length){
        NSRange  line=[[self layoutManager] _softLineRangeForCharacterAtIndex:range.location];
        unsigned max=NSMaxRange(line);
        
        if(max>=length)
            range.location=length;
        else {
            NSRange  nextLine=[[self layoutManager] _softLineRangeForCharacterAtIndex:max+1];
            unsigned offset=range.location-line.location;
            
            range.location=nextLine.location+offset;
            if(range.location>=NSMaxRange(nextLine))
                range.location=NSMaxRange(nextLine);
        }
        
        [self _setAndScrollToRange:range];
    }
}

- (void)moveDownAndModifySelection:sender {
    unsigned  length=[[_textStorage string] length];
    NSRange   range=[self selectedRange];
    unsigned  delta;
    BOOL downstream = (range.location >= _selectionOrigin);
    
    delta = (downstream ? NSMaxRange(range) : range.location);
    
    if(delta<length){
        NSRange  line=[[self layoutManager] _softLineRangeForCharacterAtIndex:delta];
        unsigned max=NSMaxRange(line);
        
        if(max>=length)
            delta=length;
        else {
            NSRange  nextLine=[[self layoutManager] _softLineRangeForCharacterAtIndex:max+1];
            unsigned offset=delta-line.location;
            
            delta=nextLine.location+offset;
            if(delta>=NSMaxRange(nextLine))
                delta=NSMaxRange(nextLine);
        }
    }
    delta -= (downstream ? NSMaxRange(range) : range.location);
    
    if (downstream) {
        range.length += delta;
    }
    else {
        if (delta > range.length) {
            range.location = _selectionOrigin;
            range.length = 0;
        }
        else {
            range.location += delta;
            range.length -= delta;
        }
    }
    
    [self _setAndScrollToRange:range upstream:(downstream == NO)];
}

-(void)moveUp:sender {
    unsigned  length=[[_textStorage string] length];
    NSRange   range=[self selectedRange];
    
    if (_rangeForUserCompletion.location != NSNotFound) {
        if (_userCompletionSelectedItem > 0)
            _userCompletionSelectedItem--;
        
        [self insertCompletion:[_userCompletions objectAtIndex:_userCompletionSelectedItem]
           forPartialWordRange:_rangeForUserCompletion
                      movement:NSUpTextMovement
                       isFinal:NO];
        return;
    }
    
    range.length=0;
    
    if(range.location<=length){
        NSRange  line=[[self layoutManager] _softLineRangeForCharacterAtIndex:range.location];
        
        if(line.location>0){
            NSRange  prevLine=[[self layoutManager] _softLineRangeForCharacterAtIndex:line.location-1];
            unsigned offset=range.location-line.location;
            
            range.location=prevLine.location+offset;
            if(range.location>=NSMaxRange(prevLine)){
                if(NSMaxRange(prevLine)==0)
                    range.location=0;
                else
                    range.location=NSMaxRange(prevLine)-1;
            }
        }
        
        [self _setAndScrollToRange:range];
    }
}

- (void)moveUpAndModifySelection:sender {
    unsigned  length=[[_textStorage string] length];
    NSRange   range=[self selectedRange];
    int       delta;
    BOOL upstream = (NSMaxRange(range) <= _selectionOrigin);
    
    delta = (upstream ? range.location : NSMaxRange(range));
    
    if(delta<=length){
        NSRange  line=[[self layoutManager] _softLineRangeForCharacterAtIndex:delta];
        
        if(line.location>0){
            NSRange  prevLine=[[self layoutManager] _softLineRangeForCharacterAtIndex:line.location-1];
            unsigned offset=delta-line.location;
            
            
            delta=prevLine.location+offset;
            if(delta>=NSMaxRange(prevLine))
                delta=NSMaxRange(prevLine)-1;
        }
        else
            delta += line.length;
    }
    
    delta -= (upstream ? range.location : NSMaxRange(range));
    delta = abs(delta);
    
    if (upstream && range.location > 0) {
        range.location -= delta;
        range.length += delta;
    }
    else if (upstream == NO) {
        if (delta < range.length) {
            range.length -= delta;
        }
        else {
            range.location = _selectionOrigin;
            range.length = 0;
        }
    }
    
    [self _setAndScrollToRange:range upstream:upstream];
}

// Returns YES if the location is in a right-to-left paragraph
-(BOOL)_locationIsLeftToRight:(NSUInteger)location
{
    BOOL leftToRight = YES;
    NSString *string = [self string];
    NSUInteger length = [string length];
    if (length > 0) {
        // We'll check the bidi direction of the start of the paragraph and use that as our "left" direction
        // That's not great when mixing scripts with different writing directions but that's better than always going
        // left-to-right...
        
        if (location >= length) {
            location = length - 1;
        }
        NSRange paragraphRange = [string paragraphRangeForRange:NSMakeRange(location, 1)];
        // Check the direction for that location
        uint8_t level;
        [[self layoutManager] getGlyphsInRange:NSMakeRange(paragraphRange.location,1) glyphs:NULL characterIndexes:NULL glyphInscriptions:NULL elasticBits:NULL bidiLevels:&level];
        leftToRight = (level & 1) == 0;
    }
    return leftToRight;
}

-(void)moveLeft:sender {
    // That really should be more complex than that, because we should actually take the glyphs drawing order into account, which
    // is not just left-to-right or right-to-left
    NSRange range=[self selectedRange];
    if ([self _locationIsLeftToRight:range.location]) {
        [self moveBackward:sender];
    } else {
        [self moveForward:sender];
    }
}

-(void)moveRight:sender {
    // That really should be more complex than that, because we should actually take the glyphs drawing order into account, which
    // is not just left-to-right or right-to-left
    NSRange range=[self selectedRange];
    if ([self _locationIsLeftToRight:range.location]) {
        [self moveForward:sender];
    } else {
        [self moveBackward:sender];
    }
}

-(void)moveBackward:sender {
    NSRange range=[self selectedRange];
    
    if(range.length>0)
        range.length=0;
    else {
        unsigned length=[[self string] length];
        
        range.location--;
        if(range.location>length)
            range.location=0;
    }
    
    [self _setAndScrollToRange:range];
}

- (void)moveBackwardAndModifySelection:sender {
    NSRange   range = [self selectedRange];
    BOOL upstream = (NSMaxRange(range) <= _selectionOrigin);
    
    if (upstream) {
        if (range.location > 0) {
            range.location--;
            range.length++;
        }
    }
    else
        range.length--;
    
    [self _setAndScrollToRange:range upstream:upstream];
}

- (void)moveWordBackward:sender {
    NSRange range = [self selectedRange];
    
    if (range.location > 0) {
        if (range.location == [[_textStorage string] length]) // hrm
            range.location--;
        
        range.location = [_textStorage nextWordFromIndex:range.location forward:NO];
        range.length = 0;
        
        [self _setAndScrollToRange:range];
    }
}

- (void)moveWordBackwardAndModifySelection:sender {
    NSRange range = [self selectedRange];
    BOOL upstream = (NSMaxRange(range) <= _selectionOrigin);
    
    if (upstream) {
        unsigned location = [_textStorage nextWordFromIndex:range.location forward:NO];
        unsigned delta = range.location - location;
        
        range.location -= delta;
        range.length += delta;
    }
    else {
        unsigned location = [_textStorage nextWordFromIndex:NSMaxRange(range) forward:NO];
        unsigned delta = NSMaxRange(range) - location;
        
        if (delta < range.length)
            range.length -= delta;
        else
            range.length = 0;
    }
    
    [self _setAndScrollToRange:range upstream:upstream];
}

- (void)moveToBeginningOfDocument:sender {
    [self _setAndScrollToRange:NSMakeRange(0, 0)];
}

- (void)moveToEndOfDocument:sender {
    [self _setAndScrollToRange:NSMakeRange([_textStorage length], 0)];
}

- (void)moveToBeginningOfDocumentAndModifySelection:sender {
    NSRange range = [self selectedRange];
    
    if (range.length == 0)
        _selectionAffinity = NSSelectionAffinityUpstream;
    
    range.length += range.location;
    range.location = 0;
    
    [self setSelectedRange:range];
    [self scrollRangeToVisible:NSMakeRange(0, 0)];
}

- (void)moveToEndOfDocumentAndModifySelection:sender {
    NSRange range = [self selectedRange];
    unsigned length = [[_textStorage string] length];
    
    if (range.length == 0)
        _selectionAffinity = NSSelectionAffinityDownstream;
    
    range.length = length - range.location;
    
    [self setSelectedRange:range];
    [self scrollRangeToVisible:NSMakeRange(length, 0)];
}

- (void)scrollToBeginningOfDocument:sender {
    NSRect rect = [self frame];
    
    [self scrollPoint:rect.origin];
}

- (void)scrollToEndOfDocument:sender {
    NSRect rect = [self frame];
    
    rect.origin.y += rect.size.height;
    
    [self scrollPoint:rect.origin];
}

-(void)deleteForward:sender {
	NSRange range=[self selectedRange];
    
    if(range.length>0){
        if (! [self _delegateChangeTextInRange: range
                             replacementString: @""])
            return;
        
        [self _replaceCharactersInRange: range withString: @""];
        [self didChangeText];
    }
    else {
        range.length=1;
        
        if (! [self _delegateChangeTextInRange: range
                             replacementString: @""])
            return;
        
        if(NSMaxRange(range)<=[[self string] length]){		//TEST
            [self _replaceCharactersInRange:range withString:@"" allowsTypingCoalescing:YES];
            [self didChangeText];
        }
    }
    [self _setAndScrollToRange:NSMakeRange(range.location,0)];
}

-(void)deleteBackward:sender {
    NSRange range=[self selectedRange];
    
    if(range.length>0){
        
        if (! [self _delegateChangeTextInRange: range
                             replacementString: @""])
            return;
        
        [self _replaceCharactersInRange:range withString:@""];
        [self didChangeText];
    }
    else {
        if(range.location>0){
            range.location--;
            if (! [self _delegateChangeTextInRange: NSMakeRange(range.location, 1)
                                 replacementString: @""])
                return;
            
            [self _replaceCharactersInRange:NSMakeRange(range.location,1) withString:@"" allowsTypingCoalescing:YES];
            [self didChangeText];
        }
    }
}

// deleteToXXX methods use the kill buffer...
- (void)_appendStringToKillBuffer:(NSString *)string {
    if (_killBufferIsAdditive == YES) {
        if (_killBuffer != nil)
            [_killBuffer appendString:string];
        else
            _killBuffer = [string mutableCopy];
    }
    else {
        [_killBuffer release];
        _killBuffer = [string copy];
    }
}

// etc
- (BOOL)killBufferIsAdditive {
    return _killBufferIsAdditive;
}

- (void)setKillBufferAdditive:(BOOL)flag {
    _killBufferIsAdditive = flag;
    [_killBuffer release];
    _killBuffer = nil;
}

- (void)deleteToBeginningOfLine:sender {
    NSRange   range=[self selectedRange];
    NSRange   lineRange=[[self layoutManager] _softLineRangeForCharacterAtIndex:NSMaxRange(range)];
    unsigned delta = range.location - lineRange.location;
    
    if (range.length == 0) {
        range.location -= delta;
        range.length += delta;
    }
    
    if (! [self _delegateChangeTextInRange: range
                         replacementString: @""])
        return;
	
    [self _appendStringToKillBuffer:[[_textStorage string] substringWithRange:range]];
    [self _replaceCharactersInRange:range withString:@""];
    [self didChangeText];
}

- (void)deleteToEndOfLine:sender {
    NSRange   range=[self selectedRange];
    NSRange   lineRange=[[self layoutManager] _softLineRangeForCharacterAtIndex:NSMaxRange(range)];
    unsigned delta = NSMaxRange(lineRange) - NSMaxRange(range);
    
    if (range.length == 0)		// "implemented to delete the selection, if there is one, *OR*
        range.length += delta;		// all text to the end of a line...
    
    if (! [self _delegateChangeTextInRange: range
                         replacementString: @""])
        return;
	
    [self _appendStringToKillBuffer:[[_textStorage string] substringWithRange:range]];
    [self _replaceCharactersInRange:range withString:@""];
    [self didChangeText];
}

- (void)deleteToBeginningOfParagraph:sender {
    NSRange   range=[self selectedRange];
    NSRange   lineRange=[[_textStorage string] lineRangeForRange:range];
    unsigned  delta = range.location - lineRange.location;
    
    // i'm not killing the newline here, not sure how this should work.
    if (range.length == 0) {
        range.location -= delta;
        range.length += delta;
    }
    
    if (! [self _delegateChangeTextInRange: range
                         replacementString: @""])
        return;
	
    [self _appendStringToKillBuffer:[[_textStorage string] substringWithRange:range]];
    [self _replaceCharactersInRange:range withString:@""];
    [self didChangeText];
}

- (void)deleteToEndOfParagraph:sender {
    NSRange   range=[self selectedRange];
    NSRange   lineRange=[[_textStorage string] lineRangeForRange:range];
    unsigned  max=NSMaxRange(lineRange);
    
    if(max!=lineRange.location && max!=[[_textStorage string] length])
        max--;
    
    // i'm not killing the newline here, not sure how this should work.
    if (range.length == 0)
        range.length += (NSMaxRange(lineRange) - NSMaxRange(range)) - 1;
    
    if (! [self _delegateChangeTextInRange: range
                         replacementString: @""])
        return;
    
    [self _appendStringToKillBuffer:[[_textStorage string] substringWithRange:range]];
    [self _replaceCharactersInRange:range withString:@""];
    [self didChangeText];
}

// deleteToMark: is 5th and final kill-buffer-aware method (marks aren't implemented..)
// .. n.b. it doesn't make a lot of sense to me that deleteWordXXX doesn't use the kill buffer, but whatever...

- (void)yank:sender {
    if (_killBuffer != nil) {
        [self insertText:_killBuffer];
        
        if (_killBufferIsAdditive == YES) {
            [_killBuffer release];
            _killBuffer = nil;
        }
    }
}

- (void)deleteWordBackward:sender {
    NSRange range = [self selectedRange];
    unsigned nextWord = [_textStorage nextWordFromIndex:range.location forward:NO];
    
    if (range.length == 0) {
        unsigned delta = range.location - nextWord;
        
        range.location -= delta;
        range.length += delta;
    }
    
    if (! [self _delegateChangeTextInRange: range
                         replacementString: @""])
        return;
	
    [self _replaceCharactersInRange:range withString:@""];
    [self didChangeText];
}

- (void)deleteWordForward:sender {
    NSRange range = [self selectedRange];
    unsigned nextWord = [_textStorage nextWordFromIndex:range.location forward:YES];
    
    if (range.length == 0 && nextWord <= [[_textStorage string] length]) {
        unsigned delta = nextWord - range.location;
        
        range.length += delta;
    }
    
    if (! [self _delegateChangeTextInRange: range
                         replacementString: @""])
        return;
	
    [self _replaceCharactersInRange:range withString:@""];
    [self didChangeText];
}

-(void)clear:sender {
    [self deleteBackward:sender];
}

- (void)moveToBeginningOfLine:sender {
    NSLayoutManager *layoutManager = [self layoutManager];
    NSRange range = [layoutManager _softLineRangeForCharacterAtIndex:[self selectedRange].location];
    
    [self _setAndScrollToRange:NSMakeRange(range.location,0)];
}

- (void)moveToBeginningOfLineAndModifySelection:sender {
    NSRange selectedRange = [self selectedRange];
    NSRange lineRange;
    unsigned index;
    BOOL upstream = NO;
    
    if (NSMaxRange(selectedRange) <= _selectionOrigin) {
        upstream = YES;
        index = selectedRange.location;
    }
    else
        index = NSMaxRange(selectedRange);
    
    lineRange = [[self layoutManager] _softLineRangeForCharacterAtIndex:index];
    
    if (upstream) {
        unsigned delta = selectedRange.location - lineRange.location;
        
        selectedRange.location -= delta;
        selectedRange.length += delta;
    }
    else {
        int delta = NSMaxRange(selectedRange) - NSMaxRange(lineRange);
        
        if (delta > 0 && delta < selectedRange.length)
            selectedRange.length -= delta;
        else
            selectedRange.length = 0;
    }
    
    [self _setAndScrollToRange:selectedRange upstream:upstream];
}

- (void)moveToEndOfLine:sender {
    NSLayoutManager *layoutManager = [self layoutManager];
    NSRange range = [layoutManager _softLineRangeForCharacterAtIndex:[self selectedRange].location];
    
    [self _setAndScrollToRange:NSMakeRange(NSMaxRange(range), 0)];
}

- (void)moveToEndOfLineAndModifySelection:sender {
    NSRange selectedRange = [self selectedRange];
    NSRange lineRange;
    unsigned index;
    BOOL downstream = NO;
    
    if (selectedRange.location >= _selectionOrigin) {
        downstream = YES;
        index = NSMaxRange(selectedRange);
    }
    else
        index = selectedRange.location;
    
    lineRange = [[self layoutManager] _softLineRangeForCharacterAtIndex:index];
    
    if (downstream) {
        unsigned delta = NSMaxRange(lineRange) - NSMaxRange(selectedRange);
        
        selectedRange.length += delta;
    }
    else {
        int delta = NSMaxRange(lineRange) - selectedRange.location;
        
        if (delta > 0 && delta < selectedRange.length) {
            selectedRange.location += delta;
            selectedRange.length -= delta;
        }
        else {
            selectedRange.location = _selectionOrigin;
            selectedRange.length = 0;
        }
    }
    
    [self _setAndScrollToRange:selectedRange upstream:(downstream == NO)];
}

-(void)moveToBeginningOfParagraph:sender {
    // was moveToBeginningOfLine
    NSString *string=[_textStorage string];
    NSRange   range=[self selectedRange];
    NSRange   lineRange=[string lineRangeForRange:range];
    
    [self _setAndScrollToRange:NSMakeRange(lineRange.location,0)];
}

- (void)moveParagraphBackwardAndModifySelection:sender {
    NSRange range = [self selectedRange];
    BOOL upstream = (NSMaxRange(range) <= _selectionOrigin);
    
    if (upstream) {
        NSRange lineRange;
        unsigned delta = 0;
        
        lineRange = [[_textStorage string] lineRangeForRange:NSMakeRange(range.location, 0)];
        
        // ugh. newlines screw up the lineRangeForRange calculation.
        if (lineRange.location == range.location && range.location > 0)
            lineRange = [[_textStorage string] lineRangeForRange:NSMakeRange(range.location-1, 0)];
        
        delta = range.location - lineRange.location;
        
        range.location -= delta;
        range.length += delta;
    }
    else {
        NSRange lineRange = [[_textStorage string] lineRangeForRange:NSMakeRange(NSMaxRange(range)-1, 0)];
        unsigned delta = NSMaxRange(range) - lineRange.location;
        
        if (delta < range.length)
            range.length -= delta;
        else
            range.length = 0;
    }
    
    [self _setAndScrollToRange:range upstream:upstream];
}

-(void)moveToEndOfParagraph:sender {
    // was moveToEndOfLine
    NSString *string=[_textStorage string];
    NSRange   range=[self selectedRange];
    NSRange   lineRange=[string lineRangeForRange:range];
    unsigned  max=NSMaxRange(lineRange);
    
    if(max!=lineRange.location && max!=[string length])
        max--;
    
    [self _setAndScrollToRange:NSMakeRange(max,0)];
}

- (void)moveParagraphForwardAndModifySelection:sender {
    NSRange range = [self selectedRange];
    BOOL downstream = range.location >= _selectionOrigin;
    
    if (downstream) {
        NSRange lineRange = [[_textStorage string] lineRangeForRange:NSMakeRange(NSMaxRange(range), 0)];
        unsigned delta = NSMaxRange(lineRange) - NSMaxRange(range);
        
        range.length += delta;
    }
    else {
        NSRange lineRange = [[_textStorage string] lineRangeForRange:NSMakeRange(range.location, 0)];
        unsigned delta = NSMaxRange(lineRange) - range.location;
        
        if (delta < range.length) {
            range.location += delta;
            range.length -= delta;
        }
        else {
            range.location = _selectionOrigin;
            range.length = 0;
        }
    }
    
    [self _setAndScrollToRange:range upstream:(downstream == NO)];
}

- (void)centerSelectionInVisibleArea:sender {
    [self scrollRangeToVisible:[self selectedRange]];
}

/* Hmm: While the spec says: "scroll the receiver up (or forward) one page in its scroll view, also moving the insertion point to the top of the newly displayed page.", Apple's implementation (at least in PB and TextEdit) does exactly the opposite. I'm following the behavior that makes the Most Sense To Me. (dwy) */
- (void)scrollPageUp:sender {
    NSRect rect = [self visibleRect];
    
    rect.origin.y -= rect.size.height;
    [self scrollPoint:rect.origin];
}

-(unsigned)glyphIndexForPoint:(NSPoint)point fractionOfDistanceThroughGlyph:(float *)fraction {
    point.x-=_textContainerInset.width;
    point.y-=_textContainerInset.height;
    
    return [[self layoutManager] glyphIndexForPoint:point inTextContainer:_textContainer fractionOfDistanceThroughGlyph:fraction];
}

- (void)pageUp:sender {
    NSRect rect;
    NSRange range;
    float fraction;
    
    [self scrollPageUp:sender];
    
    rect = [self visibleRect];
    range.location = [self glyphIndexForPoint:NSMakePoint(NSMinX(rect), NSMinY(rect))
               fractionOfDistanceThroughGlyph:&fraction];
    range.length = 0;
    
    [self _setAndScrollToRange:range];
}

- (void)pageUpAndModifySelection:sender {
    NSRect rect;
    NSRange range = [self selectedRange];
    unsigned location;
    float fraction;
    BOOL upstream;
    
    [self scrollPageUp:sender];
    
    rect = [self visibleRect];
    location = [self glyphIndexForPoint:NSMakePoint(NSMinX(rect), NSMinY(rect)) fractionOfDistanceThroughGlyph:&fraction];
    
    upstream = (NSMaxRange(range) <= _selectionOrigin);
    if (upstream) {
        unsigned delta = range.location - location;
        
        range.location -= delta;
        range.length += delta;
    }
    else {
        int delta = NSMaxRange(range) - location;
        
        if (delta > 0 && delta < range.length) {
            range.length -= delta;
        }
        else {
            range.length = 0;
        }
    }
    
    [self _setAndScrollToRange:range upstream:upstream];
}

- (void)scrollPageDown:sender {
    NSRect rect = [self visibleRect];
    
    rect.origin.y += rect.size.height;
    [self scrollPoint:rect.origin];
}

- (void)pageDown:sender {
    NSString *string = [_textStorage string];
    NSRect rect;
    NSRange range;
    float fraction;
    
    [self scrollPageDown:sender];
    
    rect = [self visibleRect];
    range.location = [self glyphIndexForPoint:NSMakePoint(NSMinX(rect), NSMaxY(rect))
               fractionOfDistanceThroughGlyph:&fraction];
    if (range.location == 0)
        range.location = [string length];
    range.length = 0;
    
    [self _setAndScrollToRange:range];
}

- (void)pageDownAndModifySelection:sender {
    NSRect rect;
    NSRange range = [self selectedRange];
    unsigned location;
    float fraction;
    BOOL downstream;
    
    [self scrollPageDown:sender];
    
    rect = [self visibleRect];
    location = [self glyphIndexForPoint:NSMakePoint(NSMinX(rect), NSMaxY(rect))
         fractionOfDistanceThroughGlyph:&fraction];
    
    downstream = (range.location >= _selectionOrigin);
    if (downstream) {
        unsigned delta = location - NSMaxRange(range);
        
        range.length += delta;
    }
    else {
        int delta = location - range.location;
        
        if (delta > 0 && delta < range.length) {
            range.location += delta;
            range.length -= delta;
        }
        else {
            range.location = _selectionOrigin;
            range.length = 0;
        }
    }
    
    [self _setAndScrollToRange:range upstream:(downstream == NO)];
}

- (void)transpose:sender {
    NSString *string = [_textStorage string];
    NSRange range = [self selectedRange];
    
    if (range.length > 0)
        return;
    
    if (range.location > 0 && NSMaxRange(range) < [string length]) {
        NSString *a, *b;
        
        a = [string substringWithRange:NSMakeRange(range.location, 1)];
        b = [string substringWithRange:NSMakeRange(range.location-1, 1)];
        
        NSString *transposed = [b stringByAppendingString: a];
        
        if (! [self _delegateChangeTextInRange: NSMakeRange(range.location-1,2)
                             replacementString: transposed])
            return;
        
        [self _replaceCharactersInRange: NSMakeRange(range.location-1, 2)
                             withString: transposed useTypingAttributes: NO];
        [self _setAndScrollToRange:NSMakeRange(range.location+1, 0)];
    }
}

// ??? unclear as to how this works, seems to only make sense to me when on whitespace
// should ask _delegateChangeTextInRange:replacementString: and use _replaceCharactersInRange:withString:
- (void)transposeWords:sender {
    NSString *string = [_textStorage string];
    NSRange range = [self selectedRange];
    unsigned nextWord, previousWord;
    NSString *a, *b, *space;
    
    if (range.length > 0 || range.location == [string length])
        return;
    
    if ([[NSCharacterSet whitespaceCharacterSet] characterIsMember:[string characterAtIndex:range.location]] == NO)
        return;
    
    space = [string substringWithRange:NSMakeRange(range.location, 1)];
    [self replaceCharactersInRange:NSMakeRange(range.location, 1) withString:@""];
    
    nextWord = [_textStorage nextWordFromIndex:range.location forward:YES];
    previousWord = [_textStorage nextWordFromIndex:range.location forward:NO];
    
    a = [string substringWithRange:NSMakeRange(previousWord, range.location-previousWord)];
    b = [string substringWithRange:NSMakeRange(range.location, nextWord-range.location)];
    [self replaceCharactersInRange:NSMakeRange(previousWord, nextWord-previousWord) withString:@""];
    
    [self replaceCharactersInRange:NSMakeRange(previousWord, 0) withString:b];
    [self replaceCharactersInRange:NSMakeRange(previousWord+[b length], 0) withString:space];
    [self replaceCharactersInRange:NSMakeRange(previousWord+[b length]+1, 0) withString:a];
    
    
    [self _setAndScrollToRange:NSMakeRange(nextWord+1, 0)];
}

- (void)complete:sender {
    if (_rangeForUserCompletion.location != NSNotFound) {		// we're already in completion mode
        NSLog(@"userCompletion STATE MISMATCH!!!");
        return;
    }
    
    _rangeForUserCompletion = [self rangeForUserCompletion];
    _userCompletionHint = [[[_textStorage string] substringWithRange:_rangeForUserCompletion] retain];
    _userCompletions = [[self completionsForPartialWordRange:_rangeForUserCompletion
                                         indexOfSelectedItem:&_userCompletionSelectedItem] retain];
    
    if ([_userCompletions count] == 0) {
        [self endUserCompletion];
        NSBeep();
        return;
    }
    
    // Hmm, what should be the initial text movement for the completion editor? Docs are unclear
    [self insertCompletion:[_userCompletions objectAtIndex:_userCompletionSelectedItem]
       forPartialWordRange:_rangeForUserCompletion
                  movement:NSOtherTextMovement
                   isFinal:NO];
}

- (void)endUserCompletion {
    _rangeForUserCompletion = NSMakeRange(NSNotFound, 0);
    [_userCompletionHint release];
    _userCompletionHint = nil;
    [_userCompletions release];
    _userCompletions = nil;
}

-(void)toggleRuler:sender {
    BOOL flag = ![self usesRuler];
    
    [self setUsesRuler:flag];
    [self setRulerVisible:flag];
    
    [self updateRuler];
    
    if ([sender isKindOfClass:[NSMenuItem class]]) {
		if (flag) {
			[sender setTitle: NSLocalizedStringFromTableInBundle(@"Hide Ruler", nil, [NSBundle bundleForClass: [NSTextView class]], @"Hide the page ruler")];
		} else {
			[sender setTitle: NSLocalizedStringFromTableInBundle(@"Show Ruler", nil, [NSBundle bundleForClass: [NSTextView class]], @"Show the page ruler")];
		}
	}
}


-(void)copyRuler:sender {
    NSUnimplementedMethod();
}

-(void)pasteRuler:sender {
    NSUnimplementedMethod();
}

// NSText

-delegate {
    return _delegate;
}

-(NSString *)string {
    return [_textStorage string];
}

-(NSData *)RTFFromRange:(NSRange)range {
    return [NSRichTextWriter dataWithAttributedString:_textStorage range:range];
}

-(NSData *)RTFDFromRange:(NSRange)range {
    return [NSRichTextWriter dataWithAttributedString:_textStorage range:range];
}

-(BOOL)isEditable {
    return _isEditable;
}

-(BOOL)isSelectable {
    return _isSelectable;
}

-(BOOL)isRichText {
    return _isRichText;
}

-(BOOL)isFieldEditor {
    return _isFieldEditor;
}

-(NSFont *)font {
    return _font;
}

-(NSTextAlignment)alignment {
    return _textAlignment;
}

-(NSColor *)textColor {
    return _textColor;
}

-(BOOL)drawsBackground {
    return _drawsBackground;
}

-(NSColor *)backgroundColor {
    return _backgroundColor;
}

-(BOOL)isHorizontallyResizable {
    return _isHorizontallyResizable;
}

-(BOOL)isVerticallyResizable {
    return _isVerticallyResizable;
}

-(NSSize)maxSize {
    return _maxSize;
}

-(NSSize)minSize {
    return _minSize;
}

-(NSRange)selectedRange {
    if([_selectedRanges count]==0)
        return NSMakeRange(0,0);
    
    return [[_selectedRanges objectAtIndex:0] rangeValue];
}

-(void)setDelegate:delegate {
    if(delegate==_delegate)
        return;
    
    NSNotificationCenter *center=[NSNotificationCenter defaultCenter];
    struct {
        SEL       selector;
        NSString *name;
    } notes[]={
        { @selector(textDidBeginEditing:),NSTextDidBeginEditingNotification },
        { @selector(textDidEndEditing:), NSTextDidEndEditingNotification },
        { @selector(textDidChange:), NSTextDidChangeNotification },
        { @selector(textViewDidChangeSelection:), NSTextViewDidChangeSelectionNotification },
        { NULL, nil }
    };
    int i;
    
    if(_delegate!=nil)
        for(i=0;notes[i].selector!=NULL;i++)
            [center removeObserver:_delegate name:notes[i].name object:self];
    
    _delegate=delegate;
    
    for(i=0;notes[i].selector!=NULL;i++)
        if([_delegate respondsToSelector:notes[i].selector])
            [center addObserver:_delegate selector:notes[i].selector name:notes[i].name object:self];
}

-(void)setString:(NSString *)string {
    if (! [self _delegateChangeTextInRange: NSMakeRange(0,[_textStorage length])
                         replacementString: string])
        return;
    
    [self _replaceCharactersInRange:NSMakeRange(0,[_textStorage length]) withString:string];
    // this does not didChangeText
}

- (void)updateFontPanel
{
	if ([self usesFontPanel] == NO) {
		return;
	}
	
	NSRange selectedRange = [self selectedRange];
	if (selectedRange.length == 0) {
		// Use the font from the typing attributes
		NSFont *font = [_typingAttributes objectForKey:NSFontAttributeName];
		if (font) {
			[[NSFontManager sharedFontManager] setSelectedFont:font isMultiple:NO];
		}
	} else {
		// Use the font at the selection point and check if the selection is using any other font
		NSRange effectiveRange;
		NSFont *font = [_textStorage attribute:NSFontAttributeName atIndex:selectedRange.location effectiveRange:&effectiveRange];
		BOOL isMultiple = NSMaxRange(effectiveRange) < NSMaxRange(selectedRange);
		if (font) {
			[[NSFontManager sharedFontManager] setSelectedFont:font isMultiple:isMultiple];
		}
	}
}

// Update the typing attributes according to the current selection
- (void)_updateTypingAttributes
{
	if (_isRichText == NO) {
		NSMutableParagraphStyle *style = [[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
		[style setAlignment: _textAlignment];
		NSDictionary *attributes = [NSDictionary dictionaryWithObjectsAndKeys:
									_font, NSFontAttributeName,
									style, NSParagraphStyleAttributeName,
									nil];
		if (attributes && [attributes isEqualToDictionary:[self typingAttributes]] == NO) {
			[self setTypingAttributes:attributes];
		}
		return;
	}
	// Update the typing attributes according to the start point of the selection if the current string isn't empty
	int length = [[self textStorage] length];
	if (length > 0) {
		int rangeLength = 0;
		NSArray *ranges = [self selectedRanges];
		if  ([ranges count] && length) {
			int min = length + 1;
			for (int i = 0; i < [ranges count]; ++i) {
				NSRange range = [[ranges objectAtIndex:i] rangeValue];
				if (range.location < min) {
					min = range.location;
					rangeLength = range.length;
				}
			}
			if (min > 0 && rangeLength == 0) {
				// Use attributes of the char just before the insertion point when we just have an insert point
				min = min - 1;
			}
			if (min >= 0 && min < length) {
				NSDictionary *attributes = [[self textStorage] attributesAtIndex:min effectiveRange:NULL];
				if (attributes && [attributes isEqualToDictionary:[self typingAttributes]] == NO) {
					[self setTypingAttributes:attributes];
				}
			}
		}
		[self updateFontPanel];
	}
}

// Should this be related to typingAttributes somehow?
-(NSDictionary *)_stringAttributes {
    NSMutableDictionary *result=[NSMutableDictionary dictionary];
    
    [result setObject:_font forKey: NSFontAttributeName];
    [result setObject:_textColor forKey:NSForegroundColorAttributeName];
    return result;
}

// Add attributes with undo support
-(void)_addAttributes:(NSDictionary *)attributes range:(NSRange)range
{
    if (_allowsUndo && self.undoManager) {
        [self breakUndoCoalescing];
        
        NSUndoSetAttributes *undoSetAttributes = [[[NSUndoSetAttributes alloc] initWithAffectedRange:range layoutManager:self.layoutManager undoManager:self.undoManager] autorelease];
        [[self.undoManager prepareWithInvocationTarget:undoSetAttributes] undoRedo:self.textStorage];
        
    }
    [[self textStorage] addAttributes:attributes range:range];
}

-(void)_addAttribute:(NSString *)key value:(id)value range:(NSRange)range
{
    NSDictionary *attributes = [NSDictionary dictionaryWithObject:value forKey:key];
    [self _addAttributes:attributes range:range];
}

// Remove attributes with undo support
-(void)_removeAttribute:(NSString *)key range:(NSRange)range
{
    if (_allowsUndo && self.undoManager) {
        [self breakUndoCoalescing];
        
        NSUndoSetAttributes *undoSetAttributes = [[[NSUndoSetAttributes alloc] initWithAffectedRange:range layoutManager:self.layoutManager undoManager:self.undoManager] autorelease];
        [[self.undoManager prepareWithInvocationTarget:undoSetAttributes] undoRedo:self.textStorage];
        
    }
    [[self textStorage] removeAttribute:key range:range];
}

-(void)replaceCharactersInRange:(NSRange)range withString:(NSString *)string {
    if (! [self _delegateChangeTextInRange: range
                         replacementString: string])
        return;
    
    [self _replaceCharactersInRange: range withString: string useTypingAttributes: NO];
}

// Replace characters with undo support (and with coalescing typing events if allowsTypingCoalescing is YES and _processingKeyEvent is YES
-(void)_replaceCharactersInRange:(NSRange)range withString:(id)string useTypingAttributes:(BOOL)useTypingAttributes allowsTypingCoalescing:(BOOL)allowsTypingCoalescing {
    NSUndoManager * undoManager = [self undoManager];
    
    if (_firstResponderButNotEditingYet)
    {
        [[NSNotificationCenter defaultCenter] postNotificationName:NSTextDidBeginEditingNotification
                                                            object:self];
        _firstResponderButNotEditingYet = NO;
    }
    
    if (_allowsUndo)
    {
        if (_processingKeyEvent && allowsTypingCoalescing) {
            if (_undoTyping) {
                // Try first to coaelesce with current undo typing object
                if ([_undoTyping coalesceAffectedRange:range replacementRange:NSMakeRange(range.location, [string length]) selectedRange:[self selectedRange] text:_textStorage] == NO) {
                    [self breakUndoCoalescing];
                }
            }
            if (_undoTyping == nil) {
                _undoTyping = [[NSUndoTyping alloc] initWithAffectedRange:range layoutManager:self.layoutManager undoManager:self.undoManager replacementRange:NSMakeRange(range.location, [string length])];
                [[self.undoManager prepareWithInvocationTarget:_undoTyping] undoRedo:self.textStorage];
            }
        } else {
            [self breakUndoCoalescing];
            
            NSUndoReplaceCharacters *replace = [[[NSUndoReplaceCharacters alloc] initWithAffectedRange:range layoutManager:self.layoutManager undoManager:self.undoManager replacementRange:NSMakeRange(range.location, [string length])] autorelease];
            [[self.undoManager prepareWithInvocationTarget:replace] undoRedo:self.textStorage];
        }
    }
	if (_isRichText) {
		NSAttributedString *attrString = nil;
		// Use the typing attributes for the inserted string
		if ([string isKindOfClass: [NSAttributedString class]]) {
            attrString = string;
            if (useTypingAttributes) {
                // We're going to merge the attributes
                NSMutableAttributedString* mutableAttrString = [[string mutableCopy] autorelease];
                [mutableAttrString addAttributes: [self typingAttributes] range: NSMakeRange(0, [string length])];
                attrString = mutableAttrString;
            }
		} else {
			attrString = [[[NSAttributedString alloc] initWithString:string attributes:[self typingAttributes]] autorelease];
		}
		[_textStorage replaceCharactersInRange:range withAttributedString:attrString];
	} else {
        // Even though we're not rich text we have to set the typing attributes if we want the layout
        // manager to honor the font settings of this text view. If the layout manager doesn't find a font in the attributed
        // string it'll substitute 12 pt sans causing layout trouble for mini text fields and other woe.
        if ([string isKindOfClass:[NSAttributedString class]]) {
            NSAttributedString *str = (NSAttributedString *)string;
            string = [str string];
        }
        NSAttributedString *attrString = [[[NSAttributedString alloc] initWithString:string attributes:[self typingAttributes]] autorelease];
		[_textStorage replaceCharactersInRange:range withAttributedString:attrString];
	}
    
	// TODO: this needs to be optimized to check the changed range expanded (probably to paragraphs)
	[self _continuousSpellCheckWithInvalidatedRange:NSMakeRange(range.location, [string length])];
    
    [self setSelectedRange:NSMakeRange(range.location+[string length],0)];
}

-(void)_replaceCharactersInRange:(NSRange)range withString:(id)string {
	[self _replaceCharactersInRange:range withString:string useTypingAttributes: YES allowsTypingCoalescing:NO];
}

-(void)_replaceCharactersInRange:(NSRange)range withString:(id)string useTypingAttributes:(BOOL)useTypingAttributes {
    [self _replaceCharactersInRange:range withString:string useTypingAttributes:useTypingAttributes allowsTypingCoalescing:NO];
}

-(void)_replaceCharactersInRange:(NSRange)range withString:(id)string allowsTypingCoalescing:(BOOL)allowsTypingCoalescing {
    [self _replaceCharactersInRange:range withString:string useTypingAttributes:YES allowsTypingCoalescing:allowsTypingCoalescing];
}

-(BOOL)readRTFDFromFile:(NSString *)path {
	NSAttributedString *contents = [NSRichTextReader attributedStringWithContentsOfFile:path];
	
    [_textStorage setAttributedString:contents];
    
    return contents != nil;
}

-(void)replaceCharactersInRange:(NSRange)range withRTF:(NSData *)rtf {
    NSAttributedString *astring=[NSRichTextReader attributedStringWithData:rtf];
    
    if (! [self _delegateChangeTextInRange: range
                         replacementString: [astring string]])
        return;
	
    [self _replaceCharactersInRange:range withString:astring useTypingAttributes:NO];
    [self setSelectedRange:NSMakeRange(range.location+[astring length],0)];
}

-(void)replaceCharactersInRange:(NSRange)range withRTFD:(NSData *)rtfd {
    NSAttributedString *astring=[NSRichTextReader attributedStringWithData:rtfd];
    
    if (! [self _delegateChangeTextInRange: range
                         replacementString: [astring string]])
        return;
	
    [self _replaceCharactersInRange:range withString:astring useTypingAttributes:NO];
    [self setSelectedRange:NSMakeRange(range.location+[astring length],0)];
}

-(void)setEditable:(BOOL)flag {
    _isEditable=flag;
}

-(void)setSelectable:(BOOL)flag {
    _isSelectable=flag;
}

-(void)setRichText:(BOOL)flag {
    _isRichText=flag;
}

-(void)setFieldEditor:(BOOL)flag {
    _isFieldEditor=flag;
}

-(void)setFont:(NSFont *)font {
    [font retain];
    [_font release];
    _font=font;
    [self _addAttribute:NSFontAttributeName value:_font range:NSMakeRange(0,[[self textStorage] length])];
    [self _updateTypingAttributes];
}

-(void)setFont:(NSFont *)font range:(NSRange)range {
    if (NSMaxRange(range) >= [[self textStorage] length])
    {
        [font retain];
        [_font release];
        _font=font;
    }
    [self _addAttribute:NSFontAttributeName value:font range:range];
	[self _updateTypingAttributes];
}

-(NSRange)_rangeForSelectedParagraph {
    if(!_isRichText)
        return NSMakeRange(0, [[self textStorage] length]);
    else {
        NSRange range=[self selectedRange];
        
        return [[[self textStorage] string] paragraphRangeForRange:range];
    }
}

-(void)_setAlignment:(NSTextAlignment)alignment range:(NSRange)range {
    NSMutableParagraphStyle *style=nil;
    
    if([[self textStorage] length]>0)
        style=[[[[self textStorage] attribute:NSParagraphStyleAttributeName
                                      atIndex:range.location
                               effectiveRange:NULL] mutableCopy] autorelease];
    
    if(style==nil)
        style=[[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
    
    [style setAlignment:alignment];
    
    [self _addAttribute:NSParagraphStyleAttributeName value:style range:range];
	
	// This method is always been called with the selected range - so update the typing attributes with that
	NSMutableDictionary *attributes = [[[self typingAttributes] mutableCopy] autorelease];
	[attributes setObject:style forKey:NSParagraphStyleAttributeName];
	[self setTypingAttributes:attributes];
}

-(void)setAlignment:(NSTextAlignment)alignment {
    [self _setAlignment:alignment range:NSMakeRange(0, [[self textStorage] length])];
    _textAlignment=alignment;
}

-(void)setTextColor:(NSColor *)color {
    color=[color copy];
    [_textColor release];
    _textColor=color;
    [self setTextColor:_textColor range:NSMakeRange(0, [[self textStorage] length])];
    
	[self _updateTypingAttributes];
}

-(void)setTextColor:(NSColor *)color range:(NSRange)range {
    if(color==nil)
        [self _removeAttribute:NSForegroundColorAttributeName range:range];
    else
        [self _addAttribute:NSForegroundColorAttributeName value:color range:range];
	
	[self _updateTypingAttributes];
}

-(void)setDrawsBackground:(BOOL)flag {
    _drawsBackground=flag;
}

-(void)setBackgroundColor:(NSColor *)color {
    color=[color copy];
    [_backgroundColor release];
    _backgroundColor=color;
}

-(void)setHorizontallyResizable:(BOOL)flag {
    _isHorizontallyResizable=flag;
	[_textContainer setWidthTracksTextView:flag];
}

-(void)setVerticallyResizable:(BOOL)flag {
    _isVerticallyResizable=flag;
	[_textContainer setHeightTracksTextView:flag];
}

-(void)setMaxSize:(NSSize)size {
    _maxSize=size;
}

-(void)setMinSize:(NSSize)size {
    _minSize=size;
}


-(void)setSelectedRange:(NSRange)range {
    [self setSelectedRange:range affinity:_selectionAffinity stillSelecting:NO];
}

// This should be done in NSTextContainer with notifications
-(void)_configureTextContainerSize {
    NSSize containerSize= [[self textContainer] containerSize];

    if([self isHorizontallyResizable] == NO)
        containerSize.width=[self bounds].size.width-_textContainerInset.width*2;

    if([self isVerticallyResizable] == NO)
        containerSize.height=[self bounds].size.height-_textContainerInset.height*2;

    [[self textContainer] setContainerSize:containerSize];
}

-(void)sizeToFit {
    NSRect usedRect,extraRect;
    NSSize size;
    
    [self _configureTextContainerSize];
    
    usedRect=[[self layoutManager] usedRectForTextContainer:[self textContainer]];
    extraRect=[[self layoutManager] extraLineFragmentUsedRect];
	if (!NSEqualRects(extraRect, NSZeroRect)) {
		extraRect.size.width=NSMaxX(usedRect)-NSMinX(extraRect);
		usedRect=NSUnionRect(usedRect,extraRect);
	}
	size=usedRect.size;
    size.width+=_textContainerInset.width*2;
    size.height+=_textContainerInset.height*2;
    
    if(![self isHorizontallyResizable])
        size.width=[self frame].size.width;
    else
        size.width=MAX([self frame].size.width,size.width);
    
    if(![self isVerticallyResizable])
        size.height=[self frame].size.height;
    else {
        
        NSClipView *clipView=(NSClipView *)[self superview];
        
        if([clipView isKindOfClass:[NSClipView class]]){
            // if we're in a clip view we should at be at least as big as the clip view
            if(size.height<[clipView bounds].size.height)
                size.height=[clipView bounds].size.height;
            if(size.width<[clipView bounds].size.width)
                size.width=[clipView bounds].size.width;
        }
        else {
            // we should at least be our frame size if we're not in a clip view
            size.height=MAX([self frame].size.height,size.height);
        }
    }
    if([self isHorizontallyResizable] || [self isVerticallyResizable])
        [self setFrameSize:size];
}

-(void)scrollRangeToVisible:(NSRange)range {
    NSRect rect=[self _viewRectForCharacterRange:range];
    
    rect=NSInsetRect(rect,-2,-2);
    [self scrollRectToVisible:rect];
}

-(void)changeFont:sender {
    if(![self isRichText]) {
        NSFont *font=[sender convertFont:[self font]];
        [self setFont:font];
    } else {
        NSRange range = [self rangeForUserCharacterAttributeChange];
        
        if (range.location == NSNotFound) {
            // Nothing to do
            return;
        }
        
        if (![self shouldChangeTextInRange: range  replacementString:nil]) {
            return;
        }
        
        int nextCharIndex = range.location;
        int maxCharIndex = NSMaxRange(range);
        
        // Get each block of font attribute and switch to the font using the one converted by the
        // sender
        [_textStorage beginEditing];
        
        while (nextCharIndex <= maxCharIndex) {
            NSRange effectiveRange = NSMakeRange(0, 0);
            
            NSFont *font = [_textStorage attribute: NSFontAttributeName atIndex: nextCharIndex effectiveRange: &effectiveRange];
            font = [sender convertFont:font];
            if (font) {
                NSRange changeRange = NSIntersectionRange(effectiveRange, range);
                [_textStorage addAttribute: NSFontAttributeName value: font range: changeRange];
            }
            if (effectiveRange.location != NSNotFound && effectiveRange.length > 0) {
                nextCharIndex = NSMaxRange(effectiveRange);
            } else {
                nextCharIndex++;
            }
        }
        [_textStorage endEditing];
        
        [self didChangeText];
    }
    NSFont *font = [[self typingAttributes] objectForKey: NSFontAttributeName];
    if (font) {
        font = [sender convertFont:font];
        if (font) {
            NSMutableDictionary *attributes = [[[self typingAttributes] mutableCopy] autorelease];
            [attributes setObject:font forKey:NSFontAttributeName];
            [self setTypingAttributes:attributes];
        }
    }
}

// making changes to textstorage attributes seems to wipe out the selection in this codebase,
// the setSelectedRange calls shouldn't be neccessary
- (void)alignCenter:sender {
    NSRange range=[self rangeForUserParagraphAttributeChange];
    if (range.location == NSNotFound) {
        return;
    }
    [self _setAlignment:NSCenterTextAlignment range:[self _rangeForSelectedParagraph]];
    [self setSelectedRange:range];
}

- (void)alignLeft:sender {
    NSRange range=[self rangeForUserParagraphAttributeChange];
    if (range.location == NSNotFound) {
        return;
    }
    [self _setAlignment:NSLeftTextAlignment range:[self _rangeForSelectedParagraph]];
    [self setSelectedRange:range];
}

- (void)alignRight:sender {
    NSRange range=[self rangeForUserParagraphAttributeChange];
    if (range.location == NSNotFound) {
        return;
    }
    [self _setAlignment:NSRightTextAlignment range:[self _rangeForSelectedParagraph]];
    [self setSelectedRange:range];
}

- (void)alignJustified:sender {
    NSRange range=[self rangeForUserParagraphAttributeChange];
    if (range.location == NSNotFound) {
        return;
    }
    [self _setAlignment:NSJustifiedTextAlignment range:[self _rangeForSelectedParagraph]];
    [self setSelectedRange:range];
}

- (void)underline:sender {
    NSRange range = [self rangeForUserCharacterAttributeChange];

    if (range.location == NSNotFound) {
        return;
    }
    id underline = [_textStorage attribute:NSUnderlineStyleAttributeName atIndex:range.location effectiveRange:NULL];
    if (underline) {
        [self _removeAttribute:NSUnderlineStyleAttributeName
                      range:range];        
    } else {
        [self _addAttribute:NSUnderlineStyleAttributeName
                      value:[NSNumber numberWithInt:NSUnderlineStyleSingle]
                      range:range];
    }
    [self setSelectedRange:range];
	
	NSMutableDictionary *attributes = [[[self typingAttributes] mutableCopy] autorelease];
	[attributes setObject:[NSNumber numberWithInt:NSUnderlineStyleSingle] forKey:NSUnderlineStyleAttributeName];
	[self setTypingAttributes:attributes];
}


-(BOOL)becomeFirstResponder {
    [self updateInsertionPointStateAndRestartTimer:YES];
    if(![self needsDisplay] && [self shouldDrawInsertionPoint])
        [self _displayInsertionPointWithState:YES];
    else
        _insertionPointOn = NO;
    _firstResponderButNotEditingYet = YES;
	_didSendTextDidEndNotification = NO;
	
    NSScrollView *enclosingScrollView = [self enclosingScrollView];
    if ([self usesRuler] == NO) {
        BOOL usesRuler = [enclosingScrollView rulersVisible];
        [self setUsesRuler:usesRuler];
    }
    if ([self usesRuler]) {
        [[enclosingScrollView horizontalRulerView] setClientView:self];
        [self updateRuler];
    }
    
    return YES;
}

-(BOOL)resignFirstResponder {
    if ([super resignFirstResponder] == NO) {
        return NO;
    }
    if (_isEditable)
        if ([_delegate respondsToSelector:@selector(textShouldEndEditing:)])
            if ([_delegate textShouldEndEditing:self] == NO)
                return NO;
    
    if ([self usesRuler]) {
        [[[self enclosingScrollView] horizontalRulerView] setClientView:nil];
    }
    
    if(_insertionPointTimer){
        [self _displayInsertionPointWithState:NO];
        [_insertionPointTimer invalidate];
        [_insertionPointTimer release];
        _insertionPointTimer=nil;
    }
    
    if(!_didSendTextDidEndNotification){
        // Let's remember that we've notified before it goes out - otherwise we could come back here again as a result
        // of the notification and end up in a death spiral.
        _didSendTextDidEndNotification=YES;
        NSNotification *note=[NSNotification notificationWithName:NSTextDidEndEditingNotification object:self userInfo:nil];
        
        [[NSNotificationCenter defaultCenter] postNotification:note];
        _didSendTextDidEndNotification=NO;
    }
    
    return YES;
}

-(void)becomeKeyWindow {
    if([self shouldDrawInsertionPoint]){
        [self updateInsertionPointStateAndRestartTimer:YES];
        [self _displayInsertionPointWithState:YES];
    }
}

-(void)resignKeyWindow {
    if(_insertionPointTimer){
        [self _displayInsertionPointWithState:NO];
        [_insertionPointTimer invalidate];
        [_insertionPointTimer release];
        _insertionPointTimer=nil;
    }
}

-(void)resizeWithOldSuperviewSize:(NSSize)oldSize {
    [super resizeWithOldSuperviewSize:oldSize];
    [self sizeToFit];
}

-(void)setFrame:(NSRect)frame {
	[super setFrame:frame];
	[self _configureTextContainerSize];
}

-(void)setFrameSize:(NSSize)size {
	[super setFrameSize:size];
	[self _configureTextContainerSize];
}

-(void)insertText:(id)object {
#if 0
    // nb I don't think I like this behavior. If this is un-ifdef'ed, any key will accept the current user
    // completion string and move the insertion point beyond the chosen completion. it looked OK in a regular
    // NSTextView, in a field editor it looks awful. see also the doCommandBySelector stuff for performClick:
    // (currently bound to space), somehow I think a space makes sense as "accept completion", but some
    // alphanumeric doesn't.
    if (_rangeForUserCompletion.location != NSNotFound) {
        [self insertCompletion:[_userCompletions objectAtIndex:_userCompletionSelectedItem]
           forPartialWordRange:_rangeForUserCompletion
                      movement:NSOtherTextMovement
                       isFinal:YES];
    }
#endif
    if (_rangeForUserCompletion.location != NSNotFound) {
        [self endUserCompletion];
    }    
	// object can be either a string or an attributed string
	// Both will be inserted using the current typing attributes
	NSString *replacementString = object;
	if ([object isKindOfClass:[NSAttributedString class]]) {
		replacementString = [object string];
	}
	
    if([self shouldChangeTextInRange:[self selectedRange] replacementString: replacementString] == NO) {
        return;
	}
    [self _replaceCharactersInRange:[self selectedRange] withString: object allowsTypingCoalescing:YES];
    [self didChangeText];
    [self scrollRangeToVisible:[self selectedRange]];
}

-(void)keyDown:(NSEvent *)event {
    if([event type]==NSKeyDown && [self isEditable]) {
        _processingKeyEvent = YES;
        [self interpretKeyEvents:[NSArray arrayWithObject:event]];
        _processingKeyEvent = NO;
    }
}

- (void)keyUp:(NSEvent*)event
{
    // Just to eat the event - else it is passed to the nextResponder, and we don't want that
}

-(void)doCommandBySelector:(SEL)selector {
    if ([_delegate respondsToSelector:@selector(textView:doCommandBySelector:)])
        if ([_delegate textView:self doCommandBySelector:selector] == YES)
            return;
    
    if (_rangeForUserCompletion.location != NSNotFound) {
        if (selector != @selector(moveDown:) &&
            selector != @selector(moveUp:) &&
            selector != @selector(insertTab:) &&
            selector != @selector(insertNewline:) &&
            selector != @selector(cancel:))
            [self insertCompletion:[_userCompletions objectAtIndex:_userCompletionSelectedItem]
               forPartialWordRange:_rangeForUserCompletion
                          movement:NSOtherTextMovement
                           isFinal:YES];
    }
    
    [super doCommandBySelector:selector];
}

-(void)drawRect:(NSRect)rect {
    NSLayoutManager *layoutManager=[self layoutManager];
    NSPoint          origin=[self textContainerOrigin];
    NSRect          glyphRect=rect;
    glyphRect.origin.x-=_textContainerInset.width;
    glyphRect.origin.y-=_textContainerInset.height;
    NSRange gRange = gRange=[layoutManager glyphRangeForBoundingRect:glyphRect inTextContainer:[self textContainer]];
    
    if([self drawsBackground]){
        [_backgroundColor setFill];
        NSRectFill(rect);
    }
    [layoutManager drawBackgroundForGlyphRange:gRange atPoint:origin];
    [layoutManager drawGlyphsForGlyphRange:gRange atPoint:origin];
    if([self shouldDrawInsertionPoint]){
        [self updateInsertionPointStateAndRestartTimer:NO];
        [self drawInsertionPointInRect:_insertionPointRect color:_insertionPointColor turnedOn:_insertionPointOn];
    }
}

-(void)mouseDown:(NSEvent *)event {
    NSEvent *lastDrag=event;
    NSPoint  point=[self convertPoint:[event locationInWindow] fromView:nil];
    float    fraction=0;
    NSRange  firstRange,lastRange,selection;
    NSSelectionAffinity affinity=NSSelectionAffinityUpstream;
    NSSelectionGranularity granularity = [event clickCount]-1;
    
    if(![self isSelectable])
        return;
    
    firstRange.location=[self glyphIndexForPoint:point fractionOfDistanceThroughGlyph:&fraction];
    firstRange.length=0;
    if(firstRange.location==NSNotFound)
        return;
    
    if(fraction>=0.5)
        firstRange.location++;
    
    
    if (firstRange.location<[_textStorage length])
        firstRange = [self selectionRangeForProposedRange:firstRange granularity:granularity];
    
    _selectionOrigin = firstRange.location;
    lastRange=firstRange;
    
    selection=NSUnionRange(firstRange,lastRange);
    
    [self setSelectedRange:selection affinity:affinity stillSelecting:YES];
    
    [NSEvent startPeriodicEventsAfterDelay:0.1 withPeriod:0.2];
    do {
        event=[[self window] nextEventMatchingMask:NSLeftMouseUpMask|
               NSLeftMouseDraggedMask|NSPeriodicMask];
        
        if([event type]==NSLeftMouseDragged)
            lastDrag=event;
        
        point=[self convertPoint:[lastDrag locationInWindow] fromView:nil];
        //  if(!NSMouseInRect(point,[self visibleRect],[self isFlipped]))
        [[self superview] autoscroll:lastDrag];
        
        lastRange.location=[self glyphIndexForPoint:point fractionOfDistanceThroughGlyph:&fraction];
        lastRange.length=0;
        
        if(lastRange.location==NSNotFound)
            continue;
        
        if(fraction>=0.5)
            lastRange.location++;
        
        if(lastRange.location<[_textStorage length])
            lastRange = [self selectionRangeForProposedRange:lastRange granularity:granularity];
        
        selection=NSUnionRange(firstRange,lastRange);
        if(firstRange.location<=lastRange.location)
            affinity=NSSelectionAffinityUpstream;
        else
            affinity=NSSelectionAffinityDownstream;
        
        [self setSelectedRange:selection affinity:affinity stillSelecting:YES];
        
    }while([event type]!=NSLeftMouseUp);
    
    [NSEvent stopPeriodicEvents];

    [self setSelectedRange:selection affinity:affinity stillSelecting:NO];
}

-(NSUndoManager *)undoManager {
    if ([_delegate respondsToSelector:@selector(undoManagerForTextView:)])
        return [_delegate undoManagerForTextView:self];
    
    if (_fieldEditorUndoManager)
        return _fieldEditorUndoManager;
    
    return [super undoManager];
}

-(BOOL)validateMenuItem:(NSMenuItem *)item {
    if ([item action] == @selector(undo:))
        return _allowsUndo ? [[self undoManager] canUndo] : NO;
    else if ([item action] == @selector(redo:))
        return _allowsUndo ? [[self undoManager] canRedo] : NO;
    else if ([item action] == @selector(toggleContinuousSpellChecking:)) {
        [item setState: [self isContinuousSpellCheckingEnabled]];
    }
    return YES;
}

-(NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender {
    NSPoint   point=[self convertPoint:[sender draggingLocation] fromView:nil];
    float     fraction=0;
    unsigned  location=[self glyphIndexForPoint:point fractionOfDistanceThroughGlyph:&fraction];
    
    if(location==NSNotFound)
        return NO;
    
    if(fraction>=0.5)
        location++;
    
    [_selectedRanges removeAllObjects];
    [_selectedRanges addObject:[NSValue valueWithRange:NSMakeRange(location,0)]];
    [self setNeedsDisplay:YES];
    
    if([sender draggingSourceOperationMask]&NSDragOperationGeneric)
        return NSDragOperationGeneric;
    
    if([sender draggingSourceOperationMask]&NSDragOperationCopy)
        return NSDragOperationCopy;
    
    return NSDragOperationNone;
}

-(unsigned)draggingEntered:(id <NSDraggingInfo>)sender {
    return [self draggingUpdated:sender];
}

-(BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender {
    return YES;
}

-(BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
    NSPoint   point=[self convertPoint:[sender draggingLocation] fromView:nil];
    NSString *string=[[sender draggingPasteboard] stringForType:NSStringPboardType];
    float     fraction=0;
    unsigned  location=[self glyphIndexForPoint:point fractionOfDistanceThroughGlyph:&fraction];
    
    if(location==NSNotFound)
        return NO;
    
    if(fraction>=0.5)
        location++;
    
    [self _replaceCharactersInRange:NSMakeRange(location,0) withString:string];
    [self didChangeText];
    
    return YES;
}

-(NSRect)firstRectForCharacterRange:(NSRange)range {
    return [self _viewRectForCharacterRange:range];
}

-(NSDragOperation)dragOperationForDraggingInfo:(id <NSDraggingInfo>)info type:(NSString *)type {
    NSUnimplementedMethod();
    return 0;
}

-(void)cleanUpAfterDragOperation {
    NSUnimplementedMethod();
}

-(NSUInteger)characterIndexForPoint:(NSPoint)point {
    NSUnimplementedMethod();
    return 0;
}

-(void)_setFieldEditorUndoManager:(NSUndoManager *)undoManager
{
    [_fieldEditorUndoManager autorelease];
    _fieldEditorUndoManager = [undoManager retain];
}

-(void)breakUndoCoalescing
{
    [_undoTyping release];
    _undoTyping = nil;
}

- (BOOL) _delegateChangeTextInRange: (NSRange)     range
				  replacementString: (NSString *) string
{
    if ([_delegate respondsToSelector:
         @selector(textView:shouldChangeTextInRange:replacementString:)])
    {
        if ([_delegate textView: self
        shouldChangeTextInRange: range
              replacementString: string] == NO)
            return NO;
    }
    
    return YES;
}

-(NSArray *)_delegateChangeSelectionFromRanges:(NSArray *)from toRanges:(NSArray *)to {
    NSArray *result=to;
    if ([_delegate respondsToSelector:@selector(textView:willChangeSelectionFromCharacterRanges:toCharacterRanges:)])
        result=[_delegate textView: self willChangeSelectionFromCharacterRanges:from toCharacterRanges:to];
    else if([_delegate respondsToSelector:@selector(textView:willChangeSelectionFromCharacterRange:toCharacterRange:)]){
        NSRange fromRange=[[from objectAtIndex:0] rangeValue];
        NSRange toRange=[[to objectAtIndex:0] rangeValue];
        NSRange resultRange=[_delegate textView: self willChangeSelectionFromCharacterRange:fromRange toCharacterRange:toRange];
        
        result=[NSArray arrayWithObject:[NSValue valueWithRange:resultRange]];
    }
    
    return result;
}

- (void)setAttributedString:(NSAttributedString *)attrString
{
	if (!NSIsControllerMarker(attrString))
	{
		[_textStorage setAttributedString:attrString];
	}
	else
	{
		[_textStorage setAttributedString:[[[NSAttributedString alloc] initWithString:NSLocalizedStringFromTableInBundle(@"", nil, [NSBundle bundleForClass: [NSTextView class]],@"") attributes:nil] autorelease]];
	}
}

- (NSAttributedString *)attributedString
{
	return [[_textStorage copy] autorelease];
}

-(void)changeSpelling:sender {
    NSString *correction=[[sender selectedCell] stringValue];
    NSRange selectedRange=[self selectedRange];
    
    if (![self _delegateChangeTextInRange: selectedRange replacementString: correction])
        return;
    
	[self _replaceCharactersInRange:selectedRange withString:correction useTypingAttributes: NO];
    [self didChangeText];
}

-(void)_changeSpellingFromMenuItem:sender {
    NSString *correction=[sender title];
    NSRange selectedRange=[self selectedRange];
    
    if (![self _delegateChangeTextInRange: selectedRange replacementString: correction])
        return;
    
    [self _replaceCharactersInRange:selectedRange withString:correction useTypingAttributes: NO];
    [self didChangeText];
}

-(void)ignoreSpelling:sender {
    [[NSSpellChecker sharedSpellChecker] ignoreWord:[[sender selectedCell] stringValue] inSpellDocumentWithTag: [self spellCheckerDocumentTag]];
}

-(void)showGuessPanel:sender {
	[self moveToBeginningOfDocument: sender];
    [[[NSSpellChecker sharedSpellChecker] spellingPanel] makeKeyAndOrderFront: self];
}

-(void)_continuousSpellCheckWithInvalidatedRange:(NSRange)invalidatedRange {
    NSString *string=[self string];
    NSUInteger start, end;
    
    // TODO, truncate invalidated range to string size if needed
    
    // round range to nearest paragraphs
    
    [string getParagraphStart:&start end:&end contentsEnd:NULL forRange:invalidatedRange];
    invalidatedRange=NSMakeRange(start,end-start);
    if (invalidatedRange.length > 0) {
        [[self layoutManager] removeTemporaryAttribute:NSSpellingStateAttributeName forCharacterRange:invalidatedRange];
    }
    if(_isContinuousSpellCheckingEnabled) {
        NSSpellChecker *checker=[NSSpellChecker sharedSpellChecker];
        
        NSArray *checking=[checker checkString:string range:invalidatedRange types:NSTextCheckingTypeSpelling options:nil inSpellDocumentWithTag:[self spellCheckerDocumentTag] orthography:NULL wordCount:NULL];
        
        NSUInteger selectionLocation=[self selectedRange].location;
        
        for(NSTextCheckingResult *result in checking){
            NSRange range=[result range];
            
            // inclusive of max range if the selection is sitting at the end of the word
            BOOL doNotMark=(selectionLocation>=range.location && selectionLocation<=NSMaxRange(range));
            
            if(!doNotMark)
                [self setSpellingState:NSSpellingStateSpellingFlag range:range];
        }
    }
}

-(void)_continuousSpellCheck {
    [self _continuousSpellCheckWithInvalidatedRange:NSMakeRange(0,[[self string] length])];
}

-(void)checkSpelling:sender {
    NSString *string=[self string];
    NSRange selection=[self selectedRange];
    
	// If we're at the end start over again
	if (NSMaxRange(selection) == [string length]) {
		[self moveToBeginningOfDocument: sender];
		selection=[self selectedRange];
	}
	else if (selection.length < [string length]) {
		selection.location += selection.length;
		if (NSMaxRange(selection) > [string length]) {
			selection.length = [string length] - selection.location;
		}
	}
	
    NSRange range=NSMakeRange(selection.location,[string length]-selection.location);
    
    NSSpellChecker *checker=[NSSpellChecker sharedSpellChecker];
    NSArray *checking=[checker checkString:string range:range types:NSTextCheckingTypeSpelling options:nil inSpellDocumentWithTag:[self spellCheckerDocumentTag] orthography:NULL wordCount:NULL];
    
    if([checking count]==0){
        [checker updateSpellingPanelWithMisspelledWord: @""];
    }
    else {
        NSTextCheckingResult *result=[checking objectAtIndex:0];
        NSRange range=[result range];
        
        [self setSelectedRange:range]; // this will clear the current spelling attributes
        
        [self setSpellingState:NSSpellingStateSpellingFlag range:range];
        
        NSString *word=[string substringWithRange:range];
        
        [checker updateSpellingPanelWithMisspelledWord: word];
    }
}

-(NSMenu *)menuForEvent:(NSEvent *)event {
    NSRange glyphRange;
    
    NSPoint point=[self convertPoint:[event locationInWindow] fromView:nil];
    float fraction;

    NSMenu *menu=[[[NSMenu alloc] initWithTitle:@""] autorelease];

    @try {
        NSRange range = [_textStorage doubleClickAtIndex: [self glyphIndexForPoint:point fractionOfDistanceThroughGlyph:&fraction]];
        
        [self setSelectedRange:range];
        
        NSSpellChecker *checker=[NSSpellChecker sharedSpellChecker];
        NSArray *guesses = [checker guessesForWordRange:range inString:[self string] language:[[NSLocale currentLocale] localeIdentifier] inSpellDocumentWithTag:[self spellCheckerDocumentTag]];
        
        
        if([guesses count]==0) {
            NSMenuItem *item=[menu addItemWithTitle:NSLocalizedStringFromTableInBundle(@"No Guesses Found", nil, [NSBundle bundleForClass: [NSTextView class]], @"Spell checker guesses") action:@selector(cut:) keyEquivalent:@""];
            [item setEnabled:NO];
        }
        else {
            for(NSString *guess in guesses){
                [menu addItemWithTitle:guess action:@selector(_changeSpellingFromMenuItem:) keyEquivalent:@""];
            }
        }
        [menu addItem:[NSMenuItem separatorItem]];
    }
    @catch (NSException *e) {
        // Ignore - doubleClickAtIndex: can throw a range exception - which means there's nothing to spellcheck
    }
    [menu addItemWithTitle: NSLocalizedStringFromTableInBundle(@"Cut", nil, [NSBundle bundleForClass: [NSTextView class]], @"Cut the selection") action:@selector(cut:) keyEquivalent:@""];
    [menu addItemWithTitle: NSLocalizedStringFromTableInBundle(@"Copy", nil, [NSBundle bundleForClass: [NSTextView class]], @"Copy the selection") action:@selector(copy:) keyEquivalent:@""];
    [menu addItemWithTitle: NSLocalizedStringFromTableInBundle(@"Paste", nil, [NSBundle bundleForClass: [NSTextView class]], @"Paste the selection") action:@selector(paste:) keyEquivalent:@""];
    [menu addItem:[NSMenuItem separatorItem]];
    [menu addItemWithTitle: NSLocalizedStringFromTableInBundle(@"Select All", nil, [NSBundle bundleForClass: [NSTextView class]], @"Select all the content") action:@selector(selectAll:) keyEquivalent:@""];
    
    return menu;
}

-(NSInteger)spellCheckerDocumentTag {
    /* There is no explicity invalid document tag, this is indirectly documented as zero in the full checkSpellingOfString: method.
     and supported by behavior. */
    if(_spellCheckerDocumentTag==0){
        
        /* Check if any other text view in the group has a document tag and use it. */
        /* We could put this in the text storage, but there are no provisions for that. */
        for(NSLayoutManager *checkLayout in [[self textStorage] layoutManagers])
            for(NSTextContainer *checkContainer in [checkLayout textContainers])
                if([checkContainer textView]->_spellCheckerDocumentTag!=0){
                    // Direct access the ivar to avoid the method logic here.
                    _spellCheckerDocumentTag=[checkContainer textView]->_spellCheckerDocumentTag;
                }
        
        if(_spellCheckerDocumentTag==0)
            _spellCheckerDocumentTag=[NSSpellChecker uniqueSpellDocumentTag];
    }
    
    return _spellCheckerDocumentTag;
}

-(BOOL)isContinuousSpellCheckingEnabled {
    return _isContinuousSpellCheckingEnabled;
}

-(void)setContinuousSpellCheckingEnabled:(BOOL)value {
    _isContinuousSpellCheckingEnabled=value;
    
    [self _continuousSpellCheck];
}

-(void)toggleContinuousSpellChecking:sender {
    _isContinuousSpellCheckingEnabled=!_isContinuousSpellCheckingEnabled;
    
    [self _continuousSpellCheck];
}

-(BOOL)isAutomaticSpellingCorrectionEnabled {
    return _isAutomaticSpellingCorrectionEnabled;
}

-(void)setAutomaticSpellingCorrectionEnabled:(BOOL)value {
    _isAutomaticSpellingCorrectionEnabled=value;
    NSUnimplementedMethod();
}

-(void)toggleAutomaticSpellingCorrection:sender {
    _isAutomaticSpellingCorrectionEnabled=!_isAutomaticSpellingCorrectionEnabled;
    NSUnimplementedMethod();
}

-(NSTextCheckingTypes)enabledTextCheckingTypes {
    return _enabledTextCheckingTypes;
}

-(void)setEnabledTextCheckingTypes:(NSTextCheckingTypes)checkingTypes {
    _enabledTextCheckingTypes=checkingTypes;
    NSUnimplementedMethod();
}

-(void)setSpellingState:(NSInteger)value range:(NSRange)characterRange {
    [[self layoutManager] addTemporaryAttribute:NSSpellingStateAttributeName value:[NSNumber numberWithUnsignedInt:value] forCharacterRange:characterRange];
}

#pragma mark Ruler client view
-(void)rulerView:(NSRulerView *)rulerView willSetClientView:(NSView *)clientView
{
}

-(void)rulerView:(NSRulerView *)rulerView handleMouseDown:(NSEvent *)event
{
    // Add a new tab stop
    NSPoint point = [self convertPoint: event.locationInWindow fromView: nil];
    float delta = rulerView.originOffset;
    NSRulerMarker *marker = [NSRulerMarker leftTabMarkerWithRulerView:rulerView
                                                             location:point.x + delta];
    NSTextTab *tabstop = [[[NSTextTab alloc] initWithType: NSLeftTabStopType location: point.x] autorelease];
    [marker setRepresentedObject: tabstop];
    [rulerView trackMarker: marker withMouseEvent: event];
}

-(BOOL)rulerView:(NSRulerView *)rulerView shouldMoveMarker:(NSRulerMarker *)marker
{
    return YES;
}

-(float)rulerView:(NSRulerView *)rulerView willMoveMarker:(NSRulerMarker *)marker toLocation:(float)location
{
    if (location < rulerView.originOffset) {
        location = rulerView.originOffset;
    }
    if (location > self.textContainer.containerSize.width - self.textContainer.lineFragmentPadding) {
        location = self.textContainer.containerSize.width - self.textContainer.lineFragmentPadding;
    }
    return location;
}

-(void)rulerView:(NSRulerView *)rulerView didMoveMarker:(NSRulerMarker *)marker
{
    float delta = rulerView.originOffset;
    float location = marker.markerLocation - delta;
    
    id representedObject = marker.representedObject;
    if ([representedObject isKindOfClass:[NSTextTab class]]) {
        NSTextTab *textTab = (NSTextTab *)representedObject;
        NSTextTab *newTab = [[[NSTextTab alloc] initWithType:[textTab tabStopType] location:location] autorelease];
        
        // We need to expand the range to full paragraphs
        NSRange range = [self rangeForUserParagraphAttributeChange];
        
        // Change the tab stop value for all of the selection
        unsigned  location = range.location;
        [_textStorage beginEditing];
        while (location < NSMaxRange(range)) {
            NSRange effectiveRange;
            NSParagraphStyle *style = [_textStorage attribute: NSParagraphStyleAttributeName atIndex:location effectiveRange: &effectiveRange];
            NSRange rangeToChange = NSIntersectionRange (effectiveRange, range);
            if (style == nil) {
                style = [NSParagraphStyle defaultParagraphStyle];
            }
            // Copy the paragraph style, changing only the tab
            NSMutableParagraphStyle *newStyle = [[style mutableCopy] autorelease];
            NSMutableArray *tabstops = [[[newStyle tabStops] mutableCopy] autorelease];
            [tabstops addObject: newTab];
            [tabstops removeObject: textTab];
            // keep the tabstops sorted
            [tabstops sortUsingDescriptors:[NSArray arrayWithObject:[NSSortDescriptor sortDescriptorWithKey:@"location" ascending:YES]]];
            [newStyle setTabStops:tabstops];
            [_textStorage addAttribute: NSParagraphStyleAttributeName value: newStyle range: rangeToChange];
            location = NSMaxRange(effectiveRange);
        }
        [_textStorage endEditing];
        [self didChangeText];
        
        // Update the typing attributes too
        NSParagraphStyle *style = [_typingAttributes objectForKey: NSParagraphStyleAttributeName];
        if (style == nil) {
            style = [NSParagraphStyle defaultParagraphStyle];
        }
        // Copy the paragraph style, changing only the tab
        NSMutableParagraphStyle *newStyle = [[style mutableCopy] autorelease];
        NSMutableArray *tabstops = [[[newStyle tabStops] mutableCopy] autorelease];
        [tabstops addObject: newTab];
        [tabstops removeObject: textTab];
        // keep the tabstops sorted
        [tabstops sortUsingDescriptors:[NSArray arrayWithObject:[NSSortDescriptor sortDescriptorWithKey:@"location" ascending:YES]]];
        [newStyle setTabStops:tabstops];
        if (![_typingAttributes isKindOfClass:[NSMutableDictionary class]]) {
            NSDictionary *dict = [_typingAttributes mutableCopy];
            [_typingAttributes release];
            _typingAttributes = dict;
        }
        [(NSMutableDictionary *)_typingAttributes setObject: newStyle forKey: NSParagraphStyleAttributeName];
        
        marker.representedObject = newTab;
    } else if ([representedObject isEqual:@"NSHeadIndentRulerMarkerTag"] ||
               [representedObject isEqual:@"NSTailIndentRulerMarkerTag"] ||
               [representedObject isEqual:@"NSFirstLineHeadIndentRulerMarkerTag"]) {
        // TODO
        // style.headIndent = location
        // style.tailIndent = view.textContainer.containerSize.width - location
        // style.firstLineHeadIndent = location
    }
}

-(BOOL)rulerView:(NSRulerView *)rulerView shouldAddMarker:(NSRulerMarker *)marker
{
    return YES;
}

-(float)rulerView:(NSRulerView *)rulerView willAddMarker:(NSRulerMarker *)marker atLocation:(float)location
{
    if (location < rulerView.originOffset) {
        location = rulerView.originOffset;
    }
    if (location > self.textContainer.containerSize.width - self.textContainer.lineFragmentPadding) {
        location = self.textContainer.containerSize.width - self.textContainer.lineFragmentPadding;
    }
    return location;
}

-(void)rulerView:(NSRulerView *)rulerView didAddMarker:(NSRulerMarker *)marker
{
    float delta = rulerView.originOffset;
    
    float location = marker.markerLocation - delta;
    
    id representedObject = marker.representedObject;
    if ([representedObject isKindOfClass:[NSTextTab class]]) {
        NSTextTab *textTab = (NSTextTab *)representedObject;
        NSTextTab *newTab = [[[NSTextTab alloc] initWithType:[textTab tabStopType] location:location] autorelease];
        
        // Change the tab stop value for all of the selection
        NSRange range = [self rangeForUserParagraphAttributeChange];
        unsigned  location = range.location;
        [_textStorage beginEditing];
        while (location < NSMaxRange(range)) {
            NSRange effectiveRange;
            NSParagraphStyle *style = [_textStorage attribute: NSParagraphStyleAttributeName atIndex:location effectiveRange: &effectiveRange];
            NSRange rangeToChange = NSIntersectionRange (effectiveRange, range);
            if (style == nil) {
                style = [NSParagraphStyle defaultParagraphStyle];
            }
            // Copy the paragraph style, changing only the tab
            NSMutableParagraphStyle *newStyle = [[style mutableCopy] autorelease];
            NSMutableArray *tabstops = [[[newStyle tabStops] mutableCopy] autorelease];
            [tabstops addObject: newTab];
            // keep the tabstops sorted
            [tabstops sortUsingDescriptors:[NSArray arrayWithObject:[NSSortDescriptor sortDescriptorWithKey:@"location" ascending:YES]]];
            [newStyle setTabStops:tabstops];
            [_textStorage addAttribute:NSParagraphStyleAttributeName value: newStyle range: rangeToChange];
            location = NSMaxRange(effectiveRange);
        }
        [_textStorage endEditing];
        
        // Update the typing attributes too
        NSParagraphStyle *style = [_typingAttributes objectForKey: NSParagraphStyleAttributeName];
        if (style == nil) {
            style = [NSParagraphStyle defaultParagraphStyle];
        }
        // Copy the paragraph style, changing only the tab
        NSMutableParagraphStyle *newStyle = [[style mutableCopy] autorelease];
        NSMutableArray *tabstops = [[[newStyle tabStops] mutableCopy] autorelease];
        [tabstops addObject: newTab];
        // keep the tabstops sorted
        [tabstops sortUsingDescriptors:[NSArray arrayWithObject:[NSSortDescriptor sortDescriptorWithKey:@"location" ascending:YES]]];
        [newStyle setTabStops:tabstops];
        if (![_typingAttributes isKindOfClass:[NSMutableDictionary class]]) {
            NSDictionary *dict = [_typingAttributes mutableCopy];
            [_typingAttributes release];
            _typingAttributes = dict;
        }
        [(NSMutableDictionary *)_typingAttributes setObject: newStyle forKey: NSParagraphStyleAttributeName];
        
        marker.representedObject = newTab;
        
        [self didChangeText];
    }
}


-(BOOL)rulerView:(NSRulerView *)rulerView shouldRemoveMarker:(NSRulerMarker *)marker
{
    return [(NSObject *)marker.representedObject isKindOfClass:[NSTextTab class]];
}

-(void)rulerView:(NSRulerView *)rulerView didRemoveMarker:(NSRulerMarker *)marker
{
    id representedObject = marker.representedObject;
    if ([representedObject isKindOfClass:[NSTextTab class]]) {
        NSTextTab *textTab = (NSTextTab *)representedObject;
        
        // Change the tab stop value for all of the selection
        NSRange range = [self rangeForUserParagraphAttributeChange];
        unsigned  location = range.location;
        
        [_textStorage beginEditing];
        while (location < NSMaxRange(range)) {
            NSRange effectiveRange;
            NSParagraphStyle *style = [_textStorage attribute: NSParagraphStyleAttributeName atIndex:location effectiveRange: &effectiveRange];
            NSRange rangeToChange = NSIntersectionRange (effectiveRange, range);
            if (style == nil) {
                style = [NSParagraphStyle defaultParagraphStyle];
            }
            // Copy the paragraph style, changing only the tab
            NSMutableParagraphStyle *newStyle = [[style mutableCopy] autorelease];
            NSMutableArray *tabstops = [[[newStyle tabStops] mutableCopy] autorelease];
            [tabstops removeObject: textTab];
            [newStyle setTabStops:tabstops];
            [_textStorage addAttribute: NSParagraphStyleAttributeName value: newStyle range: rangeToChange];
            location = NSMaxRange(effectiveRange);
        }
        [_textStorage endEditing];
        
        // Update the typing attributes
        NSParagraphStyle *style = [_typingAttributes objectForKey: NSParagraphStyleAttributeName];
        if (style == nil) {
            style = [NSParagraphStyle defaultParagraphStyle];
        }
        // Copy the paragraph style, changing only the tab
        NSMutableParagraphStyle *newStyle = [[style mutableCopy] autorelease];
        NSMutableArray *tabstops = [[[newStyle tabStops] mutableCopy] autorelease];
        [tabstops removeObject: textTab];
        [newStyle setTabStops:tabstops];
        if (![_typingAttributes isKindOfClass:[NSMutableDictionary class]]) {
            NSDictionary *dict = [_typingAttributes mutableCopy];
            [_typingAttributes release];
            _typingAttributes = dict;
        }
        [(NSMutableDictionary *)_typingAttributes setObject: newStyle forKey: NSParagraphStyleAttributeName];
        
        marker.representedObject = nil;
        
        [self didChangeText];
    }
}


@end
