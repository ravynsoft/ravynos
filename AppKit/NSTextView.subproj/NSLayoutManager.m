/* Copyright (c) 2006-2009 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSLayoutManager.h>
#import <AppKit/NSGlyphGenerator.h>
#import <AppKit/NSTypesetter.h>
#import <AppKit/NSTextContainer.h>
#import <AppKit/NSTextStorage.h>
#import <AppKit/NSTextView.h>
#import <AppKit/NSTextAttachment.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSBezierPath.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSGraphicsContextFunctions.h>
#import <AppKit/NSRaise.h>

#import <AppKit/NSAttributedString.h>
#import <AppKit/NSStringDrawing.h>
#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSGraphics.h>
#import <ApplicationServices/ApplicationServices.h>
#import "../../Foundation/NSAttributedString/NSRangeEntries.h"
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSRaiseException.h>

#import "NSRulerMarker+NSTextExtensions.h"
#import "NSBidiHelper.h"

typedef struct {
   NSRect  rect;
   NSRect  usedRect;
   NSPoint location;
   NSTextContainer *container;
    BOOL leftToRight;
} NSGlyphFragment;

typedef struct {
   int _xxxNeedSomething;
} NSInvalidFragment;

// Forward declaration
@interface NSLayoutManager()
-(NSRange)validateGlyphsAndLayoutForGlyphRange:(NSRange)glyphRange;
-(NSRange)_currentGlyphRangeForTextContainer:(NSTextContainer *)container;
@end

@implementation NSLayoutManager

static inline NSGlyphFragment *fragmentForGlyphRange(NSLayoutManager *self,NSRange range){
	NSGlyphFragment *result=NSRangeEntryAtRange(self->_glyphFragments,range);
	
	if(result==NULL) {
		// That can happens in normal cases, so we don't want to crash or log that. For example when some text can't be layout (too small container...)
		//	[NSException raise:NSGenericException format:@"fragmentForGlyphRange fragment is NULL for range %d %d",range.location,range.length];
	}
	return result;
}

static inline NSGlyphFragment *fragmentAtGlyphIndex(NSLayoutManager *self,unsigned index,NSRange *effectiveRange){
	NSGlyphFragment *result=NSRangeEntryAtIndex(self->_glyphFragments,index,effectiveRange);
	
	if(result==NULL){
		// That can happens in normal cases, so we don't want to crash or log that. For example when some text can't be layout (too small container...)
		//  [NSException raise:NSGenericException format:@"fragmentAtGlyphIndex fragment is NULL for index %d",index];
	}
	return result;
}

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;

	   // The text storage owns the layout manager (and text view can own the storage)
	   // Retaining it here would create a retain cycle.
	_textStorage=[[[keyed decodeObjectForKey:@"NSTextStorage"] retain] autorelease];
	
	_rangeToTemporaryAttributes=NSCreateRangeToCopiedObjectEntries(0);
	NSRangeEntryInsert(_rangeToTemporaryAttributes,NSMakeRange(0,[_textStorage length]),[NSDictionary dictionary]);

   _typesetter=[NSTypesetter new];
   _glyphGenerator=[[NSGlyphGenerator sharedGlyphGenerator] retain];
   _delegate=[keyed decodeObjectForKey:@"NSDelegate"];
   _textContainers=[NSMutableArray new];
   [_textContainers addObjectsFromArray:[keyed decodeObjectForKey:@"NSTextContainers"]];
   _glyphFragments=NSCreateRangeToOwnedPointerEntries(2);
   _invalidFragments=NSCreateRangeToOwnedPointerEntries(2);
   _layoutInvalid=YES;
   _rectCacheCapacity=16;
   _rectCacheCount=0;
   _rectCache=NSZoneMalloc(NULL,sizeof(NSRect)*_rectCacheCapacity);    
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }
   return self;
}

-init {
   _typesetter=[NSTypesetter new];
   _glyphGenerator=[[NSGlyphGenerator sharedGlyphGenerator] retain];
   _textContainers=[NSMutableArray new];
   _glyphFragments=NSCreateRangeToOwnedPointerEntries(2);
   _invalidFragments=NSCreateRangeToOwnedPointerEntries(2);
   _layoutInvalid=YES;
   _rectCacheCapacity=16;
   _rectCacheCount=0;
   _rectCache=NSZoneMalloc(NULL,sizeof(NSRect)*_rectCacheCapacity);
   return self;
}

-(void)dealloc {
	NSFreeRangeEntries(_rangeToTemporaryAttributes);
   _textStorage=nil;
   [_typesetter release];
   [_glyphGenerator release];
   [_textContainers release];
   NSFreeRangeEntries(_glyphFragments);
   NSFreeRangeEntries(_invalidFragments);
   NSZoneFree(NULL,_rectCache);
   [super dealloc];
}

-(NSTextStorage *)textStorage {
   return [[_textStorage retain] autorelease];
}

-(NSGlyphGenerator *)glyphGenerator {
   return [[_glyphGenerator retain] autorelease];
}

-(NSTypesetter *)typesetter {
   return [[_typesetter retain] autorelease];
}

-delegate {
   return _delegate;
}

-(NSArray *)textContainers {
   return [[_textContainers retain] autorelease];
}

-(NSTextView *)firstTextView {
   return [[_textContainers objectAtIndex:0] textView];
}

-(NSTextView *)textViewForBeginningOfSelection {
   return [[_textContainers objectAtIndex:0] textView];
}

-(BOOL)layoutManagerOwnsFirstResponderInWindow:(NSWindow *)window {
   NSResponder *first=[window firstResponder];
   int          i,count=[_textContainers count];
   
   for(i=0;i<count;i++)
    if([[_textContainers objectAtIndex:i] textView]==first)
     return YES;
     
   return NO;
}

-(void)setTextStorage:(NSTextStorage *)textStorage {

	if (textStorage == _textStorage) {
		return;
	}
	
	// The text storage owns the layout manager - so we can't retain
    _textStorage=textStorage;

	NSFreeRangeEntries(_rangeToTemporaryAttributes);

	_rangeToTemporaryAttributes=NSCreateRangeToCopiedObjectEntries(0);
	NSRangeEntryInsert(_rangeToTemporaryAttributes,NSMakeRange(0,[_textStorage length]),[NSDictionary dictionary]);
	
	_layoutInvalid = YES;
}

-(void)replaceTextStorage:(NSTextStorage *)textStorage {
	[self setTextStorage: textStorage];
}

-(void)setGlyphGenerator:(NSGlyphGenerator *)generator {
   generator=[generator retain];
   [_glyphGenerator release];
   _glyphGenerator=generator;
}

-(void)setTypesetter:(NSTypesetter *)typesetter {
   typesetter=[typesetter retain];
   [_typesetter release];
   _typesetter=typesetter;
}

-(void)setDelegate:delegate {
   _delegate=delegate;
}

-(BOOL)usesScreenFonts {
   return YES;
}

-(void)setUsesScreenFonts:(BOOL)yorn {

}

-(void)addTextContainer:(NSTextContainer *)container {
   [_textContainers addObject:container];
   [container setLayoutManager:self];
}

-(void)removeTextContainerAtIndex:(unsigned)index {
   [_textContainers removeObjectAtIndex:index];
}

-(void)insertTextContainer:(NSTextContainer *)container atIndex:(unsigned)index {
   [_textContainers insertObject:container atIndex:index];
   [container setLayoutManager:self];
}

-(void)insertGlyph:(NSGlyph)glyph atGlyphIndex:(unsigned)glyphIndex characterIndex:(unsigned)characterIndex {
}

-(void)replaceGlyphAtIndex:(unsigned)glyphIndex withGlyph:(NSGlyph)glyph {
}

-(void)deleteGlyphsInRange:(NSRange)glyphRange {
}

-(void)setCharacterIndex:(unsigned)characterIndex forGlyphAtIndex:(unsigned)glyphIndex {
}

-(void)setNotShownAttribute:(BOOL)notShown forGlyphAtIndex:(unsigned)glyphIndex {
}

-(void)setAttachmentSize:(NSSize)size forGlyphRange:(NSRange)glyphRange {
}

-(void)setDrawsOutsideLineFragment:(BOOL)drawsOutside forGlyphAtIndex:(unsigned)glyphIndex {
}

-(unsigned)numberOfGlyphs {
   return [_textStorage length];
}

- (void)_rollbackLatestFragment
{
	NSRangeEntriesRemoveEntryAtIndex(_glyphFragments, NSCountRangeEntries(_glyphFragments)- 1);
}

-(NSFont *)_fontForGlyphRange:(NSRange)glyphRange {
   NSRange       characterRange=[self characterRangeForGlyphRange:glyphRange actualGlyphRange:NULL];
   NSDictionary *attributes=[_textStorage attributesAtIndex:characterRange.location effectiveRange:NULL];

   return NSFontAttributeInDictionary(attributes);
}

#define DEBUG_LM_LAYOUT 0

-(unsigned)getGlyphs:(NSGlyph *)glyphs range:(NSRange)glyphRange {

#if DEBUG_LM_LAYOUT
    NSLog(@"getGlyphs: %p range: %@", glyphs, NSStringFromRange(glyphRange));
#define DEBUG_getGlyphs_range 0
#endif
    
   NSRange characterRange=[self characterRangeForGlyphRange:glyphRange actualGlyphRange:NULL];
   NSFont *font=[self _fontForGlyphRange:glyphRange];
   unichar buffer[characterRange.length];

   [[_textStorage string] getCharacters:buffer range:characterRange];
   [font getGlyphs:glyphs forCharacters:buffer length:characterRange.length];

#if DEBUG_getGlyphs_range
    NSLog(@"returning %u", glyphRange.length);
#endif
    
    return glyphRange.length;
}

-(unsigned)getOrderedGlyphs:(NSGlyph *)glyphs range:(NSRange)glyphRange baseLevel:(uint8_t)baseLevel order:(NSUInteger *)order
{
    
#if DEBUG_LM_LAYOUT
    NSLog(@"getOrderedGlyphs: %p range: %@ baseLevel: %u order: %p",
          glyphs,
          NSStringFromRange(glyphRange),
          baseLevel,
          order);
#define DEBUG_getOrderedGlyphs_range_baseLevel_order 0
#endif
    
    uint8_t bidiLevels[glyphRange.length];
    unsigned result = [self getGlyphsInRange:glyphRange glyphs:NULL characterIndexes:NULL glyphInscriptions:NULL elasticBits:NULL bidiLevels:bidiLevels];
    BOOL needsOrdering = baseLevel&1;
    
    for (int i = 0; i < glyphRange.length && !needsOrdering; i++) {
        if (bidiLevels[i]&1) {
            needsOrdering = YES;
            break;
        }
    }
    if (order) {
        for (int i = 0; i < glyphRange.length; i++) {
            order[i] = i;
        }
    }
    if (needsOrdering) {
        // Reorder the chars & do mirroring, and get the glyphs from that result

        // Get the char for the range
        NSRange characterRange=[self characterRangeForGlyphRange:glyphRange actualGlyphRange:NULL];
        unichar buffer[characterRange.length];
        [[_textStorage string] getCharacters:buffer range:characterRange];
        
        // Process them : that will reorder "order", and do mirroring if needed
        NSBidiHelperProcessLine(baseLevel, order, buffer, bidiLevels, true, characterRange.length);
        
        NSFont *font=[self _fontForGlyphRange:glyphRange];
        result = [font getGlyphs:glyphs forCharacters:buffer length:characterRange.length];
        // Reorder the glyph
        NSBidiHelperProcessLine(baseLevel, glyphs, NULL, bidiLevels, false, characterRange.length);
    } else {
        result = [self getGlyphs:glyphs range:glyphRange];
    }
    return result;
}

-(unsigned)getGlyphsInRange:(NSRange)range glyphs:(NSGlyph *)glyphs characterIndexes:(unsigned *)charIndexes glyphInscriptions:(NSGlyphInscription *)inscriptions elasticBits:(BOOL *)elasticBits {

#if DEBUG_LM_LAYOUT
    NSLog(@"getGlyphsInRange: %@ glyphs: p characterIndexes: %p glyphInscriptions: %p elasticBits: %p",
          NSStringFromRange(range),
          glyphs,
          charIndexes,
          inscriptions,
          elasticBits);
#endif
    
    return [self getGlyphsInRange:range glyphs:glyphs characterIndexes:charIndexes glyphInscriptions:inscriptions elasticBits:elasticBits bidiLevels:NULL];
}

-(unsigned)getGlyphsInRange:(NSRange)range glyphs:(NSGlyph *)glyphs characterIndexes:(unsigned *)charIndexes glyphInscriptions:(NSGlyphInscription *)inscriptions elasticBits:(BOOL *)elasticBits bidiLevels:(unsigned char *)bidiLevels {
    
#if DEBUG_LM_LAYOUT
    NSLog(@"getGlyphsInRange: %@ glyphs: p characterIndexes: %p glyphInscriptions: %p elasticBits: %p bidiLevels: %p",
          NSStringFromRange(range),
          glyphs,
          charIndexes,
          inscriptions,
          elasticBits,
          bidiLevels);
#endif
    
    unsigned result  = 0;
    if (glyphs) {
        result = [self getGlyphs:glyphs range:range];
    }
    // Actually ask to the typesetter for anything else - it's the one knowing all the dirty details
    [_typesetter getGlyphsInRange:range glyphs:NULL characterIndexes:charIndexes glyphInscriptions:inscriptions elasticBits:elasticBits bidiLevels:bidiLevels];
    return result;
}

-(NSTextContainer *)textContainerForGlyphAtIndex:(unsigned)glyphIndex effectiveRange:(NSRangePointer)effectiveGlyphRange {
    
#if DEBUG_LM_LAYOUT
    NSLog(@"textContainerForGlyphAtIndex: %u effectiveRange: %p", glyphIndex, effectiveGlyphRange);
#define DEBUG_textContainerForGlyphAtIndex_effectiveRange 0
#endif

	[self validateGlyphsAndLayoutForGlyphRange:NSMakeRange(glyphIndex, 1)];
	NSGlyphFragment *fragment=fragmentAtGlyphIndex(self,glyphIndex,effectiveGlyphRange);
	if(fragment==NULL) {
        
#if DEBUG_textContainerForGlyphAtIndex_effectiveRange
        NSLog(@"no fragment found - bailing...");
#endif
        
		return nil;
	}
	
	if (effectiveGlyphRange) {
		*effectiveGlyphRange = [self _currentGlyphRangeForTextContainer:fragment->container];
	}
    
#if DEBUG_textContainerForGlyphAtIndex_effectiveRange
    NSLog(@"found container: %@", fragment->container);
#endif
    
	return fragment->container;
}

-(NSRect)lineFragmentRectForGlyphAtIndex:(unsigned)glyphIndex effectiveRange:(NSRangePointer)effectiveGlyphRange {

#if DEBUG_LM_LAYOUT
    NSLog(@"lineFragmentRectForGlyphAtIndex: %u effectiveRange: %p", glyphIndex, effectiveGlyphRange);
#define DEBUG_lineFragmentRectForGlyphAtIndex_effectiveRange 0
#endif
    
	NSGlyphFragment *fragment=fragmentAtGlyphIndex(self,glyphIndex,effectiveGlyphRange);
	
	if(fragment==NULL) {
        
#if DEBUG_lineFragmentRectForGlyphAtIndex_effectiveRange
        NSLog(@"fragment not found returning zero rect");
#endif
        
		return NSZeroRect;
    }
    
#if DEBUG_lineFragmentRectForGlyphAtIndex_effectiveRange
    NSLog(@"returning rect: %@", NSStringFromRect(fragment->rect));
#endif
	
	return fragment->rect;
}

-(NSPoint)locationForGlyphAtIndex:(unsigned)glyphIndex {

#if DEBUG_LM_LAYOUT
    NSLog(@"locationForGlyphAtIndex: %u", glyphIndex);
#define DEBUG_locationForGlyphAtIndex 0
#endif
    
   NSGlyphFragment *fragment= fragmentAtGlyphIndex(self,glyphIndex,NULL);

    if(fragment==NULL) {

#if DEBUG_locationForGlyphAtIndex
        NSLog(@"fragment not found - bailing...");
#endif
        
        return NSZeroPoint;
    }

#if DEBUG_locationForGlyphAtIndex
    NSLog(@"returning location: %@", NSStringFromPoint(fragment->location));
#endif

   return fragment->location;
}

-(NSRect)lineFragmentUsedRectForGlyphAtIndex:(unsigned)glyphIndex effectiveRange:(NSRangePointer)effectiveGlyphRange {

#if DEBUG_LM_LAYOUT
    NSLog(@"lineFragmentUsedRectForGlyphAtIndex: %u effectiveRange: %p", glyphIndex, effectiveGlyphRange);
#define DEBUG_lineFragmentUsedRectForGlyphAtIndex_effectiveRange 0
#endif

    NSGlyphFragment *fragment= fragmentAtGlyphIndex(self,glyphIndex,effectiveGlyphRange);

    if(fragment==NULL) {
        
#if DEBUG_lineFragmentUsedRectForGlyphAtIndex_effectiveRange
        NSLog(@"fragment not found - bailing...");
#endif
        
        return NSZeroRect;
    }

#if DEBUG_lineFragmentUsedRectForGlyphAtIndex_effectiveRange
    NSLog(@"returning rect: %@", NSStringFromRect(fragment->usedRect));
#endif

   return fragment->usedRect;
}

-(NSRange)validateGlyphsAndLayoutForGlyphRange:(NSRange)glyphRange {

#if DEBUG_LM_LAYOUT
    NSLog(@"validateGlyphsAndLayoutForGlyphRange: %@", NSStringFromRange(glyphRange));
#define DEBUG_validateGlyphsAndLayoutForGlyphRange 0
#endif
    
   // TODO: Validate glyphs in glyph cache for glyph range

   if(_layoutInvalid){

#if DEBUG_validateGlyphsAndLayoutForGlyphRange
       NSLog(@"layout is invalid");
#endif
       
    NSResetRangeEntries(_glyphFragments);
    [_typesetter layoutGlyphsInLayoutManager:self startingAtGlyphIndex:0 maxNumberOfLineFragments:0 nextGlyphIndex:NULL];
    _layoutInvalid=NO;
	   
	   if ([_delegate respondsToSelector:@selector(layoutManager:didCompleteLayoutForTextContainer:atEnd:)]) {

           
		   NSTextContainer *container = [_textContainers lastObject];
		   NSRange containerRange = [self _currentGlyphRangeForTextContainer:container];
		   BOOL finished = NSMaxRange(containerRange) >= NSMaxRange(glyphRange);
		   [_delegate layoutManager:self didCompleteLayoutForTextContainer:container atEnd:finished];

#if DEBUG_validateGlyphsAndLayoutForGlyphRange
           NSLog(@"informed delegate with layoutManager: %p didCompleteLayoutForTextContainer: %@ atEnd: %@", self, container, finished ? @"YES" : @"NO");
#endif
           
	   }
   } else {
       
#if DEBUG_validateGlyphsAndLayoutForGlyphRange
       NSLog(@"layout is still valid - glyphRange unchanged");
#endif
       
   }

   return glyphRange;

#if 0
   unsigned glyphIndex=[self firstUnlaidGlyphIndex];

   while(glyphIndex<NSMaxRange(glyphRange)){
    [_typesetter layoutGlyphsInLayoutManager:self
      startingAtGlyphIndex:glyphIndex maxNumberOfLineFragments:2
            nextGlyphIndex:&glyphIndex];
   }
#endif
}

-(void)validateGlyphsAndLayoutForContainer:(NSTextContainer *)container 
{
#if DEBUG_LM_LAYOUT
    NSLog(@"validateGlyphsAndLayoutForContainer: %@", container);
#endif
	// Validate everything - we should at least validate everything up to this container
   [self validateGlyphsAndLayoutForGlyphRange:NSMakeRange(0,[self numberOfGlyphs])];
}


-(NSRect)usedRectForTextContainer:(NSTextContainer *)container {

#if DEBUG_LM_LAYOUT
    NSLog(@"usedRectForTextContainer: %@", container);
#define DEBUG_usedRectForTextContainer 0
#endif

   [self validateGlyphsAndLayoutForContainer:container];
  {
      
#if DEBUG_usedRectForTextContainer
      NSLog(@"done validating, now calcing used rect");
#endif
      
   NSRect            result=NSZeroRect;
   BOOL              assignFirst=YES;
   NSRangeEnumerator state=NSRangeEntryEnumerator(_glyphFragments);
   NSRange           range;
   NSGlyphFragment  *fragment;

	  while(NSNextRangeEnumeratorEntry(&state,&range,(void **)&fragment)){
		  if (fragment->container == container) 
		  {
			  NSRect rect=fragment->usedRect;
#if DEBUG_usedRectForTextContainer
              NSLog(@"fragmentRange: %@ has width: %f", NSStringFromRange(range), rect.size.width);
#endif
			  if(assignFirst){
				  result=rect;
				  assignFirst=NO;
			  }
			  else {
				  result=NSUnionRect(result,rect);
			  }
		  }
	  }
	  
   if(assignFirst){
       
#if DEBUG_usedRectForTextContainer
       NSLog(@"no fragments so checking for _extraLineFragment...");
#endif
       
    // if empty, use the extra rect
    if(container==_extraLineFragmentTextContainer){
     NSRect extra=_extraLineFragmentUsedRect;
  /* Currently extra rect has a very large width  due to the behavior of the layout mechanism, so we set it to 1 here for proper sizing
     The insertion point code does the same thing to draw the point at the end of text.
     
     If the extra rect should not be large, need to reflect that change here and everywhere else it is used.
   */
     extra.size.width=1;
    
     result=extra;		

