/* 
   GSToolbarCustomizationPalette.h

   The palette which allows to customize toolbar
   
   Copyright (C) 2007 Free Software Foundation, Inc.

   Author:  Quentin Mathe <qmathe@club-internet.fr>
   Date: January 2007
   
   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; see the file COPYING.LIB.
   If not, write to the Free Software Foundation,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/ 

#ifndef _GNUstep_H_GSToolbarCustomizationPalette
#define _GNUstep_H_GSToolbarCustomizationPalette

#import <Foundation/NSObject.h>

@class NSMutableArray;
@class NSToolbar;


@interface GSToolbarCustomizationPalette : NSObject
{
  id _customizationWindow;
  id _customizationView;
  id _defaultTemplateView;
  id _sizeCheckBox;
  id _displayPopup;
  id _doneButton;
  
  NSMutableArray *_allowedItems;
  NSMutableArray *_defaultItems;
  NSToolbar *_toolbar;
}

+ (id) palette;

- (void) showForToolbar: (NSToolbar *)toolbar;

@end

#endif /* _GNUstep_H_GSToolbarCustomizationPalette */
