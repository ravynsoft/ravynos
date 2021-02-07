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

#include <Foundation/NSObject.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSEnumerator.h>
#include <Foundation/NSString.h>

#include "StringsEntry.h"

#include "SourceEntry.h"


@implementation StringsEntry

+ stringsEntryFromSourceEntry: (SourceEntry *)e
{
  return AUTORELEASE([[self alloc] initWithKey: [e key] comment: [e comment] translated: [e key] userComment: nil
				   flags: FLAG_UNTRANSLATED  file: [e file] line: [e line]]);
}


- initWithKey: (NSString *)k comment: (NSString *)c translated: (NSString *)t userComment: (NSString *)uc
	flags: (int)f  file: (NSString *)filename line: (int)l
{
  self=[super init];
  ASSIGN(key,k);
  ASSIGN(comment,c);
  ASSIGN(translated,t);
  ASSIGN(user_comment,uc);
  flags=f;
  ASSIGN(file,filename);
  line=l;
  return self;
}

-(void) dealloc
{
  DESTROY(key);
  DESTROY(comment);
  DESTROY(translated);
  DESTROY(user_comment);
  DESTROY(file);
  [super dealloc];
}

-(NSString *) key { return key; }
-(NSString *) comment { return comment; }
-(NSString *) translated { return translated; }
-(NSString *) userComment { return user_comment; }

-(int) flags { return flags; }

-(NSString *) file { return file; }
-(unsigned int) line { return line; }


-(int) compareFileLine: (StringsEntry *)e
{
  int res=[[self file] compare: [e file]];
  if (res!=NSOrderedSame) return res;
  if ([self line]<[e line])
    return NSOrderedAscending;
  else
    return NSOrderedDescending;
}

-(int) compareFileKeyComment: (StringsEntry *)e
{
  int res;

  res=[[self file] compare: [e file]];
  if (res!=NSOrderedSame) return res;

  res=[[self key] compare: [e key]];
  if (res!=NSOrderedSame) return res;

  if (![self comment])
    return NSOrderedAscending;
  if (![e comment])
    return NSOrderedDescending;
  return [[self comment] compare: [e comment]];
}


-(void) setKey: (NSString *)k { ASSIGN(key,k); }
-(void) setComment: (NSString *)c { ASSIGN(comment,c); }
-(void) setTranslated: (NSString *)t { ASSIGN(translated,t); }
-(void) setUserComment: (NSString *)uc { ASSIGN(user_comment,uc); }

-(void) setFlags: (int)f { flags=f; }
-(void) addFlag: (int)f { flags|=f; }

-(void) setFile: (NSString *)f { ASSIGN(file,f); }
-(void) setLine: (unsigned int)l { line=l; }


-(NSString *) description
{
  return [NSString
	   stringWithFormat: @"(key='%@' comment='%@'  at '%@':%i  userComment='%@'  translated='%@'  flags=%02x)",
	   key,comment,file,line,user_comment,translated,flags];
}

@end

