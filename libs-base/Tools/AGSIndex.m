/**

   <title>AGSIndex ... a class to create references for a gsdoc file</title>
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

#import	"common.h"

#import	"Foundation/NSArray.h"
#import	"Foundation/NSAutoreleasePool.h"
#import	"Foundation/NSDictionary.h"
#import "AGSIndex.h"
#import "GNUstepBase/NSString+GNUstepBase.h"
#import "GNUstepBase/NSMutableString+GNUstepBase.h"

static int      XML_ELEMENT_NODE;
static int      XML_TEXT_NODE;

static void
mergeDictionaries(NSMutableDictionary *dst, NSDictionary *src, BOOL override)
{
  static NSMutableArray	*stack = nil;
  NSEnumerator	*e = [src keyEnumerator];
  NSString	*k;
  id		s;
  id		d;

  if (stack == nil)
    {
      stack = [[NSMutableArray alloc] initWithCapacity: 8];
    }
  while ((k = [e nextObject]) != nil)
    {
      if ([k isEqualToString: @"contents"] == YES)
	{
	  continue;	// Makes no sense to merge file contents.
	}
      s = [src objectForKey: k];
      d = [dst objectForKey: k];

      [stack addObject: k];
      if (d == nil)
	{
	  if ([s isKindOfClass: [NSString class]] == YES)
	    {
	      [dst setObject: s forKey: k];
	    }
	  else if ([s isKindOfClass: [NSArray class]] == YES)
	    {
	      [dst setObject: s forKey: k];
	    }
	  else if ([s isKindOfClass: [NSDictionary class]] == YES)
	    {
	      d = [[NSMutableDictionary alloc] initWithCapacity: [s count]];
	      [dst setObject: d forKey: k];
	      RELEASE(d);
	    }
	  else
	    {
	      NSLog(@"Unexpected class in merge %@ ignored", stack);
	      d = nil;
	    }
	}
      if (d != nil)
	{
	  if ([d isKindOfClass: [NSString class]] == YES)
	    {
	      if ([s isKindOfClass: [NSString class]] == NO)
		{
		  NSLog(@"Class mismatch in merge for %@.", stack);
		}
	      else if ([d isEqual: s] == NO)
		{
		  if (override == YES)
		    {
		      [dst setObject: s forKey: k];
		    }
		  else
		    {
		      NSLog(@"String mismatch in merge for %@. S:%@, D:%@",
			stack, s, d);
		      [dst setObject: s forKey: k];
		    }
		}
	    }
	  else if ([d isKindOfClass: [NSArray class]] == YES)
	    {
	      if ([s isKindOfClass: [NSArray class]] == NO)
		{
		  NSLog(@"Class mismatch in merge for %@.", stack);
		}
	      else if ([d isEqual: s] == NO)
		{
		  if (override == YES)
		    {
		      [dst setObject: s forKey: k];
		    }
		  else
		    {
		      NSLog(@"Array mismatch in merge for %@. S:%@, D:%@",
			stack, s, d);
		    }
		}
	    }
	  else if ([d isKindOfClass: [NSDictionary class]] == YES)
	    {
	      if ([s isKindOfClass: [NSDictionary class]] == NO)
		{
		  NSLog(@"Class mismatch in merge for %@.", stack);
		}
	      else
		{
		  mergeDictionaries(d, s, override);
		}
	    }
	}
      [stack removeLastObject];
    }
}

static void
setDirectory(NSMutableDictionary *dict, NSString *path)
{
  NSArray	*a = [dict allKeys];
  NSEnumerator	*e = [a objectEnumerator];
  NSString	*k;

  while ((k = [e nextObject]) != nil)
    {
      id	o = [dict objectForKey: k];

      if ([o isKindOfClass: [NSString class]] == YES)
	{
	  o = [path stringByAppendingPathComponent: [o lastPathComponent]];
	  [dict setObject: o forKey: k];
	}
      else if ([o isKindOfClass: [NSDictionary class]] == YES)
	{
	  setDirectory(o, path);
	}
    }
}

/**
 * This class is used to build and manipulate a dictionary of
 * cross-reference information.<br />
 * The references are held in a nested dictionary
 * with strings at the leaves (persisted to 'projectName'.igsdoc) -<br />
 * method : method-name - { class-name file-name }<br />
 * method : method-name - { protocol-name file-name }<br />
 * ivariable : variable-name - { class-name file-name }<br />
 * class : class-name - file-name<br />
 * category : category-name - file-name<br />
 * protocol : protocol-name - file-name<br />
 * function : function-name - file-name<br />
 * type : type-name - file-name<br />
 * constant : constant-name - file-name<br />
 * variable : variable-name - file-name<br />
 * entry : entry-name - { file-name ref }<br />
 * label : label-name - { file-name ref }<br />
 * contents : ref - text<br />
 * super : class-name - superclass-name<br />
 * categories : class-name - { category-name file-name }<br />
 * unitmethods : unit-name - method-name<br />
 * classvars : class-name - variables-name<br />
 * title : file-name - text<br />
 * source : file-name - array-of-source-files<br />
 */
