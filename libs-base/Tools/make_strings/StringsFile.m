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

#import "common.h"
#import "Foundation/NSString.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDate.h"

#include "StringsFile.h"

#include "StringsEntry.h"
#include "SourceEntry.h"

#include "make_strings.h"


static NSString *parse_string(NSString **ptr)
{
  NSString *str = *ptr;
  NSString *ret;
  int i,c;
  unichar ch;

  c = [str length];
  for (i = 0; i < c; i++)
    {
      ch = [str characterAtIndex: i];
      if (ch == '\\')
	{
	  if (i == c)
	    {
	      fprintf(stderr,"parse error, \\ without second character\n");
	      exit(1);
	    }
	  i++;
	}
      if (ch == '\"')
	break;
    }
  if (i == c)
    {
      fprintf(stderr,"parse error, unterminated string\n");
      exit(1);
    }
  ret = [str substringToIndex: i];
  str = [str substringFromIndex: i+1];
  *ptr = str;

  return ret;
}


#define DUMMY @"<dummy>"

@implementation StringsFile

- (BOOL) isTranslated: (NSString *)key
{
  return [keys_translated containsObject: key];
}

- (void) addTranslated: (NSString *)key
{
  if (![self isTranslated: key])
    [keys_translated addObject: key];
}


- (BOOL) isMatched: (NSString *)key
{
  return [keys_matched containsObject: key];
}

- (void) addMatched: (NSString *)key
{
  if (![self isMatched: key])
    [keys_matched addObject: key];
}


- init
{
  self = [super init];
  strings = [[NSMutableArray alloc] init];
  keys_translated = [[NSMutableArray alloc] init];
  keys_matched = [[NSMutableArray alloc] init];
  return self;
}

- (void) dealloc
{
  DESTROY(global_comment);
  DESTROY(strings);
  [super dealloc];
}

- initWithFile: (NSString *)filename
{
  NSString *str;

  self = [self init];

  str = [NSString stringWithContentsOfFile: filename];
  if (!str) return self;

  {
    StringsEntry *se = nil;
    NSMutableArray *update_list = [[NSMutableArray alloc] init];
    NSArray *lines;
    NSString *l;
    NSUInteger i,c,pos; 	
    NSMutableDictionary *dummy_entries = [[NSMutableDictionary alloc] init];

    NSMutableString *user_comment = [[NSMutableString alloc] init];

    NSString *key,*trans;

    /* this is a bit yucky, but it works */
    lines = [str componentsSeparatedByString: @"/*"];
    c = [lines count];
    for (i = 0; i < c; i++)
      {
	l = [lines objectAtIndex: i];
	/* First entry has everything before the first comment and needs
	   to be handled specially. */
	if (i)
	  {
	    /* Parse special comments. */
	    if ([l hasPrefix: @"**"])
	      { /* It's one of our banners, just ignore it. If it's the first
		   one we put all comments so far in the global user comment. */
		if (user_comment)
		  {
		    if (![user_comment isEqual: @""])
		      global_comment = [user_comment copy];
		    DESTROY(user_comment);
		  }
	      }
	    else if ([l hasPrefix: @" File: "])
	      {
		se = [[StringsEntry alloc] init];
		[se addFlag: FLAG_UNMATCHED];  /* TODO: ? */
		[update_list addObject: se];
		[se release];

		l = [l substringFromIndex: 7];
		pos = [l rangeOfString: @":"].location;
		[se setFile: [l substringToIndex: pos]];
		l = [l substringFromIndex: pos+1];
		[se setLine: [l intValue]];
	      }
	    else if ([l hasPrefix: @" Flag: untranslated */"])
	      [se addFlag: FLAG_UNTRANSLATED];
	    else if ([l hasPrefix: @" Flag: unmatched */"])
	      [se addFlag: FLAG_UNMATCHED];  /* this is essentially a noop */
	    else if ([l hasPrefix: @" Comment: "])
	      {
		l = [l substringFromIndex: 10];
		pos = [l rangeOfString: @" */"].location;
                if (pos == NSNotFound) continue;
		[se setComment: [l substringToIndex: pos]];
	      }
	    else
	      {
		pos = [l rangeOfString: @"*/"].location;
                if (pos == NSNotFound)
                  {
                    fprintf(stderr,"parse error in '%s', missing '*\'\n",
                      [filename cString]);
                    pos = [l length];
                  }
		if ([user_comment length])
		  [user_comment appendString: @"\n"];
		[user_comment appendString: [l substringToIndex: pos]];
	      }

	    pos = [l rangeOfString: @"*/"].location;
	    if (pos == NSNotFound) continue;
	    l = [l substringFromIndex: pos+2];
	  }

	while (1)
	  {
	    pos = [l rangeOfString: @"\""].location;
	    if (pos == NSNotFound) break;

	    l = [l substringFromIndex: pos+1];
	    key = parse_string(&l);

	    pos = [l rangeOfString: @"="].location;
	    if (pos == NSNotFound)
	      {
		fprintf(stderr,"parse error in '%s', expecting '='\n",
                  [filename cString]);
		exit(1);
	      }
	    l = [l substringFromIndex: pos+1];
	    pos = [l rangeOfString: @"\""].location;
	    if (pos == NSNotFound)
	      {
		fprintf(stderr,"parse error in '%s', expecting second string\n",
                  [filename cString]);
		exit(1);
	      }
	    l = [l substringFromIndex: pos+1];
	    trans = parse_string(&l);

	    pos = [l rangeOfString: @";"].location;
	    if (pos == NSNotFound)
	      {
		fprintf(stderr,"parse error in '%s', expecting ';'\n",
                  [filename cString]);
		exit(1);
	      }
	    l = [l substringFromIndex: pos+1];

	    if (![update_list count])
	      { /* we're probably parsing a file not created by us */
		if (![dummy_entries objectForKey: key])
		  {
		    se = [[StringsEntry alloc] init];
		    [se setFile: DUMMY];
		    [se setFlags: FLAG_UNMATCHED];
		    [update_list addObject: se];
		    [se release];
		    [dummy_entries setObject: se forKey: key];
		  }
	      }

	    [update_list makeObjectsPerformSelector: @selector(setKey:)
                                         withObject: key];
	    [update_list makeObjectsPerformSelector: @selector(setTranslated:)
                                         withObject: trans];

	    {
	      int i,c = [update_list count];
              for (i = 0; i < c; i++)
                {
//		    printf("%4i : %@\n",i,[update_list objectAtIndex: i]);
                  se = [update_list objectAtIndex: i];
                  if (!([se flags] & FLAG_UNTRANSLATED))
                    {
                      [self addTranslated: key];
                      break;
                    }
                }
	    }


	    [strings addObjectsFromArray: update_list];

	    [update_list removeAllObjects];
	    se = nil;
	  }
      }

    DESTROY(user_comment);
    DESTROY(dummy_entries);
    DESTROY(update_list);
  }

  return self;
}


