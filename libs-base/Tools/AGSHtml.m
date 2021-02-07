/**

   <title>AGSHtml ... a class to output html for a gsdoc file</title>
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

#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSSet.h"
#import "Foundation/NSUserDefaults.h"
#import "AGSHtml.h"
#import "GNUstepBase/NSString+GNUstepBase.h"
#import "GNUstepBase/NSMutableString+GNUstepBase.h"

/*
 * Define constants for use if we are built with apple Foundation
 */
#ifndef	GS_API_OSSPEC
#define	GS_API_OSSPEC	10000
#endif
#ifndef	GS_API_OPENSTEP
#define	GS_API_OPENSTEP	40000
#endif
#ifndef	GS_API_MACOSX
#define	GS_API_MACOSX	100000
#endif

static int      XML_ELEMENT_NODE;
static int      XML_ENTITY_REF_NODE;
static int      XML_TEXT_NODE;

static GSXMLNode	*firstElement(GSXMLNode *nodes)
{
  if (nodes == nil)
    {
      return nil;
    }
  if ([nodes type] == XML_ELEMENT_NODE)
    {
      return nodes;
    }
  return [nodes nextElement];
}

@implementation	AGSHtml

static NSMutableSet	*textNodes = nil;
static NSString		*tocFont = nil;
static NSString		*mainFont = nil;

+ (void) initialize
{
  if (self == [AGSHtml class])
    {
      /*
       * Cache XML node information.
       */
      XML_ELEMENT_NODE = [GSXMLNode typeFromDescription: @"XML_ELEMENT_NODE"];
      XML_ENTITY_REF_NODE
	= [GSXMLNode typeFromDescription: @"XML_ENTITY_REF_NODE"];
      XML_TEXT_NODE = [GSXMLNode typeFromDescription: @"XML_TEXT_NODE"];
      textNodes = [NSMutableSet new];
      [textNodes addObject: @"br"];
      [textNodes addObject: @"code"];
      [textNodes addObject: @"em"];
      [textNodes addObject: @"email"];
      [textNodes addObject: @"entry"];
      [textNodes addObject: @"file"];
      [textNodes addObject: @"label"];
      [textNodes addObject: @"prjref"];
      [textNodes addObject: @"ref"];
      [textNodes addObject: @"site"];
      [textNodes addObject: @"strong"];
      [textNodes addObject: @"uref"];
      [textNodes addObject: @"url"];
      [textNodes addObject: @"var"];
      [textNodes addObject: @"footnote"];

      // default fonts
      tocFont = @"sans";
      mainFont = @"serif";
    }
}

- (void) dealloc
{
  RELEASE(project);
  RELEASE(globalRefs);
  RELEASE(localRefs);
  RELEASE(projectRefs);
  RELEASE(indent);
  DEALLOC
}

- (void) decIndent
{
  unsigned	len = [indent length];

  if (len >= 2)
    {
      [indent deleteCharactersInRange: NSMakeRange(len - 2, 2)];
    }
}

- (void) incIndent
{
  [indent appendString: @"  "];
}

- (id) init
{
  indent = [[NSMutableString alloc] initWithCapacity: 64];
  project = RETAIN([[NSUserDefaults standardUserDefaults]
    stringForKey: @"Project"]);
  return self;
}

/**
 * Calls -makeLink:ofType:isRef: or -makeLink:ofType:inUnit:isRef: to
 * create the first part of an anchor, and fills in the text content
 * of the anchor with n (the specified name).  Returns an entire anchor
 * string as a result.<br />
 * This method is used to create all the anchors in the html output.
 */
- (NSString*) makeAnchor: (NSString*)r
		  ofType: (NSString*)t
		    name: (NSString*)n
{
  NSString	*s;

  if (n == nil)
    {
      n = @"";
    }
  if ([t isEqualToString: @"method"] || [t isEqualToString: @"ivariable"])
    {
      s = [self makeLink: r ofType: t inUnit: nil isRef: NO];
    }
  else
    {
      s = [self makeLink: r ofType: t isRef: NO];
    }
  if (s != nil)
    {
      n = [s stringByAppendingFormat: @"%@</a>", n];
    }
  return n;
}

/**
 * Make a link for the element r with the specified type. Only the start of
 * the html element is returned (&lt;a ...&gt;).
 * If the boolean f is YES, then the link is a reference to somewhere,
 * and the method will return nil if the destination is not found in the index.
 * If f is NO, then the link is an anchor for some element being output, and
 * the method is guaranteed to succeed and return the link.
 */
- (NSString*) makeLink: (NSString*)r
		ofType: (NSString*)t
		 isRef: (BOOL)f
{
  NSString	*s;
  NSString	*kind = (f == YES) ? @"rel=\"gsdoc\" href" : @"name";
  NSString	*hash = (f == YES) ? @"#" : @"";

  if (f == NO || (s = [localRefs globalRef: r type: t]) != nil)
    {
      s = [NSString stringWithFormat: @"<a %@=\"%@%@$%@\">",
	kind, hash, t, r];
    }
  else if ((s = [globalRefs globalRef: r type: t]) != nil)
    {
      s = [s stringByAppendingPathExtension: @"html"];
      s = [NSString stringWithFormat: @"<a %@=\"%@%@%@$%@\">",
	kind, s, hash, t, r];
    }
  return [s stringByReplacingString: @":" withString: @"$"];
}

/**
 * Make a link for the element r, with the specified type t,
 * in a particular unit u. Only the start of
 * the html element is returned (&lt;a ...&gt;).<br />
 * If the boolean f is YES, then the link is a reference to somewhere,
 * otherwise the link is an anchor for some element being output.<br />
 * If there is an error, the method returns nil.
 */
- (NSString*) makeLink: (NSString*)r
		ofType: (NSString*)t
		inUnit: (NSString*)u
		 isRef: (BOOL)f
{
  NSString	*s = nil;
  NSString	*kind = (f == YES) ? @"rel=\"gsdoc\" href" : @"name";
  NSString	*hash = (f == YES) ? @"#" : @"";

  if (f == NO)
    {
      if (u == nil)
	{
	  u = unit;
	  s = base;
	}
    }
  else if (u == nil)
    {
      NSString	*tmp = unit;

      s = [localRefs unitRef: r type: t unit: &tmp];
      if (s == nil)
	{
	  tmp = u;
	  s = [localRefs unitRef: r type: t unit: &tmp];
	  if (s == nil)
	    {
	      tmp = unit;
	      s = [globalRefs unitRef: r type: t unit: &tmp];
	      if (s == nil)
		{
		  tmp = nil;
		  s = [globalRefs unitRef: r type: t unit: &tmp];
		}
	    }
	}
      u = tmp;
    }
  if (s == nil)
    {
      NSString	*tmp = u;

      /*
       * Simply look up the reference.
       */
      s = [localRefs unitRef: r type: t unit: &u];
      if (s == nil)
	{
	  u = tmp;
	  s = [globalRefs unitRef: r type: t unit: &u];
	}
    }

  if (s != nil)
    {
      NSString	*sep = @"";

      if ([t isEqual: @"ivariable"] == YES)
	{
	  sep = @"*";
	}
      if ([s isEqual: base] == YES)
	{
	  s = [NSString stringWithFormat: @"<a %@=\"%@%@$%@%@%@\">",
	    kind, hash, t, u, sep, r];
	}
      else
	{
	  s = [s stringByAppendingPathExtension: @"html"];
	  s = [NSString stringWithFormat: @"<a %@=\"%@%@%@$%@%@%@\">",
	    kind, s, hash, t, u, sep, r];
	}
    }
  return [s stringByReplacingString: @":" withString: @"$"];
}

- (NSString*) outputDocument: (GSXMLNode*)node
{
  NSMutableString	*buf;

  if (localRefs == nil)
    {
      localRefs = [AGSIndex new];
      [localRefs makeRefs: node];
    }
  buf = [NSMutableString stringWithCapacity: 4096];

  /* Declaration */
  [buf appendString: @"<!DOCTYPE html PUBLIC "];
  [buf appendString: @"\"-//W3C//DTD XHTML 1.0 Strict//EN\"\n"];
  [buf appendString: @"\"http://www.w3.org/TR/xhtml1/DTD/"];
  [buf appendString: @"xhtml1-strict.dtd\">\n"];
  [buf appendString: @"<html xmlns=\"http://www.w3.org/1999/xhtml\" "];
  [buf appendString: @"xml:lang=\"en\" lang=\"en\">\n"];
 
  [self incIndent];
  [self outputNodeList: node to: buf];
  [self decIndent];
  [buf appendString: @"</html>\n"];

  return buf;
}

