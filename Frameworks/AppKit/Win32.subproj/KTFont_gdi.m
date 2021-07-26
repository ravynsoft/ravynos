#import "KTFont_gdi.h"
#import <Onyx2D/O2Font.h>
#import <Onyx2D/O2Context_gdi.h>
#import "Win32Display.h"
#import <Onyx2D/Win32Font.h>
#import <Onyx2D/O2Font_gdi.h>
#import <Onyx2D/O2MutablePath.h>
#import <AppKit/NSRaise.h>

// Define that if we want to preserve the system pref for font sizes
// That's usually a bad idea if the UI can't automatically adapt to a random control sizes
//#define USE_WIN_PREFS_FONTSIZE

#define MAXUNICHAR 0xFFFF

@interface KTFont(KTFont_gdi)
@end

@implementation KTFont(KTFont_gdi)

+allocWithZone:(NSZone *)zone {
   return NSAllocateObject([KTFont_gdi class],0,NULL);
}

@end

@implementation KTFont_gdi

static inline CGGlyph glyphForCharacter(KTFont_gdi *self,unichar character){
   CGGlyphRangeTable *table=self->_glyphRangeTable;
   unsigned           range=character>>8;
   unsigned           index=character&0xFF;

   if(table->ranges[range]!=NULL)
    return table->ranges[range]->glyphs[index];

   return CGNullGlyph;
}

static inline unichar characterForGlyph(KTFont_gdi *self,CGGlyph glyph){
   if(glyph<self->_glyphRangeTable->numberOfGlyphs)
    return self->_glyphRangeTable->characters[glyph];
   
   return 0;
}

static inline CGGlyphMetrics *glyphInfoForGlyph(KTFont_gdi *self,CGGlyph glyph){
   if(glyph<self->_glyphInfoSet->numberOfGlyphs)
    return self->_glyphInfoSet->info+glyph;

   return NULL;
}

-(float)pointSize {
    return [_font nativeSizeForSize: _size];
}

-(Win32Font *)createGDIFontSelectedInDC:(HDC)dc {
   int        height=([self pointSize]*FONT_DPI(_dc))/72.;
   Win32Font *result=[[Win32Font alloc] initWithName:(NSString *)_name height:-height antialias:NO];
   
   SelectObject(dc,[result fontHandle]);
   return result;
}

-(Win32Font *)selectFontInDefaultDC
{
	if (_dc == NULL) {
		_dc = GetDC(NULL);
	}
	if (_winFont == nil) {
		_winFont = [(O2Font_gdi *)_font createGDIFontSelectedInDC:_dc pointSize:_size];
	}
	SelectObject(_dc,[_winFont fontHandle]);
	return _winFont;
}

-(BOOL)fetchSharedGlyphRangeTable {
   static NSMapTable    *nameToGlyphRanges=NULL;
   CGGlyphRangeTable    *shared;
   BOOL                  result=YES;
   
   if(nameToGlyphRanges==NULL)
    nameToGlyphRanges=NSCreateMapTable(NSObjectMapKeyCallBacks,NSNonOwnedPointerMapValueCallBacks,0);

   shared=NSMapGet(nameToGlyphRanges,_name);

   if(shared==NULL){
    result=NO;
    shared=NSZoneCalloc(NULL,sizeof(CGGlyphRangeTable),1);
    NSMapInsert(nameToGlyphRanges,_name,shared);
   }

   _glyphRangeTable=shared;
   
   return result;
}