#if DEBUG_usedRectForTextContainer
        NSLog(@"result: %@", NSStringFromRect(result));
#endif

    } else {
        
#if DEBUG_usedRectForTextContainer
        NSLog(@"container is no the _extraLineFragmentTextContainer");
#endif
        
    }
   }

#if DEBUG_usedRectForTextContainer
      NSLog(@"result: %@", NSStringFromRect(result));
      NSLog(@"result.size.width: %f", result.size.width);
#endif

   return result;
  }
}

-(NSRect)extraLineFragmentRect {
   return _extraLineFragmentRect;
}

-(NSRect)extraLineFragmentUsedRect {
   return _extraLineFragmentUsedRect;
}

-(NSTextContainer *)extraLineFragmentTextContainer {
   return _extraLineFragmentTextContainer;
}

-(void)setTextContainer:(NSTextContainer *)container forGlyphRange:(NSRange)glyphRange {

#if DEBUG_LM_LAYOUT
    NSLog(@"setTextContainer: %@ forGlyphRange: %@", container, NSStringFromRange(glyphRange));
#define DEBUG_setTextContainer_forGlyphRange 0
#endif
    
   NSGlyphFragment *insert=NSZoneMalloc(NULL,sizeof(NSGlyphFragment));

	insert->rect=NSZeroRect;
	insert->usedRect=NSZeroRect;
	insert->location=NSZeroPoint;
	insert->container=container;
    // Get the direction for the fragment
    uint8_t bidiLevel;
    [_typesetter getGlyphsInRange:NSMakeRange(glyphRange.location, 1) glyphs:NULL characterIndexes:NULL glyphInscriptions:NULL elasticBits:NULL bidiLevels:&bidiLevel];
    
#if DEBUG_setTextContainer_forGlyphRange
    NSLog(@"text direction: %@" (bidiLevel & 1) ? @"Right to left" : @"Left to right");
#endif
    
    insert->leftToRight = (bidiLevel & 1) == 0;
   NSRangeEntryInsert(_glyphFragments,glyphRange,insert);
}

