/* StringsFile

   Copyright (C) 2002 Free Software Foundation, Inc.

   Written by:  Alexander Malmberg <alexander@malmberg.org>
   Created: 2002

   This file is part of the GNUstep Project

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   You should have received a copy of the GNU General Public
   License along with this program; see the file COPYINGv3.
   If not, write to the Free Software Foundation,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

#ifndef StringsFile_h
#define StringsFile_h

@class StringsEntry;
@class SourceEntry;
@class NSMutableArray;

@interface StringsFile : NSObject
{
@public
  NSMutableArray *strings;
  NSString *global_comment;

@private
  /** These are used for aggressive matching **/
  /* Contains all keys for which there is a translation */
  NSMutableArray *keys_translated;
  /* Contains all keys that appeared in the source */
  NSMutableArray *keys_matched;
}

- init;
- initWithFile: (NSString *)filename;

-(BOOL) writeToFile: (NSString *)filename;

-(void) addSourceEntry: (SourceEntry *)e;

@end

#endif

