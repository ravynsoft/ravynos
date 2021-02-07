/* 
   GSCharacterPanel.h

   Ghostscript image representation.

   Copyright (C) 2011 Free Software Foundation, Inc.

   Written by:  Eric Wasylishen <ewasylishen@gmail.com>
   Date: June 2011
   
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

#ifndef _GNUstep_H_GSCharacterPanel
#define _GNUstep_H_GSCharacterPanel

#import <AppKit/NSPanel.h>

@class NSTableView;
@class NSSearchField;
@class NSIndexSet;

@interface GSCharacterPanel : NSPanel
{
	NSTableView *table;
	NSSearchField *searchfield;

	NSIndexSet *assignedCodepoints;
	NSIndexSet *visibleCodepoints;
}

+ (GSCharacterPanel *) sharedCharacterPanel;

@end

#endif // _GNUstep_H_GSCharacterPanel

