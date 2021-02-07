/**

   <title>AGSOutput ... a class to output gsdoc source</title>
   Copyright (C) 2001 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: October 2001

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

#import "Foundation/NSArray.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSCharacterSet.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSPathUtilities.h"
#import "Foundation/NSUserDefaults.h"
#import "AGSOutput.h"
#import "GNUstepBase/NSString+GNUstepBase.h"
#import "GNUstepBase/NSMutableString+GNUstepBase.h"

@interface AGSOutput (Private)
- (NSString*) mergeMarkup: (NSString*)markup
		   ofKind: (NSString*)kind;
@end

static NSString *escapeType(NSString *str)
{
  str = [str stringByReplacingString: @"<" withString: @"&lt;"];
  str = [str stringByReplacingString: @">" withString: @"&gt;"];
  return str;
}

static BOOL snuggleEnd(NSString *t)
{
  static NSCharacterSet	*set = nil;

  if ([t hasPrefix: @"</"] == YES)
    {
      return YES;
    }
  if (set == nil)
    {
      set = [NSCharacterSet characterSetWithCharactersInString: @"]}).,;"];
      IF_NO_GC([set retain];)
    }
  return [set characterIsMember: [t characterAtIndex: 0]];
}

static BOOL snuggleStart(NSString *t)
{
  static NSCharacterSet	*set = nil;

  if (set == nil)
    {
      set = [NSCharacterSet characterSetWithCharactersInString: @"[{("];
      IF_NO_GC([set retain];)
    }
  return [set characterIsMember: [t characterAtIndex: [t length] - 1]];
}

/**
 * <unit>
 *  <heading>The AGSOutput class</heading>
 *  <p>This is a really great class ... but it's not really reusable since it's
 *  far too special purpose.</p>
 *  <unit />
 *  <p>Here is the afterword for the class.</p>
 *  <p> And here is some automated cross referencing ...
 *  A method in a protocol: [(NSCopying)-copyWithZone:], a class:
 *  [NSString], a protocol: [(NSCopying)], and a
 *  category: [NSRunLoop(GNUstepExtensions)].
 *  </p>
 * </unit>
 * And finally, here is the actual class description ... outside the chapter.
 * This is the class description for <code>AGSOutput</code>, including some
 * sample uses of GSDoc, such as cross-references (see [NSString]).
 * Functions, like escapeType(), are automatically referenced (if they are
 * found).
 */
@implementation	AGSOutput

- (void) appendVersions: (NSString*)versions to: (NSMutableString*)str
{
  if ([versions length] > 0)
    {
      [str appendString: versions];
    }
}

- (NSString*) checkComment: (NSString*)comment
		      unit: (NSString*)unit
		      info: (NSMutableDictionary*)d
{
  NSString	*empty = [d objectForKey: @"Empty"];
  BOOL		hadComment = ([comment length] == 0 ? NO : YES);

  if (hadComment == NO)
    {
      comment = @"<em>Description forthcoming.</em>";
      if (warn == YES)
	{
	  NSString	*name = [d objectForKey: @"Name"];
	  NSString	*type = [d objectForKey: @"Type"];

	  if (unit == nil)
	    {
	      if (type == nil)
	        {
		  NSLog(@"Warning - No comments for %@", name);
		}
	      else
	        {
		  NSLog(@"Warning - No comments for %@ %@", type, name);
		}
	    }
	  else
	    {
	      if ([d objectForKey: @"ReturnType"] != nil)
		{
		  NSLog(@"Warning - No comments for [%@ %@]", unit, name);
		}
	      else
		{
		  NSLog(@"Warning - No comments for instance variable %@ in %@",
		    name, unit);
		}
	    }
	}
    }

  if (empty != nil && [empty boolValue] == YES)
    {
#if 0
      static NSString	*today = nil;

      if (today == nil)
	{
	  NSCalendarDate	*d = [NSCalendarDate date];

	  today
	    = RETAIN([d descriptionWithCalendarFormat: @"%d-%m-%Y"]);
	}
      if (hadComment == NO)
	{
	  comment = @"";
	}
      comment = [NSString stringWithFormat:
	@"<em>Not implemented (as of %@).</em><br />"
	@"Please help us by producing an implementation of this "
	@"and donating it to the GNUstep project.<br />"
	@"You can check the task manager at "
	@"https://savannah.gnu.org/projects/gnustep "
	@"to see if anyone is already working on it.<br />",
	today, comment];
#else
      NSString	*name = [d objectForKey: @"Name"];

      NSLog(@"Warning - No implementation for [%@ %@]", unit, name);
#endif
    }

  return comment;
}

- (void) dealloc
{
  DESTROY(identifier);
  DESTROY(identStart);
  DESTROY(spaces);
  DESTROY(spacenl);
  DESTROY(informalProtocols);
  [super dealloc];
}

- (unsigned) fitWords: (NSArray*)a
		 from: (unsigned)start
		   to: (unsigned)end
	      maxSize: (unsigned)limit
	       output: (NSMutableString*)buf
{
  unsigned	size = 0;
  unsigned	nest = 0;
  unsigned	i;
  int		lastOk = -1;
  BOOL		addSpace = NO;

  for (i = start; size < limit && i < end; i++)
    {
      NSString	*t = [a objectAtIndex: i];
      BOOL	forceNewline = [t hasPrefix: @"<p>"];
      BOOL	elementEndReached = (nest == 0 && [t hasPrefix: @"</"] == YES); 

      if (elementEndReached || forceNewline)
	{
	  break;
	}

      /*
       * Check sizing and output this word if necessary.
       */
      if (addSpace == YES && snuggleEnd(t) == NO)
	{
	  size++;
	  if (buf != nil)
	    {
	      [buf appendString: @" "];
	    }
	}
      size += [t length];
      if (buf != nil)
	{
	  [buf appendString: t];
	}

      /*
       * Determine nesting level changes produced by this word, and
       * whether we need a space before the next word.
       */
      if ([t hasPrefix: @"</"] == YES)
	{
	  nest--;
	  addSpace = YES;
	}
      else if ([t hasPrefix: @"<"] == YES)
	{
	  if ([t hasSuffix: @"/>"] == YES)
	    {
	      addSpace = YES;
	    }
	  else
	    {
	      nest++;
	      addSpace = NO;
	    }
	}
      else
	{
	  if (snuggleStart(t) == NO)
	    {
	      addSpace = YES;
	    }
	  else
	    {
	      addSpace = NO;
	    }
	}

      /*
       * Record whether the word we just checked was at nesting level 0
       * and had not exceeded the line length limit.
       */
      if (nest == 0 && size <= limit)
	{
	  lastOk = i;
	}
    }
  return lastOk + 1;
}

- (id) init
{
  NSMutableCharacterSet	*m;

  m = [[NSCharacterSet controlCharacterSet] mutableCopy];
  [m addCharactersInString: @" "];
  spacenl = [m copy];
  [m removeCharactersInString: @"\n"];
  spaces = [m copy];
  RELEASE(m);
  identifier = RETAIN([NSCharacterSet characterSetWithCharactersInString:
    @"_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"]);
  identStart = RETAIN([NSCharacterSet characterSetWithCharactersInString:
    @"_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"]);
  informalProtocols = [NSMutableArray new];
  verbose = [[NSUserDefaults standardUserDefaults] boolForKey: @"Verbose"];
  warn = [[NSUserDefaults standardUserDefaults] boolForKey: @"Warn"];

  return self;
}

/**
 * Return an array containing the names of any files modified as
 * a result of outputing the specified data structure.
 */
