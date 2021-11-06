/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSFont.h>
#import <AppKit/NSFontDescriptor.h>
#import <AppKit/NSFontFamily.h>
#import <AppKit/NSFontTypeface.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSGraphicsContextFunctions.h>
#import <ApplicationServices/ApplicationServices.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSRaise.h>

#import <Onyx2D/O2Font.h>

FOUNDATION_EXPORT char *NSUnicodeToSymbol(const unichar *characters,unsigned length,
  BOOL lossy,unsigned *resultLength,NSZone *zone);

@implementation NSNibFontNameTranslator
// It seems the default mapping should really go to some platform specific place
-(NSString *)translateToNibFontName:(NSString *)name {
	NSString *displayName = [O2Font displayNameForPostscriptName:name];
	if([displayName isEqual:@"Nimbus Sans-Regular"])
		return[O2Font postscriptNameForDisplayName:@"Helvetica"];
	if([displayName isEqual:@"Nimbus Sans-Bold"])
		return @"Helvetica-Bold";
	if([displayName isEqual:@"Nimbus Sans-Italic"])
		return @"Helvetica-Oblique";
	if([displayName isEqual:@"Nimbus Sans-Bold Italic"])
		return @"Helvetica-BoldOblique";
	
	if([displayName isEqual:@"Nimbus Roman-Regular"])
		return @"Times-Roman";
	if([displayName isEqual:@"Source Code Pro-Regular"])
		return @"Courier";
	
	return name;
}

-(NSString *)translateFromNibFontName:(NSString *)name {
	NSString *displayName = [O2Font displayNameForPostscriptName:name];

	if ([name isEqual:@"Helvetica"] || [name isEqual:@".AppleSystemUIFont"])
		return [O2Font postscriptNameForDisplayName:@"Nimbus Sans-Regular"];
	if([name isEqual:@"Helvetica-Bold"] || [name isEqual:@".AppleSystemUIFontBold"])
		return [O2Font postscriptNameForDisplayName:@"Nimbus Sans-Bold"];
	if([name isEqual:@"Helvetica-Oblique"])
		return [O2Font postscriptNameForDisplayName:@"Nimbus Sans-Italic"];
	if([name isEqual:@"Helvetica-BoldOblique"])
		return [O2Font postscriptNameForDisplayName:@"Nimbus Sans-Bold Italic"];
	
	if([name isEqual:@"Times-Roman"])
		return [O2Font postscriptNameForDisplayName:@"Nimbus Roman-Regular"];
	if([name isEqual:@"Ohlfs"])
		return [O2Font postscriptNameForDisplayName:@"Nimbus Mono PS-Regular"];
	if([name isEqual:@"Courier"])
		return [O2Font postscriptNameForDisplayName:@"Source Code Pro-Regular"];
	
	if([name isEqual:@"LucidaGrande"])
		return [O2Font postscriptNameForDisplayName:@"URW Bookman-Light"];
	if([name isEqual:@"LucidaGrande-Bold"])
		return [O2Font postscriptNameForDisplayName:@"URW Bookman-Demi"];
    
    // Special fonts used by Xcode 5 when compiling some xibs
	if([name isEqual:@".LucidaGrandeUI"])
		return [O2Font postscriptNameForDisplayName:@"URW Bookman-Light"];
	if([name isEqual:@".LucidaGrandeUI-Bold"])
		return [O2Font postscriptNameForDisplayName:@"URW Bookman-Demi"];
    
	if([name isEqual:@"HelveticaNeue-CondensedBold"])
		return [O2Font postscriptNameForDisplayName:@"Nimbus Sans Narrow-Bold"];
	if([name isEqual:@"HelveticaNeue-Bold"])
		return [O2Font postscriptNameForDisplayName:@"Nimbus Sans-Bold"];
	if([name isEqual:@"HelveticaNeue-Regular"])
		return [O2Font postscriptNameForDisplayName:@"Nimbus Sans-Regular"];

	return name;
}

@end

@implementation NSFont

static NSNibFontNameTranslator* _nibFontTranslator = nil;

static unsigned _fontCacheCapacity=0;
static unsigned _fontCacheSize=0;
static NSFont **_fontCache=NULL;

static NSLock *_cacheLock=nil;

