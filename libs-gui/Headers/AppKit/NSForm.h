/* 
   NSForm.h

   Form class, a matrix of text fields with labels

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

#ifndef _GNUstep_H_NSForm
#define _GNUstep_H_NSForm
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSMatrix.h>

@class NSFormCell;
@class NSFont;

@interface NSForm : NSMatrix
{
  BOOL _title_width_needs_update;
}
//
// Laying Out the Form 
//
- (NSFormCell*)addEntry:(NSString*)title;
- (NSFormCell*)insertEntry:(NSString*)title
		    atIndex:(NSInteger)index;
- (void)removeEntryAtIndex:(NSInteger)index;
- (void)setInterlineSpacing:(CGFloat)spacing;

//
// Finding Indices
//
- (NSInteger)indexOfCellWithTag:(NSInteger)aTag;
- (NSInteger)indexOfSelectedItem;

//
// Modifying Graphic Attributes 
//
- (void)setBezeled:(BOOL)flag;
- (void)setBordered:(BOOL)flag;
- (void)setTextAlignment:(NSTextAlignment)aMode;
- (void)setTextFont:(NSFont*)fontObject;
- (void)setTitleAlignment:(NSTextAlignment)aMode;
- (void)setTitleFont:(NSFont*)fontObject;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (void) setTitleBaseWritingDirection: (NSWritingDirection)direction;
- (void) setTextBaseWritingDirection: (NSWritingDirection)direction;
#endif

//
// Getting a Cell 
//
- (id)cellAtIndex:(NSInteger)index;

//
// Displaying a Cell
//
- (void)drawCellAtIndex:(NSInteger)index;

//
// Editing Text 
//
- (void)selectTextAtIndex:(NSInteger)index;

//
// Resizing the Form 
//
- (void)setEntryWidth:(float)width;

// Private
-(void) _setTitleWidthNeedsUpdate: (NSNotification*)notification;
@end

APPKIT_EXPORT NSString *_NSFormCellDidChangeTitleWidthNotification;

#endif // _GNUstep_H_NSForm