- (NSArray*) output: (NSMutableDictionary*)d
{
  NSMutableString	*str = [NSMutableString stringWithCapacity: 10240];
  NSDictionary		*classes;
  NSDictionary		*categories;
  NSDictionary		*protocols;
  NSDictionary		*functions;
  NSDictionary		*types;
  NSDictionary		*variables;
  NSDictionary		*constants;
  NSDictionary		*macros;
  NSMutableArray	*files;
  NSArray		*authors;
  NSString		*base;
  NSString		*tmp;
  NSString		*file;
  NSString		*dest;
  unsigned		chapters = 0;

  files = [NSMutableArray arrayWithCapacity: 5];

  info = d;
  base = [info objectForKey: @"base"];
  file = base;
  if ([[file pathExtension] isEqualToString: @"gsdoc"] == NO)
    {
      file = [file stringByAppendingPathExtension: @"gsdoc"];
    }
  dest = [info objectForKey: @"directory"];
  if ([dest length] > 0 && [file isAbsolutePath] == NO)
    {
      file = [dest stringByAppendingPathComponent: file];
    }

  classes = [info objectForKey: @"Classes"];
  categories = [info objectForKey: @"Categories"];
  protocols = [info objectForKey: @"Protocols"];
  functions = [info objectForKey: @"Functions"];
  types = [info objectForKey: @"Types"];
  variables = [info objectForKey: @"Variables"];
  constants = [info objectForKey: @"Constants"];
  macros = [info objectForKey: @"Macros"];

  [str appendString: @"<?xml version=\"1.0\"?>\n"];
  [str appendString: @"<!DOCTYPE gsdoc PUBLIC "];
  [str appendString: @"\"-//GNUstep//DTD gsdoc 1.0.4//EN\" "];
  [str appendString: @"\"http://www.gnustep.org/gsdoc-1_0_4.dtd\">\n"];
  [str appendFormat: @"<gsdoc"];

  if (base != nil)
    {
      [str appendString: @" base=\""];
      [str appendString: base];
      [str appendString: @"\""];
    }

  tmp = [info objectForKey: @"up"];
  if (tmp != nil)
    {
      [str appendString: @" up=\""];
      [str appendString: tmp];
      [str appendString: @"\""];
    }

  [str appendString: @">\n"];
  [str appendString: @"  <head>\n"];

  /*
   * A title is mandatory in the head element ... obtain it
   * from the info dictionary.  Guess at a title if necessary.
   */
  tmp = [info objectForKey: @"title"];
  if (tmp != nil)
    {
      [self reformat: tmp withIndent: 4 to: str];
    }
  else
    {
      [str appendString: @"    <title>"];
      if ([classes count] == 1)
	{
	  [str appendString: [[classes allKeys] lastObject]];
	  [str appendString: @" class documentation"];
	}
      else
	{
	  [str appendFormat: @"%@ documentation",
	    [info objectForKey: @"base"]];
	}
      [str appendString: @"</title>\n"];
    }

  /*
   * The author element is compulsory ... fill in.
   */
  authors = [info objectForKey: @"authors"];
  if (authors == nil)
    {
      tmp = [NSString stringWithFormat: @"Generated by %@", NSUserName()];
      [str appendString: @"    <author name=\""];
      [str appendString: tmp];
      [str appendString: @"\"></author>\n"];
    }
  else
    {
      unsigned	i;

      for (i = 0; i < [authors count]; i++)
	{
	  NSString	*author = [authors objectAtIndex: i];

	  [self reformat: author withIndent: 4 to: str];
	}
    }

  /*
   * The version element is optional ... fill in if available.
   */
  tmp = [info objectForKey: @"version"];
  if (tmp != nil)
    {
      [self reformat: tmp withIndent: 4 to: str];
    }

  /*
   * The date element is optional ... fill in if available.
   */
  tmp = [info objectForKey: @"date"];
  if (tmp != nil)
    {
      [self reformat: tmp withIndent: 4 to: str];
    }

  /*
   * The abstract element is optional ... fill in if available.
   */
  tmp = [info objectForKey: @"abstract"];
  if (tmp != nil)
    {
      [self reformat: tmp withIndent: 4 to: str];
    }

  /*
   * The copy element is optional ... fill in if available.
   */
  tmp = [info objectForKey: @"copy"];
  if (tmp != nil)
    {
      [self reformat: tmp withIndent: 4 to: str];
    }

  [str appendString: @"  </head>\n"];
  [str appendString: @"  <body>\n"];

  // Output document forward if available.
  tmp = [info objectForKey: @"front"];
  if (tmp == nil)
    {
      [self reformat: @"<front><contents /></front>" withIndent: 4 to: str];
    }
  else
    {
      [self reformat: tmp withIndent: 4 to: str];
    }

  // Output document main chapter if available
  tmp = [info objectForKey: @"chapter"];
  if (tmp != nil)
    {
      [self reformat: tmp withIndent: 4 to: str];
      chapters++;
    }

  if ([classes count] > 0)
    {
      NSArray	*names;
      unsigned	i;
      unsigned	c = [classes count];

      chapters += c;
      names = [classes allKeys];
      names = [names sortedArrayUsingSelector: @selector(compare:)];
      for (i = 0; i < c; i++)
	{
	  NSString		*name = [names objectAtIndex: i];
	  NSMutableDictionary	*d = [classes objectForKey: name];

	  [self outputUnit: d to: str];
	}
    }

  if ([categories count] > 0)
    {
      NSArray	*names;
      unsigned	i;
      unsigned	c = [categories count];

      chapters += c;
      names = [categories allKeys];
      names = [names sortedArrayUsingSelector: @selector(compare:)];
      for (i = 0; i < c; i++)
	{
	  NSString		*name = [names objectAtIndex: i];
	  NSMutableDictionary	*d = [categories objectForKey: name];

	  [self outputUnit: d to: str];
	}
    }

  if ([protocols count] > 0)
    {
      NSArray	*names;
      unsigned	i;
      unsigned	c = [protocols count];

      chapters += c;
      names = [protocols allKeys];
      names = [names sortedArrayUsingSelector: @selector(compare:)];
      for (i = 0; i < c; i++)
	{
	  NSString		*name = [names objectAtIndex: i];
	  NSMutableDictionary	*d = [protocols objectForKey: name];

	  [self outputUnit: d to: str];
	}
    }

  if ([types count] > 0)
    {
      NSMutableString	*m = [NSMutableString new];
      NSArray		*names;
      unsigned		i;
      unsigned		c = [types count];

      [m appendString: @"    <chapter>\n"];
      [m appendFormat: @"      <heading>%@ types</heading>\n", base];
      [m appendString: @"      <p></p>\n"];

      names = [types allKeys];
      names = [names sortedArrayUsingSelector: @selector(compare:)];
      for (i = 0; i < c; i++)
	{
	  NSString		*name = [names objectAtIndex: i];
	  NSMutableDictionary	*d = [types objectForKey: name];

	  [self outputDecl: d kind: @"type" to: m];
	}

      [m appendString: @"    </chapter>\n"];

      tmp = [self mergeMarkup: m ofKind: @"Typedefs"];
      if (tmp == nil)
	{
	  [str appendString: m];
	  chapters++;
	}
      else
	{
	  [files addObject: tmp];
	}
      RELEASE(m);
    }

  if ([constants count] > 0)
    {
      NSMutableString	*m = [NSMutableString new];
      NSArray		*names;
      unsigned		i;
      unsigned		c = [constants count];

      [m appendString: @"    <chapter>\n"];
      [m appendFormat: @"      <heading>%@ constants</heading>\n", base];
      [m appendString: @"      <p></p>\n"];

      names = [constants allKeys];
      names = [names sortedArrayUsingSelector: @selector(compare:)];
      for (i = 0; i < c; i++)
	{
	  NSString		*name = [names objectAtIndex: i];
	  NSMutableDictionary	*d = [constants objectForKey: name];

	  [self outputDecl: d kind: @"constant" to: m];
	}

      [m appendString: @"    </chapter>\n"];

      tmp = [self mergeMarkup: m ofKind: @"Constants"];
      if (tmp == nil)
	{
	  [str appendString: m];
	  chapters++;
	}
      else
	{
	  [files addObject: tmp];
	}
      RELEASE(m);
    }

  if ([macros count] > 0)
    {
      NSMutableString	*m = [NSMutableString new];
      NSArray		*names;
      unsigned		i;
      unsigned		c = [macros count];

      [m appendString: @"    <chapter>\n"];
      [m appendFormat: @"      <heading>%@ macros</heading>\n", base];
      [m appendString: @"      <p></p>\n"];

      names = [macros allKeys];
      names = [names sortedArrayUsingSelector: @selector(compare:)];
      for (i = 0; i < c; i++)
	{
	  NSString		*name = [names objectAtIndex: i];
	  NSMutableDictionary	*d = [macros objectForKey: name];

	  [self outputMacro: d to: m];
	}

      [m appendString: @"    </chapter>\n"];

      tmp = [self mergeMarkup: m ofKind: @"Macros"];
      if (tmp == nil)
	{
	  [str appendString: m];
	  chapters++;
	}
      else
	{
	  [files addObject: tmp];
	}
      RELEASE(m);
    }

  if ([variables count] > 0)
    {
      NSMutableString	*m = [NSMutableString new];
      NSArray		*names;
      unsigned		i;
      unsigned		c = [variables count];

      [m appendString: @"    <chapter>\n"];
      [m appendFormat: @"      <heading>%@ variables</heading>\n", base];
      [m appendString: @"      <p></p>\n"];

      names = [variables allKeys];
      names = [names sortedArrayUsingSelector: @selector(compare:)];
      for (i = 0; i < c; i++)
	{
	  NSString		*name = [names objectAtIndex: i];
	  NSMutableDictionary	*d = [variables objectForKey: name];

	  [self outputDecl: d kind: @"variable" to: m];
	}

      [m appendString: @"    </chapter>\n"];

      tmp = [self mergeMarkup: m ofKind: @"Variables"];
      if (tmp == nil)
	{
	  [str appendString: m];
	  chapters++;
	}
      else
	{
	  [files addObject: tmp];
	}
      RELEASE(m);
    }

  if ([functions count] > 0)
    {
      NSMutableString	*m = [NSMutableString new];
      NSArray		*names;
      unsigned		i;
      unsigned		c = [functions count];

      [m appendString: @"    <chapter>\n"];
      [m appendFormat: @"      <heading>%@ functions</heading>\n", base];
      [m appendString: @"      <p></p>\n"];

      names = [functions allKeys];
      names = [names sortedArrayUsingSelector: @selector(compare:)];
      for (i = 0; i < c; i++)
	{
	  NSString		*name = [names objectAtIndex: i];
	  NSMutableDictionary	*d = [functions objectForKey: name];

	  [self outputFunction: d to: m];
	}

      [m appendString: @"    </chapter>\n"];
      tmp = [self mergeMarkup: m ofKind: @"Functions"];
      if (tmp == nil)
	{
	  [str appendString: m];
	  chapters++;
	}
      else
	{
	  [files addObject: tmp];
	}
      RELEASE(m);
    }

  if (chapters > 0)
    {
      NSData	*d;

      // Output document appendix if available.
      tmp = [info objectForKey: @"back"];
      if (tmp != nil)
	{
	  [self reformat: tmp withIndent: 4 to: str];
	}

      [str appendString: @"  </body>\n"];
      [str appendString: @"</gsdoc>\n"];
      d = [str dataUsingEncoding: NSUTF8StringEncoding];
      if ([d writeToFile: file atomically: YES] == YES)
	{
	  [files addObject: file];
	}
      else
	{
	  files = nil;
	}
    }
  return files;
}