+(void)initialize {
   if(self==[NSFont class]){
    _fontCacheCapacity=4;
    _fontCacheSize=0;
    _fontCache=NSZoneMalloc([self zone],sizeof(NSFont *)*_fontCacheCapacity);
	   _nibFontTranslator = [[NSNibFontNameTranslator alloc] init];
       _cacheLock = [[NSLock alloc] init];
   }
}

+(unsigned long)_cacheIndexOfFontWithName:(NSString *)name size:(float)size {
   unsigned long i;

   for(i=0;i<_fontCacheSize;i++){
    NSFont *check=_fontCache[i];

    if(check!=nil && [[check fontName] isEqualToString:name] && [check pointSize]==size)
     return i;
   }
    
   return NSNotFound;
}

+(NSFont *)cachedFontWithName:(NSString *)name size:(float)size {
    
    NSFont *font = nil;
    [_cacheLock lock];
    unsigned long i=[self _cacheIndexOfFontWithName:name size:size];
        
    font = (i==NSNotFound)?(NSFont *)nil:_fontCache[i];
    [_cacheLock unlock];
    return font;
}

+(void)addFontToCache:(NSFont *)font {
    
	if (font == nil) {
		return;
	}
   unsigned long i;

    [_cacheLock lock];
   for(i=0;i<_fontCacheSize;i++){
    if(_fontCache[i]==nil){
     _fontCache[i]=font;
        [_cacheLock unlock];
     return;
    }
   }

   if(_fontCacheSize>=_fontCacheCapacity){
    _fontCacheCapacity*=2;
    _fontCache=NSZoneRealloc([self zone],_fontCache,sizeof(NSFont *)*_fontCacheCapacity);
   }
   _fontCache[_fontCacheSize++]=font;
    [_cacheLock unlock];
}

+(void)removeFontFromCache:(NSFont *)font {
    [_cacheLock lock];
   unsigned long i=[self _cacheIndexOfFontWithName:[font fontName] size:[font pointSize]];

   if(i!=NSNotFound)
    _fontCache[i]=nil;
    [_cacheLock unlock];
}

+(float)systemFontSize {
   return 12.0;
}

+(float)smallSystemFontSize {
   return 10.0;
}

+(float)labelFontSize {
   return 12.0;
}

+(float)systemFontSizeForControlSize:(NSControlSize)size {
   switch(size){
    default:
    case NSRegularControlSize:
     return 13.0;
     
    case NSSmallControlSize:
     return 11.0;
     
    case NSMiniControlSize:
     return 9.0;
   }
}

+(NSFont *)_uiFontOfType:(CTFontUIFontType)type size:(float)size fallbackName:(NSString *)fallbackName
{
    NSFont *result = nil;
    CTFontRef ctFont=CTFontCreateUIFontForLanguage(type,size,nil);
    if (ctFont) {
        NSString *name=(NSString *)CTFontCopyFullName(ctFont);
        size=CTFontGetSize(ctFont);
        
        result=[NSFont fontWithName:name size:size];
        
        [ctFont release];
        [name release];
    } else {
        result = [NSFont fontWithName:[O2Font postscriptNameForDisplayName:fallbackName] size:size];
    }
    O2FontLog(@"asked for type: %d got font: %@", type, result);
    return result;
}

+(NSFont *)boldSystemFontOfSize:(float)size {
    NSFont *font = [self systemFontOfSize:size];
    return [[NSFontManager sharedFontManager] convertFont:font toHaveTrait:NSBoldFontMask];
}

+(NSFont *)controlContentFontOfSize:(float)size {
    return [self _uiFontOfType:kCTFontControlContentFontType size:(size==0)?10.0:size fallbackName:@"Nimbus Sans-Regular"];
}

+(NSFont *)labelFontOfSize:(float)size {
    return [self _uiFontOfType:kCTFontLabelFontType size:(size==0)?10.0:size fallbackName:@"Nimbus Sans-Regular"];
}

+(NSFont *)menuFontOfSize:(float)size {
    return [self _uiFontOfType:kCTFontMenuItemFontType size:size fallbackName:@"Nimbus Sans-Regular"];
}

+(NSFont *)menuBarFontOfSize:(float)size {
    return [self _uiFontOfType:kCTFontMenuTitleFontType size:size fallbackName:@"Nimbus Sans-Regular"];
}