- (void) outputIndex: (NSString*)type
	       scope: (NSString*)scope
	       title: (NSString*)title
	       style: (NSString*)style
              target: (NSString*)target
		  to: (NSMutableString*)buf
{
  NSDictionary	*refs = [localRefs refs];
  NSDictionary	*dict;
  NSArray	*a;
  unsigned	c;
  unsigned	i;
  BOOL		isBareStyle = [@"bare" isEqualToString: style];

  if (globalRefs != nil && [scope isEqual: @"global"] == YES)
    {
      refs = [globalRefs refs];
    }
  else if (projectRefs != nil && [scope isEqual: @"project"] == YES)
    {
      refs = [projectRefs refs];
    }

  if ([type isEqualToString: @"method"] == YES)
    {
      if (unit == nil)
	{
	  refs = nil;	// Can't index methods outside a unit.
	}
      dict = [refs objectForKey: @"unitmethods"];
      dict = [dict objectForKey: unit];
    }
  else if ([type isEqualToString: @"ivariable"] == YES)
    {
      if (unit == nil)
	{
	  refs = nil;	// Can't index instance variables outside a class.
	}
      dict = [refs objectForKey: @"classvars"];
      dict = [dict objectForKey: unit];
    }
  else
    {
      dict = [refs objectForKey: type];
    }

  if ([type isEqual: @"title"] == YES)
    {
      if ([dict count] > 1)
        {
          if (!isBareStyle)
            {
              [buf appendString: indent];
              [buf appendFormat: @"<b>%@ Index</b>\n", title];
              [buf appendString: indent];
              [buf appendString: @"<ul>\n"];
              [self incIndent];
            }

          a = [dict allKeys];
          a = [a sortedArrayUsingSelector: @selector(compare:)];
          c = [a count];

          for (i = 0; i < c; i++)
            {
              NSString	*ref = [a objectAtIndex: i];
              NSString	*text = [dict objectForKey: ref];
              NSString	*file = ref;

              ref = [ref stringByReplacingString: @":" withString: @"$"];
              if ([file isEqual: base] == YES)
                {
                  continue;	// Don't list current file.
                }

              [buf appendString: indent];
              if (!isBareStyle)
                {
                  [buf appendString: @"<li>"];
                }
              [buf appendString: @"<a rel=\"gsdoc\" "];
              if (target != nil)
                {
                  [buf appendFormat: @"target=\"%@\" ", target];
                }
              if  (([type isEqual: @"protocol"] == YES)
                   && ([text hasPrefix: @"("] == NO))
                {
                  // it's an informal protocol, detected earlier as an
                  // unimplemented category of NSObject; make proper link
                  [buf appendFormat: @"href=\"%@.html#%@$NSObject%@\">(%@)</a>",
                       file, @"category", ref, text];
                }
              else
                {
                  [buf appendFormat: @"href=\"%@.html#%@$%@\">%@</a>",
                       file, type, ref, text];
                }
              if (!isBareStyle)
                {
                  [buf appendString: @"</li>"];
                }
              else
                {
                  [buf appendString: @"<br/>"];
                }
              [buf appendString: @"\n"];
            }

          if (!isBareStyle)
            {
              [self decIndent];
              [buf appendString: indent];
              [buf appendString: @"</ul>\n"];
            }
        }
    }
  else if ([dict count] > 0)
    {
      NSString		*sep = @"";
      NSString		*u = unit;
      BOOL		isInUnit = NO;

      if (unit != nil)
	{
	  if ([type isEqual: @"method"] || [type isEqual: @"ivariable"])
	    {
	      isInUnit = YES;
	      if ([type isEqual: @"ivariable"])
		{
		  sep = @"*";		// List ivars in class
		}
	      else if (classname != nil && category == nil)
		{
		  NSArray	*catNames;
		  NSDictionary	*d;

		  /*
		   * For a class, we want to list methods in any associated
		   * categories as well as those of the class itself.
		   */
		  d = [refs objectForKey: @"categories"];
		  d = [d objectForKey: classname];
		  catNames = [d allKeys];
		  if ((c = [catNames count]) > 0)
		    {
		      NSMutableDictionary	*m = [dict mutableCopy];
		      NSDictionary		*unitDict;

		      unitDict = [refs objectForKey: @"unitmethods"];
		      for (i = 0; i < c; i++)
			{
			  NSString	*catName = [catNames objectAtIndex: i];
			  NSDictionary	*catDict;
			  NSString	*cName;
			  NSEnumerator	*enumerator;
			  NSString	*mname;

			  cName = [classname stringByAppendingFormat: @"(%@)",
			    catName];
			  catDict = [unitDict objectForKey: cName];
			  enumerator = [catDict keyEnumerator];
			  /*
			   * Add category references to the dictionary,
			   * prefixing them with the category they belong to.
			   */
			  while ((mname = [enumerator nextObject]) != nil)
			    {
			      NSString	*file = [catDict objectForKey: mname];
			      NSString	*ref = [NSString stringWithFormat:
				@"(%@)%@", catName, mname];

			      [m setObject: file forKey: ref];
			    }
			}
		      dict = AUTORELEASE(m);
		    }
		}
	    }
	}

      [buf appendString: indent];
      if (!isBareStyle)
        {
          [buf appendFormat: @"<b>%@</b>\n", title];
        }
      [buf appendString: indent];
      if (!isBareStyle)
	{
	  [buf appendString: @"<ul>"];
	  [self incIndent];
	}
      [buf appendString: @"\n"];

      a = [dict allKeys];
      a = [a sortedArrayUsingSelector: @selector(compare:)];
      c = [a count];

      for (i = 0; i < c; i++)
	{
	  NSString	*ref = [a objectAtIndex: i];
	  NSString	*file = [dict objectForKey: ref];
	  NSString	*text = ref;

	  ref = [ref stringByReplacingString: @":" withString: @"$"];

	  /*
	   * If a reference to a method contains a leading category name,
	   * we don't want it in the visible method name, however if it's
	   * actually a formal protocol name, we need to make it look right
	   * by changing the round brackets to angle brackets.
	   */
	  if ([text hasPrefix: @"("] == YES)
	    {
	      NSRange	r = [text rangeOfString: @")"];

	      if (NSMaxRange(r) == [text length])	// A formal protocol
	        {
		  text = [text stringByReplacingString: @"("
					    withString: @"&lt;"];
		  text = [text stringByReplacingString: @")"
					    withString: @"&gt;"];
		}
	      else	// Category name in brackets followed by class name
	        {
		  text = [text substringFromIndex: NSMaxRange(r)];
		}
	    }

	  [buf appendString: indent];
          if (!isBareStyle)
            {
              [buf appendString: @"<li>"];
            }
	  [buf appendString: @"<a rel=\"gsdoc\" "];
          if (target != nil)
            {
              [buf appendFormat: @"target=\"%@\" ", target];
            }
	  if (isInUnit == YES)
	    {
	      [buf appendFormat: @"href=\"%@.html#%@$%@%@%@\">%@</a>",
		file, type, u, sep, ref, text];
	    }
	  else
	    {
              if  (([type isEqual: @"protocol"] == YES)
		&& ([text hasPrefix: @"&lt;"] == NO))
                {
                  // it's an informal protocol, detected earlier as an
                  // unimplemented category of NSObject; make proper link
                  text = [text stringByDeletingPrefix: @"NSObject"];
                  [buf appendFormat: @"href=\"%@.html#%@$%@\">%@</a>",
                       file, @"category", ref, text];
                }
              else
                {
                  [buf appendFormat: @"href=\"%@.html#%@$%@\">%@</a>",
                       file, type, ref, text];
                }
	    }
          if (!isBareStyle)
            {
              [buf appendString: @"</li>"];
            }
          else
            {
              [buf appendString: @"<br/>"];
            }
          [buf appendString: @"\n"];

	}

      if (!isBareStyle)
	{
	  [self decIndent];
	  [buf appendString: indent];
	  [buf appendString: @"</ul>"];
	}
      [buf appendString: @"\n"];
    }
}

