/*
   AFMFileFontInfo.m

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@bx.logicnet.ro>
   Date: February 1997
   
   This file is part of the GNUstep GUI X/DPS Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include <float.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/file.h>
#include <fcntl.h>
#include "config.h"

#include <Foundation/NSDictionary.h>
#include <Foundation/NSString.h>
#include <Foundation/NSFileManager.h>
#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSValue.h>
#include <Foundation/NSDate.h>

#include <AppKit/NSFont.h>
#include <DPS/PSres.h>

#include "xdps/NSDPSContext.h"
#include "AFMFileFontInfo.h"
#include "general.h"
#include "fonts.h"

/* This dictionary keeps global font info objects indexed by AFM file names.
   A global font info instance is the representation of AFM file; all the
   dimensions are in the 1x1 space not in the 1000x1000 space of the AFM file.
   Each font object maintains a copy of this instance with all the dimensions
   scaled to the font size. */
static NSMutableDictionary* globalFontInfoDictionary = nil;

static NSMutableDictionary* globalFileDictionary = nil;

#ifndef LIB_FOUNDATION_LIBRARY
/* Define callbacks for a map table that has cStrings as keys */

/* hpjw from Aho, Sethi & Ullman: Principles of compiler design. */
static unsigned __NSHashCString(void *table, const void *aString)
{
    register const char* p = (char*)aString;
    register unsigned hash = 0, hash2;
    register int i, n = strlen((char*)aString);

    for (i=0; i < n; i++) {
        hash <<= 4;
        hash += *p++;
        if ((hash2 = hash & 0xf0000000))
            hash ^= (hash2 >> 24) ^ hash2;
    }
    return hash;
}

static BOOL __NSCompareCString(void *table, 
    const void *anObject1, const void *anObject2)
{
    return strcmp((char*)anObject1, (char*)anObject2) == 0;
}

static void __NSRetainNothing(void *table, const void *anObject)
{
}

static void __NSReleaseNothing(void *table, void *anObject)
{
}

static NSString* __NSDescribePointers(void *table, const void *anObject)
{
    return [NSString stringWithCString:anObject];
}

static const NSMapTableKeyCallBacks NSNonOwnedCStringMapKeyCallBacks = {
    (unsigned(*)(NSMapTable*, const void*))__NSHashCString,
    (BOOL(*)(NSMapTable*, const void*, const void*))__NSCompareCString,
    (void (*)(NSMapTable*, const void* anObject))__NSRetainNothing,
    (void (*)(NSMapTable*, void* anObject))__NSReleaseNothing,
    (NSString *(*)(NSMapTable*, const void*))__NSDescribePointers,
    (const void *)NULL
}; 
#endif /* LIB_FOUNDATION_LIBRARY */

static int
afmEnumerator (char* resourceType, char* resourceName, char* resourceFile,
             char* fontsList)
{
  NSString* afmFileName = [NSString stringWithCString: resourceFile];
  [(NSMutableArray*)fontsList addObject: afmFileName];
  return 0;
}

@implementation AFMFileFontInfo

+ (void)initialize
{
  static BOOL initialized = NO;

  if (!initialized) {
    initialized = YES;
    globalFontInfoDictionary = [NSMutableDictionary new];
    globalFileDictionary = [NSMutableDictionary new];
  }
}

- (NSString *) _getFontResource
{
  NSString* fontFile;
  DPSContext ctxt;
  char font_file[FILENAME_MAX + 1];
  
  ctxt = [(NSDPSContext *)GSCurrentContext() xDPSContext];
  
  
  fontFile = nil;
  PSWGetFontFile(ctxt, [fontName cString], font_file);
  if (strncmp(font_file, "/Resource", 9) == 0)
    {
      char completePath[FILENAME_MAX + 1];
      int found;
      /* We're using GS which returns files differently */
      PSWGSFontFile(ctxt, [fontName cString], font_file);
      PSWCompleteFilename (font_file, &found, completePath);
      if (found) 
	{
	  fontFile = [NSString stringWithCString: completePath];
	  fontFile = [[fontFile stringByDeletingPathExtension]
		       stringByAppendingPathExtension:@"afm"];
	}
    }
  else if (font_file[0] != '/')
    {
      /* Can't handle this yet, try the Adobe way... */
      NSMutableArray *fontsList = [[NSMutableArray new] autorelease];
      NSDebugLLog(@"Fonts", @"Font file resource returned relative-path %s", 
		  font_file);
      EnumeratePSResourceFiles (NULL, NULL, PSResFontAFM, [fontName cString],
				afmEnumerator, (char*)fontsList);
      if (![fontsList count])
	NSLog (@"WARNING: Font file not found! Check if the PSRESOURCEPATH "
	       @"environment variable points to the right directory. If so then "
	       @"probably you did not build the PS resource database using the "
	       @"`makepsres' utility.");
      else
	fontFile = [fontsList objectAtIndex: 0];
    }
     
  if (fontFile)
    NSDebugLLog(@"Fonts", @"Found AFM file %@ for font %@", fontFile, fontName);
  return fontFile;
}