@implementation	AGSIndex

+ (void) initialize
{
  if (self == [AGSIndex class])
    {
      /*
       * Cache XML node information.
       */
      XML_ELEMENT_NODE = [GSXMLNode typeFromDescription: @"XML_ELEMENT_NODE"];
      XML_TEXT_NODE = [GSXMLNode typeFromDescription: @"XML_TEXT_NODE"];
    }
}

- (void) dealloc
{
  RELEASE(refs);
  [super dealloc];
}

- (NSString*) globalRef: (NSString*)ref type: (NSString*)type
{
  NSDictionary	*t;

  t = [refs objectForKey: type];
  return [t objectForKey: ref];
}

- (id) init
{
  refs = [[NSMutableDictionary alloc] initWithCapacity: 8];
  return self;
}

/**
 * Given the root node of a gsdoc document, we traverse the tree
 * looking for interesting nodes, and recording their names in a
 * dictionary of references.
 */
- (void) makeRefs: (GSXMLNode*)node
{
  GSXMLNode	*children = [node firstChild];
  GSXMLNode	*next = [node next];
  BOOL		newUnit = NO;

  if ([node type] == XML_ELEMENT_NODE)
    {
      NSString		*name = [node name];
      NSDictionary	*prop = [node attributes];

      // special case- if has an id of "_main" this gsdoc is from a tool
      // file; add an entry to the index to indicate so
      if (([name isEqual: @"chapter"] || [name isEqual: @"section"])
          && [@"_main" isEqual: [prop objectForKey: @"id"]] == YES)
        {
          [self setGlobalRef: base type: @"tool"];
        }

      if ([name isEqual: @"category"] == YES)
	{
	  newUnit = YES;
	  classname = [prop objectForKey: @"class"];
	  category = [prop objectForKey: @"name"];
	  unit = classname;
	  /*
	   * Add this category to the list of those for the class.
	   */
	  [self setUnitRef: category type: @"categories"];
	  unit = [NSString stringWithFormat: @"%@(%@)", classname, category];
	  [self setGlobalRef: unit type: @"category"];
	}
      else if ([name isEqual: @"chapter"] == YES)
	{
	  chap++;
	  sect = 0;
	  ssect = 0;
	  sssect = 0;
	}
      else if ([name isEqual: @"class"] == YES)
	{
	  NSString	*tmp;

	  newUnit = YES;
	  classname = [prop objectForKey: @"name"];
	  unit = classname;
	  tmp = [prop objectForKey: @"super"];
	  if (tmp != nil)
	    {
	      [self setRelationship: @"super" from: unit to: tmp];
	    }
	  [self setGlobalRef: unit type: @"class"];
	}
      else if ([name isEqual: @"gsdoc"] == YES)
	{
	  base = [prop objectForKey: @"base"];
	  if (base == nil)
	    {
	      NSLog(@"No 'base' document name supplied in gsdoc element");
	      return;
	    }
	}
      else if ([name isEqual: @"heading"] == YES)
	{
	  NSMutableDictionary	*d;
	  NSString		*k;
	  NSString		*c;

	  d = [refs objectForKey: @"contents"];
	  if (d == nil)
	    {
	      d = [[NSMutableDictionary alloc] initWithCapacity: 8];
	      [refs setObject: d forKey: @"contents"];
	      RELEASE(d);
	    }

          k = [NSString stringWithFormat: @"%03u%03u%03u%03u",
	    chap, sect, ssect, sssect];
	  c = [[children content] stringByTrimmingSpaces];
	  if (c == nil) c = @"";
	  [d setObject: c forKey: k];
	  children = nil;
	}
      else if ([name isEqual: @"ivariable"] == YES)
	{
	  NSString	*tmp = [prop objectForKey: @"name"];

	  [self setUnitRef: tmp type: @"ivariable"];
	  [self setUnitRef: tmp type: @"classvars"];
	}
      else if ([name isEqual: @"entry"] || [name isEqual: @"label"])
	{
	  NSMutableDictionary	*all;
	  NSString		*text;
	  NSString		*val;

	  text = [[children content] stringByTrimmingSpaces];
	  if (text == nil) text = @"";
	  children = nil;
	  all = [refs objectForKey: name];
	  if (all == nil)
	    {
	      all = [[NSMutableDictionary alloc] initWithCapacity: 8];
	      [refs setObject: all forKey: name];
	      RELEASE(all);
	    }
	  val = [prop objectForKey: @"id"];
	  if (val == nil)
	    {
	      val = text;
	    }
	  [all setObject: base forKey: val];
	}
      else if ([name isEqual: @"method"] == YES)
	{
	  NSString	*sel = @"";
	  GSXMLNode	*tmp = children;

	  sel = [prop objectForKey: @"factory"];
	  if (sel != nil && [sel boolValue] == YES)
	    {
	      sel = @"+";
	    }
	  else
	    {
	      sel = @"-";
	    }
	  children = nil;
	  while (tmp != nil)
	    {
	      if ([tmp type] == XML_ELEMENT_NODE)
		{
		  if ([[tmp name] isEqual: @"sel"] == YES)
		    {
		      GSXMLNode	*t = [tmp firstChild];

		      while (t != nil)
			{
			  if ([t type] == XML_TEXT_NODE)
			    {
			      NSString	*s;

			      s = [[t content] stringByTrimmingSpaces];
			      if (s == nil) s = @"";
			      sel = [sel stringByAppendingString: s];
			    }
			  t = [t next];
			}
		    }
		  else if ([[tmp name] isEqual: @"vararg"] == YES)
		    {
		      sel = [sel stringByAppendingString: @",..."];
		      children = [tmp next];
		      break;
		    }
		  else if ([[tmp name] isEqual: @"arg"] == NO)
		    {
		      children = tmp;
		      break;
		    }
		}
	      tmp = [tmp next];
	    }
	  if ([sel length] > 1)
	    {
	      [self setUnitRef: sel type: @"method"];
	      [self setUnitRef: sel type: @"unitmethods"];
	    }
	}
      else if ([name isEqual: @"protocol"] == YES)
	{
	  newUnit = YES;
	  unit = [NSString stringWithFormat: @"(%@)",
	    [prop objectForKey: @"name"]];
	  [self setGlobalRef: unit type: @"protocol"];
	}
      else if ([name isEqual: @"constant"] == YES
	|| [name isEqual: @"EOEntity"] == YES
	|| [name isEqual: @"EOModel"] == YES
	|| [name isEqual: @"function"] == YES
	|| [name isEqual: @"macro"] == YES
	|| [name isEqual: @"type"] == YES
	|| [name isEqual: @"variable"] == YES)
	{
	  NSString	*tmp = [prop objectForKey: @"name"];

	  [self setGlobalRef: tmp type: name];
	  children = nil;
	}
      else if ([name isEqual: @"section"] == YES)
	{
          //FIXME-  this info needs to be placed into the "label" refs somehow
	  sect++;
	  ssect = 0;
	  sssect = 0;
	}
      else if ([name isEqual: @"subsect"] == YES)
	{
	  ssect++;
	  sssect = 0;
	}
      else if ([name isEqual: @"subsubsect"] == YES)
	{
	  sssect++;
	}
      else if ([name isEqual: @"title"] == YES)
	{
	  NSMutableDictionary	*d;
	  NSString		*s;

	  d = [refs objectForKey: @"title"];
	  if (d == nil)
	    {
	      d = [[NSMutableDictionary alloc] initWithCapacity: 8];
	      [refs setObject: d forKey: @"title"];
	      RELEASE(d);
	    }

	  s = [[children content] stringByTrimmingSpaces];
	  if (s == nil) s = @"";
	  [d setObject: s forKey: base];
	  children = nil;
	}
      else
	{
	}
    }

  if (children != nil)
    {
      [self makeRefs: children];
    }
  if (newUnit == YES)
    {
      unit = nil;
      category = nil;
      classname = nil;
    }
  if (next != nil)
    {
      [self makeRefs: next];
    }
}