/**
 * Uses -split: and -reformat:withIndent:to:.
 */
- (void) outputDecl: (NSMutableDictionary*)d
	       kind: (NSString*)kind
		 to: (NSMutableString*)str
{
  NSString	*pref = [d objectForKey: @"Prefix"];
  NSString	*type = [d objectForKey: @"BaseType"];
  NSString	*name = [d objectForKey: @"Name"];
  NSString	*comment = [d objectForKey: @"Comment"];
  NSString	*declared = [d objectForKey: @"Declared"];

  if (warn == YES && [[d objectForKey: @"Implemented"] isEqual: @"YES"] == NO)
    {
      NSLog(@"Warning ... %@ %@ is not implemented where expected", kind, name);
    }

  [str appendFormat: @"      <%@ type=\"", kind];
  [str appendString: escapeType(type)];
  if ([pref length] > 0)
    {
      [str appendString: pref];
    }
  [str appendString: @"\" name=\""];
  [str appendString: name];
  [str appendString: @"\""];
  [self appendVersions: [d objectForKey: @"Versions"] to: str];
  [str appendString: @">\n"];

  if (declared != nil)
    {
      [str appendString: @"        <declared>"];
      [str appendString: declared];
      [str appendString: @"</declared>\n"];
    }

  [str appendString: @"        <desc>\n"];
  comment = [self checkComment: comment unit: nil info: d];
  [self reformat: comment withIndent: 10 to: str];
  [str appendString: @"        </desc>\n"];
  [str appendFormat: @"      </%@>\n", kind];
}

/**
 * Uses -split: and -reformat:withIndent:to:.
 */
- (void) outputFunction: (NSMutableDictionary*)d to: (NSMutableString*)str
{
  NSArray	*aa = [d objectForKey: @"Args"];
  NSString	*pref = [d objectForKey: @"Prefix"];
  NSString	*type = [d objectForKey: @"BaseType"];
  NSString	*name = [d objectForKey: @"Name"];
  NSString	*comment = [d objectForKey: @"Comment"];
  NSString	*declared = [d objectForKey: @"Declared"];
  unsigned	i = [aa count];

  if (warn == YES && [[d objectForKey: @"Implemented"] isEqual: @"YES"] == NO)
    {
      NSLog(@"Warning ... function %@ is not implemented where expected", name);
    }

  /**
   * Place the names of function arguments in a temporary array 'args'
   * so that they will be highlighted if they appear in the function
   * description.
   */
  if (i > 0)
    {
      NSMutableArray	*tmp = [NSMutableArray arrayWithCapacity: i];

      while (i-- > 0)
	{
	  NSString	*n;
	  NSDictionary	*d;

	  d = [aa objectAtIndex: i];
	  n = [d objectForKey: @"Name"];
	  if (n != nil)
	    {
	      [tmp addObject: n];
	    }
	}
      if ([tmp count] > 0)
	{
	  args = tmp;
	}
    }

  [str appendString: @"      <function type=\""];
  [str appendString: escapeType(type)];
  if ([pref length] > 0)
    {
      [str appendString: pref];
    }
  [str appendString: @"\" name=\""];
  [str appendString: name];
  [str appendString: @"\""];
  [self appendVersions: [d objectForKey: @"Versions"] to: str];
  [str appendString: @">\n"];

  for (i = 0; i < [aa count]; i++)
    {
      NSDictionary	*a = [aa objectAtIndex: i];
      NSString		*s = [a objectForKey: @"BaseType"];

      [str appendString: @"        <arg type=\""];
      [str appendString: escapeType(s)];
      s = [a objectForKey: @"Prefix"];
      if (s != nil)
	{
	  [str appendString: escapeType(s)];
	}
      s = [a objectForKey: @"Suffix"];
      if (s != nil)
	{
	  [str appendString: escapeType(s)];
	}
      [str appendString: @"\">"];
      [str appendString: [a objectForKey: @"Name"]];
      [str appendString: @"</arg>\n"];
    }
  if ([[d objectForKey: @"VarArgs"] boolValue] == YES)
    {
      [str appendString: @"        <vararg />\n"];
    }

  if (declared != nil)
    {
      [str appendString: @"        <declared>"];
      [str appendString: declared];
      [str appendString: @"</declared>\n"];
    }

  [str appendString: @"        <desc>\n"];
  comment = [self checkComment: comment unit: nil info: d];
  [self reformat: comment withIndent: 10 to: str];
  [str appendString: @"        </desc>\n"];
  [str appendString: @"      </function>\n"];
  args = nil;
}

/**
 * Output the gsdoc code for an instance variable.
 */
- (void) outputInstanceVariable: (NSMutableDictionary*)d
			     to: (NSMutableString*)str
			    for: (NSString*)unit
{
  NSString	*pref = [d objectForKey: @"Prefix"];
  NSString	*type = [d objectForKey: @"BaseType"];
  NSString	*validity = [d objectForKey: @"Validity"];
  NSString	*name = [d objectForKey: @"Name"];
  NSString	*comment = [d objectForKey: @"Comment"];

  [str appendString: @"        <ivariable type=\""];
  [str appendString: escapeType(type)];
  if ([pref length] > 0)
    {
      [str appendString: pref];
    }
  [str appendString: @"\" name=\""];
  [str appendString: name];
  if (validity != nil)
    {
      [str appendString: @"\" validity=\""];
      [str appendString: validity];
    }
  [str appendString: @"\""];
  [self appendVersions: [d objectForKey: @"Versions"] to: str];
  [str appendString: @">\n"];

  [str appendString: @"          <desc>\n"];
  comment = [self checkComment: comment unit: unit info: d];
  [self reformat: comment withIndent: 12 to: str];
  [str appendString: @"          </desc>\n"];
  [str appendString: @"        </ivariable>\n"];
}

/**
 * Uses -split: and -reformat:withIndent:to:.
 */