- _setFontInfo
{
  NSAutoreleasePool* pool;
  AFMFontInfo* cFontInfo;
  AFMGlobalFontInfo* gfi;
  int fontType;
  FILE* file;
  int i, code;
  NSString *sweight;

  afmFileName = [globalFileDictionary objectForKey: fontName];
  if (afmFileName == nil)
    {
      afmFileName = [self _getFontResource];
      if (afmFileName == nil)
	{
	  NSLog (@"No AFM file for font %@. Can't get font info", fontName);
	  return nil;
	}
      [globalFileDictionary setObject: afmFileName forKey: fontName];
    }
  RETAIN(afmFileName);

  if (![[NSFileManager defaultManager] isReadableFileAtPath: afmFileName])
    {
      NSLog (@"Cannot read AFM file %@ for font %@", afmFileName, fontName);
      return nil;
    }

  file = fopen ([afmFileName cString], "r");
  if (!file) 
    {
      NSLog (@"Cannot open AFM file %@ for font %@", afmFileName, fontName);
      return nil;
    }

  code = AFMParseFile (file, &cFontInfo, AFM_G | AFM_M | AFM_P);
  if (code != afm_ok) {
    switch (code) {
      case afm_parseError:
	NSLog (@"parse error in AFM file %@", afmFileName);
	break;
      case afm_earlyEOF:
	NSLog (@"unexpected EOF in AFM file %@", afmFileName);
	break;
      case afm_storageProblem:
	NSLog (@"memory allocation problem while parsing the AFM file %@",
		afmFileName);
	break;
    }
    fclose (file);
    return nil;
  }
  fclose (file);

  gfi = cFontInfo->gfi;

  pool = [NSAutoreleasePool new];

  /*
   * Set the font information in instance variables and in the AFM dictionary.
   */
  fontDictionary = [[NSMutableDictionary alloc] initWithCapacity: 25];

  /* The font may actually be a GS alias, so do not set the fontName
   * field. */
  //  fontName = [[NSString stringWithCString:gfi->fontName] retain];
  [fontDictionary setObject: fontName forKey: NSAFMFontName];

  familyName = [[NSString stringWithCString:gfi->familyName] retain];
  [fontDictionary setObject: familyName forKey: NSAFMFamilyName];

  italicAngle = gfi->italicAngle;
  [fontDictionary setObject: [NSNumber numberWithFloat:gfi->italicAngle]
	      forKey: NSAFMItalicAngle];

  sweight = [NSString stringWithCString:gfi->weight];
  [fontDictionary setObject: sweight forKey: NSAFMWeight];

  underlinePosition = ((float)gfi->underlinePosition) / 1000;
  [fontDictionary setObject: [NSNumber numberWithFloat:underlinePosition]
	      forKey: NSAFMUnderlinePosition];

  underlineThickness = ((float)gfi->underlineThickness) / 1000;
  [fontDictionary setObject: [NSNumber numberWithFloat:underlineThickness]
	      forKey: NSAFMUnderlineThickness];

  capHeight = ((float)gfi->capHeight) / 1000;
  [fontDictionary setObject: [NSNumber numberWithFloat:capHeight]
	      forKey: NSAFMCapHeight];

  xHeight = ((float)gfi->xHeight) / 1000;
  [fontDictionary setObject: [NSNumber numberWithFloat:xHeight]
	      forKey: NSAFMXHeight];

  descender = ((float)gfi->descender) / 1000;
  [fontDictionary setObject: [NSNumber numberWithFloat:descender]
	      forKey: NSAFMDescender];

  ascender = ((float)gfi->ascender) / 1000;
  [fontDictionary setObject: [NSNumber numberWithFloat:ascender]
	      forKey: NSAFMAscender];

  encodingScheme = [[NSString stringWithCString:gfi->encodingScheme]
					retain];
  [fontDictionary setObject: encodingScheme forKey: NSAFMEncodingScheme];

  [fontDictionary setObject: [NSString stringWithCString:gfi->afmVersion]
	      forKey: NSAFMFormatVersion];
  [fontDictionary setObject:[NSString stringWithCString:gfi->notice]
	      forKey: NSAFMNotice];
  [fontDictionary setObject:[NSString stringWithCString:gfi->version]
	      forKey: NSAFMVersion];

  /* Setup bbox as expected by NSFont */
  fontBBox.origin.x = ((float)gfi->fontBBox.llx) / 1000;
  fontBBox.origin.y = ((float)gfi->fontBBox.lly) / 1000;
  fontBBox.size.width
      = ((float)(gfi->fontBBox.urx - gfi->fontBBox.llx)) / 1000;
  fontBBox.size.height
      = ((float)(gfi->fontBBox.ury - gfi->fontBBox.lly)) / 1000;

  isFixedPitch = gfi->isFixedPitch;

  /* Get the font type from the DGS server */
  PSWGetFontType ([fontName cString], &fontType);
  isBaseFont = (fontType != 0);

  maximumAdvancement.width = FLT_MIN;
  maximumAdvancement.height = FLT_MIN;
  minimumAdvancement.width = FLT_MAX;
  minimumAdvancement.height = FLT_MAX;

  /* Fill in the glyphs arrays. */
  glyphsByName = NSCreateMapTable (NSNonOwnedCStringMapKeyCallBacks,
				   NSObjectMapValueCallBacks, 256);
  for (i = 0; i < cFontInfo->numOfChars; i++) {
    AFMCharMetricInfo charMetricInfo = cFontInfo->cmi[i];
    AFMGlyphInfo* glyph
	= [AFMGlyphInfo glyphFromAFMCharMetricInfo:&charMetricInfo];
    NSSize glyphAdvancement = [glyph advancement];

    NSMapInsert (glyphsByName, [[glyph name] cString], glyph);
    maximumAdvancement.width
	= MAX (maximumAdvancement.width, glyphAdvancement.width);
    maximumAdvancement.height
	= MAX (maximumAdvancement.height, glyphAdvancement.height);
    minimumAdvancement.width
	= MIN (minimumAdvancement.width, glyphAdvancement.width);
    minimumAdvancement.height
	= MIN (minimumAdvancement.height, glyphAdvancement.height);
    if (charMetricInfo.code > 0) {
      glyphs[charMetricInfo.code] = [glyph retain];
    }
  }

  /* Set the entries in the kerning array for all the glyphs in the pair
     kerning array. */
  {
    AFMGlyphInfo* glyph1 = nil;
    AFMGlyphInfo* glyph2 = nil;
    const char* previousGlyphName = "";
    AFMPairKernData pkd;
    NSSize advancement;

    /* First compute the numbers of kern pairs for each glyph. This is used to
       avoid unnecessary allocations of kern pair arrays in glyph objects. */
    for (i = 0; i < cFontInfo->numOfPairs; i++) {
      pkd = cFontInfo->pkd[i];

      /* Check the name of the first glyph. Use a hint here: the kern pairs are
         grouped on the first glyph. */
      if (strcmp (pkd.name1, previousGlyphName)) {
	previousGlyphName = pkd.name1;
	glyph1 = NSMapGet (glyphsByName, pkd.name1);
      }
      [glyph1 incrementNumberOfKernPairs];
    }

    /* Now set the pair kerns in glyphs. */
    for (i = 0; i < cFontInfo->numOfPairs; i++) {
      pkd = cFontInfo->pkd[i];

      if (strcmp (pkd.name1, previousGlyphName)) {
	previousGlyphName = pkd.name1;
	glyph1 = NSMapGet (glyphsByName, pkd.name1);
      }

      glyph2 = NSMapGet (glyphsByName, pkd.name2);

      if (!glyph1) {
	NSLog (@"unknown glyph name %s in AFM file %@", pkd.name1, afmFileName);
	continue;
      }
      if (!glyph2) {
	NSLog (@"unknown glyph name %s in AFM file %@", pkd.name2, afmFileName);
	continue;
      }

      advancement = NSMakeSize (pkd.xamt, pkd.yamt);
      [glyph1 addPairKerningForGlyph:[glyph2 code] advancement:advancement];
    }
  }

  /* Free the cFontInfo structure */
  free (gfi->afmVersion);
  free (gfi->fontName);
  free (gfi->fullName);
  free (gfi->familyName);
  free (gfi->weight);
  free (gfi->version);
  free (gfi->notice);
  free (gfi->encodingScheme);
  for (i = 0; i < cFontInfo->numOfChars; i++) {
    AFMCharMetricInfo cmi = cFontInfo->cmi[i];
    free (cmi.name);
  }
  for (i = 0; i < cFontInfo->numOfPairs; i++) {
    AFMPairKernData pkd = cFontInfo->pkd[i];
    free (pkd.name1);
    free (pkd.name2);
  }
  free (cFontInfo);

  [pool release];

  return self;
}