-(void)setLineFragmentRect:(NSRect)rect forGlyphRange:(NSRange)range usedRect:(NSRect)usedRect {

#if DEBUG_LM_LAYOUT
    NSLog(@"setLineFragmentRect: %@ forGlyphRange: %@ usedRect: %@", NSStringFromRect(rect), NSStringFromRange(range), NSStringFromRect(usedRect));
#define DEBUG_setLineFragmentRect_forGlyphRange_usedRect 0
#endif

    NSGlyphFragment *fragment=fragmentForGlyphRange(self,range);

    if(fragment==NULL) {
#if DEBUG_setLineFragmentRect_forGlyphRange_usedRect
        NSLog(@"fragment not found - bailing...");
#endif
        return;
    }

   fragment->rect=rect;
   fragment->usedRect=usedRect;
}

-(void)setLocation:(NSPoint)location forStartOfGlyphRange:(NSRange)range {

#if DEBUG_LM_LAYOUT
    NSLog(@"setLocation: %@ forStartOfGlyphRange: %@", NSStringFromPoint(location), NSStringFromRange(range));
#define DEBUG_setLocation_forStartOfGlyphRange 0
#endif

    NSGlyphFragment *fragment=fragmentForGlyphRange(self,range);
    
    if(fragment==NULL) {
#if DEBUG_setLocation_forStartOfGlyphRange
        NSLog(@"fragment not found - bailing...");
#endif
        return;        
    }
    
    fragment->location=location;
}

-(void)setExtraLineFragmentRect:(NSRect)fragmentRect usedRect:(NSRect)usedRect textContainer:(NSTextContainer *)container {

#if DEBUG_LM_LAYOUT
    NSLog(@"setExtraLineFragmentRect: %@ usedRect: %@ textContainer: %@", NSStringFromRect(fragmentRect), NSStringFromRect(usedRect), container);
#endif

    _extraLineFragmentRect=fragmentRect;
   _extraLineFragmentUsedRect=usedRect;
   _extraLineFragmentTextContainer=container;
}



-(void)invalidateGlyphsForCharacterRange:(NSRange)charRange changeInLength:(int)delta actualCharacterRange:(NSRangePointer)actualRange {
    
#if DEBUG_LM_LAYOUT
    NSLog(@"invalidateGlyphsForCharacterRange: %@ changeInLength: %d actualCharacterRange: %p", NSStringFromRange(charRange), delta, actualRange);
#endif
    
   if(actualRange!=NULL)
    *actualRange=charRange;
}

-(void)invalidateLayoutForCharacterRange:(NSRange)charRange isSoft:(BOOL)isSoft actualCharacterRange:(NSRangePointer)actualRangep {
    
#if DEBUG_LM_LAYOUT
    NSLog(@"invalidateLayoutForCharacterRange: %@ isSoft: %@ actualCharacterRange: %p", NSStringFromRange(charRange), isSoft ? @"YES" : @"NO", actualRangep);
#endif
    
#if 0
   unsigned location=charRange.location;
   unsigned limit=NSMaxRange(charRange);
   NSRange  actualRange=NSMakeRange(NSNotFound,NSNotFound);

   while(location<limit){
    NSRange            effectiveRange;
    NSGlyphFragment   *fragment=fragmentAtGlyphIndex(self,location,&effectiveRange);

    if(fragment!=NULL){
     NSInvalidFragment *invalid=NSZoneMalloc(NULL,sizeof(NSInvalidFragment));

     if(actualRange.location==NSNotFound)
      actualRange=effectiveRange;
     else
      actualRange=NSUnionRange(actualRange,effectiveRange);

     NSRangeEntryInsert(_invalidFragments,effectiveRange,invalid);
    }
   }
#endif
    // We could probably just invalidate paragraphs from the ones covering the charRange to the end of the dc
   _layoutInvalid=YES;
}

-(void)invalidateDisplayForGlyphRange:(NSRange)glyphRange {
    
#if DEBUG_LM_LAYOUT
    NSLog(@"invalidateDisplayForGlyphRange: %@", NSStringFromRange(glyphRange));
#endif
    
   NSRange characterRange=[self characterRangeForGlyphRange:glyphRange actualGlyphRange:NULL];

   [self invalidateDisplayForCharacterRange:characterRange];
}

-(void)invalidateDisplayForCharacterRange:(NSRange)charRange {
    
#if DEBUG_LM_LAYOUT
    NSLog(@"invalidateDisplayForCharacterRange: %@", NSStringFromRange(charRange));
#define DEBUG_invalidateDisplayForCharacterRange 0
#endif
    
   int i,count=[_textContainers count];

//   charRange=[self validateGlyphsAndLayoutForGlyphRange:charRange];

   for(i=0;i<count;i++){
    NSTextContainer *container=[_textContainers objectAtIndex:i];
    NSTextView      *textView=[container textView];

    [textView sizeToFit];
    [textView setNeedsDisplay:YES];

#if DEBUG_invalidateDisplayForCharacterRange
       NSLog(@"told textView: %@ to redisplay", textView);
#endif
       
   }
//FIX
}

// must be a more official way to do this
-(void)fixupSelectionInRange:(NSRange)range changeInLength:(int)changeInLength {

#if DEBUG_LM_LAYOUT
    NSLog(@"fixupSelectionInRange: %@ changeInLength: %d", NSStringFromRange(range), changeInLength);
#define DEBUG_fixupSelectionInRange_changeInLength 0
#endif
    
   int i,count=[_textContainers count];

	for(i=0;i<count;i++){
		NSTextContainer *container=[_textContainers objectAtIndex:i];
		NSTextView      *textView=[container textView];
		if (textView) {
			NSRange selectedRange = [textView selectedRange];
			NSRange textRange = NSMakeRange(0, [_textStorage length]);
			NSRange range = NSIntersectionRange(selectedRange, textRange);
			if (!NSEqualRanges(selectedRange, range)) {
#if DEBUG_fixupSelectionInRange_changeInLength
                NSLog(@"textView: %@ setSelectedRange: %@", textView, NSStringFromRange(range));
#endif
				[textView setSelectedRange:range];
			}
		}
	}
}

-(void)textStorage:(NSTextStorage *)storage edited:(unsigned)editedMask range:(NSRange)range changeInLength:(int)changeInLength invalidatedRange:(NSRange)invalidateRange {
    
#if DEBUG_LM_LAYOUT
    NSLog(@"textStorage: %@ edited: %u range: %@ changeInLength: %d invalidatedRange: %@",
          storage,
          editedMask,
          NSStringFromRange(range),
          changeInLength,
          NSStringFromRange(invalidateRange));
#define DEBUG_textStorage_edited_range_changeInLength_invalidatedRange 0
#endif
    
   NSRange actualRange;

   [self invalidateGlyphsForCharacterRange:invalidateRange changeInLength:changeInLength actualCharacterRange:&actualRange];

   [self invalidateLayoutForCharacterRange:actualRange isSoft:NO actualCharacterRange:&actualRange];

   [self invalidateDisplayForCharacterRange:actualRange];

   [self fixupSelectionInRange:range changeInLength:changeInLength];
	
	// We also have to fix the temporary attributes on that range
	if (editedMask & NSTextStorageEditedCharacters) {
#if DEBUG_textStorage_edited_range_changeInLength_invalidatedRange
        NSLog(@"fixing temporary attributes on that range...");
#endif
		// Update the temporary attributes ranges according to the changes
		NSRange oldRange = range;
		oldRange.length -= changeInLength;
		NSRangeEntriesExpandAndWipe(_rangeToTemporaryAttributes,oldRange,changeInLength);
		// And clear the attributes for the new part
		if (range.length) {
#if DEBUG_textStorage_edited_range_changeInLength_invalidatedRange
            NSLog(@"clear the attributes for the new part...");
#endif
			[self setTemporaryAttributes:nil forCharacterRange:range];
		}
		NSRangeEntriesVerify(_rangeToTemporaryAttributes,[_textStorage length]);
	}
}

-(void)textContainerChangedGeometry:(NSTextContainer *)container {

#if DEBUG_LM_LAYOUT
    NSLog(@"textContainerChangedGeometry: %@", container);
#endif
    
   NSRange range=NSMakeRange(0,[_textStorage length]);

   [self invalidateLayoutForCharacterRange:range isSoft:NO actualCharacterRange:NULL];
}

-(void)ensureLayoutForTextContainer:(NSTextContainer *)container
{
    
#if DEBUG_LM_LAYOUT
    NSLog(@"ensureLayoutForTextContainer: %@", container);
#endif
    
    [self validateGlyphsAndLayoutForContainer:container];
}

-(unsigned)glyphIndexForPoint:(NSPoint)point inTextContainer:(NSTextContainer *)container fractionOfDistanceThroughGlyph:(float *)fraction {

#if DEBUG_LM_LAYOUT
    NSLog(@"glyphIndexForPoint: %@ inTextContainer: %@ fractionOfDistanceThroughGlyph: %p", NSStringFromPoint(point), container, fraction);
#define DEBUG_glyphIndexForPoint_inTextContainer_fractionOfDistanceThroughGlyph 0
#endif
    
    unsigned          endOfFragment=0;
    unsigned          result=NSMaxRange([self glyphRangeForTextContainer:container]);
   NSRange           range;
   NSGlyphFragment  *fragment;
   NSRangeEnumerator state;

   [self validateGlyphsAndLayoutForContainer:container];

   *fraction=0;

   state=NSRangeEntryEnumerator(_glyphFragments);

   while(NSNextRangeEnumeratorEntry(&state,&range,(void **)&fragment)){
    if(point.y<NSMinY(fragment->rect)){
     if(endOfFragment>0){
// if we're at the end of a line we want to back up before the newline
// This is a very ugly way to do it
         if([[_textStorage string] characterAtIndex:endOfFragment-1]=='\n') {
#if DEBUG_glyphIndexForPoint_inTextContainer_fractionOfDistanceThroughGlyph
             NSLog(@"backing up before the newline");
#endif
             endOfFragment--;
         }
     }
#if DEBUG_glyphIndexForPoint_inTextContainer_fractionOfDistanceThroughGlyph
        NSLog(@"early returning: %u", endOfFragment);
#endif
        
     return endOfFragment;
    }

    if(point.y<NSMaxY(fragment->rect) && point.y>=NSMinY(fragment->rect)){
     if(point.x>=NSMinX(fragment->rect) && point.x<NSMaxX(fragment->usedRect)){

#if DEBUG_glyphIndexForPoint_inTextContainer_fractionOfDistanceThroughGlyph
         NSLog(@"point: %@ is inside the fragment", NSStringFromPoint(point));
#endif

      NSRect   glyphRect=fragment->usedRect;
      NSGlyph  glyphs[range.length];
      NSUInteger  order[range.length];
      NSFont  *font=[self _fontForGlyphRange:range];
      unsigned i,length=[self getOrderedGlyphs:glyphs range:range baseLevel:0 order:order];
      BOOL ltor = fragment->leftToRight;
         
      glyphRect.size.width=0;
 
      for(i=0;i<length;i++){
       NSGlyph glyph=glyphs[i];

       if(glyph!=NSControlGlyph){
        NSSize  advancement=[font advancementForGlyph:glyph];

        glyphRect.size.width=advancement.width;

        if(point.x>=NSMinX(glyphRect) && point.x<=NSMaxX(glyphRect)){
         *fraction=(point.x-glyphRect.origin.x)/glyphRect.size.width;
            if (ltor == NO) {
                // Right-to-left is flipped horizontally
                *fraction = 1. - *fraction;
            }
#if DEBUG_glyphIndexForPoint_inTextContainer_fractionOfDistanceThroughGlyph
            NSLog(@"found the glyph - returning: %u", range.location+order[i]);
#endif
         return range.location+order[i];
        }

        glyphRect.origin.x+=advancement.width;
        glyphRect.size.width=0;
       }
      }
     }
        // We're on a line with fragments, but no inside a segment - that's still a candidate
        
        // For the cases we hit a line at the left or right of all fragments
        if (point.x < NSMinX(fragment->rect)) {
            if (fragment->leftToRight) {
                result=range.location;
            } else {
                result=NSMaxRange(range);
            }
        } else if (point.x > NSMaxX(fragment->rect)) {
            if (fragment->leftToRight) {
                result=NSMaxRange(range);
            } else {
                result=range.location;
            }
        }
        endOfFragment=NSMaxRange(range);
    }
   }

#if DEBUG_glyphIndexForPoint_inTextContainer_fractionOfDistanceThroughGlyph
    NSLog(@"returning: %u", result);
#endif

    return result;
}

