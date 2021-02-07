/*
   GSCodingFlags.h

   Define flags used in Cocoa for keyed coding.

   Copyright (C) 2019 Free Software Foundation, Inc.

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: 12.2019
   Original code by: Jonathan Gillaspie <jonathan.gillaspie@testplant.com>,
                     Frank Le Grand <frank.legrand@testplant.com>
                     Paul Landers <paul.landers@testplant.com> and
                     Doug Simons <doug.simons@testplant.com>

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
#ifndef _GNUstep_H_GSCodingFlags
#define _GNUstep_H_GSCodingFlags

#import "GNUstepGUI/GSNibLoading.h"

typedef struct _GSCellFlags {
#if GS_WORDS_BIGENDIAN == 1
  unsigned int        state:1;
  unsigned int        highlighted:1;
  unsigned int        disabled:1;
  unsigned int        editable:1;

  NSCellType          type:2;
  unsigned int        vCentered:1;
  unsigned int        hCentered:1;

  unsigned int        bordered:1;
  unsigned int        bezeled:1;
  unsigned int        selectable:1;
  unsigned int        scrollable:1;

  unsigned int        continuous:1;
  unsigned int        actOnMouseDown:1;
  unsigned int        isLeaf:1;
  unsigned int        invalidObjectValue:1;

  unsigned int        invalidFont:1;
  NSLineBreakMode     lineBreakMode:3;

  unsigned int        weakTargetHelperFlag:1;
  unsigned int        allowsAppearanceEffects:1;
  unsigned int        singleLineMode:1;
  unsigned int        actOnMouseDragged:1;

  unsigned int        isLoaded:1;
  unsigned int        truncateLastLine:1;
  unsigned int        dontActOnMouseUp:1;
  unsigned int        isWhite:1;

  unsigned int        useUserKeyEquivalent:1;
  unsigned int        showsFirstResponder:1;
  unsigned int        focusRingType:2;
#else
  unsigned int        focusRingType:2;
  unsigned int        showsFirstResponder:1;
  unsigned int        useUserKeyEquivalent:1;
  unsigned int        isWhite:1;
  unsigned int        dontActOnMouseUp:1;
  unsigned int        truncateLastLine:1;
  unsigned int        isLoaded:1;
  unsigned int        actOnMouseDragged:1;
  unsigned int        singleLineMode:1;
  unsigned int        allowsAppearanceEffects:1;
  unsigned int        weakTargetHelperFlag:1;
  NSLineBreakMode     lineBreakMode:3;
  unsigned int        invalidFont:1;
  unsigned int        invalidObjectValue:1;
  unsigned int        isLeaf:1;
  unsigned int        actOnMouseDown:1;
  unsigned int        continuous:1;
  unsigned int        scrollable:1;
  unsigned int        selectable:1;
  unsigned int        bezeled:1;
  unsigned int        bordered:1;
  unsigned int        hCentered:1;
  unsigned int        vCentered:1;
  NSCellType          type:2;
  unsigned int        editable:1;
  unsigned int        disabled:1;
  unsigned int        highlighted:1;
  unsigned int        state:1;
#endif
} GSCellFlags;

typedef union _GSCellFlagsUnion
{
  GSCellFlags flags;
  uint32_t    value;
} GSCellFlagsUnion;

typedef struct _GSCellflags2
{
#if GS_WORDS_BIGENDIAN == 1
  unsigned int unused1:1;
  unsigned int allowsEditingTextAttributes:1;
  unsigned int importsGraphics:1;
  unsigned int alignment:3;
  unsigned int refusesFirstResponder:1;
  unsigned int allowsMixedState:1;
  unsigned int unused2:1;
  unsigned int sendsActionOnEndEditing:1;
  unsigned int unused3:2;
  unsigned int controlSize:3;
  unsigned int unused4:4;
  unsigned int doesNotAllowUndo:1;
  unsigned int lineBreakMode:3;
  unsigned int unused5:1;
  unsigned int controlTint:3;
  unsigned int unused6:5;
#else
  unsigned int unused6:5;
  unsigned int controlTint:3;
  unsigned int unused5:1;
  unsigned int lineBreakMode:3;
  unsigned int doesNotAllowUndo:1;
  unsigned int unused4:4;
  unsigned int controlSize:3;
  unsigned int unused3:2;
  unsigned int sendsActionOnEndEditing:1;
  unsigned int unused2:1;
  unsigned int allowsMixedState:1;
  unsigned int refusesFirstResponder:1;
  unsigned int alignment:3;
  unsigned int importsGraphics:1;
  unsigned int allowsEditingTextAttributes:1;
  unsigned int unused1:1;
#endif
} GSCellFlags2;

typedef union _GSCellFlags2Union
{
  GSCellFlags2 flags;
  uint32_t     value;
} GSCellFlags2Union;

typedef struct _GSButtonCellFlags
{
#if GS_WORDS_BIGENDIAN == 1
  unsigned int isPushin:1;
  unsigned int changeContents:1;
  unsigned int changeBackground:1;
  unsigned int changeGray:1;

  unsigned int highlightByContents:1;
  unsigned int highlightByBackground:1;
  unsigned int highlightByGray:1;
  unsigned int drawing:1;

  unsigned int isBordered:1;
  unsigned int imageDoesOverlap:1;
  unsigned int isHorizontal:1;
  unsigned int isBottomOrLeft:1;

  unsigned int isImageAndText:1;
  unsigned int isImageSizeDiff:1;
  unsigned int hasKeyEquiv:1;
  unsigned int lastState:1;

  unsigned int isTransparent:1;
  unsigned int inset:2; // inset:2
  unsigned int doesNotDimImage:1; //doesn't dim:1

  unsigned int gradient:3; // gradient:3
  unsigned int useButtonImageSource:1;

  unsigned int unused2:8; // alt mnemonic loc.
#else
  unsigned int unused2:8; // alt mnemonic loc.
  unsigned int useButtonImageSource:1;
  unsigned int gradient:3; // gradient:3
  unsigned int doesNotDimImage:1; // doesn't dim:1
  unsigned int inset:2; // inset:2
  unsigned int isTransparent:1;
  unsigned int lastState:1;
  unsigned int hasKeyEquiv:1;
  unsigned int isImageSizeDiff:1;
  unsigned int isImageAndText:1;
  unsigned int isBottomOrLeft:1;
  unsigned int isHorizontal:1;
  unsigned int imageDoesOverlap:1;
  unsigned int isBordered:1;
  unsigned int drawing:1;
  unsigned int highlightByGray:1;
  unsigned int highlightByBackground:1;
  unsigned int highlightByContents:1;
  unsigned int changeGray:1;
  unsigned int changeBackground:1;
  unsigned int changeContents:1;
  unsigned int isPushin:1;
#endif
} GSButtonCellFlags;

typedef union _GSButtonCellFlagsUnion
{
  GSButtonCellFlags flags;
  uint32_t          value;
} GSButtonCellFlagsUnion;

typedef struct _GSButtonCellFlags2 {
#if GS_WORDS_BIGENDIAN == 1
  unsigned int	keyEquivalentModifierMask:24;
  unsigned int	imageScaling:2;
  unsigned int	bezelStyle2:1;
  unsigned int	mouseInside:1;
  unsigned int	showsBorderOnlyWhileMouseInside:1;
  unsigned int	bezelStyle:3;
#else
  unsigned int	bezelStyle:3;
  unsigned int	showsBorderOnlyWhileMouseInside:1;
  unsigned int	mouseInside:1;
  unsigned int	bezelStyle2:1;
  unsigned int	imageScaling:2;
  unsigned int	keyEquivalentModifierMask:24;
#endif
} GSButtonCellFlags2;

typedef union _GSButtonCellFlags2Union
{
  GSButtonCellFlags2 flags;
  uint32_t           value;
} GSButtonCellFlags2Union;

typedef struct _GSvFlags
{
#if GS_WORDS_BIGENDIAN == 1
  unsigned int isHidden:1;
  unsigned int unused1:3;
  unsigned int unused2:4;
  unsigned int unused3:4;
  unsigned int unused4:4;
  unsigned int unused5:4;
  unsigned int unused6:3;
  unsigned int autoresizesSubviews:1;
  unsigned int unused7:2;
  unsigned int autoresizingMask:6;
#else
  unsigned int autoresizingMask:6;
  unsigned int unused7:2;
  unsigned int autoresizesSubviews:1;
  unsigned int unused6:3;
  unsigned int unused5:4;
  unsigned int unused4:4;
  unsigned int unused3:4;
  unsigned int unused2:4;
  unsigned int unused1:3;
  unsigned int isHidden:1;
#endif
} GSvFlags;

typedef union _GSvFlagsUnion
{
  GSvFlags flags;
  uint32_t value;
} GSvFlagsUnion;

typedef struct _GSTabViewTypeFlags
{
#if GS_WORDS_BIGENDIAN == 1
  unsigned int reserved1:1;
  unsigned int controlTint:3;
  unsigned int controlSize:2;
  unsigned int reserved2:18;
  unsigned int tabPosition:5;
  unsigned int tabViewBorderType:3;
#else
  unsigned int tabViewBorderType:3;
  unsigned int tabPosition:5;
  unsigned int reserved2:18;
  unsigned int controlSize:2;
  unsigned int controlTint:3;
  unsigned int reserved1:1;
#endif
} GSTabViewTypeFlags;

typedef union _GSTabViewTypeFlagsUnion
{
  GSTabViewTypeFlags flags;
  unsigned int       value;
} GSTabViewTypeFlagsUnion;

/*
 * Nib compatibility struct.  This structure is used to
 * pull the attributes out of the nib that we need to fill
 * in the flags.
 */