- (void) transformUsingMatrix: (const float*)fmatrix
{
  float a = fmatrix[0];
  float b = fmatrix[1];
  float c = fmatrix[2];
  float d = fmatrix[3];
  float tx = fmatrix[4];
  float ty = fmatrix[5];
  float x1, y1, width1, height1;
  int i;

  memcpy(matrix, fmatrix, sizeof(matrix));

  for (i = 0; i < 256; i++) {
    [glyphs[i] transformUsingMatrix: fmatrix];
  }

  x1 = fontBBox.origin.x;
  y1 = fontBBox.origin.y;
  width1 = fontBBox.size.width;
  height1 = fontBBox.size.height;
  fontBBox.origin.x = a * x1 + c * y1 + tx;
  fontBBox.origin.y = b * x1 + d * y1 + ty;
  fontBBox.size.width = a * width1 + c * height1;
  fontBBox.size.height = b * width1 + d * height1;

  width1 = maximumAdvancement.width;
  height1 = maximumAdvancement.height;
  maximumAdvancement.width = a * width1 + c * height1;
  maximumAdvancement.height = b * width1 + d * height1;

  width1 = minimumAdvancement.width;
  height1 = minimumAdvancement.height;
  minimumAdvancement.width = a * width1 + c * height1;
  minimumAdvancement.height = b * width1 + d * height1;

  underlinePosition = a * underlinePosition + tx;
  underlineThickness = a * underlineThickness + tx;
  capHeight = a * capHeight + tx;
  ascender = a * ascender + tx;
  descender = a * descender + tx;

  xHeight = a * xHeight + tx;
}