+(NSFont *)messageFontOfSize:(float)size {
    return [self _uiFontOfType:kCTFontSystemFontType size:(size==0)?10.0:size fallbackName:@"Nimbus Sans-Regular"];
}

+(NSFont *)paletteFontOfSize:(float)size {
    return [self _uiFontOfType:kCTFontPaletteFontType size:(size==0)?10.0:size fallbackName:@"Nimbus Sans-Regular"];
}

+(NSFont *)systemFontOfSize:(float)size {
   return [self messageFontOfSize:size];
}

+(NSFont *)titleBarFontOfSize:(float)size {
    return [self boldSystemFontOfSize:size];
}

+(NSFont *)toolTipsFontOfSize:(float)size {
    return [self _uiFontOfType:kCTFontToolTipFontType size:(size==0)?9.:size fallbackName:@"Nimbus Sans-Regular"];
}

+(NSFont *)userFontOfSize:(float)size {
   return [NSFont fontWithName:[O2Font postscriptNameForDisplayName:@"Nimbus Sans-Regular"] size:(size==0)?10.0:size];
}

+(NSFont *)userFixedPitchFontOfSize:(float)size {
   return [NSFont fontWithName:[O2Font postscriptNameForDisplayName:@"Source Code Pro-Regular"] size:(size==0)?12.0:size];
}

+(void)setUserFont:(NSFont *)value {
   NSUnimplementedMethod();
}

+(void)setUserFixedPitchFont:(NSFont *)value {
   NSUnimplementedMethod();
}

+(NSArray *)preferredFontNames
{
    return [O2Font preferredFontNames];
}

+(void)setPreferredFontNames:(NSArray *)fontNames
{
    [O2Font setPreferredFontNames:fontNames];
}


-(void)encodeWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
     [coder encodeObject:[[NSFont nibFontTranslator] translateToNibFontName:_name] forKey:@"NSName"];
     [coder encodeFloat:_pointSize forKey:@"NSSize"];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"%@ can not encodeWithCoder:%@",isa,[coder class]];
   }
}

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
       NSString *fontName = [keyed decodeObjectForKey:@"NSName"];
    NSString          *name=[[NSFont nibFontTranslator] translateFromNibFontName: fontName];
    float              size=[keyed decodeFloatForKey:@"NSSize"];
    // int                flags=[keyed decodeIntForKey:@"NSfFlags"]; // ?
    
    [self dealloc];
    
    NSFont *realFont = [[NSFont fontWithName:name size:size] retain];
       O2FontLog(@"coded font name: %@ translated font name: %@ rendered font: %@", fontName, name, realFont);
       return realFont;
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"%@ can not initWithCoder:%@",isa,[coder class]];
   }
   return nil;
}


-initWithName:(NSString *)name size:(float)size {
   _name=[name copy];
   _pointSize=size;
   _matrix[0]=_pointSize;
   _matrix[1]=0;
   _matrix[2]=0;
   _matrix[3]=_pointSize;
   _matrix[4]=0;
   _matrix[5]=0;

   if([_name isEqualToString:@"Symbol"])
    _encoding=NSSymbolStringEncoding;
   else
    _encoding=NSUnicodeStringEncoding;

	_cgFont=CGFontCreateWithFontName((CFStringRef)_name);
	if (_cgFont) {
		_ctFont=CTFontCreateWithGraphicsFont(_cgFont,_pointSize,NULL,NULL);
		[isa addFontToCache:self];
        O2FontLog(@"name: %@ _cgFont: %@ _ctFont: %@", name, _cgFont, _ctFont);
	} else {
		[self release];
		self = nil;
	}
	return self;
}

-(void)dealloc {
   [isa removeFontFromCache:self];

   [_name release];
   CGFontRelease(_cgFont);
   [_ctFont release];
   [super dealloc];
}

+(NSFont *)fontWithName:(NSString *)name size:(float)size {
   NSFont *result;

   if(name==nil)
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] name==nil",self,sel_getName(_cmd)];

	// Name can be PS name or a display name - internally we want a PS name - that's what Cocoa is doing
	name = [O2Font postscriptNameForFontName:name];

   result=[self cachedFontWithName:name size:size];

    if(result==nil) {
        result=[[[NSFont alloc] initWithName:name size:size] autorelease];
    }

   return result;
}

+(NSFont *)fontWithName:(NSString *)name matrix:(const float *)matrix {
   return [self fontWithName:name size:matrix[0]];
}