/**
 * Merge a dictionary containing references into the current
 * index.   The flag may be used to specify that references
 * being merged in should override any pre-existing values.
 */
- (void) mergeRefs: (NSDictionary*)more override: (BOOL)flag
{
  mergeDictionaries(refs, more, flag);
}

/**
 * Informal protocols are not explicitly marked in source, but are
 * inferred to be those categories of NSObject that receive no
 * implementation.  [AGSOutput] finds and accumulates them; autogsdoc
 * passes them here, where each entry is found in the 'category'
 * section of our refs and copied over to the protocol section.
 */
- (void) addInformalProtocols: (NSArray *)protocolNames
{
  NSString	*name;
  NSString	*file;
  NSEnumerator	*pnames = [protocolNames objectEnumerator];

  //PENDING, should we worry about not overriding entries?
  while ((name = [pnames nextObject]) != nil)
    {
      NSMutableDictionary	*d;

      d = [refs objectForKey: @"category"];
      file = [d objectForKey: name];
      if (file != nil)
        {
	  d = [refs objectForKey: @"protocol"];
	  [d setObject: file forKey: name];
        }
      else
        {
          NSLog(@"Category entry not found for informal protocol '%@'", name);
        }
    }
}


- (NSArray*) methodsInUnit: (NSString*)aUnit
{
  NSDictionary		*d = [refs objectForKey: @"unitmethods"];

  d = [d objectForKey: aUnit];
  return [d allKeys];
}