- (void) _writeTo: (NSMutableString *)str  entryHead: (StringsEntry *)se
{
  if ([se file])
    {
      [str appendString: @"/* File: "];
      [str appendString: [se file]];
      [str appendString: [NSString stringWithFormat: @":%i */\n",[se line]]];
    }
  if ([se comment])
    {
      [str appendString: @"/* Comment: "];
      [str appendString: [se comment]];
      [str appendString: @" */\n"];
    }
}

- (void) _writeTo: (NSMutableString *)str  entryFlags: (StringsEntry *)se
{
  int flags = [se flags];
  if (!flags) return;
  if (flags & FLAG_UNMATCHED)
    [str appendString: @"/* Flag: unmatched */\n"];
  else
    if (flags & FLAG_UNTRANSLATED)
      [str appendString: @"/* Flag: untranslated */\n"];
    else
      {
	fprintf(stderr,"unknown flag %08x\n",flags);
      }
}

- (void) _writeTo: (NSMutableString *)str  entryKey: (StringsEntry *)se
{
  [str appendString: @"\""];
  [str appendString: [se key]];

  /* Try to write a nice readable output ... key and value with whitrespace
   * around the '=' charactger, or wrap to a new line with a two space indent.
   */
  if ([[se key] length] + [[se translated] length] < 70)
    [str appendString: @"\" = \""];
  else
    [str appendString: @"\"\n  = \""];

  [str appendString: [se translated]];
  [str appendString: @"\";\n"];
}


- (void) _writeTo: (NSMutableString *)str  manyEntries: (NSMutableArray *)list
{
  int i,c;
  StringsEntry *tr,*cur;

  [list sortUsingSelector: @selector(compareFileLine:)];
  c = [list count];
  if (!c) return;
  cur = tr = nil;
  for (i = 0; i < c; i++)
    {
      cur = [list objectAtIndex: i];
      [self _writeTo: str  entryHead: cur];
      if ([cur flags])
	[self _writeTo: str  entryFlags: cur];

      if (!([cur flags] & FLAG_UNTRANSLATED))
	tr = cur;
    }
  if (tr)
    [self _writeTo: str  entryKey: tr];
  else
    [self _writeTo: str  entryKey: cur];
}

