/*
   NSVisualEffectView.h
  
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

#import <AppKit/NSView.h>
#import <GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

typedef NSInteger NSVisualEffectMaterial;
enum {
  NSVisualEffectMaterialAppearanceBased = 0,
  NSVisualEffectMaterialTitlebar = 3,
  NSVisualEffectMaterialMenu = 5,
  NSVisualEffectMaterialPopover = 6,
  NSVisualEffectMaterialSidebar = 7,

  NSVisualEffectMaterialLight = 1,
  NSVisualEffectMaterialDark = 2,
  NSVisualEffectMaterialMediumLight = 8,
  NSVisualEffectMaterialUltraDark = 9,
};
                
typedef NSInteger NSVisualEffectBlendingMode;
enum {
  NSVisualEffectBlendingModeBehindWindow,
  NSVisualEffectBlendingModeWithinWindow,
};

typedef NSInteger NSVisualEffectState;
enum {
  NSVisualEffectStateFollowsWindowActiveState,
  NSVisualEffectStateActive,
  NSVisualEffectStateInactive,
};

@interface NSVisualEffectView : NSView {
}

#if GS_HAS_DECLARED_PROPERTIES
@property NSVisualEffectMaterial material;

@property(readonly) NSInteger interiorBackgroundStyle;
@property NSVisualEffectBlendingMode blendingMode;

@property NSVisualEffectState state;
@property(retain) NSImage *maskImage;
#else
- (NSVisualEffectMaterial) material;
- (void) setMaterial: (NSVisualEffectMaterial)material;

- (NSInteger)interiorBackgroundStyle;

- (NSVisualEffectBlendingMode) blendingMode;
- (void) setBlendingMode: (NSVisualEffectBlendingMode) mode;

- (NSVisualEffectState) state;
- (void) setState: (NSVisualEffectState)state;

- (NSImage *) maskImage;
- (void) setMaskImage: (NSImage *)image;
#endif

@end

#endif