- (void) outputNode: (GSXMLNode*)node to: (NSMutableString*)buf
{
  ENTER_POOL
  GSXMLNode	*children = [node firstChild];

  if ([node type] == XML_ELEMENT_NODE)
    {
      NSString		*name = [node name];
      NSDictionary	*prop = [node attributes];

      if ([name isEqual: @"back"] == YES)
	{
	  // Open back division
	  [buf appendString: indent];
	  [buf appendString: @"<div>\n"];
	  [self incIndent];
	  [self outputNodeList: children to: buf];

	  // Close back division
	  [self decIndent];
	  [buf appendString: indent];
	  [buf appendString: @"</div>\n"];
	}
      else if ([name isEqual: @"body"] == YES)
	{
	  /* Should already be in html body */
	  [self outputNodeList: children to: buf];

	  [buf appendString: indent];
	  [buf appendString: @"<br />\n"];
	  if (prevFile != nil)
	    {
	      [buf appendString: indent];
	      [buf appendFormat: @"<a href=\"%@\">Prev</a>\n", prevFile];
	    }
	  if (upFile != nil)
	    {
	      [buf appendString: indent];
	      [buf appendFormat: @"<a href=\"%@\">Up</a>\n", upFile];
	    }
	  if (nextFile != nil)
	    {
	      [buf appendString: indent];
	      [buf appendFormat: @"<a href=\"%@\">Next</a>\n", nextFile];
	    }

	  [self decIndent];
	  [buf appendString: indent];

          [buf appendString: indent];
          [buf appendString: @"</font>\n"];

	  [buf appendString: @"</body>\n"];
	}
      else if ([name isEqual: @"br"] == YES)
	{
	  [buf appendString: @"<br />"];
	}
      else if ([name isEqual: @"category"] == YES)
	{
	  NSString	*s;

	  category = [prop objectForKey: @"name"];
	  classname = [prop objectForKey: @"class"];
	  unit = [NSString stringWithFormat: @"%@(%@)", classname, category];
	  [buf appendString: indent];
	  [buf appendString: @"<h2>"];
	  [buf appendString: [self typeRef: classname]];
	  [buf appendString: @"("];
	  s = [self makeAnchor: unit ofType: @"category" name: category];
	  [buf appendString: s];
	  [buf appendString: @")</h2>\n"];
	  [self outputUnit: node to: buf];
	  unit = nil;
	  classname = nil;
	  category = nil;
	}
      else if ([name isEqual: @"chapter"] == YES)
	{
	  heading = @"h1";
	  chap++;
	  sect = 0;
	  ssect = 0;
	  sssect = 0;
	  [self outputNodeList: children to: buf];
	  heading = nil;
	}
      else if ([name isEqual: @"class"] == YES)
	{
	  NSString	*sup = [prop objectForKey: @"super"];

	  classname = [prop objectForKey: @"name"];
	  unit = classname;
	  [buf appendString: indent];
	  [buf appendString: @"<h2>"];
	  [buf appendString:
	    [self makeAnchor: classname ofType: @"class" name: classname]];
	  if (sup != nil)
	    {
	      sup = [self typeRef: sup];
	      if (sup != nil)
		{
		  [buf appendString: @" : "];
		  [buf appendString: sup];
		}
	    }
	  [buf appendString: @"</h2>\n"];
	  [self outputUnit: node to: buf];
	  unit = nil;
	  classname = nil;
	}
      else if ([name isEqual: @"code"] == YES)
	{
	  [buf appendString: @"<code>"];
	  [self outputText: children to: buf];
	  [buf appendString: @"</code>"];
	}
      else if ([name isEqual: @"constant"] == YES)
	{
	  NSString	*nam;
	  NSString	*str;
	  NSString	*s;

	  nam = [prop objectForKey: @"name"];
	  str = [prop objectForKey: @"type"];
	  str = [self typeRef: str];
	  str = [str stringByAppendingFormat: @" %@", nam];

	  /*
	   * Output heading.
	   */
	  [buf appendString: indent];
	  [buf appendString: @"<h3>"];
	  s = [self makeLink: nam ofType: @"constant" isRef: NO];
	  if (s != nil)
	    {
	      [buf appendString: s];
	      [buf appendString: nam];
	      [buf appendString: @"</a>"];
	    }
	  else
	    {
	      [buf appendString: nam];
	    }
	  [buf appendString: @"</h3>\n"];
	  [buf appendString: indent];
	  [buf appendString: str];
	  [buf appendString: @";<br />\n"];

	  node = firstElement(children);

	  if ([[node name] isEqual: @"declared"] == YES)
	    {
	      [self outputNode: node to: buf];
	      node = [node nextElement];
	    }

	  children = node;
	  if ([[children name] isEqual: @"standards"])
	    {
	      node = [node nextElement];
	    }
	  [self outputVersion: prop to: buf];

	  if ([[node name] isEqual: @"desc"])
	    {
	      [self outputNode: node to: buf];
	    }

	  [buf appendString: indent];
	  [buf appendString: @"<hr width=\"25%\" align=\"left\" />\n"];
	}
      else if ([name isEqual: @"contents"] == YES)
        {
	  NSDictionary	*dict;

	  dict = [[localRefs refs] objectForKey: @"contents"];
	  if ([dict count] > 1)
	    {
	      NSArray	*a;
	      unsigned	i;
	      unsigned	l = 0;

	      [buf appendString: indent];
	      [buf appendString: @"<hr width=\"50%\" align=\"left\" />\n"];
	      [buf appendString: indent];
	      [buf appendString: @"<h3>Contents -</h3>\n"];

	      a = [dict allKeys];
	      a = [a sortedArrayUsingSelector: @selector(compare:)];
	      for (i = 0; i < [a count]; i++)
		{
		  NSString	*k = [a objectAtIndex: i];
		  NSString	*v = [dict objectForKey: k];
		  unsigned	pos = 3;

		  if ([k hasSuffix: @"000"] == YES)
		    {
		      pos = 2;
		      if ([k hasSuffix: @"000000"] == YES)
			{
			  pos = 1;
			  if ([k hasSuffix: @"000"] == YES)
			    {
			      pos = 0;
			    }
			}
		      if (l == pos)
			{
			  [buf appendString: indent];
			  [buf appendString: @"<ol>\n"];
			  [self incIndent];
			}
		      else
			{
			  while (l > pos + 1)
			    {
			      [self decIndent];
			      [buf appendString: indent];
			      [buf appendString: @"</li>\n"];
			      [self decIndent];
			      [buf appendString: indent];
			      [buf appendString: @"</ol>\n"];
			      l--;
			    }
			  if (l == pos + 1)
			    {
			      [self decIndent];
			      [buf appendString: indent];
			      [buf appendString: @"</li>\n"];
			      l--;
			    }
			}
		    }
		  [buf appendString: indent];
		  [buf appendString: @"<li>\n"];
		  [self incIndent];
		  [buf appendString: indent];
		  [buf appendFormat: @"<a href=\"#%@\">%@</a>\n", k, v];
		  if (pos == 3)
		    {
		      [self decIndent];
		      [buf appendString: indent];
		      [buf appendString: @"</li>\n"];
		    }
		  else
		    {
		      l++;
		    }
		}
	      while (l > 0)
		{
		  [self decIndent];
		  [buf appendString: indent];
		  [buf appendString: @"</li>\n"];
		  [self decIndent];
		  [buf appendString: indent];
		  [buf appendString: @"</ol>\n"];
		  l--;
		}
	      [buf appendString: indent];
	      [buf appendString: @"<hr width=\"50%\" align=\"left\" />\n"];
	    }
	}
      else if ([name isEqual: @"declared"] == YES)
	{
	  [buf appendString: indent];
          [buf appendString: @"<blockquote class=\"declared\">\n"];
	  [self incIndent];
	  [buf appendString: indent];
	  [buf appendString: @"<dl>\n"];
	  [self incIndent];
	  [buf appendString: indent];
	  [buf appendString: @"<dt><b>Declared in:</b></dt>\n"];
	  [buf appendString: indent];
	  [buf appendString: @"<dd>"];
	  [self outputText: [node firstChild] to: buf];
	  [buf appendString: @"</dd>\n"];
	  [self decIndent];
	  [buf appendString: indent];
	  [buf appendString: @"</dl>\n"];
	  [self decIndent];
	  [buf appendString: indent];
	  [buf appendString: @"</blockquote>\n"];
	}
      else if ([name isEqual: @"desc"] == YES)
	{
	  [buf appendString: indent];
          [buf appendString: @"<div class=\"desc\">\n"];
	  [self incIndent];
	  while (children != nil)
	    {
	      children = [self outputBlock: children to: buf inPara: YES];
	    }
	  [self decIndent];
	  [buf appendString: indent];
	  [buf appendString: @"</div>\n"];
	}
      else if ([name isEqual: @"em"] == YES)
	{
	  [buf appendString: @"<em>"];
	  [self outputText: children to: buf];
	  [buf appendString: @"</em>"];
	}
      else if ([name isEqual: @"email"] == YES)
	{
	  NSString	*ename;

	  ename = [prop objectForKey: @"address"];
	  if (ename == nil)
	    {
	      [buf appendString: @"<code>"];
	    }
	  else
	    {
	      [buf appendFormat: @"<a href=\"mailto:%@\"><code>", ename];
	    }
	  //
	  // [node firstChild] doesn't look like it points to
	  // the mail address.
	  // not sure _where_ it points to though...
	  //
#if 0
	  [self outputText: [node firstChild] to: buf];
#endif
	  if (ename == nil)
	    {
	      [buf appendString: @"</code>"];
	    }
	  else
	    {
	      [buf appendFormat: @"%@</code></a>", ename];
	    }
	}
      else if ([name isEqual: @"embed"] == YES)
	{
	  [self outputBlock: node to: buf inPara: NO];
	}
      else if ([name isEqual: @"entry"])
	{
	  NSString		*text;
	  NSString		*val;

	  text = [children escapedContent];
	  val = [prop objectForKey: @"id"];
	  if (val == nil)
	    {
	      val = text;
	      if (val == nil) val = @"";
	    }
	  [buf appendString: [self makeAnchor: val ofType: @"label" name: @""]];
	}
      else if ([name isEqual: @"example"] == YES)
	{
	  [self outputBlock: node to: buf inPara: NO];
	}
      else if ([name isEqual: @"file"] == YES)
	{
	  [buf appendString: @"<code>"];
	  [self outputText: children to: buf];
	  [buf appendString: @"</code>"];
	}
      else if ([name isEqual: @"footnote"] == YES)
	{
	  [buf appendString: @"<blockquote>"];
	  [self outputText: children to: buf];
	  [buf appendString: @"</blockquote>"];
	}
      else if ([name isEqual: @"front"] == YES)
	{
	  // Open front division
	  [buf appendString: indent];
	  [buf appendString: @"<div>\n"];
	  [self incIndent];
	  [self outputNodeList: children to: buf];
	  // Close front division
	  [self decIndent];
	  [buf appendString: indent];
	  [buf appendString: @"</div>\n"];
	}
      else if ([name isEqual: @"function"] == YES)
	{
	  NSString	*fun;
	  NSString	*str;
	  NSString	*s;
	  GSXMLNode	*tmp = children;
	  BOOL		hadArg = NO;

	  fun = [prop objectForKey: @"name"];
	  str = [prop objectForKey: @"type"];
	  str = [self typeRef: str];
	  str = [str stringByAppendingFormat: @" %@(", fun];
	  children = nil;
	  while (tmp != nil)
	    {
	      if ([tmp type] == XML_ELEMENT_NODE)
		{
		  if ([[tmp name] isEqual: @"arg"] == YES)
		    {
		      GSXMLNode		*t = [tmp firstChild];
		      NSString		*s;

		      if (hadArg == YES)
			{
			  str = [str stringByAppendingString: @", "];
			}

		      s = [[tmp attributes] objectForKey: @"type"];
		      s = [self typeRef: s];
		      str = [str stringByAppendingString: s];

		      str = [str stringByAppendingString: @" <b>"];
		      while (t != nil)
			{
			  if ([t type] == XML_TEXT_NODE)
			    {
			      NSString	*content = [t escapedContent];

			      if (content == nil) content = @"";
			      str = [str stringByAppendingString: content];
			    }
			  t = [t next];
			}
		      str = [str stringByAppendingString: @"</b>"];
		      hadArg = YES;
		    }
		  else if ([[tmp name] isEqual: @"vararg"] == YES)
		    {
                      str = [str stringByAppendingString: @"<b>,...</b>"];
		      children = [tmp nextElement];
		      break;
		    }
		  else
		    {
		      children = tmp;
		      break;
		    }
		}
	      tmp = [tmp nextElement];
	    }

	  /*
	   * Output function heading.
	   */
	  [buf appendString: indent];
	  [buf appendString: @"<h3>"];
	  s = [self makeLink: fun ofType: @"function" isRef: NO];
	  if (s != nil)
	    {
	      [buf appendString: s];
	      [buf appendString: fun];
	      [buf appendString: @"</a>"];
	    }
	  else
	    {
	      [buf appendString: fun];
	    }
	  [buf appendString: @"</h3>\n"];
	  [buf appendString: indent];
	  [buf appendString: str];
	  [buf appendString: @");<br />\n"];

	  node = firstElement(children);

	  if ([[node name] isEqual: @"declared"] == YES)
	    {
	      [self outputNode: node to: buf];
	      node = [node nextElement];
	    }

	  children = node;
	  if ([[children name] isEqual: @"standards"])
	    {
	      node = [node nextElement];
	    }
	  [self outputVersion: prop to: buf];

	  if ([[node name] isEqual: @"desc"])
	    {
	      [self outputNode: node to: buf];
	    }

	  [buf appendString: indent];
	  [buf appendString: @"<hr width=\"25%\" align=\"left\" />\n"];
	}
      else if ([name isEqual: @"gsdoc"] == YES)
	{
	  base = [prop objectForKey: @"base"];
	  if (base == nil)
	    {
	      NSLog(@"No 'base' document name supplied in gsdoc element");
	    }
	  else
	    {
	      NSString	*stylesheetURL = [prop objectForKey: @"stylesheeturl"];

	      nextFile = [prop objectForKey: @"next"];
	      nextFile = [nextFile stringByAppendingPathExtension: @"html"];
	      prevFile = [prop objectForKey: @"prev"];
	      prevFile = [prevFile stringByAppendingPathExtension: @"html"];
	      upFile = [prop objectForKey: @"up"];
	      upFile = [upFile stringByAppendingPathExtension: @"html"];

	      // special formatting for table-of-contents frames; ultimately
	      // this should be moved to stylesheet
	      isContentsDoc = ((stylesheetURL != nil) &&
		([stylesheetURL rangeOfString: @"gsdoc_contents"].length > 0))
		? YES : NO;

	      [self outputNodeList: children to: buf];
	    }
	}
      else if ([name isEqual: @"head"] == YES)
	{
          NSString	*headerTag;

	  [buf appendString: indent];
	  [buf appendString: @"<head>\n"];
	  [self incIndent];
	  children = firstElement(children);
	  [buf appendString: indent];
	  [buf appendString: @"<title>"];
	  [self incIndent];
	  [self outputText: [children firstChild] to: buf];
	  [self decIndent];
	  [buf appendString: @"</title>\n"];
#if 0
          /** Css : TODO print.css **/
          [buf appendString:@"<meta http-equiv=\"Content-Style-Type\" content=\"text/css\"/>\n"];
          [buf appendString:@"<link rel=\"stylesheet\" type=\"text/css\" href=\"screen.css\" media=\"screen\" title=\"Normal\" />\n"];
          /** Robots **/
          [buf appendString:@"<meta name=\"robots\" content=\"all\" />\n"];
#endif
	  [self decIndent];
	  [buf appendString: indent];
	  [buf appendString: @"</head>\n"];
	  [buf appendString: indent];
	  [buf appendString: @"<body>\n"];
	  [self incIndent];

          // special formatting for table-of-contents frames; ultimately
          // this should be moved to stylesheet
          if (isContentsDoc)
            {
              [buf appendString: indent];
              [buf appendFormat: @"<font face=\"%@\" size=\"-1\">\n", tocFont];
            }
          else
            {
              [buf appendString: indent];
              [buf appendFormat: @"<font face=\"%@\">\n", mainFont];
            }

	  if (prevFile != nil)
	    {
	      [buf appendString: indent];
	      [buf appendFormat: @"<a href=\"%@\">Prev</a>\n", prevFile];
	    }
	  if (upFile != nil)
	    {
	      [buf appendString: indent];
	      [buf appendFormat: @"<a href=\"%@\">Up</a>\n", upFile];
	    }
	  if (nextFile != nil)
	    {
	      [buf appendString: indent];
	      [buf appendFormat: @"<a href=\"%@\">Next</a>\n", nextFile];
	    }
          if (prevFile != nil || upFile != nil || nextFile != nil)
            {
              [buf appendString: indent];
              [buf appendString: @"<br />\n"];
            }

	  [buf appendString: indent];
          if (isContentsDoc)
            {
              headerTag = @"h2";
            }
          else
            {
              headerTag = @"h1";
            }
	  [buf appendFormat: @"<%@><a name=\"title$%@\">", headerTag, base];
	  [self outputText: [children firstChild] to: buf];
	  [buf appendFormat: @"</a></%@>\n", headerTag];

	  children = [children nextElement];
	  if ([[children name] isEqual: @"author"] == YES)
	    {
	      [buf appendString: indent];
	      [buf appendString: @"<h3>Authors</h3>\n"];
	      [buf appendString: indent];
	      [buf appendString: @"<dl>\n"];
	      [self incIndent];
	      while ([[children name] isEqual: @"author"] == YES)
		{
		  GSXMLNode		*author = children;
		  GSXMLNode		*tmp;
		  GSXMLNode		*email = nil;
		  GSXMLNode		*url = nil;
		  GSXMLNode		*desc = nil;

		  children = [children nextElement];

		  tmp = [author firstChildElement];
		  if ([[tmp name] isEqual: @"email"] == YES)
		    {
		      email = tmp;
		      tmp = [tmp nextElement];
		    }
		  if ([[tmp name] isEqual: @"url"] == YES)
		    {
		      url = tmp;
		      tmp = [tmp nextElement];
		    }
		  if ([[tmp name] isEqual: @"desc"] == YES)
		    {
		      desc = tmp;
		    }

		  [buf appendString: indent];
		  if (url == nil)
		    {
		      [buf appendString: @"<dt>"];
		      [buf appendString: [[[author attributes]
			objectForKey: @"name"] stringByEscapingXML]];
		    }
		  else
		    {
		      [buf appendString: @"<dt><a href=\""];
		      [buf appendString: [[url attributes]
			objectForKey: @"url"]];
		      [buf appendString: @"\">"];
		      [buf appendString: [[[author attributes]
			objectForKey: @"name"] stringByEscapingXML]];
		      [buf appendString: @"</a>"];
		    }
		  if (email != nil)
		    {
		      //
		      // Add a beautifier ' ' otherwise we'll get a
		      //   <dt>John Doe(<a href="mailto:...
		      // or
		      //   <dt><a href...>John Doe</a>(<a href...
		      //
		      [buf appendString: @" ("];
		      [self outputNode: email to: buf];
		      [buf appendString: @")"];
		    }
		  [buf appendString: @"</dt>\n"];
		  [buf appendString: indent];
		  [buf appendString: @"<dd>\n"];
		  if (desc != nil)
		    {
		      [self incIndent];
                      [self outputNode: desc to: buf];
		      [self decIndent];
		    }
		  [buf appendString: indent];
		  [buf appendString: @"</dd>\n"];
		}
	      [self decIndent];
	      [buf appendString: indent];
	      [buf appendString: @"</dl>\n"];
	    }
	  if ([[children name] isEqual: @"version"] == YES)
	    {
	      [buf appendString: indent];
	      [buf appendString: @"<p><b>Version:</b> "];
	      [self outputText: [children firstChild] to: buf];
	      [buf appendString: @"</p>\n"];
	      children = [children nextElement];
	    }
	  if ([[children name] isEqual: @"date"] == YES)
	    {
	      [buf appendString: indent];
	      [buf appendString: @"<p><b>Date:</b> "];
	      [self outputText: [children firstChild] to: buf];
	      [buf appendString: @"</p>\n"];
	      children = [children nextElement];
	    }
	  if ([[children name] isEqual: @"abstract"] == YES)
	    {
	      GSXMLNode	*tmp = [children firstChild];

	      [buf appendString: indent];
	      [buf appendString: @"<blockquote>\n"];
	      [self incIndent];
	      while (tmp != nil)
		{
		  tmp = [self outputBlock: tmp to: buf inPara: NO];
		}
	      [self decIndent];
	      [buf appendString: indent];
	      [buf appendString: @"</blockquote>\n"];
	      children = [children nextElement];
	    }
	  if ([[children name] isEqual: @"copy"] == YES)
	    {
	      [buf appendString: indent];
	      [buf appendString: @"<p><b>Copyright:</b> (C) "];
	      [self outputText: [children firstChild] to: buf];
	      [buf appendString: @"</p>\n"];
	    }
	}
      else if ([name isEqual: @"heading"] == YES)
	{
	  if (heading == nil)
	    {
	    }
	  else
	    {
	      [buf appendString: indent];
	      [buf appendString: @"<"];
	      [buf appendString: heading];
	      [buf appendString: @">"];
	      [buf appendFormat: @"<a name=\"%03u%03u%03u%03u\">",
		chap, sect, ssect, sssect];
	      [self outputText: children to: buf];
	      [buf appendString: @"</a></"];
	      [buf appendString: heading];
	      [buf appendString: @">\n"];
	      heading = nil;
	    }
	}
      else if ([name isEqual: @"index"] == YES)
        {
	  NSString	*scope = [prop objectForKey: @"scope"];
	  NSString	*type = [prop objectForKey: @"type"];
	  NSString	*target = [prop objectForKey: @"target"];
	  NSString	*title = [type capitalizedString];
	  NSString	*style = [prop objectForKey: @"style"];

	  [self outputIndex: type scope: scope title: title style: style
                target: target to: buf ];
	}
      else if ([name isEqual: @"ivar"] == YES)	// %phrase
	{
	  [buf appendString: @"<var>"];
	  [self outputText: children to: buf];
	  [buf appendString: @"</var>"];
	}
      else if ([name isEqual: @"ivariable"] == YES)
	{
	  NSString	*n = [prop objectForKey: @"name"];
	  NSString	*t = [prop objectForKey: @"type"];
	  NSString	*v = [prop objectForKey: @"validity"];
	  NSString	*s;
	  GSXMLNode	*tmp;

	  tmp = children = firstElement(children);
	  [buf appendString: indent];
	  [buf appendString: @"<h3>"];
	  s = [self makeLink: n ofType: @"ivariable" inUnit: nil isRef: NO];
	  if (s != nil)
	    {
	      [buf appendString: s];
	      [buf appendString: n];
	      [buf appendString: @"</a>"];
	    }
	  else
	    {
	      [buf appendString: n];
	    }
	  [buf appendString: @"</h3>\n"];
	  if (v == nil)
	    {
	      v = @"public";
	    }
	  [buf appendFormat: @"%@@%@ %@ <b>%@</b>;<br />\n", indent, v, t, n];

/*
	  if ([[children name] isEqual: @"desc"] == YES)
	    {
	      children = [children nextElement];
	    }
*/

	  /*
	   * List standards with which ivar complies
	   */
	  [self outputVersion: prop to: buf];
	  if ([[tmp name] isEqual: @"desc"])
	    {
	      [self outputNode: tmp to: buf];
	    }

	  [buf appendString: indent];
	  [buf appendString: @"<hr width=\"25%\" align=\"left\" />\n"];
	}
      else if ([name isEqual: @"label"] == YES)	// %anchor
	{
	  NSString		*text;
	  NSString		*val;

	  text = [children escapedContent];
	  val = [prop objectForKey: @"id"];
	  if (val == nil)
	    {
	      val = text;
	      if (val == nil) val = @"";
	    }
	  [buf appendString:
	    [self makeAnchor: val ofType: @"label" name: text]];
	}
      else if ([name isEqual: @"macro"] == YES)
	{
	  NSString	*mac;
	  NSString	*str;
	  NSString	*s;
	  GSXMLNode	*tmp = children;
	  BOOL		hadArg = NO;

	  mac = [prop objectForKey: @"name"];
	  str = [NSString stringWithFormat: @" %@", mac];
	  children = nil;
	  while (tmp != nil)
	    {
	      if ([tmp type] == XML_ELEMENT_NODE)
		{
		  if ([[tmp name] isEqual: @"arg"] == YES)
		    {
		      GSXMLNode		*t = [tmp firstChild];

		      if (hadArg == YES)
			{
			  str = [str stringByAppendingString: @", "];
			}
		      else
		      	{
			  str = [str stringByAppendingString: @"("];
			}

		      str = [str stringByAppendingString: @"<b>"];
		      while (t != nil)
			{
			  if ([t type] == XML_TEXT_NODE)
			    {
			      NSString	*content = [t escapedContent];

			      if (content == nil) content = @"";
			      str = [str stringByAppendingString: content];
			    }
			  t = [t next];
			}
		      str = [str stringByAppendingString: @"</b>"];
		      hadArg = YES;
		    }
		  else if ([[tmp name] isEqual: @"vararg"] == YES)
		    {
		      if (hadArg == YES)
			{
			  str = [str stringByAppendingString: @"<b>,...</b>"];
			}
		      else
			{
			  str = [str stringByAppendingString: @"<b>(...</b>"];
			}
		      children = [tmp nextElement];
		      hadArg = YES;
		      break;
		    }
		  else
		    {
		      children = tmp;
		      break;
		    }
		}
	      tmp = [tmp nextElement];
	    }

	  /*
	   * Output macro heading.
	   */
	  [buf appendString: indent];
	  [buf appendString: @"<h3>"];
	  s = [self makeLink: mac ofType: @"macro" isRef: NO];
	  if (s != nil)
	    {
	      [buf appendString: s];
	      [buf appendString: mac];
	      [buf appendString: @"</a>"];
	    }
	  else
	    {
	      [buf appendString: mac];
	    }
	  [buf appendString: @"</h3>\n"];
	  [buf appendString: indent];
	  [buf appendString: str];
	  if (hadArg == YES)
	    {
	      [buf appendString: @")"];
	    }
	  [buf appendString: @"<br />\n"];

	  node = firstElement(children);

	  if ([[node name] isEqual: @"declared"] == YES)
	    {
	      [self outputNode: node to: buf];
	      node = [node nextElement];
	    }

	  children = node;
	  if ([[children name] isEqual: @"standards"])
	    {
	      node = [node nextElement];
	    }
	  [self outputVersion: prop to: buf];

	  if ([[node name] isEqual: @"desc"])
	    {
	      [self outputNode: node to: buf];
	    }

	  [buf appendString: indent];
	  [buf appendString: @"<hr width=\"25%\" align=\"left\" />\n"];
	}
      else if ([name isEqual: @"method"] == YES)
	{
	  NSString	*sel;
	  NSString	*str;
	  GSXMLNode	*tmp = children;
	  BOOL		hadArg = NO;

          [buf appendString:@"<div class=\"method\">\n"];

	  sel = [prop objectForKey: @"factory"];
	  str = [prop objectForKey: @"type"];
	  if (sel != nil && [sel boolValue] == YES)
	    {
	      sel = @"+";
	      str = [NSString stringWithFormat: @"+ (%@) ",
		[self typeRef: str]];
	    }
	  else
	    {
	      sel = @"-";
	      str = [NSString stringWithFormat: @"- (%@) ",
		[self typeRef: str]];
	    }
	  children = nil;
	  while (tmp != nil)
	    {
	      if ([tmp type] == XML_ELEMENT_NODE)
		{
		  if ([[tmp name] isEqual: @"sel"] == YES)
		    {
		      GSXMLNode	*t = [tmp firstChild];

		      str = [str stringByAppendingString: @"<b>"];
		      while (t != nil)
			{
			  if ([t type] == XML_TEXT_NODE)
			    {
			      NSString	*content = [t escapedContent];

			      if (content == nil) content = @"";
			      sel = [sel stringByAppendingString: content];
                              // these nbsp added for readability, but must
                              // be removed below when making href link
                              sel = [sel stringByAppendingString: @"&nbsp;"];
			      if (hadArg == YES)
				{
				  str = [str stringByAppendingString: @" "];
				}
			      str = [str stringByAppendingString: content];
			    }
			  t = [t next];
			}
		      str = [str stringByAppendingString: @"</b>"];
		    }
		  else if ([[tmp name] isEqual: @"arg"] == YES)
		    {
		      GSXMLNode	*t = [tmp firstChild];
		      NSString	*s;

		      s = [[tmp attributes] objectForKey: @"type"];
		      s = [self typeRef: s];
		      str = [str stringByAppendingFormat: @" (%@)", s];
		      while (t != nil)
			{
			  if ([t type] == XML_TEXT_NODE)
			    {
			      NSString	*content = [t escapedContent];
			
			      if (content == nil) content = @"";
			      str = [str stringByAppendingString: content];
			    }
			  t = [t next];
			}
		      hadArg = YES;	// Say we have found an arg.
		    }
		  else if ([[tmp name] isEqual: @"vararg"] == YES)
		    {
		      sel = [sel stringByAppendingString: @",..."];
		      str = [str stringByAppendingString: @"<b>,...</b>"];
		      children = [tmp nextElement];
		      break;
		    }
		  else
		    {
		      children = tmp;
		      break;
		    }
		}
	      tmp = [tmp nextElement];
	    }
	  if ([sel length] > 1)
	    {
	      NSString	*s;
              NSMutableString *linkRef;

	      /*
	       * Output selector heading.
	       */
	      [buf appendString: indent];
	      [buf appendString: @"<h3>"];
              // get rid of nbsps put in for readability above
              linkRef = [NSMutableString stringWithCapacity: [sel length]];
              [linkRef setString:sel];
              [linkRef replaceString: @"&nbsp;" withString: @""];

	      s = [self makeLink: linkRef
			  ofType: @"method"
			  inUnit: nil
			   isRef: NO];
	      if (s != nil)
		{
		  [buf appendString: s];
		  [buf appendString: [sel substringFromIndex: 1]];
		  [buf appendString: @"</a>"];
		}
	      else
		{
		  [buf appendString: [sel substringFromIndex: 1]];
		}
	      [buf appendString: @"</h3>\n"];
	      [buf appendString: indent];
	      [buf appendString: str];
	      [buf appendString: @";<br />\n"];

	      node = firstElement(children);

	      /*
	       * List standards with which method complies
	       */
	      children = node;
	      if ([[children name] isEqual: @"standards"])
		{
		  node = [node nextElement];
		}
	      [self outputVersion: prop to: buf];

	      if ((str = [prop objectForKey: @"init"]) != nil
		&& [str boolValue] == YES)
		{
		  [buf appendString: @"This is a designated initialiser "
		    @"for the class.<br />\n"];
		}
	      str = [prop objectForKey: @"override"];
	      if ([str isEqual: @"subclass"] == YES)
		{
		  [buf appendString: @"Subclasses <strong>must</strong> "
		    @"override this method.<br />\n"];
		}
	      else if ([str isEqual: @"dummy"] == YES)
		{
		  [buf appendString: @"An empty method provided for subclasses "
		    @"to override.<br />\n"];
		}
	      else if ([str isEqual: @"never"] == YES)
		{
		  [buf appendString: @"Subclasses must <strong>NOT</strong> "
		    @"override this method.<br />\n"];
		}

	      if ([[node name] isEqual: @"desc"])
		{
		  [self outputNode: node to: buf];
		}
	      [buf appendString: indent];
	      [buf appendString: @"<hr width=\"25%\" align=\"left\" />\n"];
	    }
          [buf appendString:@"</div>\n"];
	}
      else if ([name isEqual: @"p"] == YES)
	{
	  [self outputBlock: node to: buf inPara: NO];
	}
      else if ([name isEqual: @"prjref"] == YES)
	{
	  NSLog(@"Element '%@' not implemented", name); 	    // FIXME
	}
      else if ([name isEqual: @"ref"] == YES)	// %xref
	{
	  NSString	*type = [prop objectForKey: @"type"];
	  NSString	*r = [prop objectForKey: @"id"];
	  GSXMLNode	*tmp = [node firstChild];
	  NSString	*c = [prop objectForKey: @"class"];
	  NSString	*s;

          // fill in default value
          if ((type == nil) || [type isEqual: @""])
	    {
              type = @"label";
	    }
	  if ([type isEqual: @"method"] || [type isEqual: @"ivariable"])
	    {
	      s = [self makeLink: r ofType: type inUnit: c isRef: YES];
	    }
	  else
	    {
	      s = [self makeLink: r ofType: type isRef: YES];
	      /**
	       * As a special case, if we have a reference to a function,
	       * and we can't find it, we check to see if there is actually
	       * a macro of that name and refer to that instead.
	       */
	      if (s == nil && [type isEqual: @"function"] == YES)
		{
		  s = [self makeLink: r ofType: @"macro" isRef: YES];
		}
	    }
	  if (s == nil)
	    {
	      if (c == nil)
		{
		  NSLog(@"Location of %@ '%@' (referenced from %@) "
		    @"not found or not unique.", type, r, base);
		}
	      else
		{
		  NSLog(@"Location of the %@ version of %@ '%@' (referenced "
		    @"from %@) not found.", c, type, r, base);
		}
	      if (tmp == nil)
		{
		  [buf appendString: r];
		}
	      else
		{
		  [self outputText: tmp to: buf];
		}
	      [buf appendString: @"\n"];
	    }
	  else
	    {
	      [buf appendString: s];
	      if (tmp == nil)
		{
		  [buf appendString: r];
		}
	      else
		{
		  [self outputText: tmp to: buf];
		}
	      [buf appendString: @"</a>\n"];
	    }
	}
      else if ([name isEqual: @"protocol"] == YES)
	{
	  NSString	*value = [prop objectForKey: @"name"];

	  unit = [NSString stringWithFormat: @"(%@)", value];
	  [buf appendString: indent];
	  [buf appendString: @"<h2>"];
	  [buf appendString:
	    [self makeAnchor: unit ofType: @"protocol" name: value]];
	  [buf appendString: @"</h2>\n"];
	  [self outputUnit: node to: buf];
	  unit = nil;
	}
      else if ([name isEqual: @"EOEntity"] == YES
	|| [name isEqual: @"EOModel"] == YES)
	{
	  NSLog(@"Element '%@' not implemented", name); 	    // FIXME
	}
      else if ([name isEqual: @"section"] == YES)
	{
	  heading = @"h2";
	  sect++;
	  ssect = 0;
	  sssect = 0;
	  [self outputNodeList: children to: buf];
	  heading = @"h1";
	}
      else if ([name isEqual: @"site"] == YES)
	{
	  [buf appendString: @"<code>"];
	  [self outputText: children to: buf];
	  [buf appendString: @"</code>"];
	}
      else if ([name isEqual: @"standards"])
	{
	}
      else if ([name isEqual: @"strong"] == YES)
	{
	  [buf appendString: @"<strong>"];
	  [self outputText: children to: buf];
	  [buf appendString: @"</strong>"];
	}
      else if ([name isEqual: @"subsect"] == YES)
	{
	  heading = @"h3";
	  ssect++;
	  sssect = 0;
	  [self outputNodeList: children to: buf];
	  heading = @"h2";
	}
      else if ([name isEqual: @"subsubsect"] == YES)
	{
	  heading = @"h4";
	  sssect++;
	  [self outputNodeList: children to: buf];
	  heading = @"h3";
	}
      else if ([name isEqual: @"type"] == YES)
	{
	  NSString	*nam;
	  NSString	*str;
	  NSString	*s;

	  nam = [prop objectForKey: @"name"];
	  str = [prop objectForKey: @"type"];
	  str = [self typeRef: str];
	  str = [NSString stringWithFormat: @"typedef %@ %@", str, nam];

	  /*
	   * Output typedef heading.
	   */
	  [buf appendString: indent];
	  [buf appendString: @"<h3>"];
	  s = [self makeLink: nam ofType: @"type" isRef: NO];
	  if (s != nil)
	    {
	      [buf appendString: s];
	      [buf appendString: nam];
	      [buf appendString: @"</a>"];
	    }
	  else
	    {
	      [buf appendString: nam];
	    }
	  [buf appendString: @"</h3>\n"];
	  [buf appendString: indent];
	  [buf appendString: str];
	  [buf appendString: @";<br />\n"];

	  node = firstElement(children);

	  if (node != nil && [[node name] isEqual: @"declared"] == YES)
	    {
	      [self outputNode: node to: buf];
	      node = [node nextElement];
	    }

	  children = node;
	  if ([[children name] isEqual: @"standards"])
	    {
	      node = [node nextElement];
	    }
	  [self outputVersion: prop to: buf];

	  if (node != nil && [[node name] isEqual: @"desc"] == YES)
	    {
	      [self outputNode: node to: buf];
	    }

	  [buf appendString: indent];
	  [buf appendString: @"<hr width=\"25%\" align=\"left\" />\n"];
	}
      else if ([name isEqual: @"uref"] == YES)
	{
	  [buf appendString: @"<a href=\""];
	  [buf appendString: [prop objectForKey: @"url"]];
	  [buf appendString: @"\">"];
	  [self outputText: children to: buf];
	  [buf appendString: @"</a>"];
	}
      else if ([name isEqual: @"url"] == YES)
	{
          // create an HREF as before but use the URL itself as marker text
	  [buf appendString: @"<a href=\""];
	  [buf appendString: [prop objectForKey: @"url"]];
	  [buf appendString: @"\">"];
          [buf appendString: [prop objectForKey: @"url"]];
	  [buf appendString: @"</a>"];
	}
      else if ([name isEqual: @"var"] == YES)	// %phrase
	{
	  [buf appendString: @"<var>"];
	  [self outputText: children to: buf];
	  [buf appendString: @"</var>"];
	}
      else if ([name isEqual: @"variable"] == YES)
	{
	  NSString	*nam;
	  NSString	*str;
	  NSString	*s;

	  nam = [prop objectForKey: @"name"];
	  str = [prop objectForKey: @"type"];
	  str = [self typeRef: str];
	  str = [str stringByAppendingFormat: @" %@", nam];

	  /*
	   * Output variable heading.
	   */
	  [buf appendString: indent];
	  [buf appendString: @"<h3>"];
	  s = [self makeLink: nam ofType: @"variable" isRef: NO];
	  if (s != nil)
	    {
	      [buf appendString: s];
	      [buf appendString: nam];
	      [buf appendString: @"</a>"];
	    }
	  else
	    {
	      [buf appendString: nam];
	    }
	  [buf appendString: @"</h3>\n"];
	  [buf appendString: indent];
	  [buf appendString: str];
	  [buf appendString: @";<br />\n"];

	  node = firstElement(children);

	  if ([[node name] isEqual: @"declared"] == YES)
	    {
	      [self outputNode: node to: buf];
	      node = [node nextElement];
	    }

	  children = node;
	  if ([[children name] isEqual: @"standards"])
	    {
	      node = [node nextElement];
	    }
	  [self outputVersion: prop to: buf];

	  if ([[node name] isEqual: @"desc"])
	    {
	      [self outputNode: node to: buf];
	    }

	  [buf appendString: indent];
	  [buf appendString: @"<hr width=\"25%\" align=\"left\" />\n"];
	}
      else
	{
	  GSXMLNode	*tmp;
	  /*
	   * Try outputting as any of the list elements.
	   */
	  tmp = [self outputList: node to: buf];
	  if (tmp == node)
	    {
	      NSLog(@"Element '%@' not implemented", name);	// FIXME
	    }
	}
    }
  LEAVE_POOL
}

