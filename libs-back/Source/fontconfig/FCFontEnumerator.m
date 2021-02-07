/*
   FCFontEnumerator.m
 
   Copyright (C) 2003 Free Software Foundation, Inc.

   August 31, 2003
   Written by Banlu Kemiyatorn <object at gmail dot com>
   Base on original code of Alex Malmberg
   Rewrite: Fred Kiefer <fredkiefer@gmx.de>
   Date: Jan 2006
 
   This file is part of GNUstep.

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

#include <Foundation/NSObject.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSSet.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSValue.h>
#include <Foundation/NSPathUtilities.h>
#include <Foundation/NSFileManager.h>
#include <Foundation/NSUserDefaults.h>
#include <Foundation/NSBundle.h>
#include <Foundation/NSDebug.h>
#include <GNUstepGUI/GSFontInfo.h>
#include <AppKit/NSAffineTransform.h>
#include <AppKit/NSBezierPath.h>
#include <AppKit/NSFontDescriptor.h>

#include "gsc/GSGState.h"
#include "fontconfig/FCFontEnumerator.h"
#include "fontconfig/FCFontInfo.h"

// Old versions of fontconfig don't have FC_WEIGHT_ULTRABLACK defined.
// Use the maximal value instead.
#ifndef FC_WEIGHT_ULTRABLACK
#define FC_WEIGHT_ULTRABLACK FC_WEIGHT_BLACK
#endif

static float
convertWeight (int fcWeight, int bottomValue, int topValue)
{
  /*
    This is the distance between topValue and bottomValue expressed as a
    fraction between zero and one. We do this to express the range of
    fontconfig font weights in a useful manner.
  */

  if (fcWeight <= bottomValue)
    {
      return 0.0;
    }
  else if (fcWeight >= topValue)
    {
      return 1.0;
    }
  else
    {
      return (float) (fcWeight - bottomValue) * (1.0f / (topValue - bottomValue));
    }
}

static NSComparisonResult
sortFontFacesArray(id fontArr1, id fontArr2, void *context)
{
  /*
    Order of array:
      0: Font name
      1: Font style
      2: Font weight
      3: Font traits
  */
  NSString *style1 = [fontArr1 objectAtIndex: 1];
  NSString *style2 = [fontArr2 objectAtIndex: 1];
  float weight1 = [[fontArr1 objectAtIndex: 2] floatValue];
  float weight2 = [[fontArr2 objectAtIndex: 2] floatValue];
  unsigned int traits1 = [[fontArr1 objectAtIndex: 3] unsignedIntValue];
  unsigned int traits2 = [[fontArr2 objectAtIndex: 3] unsignedIntValue];

  // order first by weight
  if (weight1 < weight2)
    return NSOrderedAscending;
  if (weight1 > weight2)
    return NSOrderedDescending;

  // Italic next
  if ((traits1 & NSItalicFontMask) < (traits2 & NSItalicFontMask))
    return NSOrderedAscending;
  if ((traits1 & NSItalicFontMask) > (traits2 & NSItalicFontMask))
    return NSOrderedDescending;

  // now do condensed
  if ((traits1 & NSCondensedFontMask) < (traits2 & NSCondensedFontMask))
    return NSOrderedAscending;
  if ((traits1 & NSCondensedFontMask) > (traits2 & NSCondensedFontMask))
    return NSOrderedDescending;
  // ...and expanded
  if ((traits1 & NSExpandedFontMask) < (traits2 & NSExpandedFontMask))
    return NSOrderedAscending;
  if ((traits1 & NSExpandedFontMask) > (traits2 & NSExpandedFontMask))
    return NSOrderedDescending;

  // Special case: "Regular" sorts before non-Regular, for many reasons.
  if ([style1 isEqualToString: @"Regular"] && ![style2 isEqualToString: @"Regular"])
    return NSOrderedAscending;
  if ([style2 isEqualToString: @"Regular"] && ![style1 isEqualToString: @"Regular"])
    return NSOrderedDescending;
  if ([style1 isEqualToString: @"Normal"] && ![style2 isEqualToString: @"Normal"])
    return NSOrderedAscending;
  if ([style2 isEqualToString: @"Normal"] && ![style1 isEqualToString: @"Normal"])
    return NSOrderedDescending;
  if ([style1 isEqualToString: @"Roman"] && ![style2 isEqualToString: @"Roman"])
    return NSOrderedAscending;
  if ([style2 isEqualToString: @"Roman"] && ![style1 isEqualToString: @"Roman"])
    return NSOrderedDescending;

  // Otherwise, alphabetize
  return [style1 compare: style2];
}

@implementation FCFontEnumerator 

NSMutableDictionary * __allFonts;

