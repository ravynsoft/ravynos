/* <title>XGFontSetFontInfo</title>

   <abstract>NSFont helper for GNUstep X/GPS Backend</abstract>

   Copyright (C) 2003-2005 Free Software Foundation, Inc.

   Author: Kazunobu Kuriyama <kazunobu.kuriyama@nifty.com>
   Date: July 2003
   
   This file is part of the GNU Objective C User Interface library.

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

#include "x11/XGServer.h"
#include "xlib/XGPrivate.h"
#include "xlib/XGFontSetFontInfo.h"

#ifdef X_HAVE_UTF8_STRING

#define XSERVER [XGServer xDisplay]

typedef struct _UTF8Str {
    char    *data;
    int	    size;
} UTF8Str;

#define UTF8StrData(x)	((x)->data)
#define UTF8StrSize(x)	((x)->size)
#define UTF8StrFree(x) \
do { \
  if ((x)->data) \
    { \
      free((x)->data); \
      (x)->data = NULL; \
      (x)->size = 0; \
    } \
} while (0)
#define UTF8StrAlloc(x, length) \
do { (x)->data = malloc(6 * (length)); } while (0)
#define UTF8StrUsable(x) ((x)->data != NULL)


// Forward declarations
static BOOL load_font_set(Display *dpy, const char *given_font_name,
			  XFontSet *font_set,
			  XFontStruct ***fonts, int *num_fonts);
static BOOL glyphs2utf8(const NSGlyph* glyphs, int length, UTF8Str* ustr);
static BOOL char_struct_for_glyph(NSGlyph glyph, XFontSet font_set,
				  XFontStruct **fonts, int num_fonts,
				  XCharStruct *cs);


#if 0 // Commented out till the implementation completes.
// ----------------------------------------------------------------------------
//  XGFontSetEnumerator
// ----------------------------------------------------------------------------
@implementation XGFontSetEnumerator : GSFontEnumerator

- (void) enumerateFontsAndFamilies
{ }

@end // XGFontSetEnumerator : GSFontEnumerator
#endif // #if 0


// ----------------------------------------------------------------------------
//  XGFontSetFontInfo
// ----------------------------------------------------------------------------
@implementation XGFontSetFontInfo : GSFontInfo

- (id) initWithFontName: (NSString *)name
		 matrix: (const CGFloat*)fmatrix
	     screenFont: (BOOL)screenFont
{
  Display	*dpy;
  XFontSet	font_set;
  XFontStruct	**fonts;
  XFontStruct	*base;
  int		num_fonts;

  if (screenFont)
    {
      RELEASE(self);
      return nil;
    }
  if (!name || [name length] == 0 || (dpy = XSERVER) == NULL)
    {
      RELEASE(self);
      return nil;
    }

  if (!load_font_set(dpy, [XGXFontName(name, fmatrix[0]) cString],
		     &font_set, &fonts, &num_fonts))
    {
      RELEASE(self);
      return nil;
    }
  base = fonts[0];

  // GSFontInfo part
  [super init];
  ASSIGN(fontName, name);
  ASSIGN(familyName, XGFontFamily(dpy, base));
  memcpy(matrix, fmatrix, sizeof(matrix));
  italicAngle		= 0;
  underlinePosition	= 0;
  underlineThickness	= 0;
  capHeight		= 0;
  xHeight		= 0;
  descender		= -base->descent;
  ascender		= base->ascent;
  maximumAdvancement	=
    NSMakeSize(base->max_bounds.width,
	       base->max_bounds.ascent + base->max_bounds.descent);
  minimumAdvancement	= NSMakeSize(0, 0);
  ASSIGN(encodingScheme, @"");
  mostCompatibleStringEncoding = NSASCIIStringEncoding;
  fontBBox		=
    NSMakeRect(base->min_bounds.lbearing,
	       -base->max_bounds.descent,
	       base->max_bounds.rbearing - base->max_bounds.lbearing,
	       base->max_bounds.ascent + base->max_bounds.descent);
  isFixedPitch		= XGFontIsFixedPitch(dpy, base);
  isBaseFont		= NO;
  weight		= XGWeightOfFont(dpy, base);
  traits		= XGTraitsOfFont(dpy, base);

  // XGFontSetFontInfo part
  _font_set	= font_set;
  _fonts	= fonts;
  _num_fonts	= num_fonts;

  return self;
}

- (void) dealloc
{
  if (_font_set)
    {
      XFreeFontSet(XSERVER, _font_set);
      _font_set = NULL;
    }
  [super dealloc];
}

- (NSSize) advancementForGlyph: (NSGlyph)glyph
{
  XCharStruct cs;

  if (!char_struct_for_glyph(glyph, _font_set, _fonts, _num_fonts, &cs))
    {
      cs.width = _fonts[0]->max_bounds.width;
    }
  return NSMakeSize((float)cs.width, 0);
}

- (NSRect) boundingRectForGlyph: (NSGlyph)glyph
{
  XCharStruct cs;

  if (!char_struct_for_glyph(glyph, _font_set, _fonts, _num_fonts, &cs))
    {
      return fontBBox;
    }
  return NSMakeRect((float)cs.lbearing, (float)-cs.descent,
		    (float)(cs.rbearing - cs.lbearing),
		    (float)(cs.ascent + cs.descent));
}

- (BOOL) glyphIsEncoded: (NSGlyph)glyph
{
  XCharStruct cs;

  return char_struct_for_glyph(glyph, _font_set, _fonts, _num_fonts, &cs);
}

- (NSGlyph) glyphWithName: (NSString *)glyphName
{
  KeySym k;

  k = XStringToKeysym([glyphName cString]);
  if (k == NoSymbol)
    return 0;
  else
    return (NSGlyph)k;
}

- (void) drawGlyphs: (const NSGlyph *)glyphs
             length: (int)len
          onDisplay: (Display *)dpy
	   drawable: (Drawable)win
	       with: (GC)gc
	         at: (XPoint)xp
{
  UTF8Str ustr;

  if (glyphs2utf8(glyphs, len, &ustr))
    {
      Xutf8DrawString(dpy, win, _font_set, gc, xp.x, xp.y,
		      UTF8StrData(&ustr), UTF8StrSize(&ustr));
      UTF8StrFree(&ustr);
    }
}

- (CGFloat) widthOfGlyphs: (const NSGlyph *)glyphs
                 length: (int)len
{
  UTF8Str   ustr;
  float	    val;

  if (glyphs2utf8(glyphs, len, &ustr))
    {
      XRectangle logical;

      Xutf8TextExtents(_font_set, UTF8StrData(&ustr), UTF8StrSize(&ustr),
		       NULL, &logical);
      UTF8StrFree(&ustr);
      val = logical.width;
    }
  else
    {
      val = 0.0;
    }
  return val;
}

- (void) setActiveFor: (Display *)dpy
                   gc: (GC)gc
{
  // Do nothing.
}

@end // XGFontSetFontInfo : GSFontInfo


// ----------------------------------------------------------------------------
//  Static Functions
// ----------------------------------------------------------------------------
static BOOL
load_font_set(Display *dpy, const char *given_font_name,
	      XFontSet *font_set, XFontStruct ***fonts, int *num_fonts)
{
  int	i;
  char	xlfd[256];
#ifndef STATIC_BUFFER_IS_RELIABLE
  char	*p;
#endif
  int	xlfd_num_elms;
  BOOL	has_add_style;
  char	*xlfd_elms[14];
  char	base_font_name[256];

  char	**missing_charsets;
  int	num_missing_charsets;
  char	*def_string;

  char		**font_names;
  int		num_font_names;
  XFontStruct	**font_structs;

  if (!dpy || !given_font_name)
    {
      return NO;
    }

  strcpy(xlfd, given_font_name);
  xlfd_num_elms = 14;
  has_add_style = YES;
  // Both of the following compilation branches, based on the switch
  // 'STATIC_BUFFER_IS_RELIABLE', basically do the same thing.  Because some
  // people worry about making use of strtok(),  the latter branch is used
  // primarily unless the switch is explicitly defined somewhere.  The former
  // branch should document what the latter one does.
#ifdef STATIC_BUFFER_IS_RELIABLE
  if (strstr(xlfd, "--"))
    {
      --xlfd_num_elms;
      has_add_style = NO;
    }

  i = 0;
  xlfd_elms[i++] = strtok(xlfd, "-");
  while (i < xlfd_num_elms)
    {
      xlfd_elms[i++] = strtok(NULL, "-");
    }
#else // 'STATIC_BUFFER_IS_RELIABLE' not defined
  i = 0;
  p = xlfd;
  do
    {
      while (*p != '-')
	++p;
      *p = '\0';
      if (*++p == '-')	// The token is in the form of '--'.
	{
	  *p++ = '\0';
	  --xlfd_num_elms;
	  has_add_style = NO;
	}
      xlfd_elms[i] = p;
    }
  while (++i < xlfd_num_elms && *p != '\0');
#endif // 'STATIC_BUFFER_IS_RELIABLE' not defined

  // To let the X server determine a font set automatically, some elements
  // of the XLFD should be replaced with the wild card.
  //
  // N.B. Rigorously speaking, to make use of proportional fonts, we need to
  // define a mapping from NSGlyph to the XFontStruct's field 'per_char'.
  // The property 'spacing' is set to "*" until such a mapping is given.
  // (see also char_struct_for_glyph()).
  if (has_add_style)
    {
      sprintf(base_font_name,
	      "-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s",
	      xlfd_elms[0],	// foundry
	      "*",		// family
	      xlfd_elms[2],	// weight
	      xlfd_elms[3],	// slant
	      xlfd_elms[4],	// set width
	      xlfd_elms[5],	// add style
	      xlfd_elms[6],	// pixel size
	      xlfd_elms[7],	// point size
	      xlfd_elms[8],	// resolutionX
	      xlfd_elms[9],	// resolutionY
	      "*",		// spacing
	      xlfd_elms[11],	// avg width
	      "*",		// registry
	      "*"		// encoding
	);
    }
  else
    {
      sprintf(base_font_name,
	      "-%s-%s-%s-%s-%s--%s-%s-%s-%s-%s-%s-%s-%s",
	      xlfd_elms[0],	// foundry
	      "*",		// family
	      xlfd_elms[2],	// weight
	      xlfd_elms[3],	// slant
	      xlfd_elms[4],	// set width
	      xlfd_elms[5],	// pixel size
	      xlfd_elms[6],	// point size
	      xlfd_elms[7],	// resolutionX
	      xlfd_elms[8],	// resolutionY
	      "*",		// spacing
	      xlfd_elms[10],	// avg width
	      "*",		// registry
	      "*"		// encoding
);
    }

  // N.B. def_string is owned by the X server: Don't release it.
  missing_charsets	= NULL;
  num_missing_charsets	= 0;
  def_string		= NULL;
  *font_set		= NULL;
  *font_set = XCreateFontSet(dpy, base_font_name,
			     &missing_charsets,
			     &num_missing_charsets,
			     &def_string);
  if (!*font_set)
    {
      NSLog(@"XGFontSetFontInfo: Can't create a font set\n");
      return NO;
    }
  if (num_missing_charsets > 0)
    {
      for (i = 0; i < num_missing_charsets; ++i)
	{
	  NSLog(@"XGFontSetFontInfo: Charset %s is not available\n",
		missing_charsets[i]);
	}
      XFreeStringList(missing_charsets);
      missing_charsets = NULL;
      num_missing_charsets = 0;
    }

  // N.B. font_structs and font_names are owned by the X server: Don't
  // release them.
  num_font_names    = 0;
  font_structs	    = NULL;
  font_names	    = NULL;
  num_font_names = XFontsOfFontSet(*font_set, &font_structs, &font_names);
  if (!num_font_names)
    {
      NSLog(@"XGFontSetFontInfo: "
	    @"Can't get any information from the font set\n");
      return NO;
    }

  *fonts = font_structs;
  *num_fonts = num_font_names;

  return YES;
}

// N.B. Use UTF8StrFree() to release the space pointed to by 'ustr'.
static BOOL
glyphs2utf8(const NSGlyph* glyphs, int length, UTF8Str* ustr)
{
  int	    i;
  NSGlyph   *g;
  NSGlyph   *end;
  char	    *p;

  if (!glyphs || !length)
    {
      return NO;
    }

  UTF8StrAlloc(ustr, length);
  if (!UTF8StrUsable(ustr))
    {
      return NO;
    }

  p = UTF8StrData(ustr);
  i = 0;
  for (g = (NSGlyph *)glyphs, end = (NSGlyph *)glyphs + length; g < end; ++g)
    {
      if (*g < 0x00000080)
	{
	  p[i++] = *g;
	}
      else if (*g < 0x00000800)
	{
	  p[i++] = 0xc0 | ((*g >>  6) & 0x1f);
	  p[i++] = 0x80 | (*g        & 0x3f);
	}
      else if (*g < 0x00010000)
	{
	  p[i++] = 0xe0 | ((*g >> 12) & 0x0f);
	  p[i++] = 0x80 | ((*g >>  6) & 0x3f);
	  p[i++] = 0x80 | (*g        & 0x3f);
	}
      else if (*g < 0x00200000)
	{
	  p[i++] = 0xf0 | ((*g >> 18) & 0x07);
	  p[i++] = 0x80 | ((*g >> 12) & 0x3f);
	  p[i++] = 0x80 | ((*g >>  6) & 0x3f);
	  p[i++] = 0x80 | (*g        & 0x3f);
	}
      else if (*g < 0x04000000)
	{
	  p[i++] = 0xf8 | ((*g >> 24) & 0x03);
	  p[i++] = 0x80 | ((*g >> 18) & 0x3f);
	  p[i++] = 0x80 | ((*g >> 12) & 0x3f);
	  p[i++] = 0x80 | ((*g >>  6) & 0x3f);
	  p[i++] = 0x80 | (*g        & 0x3f);
	}
      else if (*g < 0x80000000)
	{
	  p[i++] = 0xfc | ((*g >> 30) & 0x01);
	  p[i++] = 0x80 | ((*g >> 24) & 0x3f);
	  p[i++] = 0x80 | ((*g >> 18) & 0x3f);
	  p[i++] = 0x80 | ((*g >> 12) & 0x3f);
	  p[i++] = 0x80 | ((*g >>  6) & 0x3f);
	  p[i++] = 0x80 | (*g        & 0x3f);
	}
      else
	{
	  // Out of range
	  UTF8StrFree(ustr);
	  return NO;
	}
    }
  UTF8StrSize(ustr) = i;

  return YES;
}

static BOOL
char_struct_for_glyph(NSGlyph glyph, XFontSet font_set,
		      XFontStruct **fonts, int num_fonts,
		      XCharStruct *cs)
{
  UTF8Str utf8char;

  if (glyphs2utf8(&glyph, 1, &utf8char))
    {
      XRectangle    ink, logical;
      int	    num_chars;

      Xutf8TextPerCharExtents(font_set,
			      UTF8StrData(&utf8char), UTF8StrSize(&utf8char),
			      &ink, &logical, 1, &num_chars, NULL, NULL);
      UTF8StrFree(&utf8char);

      if (num_chars != 1)
	{
	  return NO;
	}

      // When the font in use is proportional, the following variables should
      // be tuned finer based on the the XFontStruct's field 'per_char'.
      cs->lbearing	= 0;
      cs->rbearing	= 0;
      cs->width		= logical.width;
      cs->ascent	= fonts[0]->max_bounds.ascent;
      cs->descent	= fonts[0]->max_bounds.descent;
      cs->attributes	= 0;

      return YES;
    }
  else
    {
      return NO;
    }
}

#endif // X_HAVE_UTF8_STRING defined
