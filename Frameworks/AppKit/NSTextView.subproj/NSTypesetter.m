#import <AppKit/NSTypesetter.h>
#import <AppKit/NSTextTab.h>
#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSAttributedString.h>
#import "NSTypesetter_concrete.h"
#import <AppKit/NSRaise.h>

@class NSAttributedString;

@implementation NSTypesetter

+allocWithZone:(NSZone *)zone {
   if(self==[NSTypesetter class])
    return NSAllocateObject([NSTypesetter_concrete class],0,zone);
    
   return [super allocWithZone:zone];
}

+(NSTypesetterBehavior)defaultTypesetterBehavior {
   return NSTypesetterLatestBehavior;
}

+(NSSize)printingAdjustmentInLayoutManager:(NSLayoutManager *)layoutManager forNominallySpacedGlyphRange:(NSRange)glyphRange packedGlyphs:(const unsigned char *)packedGlyphs count:(unsigned)count {
   return NSMakeSize(0,0);
}

+sharedSystemTypesetterForBehavior:(NSTypesetterBehavior)behavior {
   return [[[self alloc] init] autorelease];
}

+sharedSystemTypesetter {
   return [self sharedSystemTypesetterForBehavior:[self defaultTypesetterBehavior]];
}

-(void)dealloc {
   [_layoutManager release];
   _textContainers=nil;
   [_attributedString release];
   _string=nil;
   [super dealloc];
}

-(NSTypesetterBehavior)typesetterBehavior {
   return _behavior;
}

-(float)hyphenationFactor {
   return _hyphenationFactor;
}

-(float)lineFragmentPadding {
   return _lineFragmentPadding;
}

-(BOOL)usesFontLeading {
   return _usesFontLeading;
}

-(BOOL)bidiProcessingEnabled {
   return _bidiProcessingEnabled;
}

-(void)setTypesetterBehavior:(NSTypesetterBehavior)behavior {
   _behavior=behavior;
}

-(void)setHyphenationFactor:(float)factor {
   _hyphenationFactor=factor;
}

-(void)setLineFragmentPadding:(float)padding {
   _lineFragmentPadding=padding;
}

-(void)setUsesFontLeading:(BOOL)flag {
   _usesFontLeading=flag;
}

-(void)setBidiProcessingEnabled:(BOOL)flag {
   _bidiProcessingEnabled=flag;
}

-(NSRange)characterRangeForGlyphRange:(NSRange)glyphRange actualGlyphRange:(NSRange *)actualGlyphRange {
   return [_layoutManager characterRangeForGlyphRange:glyphRange actualGlyphRange:actualGlyphRange];
}

-(NSRange)glyphRangeForCharacterRange:(NSRange)characterRange actualCharacterRange:(NSRange *)actualCharacterRange {
   return [_layoutManager glyphRangeForCharacterRange:characterRange actualCharacterRange:actualCharacterRange];
}

-(unsigned)getGlyphsInRange:(NSRange)glyphRange glyphs:(NSGlyph *)glyphs characterIndexes:(unsigned *)characterIndexes glyphInscriptions:(NSGlyphInscription *)glyphInscriptions elasticBits:(BOOL *)elasticBits bidiLevels:(unsigned char *)bidiLevels {
    NSInvalidAbstractInvocation();
    return 0;
}

-(void)getLineFragmentRect:(NSRect *)fragmentRect usedRect:(NSRect *)usedRect remainingRect:(NSRect *)remainingRect forStartingGlyphAtIndex:(unsigned)startingGlyphIndex proposedRect:(NSRect)proposedRect lineSpacing:(float)lineSpacing paragraphSpacingBefore:(float)paragraphSpacingBefore paragraphSpacingAfter:(float)paragraphSpacingAfter {
   NSInvalidAbstractInvocation();
}

-(void)setLineFragmentRect:(NSRect)fragmentRect forGlyphRange:(NSRange)glyphRange usedRect:(NSRect)usedRect baselineOffset:(float)baselineOffset {
   [_layoutManager setLineFragmentRect:fragmentRect forGlyphRange:glyphRange usedRect:usedRect];
}

-(void)substituteGlyphsInRange:(NSRange)glyphRange withGlyphs:(NSGlyph *)glyphs {
   // do nothing
}