+ (FCFaceInfo *) fontWithName: (NSString *) name
{
  FCFaceInfo *face;

  face = [__allFonts objectForKey: name];
  if (!face)
    {
      NSDebugLLog(@"NSFont", @"Font not found %@", name);
    }
  return face;
}

+ (Class) faceInfoClass
{
  [self subclassResponsibility: _cmd];
  return nil;
}

// Make a GNUstep style font descriptor from a FcPattern
static NSArray *faFromFc(FcPattern *pat)
{
  int weight, slant, spacing, width;
  float nsweight;
  unsigned int nstraits = 0;
  char *fcfamily, *fcstyle;
  NSMutableString *name, *family, *style;
#ifdef FC_POSTSCRIPT_NAME
  char *fcname;
#endif

  if (FcPatternGetInteger(pat, FC_WEIGHT, 0, &weight) != FcResultMatch
    || FcPatternGetInteger(pat, FC_SLANT,  0, &slant) != FcResultMatch
    || FcPatternGetString(pat, FC_FAMILY, 0, (FcChar8 **)&fcfamily)
      != FcResultMatch)
    return nil;

  if (FcPatternGetInteger(pat, FC_SPACING, 0, &spacing) == FcResultMatch)
    if (spacing==FC_MONO || spacing==FC_CHARCELL)
      nstraits |= NSFixedPitchFontMask;

  name = [NSMutableString stringWithCapacity: 100];
#ifdef FC_POSTSCRIPT_NAME
  if (FcPatternGetString(pat, FC_POSTSCRIPT_NAME,  0, (FcChar8 **)&fcname) == FcResultMatch)
    [name appendString: [NSMutableString stringWithUTF8String: fcname]];
#endif

  family = [NSMutableString stringWithUTF8String: fcfamily];
  style = [NSMutableString stringWithCapacity: 100];

  if (weight < FC_WEIGHT_ULTRALIGHT)
    {
      [style appendString: @"Thin"];
      nsweight = 1 + convertWeight (weight, FC_WEIGHT_THIN, FC_WEIGHT_ULTRALIGHT);
    }
  else if (weight < FC_WEIGHT_LIGHT)
    {
      [style appendString: @"Ultralight"];
      nsweight = 2 + convertWeight (weight, FC_WEIGHT_ULTRALIGHT, FC_WEIGHT_LIGHT);
    }
  else if (weight < FC_WEIGHT_BOOK)
    {
      [style appendString: @"Light"];
      nsweight = 3 + convertWeight (weight, FC_WEIGHT_LIGHT, FC_WEIGHT_BOOK);
    }
  else if (weight < FC_WEIGHT_REGULAR)
    {
      [style appendString: @"Book"];
      nsweight = 4 + convertWeight (weight, FC_WEIGHT_BOOK, FC_WEIGHT_REGULAR);
    }
  else if (weight < FC_WEIGHT_MEDIUM)
    {
      nsweight = 5 + convertWeight (weight, FC_WEIGHT_REGULAR, FC_WEIGHT_MEDIUM);
    }
  else if (weight < FC_WEIGHT_DEMIBOLD)
    {
      [style appendString: @"Medium"];
      nsweight = 6 + convertWeight (weight, FC_WEIGHT_MEDIUM, FC_WEIGHT_DEMIBOLD);
    }
  else if (weight < FC_WEIGHT_BOLD)
    {
      [style appendString: @"Demibold"];
      nsweight = 7 + convertWeight (weight, FC_WEIGHT_DEMIBOLD, FC_WEIGHT_BOLD);
    }
  else if (weight < FC_WEIGHT_ULTRABOLD)
    {
      [style appendString: @"Bold"];
      nsweight = 9 + convertWeight (weight, FC_WEIGHT_BOLD, FC_WEIGHT_ULTRABOLD);
      nstraits |= NSBoldFontMask;
    }
  else if (weight < FC_WEIGHT_BLACK)
    {
      [style appendString: @"Ultrabold"];
      nsweight = 11 + convertWeight (weight, FC_WEIGHT_ULTRABOLD, FC_WEIGHT_BLACK);
      nstraits |= NSBoldFontMask;
    }
  else if (weight < FC_WEIGHT_ULTRABLACK)
    {
      [style appendString: @"Black"];
      nsweight = 12 + convertWeight (weight, FC_WEIGHT_BLACK, FC_WEIGHT_ULTRABLACK);
      nstraits |= NSBoldFontMask;
    }
  else
    {
      [style appendString: @"Ultrablack"];
      nsweight = 13 + convertWeight (weight, FC_WEIGHT_ULTRABLACK, FC_WEIGHT_ULTRABLACK + 20);
      nstraits |= NSBoldFontMask;
    }

  if (FcPatternGetInteger(pat, FC_WIDTH, 0, &width) == FcResultMatch)
    {
      if (width < FC_WIDTH_EXTRACONDENSED)
	{
	  [style appendString: @"Ultracondensed"];
	  nstraits |= NSCondensedFontMask;
	}
      else if (width < FC_WIDTH_CONDENSED)
	{
	  [style appendString: @"Extracondensed"];
	  nstraits |= NSCondensedFontMask;
	}
      else if (width < FC_WIDTH_SEMICONDENSED)
	{
	  [style appendString: @"Condensed"];
	  nstraits |= NSCondensedFontMask;
	}
      else if (width < FC_WIDTH_SEMIEXPANDED)
	{
	  // do nothing, this is "regular"
	}
      else if (width < FC_WIDTH_EXPANDED)
	{
	  [style appendString: @"Semiexpanded"];
	  nstraits |= NSExpandedFontMask;
	}
      else if (width < FC_WIDTH_EXTRAEXPANDED)
	{
	  [style appendString: @"Expanded"];
	  nstraits |= NSExpandedFontMask;
	}
      else if (width < FC_WIDTH_ULTRAEXPANDED)
	{
	  [style appendString: @"Extraexpanded"];
	  nstraits |= NSExpandedFontMask;
	}
      else
	{
	  [style appendString: @"Ultraexpanded"];
	  nstraits |= NSExpandedFontMask;
	}
    }

  switch (slant) 
    {
      case FC_SLANT_ROMAN:
        break;
      case FC_SLANT_ITALIC:
        [style appendString: @"Italic"];
        nstraits |= NSItalicFontMask;
        break;
      case FC_SLANT_OBLIQUE:
        [style appendString: @"Oblique"];
        nstraits |= NSItalicFontMask;
        break;
    }

  if (![name length])	// no psname
    {
      NSDebugLLog(@"NSFont", @"Warning: synthesizing PSName for '%@ %@'", family, style);
      [name appendString: family];
      if ([style length] > 0)
        {
          [name appendString: @"-"];
          [name appendString: style];
        }
    }

  if (![style length])
    {
      [style setString: @"Regular"];
    }

  if (FcPatternGetString(pat, FC_STYLE, 0, (FcChar8 **)&fcstyle) == FcResultMatch)
    style = [NSString stringWithUTF8String: fcstyle];

//  NSLog (@"family: %@, style: %s/%@", name, fcstyle, style);
  return [NSArray arrayWithObjects: name, 
		  style, 
		  [NSNumber numberWithFloat: nsweight],
		  [NSNumber numberWithUnsignedInt: nstraits],
		  nil];
}