- (void) outputMacro: (NSMutableDictionary*)d
		  to: (NSMutableString*)str
{
  NSString	*name = [d objectForKey: @"Name"];
  NSString	*comment = [d objectForKey: @"Comment"];
  NSString	*declared = [d objectForKey: @"Declared"];
  unsigned	i;

  [str appendString: @"      <macro name=\""];
  [str appendString: name];
  [str appendString: @"\""];
  [self appendVersions: [d objectForKey: @"Versions"] to: str];
  [str appendString: @">\n"];

  /*
   * Storing the argument names array in the 'args' ivar ensures that
   * when we output comments, the argument names are highlighted.
   */
  args = [d objectForKey: @"Args"];
  for (i = 0; i < [args count]; i++)
    {
      NSString		*s = [args objectAtIndex: i];

      [str appendString: @"        <arg>"];
      [str appendString: s];
      [str appendString: @"</arg>\n"];
    }
  if ([[d objectForKey: @"VarArgs"] boolValue] == YES)
    {
      [str appendString: @"        <vararg />\n"];
    }

  if (declared != nil)
    {
      [str appendString: @"        <declared>"];
      [str appendString: declared];
      [str appendString: @"</declared>\n"];
    }

  [str appendString: @"        <desc>\n"];
  comment = [self checkComment: comment unit: nil info: d];
  [self reformat: comment withIndent: 10 to: str];
  [str appendString: @"        </desc>\n"];
  [str appendString: @"      </macro>\n"];
  args = nil;
}

/**
 * Uses -split: and -reformat:withIndent:to:.
 * Also has fun with YES, NO, and nil.
 */
- (void) outputMethod: (NSMutableDictionary*)d
		   to: (NSMutableString*)str
		  for: (NSString*)unit
{
  NSArray	*sels = [d objectForKey: @"Sels"];
  NSArray	*types = [d objectForKey: @"Types"];
  NSString	*name = [d objectForKey: @"Name"];
  NSString	*comment;
  unsigned	i;
  BOOL		isInitialiser = NO;
  NSString	*override = nil;

  if (warn == YES && unit != nil
    && [[d objectForKey: @"Implemented"] isEqual: @"YES"] == NO)
    {
      NSLog(@"Warning ... method %@ %@ is not implemented where expected",
        unit, name);
    }

  args = [d objectForKey: @"Args"];	// Used when splitting.

  comment = [d objectForKey: @"Comment"];

  /**
   * Check special markup which should be removed from the text
   * actually placed in the gsdoc method documentation ... the
   * special markup is included in the gsdoc markup differently.
   */
  if (comment != nil)
    {
      NSMutableString	*m = nil;
      NSRange		r;

      do
	{
	  r = [comment rangeOfString: @"<init />"];
	  if (r.length == 0)
	    r = [comment rangeOfString: @"<init/>"];
	  if (r.length == 0)
	    r = [comment rangeOfString: @"<init>"];
	  if (r.length > 0)
	    {
	      if (m == nil)
		{
		  m = [comment mutableCopy];
		}
	      [m deleteCharactersInRange: r];
	      comment = m;
	      isInitialiser = YES;
	    }
	} while (r.length > 0);
      do
	{
	  r = [comment rangeOfString: @"<override-subclass />"];
	  if (r.length == 0)
	    r = [comment rangeOfString: @"<override-subclass/>"];
	  if (r.length == 0)
	    r = [comment rangeOfString: @"<override-subclass>"];
	  if (r.length > 0)
	    {
	      if (m == nil)
		{
		  m = [comment mutableCopy];
		}
	      [m deleteCharactersInRange: r];
	      comment = m;
	      override = @"subclass";
	      /*
	       * If a method should be overridden by subclasses,
	       * we don't treat it as unimplemented.
	       */
	      [d setObject: @"NO" forKey: @"Empty"];
	    }
	} while (r.length > 0);
      do
	{
	  r = [comment rangeOfString: @"<override-dummy />"];
	  if (r.length == 0)
	    r = [comment rangeOfString: @"<override-dummy/>"];
	  if (r.length == 0)
	    r = [comment rangeOfString: @"<override-dummy>"];
	  if (r.length > 0)
	    {
	      if (m == nil)
		{
		  m = [comment mutableCopy];
		}
	      [m deleteCharactersInRange: r];
	      comment = m;
	      override = @"dummy";
	      /*
	       * If a method should be overridden by subclasses,
	       * we don't treat it as unimplemented.
	       */
	      [d setObject: @"NO" forKey: @"Empty"];
	    }
	} while (r.length > 0);
      do
	{
	  r = [comment rangeOfString: @"<override-never />"];
	  if (r.length == 0)
	    r = [comment rangeOfString: @"<override-never/>"];
	  if (r.length == 0)
	    r = [comment rangeOfString: @"<override-never>"];
	  if (r.length > 0)
	    {
	      if (m == nil)
		{
		  m = [comment mutableCopy];
		}
	      [m deleteCharactersInRange: r];
	      comment = m;
	      override = @"never";
	    }
	} while (r.length > 0);
      if (m != nil)
	{
	  IF_NO_GC([m autorelease];)
	}
    }

  [str appendString: @"        <method type=\""];
  [str appendString: escapeType([d objectForKey: @"ReturnType"])];
  if ([name hasPrefix: @"+"] == YES)
    {
      [str appendString: @"\" factory=\"yes"];
    }
  if (isInitialiser == YES)
    {
      [str appendString: @"\" init=\"yes"];
    }
  if (override != nil)
    {
      [str appendString: @"\" override=\""];
      [str appendString: override];
    }
  [str appendString: @"\""];
  [self appendVersions: [d objectForKey: @"Versions"] to: str];
  [str appendString: @">\n"];

  for (i = 0; i < [sels count]; i++)
    {
      [str appendString: @"          <sel>"];
      [str appendString: [sels objectAtIndex: i]];
      [str appendString: @"</sel>\n"];
      if (i < [args count])
	{
	  [str appendString: @"          <arg type=\""];
	  [str appendString: escapeType([types objectAtIndex: i])];
	  [str appendString: @"\">"];
	  [str appendString: [args objectAtIndex: i]];
	  [str appendString: @"</arg>\n"];
	}
    }
  if ([[d objectForKey: @"VarArgs"] boolValue] == YES)
    {
      [str appendString: @"          <vararg />\n"];
    }

  [str appendString: @"          <desc>\n"];
  comment = [self checkComment: comment unit: unit info: d];
  [self reformat: comment withIndent: 12 to: str];
  [str appendString: @"          </desc>\n"];
  [str appendString: @"        </method>\n"];
  args = nil;
}