/**
 * Output all the nodes from this one onwards ... try to output
 * as text first, if not possible, call the main method to output
 * each node.
 */
- (void) outputNodeList: (GSXMLNode*)node to: (NSMutableString*)buf
{
  while (node != nil)
    {
      GSXMLNode	*next = [node nextElement];
      GSXMLNode	*tmp;

      tmp = [self outputText: node to: buf];
      if (tmp == node)
        {
	  [self outputNode: node to: buf];
	  node = next;
	}
      else
        {
	  node = tmp;
        }
    }
}

/**
 * Outputs zero or more nodes at the same level as long as the nodes
 * are valid %block elements.  Returns nil or the first node not output.
 * The value of flag is used to control paragraph nesting ... if YES
 * we close a paragraph before opening a new one, and open again once
 * the new paragraph closes.
 */
- (GSXMLNode*) outputBlock: (GSXMLNode*)node
			to: (NSMutableString*)buf
		    inPara: (BOOL)flag
{
  if (node != nil &&  [node type] == XML_ELEMENT_NODE)
    {
      GSXMLNode	*tmp = node;
      NSString	*n;

      node = [self outputList: node to: buf];
      if (node != tmp)
	{
	  return node;
	}
      n = [node name];
      if ([n isEqual: @"p"] == YES)
	{
	  if (flag == YES)
	    {
	      [self decIndent];
	      [buf appendString: indent];
	      [buf appendString: @"</p>\n"];
	    }
	  [buf appendString: indent];
	  [buf appendString: @"<p>\n"];
	  [self incIndent];
	  [self outputText: [node firstChild] to: buf];
	  [self decIndent];
	  [buf appendString: indent];
	  [buf appendString: @"</p>\n"];
	  if (flag == YES)
	    {
	      [buf appendString: indent];
	      [buf appendString: @"<p>\n"];
	      [self incIndent];
	    }
	  return [node next];
	}
      else if ([n isEqual: @"example"] == YES)
	{
	  GSXMLNode	*c = [node firstChild];

	  [buf appendString: @"<pre>"];
	  [self outputText: c to: buf];
	  [buf appendString: @"</pre>\n"];
	  return [node next];
	}
      else if ([n isEqual: @"embed"] == YES)
	{
	  NSLog(@"Element 'embed' not supported");
	  return [node next];
	}
      else if ([n isEqual: @"index"] == YES)
	{
	  [self outputNode: node to: buf];
	  return [node next];
	}
      else if ([textNodes member: n] != nil)
	{
	  [buf appendString: indent];
	  node = [self outputText: node to: buf];
	  [buf appendString: @"\n"];
	  return node;
	}
      else
	{
	  NSLog(@"Non-block element '%@' in block ...", n);
          NSLog(@"%@",node);
	  return nil;
	}
    }

  [buf appendString: indent];
  node = [self outputText: node to: buf];
  [buf appendString: @"\n"];
  return node;
}