- (AFMFileFontInfo *) transformedFontInfoForMatrix: (const float *)fmatrix
{
  AFMFileFontInfo *new = [self mutableCopy];
  [new transformUsingMatrix: fmatrix];
  return AUTORELEASE(new);
}

- initUnscaledWithFontName: (NSString*)name
{
  AFMFileFontInfo *fontInfo;

  /* Check whether the font info is cached */
  fontInfo = [globalFontInfoDictionary objectForKey: name];
  if (fontInfo != nil)
    {
      RELEASE(self);
      // retain to act like we were alloc'd
      return RETAIN(fontInfo);
    }
  /* Tough.  Parse the AFM file and create a new font info for
     the unscaled font. */
  [super init];
  fontName = RETAIN(name);
  if ([self _setFontInfo] == nil)
    {
      RELEASE(self);
      return nil;
    }
  /* Cache the font info for later use */
  [globalFontInfoDictionary setObject: self forKey: fontName];

  return self;
}

- initWithFontName: (NSString*)name
	    matrix: (const float *)fmatrix
	screenFont: (BOOL)screenFont
{
  AFMFileFontInfo *fontInfo, *baseFontInfo;

  if (screenFont)
    {
      RELEASE(self);
      return nil;
    }

  RELEASE(self);
  /* Grab an unscaled font info and create a new scaled one. */
  baseFontInfo = [[AFMFileFontInfo alloc] initUnscaledWithFontName: name];
  if (baseFontInfo == nil) 
    {
      return nil;
    }
  fontInfo = [baseFontInfo transformedFontInfoForMatrix: fmatrix];
  RELEASE(baseFontInfo);
  RETAIN(fontInfo);

  return fontInfo;
}

- (void) dealloc
{
  int i;

  /* Don't free the glyphsByName map table. It is owned by the first font info
     object that is never freed. */
  for (i = 0; i < 256; i++)
    TEST_RELEASE(glyphs[i]);

  TEST_RELEASE(afmFileName);
  [super dealloc];
}

