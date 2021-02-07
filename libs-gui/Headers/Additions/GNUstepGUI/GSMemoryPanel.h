/* GSMemoryPanel.h                                           -*-objc-*-

   A GNUstep panel for tracking memory leaks.

   Copyright (C) 2000, 2002 Free Software Foundation, Inc.

   Author:  Nicola Pero <nicola@brainstorm.co.uk>
   Date: 2000, 2002
   
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

/*
 * Class displaying a panel showing object allocation statistics.
 *
 */

#ifndef _GNUstep_H_GSMEMORY_PANEL_
#define _GNUstep_H_GSMEMORY_PANEL_

#import <AppKit/NSApplication.h>
#import <AppKit/NSPanel.h>

@class NSTableView;
@class NSMutableArray;

@interface GSMemoryPanel: NSPanel
{
  NSTableView *table;
  NSMutableArray *array;
  /* Are we ordering by class name, or by count or total or peak number 
     of instances ? */
  SEL orderingBy;
}
+ (id) sharedMemoryPanel;

/* Updates the statistics */
+ (void) update: (id)sender;
- (void) update: (id)sender;
@end

@interface NSApplication (GSMemoryPanel)
- (void) orderFrontSharedMemoryPanel: (id)sender;
@end

#endif /* _GNUstep_H_GSMEMORY_PANEL_ */