-(void)loadGlyphRangeTable {
   if([self fetchSharedGlyphRangeTable])
    return;

   Win32Font         *gdiFont=[self selectFontInDefaultDC];
   NSRange            range=NSMakeRange(0,MAXUNICHAR);
   unichar            characters[range.length];
   unsigned short     glyphs[range.length];
   unsigned           i;
   HANDLE             library=LoadLibrary("GDI32");
   FARPROC            getGlyphIndices=GetProcAddress(library,"GetGlyphIndicesW");

   for(i=0;i<range.length;i++)
    characters[i]=range.location+i;

// GetGlyphIndicesW is around twice as fast as GetCharacterPlacementW, but only available on Win2k/XP
    if(getGlyphIndices!=NULL) {
    getGlyphIndices(_dc,characters,range.length,glyphs,0);
    }  else {
    GCP_RESULTSW results;

    results.lStructSize=sizeof(GCP_RESULTS);
    results.lpOutString=NULL;
    results.lpOrder=NULL;
    results.lpDx=NULL;
    results.lpCaretPos=NULL;
    results.lpClass=NULL;
    results.lpGlyphs=glyphs;
    results.nGlyphs=range.length;
    results.nMaxFit=0;

    if(GetCharacterPlacementW(_dc,characters,range.length,0,&results,0)==0)
     NSLog(@"GetCharacterPlacementW failed");
   }
   
   _glyphRangeTable->numberOfGlyphs=0;
   for(i=0;i<range.length;i++){
    unsigned short glyph=glyphs[i];
    unsigned range=i>>8;
    unsigned index=i&0xFF;

    if(glyph!=0){
     if(glyph>_glyphRangeTable->numberOfGlyphs)
      _glyphRangeTable->numberOfGlyphs=glyph;

     if(_glyphRangeTable->ranges[range]==NULL)
      _glyphRangeTable->ranges[range]=NSZoneCalloc(NULL,sizeof(CGGlyphRange),1);

     _glyphRangeTable->ranges[range]->glyphs[index]=glyph;
    }
   }
   _glyphRangeTable->numberOfGlyphs++;
   _glyphRangeTable->characters=NSZoneCalloc(NULL,sizeof(unichar),_glyphRangeTable->numberOfGlyphs);
   for(i=0;i<range.length;i++){
    unsigned short glyph=glyphs[i];
    
    _glyphRangeTable->characters[glyph]=range.location+i;
   }

}

-(void)fetchGlyphRanges {
   [self loadGlyphRangeTable];
   _glyphInfoSet->numberOfGlyphs=_glyphRangeTable->numberOfGlyphs;
}

-(void)fetchGlyphKerning {
   Win32Font  *gdiFont=[self selectFontInDefaultDC];
    
    // We need to use the font at Em size to get precise (or just correct for font) metrics
    LOGFONT logFont;
    GetObject([gdiFont fontHandle],sizeof(logFont),&logFont);
    logFont.lfHeight=-_unitsPerEm;
    logFont.lfWidth=0;
    
    HFONT fontHandle=CreateFontIndirect(&logFont);
    SelectObject(_dc,fontHandle);

   int         i,numberOfPairs=GetKerningPairs(_dc,0,NULL);
   KERNINGPAIR pairs[numberOfPairs];

   GetKerningPairsW(_dc,numberOfPairs,pairs);
   
    float  scale = [self pointSize]/_unitsPerEm;

    for(i=0;i<numberOfPairs;i++){
    if(pairs[i].iKernAmount==0)
     continue;

       unichar previousCharacter=pairs[i].wFirst;
       unichar currentCharacter=pairs[i].wSecond;
       float   xoffset=pairs[i].iKernAmount;
       NSGlyph previous=glyphForCharacter(self,previousCharacter);
       NSGlyph current=glyphForCharacter(self,currentCharacter);
       
    if(current==NSNullGlyph)
     ;//NSLog(@"unable to generate kern pair 0x%04X 0x%04X %f",previousCharacter,currentCharacter,xoffset);
    else {
     CGGlyphMetrics *info=glyphInfoForGlyph(self,current);

     if(info==NULL)
      NSLog(@"no info for glyph %d",current);
     else {
      unsigned index=info->numberOfKerningOffsets;

      if(index==0)
       info->kerningOffsets=NSZoneMalloc([self zone],sizeof(CGKerningOffset));
      else
       info->kerningOffsets=NSZoneRealloc([self zone],info->kerningOffsets,
               sizeof(CGKerningOffset)*(index+1));

      info->kerningOffsets[index].previous=previous;
      info->kerningOffsets[index].xoffset=xoffset*scale;
         if (_useMacMetrics == NO) {
             info->kerningOffsets[index].xoffset = roundf(info->kerningOffsets[index].xoffset);
         }
      info->numberOfKerningOffsets++;
     }
    }
   }
    DeleteObject(fontHandle);
}

-(void)fetchGlyphInfo {
   _glyphInfoSet->info=NSZoneCalloc([self zone],sizeof(CGGlyphMetrics),_glyphInfoSet->numberOfGlyphs);
   [self fetchGlyphKerning];
   
}

static inline void fetchAllGlyphRangesIfNeeded(KTFont_gdi *self){
   if(self->_glyphRangeTable==NULL)
    [self fetchGlyphRanges];
}

static inline CGGlyphMetrics *fetchGlyphInfoIfNeeded(KTFont_gdi *self,CGGlyph glyph){
   fetchAllGlyphRangesIfNeeded(self);
   if(self->_glyphInfoSet->info==NULL)
    [self fetchGlyphInfo];

   return glyphInfoForGlyph(self,glyph);
}