/* Apple's documentation claims glyphIndexForPoint:inTextContainer:fractionOfDistanceThroughGlyph: is implemented using these two methods. Verify. The method was split in two for the sake of Java, inefficient to keep it split */
-(unsigned)glyphIndexForPoint:(NSPoint)point inTextContainer:(NSTextContainer *)container {

#if DEBUG_LM_LAYOUT
    NSLog(@"glyphIndexForPoint: %@ inTextContainer: %@", NSStringFromPoint(point), container);
#endif
    
   float fraction;

   return [self glyphIndexForPoint:point inTextContainer:container fractionOfDistanceThroughGlyph:&fraction];
}

-(float)fractionOfDistanceThroughGlyphForPoint:(NSPoint)point inTextContainer:(NSTextContainer *)container {
    
#if DEBUG_LM_LAYOUT
    NSLog(@"fractionOfDistanceThroughGlyphForPoint: %@ inTextContainer: %@", NSStringFromPoint(point), container);
#define DEBUG_fractionOfDistanceThroughGlyphForPoint_inTextContainer 0
#endif
    
   float fraction;

   [self glyphIndexForPoint:point inTextContainer:container fractionOfDistanceThroughGlyph:&fraction];

   return fraction;
}

// Returns the current glyph range for the given container
-(NSRange)_currentGlyphRangeForTextContainer:(NSTextContainer *)container 
{
    
#if DEBUG_LM_LAYOUT
    NSLog(@"_currentGlyphRangeForTextContainer: %@", container);
#define DEBUG__currentGlyphRangeForTextContainer 0
#endif
    
	NSRange            result=NSMakeRange(0, 0);
	BOOL              assignFirst=YES;
	NSRangeEnumerator state=NSRangeEntryEnumerator(_glyphFragments);
	NSRange           range;
	NSGlyphFragment  *fragment;
	
	while(NSNextRangeEnumeratorEntry(&state,&range,(void **)&fragment)){
		if (fragment->container == container) 
		{
			if(assignFirst){
				result=range;
				assignFirst=NO;
			}
			else {
				result=NSUnionRange(result,range);
			}
		}
	
    }
#if DEBUG__currentGlyphRangeForTextContainer
    NSLog(@"returning: %@", NSStringFromRange(result));
#endif
    
	return result;
}

// Validate the glyphs and layout if needed and returns the glyph range for the given container
-(NSRange)glyphRangeForTextContainer:(NSTextContainer *)container 
{
#if DEBUG_LM_LAYOUT
    NSLog(@"glyphRangeForTextContainer: %@", container);
#endif
	[self validateGlyphsAndLayoutForContainer:container];
	return [self _currentGlyphRangeForTextContainer: container];
}

-(NSRange)glyphRangeForCharacterRange:(NSRange)charRange actualCharacterRange:(NSRangePointer)actualCharRange {

#if DEBUG_LM_LAYOUT
    NSLog(@"glyphRangeForCharacterRange: %@ actualCharacterRange: %p", NSStringFromRange(charRange), actualCharRange);
#endif
    
   if(actualCharRange!=NULL)
    *actualCharRange=charRange;

   return charRange;
}

-(NSRange)glyphRangeForBoundingRect:(NSRect)bounds inTextContainer:(NSTextContainer *)container {
    
#if DEBUG_LM_LAYOUT
    NSLog(@"glyphRangeForBoundingRect: %@ inTextContainer: %@", NSStringFromRect(bounds), container);
#define DEBUG_glyphRangeForBoundingRect_inTextContainer 0
#endif
    
	[self validateGlyphsAndLayoutForContainer:container];
	{
		NSRange           result=NSMakeRange(NSNotFound,0);
		NSRangeEnumerator state=NSRangeEntryEnumerator(_glyphFragments);
		NSRange           range;
		NSGlyphFragment  *fragment;
		
		while(NSNextRangeEnumeratorEntry(&state,&range,(void **)&fragment)){
			if (fragment->container == container) {
				NSRect check=fragment->rect;
				
				if(NSIntersectsRect(bounds,check)){
					NSRange extend=range;
					
					if(result.location==NSNotFound)
						result=extend;
					else
						result=NSUnionRange(result,extend);
				}
			}
		}
#if DEBUG_glyphRangeForBoundingRect_inTextContainer
        NSLog(@"returning: %@", NSStringFromRange(result));
#endif
		return result;
	}
}

-(NSRange)glyphRangeForBoundingRectWithoutAdditionalLayout:(NSRect)bounds inTextContainer:(NSTextContainer *)container {
   return NSMakeRange(0,0);
}

-(NSRange)rangeOfNominallySpacedGlyphsContainingIndex:(unsigned)glyphIndex {
   return NSMakeRange(0,0);
}

-(NSRect)boundingRectForGlyphRange:(NSRange)glyphRange inTextContainer:(NSTextContainer *)container {
    
#if DEBUG_LM_LAYOUT
    NSLog(@"boundingRectForGlyphRange: %@ inTextContainer: %@", NSStringFromRange(glyphRange), container);
#define DEBUG_boundingRectForGlyphRange_inTextContainer 0
#endif
    
   glyphRange=[self validateGlyphsAndLayoutForGlyphRange:glyphRange];
  {
   NSRect      result=NSZeroRect;
   unsigned    i,rectCount=0;
   NSRect * rects=[self rectArrayForGlyphRange:glyphRange withinSelectedGlyphRange:NSMakeRange(NSNotFound,0)
     inTextContainer:container rectCount:&rectCount];

   for(i=0;i<rectCount;i++){
    if(i==0)
     result=rects[i];
    else
     result=NSUnionRect(result,rects[i]);
   }
#if DEBUG_boundingRectForGlyphRange_inTextContainer
      NSLog(@"returning: %@", NSStringFromRect(result));
#endif
   return result;
  }
}

static inline void _appendRectToCache(NSLayoutManager *self,NSRect rect){
   if(self->_rectCacheCount>=self->_rectCacheCapacity){
    self->_rectCacheCapacity*=2;
    self->_rectCache=NSZoneRealloc(NULL,self->_rectCache,sizeof(NSRect)*self->_rectCacheCapacity);
   }

   self->_rectCache[self->_rectCacheCount++]=rect;
}

-(NSRect *)rectArrayForGlyphRange:(NSRange)glyphRange withinSelectedGlyphRange:(NSRange)selGlyphRange inTextContainer:(NSTextContainer *)container rectCount:(unsigned *)rectCount {

#if DEBUG_LM_LAYOUT
    NSLog(@"rectArrayForGlyphRange: %@ withinSelectedGlyphRange: %@ inTextContainer: %@ rectCount: %p",
          NSStringFromRange(glyphRange),
          NSStringFromRange(selGlyphRange),
          container,
          rectCount);
#define DEBUG_rectArrayForGlyphRange_withinSelectedGlyphRange_inTextContainer_rectCount 0
#endif
    
	NSRange remainder=(selGlyphRange.location==NSNotFound)?glyphRange:selGlyphRange;
	
	_rectCacheCount=0;
	do {
		NSRange          range;
		// Get the fragment to the range to process
		NSGlyphFragment *fragment=fragmentAtGlyphIndex(self,remainder.location,&range);
		
		if(fragment==NULL) {
            
#if DEBUG_rectArrayForGlyphRange_withinSelectedGlyphRange_inTextContainer_rectCount
            NSLog(@"fragment not found at index: %u - bailing", remainder.location);
#endif
            
			break;
        }
		else if (fragment->container == container) {
			// Part of the line fragment to process
			NSRange intersect=NSIntersectionRange(remainder,range);
			// The part of the that we are interested in - start with the full rect, we'll change it if we
			// don't want the full fragment
			NSRect  fill=fragment->rect;
			if(!NSEqualRanges(range,intersect)){
#if DEBUG_rectArrayForGlyphRange_withinSelectedGlyphRange_inTextContainer_rectCount
                NSLog(@"found a sub range to process: %@", NSStringFromRange(intersect));
#endif
				// We only want part of that fragment - so check the part we want by getting the
				// interesting glyphs locations
				
				// Use the usedRect - we're not interested in any potential white space lead
				fill=fragment->usedRect;

				NSGlyph glyphs[range.length],previousGlyph=NSNullGlyph;
                NSUInteger order[range.length];
				int     i,length=[self getOrderedGlyphs:glyphs range:range baseLevel:0 order:order];
				NSFont *font=[self _fontForGlyphRange:range];
				float   advance;
				BOOL    ignore;
				
                BOOL ltor = fragment->leftToRight;
                
				// Starts with a 0 width - we'll grow it with the width of the glyphs from our intersect range
				fill.size.width=0;
                
                if (ltor) {
                    for(i=0;i<length;i++){
                        NSGlyph glyph=glyphs[i];
                        
                        if(glyph==NSControlGlyph)
                            glyph=NSNullGlyph;
                        
                        advance=[font positionOfGlyph:glyph precededByGlyph:previousGlyph isNominal:&ignore].x;
                        if(range.location+i<=intersect.location) {
                            // Not yet part of intersect - advance the fill rect origin
                            fill.origin.x+=advance;
                        } else if(range.location+i<=NSMaxRange(intersect)) {
                            // Part of intersect - grow the width
                            fill.size.width+=advance;
                        }
                        previousGlyph=glyph;
                    }
                    if(NSMaxRange(range)<=NSMaxRange(remainder)){
                        // We want the full end of fragment, so grow the width to the end of the fragment rect
                        fill.size.width=NSMaxX(fragment->rect)-fill.origin.x;
                    }
				} else {
                    // Start from the right
                    fill.origin.x = NSMaxX(fragment->usedRect);
                    for(i=0;i<length;i++){
                        NSGlyph glyph=glyphs[order[i]];
                        
                        if(glyph==NSControlGlyph)
                            glyph=NSNullGlyph;
                        
                        advance=[font positionOfGlyph:glyph precededByGlyph:previousGlyph isNominal:&ignore].x;
                        if(range.location+i<=intersect.location) {
                            // Not yet part of intersect - advance the fill rect origin to the left
                            fill.origin.x-=advance;
                        } else if(range.location+i<=NSMaxRange(intersect)) {
                            // Part of intersect - grow the width to the left
                            fill.size.width+=advance;
                            fill.origin.x-=advance;
                        }
                        previousGlyph=glyph;
                    }
                    if(NSMaxRange(range)<=NSMaxRange(remainder)){
                        // We want the full start of fragment, so grow the width to the start of the fragment rect
                        float max = NSMaxX(fill);
                        fill.origin.x = fragment->rect.origin.x;
                        fill.size.width=max-fill.origin.x;
                    }
                }
				
				range = intersect;
			}

#if DEBUG_rectArrayForGlyphRange_withinSelectedGlyphRange_inTextContainer_rectCount
            NSLog(@"appending rect to cache: %@", NSStringFromRect(fill));
#endif
			
			_appendRectToCache(self,fill);
		}
		// Remove the range we just processed
		remainder.length=NSMaxRange(remainder)-NSMaxRange(range);
		remainder.location=NSMaxRange(range);
		
	} while(remainder.length>0);

	*rectCount=_rectCacheCount;

#if DEBUG_rectArrayForGlyphRange_withinSelectedGlyphRange_inTextContainer_rectCount
    NSLog(@"returning %d rects", *rectCount);
#endif

	return _rectCache;
}