- (void) enumerateFontsAndFamilies
{
  int i;
  NSMutableDictionary *fcxft_allFontFamilies = [NSMutableDictionary new];
  NSMutableDictionary *fcxft_allFonts = [NSMutableDictionary new];
  NSMutableArray *fcxft_allFontNames = [NSMutableArray new];
  Class faceInfoClass = [[self class] faceInfoClass];

  FcPattern *pat = FcPatternCreate();
  FcObjectSet *os = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FULLNAME,
#ifdef FC_POSTSCRIPT_NAME
                                     FC_POSTSCRIPT_NAME,
#endif
                                     FC_SLANT, FC_WEIGHT, FC_WIDTH,
                                     FC_SPACING, NULL);
  FcFontSet *fs = FcFontList(NULL, pat, os);

  FcPatternDestroy(pat);
  FcObjectSetDestroy(os);

  for (i = 0; i < fs->nfont; i++)
    {
      char *family;

      if (FcPatternGetString(fs->fonts[i], FC_FAMILY, 0, (FcChar8 **)&family)
          == FcResultMatch)
        {
          NSArray *fontArray;

          if ((fontArray = faFromFc(fs->fonts[i])))
            {
              NSString *name = [fontArray objectAtIndex: 0];

              if (![fcxft_allFontNames containsObject: name])
                {
                  NSString *familyString;
                  NSMutableArray *familyArray;
                  FCFaceInfo *aFont;

                  familyString = [NSString stringWithUTF8String: family];
                  familyArray = [fcxft_allFontFamilies objectForKey: familyString];

                  if (familyArray == nil)
                    {
                      NSDebugLLog(@"NSFont", @"Found font family %@", familyString);
                      familyArray = [[NSMutableArray alloc] init];
                      [fcxft_allFontFamilies setObject: familyArray
                                                forKey: familyString];
                      RELEASE(familyArray);
                    }

                  NSDebugLLog(@"NSFont", @"fc enumerator: adding font: %@", name);
                  [familyArray addObject: fontArray];
                  [fcxft_allFontNames addObject: name];
                  aFont = [[faceInfoClass alloc] initWithfamilyName: familyString
                              weight: [[fontArray objectAtIndex: 2] floatValue]
                              traits: [[fontArray objectAtIndex: 3] unsignedIntValue]
                             pattern: fs->fonts[i]];
                  [fcxft_allFonts setObject: aFont forKey: name];
                  RELEASE(aFont);
                }
            }
        }
    }
  FcFontSetDestroy (fs); 

  allFontNames = fcxft_allFontNames;
  allFontFamilies = fcxft_allFontFamilies;
  __allFonts = fcxft_allFonts;

  // Sort font families
  {
    NSComparisonResult (*fontSort)(id, id, void *) = sortFontFacesArray;
    NSEnumerator *e = [allFontFamilies keyEnumerator];
    id key;

    while ((key = [e nextObject]))
      {
        [[allFontFamilies objectForKey: key] sortUsingFunction: fontSort context: NULL];
      }
  }
}

