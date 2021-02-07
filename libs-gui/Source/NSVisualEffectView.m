/*
   NSVisualEffectView.m
  
   View with visual effects

   Copyright (C) 2017 Free Software Foundation, Inc.

   Author: Daniel Ferreira <dtf@stanford.edu>
   Date: 2017

   This file is part of the GNUstep GUI Library.

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

#import <AppKit/NSVisualEffectView.h>

@implementation NSVisualEffectView
- (NSVisualEffectMaterial) material
{
  return 0;
}

- (void) setMaterial: (NSVisualEffectMaterial)material
{
  return;
}

- (NSInteger)interiorBackgroundStyle
{
  return 0;
}

- (NSVisualEffectBlendingMode) blendingMode
{
  return 0;
}
- (void) setBlendingMode: (NSVisualEffectBlendingMode) mode
{
  return;
}

- (NSVisualEffectState) state
{
  return 0;
}

- (void) setState: (NSVisualEffectState)state
{
  return;
}

- (NSImage *) maskImage
{
  return nil;
}

- (void) setMaskImage: (NSImage *)image
{
  return;
}
@end
