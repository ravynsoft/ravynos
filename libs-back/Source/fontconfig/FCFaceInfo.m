/*
   FCFaceInfo.m
 
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

#include "fontconfig/FCFaceInfo.h"
#include "fontconfig/FCFontEnumerator.h"
#include <AppKit/NSFontManager.h>

@implementation FCFaceInfo 

- (id) initWithfamilyName: (NSString *)familyName 
                   weight: (int)weight 
                   traits: (unsigned int)traits 
                  pattern: (FcPattern *)pattern
{
  _pattern = pattern;
  FcPatternReference(_pattern);

  [self setFamilyName: familyName];
  [self setWeight: weight];
  [self setTraits: traits];

  return self;
}

- (void) dealloc
{
  FcPatternDestroy(_pattern);
  RELEASE(_familyName);
  RELEASE(_characterSet);
  [super dealloc];
}

- (void) setFamilyName: (NSString *)name
{
  ASSIGN(_familyName, name);
}

- (NSString *)familyName
{
  return _familyName;
}

- (NSString *) displayName
{
  char *fcname;

#ifdef FC_POSTSCRIPT_NAME
  char  *fcpsname;
#endif

  /*
    If fullname && fullname != psname, use fullname as displayname.
    This works around some weird Adobe OpenType fonts which contain their
    PostScript name in their "human-readable name" field.

    So, set the fullname as displayName (now AKA VisibleName) only
    if it's not the same as whatever the postscript name is.
  */
  if (FcPatternGetString(_pattern, FC_FULLNAME, 0, (FcChar8 **)&fcname) == FcResultMatch
#ifdef FC_POSTSCRIPT_NAME
      && FcPatternGetString(_pattern, FC_POSTSCRIPT_NAME, 0, (FcChar8 **)&fcpsname) == FcResultMatch
      && strcmp (fcpsname, fcname)
#endif
     )
    {
      return [NSString stringWithUTF8String: fcname];
    }
  else if (FcPatternGetString(_pattern, FC_STYLE, 0, (FcChar8 **)&fcname) == FcResultMatch)
    {
      return [NSString stringWithFormat: @"%@ %@", _familyName,
              [NSString stringWithUTF8String: fcname]];
    }
  return _familyName;
}

- (int) weight
{
  return _weight;
}

- (void) setWeight: (int)weight
{
  _weight = weight;
}

- (unsigned int) traits
{
  return _traits;
}

- (void) setTraits: (unsigned int)traits
{
  _traits = traits;
}

- (unsigned int) cacheSize
{
  return 257;
}

- (void *) fontFace
{
  [self subclassResponsibility: _cmd];
  return NULL;
}

- (FcPattern *) matchedPattern
{
  if (!_patternIsResolved)
    {
      FcResult result;
      FcPattern *resolved;

      FcConfigSubstitute(NULL, _pattern, FcMatchPattern); 
      FcDefaultSubstitute(_pattern);
      resolved = FcFontMatch(NULL, _pattern, &result);
      FcPatternDestroy(_pattern);
      _pattern = resolved;
      _patternIsResolved = YES;
    }

  // The caller expects ownership of returned pattern and will destroy it
  FcPatternReference(_pattern);
  return _pattern;
}

- (NSCharacterSet*) characterSet
{
  if (_characterSet == nil && !_hasNoCharacterSet)
    {
      FcPattern *resolved;
      FcCharSet *charset;
      
      resolved = [self matchedPattern];
      
      if (FcResultMatch == FcPatternGetCharSet(resolved, FC_CHARSET, 0, &charset))
	{
	  _characterSet = [[FontconfigCharacterSet alloc] initWithFontconfigCharSet: charset];
	}  
      
      /* Only try to get the character set once because FcFontMatch is expensive */
      if (_characterSet == nil)
	{
	  _hasNoCharacterSet = YES;
	}

      FcPatternDestroy(resolved);
    }
  return _characterSet;
}

@end