+(NSFont *)fontWithDescriptor:(NSFontDescriptor *)descriptor size:(float)size {
	
	NSDictionary* attributes = [descriptor fontAttributes];
	NSString* fontName = [attributes objectForKey: NSFontNameAttribute];
	if (fontName) {
		return [NSFont fontWithName: fontName size: size];
	}

	NSString* fontFamily = [attributes objectForKey: NSFontFamilyAttribute];
	
	if (fontFamily) {
		NSFontManager* fontMgr = [NSFontManager sharedFontManager];
		
		NSArray* matchingFonts = [fontMgr availableMembersOfFontFamily: fontFamily];
		
		if ([matchingFonts count] == 1) {
			// won't find anything better than this
			NSArray* members = [matchingFonts objectAtIndex: 0];
			return [NSFont fontWithName: [members objectAtIndex: 0] size: size];
		} else {
			// Let's hope that we've got more to go on.
			NSString* fontFace = [attributes objectForKey: NSFontFaceAttribute];
			if (fontFace != nil) {
				int i = 0;
				for (i = 0; i < [matchingFonts count]; i++) {
					NSArray* members = [matchingFonts objectAtIndex: i];
					NSString* candidateFace = [members objectAtIndex: 1];
					if ([candidateFace isEqualToString: fontFace]) {
						return [NSFont fontWithName: [members objectAtIndex: 0] size: size];
					}
				}
			} else {
				// just take the first one
				NSArray* members = [matchingFonts objectAtIndex: 0];
				return [NSFont fontWithName: [members objectAtIndex: 0] size: size];
			}
			
		}
	}
	NSLog(@"unable to match font descriptor: %@", descriptor);
	return nil;
}