- (NSString *) defaultSystemFontName
{
  if ([allFontNames containsObject: @"DejaVuSans"])
    {
      return @"DejaVuSans";
    }
  if ([allFontNames containsObject: @"BitstreamVeraSans-Roman"])
    {
      return @"BitstreamVeraSans-Roman";
    }
  if ([allFontNames containsObject: @"FreeSans"])
    {
      return @"FreeSans";
    }
  if ([allFontNames containsObject: @"Tahoma"])
    {
      return @"Tahoma";
    }
  if ([allFontNames containsObject: @"ArialMT"])
    {
      return @"ArialMT";
    }
  return @"Helvetica";
}

- (NSString *) defaultBoldSystemFontName
{
  if ([allFontNames containsObject: @"DejaVuSans-Bold"])
    {
      return @"DejaVuSans-Bold";
    }
  if ([allFontNames containsObject: @"BitstreamVeraSans-Bold"])
    {
      return @"BitstreamVeraSans-Bold";
    }
  if ([allFontNames containsObject: @"FreeSans-Bold"])
    {
      return @"FreeSans-Bold";
    }
  if ([allFontNames containsObject: @"Tahoma-Bold"])
    {
      return @"Tahoma-Bold";
    }
  if ([allFontNames containsObject: @"Arial-BoldMT"])
    {
      return @"Arial-BoldMT";
    }
  return @"Helvetica-Bold";
}

- (NSString *) defaultFixedPitchFontName
{
  if ([allFontNames containsObject: @"DejaVuSansMono"])
    {
      return @"DejaVuSansMono";
    }
  if ([allFontNames containsObject: @"BitstreamVeraSansMono-Roman"])
    {
      return @"BitstreamVeraSansMono-Roman";
    }
  if ([allFontNames containsObject: @"FreeMono"])
    {
      return @"FreeMono";
    }
  if ([allFontNames containsObject: @"CourierNewPSMT"])
    {
      return @"CourierNewPSMT";
    }
  return @"Courier";
}

/**
 * Overrides the implementation in GSFontInfo, and delegates the
 * matching to Fontconfig.
 */
- (NSArray *) matchingFontDescriptorsFor: (NSDictionary *)attributes
{
  NSMutableArray *descriptors;
  FcResult result;
  FcPattern *matchedpat, *pat;
  FontconfigPatternGenerator *generator;

  descriptors = [NSMutableArray array];

  generator = [[FontconfigPatternGenerator alloc] init];
  pat = [generator createPatternWithAttributes: attributes];
  DESTROY(generator);

  FcConfigSubstitute(NULL, pat, FcMatchPattern);
  FcDefaultSubstitute(pat);
  result = FcResultMatch;
  matchedpat = FcFontMatch(NULL, pat, &result);
  if (result != FcResultMatch)
    {
      NSLog(@"Warning, FcFontMatch failed with code: %d", result);
    }
  else
    {
      FcFontSet *fontSet;
      result = FcResultMatch;
      fontSet = FcFontSort(NULL, matchedpat, FcFalse, NULL, &result);
      if (result == FcResultMatch)
	{
	  int i;
	  for (i=0; i<fontSet->nfont; i++)
	    {
	      FontconfigPatternParser *parser = [[FontconfigPatternParser alloc] init];
	      // FIXME: do we need to match this pattern?
	      FcPattern *matchingpat = fontSet->fonts[i];
	      NSDictionary *attribs = [parser attributesFromPattern: matchingpat];
	      [parser release];

	      [descriptors addObject: [NSFontDescriptor fontDescriptorWithFontAttributes: attribs]];
	    }
	}
      else
	{
	  NSLog(@"ERROR! FcFontSort failed");
	}

      FcFontSetDestroy(fontSet);
      FcPatternDestroy(matchedpat);
    }
  
  FcPatternDestroy(pat);
  return descriptors;
}

@end


@implementation FontconfigPatternGenerator

