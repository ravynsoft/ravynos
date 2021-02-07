/*
   FCFontEnumerator.h

   Copyright (C) 2003 Free Software Foundation, Inc.
   August 31, 2003
   Written by Banlu Kemiyatorn <object at gmail dot com>

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


#ifndef FCFontEnumerator_h
#define FCFontEnumerator_h

#include <GNUstepGUI/GSFontInfo.h>
#define id fontconfig_id
#include <fontconfig/fontconfig.h>
#undef id
#include "fontconfig/FCFaceInfo.h"

@interface FCFontEnumerator : GSFontEnumerator
{
}
+ (Class) faceInfoClass;
+ (FCFaceInfo *) fontWithName: (NSString *)name;
@end

@interface FontconfigPatternGenerator : NSObject
{
  NSDictionary *_attributes;
  FcPattern *_pat;
}
- (FcPattern *)createPatternWithAttributes: (NSDictionary *)attributes;
@end

@interface FontconfigPatternParser : NSObject
{
  NSMutableDictionary *_attributes;
  FcPattern *_pat;
}
- (NSDictionary*)attributesFromPattern: (FcPattern *)pat;
@end

@interface FontconfigCharacterSet : NSCharacterSet
{
  FcCharSet *_charset;
}

- (id)initWithFontconfigCharSet: (FcCharSet*)charset;
- (FcCharSet*)fontconfigCharSet;

@end

#endif
