/* make_strings

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

#ifndef make_strings_h
#define make_strings_h

@class SourceEntry;

@interface NSMutableDictionary (make_strings)
-(void) addEntry: (SourceEntry *)e  toTable: (NSString *)table;
@end

extern int verbose,aggressive_import,aggressive_match,aggressive_remove;

#endif