- (BOOL) _shouldIgnore: (StringsEntry *)se
{
  if (([se flags] & (FLAG_UNMATCHED|FLAG_UNTRANSLATED))
    == (FLAG_UNMATCHED|FLAG_UNTRANSLATED))
    return YES;

  if (aggressive_import && [[se file] isEqual: DUMMY]
    && [self isMatched: [se key]])
    return YES;

  if (aggressive_remove && ([se flags] & FLAG_UNMATCHED)
    && [self isMatched: [se key]])
    return YES;

  return NO;
}

- (BOOL) _writeString: (NSString *)str toFile: (NSString *)filename
{
  BOOL isAscii = YES;
  NSUInteger len = [str length];
  NSUInteger i;

  for (i = 0; i < len; i++)
    {
      unichar u = [str characterAtIndex: i];
      if (u > 127)
        {
          isAscii = NO;
          break;
        }
    }

  if (isAscii)
    {
      return [str writeToFile: filename atomically: YES];
    }
  else
    {
      NSData *d = [str dataUsingEncoding: NSUTF8StringEncoding];
      NSMutableData *md = [[NSMutableData alloc] initWithCapacity: [d length] + 3];
      // Add BOM at the beginning of the file
      char bytes[] = {0xEF, 0xBB, 0xBF};
      BOOL result;

      [md appendBytes: bytes length: 3];
      [md appendData: d];
      result = [md writeToFile: filename atomically: YES];
      [md release];
      return result;
    }
}

- (BOOL) writeToFile: (NSString *)filename
{
  unsigned int i,c;
  BOOL result;
  NSMutableString *str = [[NSMutableString alloc] initWithCapacity: 32*1024];
  StringsEntry *se;

  NSMutableArray *strings_left = [strings mutableCopy];
  NSMutableArray *str_list = [[NSMutableArray alloc] init];
  NSMutableArray *dup_list = [[NSMutableArray alloc] init];
  NSMutableArray *un_list = [[NSMutableArray alloc] init];

  StringsEntry *cur,*c2;

  int single_file,wrote_banner,unflags;
  int un_count = 0;

  if (global_comment && ![global_comment isEqual: @""])
    {
      [str appendString: @"/*"];
      [str appendString: global_comment];
      [str appendString: @"*/\n\n"];
    }

  [str appendString:
	 [NSString stringWithFormat:
		     @"/***\n"
		   @"%@\n"
		   @"updated by make_strings %@\n"
		   @"add comments above this one\n"
		   @"***/\n",
		   filename,[NSDate dateWithTimeIntervalSinceNow: 0]]];

  wrote_banner = 0;

  /* First, output all keys that appear in multiple places (unless all
     appearances are in one file and none are marked unmatched or untranslated).
     Collect unmatched or untranslated single entries in un_list and matched
     translated (single/multiple in one file) entries in str_list. */
  while ([strings_left count])
    {
      cur = [strings_left objectAtIndex: 0];
      if ([self _shouldIgnore: cur])
	{
	  [strings_left removeObjectAtIndex: 0];
	  continue;
	}
      [dup_list addObject: cur];
      [strings_left removeObjectAtIndex: 0];

      single_file = 1;
      unflags = [cur flags];
      for (i = 0; i < [strings_left count]; i++)
	{
	  c2 = [strings_left objectAtIndex: i];

	  if ([self _shouldIgnore: c2])
	    {
	      [strings_left removeObjectAtIndex: i];
	      i--;
	      continue;
	    }

	  if ([[cur key] isEqual: [c2 key]])
	    {
	      unflags |= [c2 flags];
	      [dup_list addObject: c2];
	      [strings_left removeObjectAtIndex: i];
	      if (single_file)
		if (![[cur file] isEqual: [c2 file]])
		  single_file = 0;
	      i--;
	    }
	}
      if (single_file && !unflags)
	{
	  [str_list addObjectsFromArray: dup_list];
	  [dup_list removeAllObjects];
	  continue;
	}
      if ([dup_list count] == 1)
	{
	  [un_list addObjectsFromArray: dup_list];
	  [dup_list removeAllObjects];
	  continue;
	}

      if (unflags)
	un_count += [dup_list count];

      if (!wrote_banner)
	{
	  [str appendString: @"\n\n/*** Keys found in multiple places ***/\n"];
	  wrote_banner = 1;
	}

      [str appendString: @"\n"];
      [self _writeTo: str  manyEntries: dup_list];
      [dup_list removeAllObjects];
    }

  DESTROY(strings_left);

  /* Now output all single unmatched or untranslated entries. Order by line
     and file so key changes and movements are easy to spot and fix. */
  if ([un_list count])
    {
      [str appendString: @"\n\n/*** Unmatched/untranslated keys ***/\n"];
      [un_list sortUsingSelector: @selector(compareFileLine:)];
      c = [un_list count];
      un_count += c;
      for (i = 0; i < c; i++)
	{
	  se = [un_list objectAtIndex: i];
	  [str appendString: @"\n"];
	  [self _writeTo: str  entryHead: se];
	  [self _writeTo: str  entryFlags: se];
	  [self _writeTo: str  entryKey: se];
	}
    }

  /* Finally, output all matched and translated entries ordered by file. The
     translator should never have to touch these strings (unless there are typos
     or something). */
  if ([str_list count])
    {
      NSString *last_filename = nil;

      [str_list sortUsingSelector: @selector(compareFileKeyComment:)];
      c = [str_list count];
      for (i = 0; i < c; i++)
	{
	  se = [str_list objectAtIndex: i];
	  if (!last_filename || ![last_filename isEqual: [se file]])
	    {
	      last_filename = [se file];
	      [str appendString:
                [NSString stringWithFormat: @"\n\n/*** Strings from %@ ***/\n",
                  last_filename]];
	    }
	  [self _writeTo: str  entryHead: se];
	  if (i == c - 1
            || ![[se key] isEqual: [[str_list objectAtIndex: i + 1] key]])
	    [self _writeTo: str  entryKey: se];
	}
    }
  DESTROY(str_list);
  DESTROY(dup_list);

  {
    NSString *backupname = [filename stringByAppendingString: @"~"];
    [[NSFileManager defaultManager] removeFileAtPath: backupname handler: nil];
    [[NSFileManager defaultManager] movePath: filename
                                      toPath: backupname
                                     handler: nil];
    result = [self _writeString: str toFile: filename];

    if (!result)
      fprintf(stderr,"Error saving '%s'!\n",[filename cString]);
  }

  DESTROY(str);
  DESTROY(un_list);

  if (un_count)
    fprintf(stderr,"'%s': %i untranslated or unmatched messages\n",
      [filename cString],un_count);

  return result;
}


