/*
   XGFontInfo

   NSFont helper for GNUstep GUI X/GPS Backend

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Scott Christley
   Author: Ovidiu Predescu <ovidiu@bx.logicnet.ro>
   Date: February 1997
   Author:  Felipe A. Rodriguez <far@ix.netcom.com>
   Date: May, October 1998
   Author:  Michael Hanni <mhanni@sprintmail.com>
   Date: August 1998
   Author:  Fred Kiefer <fredkiefer@gmx.de>
   Date: September 2000

   This file is part of the GNUstep GUI X/GPS Backend.

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

#include <config.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "xlib/XGContext.h"
#include "xlib/XGPrivate.h"
#include <Foundation/NSDebug.h>
// For the encoding functions
#include <GNUstepBase/Unicode.h>

static Atom XA_SLANT = (Atom)0;
static Atom XA_SETWIDTH_NAME = (Atom)0;
static Atom XA_CHARSET_REGISTRY = (Atom)0;
static Atom XA_CHARSET_ENCODING = (Atom)0;
static Atom XA_SPACING = (Atom)0;
static Atom XA_PIXEL_SIZE = (Atom)0;
static Atom XA_WEIGHT_NAME = (Atom)0;

/*
  static Atom XA_RESOLUTION_X = (Atom)0;
  static Atom XA_RESOLUTION_Y = (Atom)0;
  static Atom XA_ADD_STYLE_NAME = (Atom)0;
  static Atom XA_AVERAGE_WIDTH = (Atom)0;
  static Atom XA_FACE_NAME = (Atom)0;
*/

/*
 * Initialise the X atoms we are going to use
 */
static BOOL XGInitAtoms(Display *dpy)
{
  // X atoms used to query a font

  if (!dpy)
    {
      NSDebugLog(@"No Display opened in XGInitAtoms");      
      return NO;
    }

  XA_PIXEL_SIZE = XInternAtom(dpy, "PIXEL_SIZE", False);
  XA_SPACING = XInternAtom(dpy, "SPACING", False);
  XA_WEIGHT_NAME = XInternAtom(dpy, "WEIGHT_NAME", False);
  XA_SLANT = XInternAtom(dpy, "SLANT", False);
  XA_SETWIDTH_NAME = XInternAtom(dpy, "SETWIDTH_NAME", False);
  XA_CHARSET_REGISTRY = XInternAtom(dpy, "CHARSET_REGISTRY", False);
  XA_CHARSET_ENCODING = XInternAtom(dpy, "CHARSET_ENCODING", False);

/*
  XA_ADD_STYLE_NAME = XInternAtom(dpy, "ADD_STYLE_NAME", False);
  XA_RESOLUTION_X = XInternAtom(dpy, "RESOLUTION_X", False);
  XA_RESOLUTION_Y = XInternAtom(dpy, "RESOLUTION_Y", False);
  XA_AVERAGE_WIDTH = XInternAtom(dpy, "AVERAGE_WIDTH", False);
  XA_FACE_NAME = XInternAtom(dpy, "FACE_NAME", False);
*/

  return YES;
}

/*
 * Return the name of the font cache file (used by font_cacher to write
 * the file and the backend to read the file
 */
extern NSString *XGFontCacheName(Display *dpy)
{
  NSString *dname;
  dname = [NSString stringWithCString: XDisplayName(NULL)];
  if ([dname hasPrefix: @"/tmp"])
    {
      /* This is the new Mac OS X display server information, not a real
         host name.  Use a more file-friendly name for the display */
      NSString *str = [dname lastPathComponent];
      dname = [dname stringByDeletingLastPathComponent];
      dname = [NSString stringWithFormat: @"%@%@", 
      	[dname lastPathComponent], str];
    }
  return dname;
}

/*
 * Read an X Font property of type unsigned long
 */
unsigned long XGFontPropULong(Display *dpy, XFontStruct *font_struct, 
			      Atom atom)
{
  unsigned long lvalue;
  
  if (XGetFontProperty(font_struct, atom, &lvalue))
    return lvalue;
  else
    return 0;
}

/*
 * Read an X Font property of type string
 */
NSString *XGFontPropString(Display *dpy, XFontStruct *font_struct, Atom atom)
{
  unsigned long	lvalue;
  char		*value;
  NSString	*ret = nil;

  if (XGetFontProperty(font_struct, atom, &lvalue) && dpy)
    {
      value = XGetAtomName(dpy, lvalue);
      if (value != NULL)
	{
	  // We convert all props to lowercase so comparing is easier
	  ret = [[NSString stringWithCString: value] lowercaseString];
	  XFree(value);
	}
    }
  
  return ret;
}

