/* 
   NSFormCell.h

   The cell class for the NSForm control

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: March 1997
   
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

#ifndef _GNUstep_H_NSFormCell
#define _GNUstep_H_NSFormCell
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSActionCell.h>

@interface NSFormCell : NSActionCell <NSCoding>
{
  // NB: this is the titleWidth which is effectively used -- takes in
  // account all the other cells' titleWidths.
  // If its value is -1, it means it must be recomputed.
  float _displayedTitleWidth;

  // Think the following as a BOOL ivar
  // YES if the titleWidth is automatically computed
#define _formcell_auto_title_width _cell.subclass_bool_one
  NSCell* _titleCell;
#define _formcell_placeholder_is_attributed_string _cell.subclass_bool_two
  id _placeholder;
}

//
// Modifying the Title 
//
- (void)setTitle:(NSString*)aString;
- (void)setTitleAlignment:(NSTextAlignment)mode;
- (void)setTitleFont:(NSFont*)fontObject;
- (void)setTitleWidth:(CGFloat)width;
- (NSString*)title;
- (NSTextAlignment)titleAlignment;
- (NSFont*)titleFont;
- (CGFloat)titleWidth;
- (CGFloat)titleWidth:(NSSize)aSize;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
//
// Attributed title
//
- (NSAttributedString *)attributedTitle;
- (void)setAttributedTitle:(NSAttributedString *)anAttributedString;
- (void)setTitleWithMnemonic:(NSString *)titleWithAmpersand;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSAttributedString*)placeholderAttributedString;
- (NSString*)placeholderString;
- (void)setPlaceholderAttributedString:(NSAttributedString*)string;
- (void)setPlaceholderString:(NSString*)string;

- (NSWritingDirection)titleBaseWritingDirection;
- (void)setTitleBaseWritingDirection:(NSWritingDirection)writingDirection;
#endif
@end


APPKIT_EXPORT NSString *_NSFormCellDidChangeTitleWidthNotification;

#endif // _GNUstep_H_NSFormCell