- (void)addName: (NSString*)name
{
#ifdef FC_POSTSCRIPT_NAME
  FcPatternAddString(_pat, FC_POSTSCRIPT_NAME, (const FcChar8 *)[name UTF8String]);
#else
  // FIXME: Fontconfig ignores PostScript names of fonts; we need
  // https://bugs.freedesktop.org/show_bug.cgi?id=18095 fixed.
  
  // This is a heuristic to try to 'parse' a PostScript font name,
  // however, since they are just unique identifiers for fonts and
  // don't need to follow any naming convention, this may fail
  NSRange dash = [name rangeOfString: @"-"];
  if (dash.location == NSNotFound)
    {
      FcPatternAddString(_pat, FC_FAMILY, (const FcChar8 *)[name UTF8String]);
    }
  else
    {
      NSString *weightAndSlant = [name substringFromIndex: dash.location + 1];
      NSString *family = [name substringToIndex: dash.location];

      FcPatternAddString(_pat, FC_FAMILY, (const FcChar8 *)[family UTF8String]);

      if (NSNotFound != [weightAndSlant rangeOfString: @"Light"].location)
	{
	  FcPatternAddInteger(_pat, FC_WEIGHT, FC_WEIGHT_LIGHT);
	}
      else if (NSNotFound != [weightAndSlant rangeOfString: @"Medium"].location)
	{
	  FcPatternAddInteger(_pat, FC_WEIGHT, FC_WEIGHT_MEDIUM);
	}
      else if (NSNotFound != [weightAndSlant rangeOfString: @"Demibold"].location)
	{
	  FcPatternAddInteger(_pat, FC_WEIGHT, FC_WEIGHT_DEMIBOLD);
	}
      else if (NSNotFound != [weightAndSlant rangeOfString: @"Bold"].location)
	{
	  FcPatternAddInteger(_pat, FC_WEIGHT, FC_WEIGHT_BOLD);
	}
      else if (NSNotFound != [weightAndSlant rangeOfString: @"Black"].location)
	{
	  FcPatternAddInteger(_pat, FC_WEIGHT, FC_WEIGHT_BLACK);
	}

      if (NSNotFound != [weightAndSlant rangeOfString: @"Italic"].location)
	{
	  FcPatternAddInteger(_pat, FC_SLANT, FC_SLANT_ITALIC);
	}
      else if (NSNotFound != [weightAndSlant rangeOfString: @"Oblique"].location)
	{
	  FcPatternAddInteger(_pat, FC_SLANT, FC_SLANT_OBLIQUE);
	}

      if (NSNotFound != [weightAndSlant rangeOfString: @"Condensed"].location)
	{
	  FcPatternAddInteger(_pat, FC_WIDTH, FC_WIDTH_CONDENSED);
	}
      else if (NSNotFound != [weightAndSlant rangeOfString: @"Expanded"].location)
	{
	  FcPatternAddInteger(_pat, FC_WIDTH, FC_WIDTH_EXPANDED);
	}
    }
#endif
}

- (void)addVisibleName: (NSString*)name
{
  FcPatternAddString(_pat, FC_FULLNAME, (const FcChar8 *)[name UTF8String]);
}

- (void)addFamilyName: (NSString*)name
{
  FcPatternAddString(_pat, FC_FAMILY, (const FcChar8 *)[name UTF8String]);
}

- (void)addStyleName: (NSString*)style
{
  FcPatternAddString(_pat, FC_STYLE, (const FcChar8 *)[style UTF8String]);
}

