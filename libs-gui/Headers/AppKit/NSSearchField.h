/* 
   NSSearchField.h

   Text field control class for text search

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: H. Nikolaus Schaller <hns@computer.org>
   Date: Dec 2004
   
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

#ifndef _GNUstep_H_NSSearchField
#define _GNUstep_H_NSSearchField

#import <AppKit/NSTextField.h>

@interface NSSearchField : NSTextField

- (NSArray *) recentSearches;
- (NSString *) recentsAutosaveName;
- (void) setRecentSearches: (NSArray *)searches;
- (void) setRecentsAutosaveName: (NSString *)name;

@end

#endif /* _GNUstep_H_NSSearchField */