-(void)insertGlyph:(NSGlyph)glyph atGlyphIndex:(unsigned)glyphIndex characterIndex:(unsigned)characterIndex {
   [_layoutManager insertGlyph:glyph atGlyphIndex:glyphIndex characterIndex:characterIndex];
}

-(void)deleteGlyphsInRange:(NSRange)glyphRange {
   [_layoutManager deleteGlyphsInRange:glyphRange];
}

-(void)setNotShownAttribute:(BOOL)flag forGlyphRange:(NSRange)range {
   int i,max=NSMaxRange(range);
   
   for(i=range.location;i<max;i++)
    [_layoutManager setNotShownAttribute:flag forGlyphAtIndex:i];
}

-(void)setDrawsOutsideLineFragment:(BOOL)flag forGlyphRange:(NSRange)range {
   int i,max=NSMaxRange(range);
   
   for(i=range.location;i<max;i++)
    [_layoutManager setDrawsOutsideLineFragment:flag forGlyphAtIndex:i];
}

-(void)setLocation:(NSPoint)location withAdvancements:(const float *)nominalAdvancements forStartOfGlyphRange:(NSRange)glyphRange {
   [_layoutManager setLocation:location forStartOfGlyphRange:glyphRange];
}

-(void)setAttachmentSize:(NSSize)size forGlyphRange:(NSRange)glyphRange {
   [_layoutManager setAttachmentSize:size forGlyphRange:glyphRange];
}

-(void)setBidiLevels:(const unsigned char *)bidiLevels forGlyphRange:(NSRange)glyphRange {
    
   // do nothing
}

-(void)willSetLineFragmentRect:(NSRect *)fragmentRect forGlyphRange:(NSRange)glyphRange usedRect:(NSRect *)usedRect baselineOffset:(float *)baselineOffset {
   // do nothing
}

-(BOOL)shouldBreakLineByHyphenatingBeforeCharacterAtIndex:(unsigned)characterIndex {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)shouldBreakLineByWordBeforeCharacterAtIndex:(unsigned)characterIndex {
   NSInvalidAbstractInvocation();
   return NO;
}

-(float)hyphenationFactorForGlyphAtIndex:(unsigned)glyphIndex {
   return 0.0;
}

-(unichar)hyphenCharacterForGlyphAtIndex:(unsigned)glyphIndex {
   return '-';
}

-(NSRect)boundingBoxForControlGlyphAtIndex:(unsigned)glyphIndex forTextContainer:(NSTextContainer *)textContainer proposedLineFragment:(NSRect)proposedRect glyphPosition:(NSPoint)glyphPosition characterIndex:(unsigned)characterIndex {
   NSInvalidAbstractInvocation();
   return NSMakeRect(0,0,0,0);
}

-(NSAttributedString *)attributedString {
   return _attributedString;
}

-(NSDictionary *)attributesForExtraLineFragment {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSLayoutManager *)layoutManager {
   return _layoutManager;
}

-(NSArray *)textContainers {
   return _textContainers;
}

-(NSTextContainer *)currentTextContainer {
   return _currentTextContainer;
}

-(NSParagraphStyle *)currentParagraphStyle {
   return _currentParagraphStyle;
}

-(NSRange)paragraphCharacterRange {
   NSInvalidAbstractInvocation();
   return NSMakeRange(0,0);
}

-(NSRange)paragraphGlyphRange {
   NSInvalidAbstractInvocation();
   return NSMakeRange(0,0);
}

-(NSRange)paragraphSeparatorCharacterRange {
   NSInvalidAbstractInvocation();
   return NSMakeRange(0,0);
}

-(NSRange)paragraphSeparatorGlyphRange {
   NSInvalidAbstractInvocation();
   return NSMakeRange(0,0);
}

-(NSTypesetterControlCharacterAction)actionForControlCharacterAtIndex:(unsigned)characterIndex {
    unichar c = [[_attributedString string] characterAtIndex:characterIndex];
    if ([[NSCharacterSet newlineCharacterSet] characterIsMember:c]) {
        return NSTypesetterParagraphBreakAction;
    }
    switch(c){
        case '\n':
            return NSTypesetterParagraphBreakAction;
            
        case '\t':
            return NSTypesetterHorizontalTabAction;
            
        case 0x200B:
            return NSTypesetterZeroAdvancementAction;
            
        default:
            return NSTypesetterWhitespaceAction;
    }
}