NSString *XGFontName(Display *dpy, XFontStruct *font_struct)
{
  return XGFontPropString(dpy, font_struct, XA_FONT);
}

float XGFontPointSize(Display *dpy, XFontStruct *font_struct)
{
  float size = 12.0;
  long pointSize;
  
  if (!XA_PIXEL_SIZE)
    XGInitAtoms(dpy);

  /*
   * We use pixel size here not point size, which is about 10 times its size.
   * Perhaps we will change that later on!
   */
  pointSize = XGFontPropULong(dpy, font_struct, XA_PIXEL_SIZE);
  if (pointSize != 0)
    {
      size = (float) pointSize;
    }

  return size;
}

BOOL XGFontIsFixedPitch(Display *dpy, XFontStruct *font_struct)
{
  /* Is this font fixed pitch? default, NO */
  BOOL fixedFont = NO;
  NSString *spacing;

  // If there is no information per character, all must be equal 
  if (!font_struct->per_char)
    {
	return YES;
    }

  if (!XA_SPACING)
    XGInitAtoms(dpy);

  /*
   * We could also check the value of MONOSPACED, but I have never seen it set.
   */
  spacing = XGFontPropString(dpy, font_struct, XA_SPACING);
  if (spacing != nil)
    {
      if ([spacing isEqualToString: @"m"])
	fixedFont = YES;
    }

  /*
   * We could calculate the pitch from the XLFD but that does not seem to be
   * saved. If we can't get the property, say no and cope.
   */
  return fixedFont;
}

NSString *XGFontFamily(Display *dpy, XFontStruct *font_struct)
{
  NSString *family;

  family = XGFontPropString(dpy, font_struct, XA_FAMILY_NAME);
  if (family == nil)
    return @"Unknown";	// FIXME: We should return the font name instead
  
  return [family capitalizedString];
}

// Get the weight of a X font
int XGWeightOfFont(Display *dpy, XFontStruct *info)
{
  int		w = 5;
  NSString	*string;

  if (!XA_WEIGHT_NAME)
    XGInitAtoms(dpy);

  string = XGFontPropString(dpy, info, XA_WEIGHT_NAME);

  if (string != nil)
    {
      w = [GSFontInfo weightForString: string];
    }

  return w;
}

// Get the traits of a X font
NSFontTraitMask XGTraitsOfFont(Display *dpy, XFontStruct *info)
{
  NSFontTraitMask	mask = 0;
  NSString		*string;
  int			w = XGWeightOfFont(dpy, info);
  
  if (w >= 9)
    mask |= NSBoldFontMask;
  else
    mask |= NSUnboldFontMask;
  
  if (XGFontIsFixedPitch(dpy, info))
    mask |= NSFixedPitchFontMask;
  
  string = XGFontPropString(dpy, info, XA_SLANT);
  if (string != nil)
    {
      char c = [string cString][0];
      
      if (c == 'o' || c == 'i')
	mask |= NSItalicFontMask;
      else
	mask |= NSUnitalicFontMask;
    }
  
  string = XGFontPropString(dpy, info, XA_CHARSET_REGISTRY);
  if (string != nil)
    {
      if (![string isEqualToString: @"iso8859"] &&
	  ![string isEqualToString: @"iso10646"])
	mask |= NSNonStandardCharacterSetFontMask;
    }
  string = XGFontPropString(dpy, info, XA_CHARSET_ENCODING);
  if (string != nil)
    {
      if (![string isEqualToString: @"1"])
	mask |= NSNonStandardCharacterSetFontMask;
    }
  
  string = XGFontPropString(dpy, info, XA_SETWIDTH_NAME);
  if (string != nil)
    {
      if ([string isEqualToString: @"narrow"])
	mask |= NSNarrowFontMask;
      else if ([string isEqualToString: @"semicondensed"])
	mask |= NSCondensedFontMask;
    }
    
  string = XGFontPropString(dpy, info, XA_SPACING);
  if (string != nil)
    {
      if ([string isEqualToString: @"c"])
	mask |= NSCompressedFontMask;
    }
  
  //FIXME: How can I find out about the other traits?
  /*
    unsigned long weight = XGFontPropULong(dpy, info, XA_WEIGHT);
  */

  return mask;
}
