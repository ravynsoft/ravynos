/* StringsEntry

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

#ifndef StringsEntry_h
#define StringsEntry_h

@class SourceEntry;

@interface StringsEntry : NSObject
{
	NSString *key,*comment,*translated;
	NSString *user_comment;

#define FLAG_UNTRANSLATED  0x01
#define FLAG_UNMATCHED     0x02
	int flags;

	NSString *file;
	unsigned int line;
}

+ stringsEntryFromSourceEntry: (SourceEntry *)se;

- initWithKey: (NSString *)k comment: (NSString *)c translated: (NSString *)t userComment: (NSString *)uc
	flags: (int)flags  file: (NSString *)filename line: (int)line;

-(NSString *) key;
-(NSString *) comment;
-(NSString *) translated;
-(NSString *) userComment;

-(int) flags;

-(NSString *) file;
-(unsigned int) line;

-(int) compareFileLine: (StringsEntry *)e;
-(int) compareFileKeyComment: (StringsEntry *)e;


-(void) setKey: (NSString *)k;
-(void) setComment: (NSString *)c;
-(void) setTranslated: (NSString *)t;
-(void) setUserComment: (NSString *)uc;

-(void) setFlags: (int)f;
-(void) addFlag: (int)f;

-(void) setFile: (NSString *)f;
-(void) setLine: (unsigned int)l;

@end

#endif