/**
 * Outputs a node as long as it is a
 * valid %list element.  Returns next node at this level.
 */
- (GSXMLNode*) outputList: (GSXMLNode*)node to: (NSMutableString*)buf
{
  NSString	*name = [node name];
  GSXMLNode	*children = [node firstChildElement];

  if ([name isEqual: @"list"] == YES)
    {
      [buf appendString: indent];
      [buf appendString: @"<ul>\n"];
      [self incIndent];
      while (children != nil)
	{
	  GSXMLNode	*tmp = [children firstChild];

	  [buf appendString: indent];
	  [buf appendString: @"<li>\n"];
	  [self incIndent];
	  while (tmp != nil)
	    {
	      tmp = [self outputBlock: tmp to: buf inPara: NO];
	    }
	  [self decIndent];
	  [buf appendString: indent];
	  [buf appendString: @"</li>\n"];
	  children = [children nextElement];
	}
      [self decIndent];
      [buf appendString: indent];
      [buf appendString: @"</ul>\n"];
    }
  else if ([name isEqual: @"enum"] == YES)
    {
      [buf appendString: indent];
      [buf appendString: @"<ol>\n"];
      [self incIndent];
      while (children != nil)
	{
	  GSXMLNode	*tmp = [children firstChild];

	  [buf appendString: indent];
	  [buf appendString: @"<li>\n"];
	  [self incIndent];
	  while (tmp != nil)
	    {
	      tmp = [self outputBlock: tmp to: buf inPara: NO];
	    }
	  [self decIndent];
	  [buf appendString: indent];
	  [buf appendString: @"</li>\n"];
	  children = [children nextElement];
	}
      [self decIndent];
      [buf appendString: indent];
      [buf appendString: @"</ol>\n"];
    }
  else if ([name isEqual: @"deflist"] == YES)
    {
      [buf appendString: indent];
      [buf appendString: @"<dl>\n"];
      [self incIndent];
      while (children != nil)
	{
	  GSXMLNode	*tmp;

	  [buf appendString: indent];
	  [buf appendString: @"<dt>"];
	  [self outputText: [children firstChild] to: buf];
	  [buf appendString: @"</dt>\n"];
	  children = [children nextElement];
	  [buf appendString: indent];
	  [buf appendString: @"<dd>\n"];
	  [self incIndent];
	  tmp = [children firstChild];
	  while (tmp != nil)
	    {
	      tmp = [self outputBlock: tmp to: buf inPara: NO];
	    }
	  [self decIndent];
	  [buf appendString: indent];
	  [buf appendString: @"</dd>\n"];
	  children = [children nextElement];
	}
      [self decIndent];
      [buf appendString: indent];
      [buf appendString: @"</dl>\n"];
    }
  else if ([name isEqual: @"qalist"] == YES)
    {
      [buf appendString: indent];
      [buf appendString: @"<dl>\n"];
      [self incIndent];
      while (children != nil)
	{
	  GSXMLNode	*tmp;

	  [buf appendString: indent];
	  [buf appendString: @"<dt>"];
	  [self outputText: [children firstChild] to: buf];
	  [buf appendString: @"</dt>\n"];
	  children = [children nextElement];
	  [buf appendString: indent];
	  [buf appendString: @"<dd>\n"];
	  [self incIndent];
	  tmp = [children firstChild];
	  while (tmp != nil)
	    {
	      tmp = [self outputBlock: tmp to: buf inPara: NO];
	    }
	  [self decIndent];
	  [buf appendString: indent];
	  [buf appendString: @"</dd>\n"];
	  children = [children nextElement];
	}
      [self decIndent];
      [buf appendString: indent];
      [buf appendString: @"</dl>\n"];
    }
  else if ([name isEqual: @"dictionary"] == YES)
    {
      [buf appendString: indent];
      [buf appendString: @"<dl>{\n"];
      [self incIndent];
      // all children should be dictionaryItems w/key and value attributes;
      // if the value attribute is absent, the content is the value
      for (; children != nil; children = [children nextElement])
	{
	  GSXMLNode	*dItem = children;
          NSDictionary	*dProp = [dItem attributes];
          NSString	*value = [dProp objectForKey: @"value"];
          GSXMLNode	*dChild;

          if (![@"dictionaryItem" isEqualToString: [dItem name]])
            {
              continue;
            }
          [buf appendString: indent];
          [buf appendString: @"<dt>"];
          [buf appendString:
	    [[dProp objectForKey: @"key"] stringByEscapingXML]];
          [buf appendString: @" = </dt>\n"];
	  [buf appendString: indent];
          [buf appendString: @"<dd>\n"];
          [self incIndent];
          if (value != nil)
            {
              [buf appendString: [value stringByEscapingXML]];
            }
          else
            {
              dChild = [dItem firstChildElement];
              if ( dChild == nil )
                {
                  // no elements, just text contents
                  dChild = [dItem firstChild];
                  [buf appendString: indent];
                }
              [self outputBlock: dChild to: buf inPara: NO];
              //PENDING use returne value  for dItem?
            }
          [buf appendString: @"\n"];
          [self decIndent];
          [buf appendString: indent];
          [buf appendString: @";</dd>\n"];
        }
      [self decIndent];
      [buf appendString: indent];
      [buf appendString: @"</dl>}\n"];
    }
  else
    {
      return node;	// Not a list
    }
  node = [node next];
  return node;
}