-(NSFont *)substituteFontForFont:(NSFont *)font {
   return font;
}

-(void)setAttributedString:(NSAttributedString *)text {
   text=[text retain];
   [_attributedString release];
   _attributedString=text;
   _string=[_attributedString string];
}

-(void)setHardInvalidation:(BOOL)invalidate forGlyphRange:(NSRange)glyphRange {
   NSInvalidAbstractInvocation();
}

-(void)setParagraphGlyphRange:(NSRange)glyphRange separatorGlyphRange:(NSRange)separatorGlyphRange {
   NSInvalidAbstractInvocation();
}

-(void)beginLineWithGlyphAtIndex:(unsigned)glyphIndex {
   NSInvalidAbstractInvocation();
}

-(void)endLineWithGlyphRange:(NSRange)glyphRange {
   NSInvalidAbstractInvocation();
}

-(void)beginParagraph {
   NSInvalidAbstractInvocation();
}

-(void)endParagraph {
   NSInvalidAbstractInvocation();
}

-(float)baselineOffsetInLayoutManager:(NSLayoutManager *)layoutManager glyphIndex:(unsigned)glyphIndex {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSTextTab *)textTabForGlyphLocation:(float)location writingDirection:(NSWritingDirection)direction maxLocation:(float)maxLocation {
   NSArray *stops=[_currentParagraphStyle tabStops];
   int      i,count=[stops count];

   for(i=0;i<count;i++){
    NSTextTab *tab=[stops objectAtIndex:i];
    float      check=[tab location];

    if(check>maxLocation)
     break;
     
    if(check>location)
     return tab;
   }
   
   return nil;
}

-(void)getLineFragmentRect:(NSRect *)fragmentRect usedRect:(NSRect *)usedRect forParagraphSeparatorGlyphRange:(NSRange)glyphRange atProposedOrigin:(NSPoint)proposedOrigin {
   NSInvalidAbstractInvocation();
}

-(NSParagraphStyle *)_paragraphStyleAfterGlyphIndex:(unsigned)glyphIndex {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSParagraphStyle *)_paragraphStyleBeforeGlyphIndex:(unsigned)glyphIndex {
   NSInvalidAbstractInvocation();
   return nil;
}

-(float)lineSpacingAfterGlyphAtIndex:(unsigned)glyphIndex withProposedLineFragmentRect:(NSRect)rect {
   return [[self _paragraphStyleAfterGlyphIndex:glyphIndex] lineSpacing];
}

-(float)paragraphSpacingAfterGlyphAtIndex:(unsigned)glyphIndex withProposedLineFragmentRect:(NSRect)rect {
   return [[self _paragraphStyleAfterGlyphIndex:glyphIndex] paragraphSpacing];
}

-(float)paragraphSpacingBeforeGlyphAtIndex:(unsigned)glyphIndex withProposedLineFragmentRect:(NSRect)rect {
   return [[self _paragraphStyleBeforeGlyphIndex:glyphIndex] paragraphSpacingBefore];
}

-(unsigned)layoutParagraphAtPoint:(NSPoint *)point {
   [self beginParagraph];

   NSInvalidAbstractInvocation();
   
   [self endParagraph];
   return 0;
}

-(void)layoutGlyphsInLayoutManager:(NSLayoutManager *)layoutManager startingAtGlyphIndex:(unsigned)startGlyphIndex maxNumberOfLineFragments:(unsigned)maxNumLines nextGlyphIndex:(unsigned *)nextGlyph {
   NSInvalidAbstractInvocation();
}

#pragma mark NSGlyphStorage Protocol
-(unsigned)layoutOptions
{
    return 0;
}

-(void)insertGlyphs:(const NSGlyph *)glyphs length:(unsigned)length forStartingGlyphAtIndex:(unsigned)glyphIndex characterIndex:(unsigned int)characterIndex
{
    NSInvalidAbstractInvocation();
}

-(void)setIntAttribute:(int)intAttribute value:(int)value forGlyphAtIndex:(unsigned)glyphIndex;
{
    NSInvalidAbstractInvocation();    
}
@end