- (void) outputUnit: (NSMutableDictionary*)d to: (NSMutableString*)str
{
  NSString	*name = [d objectForKey: @"Name"];
  NSString	*type = [d objectForKey: @"Type"];
  NSString	*kind = type;
  NSMutableDictionary	*methods = [d objectForKey: @"Methods"];
  NSMutableDictionary	*ivars = [d objectForKey: @"InstanceVariables"];
  NSString	*comment = [d objectForKey: @"Comment"];
  NSString	*unitName = nil;
  NSArray	*names;
  NSArray	*protocols;
  NSString	*tmp;
  NSString	*unit = nil;
  NSRange	r;
  unsigned	ind;
  unsigned	i;
  unsigned	j;

  if ([[d objectForKey: @"Implemented"] isEqual: @"YES"] == NO)
    {
      if ([name hasPrefix:  @"NSObject("] == YES)
	{
	  NSEnumerator		*e = [methods objectEnumerator];
	  NSMutableDictionary	*m;

	  /*
	   * Assume an unimplemented category of NSObject is an
	   * informal protocol, and stop warnings being issued
	   * about unimplemented methods.
	   */
	  unitName = name;
	  kind = @"informal protocol";
	  while ((m = [e nextObject]) != nil)
	    {
	      [m setObject: @"YES" forKey: @"Implemented"];
	    }

          [informalProtocols addObject: name];
	}
      else if (warn == YES)
	{
	  NSLog(@"Warning ... unit %@ is not implemented where expected", name);
	}
    }
  else
    {
      unitName = name;
    }

  /*
   * Make sure we have a 'unit' part and a class 'desc' part (comment)
   * to be output.
   */
  if (comment != nil)
    {
      r = [comment rangeOfString: @"<unit>"];
      if (r.length > 0)
	{
	  unsigned	pos = r.location;

	  r = [comment rangeOfString: @"</unit>"];
	  if (r.length == 0 || r.location < pos)
	    {
	      NSLog(@"Unterminated <unit> in comment for %@", name);
	      return;
	    }
	
	  if (pos == 0)
	    {
	      if (NSMaxRange(r) == [comment length])
		{
		  unit = comment;
		  comment = nil;
		}
	      else
		{
		  unit = [comment substringToIndex: NSMaxRange(r)];
		  comment = [comment substringFromIndex: NSMaxRange(r)];
		}
	    }
	  else
	    {
	      if (NSMaxRange(r) == [comment length])
		{
		  unit = [comment substringFromIndex: pos];
		  comment = [comment substringToIndex: pos];
		}
	      else
		{
		  unsigned	end = NSMaxRange(r);

		  r = NSMakeRange(pos, end-pos);
		  unit = [comment substringWithRange: r];
		  comment = [[comment substringToIndex: pos]
		    stringByAppendingString: [comment substringFromIndex: end]];
		}
	    }
	  unit = [unit stringByReplacingString: @"unit>"
				    withString: @"chapter>"];
	}
    }
  if (unit == nil)
    {
      unit = [NSString stringWithFormat:
        @"    <chapter>\n      <heading>Software documentation "
	@"for the %@ %@</heading>\n    </chapter>\n", name, kind];
    }

  /*
   * Get the range of the location in the chapter where the class
   * details should get substituted in.  If there is nowhere marked,
   * create a zero length range just before the end of the chapter.
   */
  r = [unit rangeOfString: @"<unit />"];
  if (r.length == 0)
    {
      r = [unit rangeOfString: @"<unit/>"];
    }
  if (r.length == 0)
    {
      r = [unit rangeOfString: @"</chapter>"];
      r.length = 0;
    }

  /*
   * Output first part of chapter and note indentation.
   */
  ind = [self reformat: [unit substringToIndex: r.location]
	    withIndent: 4
		    to: str];

  for (j = 0; j < ind; j++) [str appendString: @" "];
  [str appendString: @"<"];
  [str appendString: type];
  [str appendString: @" name=\""];
  if ([type isEqual: @"category"] == YES)
    {
      [str appendString: [d objectForKey: @"Category"]];
    }
  else
    {
      [str appendString: name];
    }
  tmp = [d objectForKey: @"BaseClass"];
  if (tmp != nil)
    {
      if ([type isEqual: @"class"] == YES)
	{
	  [str appendString: @"\" super=\""];
	}
      else if ([type isEqual: @"category"] == YES)
	{
	  [str appendString: @"\" class=\""];
	}
      [str appendString: tmp];
    }
  [str appendString: @"\""];
  [self appendVersions: [d objectForKey: @"Versions"] to: str];
  [str appendString: @">\n"];

  ind += 2;
  for (j = 0; j < ind; j++) [str appendString: @" "];
  [str appendString: @"<declared>"];
  [str appendString: [d objectForKey: @"Declared"]];
  [str appendString: @"</declared>\n"];

  protocols = [d objectForKey: @"Protocols"];
  if ([protocols count] > 0)
    {
      for (i = 0; i < [protocols count]; i++)
	{
	  for (j = 0; j < ind; j++) [str appendString: @" "];
	  [str appendString: @"<conform>"];
	  [str appendString: [protocols objectAtIndex: i]];
	  [str appendString: @"</conform>\n"];
	}
    }

  for (j = 0; j < ind; j++) [str appendString: @" "];
  [str appendString: @"<desc>\n"];
  comment = [self checkComment: comment unit: nil info: d];
  [self reformat: comment withIndent: ind + 2 to: str];
  for (j = 0; j < ind; j++) [str appendString: @" "];
  [str appendString: @"</desc>\n"];

  names = [[ivars allKeys] sortedArrayUsingSelector: @selector(compare:)];
  for (i = 0; i < [names count]; i++)
    {
      NSString	*vName = [names objectAtIndex: i];

      [self outputInstanceVariable: [ivars objectForKey: vName]
				to: str
			       for: unitName];
    }

  names = [[methods allKeys] sortedArrayUsingSelector: @selector(compare:)];
  for (i = 0; i < [names count]; i++)
    {
      NSString	*mName = [names objectAtIndex: i];

      [self outputMethod: [methods objectForKey: mName]
     		      to: str
		     for: unitName];
    }

  ind -= 2;
  for (j = 0; j < ind; j++) [str appendString: @" "];
  [str appendString: @"</"];
  [str appendString: type];
  [str appendString: @">\n"];

  /*
   * Output tail end of chapter.
   */
  [self reformat: [unit substringFromIndex: NSMaxRange(r)]
      withIndent: ind
	      to: str];
}

- (unsigned) reformat: (NSString*)str
	   withIndent: (unsigned)ind
		   to: (NSMutableString*)buf
{
  ENTER_POOL
  unsigned	l = [str length];
  NSRange	r = [str rangeOfString: @"<example"];
  unsigned	i = 0;
  NSArray	*a;

  /*
   * Split out <example>...</example> sequences and output them literally.
   * All other text has reformatting applied as necessary.
   */
  while (r.length > 0)
    {
      NSString	*tmp;

      if (r.location > i)
	{
	  /*
	   * There was some text before the example - call this method
	   * recursively to format and output it.
	   */
	  tmp = [str substringWithRange: NSMakeRange(i, r.location - i)];
	  [self reformat: tmp withIndent: ind to: buf];
	  i = r.location;
	}
      /*
       * Now find the end of the example, and output the whole example
       * literally as it appeared in the comment.
       */
      r = [str rangeOfString: @"</example>"
		     options: NSLiteralSearch
		       range: NSMakeRange(i, l - i)];
      if (r.length == 0)
	{
	  NSLog(@"unterminated <example>");
	  return ind;
	}
      tmp = [str substringWithRange: NSMakeRange(i, NSMaxRange(r) - i)];
      [buf appendString: tmp];
      [buf appendString: @"\n"];
      /*
       * Set up the start location and search for another example so
       * we will loop round again if necessary.
       */
      i = NSMaxRange(r);
      r = [str rangeOfString: @"<example"
		     options: NSLiteralSearch
		       range: NSMakeRange(i, l - i)];
    }

  /*
   * If part of the string has already been consumed, just use
   * the remaining substring.
   */
  if (i > 0)
    {
      str = [str substringWithRange: NSMakeRange(i, l - i)];
    }

  /*
   * Split the string up into parts separated by newlines.
   */
  a = [self split: str];
  for (i = 0; i < [a count]; i++)
    {
      unsigned int	j;

      str = [a objectAtIndex: i];

      if ([str hasPrefix: @"</"] == YES)
	{
	  if (ind > 2)
	    {
	      /*
	       * decrement indentation after the end of an element.
	       */
	      ind -= 2;
	    }
	  for (j = 0; j < ind; j++)
	    {
	      [buf appendString: @" "];
	    }
	  [buf appendString: str];
	  [buf appendString: @"\n"];
	}
      else
	{
	  unsigned	size = 70 - ind - [str length];
	  unsigned	end;

	  for (j = 0; j < ind; j++)
	    {
	      [buf appendString: @" "];
	    }
	  end = [self fitWords: a
			  from: i
			    to: [a count]
		       maxSize: size
			output: nil];
	  if (end <= i)
	    {
	      [buf appendString: str];
	      if ([str hasPrefix: @"<"] == YES && [str hasSuffix: @" />"] == NO)
		{
		  ind += 2;
		}
	    }
	  else
	    {
	      [self fitWords: a
			from: i
			  to: end
		     maxSize: size
		      output: buf];
	      i = end - 1;
	    }
	  [buf appendString: @"\n"];
	}
    }
  LEAVE_POOL
  return ind;
}

/**
 * Split comment text into an array of words (to be reformatted) and
 * insert markup for cross referencing and highlighting.
 */