-(void)fetchAdvancementsForGlyph:(CGGlyph)glyph {
    Win32Font *gdiFont=[self selectFontInDefaultDC];
    
    // We need to use the font at Em size to get precise (or just correct for font) metrics
    LOGFONT logFont;
    GetObject([gdiFont fontHandle],sizeof(logFont),&logFont);
    logFont.lfHeight=-_unitsPerEm;
    logFont.lfWidth=0;
    
    HFONT fontHandle=CreateFontIndirect(&logFont);
    SelectObject(_dc,fontHandle);

   ABCFLOAT *abc;
   int       i,max;

   for(max=0;max<MAXUNICHAR;max++){
    NSGlyph check=glyphForCharacter(self,max);

    if(check==glyph)
     break;
   }

   if(max==MAXUNICHAR){
    CGGlyphMetrics *info=glyphInfoForGlyph(self,glyph);

    info->hasAdvancement=YES;
    info->advanceA=0;
    info->advanceB=0;
    info->advanceC=0;
       
       DeleteObject(fontHandle);
    return;
   }

   max=((max/128)+1)*128;
   abc=__builtin_alloca(sizeof(ABCFLOAT)*max);
   if(!GetCharABCWidthsFloatW(_dc,0,max-1,abc))
    NSLog(@"GetCharABCWidthsFloat failed");
   else {
       float  scale = _size/_unitsPerEm;
       scale *= (FONT_DPI(_dc)/96.);

       for(i=0;i<max;i++){
     NSGlyph      glyph=glyphForCharacter(self,i);
     CGGlyphMetrics *info=glyphInfoForGlyph(self,glyph);

     if(info==NULL)
      NSLog(@"no info for glyph %d",glyph);
     else {
         info->hasAdvancement=YES;
         info->advanceA=abc[i].abcfA*scale;
         info->advanceB=abc[i].abcfB*scale;
         info->advanceC=abc[i].abcfC*scale;
     }
    }
   }
    DeleteObject(fontHandle);
}

static inline CGGlyphMetrics *fetchGlyphAdvancementIfNeeded(KTFont_gdi *self,CGGlyph glyph){
   CGGlyphMetrics *info=fetchGlyphInfoIfNeeded(self,glyph);

   if(info==NULL)
    return info;

   if(!info->hasAdvancement){
    [self fetchAdvancementsForGlyph:glyph];
   }

   return info;
}

-(void)fetchMetrics {
   Win32Font    *gdiFont=[self selectFontInDefaultDC];
   TEXTMETRIC    gdiMetrics;

   GetTextMetrics(_dc,&gdiMetrics);

   _metrics.emsquare=1;
   _metrics.scale=1;
   _metrics.boundingRect.origin.x=0;
   _metrics.boundingRect.origin.y=0;
   _metrics.boundingRect.size.width=gdiMetrics.tmMaxCharWidth;
   _metrics.boundingRect.size.height=gdiMetrics.tmHeight;
   _metrics.ascender=gdiMetrics.tmAscent;
   _metrics.descender=-gdiMetrics.tmDescent;
   _metrics.italicAngle=0;
   _metrics.capHeight=0;
   _metrics.leading=0;
   _metrics.xHeight=0;
   _metrics.underlineThickness=_size/24.0;
   if(_metrics.underlineThickness<0)
    _metrics.underlineThickness=1;

   _metrics.underlinePosition=-(_metrics.underlineThickness*2);

   if(!(gdiMetrics.tmPitchAndFamily&TMPF_TRUETYPE)){
    return;
   }
    
   int size=GetOutlineTextMetricsA(_dc,0,NULL);
   
	_useMacMetrics=NO;
   
   if(size<=0){
    return;
   }

   OUTLINETEXTMETRICA *ttMetrics=__builtin_alloca(size);

   ttMetrics->otmSize=sizeof(OUTLINETEXTMETRICA);
   if(!GetOutlineTextMetricsA(_dc,size,ttMetrics)){
    return;
   }
       
/* P. 931 "Windows Graphics Programming" by Feng Yuan, 1st Ed.
   A font with height of negative otmEMSquare will have precise metrics  */
   
   LOGFONT logFont;
   
   GetObject([gdiFont fontHandle],sizeof(logFont),&logFont);
   logFont.lfHeight=-ttMetrics->otmEMSquare;
   logFont.lfWidth=0;
     
   HFONT fontHandle=CreateFontIndirect(&logFont);
   SelectObject(_dc,fontHandle);
   
   ttMetrics->otmSize=sizeof(OUTLINETEXTMETRICA);
   size=GetOutlineTextMetricsA(_dc,size,ttMetrics);
   DeleteObject(fontHandle);
   
   if(size<=0){
    return;
   }

	// Don't use the magic pointSize scaling formula on these font (UI fonts)
   if(![(NSString *)_name isEqualToString:@"Marlett"] && ![(NSString *)_name isEqualToString:@"Segoe UI"] && ![(NSString *)_name isEqualToString:@"Tahoma"])
    _useMacMetrics=YES;

    _metrics.emsquare=ttMetrics->otmEMSquare;
    _metrics.scale=_size*(FONT_DPI(_dc)/96.);

   _metrics.boundingRect.origin.x=ttMetrics->otmrcFontBox.left;
   _metrics.boundingRect.origin.y=ttMetrics->otmrcFontBox.bottom;
   _metrics.boundingRect.size.width=ttMetrics->otmrcFontBox.right-ttMetrics->otmrcFontBox.left;
   _metrics.boundingRect.size.height=ttMetrics->otmrcFontBox.top-ttMetrics->otmrcFontBox.bottom;

    if (_useMacMetrics) {
        _metrics.ascender=ttMetrics->otmMacAscent;
        _metrics.descender=ttMetrics->otmMacDescent;
        _metrics.leading=ttMetrics->otmMacLineGap;
    } else {
        _metrics.ascender=ttMetrics->otmAscent;
        _metrics.descender=ttMetrics->otmDescent;
        _metrics.leading=ttMetrics->otmLineGap;
        _metrics.scale *= (96./72.);
    }
   _metrics.italicAngle=ttMetrics->otmItalicAngle;
   _metrics.capHeight=ttMetrics->otmsCapEmHeight;
   _metrics.xHeight=ttMetrics->otmsXHeight;

   _metrics.underlineThickness=ttMetrics->otmsUnderscoreSize;
   if(_metrics.underlineThickness<0)
    _metrics.underlineThickness=1;
   _metrics.underlinePosition=ttMetrics->otmsUnderscorePosition;
}

