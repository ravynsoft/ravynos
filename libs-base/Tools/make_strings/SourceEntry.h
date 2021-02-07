/* SourceEntry

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

#ifndef SourceEntry_h
#define SourceEntry_h

@interface SourceEntry : NSObject
{
	NSString *file,*comment,*key;
	unsigned int line;
}

/* TODO: very cryptic error message if duplicate name in argument list,
gcc issue */
- initWithKey: (NSString *)k comment: (NSString *)c file: (NSString *)f line: (unsigned int)l;

-(NSString *) file;
-(NSString *) comment;
-(NSString *) key;
-(unsigned int) line;

@end

#endif