/**
 * Outputs zero or more nodes at the same level as long as the nodes
 * are valid %text elements.  Returns nil or the first node not output.
 */
- (GSXMLNode*) outputText: (GSXMLNode*)node to: (NSMutableString*)buf
{
  while (node != nil)
    {
      if ([node type] == XML_TEXT_NODE)
	{
	  NSString	*str = [node escapedContent];

	  if (str == nil) str = @"";
	  [buf appendString: str];
	}
      else if ([node type] == XML_ENTITY_REF_NODE)
	{
	  [buf appendString: @"&"];
	  [buf appendString: [node name]];
	  [buf appendString: @";"];
	}
      else if ([node type] == XML_ELEMENT_NODE)
	{
	  NSString	*name = [node name];

	  if ([textNodes member: name] != nil)
	    {
	      [self outputNode: node to: buf];
	    }
	  else
	    {
	      return node;	// Not a text node.
	    }
	}
      node = [node next];
    }
  return node;
}

- (void) outputUnit: (GSXMLNode*)node to: (NSMutableString*)buf
{
  NSArray	*a;
  NSMutableString *ivarBuf = ivarsAtEnd ?
    (id)[NSMutableString stringWithCapacity: 1024] : nil;
  NSDictionary	*prop = [node attributes];

  node = [node firstChildElement];
  if (node != nil && [[node name] isEqual: @"declared"] == YES)
    {
      [self outputNode: node to: buf];
      node = [node nextElement];
    }

  if (node != nil && [[node name] isEqual: @"conform"] == YES)
    {
      [buf appendString: indent];
      [buf appendString: @"<blockquote>\n"];
      [self incIndent];
      [buf appendString: indent];
      [buf appendString: @"<dl>\n"];
      [self incIndent];
      [buf appendString: indent];
      [buf appendString: @"<dt><b>Conforms to:</b></dt>\n"];
      while (node != nil && [[node name] isEqual: @"conform"] == YES)
	{
	  NSString	*text = [[node firstChild] escapedContent];

	  if (text == nil) text = @"";
	  [buf appendString: indent];
	  [buf appendString: @"<dd>"];
	  [buf appendString: [self protocolRef: text]];
	  [buf appendString: @"</dd>\n"];
	  node = [node nextElement];
	}
      [self decIndent];
      [buf appendString: indent];
      [buf appendString: @"</dl>\n"];
      [self decIndent];
      [buf appendString: indent];
      [buf appendString: @"</blockquote>\n"];
    }

  while (node != nil && [[node name] isEqual: @"desc"] == NO)
    {
      node = [node nextElement];
    }
  [self outputVersion: prop to: buf];

  if (node != nil && [[node name] isEqual: @"desc"] == YES)
    {
      [self outputNode: node to: buf];
      node = [node nextElement];
    }

  if (node != nil && [[node name] isEqual: @"ivariable"] == YES)
    {
      NSMutableString	*ibuf = buf;

      /*
       * If want instance variables at end, throw it all into an alternate
       * buffer and just put a link here; later alt buf must be appended.
       */
      if (ivarsAtEnd)
	{
	  ibuf = ivarBuf;
	  [buf appendString: indent];
	  [buf appendString: @"<hr width=\"50%\" align=\"left\" />\n"];
	  [buf appendString: indent];
	  [buf appendFormat: @"<a href=\"#_%@_ivars\">Instance Variables</a>\n",
			     classname];
	  [buf appendString: indent];
	  [buf appendString: @"<br/><br/>\n"];
	  [ibuf appendFormat: @"<a name=\"_%@_ivars\"/>", classname];
	}
      [ibuf appendString: indent];
      [ibuf appendString: @"<br/><hr width=\"50%\" align=\"left\" />\n"];
      [ibuf appendString: indent];
      [ibuf appendFormat: @"<h2>Instance Variables for %@ Class</h2>\n",
	classname];
      while (node != nil && [[node name] isEqual: @"ivariable"] == YES)
	{
	  [self outputNode: node to: ibuf];
	  node = [node nextElement];
	}
      [ibuf appendString: indent];
      [ibuf appendString: @"<br/><hr width=\"50%\" align=\"left\" /><br/>\n"];
    }

  a = [localRefs methodsInUnit: unit];
  if ([a count] > 0)
    {
      [self outputIndex: @"method"
		  scope: @"global"
		  title: @"Method summary"
		  style: nil
                 target: nil
		     to: buf];
      [buf appendString: indent];
      [buf appendString: @"<hr width=\"50%\" align=\"left\" />\n"];
      while (node != nil)
	{
	  if ([[node name] isEqual: @"method"] == YES)
	    {
	      [self outputNode: node to: buf];
	    }
	  node = [node nextElement];
	}
    }

  // if had ivars docs, insert them now
  if (ivarsAtEnd)
    {
      [buf appendString: ivarBuf];
    }
}

