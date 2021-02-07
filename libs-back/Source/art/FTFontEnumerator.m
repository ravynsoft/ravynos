/*
   Copyright (C) 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   Author:  Alexander Malmberg <alexander@malmberg.org>

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

#include <Foundation/NSArray.h>
#include <Foundation/NSBundle.h>
#include <Foundation/NSDebug.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSFileManager.h>
#include <Foundation/NSObject.h>
#include <Foundation/NSPathUtilities.h>
#include <Foundation/NSSet.h>
#include <Foundation/NSUserDefaults.h>
#include <Foundation/NSValue.h>

#include "FTFontEnumerator.h"
#include "FTFaceInfo.h"

#if 0

/*
This is a list of "standard" face names. It is here so make_strings can pick
it up and generate .strings files with them.
*/

NSLocalizedStringFromTable(@"Book", @"nfontFaceNames", @"")
NSLocalizedStringFromTable(@"Regular", @"nfontFaceNames", @"")
NSLocalizedStringFromTable(@"Roman", @"nfontFaceNames", @"")
NSLocalizedStringFromTable(@"Medium", @"nfontFaceNames", @"")
NSLocalizedStringFromTable(@"Demi", @"nfontFaceNames", @"")
NSLocalizedStringFromTable(@"Demibold", @"nfontFaceNames", @"")
NSLocalizedStringFromTable(@"Bold", @"nfontFaceNames", @"")
NSLocalizedStringFromTable(@"Heavy", @"nfontFaceNames", @"")

NSLocalizedStringFromTable(@"Italic", @"nfontFaceNames", @"")
NSLocalizedStringFromTable(@"Oblique", @"nfontFaceNames", @"")

NSLocalizedStringFromTable(@"Bold Italic", @"nfontFaceNames", @"")
NSLocalizedStringFromTable(@"Bold Oblique", @"nfontFaceNames", @"")

#endif

static NSMutableArray *fcfg_allFontNames;
static NSMutableDictionary *fcfg_allFontFamilies;
static NSMutableDictionary *fcfg_all_fonts;
static NSMutableSet *families_seen, *families_pending;

static BOOL anti_alias_by_default;



static int traits_from_string(NSString *s, unsigned int *traits, unsigned int *weight)
{
static struct
{
  NSString *str;
  unsigned int trait;
  int weight;
} suffix[] = {
/* TODO */
{@"Normal"         ,0                         ,-1},

{@"Ultralight"     ,0                         , 1},
{@"Thin"           ,0                         , 2},
{@"Light"          ,0                         , 3},
{@"Extralight"     ,0                         , 3},
{@"Book"           ,0                         , 4},
{@"Regular"        ,0                         , 5},
{@"Plain"          ,0                         , 5},
{@"Display"        ,0                         , 5},
{@"Roman"          ,0                         , 5},
{@"Semilight"      ,0                         , 5},
{@"Medium"         ,0                         , 6},
{@"Demi"           ,0                         , 7},
{@"Demibold"       ,0                         , 7},
{@"Semi"           ,0                         , 8},
{@"Semibold"       ,0                         , 8},
{@"Bold"           ,NSBoldFontMask            , 9},
{@"Extra"          ,NSBoldFontMask            ,10},
{@"Extrabold"      ,NSBoldFontMask            ,10},
{@"Heavy"          ,NSBoldFontMask            ,11},
{@"Heavyface"      ,NSBoldFontMask            ,11},
{@"Ultrabold"      ,NSBoldFontMask            ,12},
{@"Black"          ,NSBoldFontMask            ,12},
{@"Ultra"          ,NSBoldFontMask            ,13},
{@"Ultrablack"     ,NSBoldFontMask            ,13},
{@"Fat"            ,NSBoldFontMask            ,13},
{@"Extrablack"     ,NSBoldFontMask            ,14},
{@"Obese"          ,NSBoldFontMask            ,14},
{@"Nord"           ,NSBoldFontMask            ,14},

{@"Italic"         ,NSItalicFontMask          ,-1},
{@"Oblique"        ,NSItalicFontMask          ,-1},

{@"Cond"           ,NSCondensedFontMask       ,-1},
{@"Condensed"      ,NSCondensedFontMask       ,-1},
{nil,0,-1}
};
  int i;

  *traits = 0;
//  printf("do '%@'\n", s);
  while ([s length] > 0)
    {
//      printf("  got '%@'\n", s);
      if ([s hasSuffix: @"-"] || [s hasSuffix: @" "])
	{
//	  printf("  do -\n");
	  s = [s substringToIndex: [s length] - 1];
	  continue;
	}
      for (i = 0; suffix[i].str; i++)
	{
	  if (![s hasSuffix: suffix[i].str])
	    continue;
//	  printf("  found '%@'\n", suffix[i].str);
	  if (suffix[i].weight != -1)
	    *weight = suffix[i].weight;
	  (*traits) |= suffix[i].trait;
	  s = [s substringToIndex: [s length] - [suffix[i].str length]];
	  break;
	}
      if (!suffix[i].str)
	break;
    }
//  printf("end up with '%@'\n", s);
  return [s length];
}


