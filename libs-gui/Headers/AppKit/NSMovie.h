/** <title>NSMovie</title>

   <abstract>Encapsulate a Quicktime movie</abstract>

   Copyright <copy>(C) 2003 Free Software Foundation, Inc.</copy>

   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: March 2003

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

#ifndef _GNUstep_H_NSMovie
#define _GNUstep_H_NSMovie
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>

@class NSArray;
@class NSData;
@class NSURL;
@class NSPasteboard;

@interface NSMovie : NSObject <NSCopying, NSCoding> 
{
  @private
    NSData*  _movie;
    NSURL*   _url;
}

+ (NSArray*) movieUnfilteredFileTypes;
+ (NSArray*) movieUnfilteredPasteboardTypes;
+ (BOOL) canInitWithPasteboard: (NSPasteboard*)pasteboard;

- (id) initWithMovie: (void*)movie;
- (id) initWithURL: (NSURL*)url byReference: (BOOL)byRef;
- (id) initWithPasteboard: (NSPasteboard*)pasteboard;

- (void*) QTMovie;
- (NSURL*) URL;

@end

#endif /* _GNUstep_H_NSMovie */