-initWithFont:(O2Font *)font size:(CGFloat)size {
   [super initWithFont:font size:size];
   _name=O2FontCopyFullName(font);
   [self fetchMetrics];
   _glyphRangeTable=NULL;
   _glyphInfoSet=NSZoneMalloc([self zone],sizeof(CGGlyphMetricsSet));
   _glyphInfoSet->numberOfGlyphs=0;
   _glyphInfoSet->info=NULL;
   return self;
}

-initWithUIFontType:(CTFontUIFontType)uiFontType size:(CGFloat)size language:(NSString *)language {
   O2Font *font=nil;
   
    // Try to ask the system which font we should use for menus
    NONCLIENTMETRICSW nm;
    nm.cbSize = sizeof (NONCLIENTMETRICSW);
    BOOL systemInfoIsValid = NO;
    if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS,0,&nm,0)) {
        systemInfoIsValid = YES;
    }

    NSString *name =nil;
    switch(uiFontType){
        case kCTFontMenuItemMarkFontType:
        case kCTFontMenuItemCmdKeyFontType:
        case kCTFontMenuTitleFontType:
        case kCTFontMenuItemFontType: {
            name = @"Tahoma";
            if (systemInfoIsValid) {
                // Try to ask the system which font we should use for menus
                LOGFONTW fl = nm.lfMenuFont;
                name = [NSString stringWithFormat:@"%S", fl.lfFaceName];
                if (size == 0) {
                    size = ABS(fl.lfHeight);
                }
                if (size == 0) {
                    size = 10.;
                }
            }
        }
            break;
            
        case kCTFontControlContentFontType:
        case kCTFontToolbarFontType:
        case kCTFontLabelFontType: {
            name = @"Tahoma";
            if (systemInfoIsValid) {
                // Try to ask the system which font we should use for menus
                LOGFONTW fl = nm.lfCaptionFont;
                name = [NSString stringWithFormat:@"%S", fl.lfFaceName];
#if USE_WIN_PREFS_FONTSIZE
                if (size == 0) {
                    size = ABS(fl.lfHeight);
                }
#endif
                if (size == 0) {
                    size = 10.;
                }
            }
        }
            break;
            
        case kCTFontSystemFontType: {
            name = @"Tahoma";
            if (systemInfoIsValid) {
                // Try to ask the system which font we should use for menus
                LOGFONTW fl = nm.lfMessageFont;
                name = [NSString stringWithFormat:@"%S", fl.lfFaceName];
#if USE_WIN_PREFS_FONTSIZE
                if (size == 0) {
                    size = ABS(fl.lfHeight);
                }
#endif
                if (size == 0) {
                    size = 12.;
                }
           }
        }
            break;

        case kCTFontToolTipFontType: {
            name = @"Tahoma";
            if (systemInfoIsValid) {
                // Try to ask the system which font we should use for menus
                LOGFONTW fl = nm.lfStatusFont;
                name = [NSString stringWithFormat:@"%S", fl.lfFaceName];
#if USE_WIN_PREFS_FONTSIZE
                if (size == 0) {
                    size = ABS(fl.lfHeight);
                }
#endif
                if (size == 0) {
                    size = 10.;
                }
            }
        }
            break;

        default:
            NSLog(@"Unimplemented %d type", uiFontType);
            return nil;
    }

    KTFont *result = nil;
    if (name && size > 0.) {
        font=O2FontCreateWithFontName(name);
        if (font == nil) {
            font=O2FontCreateWithFontName(@"Tahoma");
        }
        
        result=[self initWithFont:font size:size];
        [font release];
    }
   return result;
}

