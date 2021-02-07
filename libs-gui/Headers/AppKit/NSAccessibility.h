/** NSAccessibility.h

   <abstract>Contains functions for accessibility functionality</abstract>

   Copyright <copy>(C) 2017 Free Software Foundation, Inc.</copy>

   Author: Daniel Ferreira <dtf@stanford.edu>
   Date: 2017

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

#ifndef _GNUstep_H_NSAccessibility
#define _GNUstep_H_NSAccessibility

#import <AppKit/NSAccessibilityConstants.h>

APPKIT_EXTERN void NSAccessibilityPostNotification(
  id element,
  NSString *notification);

APPKIT_EXTERN id NSAccessibilityUnignoredAncestor(id element);
APPKIT_EXTERN id NSAccessibilityUnignoredDescendant(id element);
APPKIT_EXTERN NSArray *NSAccessibilityUnignoredChildren(
  NSArray *originalChildren);
APPKIT_EXTERN NSArray *NSAccessibilityUnignoredChildrenForOnlyChild(
  id originalChild);

APPKIT_EXTERN NSString *NSAccessibilityRoleDescription(
  NSString *role,
  NSString *subrole);
APPKIT_EXTERN NSString *NSAccessibilityRoleDescriptionForUIElement(id element);
APPKIT_EXTERN NSString *NSAccessibilityActionDescription(NSString *action);

#endif