typedef struct _tableViewFlags
{
#if GS_WORDS_BIGENDIAN == 1
  unsigned int columnOrdering:1;
  unsigned int columnResizing:1;
  unsigned int drawsGrid:1;
  unsigned int emptySelection:1;
  unsigned int multipleSelection:1;
  unsigned int columnSelection:1;
  unsigned int unknown1:1;
  unsigned int columnAutosave:1;
  unsigned int alternatingRowBackgroundColors:1;
  unsigned int unknown2:3;
  unsigned int _unused:20;
#else
  unsigned int _unused:20;
  unsigned int unknown2:3;
  unsigned int alternatingRowBackgroundColors:1;
  unsigned int columnAutosave:1;
  unsigned int unknown1:1;
  unsigned int columnSelection:1;
  unsigned int multipleSelection:1;
  unsigned int emptySelection:1;
  unsigned int drawsGrid:1;
  unsigned int columnResizing:1;
  unsigned int columnOrdering:1;
#endif
} GSTableViewFlags;

typedef union _GSTableViewFlagsUnion
{
  GSTableViewFlags flags;
  uint32_t         value;
} GSTableViewFlagsUnion;

typedef union _GSWindowTemplateFlagsUnion
{
  GSWindowTemplateFlags  flags;
  uint32_t               value;
} GSWindowTemplateFlagsUnion;