- (NSArray*) split: (NSString*)str
{
  NSMutableArray	*a = [NSMutableArray arrayWithCapacity: 128];
  unsigned		l = [str length];
  NSMutableData		*data;
  unichar		*ptr;
  unichar		*end;
  unichar		*buf;

  /**
   * Phase 1 ... we take the supplied string and check for white space.
   * Any white space sequence is deleted and treated as a word separator
   * except within xml element markup.  The format of element start and
   * end marks is tidied for consistency.  The resulting data is made
   * into an array of strings, each containing either an element start
   * or end tag, or one of the whitespace separated words.
   * What about str?
   */
  data = [[NSMutableData alloc] initWithLength: l * sizeof(unichar)];
  ptr = buf = [data mutableBytes];
  [str getCharacters: buf];
  end = buf + l;
  while (ptr < end)
    {
      if ([spacenl characterIsMember: *ptr] == YES)
	{
	  if (ptr != buf)
	    {
	      NSString	*tmp;

	      tmp = [NSString stringWithCharacters: buf length: ptr - buf];
	      [a addObject: tmp];
	      buf = ptr;
	    }
	  ptr++;
	  buf++;
	}
      else if (*ptr == '<')
	{
	  BOOL		elideSpace = YES;
	  unichar	*optr = ptr;

	  if (ptr != buf)
	    {
	      NSString	*tmp;

	      tmp = [NSString stringWithCharacters: buf length: ptr - buf];
	      [a addObject: tmp];
	      buf = ptr;
	    }
	  while (ptr < end && *ptr != '>')
	    {
	      /*
	       * We convert whitespace sequences inside element markup
	       * to single space characters unless protected by quotes.
	       */
	      if ([spacenl characterIsMember: *ptr] == YES)
		{
		  if (elideSpace == NO)
		    {
		      *optr++ = ' ';
		      elideSpace = YES;
		    }
		  ptr++;
		}
	      else if (*ptr == '"')
		{
		  while (ptr < end && *ptr != '"')
		    {
		      *optr++ = *ptr++;
		    }
		  if (ptr < end)
		    {
		      *optr++ = *ptr++;
		    }
		  elideSpace = NO;
		}
	      else
		{
		  /*
		   * We want param=value sequences to be standardised to
		   * not have spaces around the equals sign.
		   */
		  if (*ptr == '=')
		    {
		      elideSpace = YES;
		      if (optr[-1] == ' ')
			{
			  optr--;
			}
		    }
		  else
		    {
		      elideSpace = NO;
		    }
		  *optr++ = *ptr++;
		}
	    }
	  if (*ptr == '>')
	    {
	      /*
	       * remove space immediately before closing bracket.
	       */
	      if (optr[-1] == ' ')
		{
		  optr--;
		}
	      *optr++ = *ptr++;
	    }
	  if (optr != buf)
	    {
	      NSString	*tmp;

	      /*
	       * Ensure that elements with no content ('/>' endings)
	       * are standardised to have a space before their terminators.
	       */
	      if (optr[-2] == '/' && optr[-3] != ' ')
		{
		  unsigned	len = optr - buf;
		  unichar	c[len + 1];

		  memcpy(c, buf, (len+1)*sizeof(unichar));
		  c[len-2] = ' ';
		  c[len-1] = '/';
		  c[len] = '>';
		  tmp = [NSString stringWithCharacters: c length: len+1];
		}
	      else
		{
		  tmp = [NSString stringWithCharacters: buf length: optr - buf];
		}
	      [a addObject: tmp];
	    }
	  buf = ptr;
	}
      else
	{
	  ptr++;
	}
    }
  if (ptr != buf)
    {
      NSString	*tmp;

      tmp = [NSString stringWithCharacters: buf length: ptr - buf];
      [a addObject: tmp];
    }

  /*
   * Phase 2 ... the array of words is checked to see if a word contains
   * a well known constant, or a method or function name specification.
   * Where these special cases apply, the array of words is modified to
   * insert extra gsdoc markup to highlight the constants and to create
   * references to where the named methods or functions are documented.
   */
  for (l = 0; l < [a count]; l++)
    {
      static NSArray	*constants = nil;
      static NSArray	*types = nil;
      unsigned		count;
      NSString		*tmp = [a objectAtIndex: l];
      unsigned		pos;
      NSRange		r;

      if (constants == nil)
	{
	  constants = [[NSArray alloc] initWithObjects:
	    @"YES", @"NO", @"nil", nil];
	}

      if (types == nil)
	{
	  types = [[NSArray alloc] initWithObjects:
	    @"Class",
	    @"SEL",
	    @"char",
	    @"double",
	    @"float",
	    @"id",
	    @"int",
	    @"long",
	    @"short",
	    @"signed",
	    @"unichar",
	    @"unsigned",
	    @"void",
	    nil];
	}

      if (l == 0 || [[a objectAtIndex: l-1] isEqual: @"<code>"] == NO)
	{
	  /*
	   * Ensure that well known constants are rendered as 'code'
	   */
	  count = [constants count];
	  for (pos = 0; pos < count; pos++)
	    {
	      NSString	*c = [constants objectAtIndex: pos];

	      r = [tmp rangeOfString: c];

	      if (r.length > 0)
		{
		  NSString	*start;
		  NSString	*end;

		  if (r.location > 0)
		    {
		      start = [tmp substringToIndex: r.location];
		    }
		  else
		    {
		      start = nil;
		    }
		  if (NSMaxRange(r) < [tmp length])
		    {
		      end = [tmp substringFromIndex: NSMaxRange(r)];
		    }
		  else
		    {
		      end = nil;
		    }
		  if ((start == nil || snuggleStart(start) == YES)
		    && (end == nil || snuggleEnd(end) == YES))
		    {
		      NSString	*sub;

		      if (start != nil || end != nil)
			{
			  sub = [tmp substringWithRange: r];
			}
		      else
			{
			  sub = nil;
			}
		      if (start != nil)
			{
			  [a insertObject: start atIndex: l++];
			}
		      [a insertObject: @"<code>" atIndex: l++];
		      if (sub != nil)
			{
			  [a replaceObjectAtIndex: l withObject: sub];
			}
		      l++;
		      [a insertObject: @"</code>" atIndex: l];
		      if (end != nil)
			{
			  [a insertObject: end atIndex: ++l];
			}
		    }
		}
	    }
	}

      if (l == 0 || [[a objectAtIndex: l-1] isEqual: @"<strong>"] == NO)
	{
	  /*
	   * Ensure that well known types are rendered as 'strong'
	   */
	  count = [types count];
	  for (pos = 0; pos < count; pos++)
	    {
	      NSString	*t = [types objectAtIndex: pos];

	      r = [tmp rangeOfString: t];

	      if (r.length > 0)
		{
		  NSString	*start;
		  NSString	*end;

		  if (r.location > 0)
		    {
		      start = [tmp substringToIndex: r.location];
		    }
		  else
		    {
		      start = nil;
		    }
		  if (NSMaxRange(r) < [tmp length])
		    {
		      end = [tmp substringFromIndex: NSMaxRange(r)];
		    }
		  else
		    {
		      end = nil;
		    }
		  if ((start == nil || snuggleStart(start) == YES)
		    && (end == nil || snuggleEnd(end) == YES))
		    {
		      NSString	*sub;

		      if (start != nil || end != nil)
			{
			  sub = [tmp substringWithRange: r];
			}
		      else
			{
			  sub = nil;
			}
		      if (start != nil)
			{
			  [a insertObject: start atIndex: l++];
			}
		      [a insertObject: @"<strong>" atIndex: l++];
		      if (sub != nil)
			{
			  [a replaceObjectAtIndex: l withObject: sub];
			}
		      l++;
		      [a insertObject: @"</strong>" atIndex: l];
		      if (end != nil)
			{
			  [a insertObject: end atIndex: ++l];
			}
		    }
		}
	    }
	}

      /*
       * Ensure that method arguments are rendered as 'var'
       */
      if (l == 0 || [[a objectAtIndex: l-1] isEqual: @"<var>"] == NO)
	{
	  count = [args count];
	  for (pos = 0; pos < count; pos++)
	    {
	      NSString	*c = [args objectAtIndex: pos];

	      r = [tmp rangeOfString: c];

	      if (r.length > 0)
		{
		  NSString	*start;
		  NSString	*end;

		  if (r.location > 0)
		    {
		      start = [tmp substringToIndex: r.location];
		    }
		  else
		    {
		      start = nil;
		    }
		  if (NSMaxRange(r) < [tmp length])
		    {
		      end = [tmp substringFromIndex: NSMaxRange(r)];
		    }
		  else
		    {
		      end = nil;
		    }
		  if ((start == nil || snuggleStart(start) == YES)
		    && (end == nil || snuggleEnd(end) == YES))
		    {
		      NSString	*sub;

		      if (start != nil || end != nil)
			{
			  sub = [tmp substringWithRange: r];
			}
		      else
			{
			  sub = nil;
			}
		      if (start != nil)
			{
			  [a insertObject: start atIndex: l++];
			}
		      [a insertObject: @"<var>" atIndex: l++];
		      if (sub != nil)
			{
			  [a replaceObjectAtIndex: l withObject: sub];
			}
		      l++;
		      [a insertObject: @"</var>" atIndex: l];
		      if (end != nil)
			{
			  [a insertObject: end atIndex: ++l];
			}
		    }
		}
	    }
	}

      /*
       * Ensure that methods are rendered as references.
       * First look for format with class name in square brackets.
       * If that's all there is, we make a class reference.
       */
      r = [tmp rangeOfString: @"["];
      if (r.length > 0
        && (0 == r.location || isspace([tmp characterAtIndex: r.location - 1])))
	{
	  unsigned	sPos = NSMaxRange(r);

	  pos = sPos;
	  r = NSMakeRange(pos, [tmp length] - pos);
	  r = [tmp rangeOfString: @"]" options: NSLiteralSearch range: r];
	  if (r.length > 0)
	    {
	      unsigned	ePos = r.location;
	      NSString	*cName = nil;
	      NSString	*mName = nil;
	      unichar	c = 0;
	      BOOL	isProtocol = NO;

	      if (pos < ePos
		&& [identStart characterIsMember:
		  (c = [tmp characterAtIndex: pos])] == YES)
		{
		  /*
		   * Look for class or category name.
		   */
		  pos++;
		  while (pos < ePos)
		    {
		      c = [tmp characterAtIndex: pos];
		      if ([identifier characterIsMember: c] == NO)
			{
			  break;
			}
		      pos++;
		    }
		  if (c == '(')
		    {
		      pos++;
		      if (pos < ePos
			&& [identStart characterIsMember:
			  (c = [tmp characterAtIndex: pos])] == YES)
			{
			  while (pos < ePos)
			    {
			      c = [tmp characterAtIndex: pos];
			      if ([identifier characterIsMember: c] == NO)
				{
				  break;
				}
			      pos++;
			    }
			  if (c == ')')
			    {
			      pos++;
			      r = NSMakeRange(sPos, pos - sPos);
			      cName = [tmp substringWithRange: r];
			      if (pos < ePos)
				{
				  c = [tmp characterAtIndex: pos];
				}
			    }
			}
		      if (cName == nil)
			{
			  pos = ePos;	// Bad class name!
			}
		    }
		  else
		    {
		      r = NSMakeRange(sPos, pos - sPos);
		      cName = [tmp substringWithRange: r];
		    }
		}
	      else if (pos < ePos && (c = [tmp characterAtIndex: pos]) == '(')
		{
		  /*
		   * Look for protocol name.
		   */
		  pos++;
		  while (pos < ePos)
		    {
		      c = [tmp characterAtIndex: pos];
		      if (c == ')')
			{
			  pos++;
			  r = NSMakeRange(sPos, pos - sPos);
			  cName = [tmp substringWithRange: r];
			  if (pos < ePos)
			    {
			      c = [tmp characterAtIndex: pos];
			    }
			  break;
			}
		      pos++;
		    }
		  isProtocol = YES;
		}

	      if (pos < ePos && (c == '+' || c == '-'))
		{
		  unsigned	mStart = pos;

		  pos++;
		  if (pos < ePos
		    && [identStart characterIsMember:
		      (c = [tmp characterAtIndex: pos])] == YES)
		    {
		      while (pos < ePos)
			{
			  c = [tmp characterAtIndex: pos];
			  if (c != ':'
			    && [identifier characterIsMember: c] == NO)
			    {
			      break;
			    }
			  pos++;
			}
		      /*
		       * Varags methods end with ',...'
		       */
		      if (ePos - pos >= 4
			&& [[tmp substringWithRange: NSMakeRange(pos, 4)]
			  isEqual: @",..."])
			{
			  pos += 4;
			}
		      /*
		       * The end of the method name should be immediately
		       * before the closing square bracket at 'ePos'
		       */
		      if (pos == ePos && pos - mStart > 1)
			{
			  r = NSMakeRange(mStart, pos - mStart);
			  mName = [tmp substringWithRange: r];
			}
		    }
		}
	      if (mName != nil)
		{
		  NSString	*start;
		  NSString	*end;
		  NSString	*sub;
		  NSString	*ref;

		  if (sPos > 0)
		    {
		      start = [tmp substringToIndex: sPos];
		      if ([start isEqualToString: @"["] == YES)
			{
			  start = nil;
			}
		      else if ([start hasSuffix: @"["] == YES)
			{
			  start = [start substringToIndex: [start length] - 1];
			}
		    }
		  else
		    {
		      start = nil;
		    }
		  if (ePos < [tmp length])
		    {
		      end = [tmp substringFromIndex: ePos];
		      if ([end isEqualToString: @"]"] == YES)
			{
			  end = nil;
			}
		      else if ([end hasPrefix: @"]"] == YES)
			{
			  end = [end substringFromIndex: 1];
			}
		    }
		  else
		    {
		      end = nil;
		    }

		  if (start != nil || end != nil)
		    {
		      sub = [tmp substringWithRange:
			NSMakeRange(sPos, ePos - sPos)];
		    }
		  else
		    {
		      sub = nil;
		    }
		  if (start != nil)
		    {
		      [a insertObject: start atIndex: l++];
		    }
		  if (cName == nil)
		    {
		      ref = [NSString stringWithFormat:
			@"<ref type=\"method\" id=\"%@\">", mName];
		    }
		  else if (isProtocol == YES)
		    {
		      ref = [NSString stringWithFormat:
			@"<ref type=\"method\" id=\"%@\" class=\"%@\">",
			mName, cName];
		      if (isProtocol == YES)
			{
			  if (sub == nil)
			    {
			      sub = tmp;
			    }
			  sub = [sub stringByReplacingString: @"("
						  withString: @"&lt;"];
			  sub = [sub stringByReplacingString: @")"
						  withString: @"&gt;"];
			}
		    }
		  else
		    {
		      ref = [NSString stringWithFormat:
			@"<ref type=\"method\" id=\"%@\" class=\"%@\">",
			mName, cName];
		      sub = [NSString stringWithFormat: @"[%@ %@]",
			cName, mName];
		    }
		  [a insertObject: ref atIndex: l++];
		  if (sub != nil)
		    {
		      [a replaceObjectAtIndex: l withObject: sub];
		    }
		
		  l++;
		  [a insertObject: @"</ref>" atIndex: l];
		  if (end != nil)
		    {
		      [a insertObject: end atIndex: ++l];
		    }
		}
	      else if (pos == ePos && cName != nil)
		{
		  NSString	*ref;

		  if (isProtocol == YES)
		    {
		      NSRange	r;

		      r = NSMakeRange(1, [cName length] - 2);
		      cName = [cName substringWithRange: r];
		      ref = [NSString stringWithFormat:
			@"<ref type=\"protocol\" id=\"(%@)\">&lt;%@&gt;</ref>",
			cName, cName];
		    }
		  else if ([cName hasSuffix: @")"] == YES)
		    {
		      ref = [NSString stringWithFormat:
			@"<ref type=\"category\" id=\"%@\">%@</ref>",
			cName, cName];
		    }
		  else
		    {
		      ref = [NSString stringWithFormat:
			@"<ref type=\"class\" id=\"%@\">%@</ref>",
			cName, cName];
		    }
		  [a replaceObjectAtIndex: l withObject: ref];
		  if (ePos < [tmp length])
		    {
		      NSString	*end = [tmp substringFromIndex: ePos];

		      if ([end isEqualToString: @"]"] == YES)
			{
			  end = nil;
			}
		      if ([end hasPrefix: @"]"] == YES)
			{
			  end = [end substringFromIndex: 1];
			}
		      if ([end length] > 0)
			{
			  [a insertObject: end atIndex: ++l];
			}
		    }
		}
	    }
	  continue;
	}

      /*
       * Now handle bare method names for current class ... outside brackets.
       */
      if ([tmp hasPrefix: @"-"] || [tmp hasPrefix: @"+"])
	{
	  unsigned	ePos = [tmp length];
	  NSString	*mName = nil;
	  unsigned	c;

	  pos = 1;
	  if (pos < ePos
	    && [identStart characterIsMember:
	      (c = [tmp characterAtIndex: pos])] == YES)
	    {
	      while (pos < ePos)
		{
		  c = [tmp characterAtIndex: pos];
		  if (c != ':'
		    && [identifier characterIsMember: c] == NO)
		    {
		      break;
		    }
		  pos++;
		}
	      /*
	       * Varags methods end with ',...'
	       */
	      if (ePos - pos >= 4
		&& [[tmp substringWithRange: NSMakeRange(pos, 4)]
		  isEqual: @",..."])
		{
		  pos += 4;
		  if (pos < ePos)
		    {
		      c = [tmp characterAtIndex: pos];
		    }
		}
	      if (pos > 1 && (pos == ePos || c == ',' || c == '.' || c == ';'))
		{
		  NSString	*end;
		  NSString	*sub;
		  NSString	*ref;

		  mName = [tmp substringWithRange: NSMakeRange(0, pos)];

		  if (pos < [tmp length])
		    {
		      end = [tmp substringFromIndex: pos];
		      sub = [tmp substringToIndex: pos];
		    }
		  else
		    {
		      end = nil;
		      sub = nil;
		    }

		  ref = [NSString stringWithFormat:
		    @"<ref type=\"method\" id=\"%@\">", mName];
		  [a insertObject: ref atIndex: l++];
		  if (sub != nil)
		    {
		      [a replaceObjectAtIndex: l withObject: sub];
		    }
		  l++;
		  [a insertObject: @"</ref>" atIndex: l];
		  if (end != nil)
		    {
		      [a insertObject: end atIndex: ++l];
		    }
		}
	    }
	  continue;
	}
      /*
       * Now handle function names.  Anything ending in '()' is assumed to
       * be a referencable function name except 'main()' ... which is special.
       * NB. A comma, fullstop, or semicolon following '()' is counted as if
       * the text ended in '()'
       */
      r = [tmp rangeOfString: @"()"];
      if (r.length > 0)
	{
	  unsigned	c = [tmp characterAtIndex: 0];
	  unsigned	len = [tmp length];
	  NSString	*str = [tmp substringToIndex: r.location];
	  BOOL		ok = NO;

	  if ([identStart characterIsMember: c] == YES
	    && [str isEqual: @"main"] == NO)
	    {
	      ok = YES;
	      if (len > NSMaxRange(r))
		{
		  NSString	*end;

		  end = [tmp substringFromIndex: NSMaxRange(r)];
		  c = [end characterAtIndex: 0];
		  if (c == ',' || c == '.' || c == ';')
		    {
		      [a insertObject: end atIndex: l + 1];
		      tmp = [tmp substringToIndex: NSMaxRange(r)];
		      [a replaceObjectAtIndex: l withObject: tmp];
		    }
		  else
		    {
		      ok = NO;
		    }
		}
	    }
	  if (ok == YES)
	    {
	      str = [NSString stringWithFormat:
		@"<ref type=\"function\" id=\"%@\">", str];
	      [a insertObject: str atIndex: l++];
	      l++;	// Point past the function name in the array.
	      [a insertObject: @"</ref>" atIndex: l++];
	      continue;
	    }
	}
    }

  return a;
}

