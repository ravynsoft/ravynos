/* 
   GSTextStorage.h

   Implementation of concrete subclass of a string class with attributes

   Copyright (C) 1999 Free Software Foundation, Inc.

   Based on code by: ANOQ of the sun <anoq@vip.cybercity.dk>
   Written by: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: July 1999
   
   This file is part of GNUStep-gui

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

#import "AppKit/NSTextStorage.h"

@class NSMutableString;
@class NSMutableArray;

@interface GSTextStorage : NSTextStorage
{
  NSMutableString       *_textChars;
  NSMutableArray        *_infoArray;
  NSString		*_textProxy;
}
@end

