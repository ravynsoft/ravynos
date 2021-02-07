/* -*-objc-*-
   NSPredicateEditor.h

   The predicate editor class

   Copyright (C) 2020 Free Software Foundation, Inc.

   Created by Fabian Spillner on 03.12.07.

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

#ifndef _GNUstep_H_NSPredicateEditor
#define _GNUstep_H_NSPredicateEditor

#import <AppKit/NSRuleEditor.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)

@class NSArray;

@interface NSPredicateEditor : NSRuleEditor {
  NSArray *_rowTemplates;
}

- (NSArray *) rowTemplates;
- (void) setRowTemplates: (NSArray *)templates;

@end

#endif
#endif /* _GNUstep_H_NSPredicateEditor */
