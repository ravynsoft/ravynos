/* 
   NSFontPanel.h

   Standard panel for selecting and previewing fonts.

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   
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

#ifndef _GNUstep_H_NSFontPanel
#define _GNUstep_H_NSFontPanel
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSPanel.h>
#import <AppKit/NSFontManager.h>
#import <AppKit/NSTextField.h> // needed for NSTextFieldDelegate

@class NSFont;
@class NSView;
@class NSButton;
@class NSBrowser;
@class NSTextField;

enum {
  NSFPPreviewButton,
  NSFPRevertButton,
  NSFPSetButton,
  NSFPPreviewField,
  NSFPSizeField,
  NSFPSizeTitle,
  NSFPCurrentField,

  // GNUstep extensions
  NSFPFamilyBrowser,
  NSFPFaceBrowser,
  NSFPSizeBrowser
};

@interface NSFontPanel : NSPanel <NSTextFieldDelegate>
{
  // Attributes
  NSFont *_panelFont;
  BOOL _multiple;
  BOOL _preview;

  // store currently selected information
  NSArray *_familyList;
  NSArray *_faceList;
  int _family;
  int _face;
  NSFontTraitMask _traits;
  int _weight;
  // user typed string for preview area
  NSString *_previewString;

  // field for display
  NSView *_accessoryView;
  NSView *_topView;
  NSView *_bottomView;

  // sizes
  NSSize _originalMinSize;
  NSSize _originalSize;
}

//
// Creating an NSFontPanel 
//
+ (NSFontPanel *)sharedFontPanel;
+ (BOOL)sharedFontPanelExists;

//
// Enabling
//
- (BOOL)isEnabled;
- (void)setEnabled:(BOOL)flag;
- (void)reloadDefaultFontFamilies;

//
// Updating font
//
- (void)setPanelFont:(NSFont *)fontObject
	  isMultiple:(BOOL)flag;

//
// Converting
//
- (NSFont *)panelConvertFont:(NSFont *)fontObject;

//
// Works in modal loops
//
- (BOOL)worksWhenModal;

//
// Configuring the NSFontPanel 
//
- (NSView *)accessoryView;
- (void)setAccessoryView:(NSView *)aView;

@end

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
enum
{
	NSFontPanelFaceModeMask = 1 << 0,
	NSFontPanelSizeModeMask = 1 << 1,
	NSFontPanelCollectionModeMask = 1 << 2,

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
	NSFontPanelUnderlineEffectModeMask = 1 << 8,
	NSFontPanelStrikethroughEffectModeMask = 1 << 9,
	NSFontPanelTextColorEffectModeMask = 1 << 10,
	NSFontPanelDocumentColorEffectModeMask = 1 << 11,
	NSFontPanelShadowEffectModeMask = 1 << 12,
	NSFontPanelAllEffectsModeMask = 0xfff00,
#endif

	NSFontPanelStandardModesMask = 0xffff,
	NSFontPanelAllModesMask = 0xffffffff
};

@interface NSObject (NSFontPanelValidation)

- (unsigned int)validModesForFontPanel:(NSFontPanel *)fontPanel;

@end

#endif

#endif // _GNUstep_H_NSFontPanel