static NSArray *fix_path(NSString *path, NSArray *files)
{
  int i, c = [files count];
  NSMutableArray *nfiles;

  if (!files)
    return nil;

  nfiles = [[NSMutableArray alloc] init];
  for (i = 0; i < c; i++)
    {
      if ([[files objectAtIndex: i] isAbsolutePath])
        [nfiles addObject: [files objectAtIndex: i]];
      else
        [nfiles addObject: [path stringByAppendingPathComponent:
	  [files objectAtIndex: i]]];
    }
  return nfiles;
}

/* TODO: handling of .font packages needs to be reworked */
static void add_face(NSString *family, int family_weight,
	unsigned int family_traits, NSDictionary *d, NSString *path,
	BOOL from_nfont)
{
  FTFaceInfo *fi;
  unsigned int weight;
  unsigned int traits;

  NSString *fontName;
  NSString *faceName, *rawFaceName;


  fontName = [d objectForKey: @"PostScriptName"];
  if (!fontName)
    {
      NSLog(@"Warning: Face in %@ has no PostScriptName!",path);
      return;
    }

  if ([fcfg_allFontNames containsObject: fontName])
    return;

  fi = [[FTFaceInfo alloc] init];
  fi->familyName = [family copy];

  if ([d objectForKey: @"LocalizedNames"])
    {
      NSDictionary *l;
      NSArray *lang;
      int i;

      l = [d objectForKey: @"LocalizedNames"];
      lang = [[NSUserDefaults standardUserDefaults]
	stringArrayForKey: @"NSLanguages"];
      faceName = nil;
      rawFaceName = [l objectForKey: @"English"];
      for (i = 0; i < [lang count] && !faceName; i++)
        {
          faceName = [l objectForKey: [lang objectAtIndex: i]];
        }
      if (!faceName)
        faceName = rawFaceName;
      if (!faceName)
        {
          faceName = @"<unknown face>";
          NSLog(@"Warning: couldn't find localized face name or fallback for %@",
                fontName);
        }
    }
  else if ((faceName = [d objectForKey: @"Name"]))
    {
      rawFaceName = faceName;
      /* TODO: Smarter localization? Parse space separated parts and
      translate individually? */
      /* TODO: Need to define the strings somewhere, and make sure the
      strings files get created.  */
      faceName = [NSLocalizedStringFromTableInBundle(faceName,@"nfontFaceNames",
			[NSBundle bundleForClass: [fi class]],nil) copy];
      fi->faceName = faceName;
    }
  else if (!from_nfont)
    { /* try to guess something for .font packages */
      unsigned int dummy;
      int split = traits_from_string(family,&dummy,&dummy);
      rawFaceName = faceName = [family substringFromIndex: split];
      family = [family substringToIndex: split];
      faceName = [NSLocalizedStringFromTableInBundle(faceName,@"nfontFaceNames",
			[NSBundle bundleForClass: [fi class]],nil) copy];
      fi->faceName = faceName;
    }
  else
    {
      NSLog(@"Warning: Can't find name for face %@ in %@!",fontName,path);
      return;
    }

  fi->displayName = [[NSString stringWithFormat: @"%@ %@", family, faceName] retain];


  weight = family_weight;
  if (rawFaceName)
    traits_from_string(rawFaceName, &traits, &weight);

  {
    NSDictionary *sizes;
    NSEnumerator *e;
    NSString *size;
    int i;

    sizes = [d objectForKey: @"ScreenFonts"];

    fi->num_sizes = [sizes count];
    if (fi->num_sizes)
      {
	fi->sizes = malloc(sizeof(fi->sizes[0])*[sizes count]);
	e = [sizes keyEnumerator];
	i = 0;
	while ((size = [e nextObject]))
	  {
	    fi->sizes[i].pixel_size = [size intValue];
	    fi->sizes[i].files = fix_path(path,[sizes objectForKey: size]);
	    NSDebugLLog(@"ftfont",@"%@ size %i files |%@|",
	      fontName,fi->sizes[i].pixel_size,fi->sizes[i].files);
	    i++;
          }
      }
  }

  fi->files = fix_path(path,[d objectForKey: @"Files"]);

  if ([d objectForKey: @"Weight"])
    weight = [[d objectForKey: @"Weight"] intValue];
  fi->weight = weight;

  if ([d objectForKey: @"Traits"])
    traits = [[d objectForKey: @"Traits"] intValue];
  traits |= family_traits;
  fi->traits = traits;

  if ([d objectForKey: @"RenderHints_hack"])
    fi->render_hints_hack
      = strtol([[d objectForKey: @"RenderHints_hack"] cString], NULL, 0);
  else
    {
      if (anti_alias_by_default)
        fi->render_hints_hack = 0x10202;
      else
        fi->render_hints_hack = 0x00202;
    }

  NSDebugLLog(@"ftfont", @"adding '%@' '%@'", fontName, fi);

  [fcfg_all_fonts setObject: fi forKey: fontName];
  [fcfg_allFontNames addObject: fontName];

    {
      NSArray *a;
      NSMutableArray *ma;
      a = [NSArray arrayWithObjects:
	fontName,
	faceName,
	[NSNumber numberWithInt: weight],
	[NSNumber numberWithUnsignedInt: traits],
	nil];
      ma = [fcfg_allFontFamilies objectForKey: family];
      if (!ma)
	{
	  ma = [[NSMutableArray alloc] init];
	  [fcfg_allFontFamilies setObject: ma forKey: family];
	  [ma release];
	}
      [ma addObject: a];
    }

  DESTROY(fi);
}

