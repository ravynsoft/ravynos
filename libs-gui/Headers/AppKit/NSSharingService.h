/** NSSharingService.h

   <abstract>Interface for sharing things</abstract>

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
#ifndef __GNUstep_NSSharingService
#define __GNUstep_NSSharingService

#import <Foundation/NSObject.h>

@interface NSSharingService : NSObject
@end

@protocol NSSharingServiceDelegate <NSObject>
@end

@interface NSSharingServicePicker : NSObject 
@end

@protocol NSSharingServicePickerDelegate <NSObject>
@end

#endif