-(void)dealloc {
	[_winFont release];
	ReleaseDC(NULL,_dc);
  _glyphRangeTable=NULL;

   if(_glyphInfoSet!=NULL){
    if(_glyphInfoSet->info!=NULL){
     unsigned i;

     for(i=0;i<_glyphInfoSet->numberOfGlyphs;i++){
      if(_glyphInfoSet->info[i].kerningOffsets!=NULL)
       NSZoneFree([self zone],_glyphInfoSet->info[i].kerningOffsets);
     }
     NSZoneFree([self zone],_glyphInfoSet->info);
    }

    NSZoneFree([self zone],_glyphInfoSet);
   }

   [super dealloc];
}

-(CGRect)boundingRect {
   CGRect result=_metrics.boundingRect;
   
   result.origin.x/=_metrics.emsquare*_metrics.scale;
   result.origin.y/=_metrics.emsquare*_metrics.scale;
   result.size.width/=_metrics.emsquare*_metrics.scale;
   result.size.height/=_metrics.emsquare*_metrics.scale;
   
   return result;
}

#if 1
-(float)ascender {
   return _metrics.ascender/_metrics.emsquare*_metrics.scale;
}

-(float)descender {
   return -_metrics.descender/_metrics.emsquare*_metrics.scale;
}

-(float)leading {
   return _metrics.leading/_metrics.emsquare*_metrics.scale;
}
#endif

-(float)underlineThickness {
   return _metrics.underlineThickness/_metrics.emsquare*_metrics.scale;
}

-(float)underlinePosition {
   return _metrics.underlinePosition/_metrics.emsquare*_metrics.scale;
}

-(unsigned)numberOfGlyphs {
   return _glyphInfoSet->numberOfGlyphs;
}

-(CGPoint)positionOfGlyph:(CGGlyph)current precededByGlyph:(CGGlyph)previous isNominal:(BOOL *)isNominalp {
   CGGlyphMetrics *previousInfo;
   CGGlyphMetrics *currentInfo;
   CGPoint      result=CGPointMake(0,0);

   previousInfo=fetchGlyphAdvancementIfNeeded(self,previous);
   currentInfo=fetchGlyphAdvancementIfNeeded(self,current);
   *isNominalp=YES;

   if(previous==CGNullGlyph){
    if(currentInfo!=NULL){
    }
   }
   else {
       if(previousInfo!=NULL) {
           float delta = previousInfo->advanceA+previousInfo->advanceB+previousInfo->advanceC;
           if (_useMacMetrics == NO) {
               delta = roundf(delta);
           }
          result.x+=delta;
       }
    
    if(current==CGNullGlyph){
    }
    else if(currentInfo!=NULL){
     int i;

     for(i=0;i<currentInfo->numberOfKerningOffsets;i++){
      if(currentInfo->kerningOffsets[i].previous==previous){
       result.x+=currentInfo->kerningOffsets[i].xoffset;
       *isNominalp=NO;
       break;
      }
     }
    }
   }
   return result;
}

-(void)getGlyphs:(CGGlyph *)glyphs forCharacters:(const unichar *)characters length:(unsigned)length {
   int i;

   fetchAllGlyphRangesIfNeeded(self);
   for(i=0;i<length;i++)
    glyphs[i]=glyphForCharacter(self,characters[i]);
}

-(void)getCharacters:(unichar *)characters forGlyphs:(const CGGlyph *)glyphs length:(unsigned)length {
   int i;

   fetchAllGlyphRangesIfNeeded(self);
   for(i=0;i<length;i++)
    characters[i]=characterForGlyph(self,glyphs[i]);
}