typedef struct _GSMatrixFlags {
#if GS_WORDS_BIGENDIAN == 1
  unsigned int isHighlight:1;
  unsigned int isRadio:1;
  unsigned int isList:1;
  unsigned int allowsEmptySelection:1;
  unsigned int autoScroll:1;
  unsigned int selectionByRect:1;
  unsigned int drawCellBackground:1;
  unsigned int drawBackground:1;
  unsigned int autosizesCells:1;
  unsigned int drawingAncestor:1;
  unsigned int tabKeyTraversesCells:1;
  unsigned int tabKeyTraversesCellsExplicitly:1;
  unsigned int canSearchIncrementally:1;
  unsigned int unused:19;
#else
  unsigned int unused:19;
  unsigned int canSearchIncrementally:1;
  unsigned int tabKeyTraversesCellsExplicitly:1;
  unsigned int tabKeyTraversesCells:1;
  unsigned int drawingAncestor:1;
  unsigned int autosizesCells:1;
  unsigned int drawBackground:1;
  unsigned int drawCellBackground:1;
  unsigned int selectionByRect:1;
  unsigned int autoScroll:1;
  unsigned int allowsEmptySelection:1;
  unsigned int isList:1;
  unsigned int isRadio:1;
  unsigned int isHighlight:1;
#endif
} GSMatrixFlags;

typedef union _GSMatrixFlagsUnion
{
  GSMatrixFlags flags;
  unsigned int  value;
} GSMatrixFlagsUnion;

#endif // _GNUstep_H_GSCodingFlags