-(unsigned)characterIndexForGlyphAtIndex:(unsigned)glyphIndex {
// Validate glyphs;

   return glyphIndex;
}

-(NSRange)characterRangeForGlyphRange:(NSRange)glyphRange actualGlyphRange:(NSRangePointer)actualGlyphRange {
   if(actualGlyphRange!=NULL)
    *actualGlyphRange=glyphRange;

   return glyphRange;
}

-(NSRect *)rectArrayForCharacterRange:(NSRange)characterRange withinSelectedCharacterRange:(NSRange)selectedCharRange inTextContainer:(NSTextContainer *)container rectCount:(unsigned *)rectCount {

#if DEBUG_LM_LAYOUT
    NSLog(@"rectArrayForCharacterRange: %@ withinSelectedCharacterRange: %@ inTextContainer: %@ rectCount: %p",
          NSStringFromRange(characterRange),
          NSStringFromRange(selectedCharRange),
          container,
          rectCount);
#endif
    
   NSRange glyphRange=[self glyphRangeForCharacterRange:characterRange actualCharacterRange:NULL];
   NSRange glyphSelRange=[self glyphRangeForCharacterRange:selectedCharRange actualCharacterRange:NULL];

   return [self rectArrayForGlyphRange:glyphRange withinSelectedGlyphRange:glyphSelRange inTextContainer:container rectCount:rectCount];
}

-(unsigned)firstUnlaidGlyphIndex {
   return NSNotFound;
}

-(unsigned)firstUnlaidCharacterIndex {
   return NSNotFound;
}

-(void)getFirstUnlaidCharacterIndex:(unsigned *)charIndex glyphIndex:(unsigned *)glyphIndex {
   *charIndex=[self firstUnlaidCharacterIndex];
   *glyphIndex=[self firstUnlaidGlyphIndex];
}

#define DEBUG_LM_DRAWING 0

-(void)showPackedGlyphs:(char *)glyphs length:(unsigned)length glyphRange:(NSRange)glyphRange atPoint:(NSPoint)point font:(NSFont *)font color:(NSColor *)color printingAdjustment:(NSSize)printingAdjustment {

#if DEBUG_LM_DRAWING
    NSLog(@"showPackedGlyphs: %p length: %d glyphRange: %@ atPoint: %@ font: %@ color: %@ printingAdjustment: %@", glyphs, length, NSStringFromRange(glyphRange), NSStringFromPoint(point), font, color, NSStringFromSize(printingAdjustment));
#define DEBUG_LM_SHOWPACKEDGLYPHS 0
#endif
    
	CGContextRef context=NSCurrentGraphicsPort();
	CGGlyph     *cgGlyphs=(CGGlyph *)glyphs;
	int          cgGlyphsLength=length/2;
    CGSize advances[cgGlyphsLength];
    NSGlyph nsglyphs[cgGlyphsLength];
    for (int i = 0; i < cgGlyphsLength; ++i) {
        nsglyphs[i] = cgGlyphs[i];
    }
    [font getAdvancements:advances forGlyphs:nsglyphs count:cgGlyphsLength];
    
#if DEBUG_LM_SHOWPACKEDGLYPHS
    for (int i = 0; i < cgGlyphsLength; i++) {
        NSLog(@"glyph: %d advancement: %@", nsglyphs[i], NSStringFromSize(advances[i]));
    }
#endif
    
    CGContextSetTextPosition(context, point.x, point.y);
    CGContextShowGlyphsWithAdvances(context, cgGlyphs, advances, cgGlyphsLength);
}

-(void)drawSelectionAtPoint:(NSPoint)origin {
    
#if DEBUG_LM_DRAWING
    NSLog(@"drawSelectionAtPoint: %@", NSStringFromPoint(origin));
#define DEBUG_LM_DRAWSELECTIONATPOINT 0
#endif
    
   NSTextView *textView=[self textViewForBeginningOfSelection];
   NSColor    *selectedColor=[[textView selectedTextAttributes] objectForKey:NSBackgroundColorAttributeName];
   NSRange     range;
   NSRect * rectArray;
   unsigned    i,rectCount=0;

   if(textView==nil)
    return;

   range=[textView selectedRange];
   if(range.length==0)
    return;

   rectArray=[self rectArrayForGlyphRange:range withinSelectedGlyphRange:range inTextContainer:[textView textContainer] rectCount:&rectCount];

#if DEBUG_LM_DRAWSELECTIONATPOINT
    NSLog(@"    rectArray: %@", rectArray);
#endif
    
   if(selectedColor==nil)
    selectedColor=[NSColor selectedTextBackgroundColor];

   [selectedColor setFill];
   for(i=0;i<rectCount;i++){
    NSRect fill=rectArray[i];
    fill.origin.x+=origin.x;
    fill.origin.y+=origin.y;
    NSRectFill(fill);
   }
}

-(void)drawBackgroundForGlyphRange:(NSRange)glyphRange atPoint:(NSPoint)origin {

#if DEBUG_LM_DRAWING
    NSLog(@"drawBackgroundForGlyphRange: %@ atPoint: %@", NSStringFromRange(glyphRange), NSStringFromPoint(origin));
#define DEBUG_LM_DRAWBACKGROUNDFORGLYPHRANGE 0
#endif

    glyphRange=[self validateGlyphsAndLayoutForGlyphRange:glyphRange];
   {
    NSTextContainer *container=[self textContainerForGlyphAtIndex:glyphRange.location effectiveRange:&glyphRange];
	   if (container == nil) {
		   return;
	   }
	NSRange          characterRange=[self characterRangeForGlyphRange:glyphRange actualGlyphRange:NULL];
    unsigned         location=characterRange.location;
    unsigned         limit=NSMaxRange(characterRange);
    BOOL             isFlipped=[[NSGraphicsContext currentContext] isFlipped];
    float            usedHeight=[self usedRectForTextContainer:container].size.height;
    
    while(location<limit){
     NSRange          effectiveRange;
     NSDictionary    *attributes=[_textStorage attributesAtIndex: location effectiveRange: &effectiveRange];
     // Make sure the effectiveRange doesn't take us backwards!
     int reduction = location - effectiveRange.location;
     effectiveRange.location = location;
     effectiveRange.length -= reduction;

    BOOL continueSearch = YES;
    // We've got to look carefully because we need to chop the effectiveRange so that the temporary attributes get added at the right places
    for (int tempOffset = effectiveRange.location; continueSearch && tempOffset < NSMaxRange(effectiveRange); tempOffset++) {
        NSRange          tempEffectiveRange = NSMakeRange(0, 0);
        NSDictionary    *tempAttrs = [self temporaryAttributesAtCharacterIndex: tempOffset effectiveRange: &tempEffectiveRange];
        if (NSEqualRanges(effectiveRange, tempEffectiveRange)) {
            // Things are lined up nicely - we can just use the effectiveRange as is
            continueSearch = NO;
        } else {
            if ([tempAttrs count] > 0) {
                // We've got some real attributes to worry about
                if (tempOffset > effectiveRange.location) {
                    // The temporary attributes start further along so cut effectiveRange down
                    // so it ends at the start of the temp range
                    effectiveRange.length = tempOffset - effectiveRange.location;
                    continueSearch = NO;
                }
                else if (tempEffectiveRange.length < effectiveRange.length) {
                    // The temp range starts at the same point as the effective range - but it's shorter
                    // so cut the effectiveRange length down to match.
                    effectiveRange.length = tempEffectiveRange.length;
                    continueSearch = NO;
                }
            }
        }

    }
        
    NSRange          tempEffectiveRange = NSMakeRange(0, 0);
    NSDictionary    *tempAttrs = [self temporaryAttributesAtCharacterIndex: location effectiveRange: &tempEffectiveRange];
    if ([tempAttrs count] > 0) {
        // Merge in the temporary attributes
        BOOL checkTemporaryAttributesUsage = [_delegate respondsToSelector:@selector(layoutManager:shouldUseTemporaryAttributes:forDrawingToScreen:atCharacterIndex:effectiveRange:)];
        if (checkTemporaryAttributesUsage) {
            tempAttrs = [_delegate layoutManager: self shouldUseTemporaryAttributes: tempAttrs forDrawingToScreen: [[NSGraphicsContext currentContext] isDrawingToScreen] atCharacterIndex: location effectiveRange: NULL];
        }
        NSMutableDictionary *dict = [[attributes mutableCopy] autorelease];
        [dict addEntriesFromDictionary: tempAttrs];
        attributes = dict;
    }

	 NSColor         *color=[attributes objectForKey:NSBackgroundColorAttributeName];

     effectiveRange=NSIntersectionRange(characterRange,effectiveRange);

     if(color!=nil){
      unsigned         i,rectCount;
      NSRect *      rects=[self rectArrayForCharacterRange:effectiveRange withinSelectedCharacterRange:NSMakeRange(NSNotFound,0) inTextContainer:container rectCount:&rectCount];

      [color setFill];

      for(i=0;i<rectCount;i++){
       NSRect fill=rects[i];

       if(!isFlipped)
        fill.origin.y=usedHeight-(fill.origin.y+fill.size.height);

       fill.origin.x+=origin.x;
       fill.origin.y+=origin.y;
        
       NSRectFill(fill);
      }
     }

     location=NSMaxRange(effectiveRange);
    }
   }
	[self drawSelectionAtPoint:origin];
}

- (void)drawUnderlineForGlyphRange:(NSRange)glyphRange underlineType:(NSInteger)underlineVal baselineOffset:(CGFloat)baselineOffset lineFragmentRect:(NSRect)lineRect lineFragmentGlyphRange:(NSRange)lineGlyphRange containerOrigin:(NSPoint)containerOrigin
{
#if DEBUG_LM_DRAWING
    NSLog(@"drawUnderlineForGlyphRange: %@ underlineType: %@ baselineOffset: %f lineFragmentRect: %@ lineFragmentGlyphRange: %@ containerOrigin: %@",
                                        NSStringFromRange(glyphRange),
                                        underlineVal,
                                        baselineOffset,
                                        NSStringFromRect(lineRect),
                                        NSStringFromRange(lineGlyphRange),
                                        NSStringFromPoint(containerOrigin));
#define DEBUG_LM_DRAWUNDERLINEFORGLYPHRANGE 0
#endif

    unsigned i,rectCount;
	NSRange characterRange = [self characterRangeForGlyphRange: glyphRange actualGlyphRange:NULL];
    BOOL             isFlipped = [[NSGraphicsContext currentContext] isFlipped];
	NSTextContainer* container = [self textContainerForGlyphAtIndex: glyphRange.location effectiveRange: NULL];

    NSRect *rects = [self rectArrayForCharacterRange: characterRange
						withinSelectedCharacterRange: NSMakeRange(NSNotFound,0)
									 inTextContainer: container
										   rectCount:&rectCount];
	
	NSBezierPath *path = [NSBezierPath bezierPath];
	
	// Lots more stylistic options available than just this
	[path setLineWidth: (underlineVal & NSUnderlineStyleThick) ? 1 : .5 ];
	[path setLineCapStyle:NSSquareLineCapStyle];
	if (underlineVal & NSUnderlinePatternDash) {
		CGFloat lineDash[] = {.75, 3.25};
		[path setLineDash:lineDash count:sizeof(lineDash)/sizeof(lineDash[0]) phase:0.0];
	}
	
	NSPoint origin = containerOrigin;
	
    for(i=0;i<rectCount;i++){
        NSRect fill=rects[i];
        
        if(isFlipped)
            fill.origin.y+=(fill.size.height - 1);
        
        fill.origin.x+=origin.x;
        fill.origin.y+=origin.y + baselineOffset - .5; // .5 so it's better aligned on pixels - looks sharper
        [path moveToPoint:fill.origin];
		float width = fill.size.width;
		[path relativeLineToPoint:NSMakePoint(width, 0)];
    }
    
	NSDictionary *attributes=[_textStorage attributesAtIndex:characterRange.location effectiveRange:NULL];
    // Don't forget the temporary attributes
    NSDictionary *tmpAttrs = [self temporaryAttributesAtCharacterIndex:characterRange.location effectiveRange:NULL];
    if ([tmpAttrs count] > 0) {
        BOOL checkTemporaryAttributesUsage = [_delegate respondsToSelector:@selector(layoutManager:shouldUseTemporaryAttributes:forDrawingToScreen:atCharacterIndex:effectiveRange:)];
        if (checkTemporaryAttributesUsage) {
            tmpAttrs = [_delegate layoutManager: self shouldUseTemporaryAttributes: tmpAttrs forDrawingToScreen: [[NSGraphicsContext currentContext] isDrawingToScreen] atCharacterIndex: characterRange.location effectiveRange: NULL];
        }
        NSMutableDictionary *dict = [[attributes mutableCopy] autorelease];
        [dict addEntriesFromDictionary:tmpAttrs];
        attributes = dict;
    }

	NSColor* underlineColor = [attributes objectForKey: NSUnderlineColorAttributeName];
	if (underlineColor == nil) {
        // Default to foreground color attribute...
        underlineColor = [attributes objectForKey: NSForegroundColorAttributeName];
        if (underlineColor == nil) {
            // ... and to black if still no luck
            underlineColor = [NSColor blackColor];
        }
	}
	[underlineColor set];
	[path stroke];
	
}

