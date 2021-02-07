/*
   GSInfoPanel.h

   Standard GNUstep info panel

   Copyright (C) 2000 Free Software Foundation, Inc.

   Author:  Nicola Pero <n.pero@mi.flashnet.it>
   Date: January 2000

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

#ifndef _GNUstep_H_GSInfoPanel
#define _GNUstep_H_GSInfoPanel

#import <AppKit/NSPanel.h>

@class NSDictionary;

/* If you need an Info Panel, the simpler thing to do is to use
 * NSApplication -orderFrontStandardInfoPanel: and 
 * NSApplication -orderFrontStandardInfoPanelWithOptions:.
 * They automatically manage an infoPanel for you.
 */

@interface GSInfoPanel: NSPanel
{
}
/*
 * Instance Methods
 */

/*
 * The designated initializer
 *
 * Useful keys (with example values) for the dictionary are:
 *
 * ApplicationName = @"Gorm"
 *
 * ApplicationDescription = @"GNUstep Graphics Object Relationship Modeller"
 *
 * ApplicationIcon = an image
 *
 * ApplicationRelease = @"Gorm 0"
 * (ApplicationVersion in place of ApplicationRelease is also accepted for macosx compatibility)
 *
 * FullVersionID = @"0.0.1 1999"
 * (Version also accepted for macosx compatibility)
 *
 * Authors = an array of (NSString*)s, each one probably similar to the following
 * @"Richard Frith-Macdonald <richard@brainstorm.co.uk>"
 *
 * URL = @"See http://www.gnustep.org"
 * (still to fix/improve position of this thing so perhaps you don't want to use it)
 *
 * Copyright = @"Copyright (C) 1999, 2000 The Free Software Foundation, Inc."
 *
 * CopyrightDescription = @"Released under the GNU General Public License 2.0"
 *
 */
-(id) initWithDictionary: (NSDictionary *)dictionary;
/*
 * NB: Once initialized, a GSInfoPanel is immutable.
 */
@end

#endif /* _GNUstep_H_GSInfoPanel */
