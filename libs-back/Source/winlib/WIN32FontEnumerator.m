/* Win32FontEnumerator - Implements font enumerator for MSWindows

   Copyright (C) 2002 Free Software Foundation, Inc.

   Written by: Fred Kiefer <FredKiefer@gmx.de>
   Date: March 2002
   
   This file is part of the GNU Objective C User Interface Library.

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
#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSDebug.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSValue.h>

#include "winlib/WIN32FontEnumerator.h"
#include "windows.h"

@implementation WIN32FontEnumerator

int win32_font_weight(LONG tmWeight)
{
  int weight;

  // The MS names are a bit different from the NS ones!
  switch (tmWeight)
    {
      case FW_THIN:
	weight = 1;
	break;
      case FW_EXTRALIGHT:
	weight = 2;
	break;
      case FW_LIGHT:
	weight = 3;
	break;
      case FW_REGULAR:
	weight = 5;
	break;
      case FW_MEDIUM:
	weight = 6;
	break;
      case FW_DEMIBOLD:
	weight = 7;
	break;
      case FW_BOLD:
	weight = 9;
	break;
      case FW_EXTRABOLD:
	weight = 10;
	break;
      case FW_BLACK:
	weight = 12;
	break;
    default:
	// Try to map the range 0 to 1000 into 1 to 14.
	weight = (int)(tmWeight * 14 / 1000);
	break;
    }

  return weight;
}

// FIXME
NSString *fontStyles[] = {@" Italic", @" Oblique", @" Bold", 
			  @" BoldItalic", @" Demibold", 
			  @" Normal",  @" Kursiv", @" Fett"};

NSString *win32_font_family(NSString *fontName)
{
  NSString *fontFamily;
  int i;
  int max = sizeof(fontStyles) / sizeof(NSString*);

  fontFamily = fontName;
  for (i = 0; i < max; i++)
    {
      if ([fontFamily hasSuffix: fontStyles[i]])
	{
	  fontFamily = [fontFamily substringToIndex: 
				     ([fontFamily length] - 
				      [fontStyles[i] length])];
	}
    }

  //NSLog(@"Font Family %@ for %@", fontFamily, fontName);
  return fontFamily;
}

static 
void add_font(NSMutableArray *fontDefs, NSString *fontName, 
	      ENUMLOGFONTEXW *lpelfe, NEWTEXTMETRICEXW *lpntme)
{
  NSArray *fontDef;
  NSString *fontStyle;
  NSFontTraitMask traits = 0;
  int weight;
  
  weight = win32_font_weight(lpntme->ntmTm.tmWeight);
  if (weight >= 9)
    traits |= NSBoldFontMask;
  else
    traits |= NSUnboldFontMask;
  
  if (lpntme->ntmTm.tmItalic)
    traits |= NSItalicFontMask;
  else
    traits |= NSUnitalicFontMask;
  
  fontStyle = [NSString stringWithCharacters: lpelfe->elfStyle
				      length: wcslen(lpelfe->elfStyle)];
  fontDef = [NSArray arrayWithObjects:
    fontName, 
    fontStyle, 
    [NSNumber numberWithInt: weight],
    [NSNumber numberWithUnsignedInt: traits],
    nil];
  [fontDefs addObject: fontDef];
}

int CALLBACK fontenum(ENUMLOGFONTEXW *lpelfe, NEWTEXTMETRICEXW *lpntme,
		      DWORD FontType, LPARAM lParam)
{
  NSString *fontName;

  fontName = [NSString stringWithCharacters: lpelfe->elfFullName
				     length: wcslen(lpelfe->elfFullName)];
  NSDebugLLog(@"NSFont", @"Found font %@", fontName);
  add_font((NSMutableArray *)lParam, fontName, lpelfe, lpntme);
  return 1;
}

static
void enumerate_font(NSMutableArray *fontDefs, NSString *fontFamily)
{
  HDC hdc;
  LOGFONTW logfont;
  int res;
  CREATE_AUTORELEASE_POOL(pool);

  NSDebugLLog(@"NSFont", @"Enumerate font family %@", fontFamily);
  hdc = GetDC(NULL);
  logfont.lfCharSet = DEFAULT_CHARSET;
  wcsncpy(logfont.lfFaceName,
    (const unichar*)[fontFamily cStringUsingEncoding: NSUnicodeStringEncoding],
    LF_FACESIZE);
  logfont.lfPitchAndFamily = 0;
  res = EnumFontFamiliesExW(hdc, &logfont, (FONTENUMPROCW)fontenum, 
			   (LPARAM)fontDefs, 0);
  ReleaseDC(NULL, hdc);
  RELEASE(pool);
}

int CALLBACK fontfamilyenum(ENUMLOGFONTEXW *lpelfe, NEWTEXTMETRICEXW *lpntme,
			    DWORD FontType, LPARAM lParam)
{
  NSString *fontName;
  NSString *familyName;
  NSMutableArray *fontDefs;
  WIN32FontEnumerator *enumer = (WIN32FontEnumerator*)lParam;

  fontName = [NSString stringWithCharacters: lpelfe->elfFullName
				     length: wcslen(lpelfe->elfFullName)];

  familyName = win32_font_family(fontName);
  fontDefs = [enumer->allFontFamilies objectForKey: familyName];
  if (fontDefs == nil)
    {
      NSArray *fontDef;

      fontDefs = [NSMutableArray arrayWithCapacity: 10];
      [enumer->allFontFamilies setObject: fontDefs forKey: familyName];
      // FIXME: Need to loop over all fonts for this family
      //add_font(fontDefs, fontName, lpelfe, lpntme);
      //enumerate_font(fontDefs, familyName);
      fontDef = [NSArray arrayWithObjects:
	familyName, 
	@"Normal", 
	[NSNumber numberWithInt: 6],
	[NSNumber numberWithUnsignedInt: 0],
	nil];
      [fontDefs addObject: fontDef];
      fontDef = [NSArray arrayWithObjects:
	[familyName stringByAppendingString: @" Bold"], 
	@"Bold", 
	[NSNumber numberWithInt: 9],
	[NSNumber numberWithUnsignedInt: NSBoldFontMask],
	nil];
      [fontDefs addObject: fontDef];
      fontDef = [NSArray arrayWithObjects:
	[familyName stringByAppendingString: @" Italic"], 
	@"Italic", 
	[NSNumber numberWithInt: 6],
	[NSNumber numberWithUnsignedInt: NSItalicFontMask],
	nil];
      [fontDefs addObject: fontDef];
      fontDef = [NSArray arrayWithObjects:
	[familyName stringByAppendingString: @" Bold Italic"], 
	@"Bold Italic", 
	[NSNumber numberWithInt: 9],
	[NSNumber numberWithUnsignedInt: NSBoldFontMask | NSItalicFontMask],
	nil];
      [fontDefs addObject: fontDef];

      [(NSMutableArray*)(enumer->allFontNames) addObject: fontName];
    }

  return 1;
}

- (void) enumerateFontsAndFamilies
{
  static BOOL done = NO;

  if (!done)
    {
      HDC hdc;
      LOGFONTW logfont;
      int res;
      CREATE_AUTORELEASE_POOL(pool);

      allFontFamilies = [[NSMutableDictionary alloc] init];
      allFontNames  = [[NSMutableArray alloc] init];
	
      hdc = GetDC(NULL);
      logfont.lfCharSet = DEFAULT_CHARSET;
      logfont.lfFaceName[0] = '\0';
      logfont.lfPitchAndFamily = 0;
      // This get ignored
      logfont.lfItalic = 0;
      logfont.lfWeight = FW_NORMAL;
      res = EnumFontFamiliesExW(hdc, &logfont, (FONTENUMPROCW)fontfamilyenum, 
			       (LPARAM)self, 0);

      ReleaseDC(NULL, hdc);
      RELEASE(pool);
      done = YES;
    }
}

- (NSString*) defaultSystemFontName
{
  return @"Tahoma";
}

- (NSString*) defaultBoldSystemFontName
{
  return @"Tahoma Bold";
}

- (NSString*) defaultFixedPitchFontName
{
  return @"Courier New";
}

@end