- (void)underlineGlyphRange:(NSRange)glyphRange underlineType:(NSInteger)underlineVal lineFragmentRect:(NSRect)lineRect lineFragmentGlyphRange:(NSRange)lineGlyphRange containerOrigin:(NSPoint)containerOrigin
{
	// A full implementation would honor options like breaking the underline for whitespace.
	[self drawUnderlineForGlyphRange: glyphRange underlineType: underlineVal baselineOffset: 0 lineFragmentRect: lineRect lineFragmentGlyphRange: lineGlyphRange containerOrigin: containerOrigin];
}

- (void)drawStrikethroughForGlyphRange:(NSRange)glyphRange strikethroughType:(NSInteger)strikethroughVal baselineOffset:(CGFloat)baselineOffset lineFragmentRect:(NSRect)lineRect lineFragmentGlyphRange:(NSRange)lineGlyphRange containerOrigin:(NSPoint)containerOrigin
{
#if DEBUG_LM_DRAWING
    NSLog(@"drawStrikethroughForGlyphRange: %@ strikethroughType: %@ baselineOffset: %f lineFragmentRect: %@ lineFragmentGlyphRange: %@ containerOrigin: %@",
          NSStringFromRange(glyphRange),
          strikethroughVal,
          baselineOffset,
          NSStringFromRect(lineRect),
          NSStringFromRange(lineGlyphRange),
          NSStringFromPoint(containerOrigin));
#define DEBUG_LM_DRAWSTRIKETHROUGHFORGLYPHRANGE 0
#endif

    unsigned i,rectCount;
	NSRange characterRange = [self characterRangeForGlyphRange: glyphRange actualGlyphRange:NULL];
    BOOL             isFlipped=[[NSGraphicsContext currentContext] isFlipped];
	NSTextContainer* container = [self textContainerForGlyphAtIndex: glyphRange.location effectiveRange: NULL];
    NSRect *rects=[self rectArrayForCharacterRange:characterRange withinSelectedCharacterRange:NSMakeRange(NSNotFound,0) inTextContainer:container rectCount:&rectCount];
	
	NSBezierPath *path = [NSBezierPath bezierPath];
	
	// Lots more stylistic options available than just this
	[path setLineWidth: (strikethroughVal & NSUnderlineStyleThick) ? 1 : .5 ];
	[path setLineCapStyle:NSSquareLineCapStyle];
	if (strikethroughVal & NSUnderlinePatternDash) {
		CGFloat lineDash[] = {.75, 3.25};
		[path setLineDash:lineDash count:sizeof(lineDash)/sizeof(lineDash[0]) phase:0.0];
	}
	
	NSPoint origin = containerOrigin;
	
    for(i=0;i<rectCount;i++){
        NSRect fill=rects[i];
        
		fill.origin.y+=(fill.size.height/2);
        
        fill.origin.x+=origin.x;
        fill.origin.y+=origin.y + baselineOffset;
        [path moveToPoint:fill.origin];
		float width = fill.size.width;
		[path relativeLineToPoint:NSMakePoint(width, 0)];
    }
	
	NSDictionary *attributes=[_textStorage attributesAtIndex:characterRange.location effectiveRange:NULL];
	NSColor* underlineColor = [attributes objectForKey: NSUnderlineColorAttributeName];
	if (underlineColor == nil) {
		underlineColor = [NSColor blackColor];
	}
	[underlineColor set];
	[path stroke];	
}

- (void)strikethroughGlyphRange:(NSRange)glyphRange strikethroughType:(NSInteger)strikethroughVal lineFragmentRect:(NSRect)lineRect lineFragmentGlyphRange:(NSRange)lineGlyphRange containerOrigin:(NSPoint)containerOrigin
{
	// A full implementation would honor options like breaking the strikethrough for whitespace.
	[self drawStrikethroughForGlyphRange: glyphRange strikethroughType: strikethroughVal  baselineOffset: 0 lineFragmentRect: lineRect lineFragmentGlyphRange: lineGlyphRange containerOrigin: containerOrigin];
}

-(void)drawSpellingState:(NSNumber *)spellingState glyphRange:(NSRange)glyphRange container:(NSTextContainer *)container origin:(NSPoint)origin {
#if DEBUG_LM_DRAWING
    NSLog(@"drawSpellingState: %@ glyphRange: %@ container: %p origin: %@",
          spellingState,
          NSStringFromRange(glyphRange),
          container,
          NSStringFromPoint(origin));
#define DEBUG_LM_DRAWSPELLINGSTATE 0
#endif
	if ([container textView] == nil) {
		// Don't draw anything if we aren't editing
		return;
	}
    unsigned i,rectCount;
	NSRange characterRange = [self characterRangeForGlyphRange: glyphRange actualGlyphRange:NULL];
    BOOL             isFlipped=[[NSGraphicsContext currentContext] isFlipped];
    float            usedHeight=[self usedRectForTextContainer:container].size.height;
    NSRect *rects=[self rectArrayForCharacterRange:characterRange withinSelectedCharacterRange:NSMakeRange(NSNotFound,0) inTextContainer:container rectCount:&rectCount];
        
	NSBezierPath *path = [NSBezierPath bezierPath];
	[path setLineWidth:2.];
	[path setLineCapStyle:NSRoundLineCapStyle];
    CGFloat lineDash[] = {.75, 3.25};
	[path setLineDash:lineDash count:sizeof(lineDash)/sizeof(lineDash[0]) phase:0.0];

    for(i=0;i<rectCount;i++){
        NSRect fill=rects[i];
        
        if(isFlipped)
            fill.origin.y+=(fill.size.height-1);
        
        fill.origin.x+=origin.x + 2; // some margin because of the line cap
        fill.origin.y+=origin.y;
        [path moveToPoint:fill.origin];
		float width = fill.size.width;
		[path relativeLineToPoint:NSMakePoint(width, 0)];
    }
    [[NSColor redColor] setStroke];
	[path stroke];
}

- (NSColor*)_selectedColor
{
	NSTextView *textView=[self textViewForBeginningOfSelection];
	NSColor    *selectedColor=[[textView selectedTextAttributes] objectForKey:NSForegroundColorAttributeName];
	
	if(selectedColor==nil) {
		selectedColor=[NSColor selectedTextColor];
	}
	return selectedColor;
}
	
- (void)_drawGlyphs: (NSGlyph*)glyphs
					length: (unsigned)length
					 range: (NSRange)range
				   atPoint: (NSPoint)point
			   inContainer: (NSTextContainer*)container
			withAttributes: (NSDictionary*)attributes
					origin: (NSPoint)origin
{
#if DEBUG_LM_DRAWING
    NSLog(@"_drawGlyphs: %p length: %d range: %@ atPoint: %@ inContainer: %p withAttributes: %@ origin: %@",
          glyphs,
          length,
          NSStringFromRange(range),
          NSStringFromPoint(point),
          container,
          attributes,
          NSStringFromPoint(origin));
#define DEBUG_LM_DRAWGLYPHS 0
#endif

	NSColor      *color=NSForegroundColorAttributeInDictionary(attributes);
	NSFont       *font=NSFontAttributeInDictionary(attributes);

	// First draw the packed glyphs
	char          packedGlyphs[length*2]; // Specs says length*4+1 but needed buffer size is at max length*2 on Cocotron
	int packedGlyphsLength = NSConvertGlyphsToPackedGlyphs(glyphs, length, NSNativeShortGlyphPacking, packedGlyphs);
  [self showPackedGlyphs:packedGlyphs length:packedGlyphsLength glyphRange:range atPoint:point font:font color:color printingAdjustment:NSZeroSize];

	NSRange glyphRange = range;

	// Next take a look at the overprinting options
	BOOL		 underline = [[attributes objectForKey: NSUnderlineStyleAttributeName] boolValue];
	BOOL		 strikeThru = [[attributes objectForKey: NSStrikethroughStyleAttributeName] boolValue];
	
	if (underline || strikeThru) {
		NSRange lineGlyphRange;
		NSRect lineRect = [self lineFragmentRectForGlyphAtIndex: glyphRange.location effectiveRange: &lineGlyphRange];
		
		if (underline) {
			[self underlineGlyphRange: glyphRange underlineType: NSUnderlineStyleThick lineFragmentRect: lineRect lineFragmentGlyphRange: lineGlyphRange containerOrigin: origin];
		}
		
		if (strikeThru) {
			// Make sure we've got a good strikeThru color
			NSColor* strikeThruColor = [attributes objectForKey: NSStrikethroughColorAttributeName];
			if (strikeThruColor == nil) {
				strikeThruColor = [NSColor blackColor];
			}
			[self strikethroughGlyphRange: glyphRange strikethroughType: NSUnderlineStyleThick lineFragmentRect: lineRect lineFragmentGlyphRange: lineGlyphRange containerOrigin: origin];
		}
	}
	
	NSNumber	 *spellingState=[attributes objectForKey:NSSpellingStateAttributeName];
	if(spellingState!=nil){
		[self drawSpellingState:spellingState glyphRange: glyphRange container:container origin:origin];
	}
}

- (void)_drawAttachment:(NSTextAttachment*)attachment atCharacterIndex:(unsigned)index atPoint:(NSPoint)point
{
#if DEBUG_LM_DRAWING
    NSLog(@"_drawAttachment: %@ atCharacterIndex: %d atPoint: %@",
          attachment,
          index,
          NSStringFromPoint(point));
#define DEBUG_LM_DRAWATTACHMENT 0
#endif
	id <NSTextAttachmentCell> cell=[attachment attachmentCell];
	NSRect frame;
	
	frame.origin=point;
	frame.size=[cell cellSize];

	NSTextView *textView=[self textViewForBeginningOfSelection];

	[cell drawWithFrame:frame inView: textView characterIndex: index layoutManager:self];
}