- (NSArray*) informalProtocols
{
  return informalProtocols;
}

@end


@implementation AGSOutput (Private)
- (NSString*) mergeMarkup: (NSString*)markup
		   ofKind: (NSString*)kind
{
  NSUserDefaults	*ud = [NSUserDefaults standardUserDefaults];
  NSString		*key = [kind stringByAppendingString: @"Template"];
  NSString		*name = [ud stringForKey: key];
  NSData		*d;
  NSString		*file;
  NSFileManager		*mgr;
  NSString		*base;
  NSString		*tmp;
  NSMutableString	*str;
  NSRange		range;
  NSRange		start;
  NSRange		end;
  NSString		*dest;

  dest = [info objectForKey: @"directory"];
  if ([name length] == 0)
    {
      return nil;	// No common document.
    }

  file = [name stringByAppendingPathExtension: @"gsdoc"];
  if ([dest length] > 0 && [file isAbsolutePath] == NO)
    {
      file = [dest stringByAppendingPathComponent: file];
    }
  mgr = [NSFileManager defaultManager];
  base = [info objectForKey: @"base"];

  /*
   * Load the current file that info should be merged into.
   */
  if ([mgr isReadableFileAtPath: file] == YES)
    {
      str = [NSMutableString stringWithContentsOfFile: file];
    }
  else
    {
      tmp = [name stringByAppendingPathExtension: @"template"];

      if ([mgr isReadableFileAtPath: tmp] == YES)
	{
	  str = [NSMutableString stringWithContentsOfFile: tmp];
	}
      else
	{
	  NSString	*up = [ud stringForKey: @"Up"];

	  /*
	   * No pre-existing file, and no blank template available ...
	   * Generate a standard template.
	   */
	  str = [[NSMutableString alloc] initWithCapacity: 1024];

	  [str appendString: @"<?xml version=\"1.0\"?>\n"];
	  [str appendString: @"<!DOCTYPE gsdoc PUBLIC "];
	  [str appendString: @"\"-//GNUstep//DTD gsdoc 1.0.4//EN\" "];
	  [str appendString: @"\"http://www.gnustep.org/gsdoc-1_0_4.dtd\">\n"];

	  [str appendString: @"<gsdoc base=\""];
	  [str appendString: [name lastPathComponent]];
	  /*
	   * If a -Up default has been set, create an up link in this
	   * template file... as long as the specified up link is not
	   * the template file itself.
	   */
	  if (up != nil && [up isEqual: [name lastPathComponent]] == NO)
	    {
	      [str appendString: @"\" up=\""];
	      [str appendString: up];
	    }
	  [str appendString: @"\">\n"];
	  [str appendString: @"  <head>\n"];
	  [str appendString: @"    <title>"];
	  [str appendString: kind];
	  [str appendString: @"</title>\n"];
	  tmp = [NSString stringWithFormat: @"Generated by %@", NSUserName()];
	  [str appendString: @"    <author name=\""];
	  [str appendString: tmp];
	  [str appendString: @"\"></author>\n"];
	  [str appendString: @"  </head>\n"];
	  [str appendString: @"  <body>\n"];
	  [str appendString: @"    <front><contents /></front>\n"];
	  [str appendString: @"  </body>\n"];
	  [str appendString: @"</gsdoc>\n"];
	}
    }

  /*
   * Locate  start and end points for all markup of this 'kind'.
   */
  tmp = [NSString stringWithFormat: @"<!--Start%@-->\n", kind];
  start = [str rangeOfString: tmp];
  if (start.length == 0)
    {
      start = [str rangeOfString: @"<back>"];
      if (start.length == 0)
	{
	  start = [str rangeOfString: @"</body>"];
	}
      if (start.length == 0)
	{
	  NSLog(@"No <back> or </body> markup in %@ document", kind);
	  return nil;
	}
      [str insertString: tmp atIndex: start.location];
      start.length = [tmp length];
    }
  tmp = [NSString stringWithFormat: @"<!--End%@-->\n", kind];
  end = [str rangeOfString: tmp];
  if (end.length == 0)
    {
      end.location = NSMaxRange(start);
      end.length = [tmp length];
      [str insertString: tmp atIndex: end.location];
    }
  else if (end.location <= start.location)
    {
      NSLog(@"End marker comes before start marker in %@ document", kind);
      return nil;
    }

  /*
   * Now locate start and end points for markup for this file.
   */
  tmp = [NSString stringWithFormat: @"<!--Start%@%@-->\n", base, kind];
  start = [str rangeOfString: tmp];
  if (start.length == 0)
    {
      start.location = end.location;	// Insert before end of section.
      start.length = [tmp length];
      [str insertString: tmp atIndex: end.location];
    }
  tmp = [NSString stringWithFormat: @"<!--End%@%@-->\n", base, kind];
  end = [str rangeOfString: tmp];
  if (end.length == 0)
    {
      end.location = NSMaxRange(start);
      end.length = [tmp length];
      [str insertString: tmp atIndex: end.location];
    }

  range = NSMakeRange(NSMaxRange(start), end.location - NSMaxRange(start));
  [str replaceCharactersInRange: range withString: markup];

  d = [str dataUsingEncoding: NSUTF8StringEncoding];
  if ([d writeToFile: file atomically: YES] == NO)
    {
      NSLog(@"Unable to write %@ markup to %@", kind, file);
      return nil;
    }
  return file;
}
@end