/**
 * Return a list of output files for the header
 */
- (NSMutableArray*) outputsForHeader: (NSString*)h
{
  NSDictionary	*dict = [refs objectForKey: @"output"];
  NSArray	*array = [dict objectForKey: h];

  if (array == nil)
    {
      return [NSMutableArray arrayWithCapacity: 2];
    }
  return AUTORELEASE([array mutableCopy]);
}

- (NSMutableDictionary*) refs
{
  return refs;
}

- (void) setDirectory: (NSString*)path
{
  if (path != nil)
    {
      ENTER_POOL
      setDirectory(refs, path);
      LEAVE_POOL
    }
}

- (void) setGlobalRef: (NSString*)ref
		 type: (NSString*)type
{
  NSMutableDictionary	*t;
  NSString		*old;

  t = [refs objectForKey: type];
  if (t == nil)
    {
      t = [NSMutableDictionary new];
      [refs setObject: t forKey: type];
      RELEASE(t);
    }
  old = [t objectForKey: ref];
  if (old != nil && [old isEqual: base] == NO)
    {
      NSLog(@"Warning ... %@ %@ appears in %@ and %@ ... using the latter",
	type, ref, old, base);
    }
  [t setObject: base forKey: ref];
}

/**
 * Set up an array listing the output files for a particular header.
 */
- (void) setOutputs: (NSArray*)a forHeader: (NSString*)h
{
  NSMutableDictionary	*dict;

  dict = [refs objectForKey: @"output"];
  if (dict == nil)
    {
      dict = [NSMutableDictionary new];
      [refs setObject: dict forKey: @"output"];
      RELEASE(dict);
    }
  [dict setObject: a forKey: h];
}

- (void) setRelationship: (NSString*)r from: (NSString*)from to: (NSString*)to
{
  NSMutableDictionary	*dict;

  dict = [refs objectForKey: r];
  if (dict == nil)
    {
      dict = [NSMutableDictionary new];
      [refs setObject: dict forKey: r];
      RELEASE(dict);
    }
  [dict setObject: to forKey: from];
}

/**
 * Set up an array listing the source files for a particular header.
 */
- (void) setSources: (NSArray*)a forHeader: (NSString*)h
{
  NSMutableDictionary	*dict;

  dict = [refs objectForKey: @"source"];
  if (dict == nil)
    {
      dict = [NSMutableDictionary new];
      [refs setObject: dict forKey: @"source"];
      RELEASE(dict);
    }
  [dict setObject: a forKey: h];
}