- (float)_drawGlyphsForSubGlyphRange:(NSRange)range
						 forFragment:(NSGlyphFragment*)fragment
							atPoint: (NSPoint)origin
					 subRangeXOffset: (float)xOffset
						inContainer: (NSTextContainer*)container
					 withAttributes: (NSDictionary*)attributes
						 usedHeight: (float)usedHeight
{
#if DEBUG_LM_DRAWING
    NSLog(@"_drawGlyphsForSubGlyphRange: %@ forFragment: atPoint: %@ subRangeXOffset: %f inContainer: %P withAttributes: %P usedHeight: %f",
          NSStringFromRange(range),
//          fragment, // Trying to log fragment results in seg-fault.
          NSStringFromPoint(origin),
          xOffset,
          container,
          attributes,
          usedHeight);
#define DEBUG_LM_DRAWGLYPHSFORSUBGLYPHRANGE 0
#endif

	NSTextView *textView = [self textViewForBeginningOfSelection];

	NSColor	*selectedColor = [self _selectedColor];
	
	NSRange	selectedRange = (textView == nil) ? NSMakeRange(0, 0) : [textView selectedRange];
	
	NSRange characterRange = [self characterRangeForGlyphRange: range actualGlyphRange: NULL];

	NSRange intersectRange = NSIntersectionRange(selectedRange, characterRange);
		
	NSColor *color = NSForegroundColorAttributeInDictionary(attributes);
	
	NSTextAttachment *attachment = [attributes objectForKey:NSAttachmentAttributeName];
	
	// Get the location of the fragment
	NSPoint point = fragment->location;
	
	NSGraphicsContext *context = [NSGraphicsContext currentContext];
	BOOL isFlipped = [context isFlipped];

	// Correct its y-coord depending on the flippedness of the context
	if (!isFlipped) {
		point.y=usedHeight-point.y;
	}

	// Offset the point by the origin of the text container and the subRange offset - we break fragments
	// up because of the temp attributes...
	point.x += origin.x + xOffset;
	point.y += origin.y;
	
	if(attachment!=nil){
		// Draw the attachment at the calculated point
		[self _drawAttachment: attachment atCharacterIndex: characterRange.location atPoint: point];
		
	} else {
		
		// Load the glyphs for this run
		NSGlyph       glyphs[range.length];
		unsigned      glyphsLength;
		 
		glyphsLength = [self getOrderedGlyphs:glyphs range:range baseLevel:0 order:NULL];

		// Prepare the font - we're within an attribute run - so this font will be good for
		// this entire range
		NSFont *font = NSFontAttributeInDictionary(attributes);
		[font setInContext: context];
		
		// We intersect with the selection so we apparently need to tread carefully...
		if (intersectRange.length > 0){
            BOOL ltor = fragment->leftToRight;
            
            if (ltor == NO) {
                // Mirror the selection since the glyphs order is Right-to-Left
                unsigned int distanceToEnd = NSMaxRange(characterRange) - NSMaxRange(intersectRange);
                intersectRange.location = characterRange.location + distanceToEnd;
            }

			NSGlyph  previousGlyph=NSNullGlyph;
			float    partWidth = 0;
			unsigned i = 0;
			unsigned location = range.location;
			
			// We're going to identify runs of glyphs (unselected and selected) 
			for (i = 0; location <= NSMaxRange(range); i++, location++){
				
				unsigned offset = 0; 
				unsigned length = 0;
				BOOL     showGlyphs = NO;
				
				NSGlyph  glyph = (location < NSMaxRange(range)) ? glyphs[i] : NSNullGlyph;
				
				// Glyph is invisible so just translate it to a Null glyph
				if (glyph == NSControlGlyph) {
					glyph = NSNullGlyph;
				}
				
				// We've found the start of the selected range - so we need to draw the
				// glyphs we've processed so far
				if (location == intersectRange.location && location > range.location) {
					[color setFill];
					
					offset=0;
					
					// length is the difference between the two locations
					length= intersectRange.location - range.location;
					
					showGlyphs=YES;
				}
				
				// We're at the end of the selected range intersection so make sure the glyphs
				// are drawn against the selectedColor background
				else if (location == NSMaxRange(intersectRange)){
					[selectedColor setFill];
					
					offset = intersectRange.location - range.location;
					
					// The length is simply the length of the intersecting range
					length = intersectRange.length;
					
					showGlyphs=YES;
				}
				
				// We're at the end of our fragment range so switch back to the regular background color...
				// and show the remaining glyphs
				else if (location == NSMaxRange(range)){
					[color setFill];
					
					offset = NSMaxRange(intersectRange) - range.location;
					
					// the length is the difference between the end of the full range and the end of
					// the intersection range
					length = NSMaxRange(range) - NSMaxRange(intersectRange);
					
					showGlyphs=YES;
				}
				
				BOOL ignore = NO;

				// Make sure we keep track of how many points we've used with each one
				partWidth += [font positionOfGlyph:glyph precededByGlyph:previousGlyph isNominal:&ignore].x;

				if (showGlyphs) {
					// Show the range of glyphs specifed by offset and length
					NSRange subRange = NSMakeRange(range.location + offset, length);
					
					[self _drawGlyphs:glyphs+offset length: length  range: subRange atPoint: point inContainer: container withAttributes: attributes origin: origin];
					
					// And make sure we know where we are witin the fragment.
					point.x+=partWidth;
					partWidth=0;
				}
				
				previousGlyph=glyph;
			}
		}
		else {
			// No intersection with selection so we can just process the whole thing.
			[color setFill];
			NSGlyph  previousGlyph=NSNullGlyph;
			
			float partWidth = 0;
			BOOL ignore = NO;
			
			for (NSUInteger i = 0; i <= range.length; i++){
				NSGlyph  glyph = i < range.length ? glyphs[i] : NSNullGlyph;
				
				// Glyph is invisible so just translate it to a Null glyph
				if (glyph == NSControlGlyph) {
					glyph = NSNullGlyph;
				}
				partWidth += [font positionOfGlyph:glyph precededByGlyph:previousGlyph isNominal:&ignore].x;
				previousGlyph = glyph;
			}
			
			[self _drawGlyphs:glyphs length: glyphsLength range: range atPoint: point  inContainer: container withAttributes: attributes origin: origin];
			point.x += partWidth;
		}
	}
	// Return the distance travelled relative to the fragment origin (we added these on at the start of this method)
	float newXOffset = point.x - (origin.x + fragment->location.x);
	return newXOffset;
}

- (void)drawGlyphsForGlyphRange:(NSRange)glyphRange atPoint:(NSPoint)origin
{
    
#if DEBUG_LM_DRAWING
    NSLog(@"drawGlyphsForGlyphRange: %@ atPoint: %@",
          NSStringFromRange(glyphRange),
          NSStringFromPoint(origin));
#define DEBUG_LM_DRAWGLYPHSFORGLYPHRANGE 0
#endif

	NSTextView *textView = [self textViewForBeginningOfSelection];
	NSRange selectedRange = (textView == nil) ? NSMakeRange(0,0) : [textView selectedRange];
	
    glyphRange=[self validateGlyphsAndLayoutForGlyphRange:glyphRange];
	
	NSTextContainer *container=[self textContainerForGlyphAtIndex:glyphRange.location effectiveRange:NULL];
	if (container == nil) {
		// Not sure if this is ever a good thing - but it's certainly a bad thing and if there's no container within
		// which to layout - we're done.
		return;
	}
	float usedHeight = [self usedRectForTextContainer:container].size.height;

	NSRangeEnumerator state = NSRangeEntryEnumerator(_glyphFragments);
	NSRange range;
	NSGlyphFragment *fragment;
	
    
    BOOL checkTemporaryAttributesUsage = [_delegate respondsToSelector:@selector(layoutManager:shouldUseTemporaryAttributes:forDrawingToScreen:atCharacterIndex:effectiveRange:)];
    
	// Iterate over the glyph fragments (which identify runs of common attributes)
	while (NSNextRangeEnumeratorEntry(&state,&range,(void **)&fragment)) {

#if DEBUG_LM_DRAWGLYPHSFORGLYPHRANGE
        NSLog(@"    looking at range: %@", NSStringFromRange(range));
#endif
		// Reset the offset for this fragment
		float glyphXOffset = 0;

		// Find out if we're within the range to be drawn
		NSRange intersect=NSIntersectionRange(range,glyphRange);
		
		if (intersect.length > 0) {
			// We don't care about the actual range or effective range here because we know we're within
			// a glyph fragment which means that the character range and attributes are already limited
			// correctly.
			NSRange characterRange = [self characterRangeForGlyphRange: intersect actualGlyphRange: NULL];
			
			NSDictionary *attributes = [_textStorage attributesAtIndex: characterRange.location effectiveRange: NULL];

#if DEBUG_LM_DRAWGLYPHSFORGLYPHRANGE
            NSLog(@"    characterRange: %@", NSStringFromRange(characterRange));
            NSLog(@"    attributes: %@", attributes);
#endif

			BOOL tempAttributesInRange = NO;
            NSUInteger tempAttributesIndex = NSNotFound;
            
			if (NSCountRangeEntries(_rangeToTemporaryAttributes) > 0) {
				// But we do have to worry about the temporary attributes so let's do a quick check with the character range and find out
				// if we have any temp attributes to be concerned about
                NSUInteger index = characterRange.location;
				NSRange tempRange = NSMakeRange(0, 0);
                while (tempAttributesInRange == NO && index < NSMaxRange(characterRange)) {
                    if (NSRangeEntryAtIndex(_rangeToTemporaryAttributes, index, &tempRange) != nil) {
                        tempAttributesInRange = YES;
                        tempAttributesIndex = index;
                    } else {
                        index++;
                    }
                }
			}

#if DEBUG_LM_DRAWGLYPHSFORGLYPHRANGE
            NSLog(@"    temporaryAttributesInRange: %@", tempAttributesInRange ? @"YES" : @"NO");
            if (tempAttributesInRange) {
                NSRange tempRange;
                NSDictionary *tmpAttrs = [self temporaryAttributesAtCharacterIndex:tempAttributesIndex effectiveRange:&tempRange];
                NSLog(@"     temporaryAttributes: %@", tmpAttrs);
            }
#endif
            
			if (tempAttributesInRange) {
				
				// Ok - so we've got to proceed with caution - there are temp attributes
				unsigned length = 0;
				unsigned offset = characterRange.location;
				
				// Iterate over the characters in the range
				for (NSUInteger charIndex = characterRange.location; charIndex < NSMaxRange(characterRange); charIndex++, length++) {

					// We've found some temp attributes
					NSRange tempRange = NSMakeRange(0, 0);
					// Make sure we don't go beyond the remaining range of the current fragment
					NSRange remainingRange = NSMakeRange(charIndex, NSMaxRange(characterRange) - charIndex);
					if (NSRangeEntryAtIndex(_rangeToTemporaryAttributes, charIndex, &tempRange) != nil &&
						NSIntersectionRange(remainingRange, tempRange).length > 0) {
						
						if (length > 0) {
							// draw the glyphs that we've encountered so far (if any)
							NSRange subRange = NSMakeRange(offset, length);
							subRange = [self glyphRangeForCharacterRange: subRange actualCharacterRange: NULL];
							glyphXOffset = [self _drawGlyphsForSubGlyphRange: subRange forFragment: fragment atPoint: origin subRangeXOffset: glyphXOffset inContainer: container withAttributes: attributes usedHeight: usedHeight];
						}
												
						
						NSDictionary* tempAttrs = [self temporaryAttributesAtCharacterIndex: charIndex effectiveRange: &tempRange];
						
						if (checkTemporaryAttributesUsage) {
							tempAttrs = [_delegate layoutManager: self shouldUseTemporaryAttributes: tempAttrs forDrawingToScreen: [[NSGraphicsContext currentContext] isDrawingToScreen] atCharacterIndex: charIndex effectiveRange: &tempRange];
						}
					
						tempRange = NSIntersectionRange(remainingRange, tempRange);
						
						// Merge the temp attributes with the permanent attributes
						NSMutableDictionary *mergedAttrs = [[attributes mutableCopy] autorelease];
						[mergedAttrs addEntriesFromDictionary: tempAttrs];
						
						NSRange tempGlyphRange = [self glyphRangeForCharacterRange: tempRange actualCharacterRange: NULL];

						glyphXOffset = [self _drawGlyphsForSubGlyphRange: tempGlyphRange forFragment: fragment atPoint: origin subRangeXOffset: glyphXOffset inContainer: container withAttributes: mergedAttrs usedHeight: usedHeight];
						
						// And reset ready for the next run
						offset = NSMaxRange(tempRange);
						charIndex = offset - 1; // Make sure we leap over the temp range (one off to accommodate the auto-increment)

						length = -1; // accommodate the auto-increment
					}
				}
				if (offset < NSMaxRange(characterRange)  && length > 0) {
					// We've got some glyphs left over - so draw them too
					NSRange subRange = NSMakeRange(offset, length);
					subRange = [self glyphRangeForCharacterRange: subRange actualCharacterRange: NULL];
					[self _drawGlyphsForSubGlyphRange: subRange forFragment: fragment atPoint: origin subRangeXOffset: glyphXOffset inContainer: container withAttributes: attributes usedHeight: usedHeight];
				}
				
			} else {
				// We don't have any temp attributes - so draw this sub range of glyphs
				[self _drawGlyphsForSubGlyphRange:intersect forFragment: fragment atPoint: origin subRangeXOffset: 0 inContainer: container withAttributes: attributes usedHeight: usedHeight];
			}
		}
	}
}