- (void) addSourceEntry: (SourceEntry *)e
{
  /* First try to find a match among our unmatched strings. We consider
     two entries to match if they have the same key, file and comment. This
     could be extended, but the risk of errors increases. */
  int i,c;
  StringsEntry *se;

  c = [strings count];

  [self addMatched: [e key]];

  /* Look for exact matches. If we find an exact match (same file, key, and
     comment) we mark the StringsEntry matched and don't add the SourceEntry.
  */
  for (i = 0; i < c; i++)
    {
      se = [strings objectAtIndex: i];
      if (!([se flags] & FLAG_UNMATCHED))
	continue;

      if (![[se key] isEqual: [e key]])
	continue;

      if (([se flags] & FLAG_UNMATCHED) && [[se file] isEqual: [e file]])
	{
	  if ((![se comment] && ![e comment])
            || ([[se comment] isEqual: [e comment]]))
	    {
	      [se setFlags: [se flags] & ~FLAG_UNMATCHED];
	      [se setLine: [e line]];
	      return;
	    }
	}
    }

  if (aggressive_match)
    {
      /* If aggressive match is enabled, we try to find an existing and
	 translated StringsEntry. If we find we add a new StringsEntry from
	 the SourceEntry with the same translation and marked as translated.
      */
      for (i = 0; i < c; i++)
	{
	  se = [strings objectAtIndex: i];

	  if ([se flags] & FLAG_UNTRANSLATED)
	    continue;

	  if (![[se key] isEqual: [e key]])
	    continue;

	  {
	    StringsEntry *se2 = [StringsEntry stringsEntryFromSourceEntry: e];
	    [se2 setFlags: 0];
	    [se2 setTranslated: [se translated]];
	    [strings addObject: se2];
	    return;
	  }
	}
    }

  /* No match, add a new, untranslated StringsEntry. */
  [strings addObject: [StringsEntry stringsEntryFromSourceEntry: e]];
}

@end