- (void) outputVersion: (NSDictionary*)prop to: (NSMutableString*)buf
{
  NSString	*ovadd = [prop objectForKey: @"ovadd"];
  NSString	*gvadd = [prop objectForKey: @"gvadd"];
  NSString	*ovdep = [prop objectForKey: @"ovdep"];
  NSString	*gvdep = [prop objectForKey: @"gvdep"];
  NSString	*ovrem = [prop objectForKey: @"ovrem"];
  NSString	*gvrem = [prop objectForKey: @"gvrem"];
  const char	*str;
  int		maj;
  int		min;
  int		sub;

  if ([ovadd length] > 0)
    {
      int	add;
      int	dep;
      int	rem;

      str = [ovadd UTF8String];
      if (str != 0 && sscanf(str, "%d.%d.%d", &maj, &min, &sub) == 3)
	add = maj * 10000 + min * 100 + sub;
      else
	add = 0;

      str = [ovdep UTF8String];
      if (str != 0 && sscanf(str, "%d.%d.%d", &maj, &min, &sub) == 3)
	dep = maj * 10000 + min * 100 + sub;
      else
	dep = 0;

      str = [ovrem UTF8String];
      if (str != 0 && sscanf(str, "%d.%d.%d", &maj, &min, &sub) == 3)
	rem = maj * 10000 + min * 100 + sub;
      else
	rem = 0;

      [buf appendString: indent];
      [buf appendString: @"<div class=\"availability\">\n"];
      [buf appendString: @"<b>Availability:</b> "];
      if (add < GS_API_OSSPEC)
	{
	  [buf appendString: @"Not in OpenStep/MacOS-X"];
	}
      else if (add < GS_API_OPENSTEP)
	{
	  [buf appendString: @"OpenStep"];
	}
      else if (add < GS_API_MACOSX)
	{
	  [buf appendString: @"OPENSTEP "];
	  [buf appendString: ovadd];
	}
      else
	{
	  [buf appendString: @"MacOS-X "];
	  [buf appendString: ovadd];
	}
      if (dep > add)
	{
	  [buf appendString: @" deprecated at "];
	  if (dep < GS_API_MACOSX)
	    {
	      [buf appendString: @"OPENSTEP "];
	      [buf appendString: ovdep];
	    }
	  else
	    {
	      [buf appendString: @"MacOS-X "];
	      [buf appendString: ovdep];
	    }
	}
      if (rem > add)
	{
	  [buf appendString: @" removed at "];
	  if (rem < GS_API_MACOSX)
	    {
	      [buf appendString: @"OPENSTEP "];
	      [buf appendString: ovrem];
	    }
	  else
	    {
	      [buf appendString: @"MacOS-X "];
	      [buf appendString: ovrem];
	    }
	}
      if ([gvadd length] > 0)
	{
	  [buf appendString: @", "];
	  [buf appendString: project];
	  if ([gvadd isEqualToString: @"0.0.0"] == NO)
	    {
	      [buf appendString: @" "];
	      [buf appendString: gvadd];
	    }
	  if ([gvdep length] > 0)
	    {
	      [buf appendString: @" deprecated at "];
	      [buf appendString: gvdep];
	    }
	  if ([gvrem length] > 0)
	    {
	      [buf appendString: @" Likely to be changed/moved/removed at "];
	      [buf appendString: gvrem];
	    }
	}
      [buf appendString:@"</div>\n"];
      [buf appendString: @"<br />\n"];
    }
  else if ([gvadd length] > 0)
    {
      [buf appendString: indent];
      [buf appendString: @"<div class=\"availability\">\n"];
      [buf appendString: @"<b>Availability:</b> "];
      [buf appendString: project];
      if ([gvadd isEqualToString: @"0.0.0"] == NO)
	{
	  [buf appendString: @" "];
	  [buf appendString: gvadd];
	}
      if ([gvdep length] > 0)
	{
	  [buf appendString: @" deprecated at "];
	  [buf appendString: gvdep];
	}
      [buf appendString: @"<br />\n"];
      if ([gvrem length] > 0)
	{
          [buf appendString: @" Likely to be changed/moved/removed at "];
	  [buf appendString: gvrem];
	}
      [buf appendString:@"</div>\n"];
      [buf appendString: @"<br />\n"];
    }
}