// dwy
- (NSRange)_softLineRangeForCharacterAtIndex:(unsigned)location {
    NSRange result=NSMakeRange(location,0);
    int i, j;
    float origin;
    NSGlyphFragment *fragment;

    if (location >= [[_textStorage string] length])
        location = [[_textStorage string] length]-1;

    result = [self glyphRangeForCharacterRange:result actualCharacterRange:NULL];

    fragment = NSRangeEntryAtIndex(self->_glyphFragments, result.location, NULL);
    if (fragment == NULL)
        return result;
    
    origin = fragment->location.y;

    i = result.location;
    j = result.location;
    while ((fragment = NSRangeEntryAtIndex(self->_glyphFragments, i-1, NULL))) {
        if (fragment->location.y != origin)
            break;
        result.location=i;
        i--;
    }

    result.location = i;
    while ((fragment = NSRangeEntryAtIndex(self->_glyphFragments, j, NULL))) {
        if (fragment->location.y != origin)
            break;
        j++;
    }

    result.length = j - i;

#if 0    
// broken for empty lines
    // word-break fixup; best effort; produces some strange effects when a single word is wider than the view
    if ([[NSCharacterSet whitespaceAndNewlineCharacterSet] characterIsMember:[[_textStorage string] characterAtIndex:NSMaxRange(result)-1]])
        result.length--;
#endif

    return [self characterRangeForGlyphRange:result actualGlyphRange:NULL];
}

-(float)defaultLineHeightForFont:(NSFont *)font {
   return [font defaultLineHeightForFont];
}

- (NSDictionary *)temporaryAttributesAtCharacterIndex:(NSUInteger)charIndex effectiveRange:(NSRangePointer)effectiveCharRange
{
	NSDictionary *result;
	
	if (charIndex >= [_textStorage length]) {
		NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond length %d",charIndex, [_textStorage length]);
	}
	
	if ((result = NSRangeEntryAtIndex(_rangeToTemporaryAttributes, charIndex, effectiveCharRange)) == nil) {
		result=[NSDictionary dictionary];
	}
		
	// The string could be being mutated so these attributes could disappear on an unwary caller
	return [[result retain] autorelease];
}

- (void)setTemporaryAttributes:(NSDictionary *)attrs forCharacterRange:(NSRange)charRange
{
	if (attrs == nil) {
		attrs = [NSDictionary dictionary];
	} else {
		attrs = [[attrs copy] autorelease];
	}
	
	if ([_textStorage length] == 0) {
		NSResetRangeEntries(_rangeToTemporaryAttributes);
	}
	else if (charRange.length > 0) {
		// Make sure we don't go beyond the actual text storage range
		NSRange intersect = NSIntersectionRange(NSMakeRange(0, [_textStorage length]), charRange);

		NSRangeEntriesDivideAndConquer(_rangeToTemporaryAttributes, intersect);
		NSRangeEntryInsert(_rangeToTemporaryAttributes, intersect, attrs);
	}
	
	NSRangeEntriesVerify(_rangeToTemporaryAttributes, [_textStorage length]);

	[self invalidateDisplayForCharacterRange: charRange];
}

- (void)addTemporaryAttributes:(NSDictionary *)attrs forCharacterRange:(NSRange)charRange
{	
	NSUInteger location = charRange.location;
	NSUInteger limit = NSMaxRange(charRange);
    // clip the limit - Cocoa doesn't seem to complain when we exceed the text storage range
	limit = MIN(limit, [_textStorage length]);
    
	while (location < limit) {
		NSRange       effectiveRange;
		NSMutableDictionary *modify = [[[self temporaryAttributesAtCharacterIndex: location effectiveRange: &effectiveRange] mutableCopy] autorelease];
		NSRange       replace;
		
		[modify addEntriesFromDictionary: attrs];
		
		replace.location = MAX(location,effectiveRange.location);
		replace.length = MIN(NSMaxRange(charRange), NSMaxRange(effectiveRange)) - replace.location;
		
		[self setTemporaryAttributes: modify forCharacterRange: replace];
		
		location=NSMaxRange(replace);
	}
}

- (void)removeTemporaryAttribute:(NSString *)attrName forCharacterRange:(NSRange)charRange
{
	if ([_textStorage length] == 0) {
		// Nothing to do
		return;
	}

	NSUInteger location = charRange.location;
		 
	if (location >= [_textStorage length]) {
		NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond length %d", location, [_textStorage length]);
	}
	
	NSUInteger limit = MIN(NSMaxRange(charRange), [_textStorage length]);
	
	while(location<limit){
		NSRange       effectiveRange;
		NSDictionary *check = [self temporaryAttributesAtCharacterIndex: location effectiveRange:&effectiveRange];
		NSRange       replace;
		
		replace.location = location;
		replace.length = MIN(NSMaxRange(charRange),NSMaxRange(effectiveRange))-location;
		
		if ([check objectForKey:attrName] != nil) {
			NSMutableDictionary *modify=[[check mutableCopy] autorelease];
			
			[modify removeObjectForKey:attrName];
			
			[self setTemporaryAttributes:modify forCharacterRange:replace];
		}
		
		location = NSMaxRange(replace);
	}
}

- (id)temporaryAttribute:(NSString *)attrName atCharacterIndex:(NSUInteger)location effectiveRange:(NSRangePointer)range
{
    return [[self temporaryAttributesAtCharacterIndex:location effectiveRange:range] objectForKey:attrName];
}

- (id)temporaryAttribute:(NSString *)attrName atCharacterIndex:(NSUInteger)location longestEffectiveRange:(NSRangePointer)range inRange:(NSRange)rangeLimit
{
    id result = [self temporaryAttribute:attrName atCharacterIndex:location effectiveRange:range];
    if (range) {
        // Check if we can expand the range

        // Check if we can expand it before the found range
        NSRange effectiveRange;
        while (range->location > rangeLimit.location) {
            id attr = [self temporaryAttribute:attrName atCharacterIndex:range->location - 1 effectiveRange:&effectiveRange];
            if (attr == result || [attr isEqual:result]) {
                // Expand the found range location
                range->length = NSMaxRange(*range) - effectiveRange.location;
                range->location = effectiveRange.location;
            }  else {
                break;
            }
        }
        // Check if we can expand it after the found range
        while (NSMaxRange(*range) < NSMaxRange(rangeLimit)) {
            id attr = [self temporaryAttribute:attrName atCharacterIndex:NSMaxRange(*range) effectiveRange:&effectiveRange];
            if (attr == result || [attr isEqual:result]) {
                // Expand the found range length
                range->length = NSMaxRange(effectiveRange) - range->location;
            }  else {
                break;
            }
        }
        // Ensure we don't go outside of the rangeLimit
        *range = NSIntersectionRange(*range,rangeLimit);
    }
    return result;
}

- (NSDictionary *)temporaryAttributesAtCharacterIndex:(NSUInteger)location longestEffectiveRange:(NSRangePointer)range inRange:(NSRange)rangeLimit
{
    id result = [self temporaryAttributesAtCharacterIndex:location effectiveRange:range];
    if (range) {
        // Check if we can expand the range
        
        // Check if we can expand it before the found range
        NSRange effectiveRange;
        while (range->location > rangeLimit.location) {
            id attr = [self temporaryAttributesAtCharacterIndex:range->location - 1 effectiveRange:&effectiveRange];
            if (attr == result || [attr isEqual:result]) {
                // Expand the found range location
                range->length = NSMaxRange(*range) - effectiveRange.location;
                range->location = effectiveRange.location;
            }  else {
                break;
            }
        }
        // Check if we can expand it after the found range
        while (NSMaxRange(*range) < NSMaxRange(rangeLimit)) {
            id attr = [self temporaryAttributesAtCharacterIndex:NSMaxRange(*range) effectiveRange:&effectiveRange];
            if (attr == result || [attr isEqual:result]) {
                // Expand the found range length
                range->length = NSMaxRange(effectiveRange) - range->location;
            }  else {
                break;
            }
        }
        // Ensure we don't go outside of the rangeLimit
        *range = NSIntersectionRange(*range,rangeLimit);
    }
    return result;
}

- (void)addTemporaryAttribute:(NSString *)attrName value:(id)value forCharacterRange:(NSRange)charRange
{
	[self addTemporaryAttributes: [NSDictionary dictionaryWithObject: value forKey: attrName] forCharacterRange: charRange];
}

- (NSArray *)rulerMarkersForTextView:(NSTextView *)view paragraphStyle:(NSParagraphStyle *)style ruler:(NSRulerView *)ruler
{
    NSMutableArray *markers = [NSMutableArray array];
    
    float delta = view.textContainer.lineFragmentPadding + view.textContainerOrigin.x;
    
    // Add the margins markers
#if 0
    // Don't add these markers for now - their values are ignored by the layout manager
    NSRulerMarker *marker = nil;
    
    marker = [NSRulerMarker leftMarginMarkerWithRulerView:ruler location:style.headIndent + delta];
    [marker setRepresentedObject:@"NSHeadIndentRulerMarkerTag"];
    [markers addObject:marker];
    
    // Looks like tailIndent value is a bit more complex - see Cocoa specs
    marker = [NSRulerMarker rightMarginMarkerWithRulerView:ruler location:view.textContainer.containerSize.width - style.tailIndent - delta];
    [marker setRepresentedObject:@"NSTailIndentRulerMarkerTag"];
    [markers addObject:marker];
    
    marker = [NSRulerMarker firstIndentMarkerWithRulerView:ruler location:style.firstLineHeadIndent + delta];
    [marker setRepresentedObject:@"NSFirstLineHeadIndentRulerMarkerTag"];
    [markers addObject:marker];
#endif
    // Add the tab stops markers
    for (NSTextTab *textTab in style.tabStops) {
        NSRulerMarker *marker = nil;
        switch (textTab.tabStopType) {
            case NSLeftTabStopType:
                marker = [NSRulerMarker leftTabMarkerWithRulerView:ruler location:textTab.location + delta];
                break;
            case NSRightTabStopType:
                marker = [NSRulerMarker rightTabMarkerWithRulerView:ruler location:textTab.location + delta];
                break;
            case NSCenterTabStopType:
                marker = [NSRulerMarker centerTabMarkerWithRulerView:ruler location:textTab.location + delta];
                break;
            case NSDecimalTabStopType:
                marker = [NSRulerMarker decimalTabMarkerWithRulerView:ruler location:textTab.location + delta];
                break;
            default:
                break;
        }
        if (marker) {
            [marker setRepresentedObject:textTab];
            [marker setRemovable:YES];
            [markers addObject:marker];
        }
    }
    return markers;
}

- (NSView *)rulerAccessoryViewForTextView:(NSTextView *)view paragraphStyle:(NSParagraphStyle *)style ruler:(NSRulerView *)ruler enabled:(BOOL)isEnabled
{
    return nil;
}
@end