static void load_font_configuration(void)
{
  int i, j, k, c;
  NSArray *paths;
  NSString *path, *font_path;
  NSFileManager *fm = [NSFileManager defaultManager];
  NSArray *files;
  NSDictionary *d;
  NSArray *faces;
  NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];

  if ([ud objectForKey: @"GSFontAntiAlias"])
    anti_alias_by_default = [ud boolForKey: @"GSFontAntiAlias"];
  else
    anti_alias_by_default = YES;

  fcfg_all_fonts = [[NSMutableDictionary alloc] init];
  fcfg_allFontFamilies = [[NSMutableDictionary alloc] init];
  fcfg_allFontNames = [[NSMutableArray alloc] init];

  families_seen = [[NSMutableSet alloc] init];
  families_pending = [[NSMutableSet alloc] init];

  paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSAllDomainsMask, YES);
  for (i = 0; i < [paths count]; i++)
    {
      path = [paths objectAtIndex: i];
      path = [path stringByAppendingPathComponent: @"Fonts"];
      files = [fm directoryContentsAtPath: path];
      c = [files count];

      for (j = 0; j < c; j++)
	{
	  NSString *family;
	  NSDictionary *face_info;
	  NSString *font_info_path;

	  int weight;
	  unsigned int traits;

	  font_path = [files objectAtIndex: j];
	  if (![[font_path pathExtension] isEqual: @"nfont"])
	    continue;

	  family = [font_path stringByDeletingPathExtension];

	  if ([families_seen member: family])
	    {
	      NSDebugLLog(@"ftfont", @"'%@' already seen, skipping", family);
	      continue;
	    }
	  [families_seen addObject: family];

	  font_path = [path stringByAppendingPathComponent: font_path];

	  NSDebugLLog(@"ftfont",@"loading %@",font_path);

	  font_info_path = [font_path stringByAppendingPathComponent: @"FontInfo.plist"];
	  if (![fm fileExistsAtPath: font_info_path])
	    continue;
	  d = [NSDictionary dictionaryWithContentsOfFile: font_info_path];
	  if (!d)
	    continue;

	  if ([d objectForKey: @"Family"])
	    family = [d objectForKey: @"Family"];

	  if ([d objectForKey: @"Weight"])
	    weight = [[d objectForKey: @"Weight"] intValue];
	  else
	    weight = 5;

	  if ([d objectForKey: @"Traits"])
	    traits = [[d objectForKey: @"Traits"] intValue];
	  else
	    traits = 0;

	  faces = [d objectForKey: @"Faces"];
	  if (![faces isKindOfClass: [NSArray class]])
	    {
	      NSLog(@"Warning: %@ isn't a valid .nfont package, ignoring.",
 	        font_path);
	      if ([faces isKindOfClass: [NSDictionary class]])
	        NSLog(@"(it looks like an old-style .nfont package)");
	      continue;
	    }

	  for (k = 0; k < [faces count]; k++)
	    {
	      face_info = [faces objectAtIndex: k];
	      add_face(family, weight, traits, face_info, font_path, YES);
	    }
	}

      for (j = 0; j < c; j++)
        {
          NSString *family;

          font_path = [files objectAtIndex: j];
          if (![[font_path pathExtension] isEqual: @"font"])
            continue;

          family = [font_path stringByDeletingPathExtension];
          font_path = [path stringByAppendingPathComponent: font_path];
          d = [NSDictionary dictionaryWithObjectsAndKeys:
                [NSArray arrayWithObjects:
                  family,
                  [family stringByAppendingPathExtension: @"afm"],
                  nil],
                @"Files",
                family,@"PostScriptName",
                nil];
          add_face(family, 5, 0, d, font_path, NO);
        }
      [families_seen unionSet: families_pending];
      [families_pending removeAllObjects];
    }

  NSDebugLLog(@"ftfont", @"got %lu fonts in %lu families",
    [fcfg_allFontNames count], [fcfg_allFontFamilies count]);

  if (![fcfg_allFontNames count])
    {
      NSLog(@"No fonts found!");
      exit(1);
    }

  DESTROY(families_seen);
  DESTROY(families_pending);
}