- (void)set
{
  if ([[GSCurrentContext() focusView] isFlipped])
    {
      float invmatrix[6];
      memcpy(invmatrix, matrix, sizeof(invmatrix));
      invmatrix[3] = -invmatrix[3];
      PSWSetFont ([fontName cString], invmatrix);
    }
  else
    PSWSetFont ([fontName cString], matrix);
}

- (NSString*)afmFileContents
{
  return [NSString stringWithContentsOfFile: afmFileName];
}

- copyWithZone: (NSZone *)zone
{
  AFMFileFontInfo* new;
  if (NSShouldRetainWithZone(self, zone))
    new = RETAIN(self);
  else
    {
      int i;
      new = [super copyWithZone: zone];
      for (i = 0; i < 256; i++) {
        new->glyphs[i] = [glyphs[i] copyWithZone: zone];
      }
      new->afmFileName = [afmFileName copyWithZone: zone];
    }
  return new;
}

- mutableCopyWithZone: (NSZone *)zone
{
  int i;
  AFMFileFontInfo* new = [super mutableCopyWithZone: zone];
  for (i = 0; i < 256; i++) {
    new->glyphs[i] = [glyphs[i] mutableCopyWithZone: zone];
  }
  new->afmFileName = [afmFileName copyWithZone: zone];

  return new;
}

- (NSSize)advancementForGlyph:(NSGlyph)glyph
{
  return [glyphs[glyph] advancement];
}

- (NSRect)boundingRectForGlyph:(NSGlyph)glyph
{
  return [glyphs[glyph] boundingRect];
}

- (BOOL)glyphIsEncoded:(NSGlyph)glyph
{
  return [glyphs[glyph] isEncoded];
}

- (NSGlyph)glyphWithName:(NSString*)glyphName
{
  AFMGlyphInfo* glyph = NSMapGet (glyphsByName, [glyphName cString]);

  return glyph ? [glyph code] : -1;
}

- (NSPoint)positionOfGlyph:(NSGlyph)curGlyph
  precededByGlyph:(NSGlyph)prevGlyph
  isNominal:(BOOL*)nominal
{
  NSPoint point;
  NSSize size;

  if (curGlyph == NSControlGlyph || prevGlyph == NSControlGlyph
      || curGlyph < 0 || curGlyph > 255 || prevGlyph < 0 || prevGlyph >255) {
    if (nominal)
      *nominal = NO;
    return NSZeroPoint;
  }

  if (curGlyph == NSNullGlyph) {
    size = [glyphs[prevGlyph] advancement];
    point.x = size.width;
    point.y = size.height;
    if (nominal)
      *nominal = NO;
    return point;
  }

  if (glyphs[prevGlyph]) {
    if (!glyphs[curGlyph]) {
      point.x = maximumAdvancement.width;
      point.y = maximumAdvancement.height;
      if (nominal)
	*nominal = NO;
      return point;
    }
    size = [glyphs[prevGlyph] advancementIfFollowedByGlyph:
				      [glyphs[curGlyph] code]
			      isNominal:nominal];
    point.x = size.width;
    point.y = size.height;
    return point;
  }
  return NSZeroPoint;
}

- (float)widthOfString:(NSString*)string
{
  /* TODO: We should really map here the characters from string to
     series of glyphs. Until then we consider the glyphs to be
     equivalent with characters. */

  int i, length = [string length];
  const char* cString = [string cString];
  float width = 0;

  for (i = 0; i < length; i++) {
    width += [glyphs[(int)cString[i]] advancement].width;
  }

  return width;
}

@end /* AFMFileFontInfo */


@implementation AFMGlyphInfo

+ (AFMGlyphInfo*)glyphFromAFMCharMetricInfo:(AFMCharMetricInfo*)mi
{
  AFMGlyphInfo* glyph = [[self new] autorelease];

  glyph->name = [[NSString stringWithCString:mi->name] retain];
  glyph->code = mi->code;

  /* Setup bbox as defined by NSRect */
  glyph->bbox.origin.x = ((float)mi->charBBox.llx) / 1000;
  glyph->bbox.origin.y = ((float)mi->charBBox.lly) / 1000;
  glyph->bbox.size.width = ((float)mi->charBBox.urx - mi->charBBox.llx) / 1000;
  glyph->bbox.size.height = ((float)mi->charBBox.ury - mi->charBBox.lly)/ 1000;

  glyph->advancement.width = ((float)mi->wx) / 1000;
  glyph->advancement.height = ((float)mi->wy) / 1000;

  return glyph;
}