-(void)getAdvancements:(CGSize *)advancements forGlyphs:(const CGGlyph *)glyphs count:(unsigned)count {
   int i;
   
   for(i=0;i<count;i++){
    CGGlyphMetrics *info=fetchGlyphAdvancementIfNeeded(self,glyphs[i]);

    if(info==NULL){
     NSLog(@"no info for glyph %d",glyphs[i]);
     advancements[i].width=0;
     advancements[i].height=0;
    }
    else {
        float delta = info->advanceA+info->advanceB+info->advanceC;
        if (_useMacMetrics == NO) {
            delta = roundf(delta);
        }
        advancements[i].width=delta;
        advancements[i].height=0;
    }
   }
}

-(CGSize)advancementForNominalGlyphs:(const CGGlyph *)glyphs count:(unsigned)count {
   CGSize result=CGSizeMake(0,0);
   int i;
   
   for(i=0;i<count;i++){
    CGGlyphMetrics *info=fetchGlyphAdvancementIfNeeded(self,glyphs[i]);

    if(info==NULL)
     NSLog(@"no info for glyph %d",glyphs[i]);
    else {
        float delta = info->advanceA+info->advanceB+info->advanceC;
        if (_useMacMetrics == NO) {
            delta = roundf(delta);
        }
        result.width+=delta;
        result.height+=0;
    }
   }
   
   return result;
}

/****************************************************************************
 *  FUNCTION   : fxDiv2
 *  RETURNS    : (val1 + val2)/2 for FIXED values
 ****************************************************************************/ 
static FIXED fxDiv2(FIXED fxVal1, FIXED fxVal2)
{
	// Note: the "volatile" is there to prevent some wrong result because of some compiler "optimisations"
    int32_t l = (*((int32_t volatile *)&(fxVal1)) + *((int32_t volatile *)&(fxVal2)))/2;
    return(*(FIXED volatile *)&l);
}

static FIXED FloatToFIXED(const float d)
{
	FIXED f;
	int32_t v = d * 65536.;
	
	f.value = v >> 16;
	f.fract = v & 0xFFFF;
	
	return f;
}

static float FIXEDToFloat(FIXED f)
{
	return (f.value<<16 | f.fract )  / 65536.0;
}

/****************************************************************************
 *  FUNCTION   : MakeBezierFromQBSpline
 *
 *  PURPOSE    : Converts a quadratic spline in pSline to a four point Bezier
 *               spline in pPts.
 *
 *
 *  RETURNS    : number of Bezier points placed into the pPts POINT array.
 ****************************************************************************/ 
static UINT MakeBezierFromQBSpline( CGPoint *pPts, POINTFX *pSpline )
{
    CGPoint   P0,         // Quadratic on curve start point
	P1,         // Quadratic control point
	P2;         // Quadratic on curve end point
    UINT    cTotal = 0;
	
    // Convert the Quadratic points to integer
    P0.x = FIXEDToFloat( pSpline[0].x );
    P0.y = FIXEDToFloat( pSpline[0].y );
    P1.x = FIXEDToFloat( pSpline[1].x );
    P1.y = FIXEDToFloat( pSpline[1].y );
    P2.x = FIXEDToFloat( pSpline[2].x );
    P2.y = FIXEDToFloat( pSpline[2].y );
	
    // conversion of a quadratic to a cubic
	
    // Cubic P0 is the on curve start point
    pPts[cTotal] = P0;
    cTotal++;
    
    // Cubic P1 in terms of Quadratic P0 and P1
    pPts[cTotal].x = P0.x + 2*(P1.x - P0.x)/3;
    pPts[cTotal].y = P0.y + 2*(P1.y - P0.y)/3;
    cTotal++;
	
    // Cubic P2 in terms of Qudartic P1 and P2
    pPts[cTotal].x = P1.x + 1*(P2.x - P1.x)/3;
    pPts[cTotal].y = P1.y + 1*(P2.y - P1.y)/3;
    cTotal++;
	
    // Cubic P3 is the on curve end point
    pPts[cTotal] = P2;
    cTotal++;
	
    return cTotal;
}

/****************************************************************************
 *  FUNCTION   : AppendQuadBSplineToBezier
 *
 *  PURPOSE    : Converts Quadratic spline segments into their Bezier point 
 *               representation and appends them to a list of Bezier points. 
 *
 *               WARNING - The array must have at least one valid
 *               start point prior to the address of the element passed.
 *
 *  RETURNS    : number of Bezier points added to the POINT array.
 ****************************************************************************/ 