- (void)addTraits: (NSDictionary*)traits
{
  if ([traits objectForKey: NSFontSymbolicTrait])
    {
      NSFontSymbolicTraits symTraits = [[traits objectForKey: NSFontSymbolicTrait] intValue];

      if (symTraits & NSFontItalicTrait)
	{
	  // NOTE: May be overridden by NSFontSlantTrait
	  FcPatternAddInteger(_pat, FC_SLANT, FC_SLANT_ITALIC);
	}
      if (symTraits & NSFontBoldTrait)
	{
	  // NOTE: May be overridden by NSFontWeightTrait
	  FcPatternAddInteger(_pat, FC_WEIGHT, FC_WEIGHT_BOLD);
	}
      if (symTraits & NSFontExpandedTrait)
	{
	  // NOTE: May be overridden by NSFontWidthTrait
	  FcPatternAddInteger(_pat, FC_WIDTH, FC_WIDTH_EXPANDED);
	}
      if (symTraits & NSFontCondensedTrait)
	{
	  // NOTE: May be overridden by NSFontWidthTrait
	  FcPatternAddInteger(_pat, FC_WIDTH, FC_WIDTH_CONDENSED);
	}
      if (symTraits & NSFontMonoSpaceTrait)
	{
	  FcValue value;
	  // If you run "fc-match :spacing=100", you get "DejaVu Sans" even though you would
	  // expect to get "DejaVu Sans Mono". So, we also add "monospace" as a weak family
	  // name to fix the problem.
	  FcPatternAddInteger(_pat, FC_SPACING, FC_MONO);
	  
	  value.type = FcTypeString;
	  value.u.s = (FcChar8*)"monospace";
	  FcPatternAddWeak(_pat, FC_FAMILY, value, FcTrue);
	}
      if (symTraits & NSFontVerticalTrait)
	{
	  // Fontconfig can't express this (it means sideways letters)
	}
      if (symTraits & NSFontUIOptimizedTrait)
	{
	  // NOTE: Fontconfig can't express this
	}

      {
	NSFontFamilyClass class = symTraits & NSFontFamilyClassMask;
	char *addWeakFamilyName = NULL;
	switch (class)
	  {
	  default:
	  case NSFontUnknownClass:
	  case NSFontOrnamentalsClass:
	  case NSFontScriptsClass:
	  case NSFontSymbolicClass:
	    // FIXME: Is there some way to convey these to Fontconfig?
	    break;
	  case NSFontOldStyleSerifsClass:
	  case NSFontTransitionalSerifsClass:
	  case NSFontModernSerifsClass:
	  case NSFontClarendonSerifsClass:
	  case NSFontSlabSerifsClass:
	  case NSFontFreeformSerifsClass:
	    addWeakFamilyName = "serif";
	    break;
	  case NSFontSansSerifClass:
	    addWeakFamilyName = "sans";
	    break;
	  }
	if (addWeakFamilyName)
	  {
	    FcValue value;
	    value.type = FcTypeString;
	    value.u.s = (const FcChar8 *)addWeakFamilyName;
	    FcPatternAddWeak(_pat, FC_FAMILY, value, FcTrue);
	  }
      }
    }

  if ([traits objectForKey: NSFontWeightTrait])
    {
      /**
       * Scale: -1 is thinnest, 0 is normal, 1 is heaviest
       */
      double weight = [[traits objectForKey: NSFontWeightTrait] doubleValue];
      int fcWeight;

      weight = MAX(-1, MIN(1, weight));
      if (weight <= 0)
	{
	  fcWeight = FC_WEIGHT_THIN + ((weight + 1.0) * (FC_WEIGHT_NORMAL - FC_WEIGHT_THIN));
	}
      else
	{
	  fcWeight = FC_WEIGHT_NORMAL + (weight * (FC_WEIGHT_ULTRABLACK - FC_WEIGHT_NORMAL));
	}
      FcPatternAddInteger(_pat, FC_WEIGHT, fcWeight);
    }

  if ([traits objectForKey: NSFontWidthTrait])
    {
      /**
       * Scale: -1 is most condensed, 0 is normal, 1 is most spread apart
       */
      double width = [[traits objectForKey: NSFontWidthTrait] doubleValue];
      int fcWidth;

      width = MAX(-1, MIN(1, width));
      if (width <= 0)
	{
	  fcWidth = FC_WIDTH_ULTRACONDENSED + ((width + 1.0) * (FC_WIDTH_NORMAL - FC_WIDTH_ULTRACONDENSED));
	}
      else
	{
	  fcWidth = FC_WIDTH_NORMAL + (width * (FC_WIDTH_ULTRAEXPANDED - FC_WIDTH_NORMAL));
	}
      FcPatternAddInteger(_pat, FC_WIDTH, fcWidth);
    }

  if ([traits objectForKey: NSFontSlantTrait])
    {
      /**
       * Scale: -1 is 30 degree counterclockwise slant, 0 is no slant, 1
       * is 30 degree clockwise slant
       */
      double slant = [[traits objectForKey: NSFontSlantTrait] doubleValue];

      // NOTE: Fontconfig can't express this as a scale
      if (slant > 0)
	{
	  FcPatternAddInteger(_pat, FC_SLANT, FC_SLANT_ITALIC);
	}
      else
	{
	  FcPatternAddInteger(_pat, FC_SLANT, FC_SLANT_ROMAN);
	}
    }
}

- (void)addSize: (NSNumber*)size
{
  FcPatternAddDouble(_pat, FC_SIZE, [size doubleValue]);
}

- (void)addCharacterSet: (NSCharacterSet*)characterSet
{
  if ([characterSet isKindOfClass: [FontconfigCharacterSet class]])
    {
      // Fast case
      FcPatternAddCharSet(_pat, FC_CHARSET, [(FontconfigCharacterSet*)characterSet fontconfigCharSet]);
    }
  else
    {
      // Slow case
      FcCharSet *fcSet = FcCharSetCreate();
      uint32_t plane;
      for (plane=0; plane<=16; plane++)
	{
	  if ([characterSet hasMemberInPlane: plane])
	    {
	      uint32_t codePoint;
	      for (codePoint = plane<<16; codePoint <= 0xffff + (plane<<16); codePoint++)
		{
		  if ([characterSet longCharacterIsMember: codePoint])
		    {
		      FcCharSetAddChar(fcSet, codePoint);
		    }
		}
	    }
	}
      
      FcPatternAddCharSet(_pat, FC_CHARSET, fcSet);
      FcCharSetDestroy(fcSet);
    }
}