- (void)transformUsingMatrix:(const float*)matrix;
{
  float a = matrix[0];
  float b = matrix[1];
  float c = matrix[2];
  float d = matrix[3];
  float tx = matrix[4];
  float ty = matrix[5];
  float x1 = bbox.origin.x;
  float y1 = bbox.origin.y;
  float width1 = bbox.size.width;
  float height1 = bbox.size.height;
  int i;

  bbox.origin.x = a * x1 + c * y1 + tx;
  bbox.origin.y = b * x1 + d * y1 + ty;
  bbox.size.width = a * width1 + c * height1;
  bbox.size.height = b * width1 + d * height1;

  x1 = advancement.width;
  y1 = advancement.height;
  advancement.width = a * x1 + c * y1 + tx;
  advancement.height = b * x1 + d * y1 + ty;

  for (i = 0; i < numOfPairs; i++) {
    x1 = kerning[i].advancement.width;
    y1 = kerning[i].advancement.height;
    kerning[i].advancement.width = a * x1 + c * y1 + tx;
    kerning[i].advancement.height = b * x1 + d * y1 + ty;
  }
}

- (void)incrementNumberOfKernPairs
{
  numOfPairs++;
}

- (void)addPairKerningForGlyph:(NSGlyph)glyph advancement:(NSSize)_advancement
{
  tPairKerningInfo kernInfo = {
      glyph,
      { advancement.width + ((float)_advancement.width) / 1000,
        advancement.height + ((float)_advancement.height) / 1000
      }
  };
  int i;

  if (kerning == NULL)
    kerning = malloc (numOfPairs * sizeof (tPairKerningInfo));

  /* Insert the glyph in the proper position in the kerning array so this will
     be sorted ascending after the glyph code. */
  for (i = 0; i < lastKernPair; i++)
    if (kerning[i].glyph > glyph) {
      /* Make room for a new kerning pair in this position. */
      memmove (&kerning[i + 1], &kerning[i],
	       (lastKernPair - i) * sizeof (tPairKerningInfo));
      break;
    }

  kerning[i] = kernInfo;
  lastKernPair++;
}

- (NSSize)advancementIfFollowedByGlyph:(NSGlyph)glyph
  isNominal:(BOOL*)nominal
{
  /* Search for the glyph using a binary search algorithm */
  int lower = 0;
  int upper = numOfPairs;
  int midpoint;

  if (!kerning) {
    if (nominal)
      *nominal = NO;
    return advancement;
  }

  while (upper >= lower) {
    midpoint = (lower + upper) / 2;
    if (kerning[midpoint].glyph == glyph) {
      if (nominal)
	*nominal = YES;
      return kerning[midpoint].advancement;
    }
    else if (kerning[midpoint].glyph > glyph)
      upper = midpoint - 1;
    else if (kerning[midpoint].glyph < glyph)
      lower = midpoint + 1;
  }

  /* The glyph was not found in the kernings array. Return the advancement of
     receiver. */
  if (nominal)
    *nominal = NO;
  return advancement;
}

- copyWithZone: (NSZone *)zone
{
  AFMGlyphInfo *copy;
  if (NSShouldRetainWithZone(self, zone))
    copy = RETAIN(self);
  else
    {
      copy = (AFMGlyphInfo*) NSCopyObject (self, 0, zone);
      copy->name = [name copyWithZone: zone];
      if (kerning) 
	{
	  copy->kerning = malloc (numOfPairs * sizeof (tPairKerningInfo));
	  memcpy (copy->kerning, kerning, 
		  numOfPairs * sizeof (tPairKerningInfo));
	}
    }
  return copy;
}

- mutableCopyWithZone: (NSZone *)zone
{
  AFMGlyphInfo *copy;
  copy = (AFMGlyphInfo*) NSCopyObject (self, 0, zone);
  copy->name = [name copyWithZone: zone];
  if (kerning) 
    {
      copy->kerning = malloc (numOfPairs * sizeof (tPairKerningInfo));
      memcpy (copy->kerning, kerning, 
              numOfPairs * sizeof (tPairKerningInfo));
    }
  return copy;
}

- (void)dealloc
{
  RELEASE(name);
  if (kerning)
    free (kerning);
  [super dealloc];
}

- (NSString*)name	{ return name; }
- (NSGlyph)code		{ return code; }
- (NSRect)boundingRect	{ return bbox; }
- (NSSize)advancement	{ return advancement; }
- (BOOL)isEncoded	{ return YES; }

@end /* AFMGlyphInfo */