/**
 * Set up a reference for something inside a unit (class, category or protocol)
 * We store 'method' and 'ivariable' by ref then unit (class),
 * but we store 'unitmethods' * and 'classvars' by unit then ref.
 */
- (void) setUnitRef: (NSString*)ref
	       type: (NSString*)type
{
  NSMutableDictionary	*t;
  NSMutableDictionary	*r;
  NSString		*u = unit;
  NSString		*old;

  if ([type isEqualToString: @"method"] || [type isEqualToString: @"ivariable"])
    {
      // type ... ref ... unit ... file
    }
  else
    {
      u = ref;
      ref = unit;
    }

  t = [refs objectForKey: type];
  if (t == nil)
    {
      t = [NSMutableDictionary new];
      [refs setObject: t forKey: type];
      RELEASE(t);
    }
  r = [t objectForKey: ref];
  if (r == nil)
    {
      r = [NSMutableDictionary new];
      [t setObject: r forKey: ref];
      RELEASE(r);
    }
  old = [r objectForKey: u];
  if (old != nil && [old isEqual: base] == NO)
    {
      NSLog(@"Warning ... %@ %@ %@ appears in %@ and %@ ... using the latter",
	type, ref, u, old, base);
    }
  [r setObject: base forKey: u];
}

/**
 * Return a list of source files for the header.
 */
- (NSMutableArray*) sourcesForHeader: (NSString*)h
{
  NSDictionary	*dict = [refs objectForKey: @"source"];
  NSArray	*array = [dict objectForKey: h];

  if (array == nil)
    {
      return [NSMutableArray arrayWithCapacity: 2];
    }
  return AUTORELEASE([array mutableCopy]);
}

/**
 * Return a dictionary containing info on all the units containing the
 * specified method or instance variable.
 */
- (NSDictionary*) unitRef: (NSString*)ref type: (NSString*)type
{
  NSDictionary	*t;

  t = [refs objectForKey: type];
  return [t objectForKey: ref];
}

/**
 * Return the name of the file containing the ref and return
 * the unit name in which it was found.  If not found, return nil for both.
 */
- (NSString*) unitRef: (NSString*)ref type: (NSString*)type unit: (NSString**)u
{
  NSString	*s;
  NSDictionary	*t;

  /**
   * If ref does not occur in the index, this method returns nil.
   */
  t = [self unitRef: ref type: type];
  if (t == nil)
    {
      *u = nil;
      return nil;
    }
  if (*u == nil)
    {
      NSEnumerator	*e;
      NSString		*n;
      unsigned		count = 0;

      /**
       * If the method was given no unit to look in, then it will succeed
       * and return a value if (and only if) the required reference is
       * defined only in one unit (excluding protocols).
       * In the case where it is in two units (one of them a protocol)
       * the class is taken in preference to the protocol.
       */
      e = [t keyEnumerator];
      while ((n = [e nextObject]) != nil)
	{
	  *u = n;
	  if ([n hasPrefix: @"("] == NO)
	    {
	      if (count++ > 0)
		{
		  *u = nil;	// More than one match
		  break;
		}
	    }
	}
      if (*u != nil)
	{
	  return [t objectForKey: *u];
	}
      return nil;
    }

  /**
   * If ref exists in the unit specified, the method will succeed and
   * return the name of the file in which the reference is located.
   */
  s = [t objectForKey: *u];
  if (s != nil)
    {
      return s;
    }

  /**
   * If the unit that the method has been asked to look in is a
   * protocol which is not found, the lookup must fail.
   */
  if ([*u hasPrefix: @"("] == YES)
    {
      *u = nil;
      return nil;
    }

  /*
   * If unit is a category, method was probably indexed under the class,
   * so work with the class instead of the category.
   */
  if ([*u length] > 0 && [*u characterAtIndex: [*u length] - 1] == ')')
    {
      *u = [*u substringToIndex: [*u rangeOfString: @"("].location];
      s = [t objectForKey: *u];
      if (s != nil)
        {
          return s;
        }
    }

  /**
   * Try all superclasses in turn.
   */
  while (*u != nil)
    {
      *u = [self globalRef: *u type: @"super"];
      if (*u != nil)
	{
	  s = [t objectForKey: *u];
	  if (s != nil)
	    {
	      return s;
	    }
	}
    }

  return nil;
}

@end