#define ADD_TO_PATTERN(key, handlerMethod, valueClass)	\
  do {							\
    id value = [_attributes objectForKey: key];		\
    if (value)						\
      {							\
	if ([value isKindOfClass: valueClass])		\
	  {								\
	    [self handlerMethod value];					\
	  }								\
	else								\
	  {								\
	    NSLog(@"NSFontDescriptor: Ignoring invalid value %@ for attribute %@", value, key);	\
	  }								\
      }									\
  } while (0);

- (void)addAttributes
{
  ADD_TO_PATTERN(NSFontNameAttribute, addName:, [NSString class]);
  ADD_TO_PATTERN(NSFontVisibleNameAttribute, addVisibleName:, [NSString class]);
  ADD_TO_PATTERN(NSFontFamilyAttribute, addFamilyName:, [NSString class]);
  ADD_TO_PATTERN(NSFontFaceAttribute, addStyleName:, [NSString class]);
  ADD_TO_PATTERN(NSFontTraitsAttribute, addTraits:, [NSDictionary class]);
  ADD_TO_PATTERN(NSFontSizeAttribute, addSize:, [NSNumber class]);
  ADD_TO_PATTERN(NSFontCharacterSetAttribute, addCharacterSet:, [NSCharacterSet class]);
}

- (FcPattern *)createPatternWithAttributes: (NSDictionary *)attributes
{
  _attributes = attributes;
  _pat = FcPatternCreate();
  [self addAttributes];

  return _pat;
}

@end


@implementation FontconfigPatternParser

- (NSString*)readFontconfigString: (const char *)key fromPattern: (FcPattern*)pat
{
  unsigned char *string = NULL;
  if (FcResultMatch == FcPatternGetString(pat, key, 0, &string))
    {
      if (string)
	{
	  return [NSString stringWithUTF8String: (const char *)string];
	}
    }
  return nil;
}

- (NSNumber*)readFontconfigInteger: (const char *)key fromPattern: (FcPattern*)pat
{
  int value;
  if (FcResultMatch == FcPatternGetInteger(pat, key, 0, &value))
    {
      return [NSNumber numberWithInt: value];
    }
  return nil;
}

- (NSNumber*)readFontconfigDouble: (const char *)key fromPattern: (FcPattern*)pat
{
  double value;
  if (FcResultMatch == FcPatternGetDouble(pat, key, 0, &value))
    {
      return [NSNumber numberWithDouble: value];
    }
  return nil;
}



- (NSString*)readNameFromPattern: (FcPattern*)pat
{
#ifdef FC_POSTSCRIPT_NAME
  NSString *name = [self readFontconfigString: FC_POSTSCRIPT_NAME fromPattern: pat];
#endif
  NSString *family = [self readFontconfigString: FC_FAMILY fromPattern: pat];
  NSString *style = [self readFontconfigString: FC_STYLE fromPattern: pat];

#ifdef FC_POSTSCRIPT_NAME
  if (name)
    return name;
  else
#endif
    if (style)
      {
        return [NSString stringWithFormat: @"%@-%@", family, style];
      }
    else
      {
        return family;
      }
}