/**
 * Try to make a link to the documentation for the supplied protocol.
 */
- (NSString*) protocolRef: (NSString*)t
{
  NSString	*n;
  NSString	*s;

  t = [t stringByTrimmingSpaces];
  n = [NSString stringWithFormat: @"(%@)", t];
  s = [self makeLink: n ofType: @"protocol" isRef: YES];
  if (s != nil)
    {
      s = [s stringByAppendingString: t];
      t = [s stringByAppendingString: @"</a>"];
    }
  return t;
}

- (void) setGlobalRefs: (AGSIndex*)r
{
  ASSIGN(globalRefs, r);
}

- (void) setLocalRefs: (AGSIndex*)r
{
  ASSIGN(localRefs, r);
}

- (void) setProjectRefs: (AGSIndex*)r
{
  ASSIGN(projectRefs, r);
}

- (void) setInstanceVariablesAtEnd: (BOOL)val
{
  ivarsAtEnd = val;
}

/**
 * Assuming that the supplied string contains type information (as used
 * in a method declaration or type cast), we make an attempt at extracting
 * the basic type, and seeing if we can find a documented declaration for
 * it.  If we can, we return a modified version of the string containing
 * a link to the appropriate documentation.  Otherwise, we just return the
 * plain type string.  In all cases, we strip leading and trailing white space.
 */
- (NSString*) typeRef: (NSString*)t
{
  NSString	*str = [t stringByTrimmingSpaces];
  NSString	*s;
  unsigned	end = [str length];
  unsigned	start;
  NSMutableString	*ms = nil;
  NSRange		er;
  NSRange		sr;

  if (end == 0)
    {
      return nil;
    }
  t = str;
  while (end > 0)
    {
      unichar	c = [t characterAtIndex: end-1];

      if (c != '*' && !isspace(c))
	{
	  break;
	}
      end--;
    }
  start = end;
  while (start > 0)
    {
      unichar	c = [t characterAtIndex: start-1];

      if (c != '_' && !isalnum(c))
	{
	  break;
	}
      start--;
    }
  t = [str substringWithRange: NSMakeRange(start, end - start)];

  s = [self makeLink: t ofType: @"type" isRef: YES];
  if (s == nil)
    {
      s = [self makeLink: t ofType: @"class" isRef: YES];
    }

  s = [s stringByAppendingFormat: @"%@</a>", t];
  if (s != nil && [str length] == [t length])
    {
      return s;
    }

  /*
   * Look for protocol spec.
   */
  sr = [str rangeOfString: @"<"];
  if (sr.length == 0)
    {
      sr = [str rangeOfString: @"&lt;"];
    }
  if (sr.length == 0)
    {
      sr = [str rangeOfString: @"&#60;"];
    }
  er = [str rangeOfString: @">"];
  if (er.length == 0)
    {
      er = [str rangeOfString: @"&gt;"];
    }
  if (er.length == 0)
    {
      er = [str rangeOfString: @"&#62;"];
    }

  /*
   * Substitute in protocol references.
   */
  if (sr.length > 0 && er.length > 0 && er.location > sr.location)
    {
      NSString	*pString;
      NSRange	r;
      NSArray	*protocols;
      unsigned	i;

      r = NSMakeRange(NSMaxRange(sr), er.location - NSMaxRange(sr));
      pString = [str substringWithRange: r];
      protocols = [pString componentsSeparatedByString: @","];
      ms = [str mutableCopy];
      pString = @"";
      for (i = 0; i < [protocols count]; i++)
	{
	  NSString	*p = [protocols objectAtIndex: i];
	  NSString	*l;

	  l = [self makeLink: [NSString stringWithFormat: @"(%@)", p]
		      ofType: @"protocol"
		       isRef: YES];
	  if (l != nil)
	    {
	      p = [l stringByAppendingFormat: @"%@</a>", p];
	    }
	  if (i > 0)
	    {
	      pString = [pString stringByAppendingString: @","];
	    }
	  pString = [pString stringByAppendingString: p];
	}
      [ms replaceCharactersInRange: r withString: pString];
    }


  /*
   * Substitute in basic type reference.
   */
  if (s != nil)
    {
      if (ms == nil)
	{
	  ms = [str mutableCopy];
	}
      [ms replaceCharactersInRange: NSMakeRange(start, end - start)
			withString: s];
    }
  if (ms != nil)
    {
      str = AUTORELEASE(ms);
    }
  return str;
}

@end