@implementation FTFontEnumerator

+ (FTFaceInfo *) fontWithName: (NSString *)name
{
  FTFaceInfo *face;

  face = [fcfg_all_fonts objectForKey: name];
  if (!face)
    {
      NSLog (@"Font not found %@", name);
    }
  return face;
}

- (void) enumerateFontsAndFamilies
{
  load_font_configuration();

  ASSIGN(allFontNames, fcfg_allFontNames);
  ASSIGN(allFontFamilies, fcfg_allFontFamilies);
}

- (NSString *) defaultSystemFontName
{
  if ([fcfg_allFontNames containsObject: @"BitstreamVeraSans-Roman"])
    return @"BitstreamVeraSans-Roman";
  if ([fcfg_allFontNames containsObject: @"FreeSans"])
    return @"FreeSans";
  return @"Helvetica";
}

- (NSString *) defaultBoldSystemFontName
{
  if ([fcfg_allFontNames containsObject: @"BitstreamVeraSans-Bold"])
    return @"BitstreamVeraSans-Bold";
  if ([fcfg_allFontNames containsObject: @"FreeSansBold"])
    return @"FreeSansBold";
  return @"Helvetica-Bold";
}

- (NSString *) defaultFixedPitchFontName
{
  if ([fcfg_allFontNames containsObject: @"BitstreamVeraSansMono-Roman"])
    return @"BitstreamVeraSansMono-Roman";
  if ([fcfg_allFontNames containsObject: @"FreeMono"])
    return @"FreeMono";
  return @"Courier";
}

@end