+(NSFont *)fontWithDescriptor:(NSFontDescriptor *)descriptor size:(float)size textTransform:(NSAffineTransform *)transform {
   NSUnimplementedMethod();
   return 0;
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

-(float)pointSize {
   return _pointSize;
}

-(NSString *)fontName {
   return _name;
}

-(const float *)matrix {
   return _matrix;
}

-(NSAffineTransform *)textTransform {
   NSAffineTransform      *result=[NSAffineTransform transform];
   NSAffineTransformStruct fields={
    _matrix[0],_matrix[1],_matrix[2],
    _matrix[3],_matrix[4],_matrix[5],
   };
   
   [result setTransformStruct:fields];

   return result;
}

-(NSFontRenderingMode)renderingMode {
   NSUnimplementedMethod();
   return 0;
}

-(NSCharacterSet *)coveredCharacterSet {
   return O2FontGetCoveredCharacterSet(_cgFont);
}

-(NSStringEncoding)mostCompatibleStringEncoding {
   return _encoding;
}

-(NSString *)familyName {
   NSString *familyName = [[NSFontFamily fontFamilyWithTypefaceName:_name]
name];
   if (familyName == nil)
   {
      NSString *blank = @" ";
      NSMutableArray *nameComponents = [NSMutableArray
arrayWithArray:[_name componentsSeparatedByString:blank]];
      while ([nameComponents count] > 1 && familyName == nil)
      {
         [nameComponents removeLastObject];
         familyName = [[NSFontFamily fontFamilyWithName:
[nameComponents componentsJoinedByString:blank]] name];
      }
   }

	// Fall back to using the font name - nil is not an option
	if (familyName == nil) {
		familyName = _name;
	}
   return familyName;
}

-(NSString *)displayName {
	NSFontTypeface *typeFace = [NSFontFamily fontTypefaceWithName:_name];
   return [typeFace displayName];
}

- (NSDictionary*)_fontTraitsAsDictionary
{
	NSFontManager* fm = [NSFontManager sharedFontManager];
	
	NSMutableDictionary* traitsDictionary = [NSMutableDictionary dictionaryWithCapacity: 4];
	NSFontFamily   *family=[NSFontFamily fontFamilyWithTypefaceName:[self fontName]];
	NSFontTypeface *typeface=[family typefaceWithName:[self fontName]];
	NSFontTraitMask symbolicTraits=[typeface traits];
	[traitsDictionary setObject: [NSNumber numberWithInt: symbolicTraits] forKey: NSFontSymbolicTrait];
	[traitsDictionary setObject: [NSNumber numberWithInt: [fm weightOfFont: self]] forKey: NSFontWeightTrait];
//	[traitsDictionary setObject: [NSNumber numberWithInt: ??] forKey: NSFontWidthTrait]; // not sure what's put here
	[traitsDictionary setObject: [NSNumber numberWithFloat: [self italicAngle]] forKey: NSFontSlantTrait];
	return traitsDictionary;
}

-(NSFontDescriptor *)fontDescriptor {
	
	NSFontFamily   *fontFamily=[NSFontFamily fontFamilyWithName: [self familyName]];
	NSFontTypeface *typeface=[fontFamily typefaceWithName:[self fontName]];

	NSDictionary* attributes = [NSDictionary dictionaryWithObjectsAndKeys:
								[self fontName],	NSFontNameAttribute,
								[self familyName],	NSFontFamilyAttribute,
								[[NSNumber numberWithFloat: [self pointSize]] stringValue], NSFontSizeAttribute,
//								[self matrix], NSFontMatrixAttribute, // currently returns nil
								[self coveredCharacterSet], NSFontCharacterSetAttribute, 
								[self _fontTraitsAsDictionary], NSFontTraitsAttribute,
								[typeface traitName], NSFontFaceAttribute,
								[NSNumber numberWithFloat: [self maximumAdvancement].width], NSFontFixedAdvanceAttribute,
								[self displayName], NSFontVisibleNameAttribute,
								nil];
								
	NSFontDescriptor* descriptor = [NSFontDescriptor fontDescriptorWithFontAttributes: attributes];
	return descriptor;
}

-(NSFont *)printerFont {
   NSUnimplementedMethod();
   return nil;
}

-(NSFont *)screenFont {
   return self;
}

-(NSFont *)screenFontWithRenderingMode:(NSFontRenderingMode)mode {
   NSUnimplementedMethod();
   return nil;
}

-(NSRect)boundingRectForFont {
   return CTFontGetBoundingBox(_ctFont);
}

-(NSRect)boundingRectForGlyph:(NSGlyph)glyph {
   NSUnimplementedMethod();
   return NSMakeRect(0,0,0,0);
}

-(NSMultibyteGlyphPacking)glyphPacking {
   return NSNativeShortGlyphPacking;
}

-(unsigned)numberOfGlyphs {
   return CTFontGetGlyphCount(_ctFont);
}

-(NSGlyph)glyphWithName:(NSString *)name {
   NSUnimplementedMethod();
   return 0;
}

-(BOOL)glyphIsEncoded:(NSGlyph)glyph {
   return (glyph<CTFontGetGlyphCount(_ctFont))?YES:NO;
}

-(NSSize)advancementForGlyph:(NSGlyph)glyph {
   CGSize  cgSize;
   CGGlyph cgGlyphs[1]={glyph};
   
   CTFontGetAdvancesForGlyphs(_ctFont,0,cgGlyphs,&cgSize,1);

   return NSMakeSize(cgSize.width,cgSize.height);
}

-(NSSize)maximumAdvancement {
   CGSize  max=CGSizeZero;
   int     glyph,glyphCount=CTFontGetGlyphCount(_ctFont);
   CGGlyph glyphs[glyphCount];
   CGSize  advances[glyphCount];
   
   for(glyph=0;glyph<glyphCount;glyph++)
    glyphs[glyph]=glyph;
    
   CTFontGetAdvancesForGlyphs(_ctFont,0,glyphs,advances,glyphCount);
   
   for(glyph=0;glyph<glyphCount;glyph++){
    max.width=MAX(max.width,advances[glyph].width);
    max.height=MAX(max.height,advances[glyph].height);
   }

   return max;
}

-(CGFloat)underlinePosition {
   return CTFontGetUnderlinePosition(_ctFont);
}

-(CGFloat)underlineThickness {
   return CTFontGetUnderlineThickness(_ctFont);
}

-(CGFloat)ascender {
   return CTFontGetAscent(_ctFont);
}

// CT & NS descender value have opposite value on Cocoa
-(CGFloat)descender {
   return -CTFontGetDescent(_ctFont);
}

-(CGFloat)leading {
   return CTFontGetLeading(_ctFont);
}

-(CGFloat)defaultLineHeightForFont {
   return roundf(CTFontGetAscent(_ctFont) + CTFontGetDescent(_ctFont) + CTFontGetLeading(_ctFont));
}

-(BOOL)isFixedPitch {
   CGSize  current;
   int     glyph,glyphCount=CTFontGetGlyphCount(_ctFont);
   CGGlyph glyphs[glyphCount];
   CGSize  advances[glyphCount];
   
   for(glyph=0;glyph<glyphCount;glyph++)
    glyphs[glyph]=glyph;
   
   CTFontGetAdvancesForGlyphs(_ctFont,0,glyphs,advances,glyphCount);
   current=advances[0];
   
   for(glyph=1;glyph<glyphCount;glyph++){
    if(advances[glyph].width!=current.width || advances[glyph].height!=current.height)
     return NO;
   }

   return YES;
}

-(float)italicAngle {
   return CTFontGetSlantAngle(_ctFont);
}

-(float)xHeight {
   return CTFontGetXHeight(_ctFont);
}

-(float)capHeight {
   return CTFontGetCapHeight(_ctFont);
}

-(void)setInContext:(NSGraphicsContext *)context {
   CGContextRef cgContext=[context graphicsPort];
   
   CGContextSetFont(cgContext,_cgFont);
   CGContextSetFontSize(cgContext,_pointSize);

   CGAffineTransform textMatrix;
   
// FIX, should check the focusView in the context instead of NSView's
   if([[NSGraphicsContext currentContext] isFlipped])
    textMatrix=(CGAffineTransform){1,0,0,-1,0,0};
   else
    textMatrix=CGAffineTransformIdentity;

   CGContextSetTextMatrix(cgContext,textMatrix);
}

-(void)set {
   [self setInContext:[NSGraphicsContext currentContext]];
}

-(NSPoint)positionOfGlyph:(NSGlyph)current precededByGlyph:(NSGlyph)previous isNominal:(BOOL *)isNominalp {
   return [_ctFont positionOfGlyph:current precededByGlyph:previous isNominal:isNominalp];
}

-(void)getAdvancements:(NSSize *)advancements forGlyphs:(const NSGlyph *)glyphs count:(unsigned)count {
   CGGlyph cgGlyphs[count];
   int     i;
   
   for(i=0;i<count;i++)
    cgGlyphs[i]=glyphs[i];
   
   CTFontGetAdvancesForGlyphs(_ctFont,0,cgGlyphs,advancements,count);
}

-(void)getAdvancements:(NSSize *)advancements forPackedGlyphs:(const void *)packed length:(unsigned)length {
   CTFontGetAdvancesForGlyphs(_ctFont,0,packed,advancements,length);
}

-(void)getBoundingRects:(NSRect *)rects forGlyphs:(const NSGlyph *)glyphs count:(unsigned)count {
   NSUnimplementedMethod();
}

-(unsigned)getGlyphs:(NSGlyph *)glyphs forCharacters:(unichar *)characters length:(unsigned)length {
   CGGlyph  cgGlyphs[length];
   int      i;
   
   CTFontGetGlyphsForCharacters(_ctFont,characters,cgGlyphs,length);
   
   for(i=0;i<length;i++){
    unichar check=characters[i];

    if(check<' ' || (check>=0x7F && check<=0x9F) || check==0x200B || check==0x2028 || check==0x2029)
     glyphs[i]=NSControlGlyph;
    else
     glyphs[i]=cgGlyphs[i];
   }

   return length;
}

-(NSString *)description {
   return [NSString stringWithFormat:@"<%@ %@ %f>",isa,_name,_pointSize];
}

int NSConvertGlyphsToPackedGlyphs(NSGlyph *glyphs,int length,NSMultibyteGlyphPacking packing,char *outputX) {
   int      i,result=0;
   CGGlyph *output=(CGGlyph *)outputX;

   for(i=0;i<length;i++){
    NSGlyph check=glyphs[i];

    if(check!=NSNullGlyph && check!=NSControlGlyph)
     output[result++]=check;
   }

   return result*2;
}

@end

@implementation NSFont (PortatibilityAdditions)

+ (void)setNibFontTranslator:(NSNibFontNameTranslator*)fontTranslator
{
	[fontTranslator retain];
	[_nibFontTranslator release];
	_nibFontTranslator = fontTranslator;
}

+ (NSNibFontNameTranslator*)nibFontTranslator
{
	return _nibFontTranslator;
}

@end