static UINT AppendQuadBSplineToBezier( POINTFX start, LPTTPOLYCURVE lpCurve, O2MutablePathRef path)
{
    WORD                i;
    UINT                cTotal = 0;
    POINTFX             spline[3];  // a Quadratic is defined by 3 points
    CGPoint               bezier[4];  // a Cubic by 4
	
    // The initial A point is on the curve.
    spline[0] = start;
	
    for (i = 0; i < lpCurve->cpfx;)
    {
        // The B point.
        spline[1] = lpCurve->apfx[i++];
		
        // Calculate the C point.
        if (i == (lpCurve->cpfx - 1))
        {
            // The last C point is described explicitly
            // i.e. it is on the curve.
            spline[2] = lpCurve->apfx[i++];
        }     
        else
        {
            // C is midpoint between B and next B point
            // because that is the on curve point of 
            // a Quadratic B-Spline.
            spline[2].x = fxDiv2(
								 lpCurve->apfx[i-1].x,
								 lpCurve->apfx[i].x
								 );
            spline[2].y = fxDiv2(
								 lpCurve->apfx[i-1].y,
								 lpCurve->apfx[i].y
								 );
        }
		
        // convert the Q Spline to a Bezier
        MakeBezierFromQBSpline( bezier, spline );
        
        // append the Bezier to the existing ones
		// Point 0 is Point 3 of previous.
		O2PathAddCurveToPoint(path, NULL, bezier[1].x, bezier[1].y, bezier[2].x, bezier[2].y, bezier[3].x, bezier[3].y);
		
        // New A point for next slice of spline is the 
        // on curve C point of this B-Spline
        spline[0] = spline[2];
    }
	
    return cTotal;
}

/****************************************************************************
 *  FUNCTION   : AppendPolyLineToBezier
 *
 *  PURPOSE    : Converts line segments into their Bezier point 
 *               representation and appends them to a list of Bezier points. 
 *
 *               WARNING - The array must have at least one valid
 *               start point prior to the address of the element passed.
 *
 *  RETURNS    : number of Bezier points added to the POINT array.
 ****************************************************************************/ 
static UINT AppendPolyLineToBezier( POINTFX start, LPTTPOLYCURVE lpCurve, O2MutablePathRef path )
{
    int     i;
    UINT    cTotal = 0;
    CGPoint   endpt;
    CGPoint   startpt;
	
    endpt.x = FIXEDToFloat(start.x);
    endpt.y = FIXEDToFloat(start.y);
	
    for (i = 0; i < lpCurve->cpfx; i++)
    {
        // define the line segment
        startpt = endpt;
        endpt.x = FIXEDToFloat(lpCurve->apfx[i].x);
        endpt.y = FIXEDToFloat(lpCurve->apfx[i].y);
		
		O2PathAddLineToPoint(path, NULL, endpt.x, endpt.y);
    }
	
    return cTotal;
}