- (NSString*)readVisibleNameFromPattern: (FcPattern*)pat
{
  // FIXME: try to get the localized one
  return [self readFontconfigString: FC_FULLNAME fromPattern: pat];
}
- (NSString*)readFamilyNameFromPattern: (FcPattern*)pat
{
  // FIXME: try to get the localized one
  return [self readFontconfigString: FC_FAMILY fromPattern: pat];
}
- (NSString*)readStyleNameFromPattern: (FcPattern*)pat
{
  // FIXME: try to get the localized one
  return [self readFontconfigString: FC_STYLE fromPattern: pat];
}
- (NSDictionary*)readTraitsFromPattern: (FcPattern*)pat
{
  NSMutableDictionary *traits = [NSMutableDictionary dictionary];

  NSFontSymbolicTraits symTraits = 0;

  int value;
  if (FcResultMatch == FcPatternGetInteger(pat, FC_SLANT, 0, &value))
    {
      if (value > FC_SLANT_ROMAN)
	{
	  symTraits |= NSFontItalicTrait;
	}
    }
  if (FcResultMatch == FcPatternGetInteger(pat, FC_WEIGHT, 0, &value))
    {
      double weight;

      if (value >= FC_WEIGHT_BOLD)
	{
	  symTraits |= NSFontBoldTrait;
	}

      if (value <= FC_WEIGHT_NORMAL)
	{
	  weight = ((value - FC_WEIGHT_THIN) / (double)(FC_WEIGHT_NORMAL - FC_WEIGHT_THIN)) - 1.0;
	}
      else
	{
	  weight = (value - FC_WEIGHT_NORMAL) / (double)(FC_WEIGHT_ULTRABLACK - FC_WEIGHT_NORMAL);
	}

      [traits setObject: [NSNumber numberWithDouble: weight]
		 forKey: NSFontWeightTrait];
    }
  if (FcResultMatch == FcPatternGetInteger(pat, FC_WIDTH, 0, &value))
    {
      double width;

      if (value >= FC_WIDTH_EXPANDED)
	{
	  symTraits |= NSFontExpandedTrait;
	}
      if (value <= FC_WIDTH_CONDENSED)
	{
	  symTraits |= NSFontCondensedTrait;
	}

      if (value <= FC_WIDTH_NORMAL)
	{
	  width = ((value - FC_WIDTH_ULTRACONDENSED) / (double)(FC_WIDTH_NORMAL - FC_WIDTH_ULTRACONDENSED)) - 1.0;
	}
      else
	{
	  width = (value - FC_WIDTH_NORMAL) / (double)(FC_WIDTH_ULTRAEXPANDED - FC_WIDTH_NORMAL);
	}

      [traits setObject: [NSNumber numberWithDouble: width]
		 forKey: NSFontWidthTrait];
    }
  if (FcResultMatch == FcPatternGetInteger(pat, FC_SPACING, 0, &value))
    {
      if (value == FC_MONO || value == FC_CHARCELL)
	{
	  symTraits |= NSFontMonoSpaceTrait;
	}
    }

  if (symTraits != 0)
    {
      [traits setObject: [NSNumber numberWithUnsignedInt: symTraits]
		 forKey: NSFontSymbolicTrait];
    }

  return traits;
}

- (NSNumber*)readSizeFromPattern: (FcPattern*)pat
{
  return [self readFontconfigDouble: FC_SIZE fromPattern: pat];
}

- (NSCharacterSet*)readCharacterSetFromPattern: (FcPattern*)pat
{
  FcCharSet *value;
  if (FcResultMatch == FcPatternGetCharSet(pat, FC_CHARSET, 0, &value))
    {
      return [[[FontconfigCharacterSet alloc] initWithFontconfigCharSet: value] autorelease];
    }
  return nil;
}

#define READ_FROM_PATTERN(key, readMethod)	\
  do {						\
    id result = [self readMethod _pat];		\
    if (result != nil)				\
      {						\
	[_attributes setObject: result		\
			forKey: key];		\
      }						\
  } while (0);

- (void)parseAttributes
{
  READ_FROM_PATTERN(NSFontNameAttribute, readNameFromPattern:);
  READ_FROM_PATTERN(NSFontVisibleNameAttribute, readVisibleNameFromPattern:);
  READ_FROM_PATTERN(NSFontFamilyAttribute, readFamilyNameFromPattern:);
  READ_FROM_PATTERN(NSFontFaceAttribute, readStyleNameFromPattern:);
  READ_FROM_PATTERN(NSFontTraitsAttribute, readTraitsFromPattern:);
  READ_FROM_PATTERN(NSFontSizeAttribute, readSizeFromPattern:);
  READ_FROM_PATTERN(NSFontCharacterSetAttribute, readCharacterSetFromPattern:);
}

- (NSDictionary*)attributesFromPattern: (FcPattern *)pat
{
  _attributes = [NSMutableDictionary dictionary];
  _pat = pat;
  [self parseAttributes];
  return _attributes;
}

@end



@implementation FontconfigCharacterSet

- (id)initWithFontconfigCharSet: (FcCharSet*)charset
{
  if ((self = [super init]))
    {
      _charset = FcCharSetCopy(charset);
    }
  return self;
}

- (id)mutableCopyWithZone: (NSZone*)aZone
{
  return [[NSMutableCharacterSet characterSetWithBitmapRepresentation: 
				   [self bitmapRepresentation]] retain];
}

- (void)dealloc
{
  FcCharSetDestroy(_charset);
  [super dealloc];
}

- (FcCharSet*)fontconfigCharSet
{
  return _charset;
}

- (BOOL)characterIsMember: (unichar)c
{
  return FcCharSetHasChar(_charset, c);
}

- (BOOL)longCharacterIsMember: (UTF32Char)c
{
  return FcCharSetHasChar(_charset, c);
}

// FIXME: Implement for better performance
//- (NSData *)bitmapRepresentation
//{
//}

@end