// Code adapted from here: http://support.microsoft.com/kb/243285 - fixed-to-int conversions replaced with fixed-to-float
static void ConvertTTPolygonToPath(LPTTPOLYGONHEADER lpHeader, DWORD size, O2MutablePathRef path)
{
	WORD                i;
	LPTTPOLYGONHEADER   lpStart;    // the start of the buffer
	LPTTPOLYCURVE       lpCurve;    // the current curve of a contour
	CGPoint				pt;         // the bezier buffer
	POINTFX             ptStart;    // The starting point of a curve
	DWORD               dwMaxPts = size/sizeof(POINTFX); // max possible pts.

	lpStart = lpHeader;

	// Loop until we have processed the entire buffer of contours.
	// The buffer may contain one or more contours that begin with
	// a TTPOLYGONHEADER. We have them all when we the end of the buffer.
	while ((DWORD)lpHeader < (DWORD)(((LPSTR)lpStart) + size))
	{
		if (lpHeader->dwType == TT_POLYGON_TYPE)
			// Draw each coutour, currently this is the only valid
			// type of contour.
		{
			// Convert the starting point. It is an on curve point.
			// All other points are continuous from the "last" 
			// point of the contour. Thus the start point the next
			// bezier is always pt[cTotal-1] - the last point of the 
			// previous bezier. See PolyBezier.
			pt.x = FIXEDToFloat(lpHeader->pfxStart.x);
			pt.y = FIXEDToFloat(lpHeader->pfxStart.y);
			
			O2PathMoveToPoint(path, NULL, pt.x, pt.y);
			
			// Get to first curve of contour - 
			// it starts at the next byte beyond header
			lpCurve = (LPTTPOLYCURVE) (lpHeader + 1);
			
			// Walk this contour and process each curve( or line ) segment 
			// and add it to the Beziers
			while ((DWORD)lpCurve < (DWORD)(((LPSTR)lpHeader) + lpHeader->cb))
			{
				//**********************************************
				// Format assumption:
				//   The bytes immediately preceding a POLYCURVE
				//   structure contain a valid POINTFX.
				// 
				//   If this is first curve, this points to the 
				//      pfxStart of the POLYGONHEADER.
				//   Otherwise, this points to the last point of
				//      the previous POLYCURVE.
				// 
				//   In either case, this is representative of the
				//      previous curve's last point.
				//**********************************************
				
				ptStart = *(LPPOINTFX)((LPSTR)lpCurve - sizeof(POINTFX));
				if (lpCurve->wType == TT_PRIM_LINE)
				{
					// add the polyline segments
					AppendPolyLineToBezier(ptStart, lpCurve, path);
					i = lpCurve->cpfx;
				}
				else if (lpCurve->wType == TT_PRIM_QSPLINE)
				{
					// Decode each Quadratic B-Spline segment, convert to bezier,
					// and append to the Bezier segments
					AppendQuadBSplineToBezier(ptStart, lpCurve, path );
					i = lpCurve->cpfx;
				}
				else
					// Oops! A POLYCURVE format we don't understand.
					; // error, error, error
				
				// Move on to next curve in the contour.
				lpCurve = (LPTTPOLYCURVE)&(lpCurve->apfx[i]);
			}
			
			// All contours are implied closed by TrueType definition.
			// Depending on the specific font and glyph being used, these
			// may not always be needed.
			O2PathCloseSubpath(path);
		}
		else
			// Bad, bail, must have a bogus buffer.
			break; // error, error, error
		
		// Move on to next Contour.
		// Its header starts immediate after this contour
		lpHeader = (LPTTPOLYGONHEADER)(((LPSTR)lpHeader) + lpHeader->cb);
	}
}

-(O2Path *)createPathForGlyph:(CGGlyph)glyph transform:(CGAffineTransform *)xform {
   O2MutablePath *result=[[O2MutablePath alloc] init];
   Win32Font *gdiFont=[self selectFontInDefaultDC];
   int        size=GetOutlineTextMetricsA(_dc,0,NULL);
    
   if(size<=0){
    return result;
   }

   OUTLINETEXTMETRICA *ttMetrics=__builtin_alloca(size);

   ttMetrics->otmSize=sizeof(OUTLINETEXTMETRICA);
   if(!GetOutlineTextMetricsA(_dc,size,ttMetrics))
    return result;
    
/* P. 931 "Windows Graphics Programming" by Feng Yuan, 1st Ed.
   A font with height of negative otmEMSquare will have precise metrics  */
   
   LOGFONT logFont;
     
   GetObject([gdiFont fontHandle],sizeof(logFont),&logFont);
   logFont.lfHeight=-ttMetrics->otmEMSquare;
   logFont.lfWidth=0;
     
   HFONT fontHandle=CreateFontIndirect(&logFont);
   SelectObject(_dc,fontHandle);
   DeleteObject(fontHandle);

	// _metrics.scale seems to be the configured font size
	// ttMetrics->otmEMSquare defines the size of the glyph box
	float scale = _size / (float)ttMetrics->otmEMSquare;
//    scale *= (FONT_DPI(_dc)/96.);

	// Scale the glyph box down to fit our font size
	MAT2 mat2;
	ZeroMemory(&mat2,sizeof(MAT2));
	mat2.eM11 = FloatToFIXED(scale);
	mat2.eM12.value = 0;
	mat2.eM21.value = 0;
	mat2.eM22 = FloatToFIXED(scale);
	
	GLYPHMETRICS glyphMetrics;
	
	int          outlineSize=GetGlyphOutline(_dc, glyph, GGO_NATIVE | GGO_GLYPH_INDEX, &glyphMetrics, 0, NULL, &mat2);
	if (outlineSize == GDI_ERROR) {
		DWORD err = GetLastError();
		NSLog(@"GetGlyphOutline failed(%d) for glyph: %d", err, glyph);
	} else {
	
		void *outline=__builtin_alloca(outlineSize);

	   if(GetGlyphOutline(_dc, glyph, GGO_NATIVE | GGO_GLYPH_INDEX, &glyphMetrics, outlineSize, outline, &mat2) != GDI_ERROR){
		   ConvertTTPolygonToPath(outline, outlineSize, result);
	   }
	}
	
   return result;
}

- (NSString *)description
{
    return [NSString stringWithFormat: @"%@ name: %@", [super description], _name];
}
@end
