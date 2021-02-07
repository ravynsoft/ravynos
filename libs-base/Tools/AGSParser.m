/*
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
#import "Foundation/NSException.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSUserDefaults.h"
#import "Foundation/NSScanner.h"
#import "Foundation/NSSet.h"
#import "Foundation/NSValue.h"
#import "AGSParser.h"
#import "GNUstepBase/NSString+GNUstepBase.h"
#import "GNUstepBase/NSMutableString+GNUstepBase.h"

/**
 *  The AGSParser class parses Objective-C header and source files
 *  to produce a property-list which can be handled by [AGSOutput].
 */
@implementation	AGSParser

/**
 * Method to add the comment from the main() function to the end
 * of the initial chapter in the output document.  We do this to
 * support the use of autogsdoc to document tools.
 */
- (void) addMain: (NSString*)c
{
  NSString		*chap;
  NSString		*toolName;
  NSString		*secHeading;
  BOOL			createSec = NO;
  NSMutableString	*m;
  NSRange		r;

  chap = [info objectForKey: @"chapter"];
  toolName = [[fileName lastPathComponent] stringByDeletingPathExtension];
  if (chap == nil)
    {
      chap = [NSString stringWithFormat:
        @"<chapter id=\"_main\"><heading>%@</heading></chapter>", toolName];
    }
  else
    {
      createSec = YES;
    }
  m = [chap mutableCopy];
  r = [m rangeOfString: @"</chapter>"];
  r.length = 0;
  if (createSec)
    {
      [m replaceCharactersInRange: r withString: @"</section>\n"];
    }
  [m replaceCharactersInRange: r withString: c];
  if (createSec)
    {
      secHeading = [NSString stringWithFormat:
        @"<section id=\"_main\">\n<heading>%@</heading>\n", toolName];
  //The %@ tool
      [m replaceCharactersInRange: r withString: secHeading];
    }
  [info setObject: m forKey: @"chapter"];
  RELEASE(m);
}

/**
 * Append a comment (with leading and trailing space stripped)
 * to an information dictionary.<br />
 * If the dictionary is nil, accumulate in the comment ivar instead.<br />
 * If the comment is empty, ignore it.<br />
 * If there is no comment in the dictionary, simply set the new value.<br />
 * If a comment already exists then the new comment text is appended to
 * it with a separating line break inserted if necessary.<br />
 */
- (void) appendComment: (NSString*)s to: (NSMutableDictionary*)d
{
  s = [s stringByTrimmingSpaces];
  if ([s length] > 0)
    {
      NSString	*old;

      if (d == nil)
        {
	  old = comment;
	}
      else
        {
	  old = [d objectForKey: @"Comment"];
	}
      if (old != nil)
        {
	  if ([old hasSuffix: @"</p>"] == NO
	    && [old hasSuffix: @"<br />"] == NO
	    && [old hasSuffix: @"<br/>"] == NO)
	    {
	      s = [old stringByAppendingFormat: @"<br />%@", s];
	    }
	  else
	    {
	      s = [old stringByAppendingString: s];
	    }
	}
      if (d == nil)
        {
	  ASSIGN(comment, s);
	}
      else
	{
	  [d setObject: s forKey: @"Comment"];
	}
    }
}

- (void) dealloc
{
  DESTROY(wordMap);
  DESTROY(ifStack);
  DESTROY(declared);
  DESTROY(info);
  DESTROY(orderedSymbolDeclsByUnit);
  DESTROY(comment);
  DESTROY(identifier);
  DESTROY(identStart);
  DESTROY(spaces);
  DESTROY(spacenl);
  DESTROY(source);
  DESTROY(itemName);
  DESTROY(unitName);
  [super dealloc];
}

- (NSMutableDictionary*) info
{
  return info;
}

/** Returns the methods, functions and C data types in their header declaration 
order, by organizing them into arrays as described below. 

Methods are grouped by class, category or protocol references. For example, 
valid keys could be <em>ClassName</em>, <em>ClassName(CategoryName)</em> and 
<em>(ProtocolName)</em>.

Functions and C data types are grouped by header file names. For example, 
<em>AGParser.h</em> would a valid key.

TODO: Collect functions and C data types. Only methods are currently included 
in the returned dictionary. */
- (NSDictionary *) orderedSymbolDeclarationsByUnit
{
  return orderedSymbolDeclsByUnit;
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
  info = [[NSMutableDictionary alloc] initWithCapacity: 6];
  orderedSymbolDeclsByUnit = [[NSMutableDictionary alloc] init];
  source = [NSMutableArray new];
  verbose = [[NSUserDefaults standardUserDefaults] boolForKey: @"Verbose"];
  warn = [[NSUserDefaults standardUserDefaults] boolForKey: @"Warn"];
  documentInstanceVariables = YES;
  ifStack = [[NSMutableArray alloc] initWithCapacity: 4];
  [ifStack addObject: [NSDictionary dictionary]];
  return self;
}

- (void) log: (NSString*)fmt arguments: (va_list)args
{
  const char	*msg;
  int		where;

  /*
   * Take the current position in the character buffer and
   * step through the lines array to find which line of the
   * original document it was on.
   * NB. Each item in the array represents the position *after*
   * a newline in the original data - so the zero'th array
   * element contains the character position of the start of
   * line two in human readable numbering (ie starting from 1).
   */
  for (where = [lines count] - 1; where >= 0; where--)
    {
      NSNumber	*num = [lines objectAtIndex: where];

      if ([num intValue] <= (int)pos)
	{
	  break;
	}
    }
  where += 2;

  if (unitName != nil)
    {
      if (itemName != nil)
	{
          fmt = [NSString stringWithFormat: @"%@:%u %@(%@): %@",
	    fileName, where, unitName, itemName, fmt];
	}
      else
	{
          fmt = [NSString stringWithFormat: @"%@:%u %@: %@",
	    fileName, where, unitName, fmt];
	}
    }
  else
    {
      fmt = [NSString stringWithFormat: @"%@:%u %@", fileName, where, fmt];
    }
  fmt = AUTORELEASE([[NSString alloc] initWithFormat: fmt arguments: args]);
  if ([fmt hasSuffix: @"\n"] == NO)
    {
      fmt = [fmt stringByAppendingString: @"\n"];
    }
  msg = [fmt lossyCString];
  fwrite(msg, strlen(msg), 1, stderr);
}

- (void) log: (NSString*)fmt, ...
{
  va_list ap;

  va_start (ap, fmt);
  [self log: fmt arguments: ap];
  va_end (ap);
}

- (void) parseArgsInto: (NSMutableDictionary*)d
{
  BOOL			wasInArgList = inArgList;
  NSMutableArray	*a = nil;

  NSAssert([d objectForKey: @"Args"] == nil, NSInternalInconsistencyException);
  a = [[NSMutableArray alloc] initWithCapacity: 4];
  [d setObject: a forKey: @"Args"];
  RELEASE(a);

  inArgList = YES;
  pos++;	// Step past opening '('

  while ([self parseSpace] < length && buffer[pos] != ')')
    {
      if (buffer[pos] == ',')
	{
	  pos++;
	}
      else if (buffer[pos] == '.')
	{
	  pos += 3;	// Skip '...'
	  [d setObject: @"YES" forKey: @"VarArgs"];
	}
      else
	{
	  NSMutableDictionary	*m;

	  m = [self parseDeclaration];
	  if (m == nil)
	    {
	      break;
	    }
	  if ([[m objectForKey: @"BaseType"] isEqual: @"void"] == YES
	     && [m objectForKey: @"Prefix"] == nil)
	    {
	      continue;	// C++ style empty arg list. eg. 'int foo(void);'
	    }
	  [a addObject: m];
	}
    }
  if (pos < length)
    {
      pos++;	// Step past closing ')'
    }
  inArgList = wasInArgList;
}

/**
 * Return the list of known output files depending on this source/header.
 */
- (NSMutableArray*) outputs
{
  NSUserDefaults	*defs = [NSUserDefaults standardUserDefaults];
  NSMutableArray	*output = [NSMutableArray arrayWithCapacity: 6];
  NSString		*basic = [info objectForKey: @"Header"];
  NSString		*names[5] = { @"Functions", @"Typedefs", @"Variables",
    @"Macros", @"Constants" };
  unsigned		i;

  basic = [basic lastPathComponent];
  basic = [basic stringByDeletingPathExtension];
  basic = [basic stringByAppendingPathExtension: @"gsdoc"];

  /**
   * If there are any classes, categories, or protocols, there will be
   * an output file for them whose name is based on the name of the header.
   */
  if ([[info objectForKey: @"Classes"] count] > 0
    || [[info objectForKey: @"Categories"] count] > 0
    || [[info objectForKey: @"Protocols"] count] > 0)
    {
      [output addObject: basic];
    }

  /**
   * If there are any constants, variables, typedefs or functions, there
   * will either be a shared output file for them (defined by a template
   * name set in the user defaults system), or they will go in the same
   * file as classes etc.
   */
  for (i = 0; i < sizeof(names) / sizeof(NSString*); i++)
    {
      NSString		*base = names[i];

      if ([[info objectForKey: base] count] > 0)
	{
	  NSString	*file;

	  base = [base stringByAppendingString: @"Template"];
	  file = [defs stringForKey: base];
	  if ([file length] == 0)
	    {
	      if ([output containsObject: basic] == NO)
		{
		  [output addObject: basic];
		}
	    }
	  else
	    {
	      if ([[file pathExtension] isEqual: @"gsdoc"] == NO)
		{
		  file = [file stringByAppendingPathExtension: @"gsdoc"];
		}
	      if ([output containsObject: file] == NO)
		{
		  [output addObject: file];
		}
	    }
	}
    }

  return output;
}

/* When the paragraph string contains a GSDoc block element which is not a text 
element (in the GSDoc DTD sense), we return NO, otherwise we return YES. 

A GSDoc or HTML paragraph content is limited to text elements (see GSDoc DTD).
e.g. 'list' or 'example' cannot belong to a 'p' element.

Any other non-block elements are considered valid. Whether or not they can be 
embedded within a paragraph in the final output is the doc writer 
responsability.
 
For 'item' and 'answer' which can contain arbitrary block elements, explicit 
'p' tags should be used, because we won't wrap 'patata' and 'patati' as two 
paragraphs in the example below:
<list>
<item>patata

patati</item>
</list>

When <example> starts a paragraph, \n\n sequence are allowed in the example. 
In the example below, bla<example> or bla\n<example> wouldn't be handled 
correctly unlike: 
bla

<example>
patati

patata
</example> */
- (BOOL) canWrapWithParagraphMarkup: (NSString *)para
{
  NSScanner *scanner = [NSScanner scannerWithString: para];
  NSSet *blockTags = [NSSet setWithObjects: @"list", @"enum", @"item", 
    @"deflist", @"term", @"qalist", @"question", @"answer", 
    @"p", @"example", @"embed", @"index", nil]; 
  NSMutableCharacterSet *skippedChars = 
    (id)[NSMutableCharacterSet punctuationCharacterSet];

  if (inUnclosedExample)
    {
        /* We don't need to check block element presence within an example, 
           since an example content is limited to PCDATA. */
        [scanner scanUpToString: @"</example>" intoString: NULL];
        if ([scanner scanString: @"</example>" intoString: NULL])
          {
            inUnclosedExample = NO;
          }
          return NO;
    }

  /* Set up the scanner to treat opening and closing tags in the same way.
     Punctuation character set includes '/' but not '<' and '>' */
  [skippedChars formUnionWithCharacterSet: [scanner charactersToBeSkipped]];
  [scanner setCharactersToBeSkipped: AUTORELEASE([skippedChars copy])];

  while (![scanner isAtEnd])
    {
      NSString *tag = @"";
      BOOL foundBlockTag = NO;
   
      [scanner scanUpToString: @"<" intoString: NULL];
      if (![scanner scanString: @"<" intoString: NULL])
        return YES;

      [scanner scanUpToString: @">" intoString: &tag];
      foundBlockTag = [blockTags containsObject: tag];

      if (foundBlockTag)
      {
        /* When the first block tag is <example> and the example is unclosed in 
           the current paragraph, we stop to insert <p> tags in the next 
           paragraphs until we reach </example> */
        if ([tag isEqualToString: @"example"])
          {
            [scanner setCharactersToBeSkipped:
              [NSCharacterSet whitespaceAndNewlineCharacterSet]];
            [scanner scanUpToString: @"</example>" intoString: NULL];
            inUnclosedExample
              = ([scanner scanString: @"</example>" intoString: NULL] == NO);
          }

        return NO;
      }
    }

   return YES;
}

// NOTE: We could be able to eliminate that if -parseComment processes the 
// first comment tags before calling -generateParagraphMarkups:
- (BOOL) shouldInsertParagraphMarkupInFirstComment: (NSString *)aComment
{
  NSArray *firstCommentTags = [NSArray arrayWithObjects: @"<title>", 
    @"<abstract>", @"<author>", @"<copy>", @"<version>", @"<date>", 
    @"Author:", @"By:", @"Copyright (C)", @"Revision:", @"Date:", nil];
   NSEnumerator *e = [firstCommentTags objectEnumerator];
   NSString *tag = nil;

   while ((tag = [e nextObject]) != nil)
     {
       if ([aComment rangeOfString: tag 
                           options: NSCaseInsensitiveSearch].location != NSNotFound)
         {
            return NO;
         }
     }

  return YES;
}

- (NSString *) generateParagraphMarkupForString: (NSString *)aComment
{
  NSMutableString *formattedComment = [NSMutableString stringWithCapacity: [aComment length] + 100];
  NSArray *paras = [aComment componentsSeparatedByString: @"\n\n"];
  NSEnumerator *e = [paras objectEnumerator];
  NSString *para = nil;
  BOOL isFirstComment = (commentsRead == NO);

  if (isFirstComment 
   && ![self shouldInsertParagraphMarkupInFirstComment: aComment])
    {
      return aComment;
    }

  while ((para = [e nextObject]) != nil)
    {
      NSString *newPara = para;
      /* -canWrapWithParagraph: can change its value */
      BOOL wasInUnclosedExample = inUnclosedExample;

      if ([self canWrapWithParagraphMarkup: para])
      {
        newPara = [NSString stringWithFormat: @"<p>%@</p>", para];
      }
      else if (wasInUnclosedExample)
      {
        newPara = [NSString stringWithFormat: @"\n\n%@", para];
      }
      [formattedComment appendString: newPara];
    }

  return formattedComment;
}

/**
 * In spite of its trivial name, this is one of the key methods -
 * it parses and skips past comments, but it also recognizes special
 * comments (with an additional asterisk after the start of the block
 * comment) and extracts their contents, accumulating them into the
 * 'comment' instance variable.<br />
 * When the data provided by a comment is appended to the data
 * stored in the 'comment' instance variable, a line break (&lt;br /&gt;)is
 * automatically forced to separate it from the proceding info.<br />
 * In addition, the first extracted documentation is checked for the
 * prsence of file header markup, which is extracted into the 'info'
 * dictionary.
 */
- (unsigned) parseComment
{
  if (buffer[pos + 1] == '/')
    {
      return [self skipRemainderOfLine];
    }
  else if (buffer[pos + 1] == '*')
    {
      unichar	*start = 0;
      BOOL	isDocumentation = NO;
      BOOL	skippedFirstLine = NO;
      NSRange	r;
      BOOL	ignore = NO;


      /* Jump back here if we have ignored data up to a new comment.
       */
comment:

      pos += 2;	/* Skip opening part */

      /*
       * Only comments starting with slash and TWO asterisks are special.
       */
      if (pos < length - 2 && buffer[pos] == '*' && buffer[pos + 1] != '*')
	{
	  isDocumentation = YES;
	  pos++;

	  /*
	   * Ignore first line of comment if it is empty.
	   */
	  if ([self skipSpaces] < length && buffer[pos] == '\n')
	    {
	      pos++;
	      skippedFirstLine = YES;
	    }
	}

      /*
       * Find end of comment.
       */
      start = &buffer[pos];
      while (pos < length)
	{
	  unichar	c = buffer[pos++];

	  if (c == '*' && pos < length && buffer[pos] == '/')
	    {
	      pos++;	// Position after trailing slash.
	      break;
	    }
	}

      if (isDocumentation == YES)
	{
	  unichar	*end = &buffer[pos - 1];
	  unichar	*ptr = start;
	  unichar	*newLine = ptr;
	  BOOL		stripAsterisks = NO;

	  /*
	   * Remove any asterisks immediately before end of comment.
	   */
	  while (end > start && end[-1] == '*')
	    {
	      end--;
	    }
	  /*
	   * Remove any trailing whitespace in the comment, but ensure that
	   * there is a final newline.
	   */
	  while (end > start && [spacenl characterIsMember: end[-1]] == YES)
	    {
	      end--;
	    }
	  *end++ = '\n';

	  /*
	   * If second line in the comment starts with whitespace followed
	   * by an asterisk, we assume all the lines in the comment start
	   * in a similar way, and everything up to and including the
	   * asterisk on each line should be stripped.
	   * Otherwise we take the comment verbatim.
	   */
	  if (skippedFirstLine == NO)
	    {
	      while (ptr < end && *ptr != '\n')
		{
		  ptr++;
		}
	      ptr++;	// Step past the end of the first line.
	    }
	  while (ptr < end)
	    {
	      unichar	c = *ptr++;

	      if (c == '\n')
		{
		  break;
		}
	      else if (c == '*')
		{
		  stripAsterisks = YES;
		  break;
		}
	      else if ([spaces characterIsMember: c] == NO)
		{
		  break;
		}
	    }

	  if (stripAsterisks == YES)
	    {
	      /*
	       * Strip parts of lines up to leading asterisks.
	       */
	      ptr = start;
	      while (ptr < end)
		{
		  unichar	c = *ptr++;

		  if (c == '\n')
		    {
		      newLine = ptr;
		    }
		  else if (c == '*' && newLine != 0)
		    {
		      unichar	*out = newLine;

		      while (ptr < end)
			{
			  *out++ = *ptr++;
			}
		      end = out;
		      ptr = newLine;
		      newLine = 0;
		    }
		  else if ([spaces characterIsMember: c] == NO)
		    {
		      newLine = 0;
		    }
		}
	    }

	  /*
	   * If we have something for documentation, accumulate it in the
	   * 'comment' ivar.
	   */
	  if (end > start)
	    {
	      NSString 		*tmp;
	      NSRange		r;
              NSUserDefaults	*defs = [NSUserDefaults standardUserDefaults];

	      tmp = [NSString stringWithCharacters: start length: end - start];

              /* 
               * If the comment does not contain block markup already and we 
               * were asked to generate it, we insert <p> tags to get an 
               * explicit paragraph structure.
               */
              if ([defs boolForKey: @"GenerateParagraphMarkup"])
                {
                  // FIXME: Should follow <ignore> processing and be called 
                  // just before using -appendComment:to:
                  tmp = [self generateParagraphMarkupForString: tmp]; 
                }
recheck:
	      if (YES == ignore)
		{
	          r = [tmp rangeOfString: @"</ignore>"];
		  if (r.length > 0)
		    {
		      tmp = [tmp substringFromIndex: NSMaxRange(r)];
		      ignore = NO;
		    }
		}
	      if (NO == ignore)
		{
	          r = [tmp rangeOfString: @"<ignore>"];
		  if (r.length > 0)
		    {
		      [self appendComment: [tmp substringToIndex: r.location]
				       to: nil];
		      tmp = [tmp substringFromIndex: NSMaxRange(r)];
		      ignore = YES;
		      goto recheck;
		    }
		  [self appendComment: tmp to: nil];
		}
	    }

          /*
           * We're in the first comment of a file; perform special processing.
           */
	  if (commentsRead == NO && comment != nil)
	    {
	      unsigned		commentLength = [comment length];
	      NSMutableArray	*authors;
	      NSEnumerator	*enumerator;
	      NSArray		*keys;
	      NSString		*key;

	      authors = (NSMutableArray*)[info objectForKey: @"authors"];
	      /*
	       * Scan through for more authors
	       */
	      r = NSMakeRange(0, commentLength);
	      while (r.length > 0)
		{
		  r = [comment rangeOfString: @"<author "
				     options: NSLiteralSearch
				       range: r];
		  if (r.length > 0)
		    {
		      unsigned	i = r.location;

		      r = NSMakeRange(i, commentLength - i);
		      r = [comment rangeOfString: @"</author>"
					 options: NSLiteralSearch
					   range: r];
		      if (r.length > 0)
			{
			  NSString		*author;

			  r = NSMakeRange(i, NSMaxRange(r) - i);
			  author = [comment substringWithRange: r];
			  i = NSMaxRange(r);
			  r = NSMakeRange(i, commentLength - i);
			  /*
			   * There may be more than one author
			   * of a document.
			   */
			  if (authors == nil)
			    {
			      authors = [NSMutableArray new];
			      [info setObject: authors forKey: @"authors"];
			      RELEASE(authors);
			    }
			  if ([authors containsObject: author] == NO)
			    {
			      [authors addObject: author];
			    }
			}
		      else
			{
			  [self log: @"unterminated <author> in comment"];
			}
		    }
		}
	      /*
	       * In addition to fully specified author elements in the
	       * comment, we look for lines of the formats -
	       * Author: name <email>
	       * Author: name
	       * By: name <email>
	       * By: name
	       */
	      r = NSMakeRange(0, commentLength);
	      while (r.length > 0)
		{
		  NSString	*term = @"\n";
		  NSRange	a;
		  NSRange	b;

		  /*
		   * Look for 'Author:' or 'By:' and use whichever we
		   * find first.
		   */
		  a = [comment rangeOfString: @"author:"
				     options: NSCaseInsensitiveSearch
				       range: r];
		  b = [comment rangeOfString: @"by:"
				     options: NSCaseInsensitiveSearch
				       range: r];
		  if (a.length > 0)
		    {
		      if (b.length > 0 && b.location < a.location)
			{
			  r = b;
			}
		      else
			{
			  r = a;
			  /*
			   * A line '$Author$' is an RCS tag and is
			   * terminated by the second dollar rather than
			   * by a newline.
			   */
			  if (r.location > 0
			    && [comment characterAtIndex: r.location-1] == '$')
			    {
			      term = @"$";
			    }
			}
		    }
		  else
		    {
		      r = b;
		    }

		  if (r.length > 0)
		    {
		      unsigned	i = NSMaxRange(r);
		      NSString	*line;
		      NSString	*author;

		      r = NSMakeRange(i, commentLength - i);
		      r = [comment rangeOfString: term
					 options: NSLiteralSearch
					   range: r];
		      if (r.length == 0)
			{
			  r.location = commentLength;
			}
		      r = NSMakeRange(i, NSMaxRange(r) - i);
		      line = [comment substringWithRange: r];
		      line = [line stringByTrimmingSpaces];
		      i = NSMaxRange(r);
		      r = [line rangeOfString: @"<"];
		      if (r.length > 0)
			{
			  NSString	*name;
			  NSString	*mail;

			  name = [line substringToIndex: r.location];
			  name = [name stringByTrimmingSpaces];
			  mail = [line substringFromIndex: r.location+1];
			  r = [mail rangeOfString: @">"];
			  if (r.length > 0)
			    {
			      mail = [mail substringToIndex: r.location];
			    }
			  author = [NSString stringWithFormat:
			    @"<author name=\"%@\"><email address=\"%@\">"
			    @"%@</email></author>", name, mail, mail];
			}
		      else
			{
			  author = [NSString stringWithFormat:
			    @"<author name=\"%@\"></author>", line];
			}
		      r = NSMakeRange(i, commentLength - i);
		      if (authors == nil)
			{
			  authors = [NSMutableArray new];
			  [info setObject: authors forKey: @"authors"];
			  RELEASE(authors);
			}
		      if ([authors containsObject: author] == NO)
			{
			  [authors addObject: author];
			}
		    }
		}

	      /*
	       * Lines of the form 'AutogsdocSource: ...' are used as the
	       * names of source files to provide documentation information.
	       * whitespace around a filename is stripped.
	       */
	      r = NSMakeRange(0, commentLength);
	      while (r.length > 0)
		{
		  /*
		   * Look for 'AutogsdocSource:' lines.
		   */
		  r = [comment rangeOfString: @"AutogsdocSource:"
				     options: NSCaseInsensitiveSearch
				       range: r];
		  if (r.length > 0)
		    {
		      unsigned	i = NSMaxRange(r);
		      NSString	*line;

		      r = NSMakeRange(i, commentLength - i);
		      r = [comment rangeOfString: @"\n"
					 options: NSLiteralSearch
					   range: r];
		      if (r.length == 0)
			{
			  r.location = commentLength;
			}
		      r = NSMakeRange(i, NSMaxRange(r) - i);
		      line = [comment substringWithRange: r];
		      line = [line stringByTrimmingSpaces];
		
		      if (haveSource == NO)
			{
			  haveSource = YES;
			  [source removeAllObjects]; // remove default.
			}
		      if ([line length] > 0
			&& [source containsObject: line] == NO)
			{
			  NSFileManager	*mgr;

			  /*
			   * See if the path given exists, and add it to
			   * the list of source files parsed for this
			   * header.
			   */
			  mgr = [NSFileManager defaultManager];
			  if ([line isAbsolutePath] == YES)
			    {
			      if ([mgr isReadableFileAtPath: line] == NO)
				{
				  [self log: @"AutogsdocSource: %@ not found!",
				    line];
				  line = nil;
				}
			    }
			  else
			    {
			      NSString	*p;

			      /*
			       * Try forming a path relative to the header.
			       */
			      p = [info objectForKey: @"Header"];
			      p = [p stringByDeletingLastPathComponent];
			      p = [p stringByAppendingPathComponent: line];
			      if ([mgr isReadableFileAtPath: p] == YES)
				{
				  line = p;
				}
			      else if ([mgr isReadableFileAtPath: line] == NO)
				{
				  NSUserDefaults	*defs;
				  NSString		*ddir;
				  NSString		*old = p;

				  defs = [NSUserDefaults standardUserDefaults];
				  ddir = [defs stringForKey:
				    @"DocumentationDirectory"];
				  if ([ddir length] > 0)
				    {
				      p = [ddir stringByAppendingPathComponent:
					line];
				      if ([mgr isReadableFileAtPath: p] == YES)
					{
					  line = p;
					}
				      else
					{
					  [self log: @"AutogsdocSource: %@ not "
					    @"found (tried %@ and %@ too)!",
					    line, old, p];
					  line = nil;
					}
				    }
				  else
				    {
				      [self log: @"AutogsdocSource: %@ not "
					@"found (tried %@ too)!",
					line, old];
				      line = nil;
				    }
				}
			    }
			  if (line != nil)
			    {
			      [source addObject: line];
			    }
			}
		      i = NSMaxRange(r);
		      r = NSMakeRange(i, commentLength - i);
		    }
		}

	      /**
	       * There are various sections we can extract from the
	       * document - at most one of each.
	       * If date and version are not supplied RCS Date and Revision
	       * tags will be extracted where available.
	       */
	      keys = [NSArray arrayWithObjects:
		@"abstract",	// Abstract for document head
		@"back",	// Appendix for document body
		@"chapter",	// Chapter at start of document
		@"copy",	// Copyright for document head
		@"date",	// date for document head
		@"front",	// Forward for document body
		@"title",	// Title for document head
		@"version",	// Version for document head
		nil];
	      enumerator = [keys objectEnumerator];

	      while ((key = [enumerator nextObject]) != nil)
		{
		  NSString	*s = [NSString stringWithFormat: @"<%@>", key];
		  NSString	*e = [NSString stringWithFormat: @"</%@>", key];
	
		  /*
		   * Read date information if available
		   */
		  r = [comment rangeOfString: s];
		  if (r.length > 0)
		    {
		      unsigned	i = r.location;

		      r = NSMakeRange(i, commentLength - i);
		      r = [comment rangeOfString: e
					 options: NSLiteralSearch
					   range: r];
		      if (r.length > 0)
			{
			  NSString	*val;

			  r = NSMakeRange(i, NSMaxRange(r) - i);
			  val = [comment substringWithRange: r];
			  [info setObject: val forKey: key];
			}
		      else
			{
			  [self log: @"unterminated %@ in comment", s];
			}
		    }
		}

	      /*
	       * If no <copy> ... </copy> then try Copyright:
	       */
	      if ([info objectForKey: @"copy"] == nil)
		{
		  r = NSMakeRange(0, commentLength);
		  while (r.length > 0)
		    {
		      /*
		       * Look for 'Copyright:'
		       */
		      r = [comment rangeOfString: @"copyright (c)"
					 options: NSCaseInsensitiveSearch
					   range: r];
		      if (r.length > 0)
			{
			  unsigned	i = NSMaxRange(r);
			  NSString	*line;

			  r = NSMakeRange(i, commentLength - i);
			  r = [comment rangeOfString: @"\n"
					     options: NSLiteralSearch
					       range: r];
			  if (r.length == 0)
			    {
			      r.location = commentLength;
			    }
			  r = NSMakeRange(i, NSMaxRange(r) - i);
			  line = [comment substringWithRange: r];
			  line = [line stringByTrimmingSpaces];
			  line = [NSString stringWithFormat:
			    @"<copy>%@</copy>", line];
			  [info setObject: line forKey: @"copy"];
			}
		    }
		}

	      /*
	       * If no <date> ... </date> then try RCS info.
	       */
	      if ([info objectForKey: @"date"] == nil)
		{
		  r = [comment rangeOfString: @"$Date:"];
		  if (r.length > 0)
		    {
		      unsigned	i = NSMaxRange(r);
		      NSString	*date;

		      r = NSMakeRange(i, commentLength - i);
		      r = [comment rangeOfString: @"$"
					 options: NSLiteralSearch
					   range: r];
		      if (r.length > 0)
			{
			  r = NSMakeRange(i, r.location - i);
			  date = [comment substringWithRange: r];
			  date = [date stringByTrimmingSpaces];
			  date = [NSString stringWithFormat:
			    @"<date>%@</date>", date];
			  [info setObject: date forKey: @"date"];
			}
		    }
		}

	      /*
	       * If no <version> ... </version> then try RCS info.
	       */
	      if ([info objectForKey: @"version"] == nil)
		{
		  r = [comment rangeOfString: @"$Revision:"];
		  if (r.length > 0)
		    {
		      unsigned	i = NSMaxRange(r);
		      NSString	*version;

		      r = NSMakeRange(i, commentLength - i);
		      r = [comment rangeOfString: @"$"
					 options: NSLiteralSearch
					   range: r];
		      if (r.length > 0)
			{
			  r = NSMakeRange(i, r.location - i);
			  version = [comment substringWithRange: r];
			  version = [version stringByTrimmingSpaces];
			  version = [NSString stringWithFormat:
			    @"<version>%@</version>", version];
			  [info setObject: version forKey: @"version"];
			}
		    }
		}
	    }
	  commentsRead = YES;
	}
      if (YES == ignore)
	{
	  while (pos < length)
	    {
	      switch (buffer[pos])
		{
		  case '\'':
		  case '"':
		    [self skipLiteral];
		    break;

		  case '/':
		    if (pos + 1 < length)
		      {
			if (buffer[pos + 1] == '/')
			  {
			    [self skipRemainderOfLine];
			  }
			else if (buffer[pos + 1] == '*')
			  {
			    goto comment;
			  }
		      }
		    pos++;
		    break;

		  default:
		    pos++;
		    break;
		}
	    }
	}
    }
  return pos;
}

- (void) parseDeclaratorInto: (NSMutableDictionary*)d
{
  NSMutableString	*p = nil;
  NSMutableString	*s = nil;

  while ([self parseSpace] < length)
    {
      while (pos < length && buffer[pos] == '*')
	{
	  if (p == nil && (p = [d objectForKey: @"Prefix"]) == nil)
	    {
	      p = [NSMutableString new];
	      [d setObject: p forKey: @"Prefix"];
	      RELEASE(p);
	    }
	  else if ([p hasSuffix: @"("] == NO && [p hasSuffix: @"*"] == NO)
	    {
	      [p appendString: @" "];
	    }
	  [p appendString: @"*"];
	  pos++;
	}
      if (buffer[pos] == '(')
	{
	  if (p == nil && (p = [d objectForKey: @"Prefix"]) == nil)
	    {
	      p = [NSMutableString new];
	      [d setObject: p forKey: @"Prefix"];
	      RELEASE(p);
	    }
	  else if ([p hasSuffix: @"("] == NO && [p hasSuffix: @"*"] == NO)
	    {
	      [p appendString: @" "];
	    }
	  [p appendString: @"("];
	  pos++;
	  [self parseDeclaratorInto: d];
	  if ([self parseSpace] < length && buffer[pos] == '(')
	    {
	      [self parseArgsInto: d];	// parse function args.
	    }
	  if ([self parseSpace] < length && buffer[pos] == ')')
	    {
	      if (s == nil && (s = [d objectForKey: @"Suffix"]) == nil)
		{
		  s = [NSMutableString new];
		  [d setObject: s forKey: @"Suffix"];
		  RELEASE(s);
		}
	      [s appendString: @")"];
	      pos++;
	      return;
	    }
	  else
	    {
	      [self log: @"missing ')' in declarator."];
	      return;
	    }
	}
      else
	{
	  NSString	*t;

	  t = [self parseIdentifier];
	  if (t == nil)
	    {
	      return;
	    }
	  if ([t isEqualToString: @"const"] || [t isEqualToString: @"volatile"])
	    {
	      if (p == nil && (p = [d objectForKey: @"Prefix"]) == nil)
		{
		  p = [NSMutableString new];
		  [d setObject: p forKey: @"Prefix"];
		  RELEASE(p);
		}
	      else if ([p hasSuffix: @"("] == NO)
		{
		  [p appendString: @" "];
		}
	      [p appendString: t];
	    }
	  else
	    {
	      [d setObject: t forKey: @"Name"];
	      return;
	    }
	}
    }
}

- (NSMutableDictionary*) parseDeclaration
{
  NSMutableDictionary	*d = [NSMutableDictionary dictionary];
  CREATE_AUTORELEASE_POOL(arp);
  static NSSet		*qualifiers = nil;
  static NSSet		*keep = nil;
  NSMutableString	*t = nil;
  NSMutableArray	*a;
  NSString		*s;
  BOOL			isTypedef = NO;
  BOOL			isPointer = NO;
  BOOL			isFunction = NO;
  BOOL			baseConstant = NO;
  BOOL			needScalarType = NO;

  if (qualifiers == nil)
    {
      qualifiers = [NSSet setWithObjects:
	@"auto",
	@"const",
	@"extern",
	@"inline",
	@"long",
	@"register",
	@"short",
	@"signed",
	@"static",
	@"typedef",
	@"unsigned",
	@"volatile",
	nil];
      IF_NO_GC([qualifiers retain];)
      keep = [NSSet setWithObjects:
	@"const",
	@"long",
	@"short",
	@"signed",
	@"unsigned",
	@"volatile",
	nil];
      IF_NO_GC([keep retain];)
    }

  a = [NSMutableArray array];
  while ((s = [self parseIdentifier]) != nil)
    {
      if (inHeader == NO && [s isEqualToString: @"static"] == YES)
	{
	  /*
	   * We don't want to document static declarations unless they
	   * occur in a public header.
	   */
	  [self skipStatementLine];
	  goto fail;
	}
      if (([s isEqual: @"__attribute__"] == YES)
          || ([s isEqual: @"__asm__"] == YES))
	{
	  if ([self skipSpaces] < length && buffer[pos] == '(')
	    {
	      unsigned	start = pos;

	      [self skipBlock];	// Skip the attributes
	      if (YES == verbose)
		{
		  NSString	*attr;

		  attr = [NSString stringWithCharacters: buffer + start
						 length: pos - start];
		  [self log: @"skip %@ %@", s, attr];
		}
	    }
	  else
	    {
	      [self log: @"strange format %@", s];
	    }
	  continue;
	}
      if ([s isEqualToString: @"GS_EXPORT"] == YES)
	{
	  s = @"extern";
	}
      if ([qualifiers member: s] == nil)
	{
	  break;
	}
      else
	{
	  if ([s isEqualToString: @"extern"] == YES
	    && [self skipSpaces] < length - 3 && buffer[pos] == '\"'
	    && buffer[pos+1] == 'C' && buffer[pos+2] == '\"')
	    {
	      /*
	       * Found 'extern "C" ...'
	       * Which is for C++ and should be ignored
	       */
	      pos += 3;
	      if ([self skipSpaces] < length && buffer[pos] == '{')
		{
		  pos++;
		  [self skipSpaces];
		}
	      IF_NO_GC(DESTROY(arp);)
	      return nil;
	    }

	  if ([s isEqualToString: @"typedef"] == YES)
	    {
	      isTypedef = YES;
	    }
	  if ([keep member: s] != nil)
	    {
	      [a addObject: s];
	      if ([s isEqual: @"const"] == NO && [s isEqual: @"volatile"] == NO)
		{
		  needScalarType = YES;
		}
	    }
	}
    }

  /**
   * We handle struct, union, and enum declarations by skipping the
   * stuff enclosed in curly braces.  If there was an identifier
   * after the keyword we use it as the struct name, otherwise we
   * use '...' to denote a nameless type.
   */
  if ([s isEqualToString: @"struct"] == YES
    || [s isEqualToString: @"union"] == YES
    || [s isEqualToString: @"enum"] == YES
    || [s isEqualToString: @"NS_ENUM"] == YES
    || [s isEqualToString: @"NS_OPTIONS"] == YES)
    {
      BOOL      isEnum = NO;
      NSString	*tmp = s;

      if ([s isEqualToString: @"NS_ENUM"]
        || [s isEqualToString: @"NS_OPTIONS"])
        {
          if ([self parseSpace] < length && buffer[pos] == '(')
            {
              pos++;
              [self parseSpace];
              s = [self parseIdentifier];
              if (nil != s && [self parseSpace] < length
                && buffer[pos] == ',')
                {
                  tmp = [tmp stringByAppendingFormat: @"(%@)", s];
                  pos++;
                  [self parseSpace];
                  s = [self parseIdentifier];
                  if (nil != s && [self parseSpace] < length
                    && buffer[pos] == ')')
                    {
                      isEnum = YES;
                      pos++;
                      [d setObject: s forKey: @"Name"];
                      s = tmp;
                    }
                }
            }
          if (NO == isEnum)
            {
              [self log: @"messed up NS_ENUM/NS_OPTIONS declaration"];
              [arp drain];
              return nil;
            }
        }
      else
        {
          isEnum = [s isEqualToString: @"enum"];

          s = [self parseIdentifier];
          if (s == nil)
            {
              s = [NSString stringWithFormat: @"%@ ...", tmp];
            }
          else
            {
              s = [NSString stringWithFormat: @"%@ %@", tmp, s];
              /*
               * It's possible to declare a struct, union, or enum without
               * giving it a name beyond after the declaration, in this case
               * we can use something like 'struct foo' as the name.
               */
              [d setObject: s forKey: @"Name"];
            }
        }

      /* We parse enum and options comment of the form:
       * <introComment> enum { <comment1> field1, <comment2> field2 } bla;
       */
      if (isEnum && [self parseSpace] < length && buffer[pos] == '{')
	{
          NSString *ident;
          NSString *introComment;
          NSMutableString *fieldComments = [NSMutableString string];
          BOOL foundFieldComment = NO;

          /* We want to be able to parse new comments while retaining the 
             originally parsed comment for the enum/union/struct. */
          introComment = [comment copy];
          DESTROY(comment);

          pos++; /* Skip '{' */

          [fieldComments appendString: @"<deflist>"];

          // TODO: We should put the parsed field into the doc index and 
          // let AGSOutput generate the deflist.
	  while (buffer[pos] != '}')
            {
	      /*
		 A comment belongs with the declaration following it,
		 unless it begins on the same line as a declaration.
                 Six combinations can be parsed:
                 - fieldDecl,
                 - <comment> fieldDecl,
                 - fieldDecl, <comment>
                 - <comment> fieldDecl, <comment>
                 - fieldDecl }
                 - <comment> fieldDecl }
	       */

	      /* Parse any space and comments before the identifier into
	       * 'comment' and get the identifier in 'ident'.
	       */
              ident = [self parseIdentifier];

              /* Skip the left-hand side such as ' = aValue'
	       */
              while (pos < length && buffer[pos] != ',' && buffer[pos] != '}')
                {
                  pos++;
                }
              if (buffer[pos] == ',')
		{
                  /* Parse any more space on the same line as the identifier
		   * appending it to the 'comment' ivar
		   */
		  [self parseSpace: spaces];
                  pos++;
		}

              if (ident != nil)
                {
                  foundFieldComment = YES;
                  [fieldComments appendString: @"<term><em>"];
                  [fieldComments appendString: ident];
                  [fieldComments appendString: @"</em></term>"];
                  [fieldComments appendString: @"<desc>"];
                  // NOTE: We could add a 'Description forthcoming' if nil
                  if (comment != nil)
                    {
                      [fieldComments appendString: comment];
                    }
                  [fieldComments appendString: @"</desc>\n"];
                }
              DESTROY(comment);
            }

          [fieldComments appendString: @"</deflist>"];

          /* Restore the comment as initially parsed before -parseDeclaration 
             was called and add the comments parsed per field into a deflist. */
          ASSIGN(comment, introComment);
          if (foundFieldComment)
            {
              NSString *enumComment = 
                [comment stringByAppendingFormat: @"\n\n%@", fieldComments];

              ASSIGN(comment, enumComment);
            }

          pos++; /* Skip closing curly brace */
	}
      [a addObject: s];
      s = nil;
    }
  else
    {
      if (s == nil)
	{
	  /*
	   * If there is no identifier here, the line must have been
	   * something like 'unsigned *length' so we must set the default
	   * base type of 'int'
	   */
	  [a addObject: @"int"];
	}
      else if (needScalarType == YES
	&& [s isEqualToString: @"char"] == NO
	&& [s isEqualToString: @"int"] == NO)
	{
	  /*
	   * If we had something like 'unsigned' in the qualifiers, we must
	   * have a 'char' or an 'int', and if we didn't find one we should
	   * insert one and use what we found as the variable name.
	   */
	  [a addObject: @"int"];
	}
      else
	{
	  [a addObject: s];
	  s = nil;	// s used as baseType
	}
    }

  /*
   * Now build a string containing the base type in a standardised form.
   */
  t = [NSMutableString new];

  if ([a containsObject: @"const"] == YES)
    {
      [t appendString: @"const"];
      [t appendString: @" "];
      [a removeObject: @"const"];
      baseConstant = YES;
    }
  else if ([a containsObject: @"volatile"] == YES)
    {
      [t appendString: @"volatile"];
      [t appendString: @" "];
      [a removeObject: @"volatile"];
    }

  if ([a containsObject: @"signed"] == YES)
    {
      [t appendString: @"signed"];
      [t appendString: @" "];
      [a removeObject: @"signed"];
    }
  else if ([a containsObject: @"unsigned"] == YES)
    {
      [t appendString: @"unsigned"];
      [t appendString: @" "];
      [a removeObject: @"unsigned"];
    }

  if ([a containsObject: @"short"] == YES)
    {
      [t appendString: @"short"];
      [t appendString: @" "];
      [a removeObject: @"short"];
    }
  else if ([a containsObject: @"long"] == YES)
    {
      unsigned	c = [a count];

      /*
       * There may be more than one 'long' in a type spec
       */
      while (c-- > 0)
	{
	  NSString	*tmp = [a objectAtIndex: c];

	  if ([tmp isEqual: @"long"] == YES)
	    {
	      [t appendString: tmp];
	      [t appendString: @" "];
	      [a removeObjectAtIndex: c];
	    }
	}
    }

  if ([a count] != 1)
    {
      [self log: @"odd values in declaration base type - '%@'", a];
      [t appendString: [a componentsJoinedByString: @" "]];
    }
  else
    {
      [t appendString: [a objectAtIndex: 0]];
    }
  [a removeAllObjects];		// Parsed base type


  /*
   * Handle protocol specification if necessary
   */
  if ([self parseSpace] < length && buffer[pos] == '<')
    {
      NSString	*p;

      do
	{
	  pos++;
	  p = [self parseIdentifier];
	  if (p != nil)
	    {
	      [a addObject: p];
	    }
	}
      while ([self parseSpace] < length && buffer[pos] == ',');
      pos++;
      [self parseSpace];
      [a sortUsingSelector: @selector(compare:)];
      [t appendString: @"<"];
      [t appendString: [a componentsJoinedByString: @","]];
      [t appendString: @">"];
      [a removeAllObjects];
    }

  [d setObject: t forKey: @"BaseType"];
  RELEASE(t);
  /*
   * Set the 'Kind' of declaration ... one of 'Types', 'Functions',
   * 'Variables', or 'Constants'
   * We may override this later.
   */
  if (isTypedef == YES)
    {
      [d setObject: @"Types" forKey: @"Kind"];
      [d setObject: @"YES" forKey: @"Implemented"];
    }
  else if (baseConstant == YES)
    {
      [d setObject: @"Constants" forKey: @"Kind"];
      [d setObject: @"YES" forKey: @"Implemented"];
    }
  else
    {
      [d setObject: @"Variables" forKey: @"Kind"];
    }

  if (s == nil)
    {
      [self parseDeclaratorInto: d];
      /*
       * There may have been '*' and 'const' applied to the declarator
       * which will change whether it is a constant or a variable, and
       * whether it is a pointer to something.
       * If the last thing to be applied was a '*' it is a variable
       * which points to a constant.  If the last thing was 'const'
       * then it is a constant (and may be a pointer too).
       */
      s = [d objectForKey: @"Prefix"];
      if (s != nil)
	{
	  NSRange	r;

	  r = [s rangeOfString: @"*"
		       options: NSBackwardsSearch|NSLiteralSearch];
	  if (r.length > 0)
	    {
	      unsigned	p = r.location;

	      isPointer = YES;
	      if (isTypedef == NO)
		{
		  r = [s rangeOfString: @"const"
			       options: NSBackwardsSearch|NSLiteralSearch];
		  if (r.length > 0 && r.location >= p)
		    {
		      [d setObject: @"Constants" forKey: @"Kind"];
		    }
		}
	    }
	}
    }
  else
    {
      [d setObject: s forKey: @"Name"];
    }

  if ([self parseSpace] < length)
    {
      if (buffer[pos] == '[')
	{
	  NSMutableString	*suffix;

	  if ((suffix = [d objectForKey: @"Suffix"]) == nil)
	    {
	      suffix = [NSMutableString new];
	      [d setObject: suffix forKey: @"Suffix"];
	      RELEASE(suffix);
	    }
	  while (buffer[pos] == '[')
	    {
	      unsigned	old = pos;

	      if ([self skipArray] == old)
		{
		  break;
		}
	      [suffix appendString: @"[]"];
	    }
	}
      else if (buffer[pos] == '(')
	{
	  [self parseArgsInto: d];
	}
    }

  if ([d objectForKey: @"Args"] != nil)
    {
      /*
       * If the declaration looked like this int (*foo)() then
       * 'isPointer' will be YES and 'Suffix' will contain the
       * bracket after 'foo'.  In this case, what we have is a
       * variable or constant pointer to a function.
       * Otherwise, we have a function declaration and the
       * 'Kind' should be set to 'function'.
       */
      if (isPointer == NO || [d objectForKey: @"Suffix"] == nil)
	{
	  [d setObject: @"Functions" forKey: @"Kind"];
	  isFunction = YES;
	}
    }

  if ([self parseSpace] < length)
    {
      if (inArgList == YES)
	{
	  if (buffer[pos] == ')' || buffer[pos] == ',')
	    {
	      [arp drain];
	      return d;
	    }
	  else
	    {
	      [self log: @"Unexpected char (%c) in arg list", buffer[pos]];
	      [self skipStatement];
	      goto fail;
	    }
	}
      else
	{
	  if (isFunction == YES)
	    {
	      NSString	*ident = [self parseIdentifier];

	      if ([ident isEqual: @"__attribute__"] == YES)
		{
		  if ([self skipSpaces] < length && buffer[pos] == '(')
		    {
		      unsigned	start = pos;
		      NSString	*attr;

		      [self skipBlock];	// Skip the attributes
		      attr = [NSString stringWithCharacters: buffer + start
						     length: pos - start];
		      if ([attr rangeOfString: @"deprecated"].length > 0)
			{
			  [self appendComment: @"<em>Warning</em> this is "
			    @"<em>deprecated</em> and may be removed in "
			    @"future versions"
			    to: nil];
			}
		    }
		  else
		    {
		      [self log: @"strange format function attributes"];
		    }
		}
	      else if ([ident length] > 0)
		{
		  [self log: @"ignoring '%@' in function declaration", ident];
		}
	    }

	  if (buffer[pos] == '_')
	    {
	      NSString	*ident = [self parseIdentifier];

	      if ([ident isEqualToString: @"__attribute__"] == YES)
		{
		  [self skipSpaces];
		  if (pos < length && buffer[pos] == '(')
		    {
		      unsigned	start = pos;
		      NSString	*attr;

		      [self skipBlock];
		      attr = [NSString stringWithCharacters: buffer + start
						     length: pos - start];
		      if ([attr rangeOfString: @"deprecated"].length > 0)
			{
			  [self appendComment: @"<em>Warning</em> this is "
			    @"<em>deprecated</em> and may be removed in "
			    @"future versions"
			    to: nil];
			}
		      [self skipSpaces];
		    }
		}
	      else
		{
		  [self log: @"Underscore is not from __attribute__"];
		  goto fail;
		}
	      if (pos >= length)
		{
		  [self log: @"Unexpected end of declaration"];
		  goto fail;
		}
	    }

	  if (buffer[pos] == ';')
	    {
	      [self skipStatement];
	    }
	  else if (buffer[pos] == ',')
	    {
	      [self log: @"ignoring multiple comma separated declarations"];
	      [self skipStatement];
	    }
	  else if (buffer[pos] == '=')
	    {
	      [self skipStatement];
	    }
	  else if (buffer[pos] == '{')
	    {
	      /*
	       * Inline functions may be implemented in the header.
	       */
	      if (isFunction == YES)
	      	{
		  [d setObject: @"YES" forKey: @"Implemented"];
		}
	      [self skipBlock];
	    }
	  else if (buffer[pos] == '}')
	    {
	      pos++;			// Ignore extraneous '}'
	      [self skipSpaces];
	    }
	  else
	    {
	      [self log: @"Unexpected char (%c) in declaration", buffer[pos]];
	      [self skipStatement];
	      goto fail;
	    }

	  /*
	   * Read in any comment on the same line in case it
	   * contains documentation for the declaration.
	   */
	  if ([self skipSpaces] < length && buffer[pos] == '/')
	    {
	      [self parseComment];
	    }
	}
      if (comment != nil)
	{
	  [self appendComment: comment to: d];
	}
      DESTROY(comment);

      [arp drain];
      if (inArgList == NO)
	{
	  /*
	   * This is a top-level declaration, so let's tidy up ready for
	   * linking into the documentation tree.
	   */
	  if ([d objectForKey: @"Name"] == nil)
	    {
	      NSString	*t = [d objectForKey: @"BaseType"];

	      /*
	       * Don't bother to warn about nameless enumerations.
	       */
	      if (verbose == YES && [t isEqual: @"enum ..."] == NO)
		{
		  [self log: @"parse declaration with no name - %@", d];
		}
	      return nil;
	    }
	}
      [self setStandards: d];
      return d;
    }
  else
    {
      [self log: @"unexpected end of data parsing declaration"];
    }
fail:
  DESTROY(comment);
  [arp drain];
  return nil;
}

- (NSMutableDictionary*) parseFile: (NSString*)name isSource: (BOOL)isSource
{
  NSString		*token;
  NSMutableDictionary	*nDecl;

  if (isSource == YES)
    {
      inHeader = NO;
    }
  else
    {
      inHeader = YES;
    }
  commentsRead = NO;
  ASSIGNCOPY(fileName, name);
  if (declared == nil)
    {
      ASSIGN(declared, [fileName lastPathComponent]);
    }
  /**
   * If this is parsing a header file (isSource == NO) then we reset the
   * list of known source files associated with the header before proceeding.
   */
  [source removeAllObjects];
  if (isSource == NO)
    {
      NSFileManager	*mgr = [NSFileManager defaultManager];
      NSString		*path;

      [info setObject: fileName forKey: @"Header"];
      [source removeAllObjects];

      /**
       * We initially assume that the location of a source file is the
       * same as the header, but if there is no file at that location,
       * we expect the source to be in the documentatation directory
       * or the current directory instead.
       */
      path = [fileName stringByDeletingPathExtension];
      path = [path stringByAppendingPathExtension: @"m"];
      if ([mgr isReadableFileAtPath: path] == NO)
	{
	  path = [path lastPathComponent];
	  if ([mgr isReadableFileAtPath: path] == NO)
	    {
	      NSUserDefaults	*defs;
	      NSString		*ddir;

	      defs = [NSUserDefaults standardUserDefaults];
	      ddir = [defs stringForKey: @"DocumentationDirectory"];
	      if ([ddir length] > 0)
		{
		  path = [ddir stringByAppendingPathComponent: path];
		  if ([mgr isReadableFileAtPath: path] == NO)
		    {
		      path = nil;	// No default source file found.
		    }
		}
	      else
		{
		  path = nil;	// No default source file found.
		}
	    }
	}
      if (path != nil)
	{
	  [source addObject: path];
	}
    }
  DESTROY(unitName);
  DESTROY(itemName);
  DESTROY(comment);

  [self setupBuffer];

  while ([self parseSpace] < length)
    {
      unichar	c = buffer[pos++];

      switch (c)
	{
	  case '#':
	    /*
	     * Some preprocessor directive ... must be on one line ... skip
	     * past it and delete any comment accumulated while doing so.
	     */
	    [self parsePreprocessor];
	    DESTROY(comment);
	    break;

	  case '@':
	    token = [self parseIdentifier];
	    if (token != nil)
	      {
		if ([token isEqual: @"interface"] == YES)
		  {
		    if (isSource == YES)
		      {
			[self skipUnit];
			DESTROY(comment);
		      }
		    else
		      {
			[self parseInterface];
		      }
		  }
		else if ([token isEqual: @"protocol"] == YES)
		  {
		    if (isSource == YES)
		      {
			[self skipUnit];
			DESTROY(comment);
		      }
		    else
		      {
			[self parseProtocol];
		      }
		  }
		else if ([token isEqual: @"implementation"] == YES)
		  {
		    if (isSource == YES)
		      {
			[self parseImplementation];
		      }
		    else
		      {
			[self skipUnit];
			DESTROY(comment);
		      }
		  }
		else
		  {
		    [self skipStatementLine];
		  }
	      }
	    break;

	  default:
	    /*
	     * Must be some sort of declaration ...
	     */
	    pos--;
	    nDecl = [self parseDeclaration];
	    if (nDecl != nil)
	      {
		NSString		*name = [nDecl objectForKey: @"Name"];
		NSString		*kind = [nDecl objectForKey: @"Kind"];
		NSMutableDictionary	*dict = [info objectForKey: kind];

		if (isSource == NO)
		  {
		    /*
		     * Ensure that we have an entry for this declaration.
		     */
		    if (dict == nil)
		      {
			dict = [NSMutableDictionary new];
			[info setObject: dict forKey: kind];
			RELEASE(dict);
		      }
		    [dict setObject: nDecl forKey: name];
		  }
		else
		  {
		    NSMutableDictionary	*oDecl = [dict objectForKey: name];

		    if (oDecl != nil)
		      {
		        NSString	*oc = [oDecl objectForKey: @"Comment"];
		        NSString	*nc = [nDecl objectForKey: @"Comment"];

			/*
			 * If the old comment from the header parsing is
			 * the same as the new comment from the source
			 * parsing, assume we parsed the same file as both
			 * source and header ... otherwise append the new
			 * comment.
			 */
		        if ([oc isEqual: nc] == NO)
			  {
			    [self appendComment: nc to: oDecl];
			  }
			[oDecl setObject: @"YES" forKey: @"Implemented"];

			if ([kind isEqualToString: @"Functions"] == YES)
			  {
			    NSArray	*a1 = [oDecl objectForKey: @"Args"];
			    NSArray	*a2 = [nDecl objectForKey: @"Args"];

			    if ([a1 isEqual: a2] == NO)
			      {
				[self log: @"Function %@ args mismatch - "
				  @"%@ %@", name, a1, a2];
			      }
			    /*
			     * A main function is not documented as a
			     * function, but as a special case its
			     * comments are added to the 'front'
			     * section of the documentation.
			     */
			    if ([name isEqual: @"main"] == YES)
			      {
			        NSString	*c;

			        c = [oDecl objectForKey: @"Comment"];
				if (c != nil)
				  {
				    [self addMain: c];
				  }
				[dict removeObjectForKey: name];
			      }
			  }
		      }
		  }
	      }
	    break;
        }
    }

  return info;
}

- (NSMutableDictionary*) parseImplementation
{
  NSString		*nc = nil;
  NSString		*name;
  NSString		*base = nil;
  NSString		*category = nil;
  NSDictionary		*methods = nil;
  NSMutableDictionary	*d;
  NSMutableDictionary	*dict = nil;
  CREATE_AUTORELEASE_POOL(arp);

  /*
   * Record any class documentation for this class
   */
  nc = AUTORELEASE(comment);
  comment = nil;

  if ((name = [self parseIdentifier]) == nil
    || [self parseSpace] >= length)
    {
      [self log: @"implementation with bad name"];
      goto fail;
    }
  ASSIGNCOPY(unitName, name);

  /*
   * After the class name, we may have a category name or
   * a base class, but not both.
   */
  if (buffer[pos] == '(')
    {
      pos++;
      if ((category = [self parseIdentifier]) == nil
	|| [self parseSpace] >= length
	|| buffer[pos++] != ')'
	|| [self parseSpace] >= length)
	{
	  [self log: @"interface with bad category"];
	  goto fail;
	}
      name = [name stringByAppendingFormat: @"(%@)", category];
      ASSIGN(unitName, name);
    }
  else if (buffer[pos] == ':')
    {
      pos++;
      if ((base = [self parseIdentifier]) == nil
	|| [self parseSpace] >= length)
	{
	  [self log: @"@interface with bad base class"];
	  goto fail;
	}
    }

  if (category == nil)
    {
      d = [info objectForKey: @"Classes"];
    }
  else
    {
      d = [info objectForKey: @"Categories"];
    }
  dict = [d objectForKey: unitName];

  if (dict == nil)
    {
      /*
       * If the implementation found does not correspond to an
       * interface found in the header file, it should not be
       * documented, and we skip it.
       */
      [self skipUnit];
      DESTROY(comment);
      [arp drain];
      return [NSMutableDictionary dictionary];
    }
  else
    {
      NSString	*oc = [dict objectForKey: @"Comment"];

      [dict setObject: @"YES" forKey: @"Implemented"];
      /*
       * Append any comment we have for this ... if it's not just a copy
       * because we've parsed the same file twice.
       */
      if ([oc isEqual: nc] == NO)
	{
	  [self appendComment: nc to: dict];
	}
      /*
       * Update base class if necessary.
       */
      if (base != nil)
	{
	  if ([base isEqual: [dict objectForKey: @"BaseClass"]] == NO)
	    {
	      [self log: @"implementation base class differs from interface"];
	    }
	  [dict setObject: base forKey: @"BaseClass"];
	}
    }

  methods = [self parseMethodsAreDeclarations: NO];
  if (methods != nil && [methods count] > 0)
    {
      // [dict setObject: methods forKey: @"Methods"];
    }

  // [self log: @"Found implementation %@", dict];

  DESTROY(unitName);
  DESTROY(comment);
  [arp drain];
  return dict;

fail:
  DESTROY(unitName);
  DESTROY(comment);
  [arp drain];
  return nil;
}

- (NSMutableDictionary*) parseInterface
{
  NSString		*name;
  NSString		*base = nil;
  NSString		*category = nil;
  NSDictionary		*methods = nil;
  NSMutableDictionary	*d;
  NSMutableDictionary	*dict;
  CREATE_AUTORELEASE_POOL(arp);

  dict = [NSMutableDictionary dictionaryWithCapacity: 8];

  /*
   * Record any class documentation for this class
   */
  if (comment != nil)
    {
      [self appendComment: comment to: dict];
      DESTROY(comment);
    }

  if ((name = [self parseIdentifier]) == nil
    || [self parseSpace] >= length)
    {
      [self log: @"interface with bad name"];
      goto fail;
    }
  ASSIGNCOPY(unitName, name);

  [dict setObject: @"class" forKey: @"Type"];
  [self setStandards: dict];

  /*
   * After the class name, we may have a category name or
   * a base class, but not both.
   */
  if (buffer[pos] == '(')
    {
      pos++;
      if ((category = [self parseIdentifier]) == nil
	|| [self parseSpace] >= length
	|| buffer[pos++] != ')'
	|| [self parseSpace] >= length)
	{
	  [self log: @"interface with bad category"];
	  goto fail;
	}
      [dict setObject: category forKey: @"Category"];
      [dict setObject: name forKey: @"BaseClass"];
      name = [name stringByAppendingFormat: @"(%@)", category];
      ASSIGN(unitName, name);
      [dict setObject: @"category" forKey: @"Type"];
      if ([category length] >= 7
	&& [category compare: @"Private"
		     options: NSCaseInsensitiveSearch
		       range: NSMakeRange(0, 7)] == NSOrderedSame)
	{
	  NSString	*c;

	  c = @"<em>Warning</em> this category is <em>private</em>, which "
	    @"means that the methods are for internal use by the package. "
	    @"You should not use them in external code.";
	  [self appendComment: c to: dict];
	}
    }
  else if (buffer[pos] == ':')
    {
      pos++;
      if ((base = [self parseIdentifier]) == nil
	|| [self parseSpace] >= length)
	{
	  [self log: @"@interface with bad base class"];
	  goto fail;
	}
      [dict setObject: base forKey: @"BaseClass"];
    }
  [dict setObject: name forKey: @"Name"];

  /*
   * Interfaces or categories may conform to protocols.
   */
  if (buffer[pos] == '<')
    {
      NSArray	*protocols = [self parseProtocolList];

      if (protocols == nil)
	{
	  goto fail;
	}
      else if ([protocols count] > 0)
	{
	  [dict setObject: protocols forKey: @"Protocols"];
	}
    }

  /*
   * Interfaces may have instance variables, but categories may not.
   */
  if (buffer[pos] == '{' && category == nil)
    {
      NSDictionary	*ivars = [self parseInstanceVariables];
      if (ivars == nil)
	{
	  goto fail;
	}
      else if ([ivars count] > 0)
	{
	  [dict setObject: ivars forKey: @"InstanceVariables"];
	}
      DESTROY(comment);		// Ignore any ivar comments.
    }

  methods = [self parseMethodsAreDeclarations: YES];
  if (methods != nil && [methods count] > 0)
    {
      [dict setObject: methods forKey: @"Methods"];
    }

  [dict setObject: declared forKey: @"Declared"];

  if (category == nil)
    {
      d = [info objectForKey: @"Classes"];
      if (d == nil)
	{
	  d = [[NSMutableDictionary alloc] initWithCapacity: 4];
	  [info setObject: d forKey: @"Classes"];
	  RELEASE(d);
	}
    }
  else
    {
      d = [info objectForKey: @"Categories"];
      if (d == nil)
	{
	  d = [[NSMutableDictionary alloc] initWithCapacity: 4];
	  [info setObject: d forKey: @"Categories"];
	  RELEASE(d);
	}
    }
  [d setObject: dict forKey: unitName];

  // [self log: @"Found interface %@", dict];

  DESTROY(unitName);
  DESTROY(comment);
  [arp drain];
  return dict;

fail:
  DESTROY(unitName);
  DESTROY(comment);
  [arp drain];
  return nil;
}

/**
 * Attempt to parse an identifier/keyword (with optional whitespace in
 * front of it).  Perform mappings using the wordMap dictionary.  If a
 * mapping produces an empty string, we treat it as if we had read
 * whitespace and try again.
 * If we read end of data, or anything which is invalid inside an
 * identifier, we return nil.
 * If we read a GS_GENERIC... macro, we return its first argument.
 */
- (NSString*) parseIdentifier
{
  unsigned	start;

try:
  [self parseSpace];
  if (pos >= length || [identStart characterIsMember: buffer[pos]] == NO)
    {
      return nil;
    }
  start = pos;
  while (pos < length)
    {
      if ([identifier characterIsMember: buffer[pos]] == NO)
	{
	  NSString	*tmp;
	  NSString	*val;

	  tmp = [[NSString alloc] initWithCharacters: &buffer[start]
					      length: pos - start];
          if ([tmp isEqual: @"GS_GENERIC_CLASS"]
            || [tmp isEqual: @"GS_GENERIC_TYPE"])
            {
              [self skipSpaces];
              if (pos < length && buffer[pos] == '(')
                {
                  pos++;
                  /* Found a GS_GENERIC_ macro ... the first
                   * identifier inside the macro arguments is the 
                   * name we want to return.
                   */
                  RELEASE(tmp);
                  tmp = RETAIN([self parseIdentifier]);
                  while (pos < length)
                    {
                      if (buffer[pos++] == ')')
                        {
                          break;
                        }
                    }
                }
            }
	  val = [wordMap objectForKey: tmp];
	  if (val == nil)
	    {
	      return AUTORELEASE(tmp);	// No mapping found.
	    }
	  RELEASE(tmp);
	  if ([val length] > 0)
	    {
	      if ([val isEqualToString: @"//"] == YES)
		{
		  [self skipToEndOfLine];
		  return [self parseIdentifier];
		}
	      return val;	// Got mapped identifier.
	    }
	  goto try;		// Mapping removed the identifier.
	}
      pos++;
    }
  return nil;
}

- (NSMutableDictionary*) parseInstanceVariables
{
  NSString		*validity = @"protected";
  NSMutableDictionary	*ivars;
  BOOL			shouldDocument = documentInstanceVariables;
  DESTROY(comment);

  inInstanceVariables = YES;

  ivars = [NSMutableDictionary dictionaryWithCapacity: 8];
  pos++;
  while ([self parseSpace] < length && buffer[pos] != '}')
    {
      if (buffer[pos] == '@')
	{
	  NSString	*token;

	  pos++;
	  if ((token = [self parseIdentifier]) == nil
	    || [self parseSpace] >= length)
	    {
	      [self log: @"interface with bad validity directive"];
	      goto fail;
	    }
	  if ([token isEqual: @"private"] == YES)
	    {
	      ASSIGN(validity, token);
	      shouldDocument = documentInstanceVariables
                                 && documentAllInstanceVariables;
	    }
	  else if ([token isEqual: @"protected"] == YES)
	    {
	      ASSIGN(validity, token);
	      shouldDocument = documentInstanceVariables;
	    }
	  else if ([token isEqual: @"package"] == YES)
	    {
	      ASSIGN(validity, token);
	      shouldDocument = documentInstanceVariables;
	    }
	  else if ([token isEqual: @"public"] == YES)
	    {
	      ASSIGN(validity, token);
	      shouldDocument = documentInstanceVariables;
	    }
	  else
	    {
	      [self log: @"interface with bad validity (%@)", token];
	      goto fail;
	    }
	}
      else if (buffer[pos] == '#')
	{
	  [self parsePreprocessor];	// Ignore preprocessor directive.
	  DESTROY(comment);
	}
      else if (shouldDocument == YES)
	{
	  NSMutableDictionary	*iv = [self parseDeclaration];

	  if (iv != nil)
	    {
	      if ([validity isEqual: @"private"] == NO)
		{
		  NSString	*n = [iv objectForKey: @"Name"];

		  if ([n hasPrefix: @"_"] == YES)
		    {
		      NSString	*c;

		      c = @"<em>Warning</em> the underscore at the start of "
			@"the name of this instance variable indicates that, "
			@"even though it is not technically <em>private</em>, "
			@"it is intended for internal use within the package, "
			@"and you should not use the variable in other code.";
		      [self appendComment: c to: iv];
		    }
		}
	      [iv setObject: validity forKey: @"Validity"];
	      [ivars setObject: iv forKey: [iv objectForKey: @"Name"]];
	    }
	}
      else
	{
	  [self skipStatement];
	}
    }

  inInstanceVariables = NO;

  if (pos >= length)
    {
      [self log: @"interface with bad instance variables"];
      return nil;
    }
  pos++;	// Step past closing bracket.
  return ivars;
fail:
  DESTROY(comment);
  inInstanceVariables = NO;
  return nil;
}

/**
 * Parse a macro definition ... we are expected to have read #define already
 */
- (NSMutableDictionary*) parseMacro
{
  NSMutableDictionary	*dict;
  NSMutableArray	*a = nil;
  NSString		*name;

  dict = [[NSMutableDictionary alloc] initWithCapacity: 4];
  [self parseSpace: spaces];
  name = [self parseIdentifier];
  if (nil == name)
    {
      // [self log: @"Missing name in #define"];
      return nil;
    }
  [self parseSpace: spaces];
  if (pos < length && buffer[pos] == '(')
    {
      a = [[NSMutableArray alloc] initWithCapacity: 4];

      pos++;	// Step past opening '('

      while ([self parseSpace: spaces] < length && buffer[pos] != ')')
	{
	  if (buffer[pos] == ',')
	    {
	      pos++;
	    }
	  else if (buffer[pos] == '.')
	    {
	      pos += 3;	// Skip '...'
	      [dict setObject: @"YES" forKey: @"VarArgs"];
	    }
	  else
	    {
	      NSString	*s;

	      s = [self parseIdentifier];
	      if (s == nil)
		{
		  break;
		}
	      [a addObject: s];
	    }
	}
      if (pos < length)
	{
	  pos++;	// Step past closing ')'
	}
    }

  /*
   * Now parse macro body (to end of line) gathering any comments.
   */
  [self parseSpace: spaces];
  while (pos < length)
    {
      unsigned	c = buffer[pos];

      if (c == '\n')
        {
	  break;
	}
      else if (c == '/')
	{
	  unsigned	save = pos;

	  if ([self parseComment] == save)
	    {
	      pos++;	// Step past  '/'
	    }
	}
      else if (c == '\'' || c == '"')
        {
	  [self skipLiteral];
        }
      else if ([spaces characterIsMember: c] == NO)
        {
	  pos++;
	}
      else
        {
	  [self parseSpace: spaces];
	}
    }

  /**
   * It's common to have macros which don't need commenting ...
   * like the ones used to protect a header against multiple
   * inclusion for instance.  For this reason, we ignore any
   * macro which is not preceded by a documentation comment.
   */
  if ([comment length] > 0)
    {
      [dict setObject: name forKey: @"Name"];
      if (a != nil)
        {
	  [dict setObject: a forKey: @"Args"];
	}
      /* A macro is implemented as soon as it is defined. */
      [dict setObject: @"YES" forKey: @"Implemented"];
      [self appendComment: comment to: dict];
    }
  else
    {
      DESTROY(dict);
    }
  RELEASE(a);
  [self setStandards: dict];
  return AUTORELEASE(dict);
}

- (NSMutableDictionary*) parseMethodIsDeclaration: (BOOL)flag
{
  CREATE_AUTORELEASE_POOL(arp);
  NSMutableDictionary	*method;
  NSMutableString	*mname;
  NSString		*token;
  NSMutableArray	*types = nil;
  NSMutableArray	*args = nil;
  NSMutableArray	*sels = [NSMutableArray arrayWithCapacity: 2];
  unichar		term;

  method = [[NSMutableDictionary alloc] initWithCapacity: 4];
  if (buffer[pos++] == '-')
    {
      mname = [NSMutableString stringWithUTF8String: "-"];
    }
  else
    {
      mname = [NSMutableString stringWithUTF8String: "+"];
    }
  [method setObject: sels forKey: @"Sels"];	// Parts of selector.

  /*
   * Parse return type ... defaults to 'id'
   */
  if ([self parseSpace] >= length)
    {
      [self log: @"error parsing method return type"];
      goto fail;
    }
  if (buffer[pos] == '(')
    {
      if ((token = [self parseMethodType]) == nil
	|| [self parseSpace] >= length)
	{
	  [self log: @"error parsing method return type"];
	  goto fail;
	}
      [method setObject: token forKey: @"ReturnType"];
    }
  else
    {
      [method setObject: @"id" forKey: @"ReturnType"];
    }

  if (flag == YES)
    {
      term = ';';
    }
  else
    {
      term = '{';
    }

  while (buffer[pos] != term)
    {
      token = [self parseIdentifier];
      if ([token isEqual: @"__attribute__"] == YES)
	{
	  if ([self skipSpaces] < length && buffer[pos] == '(')
	    {
	      unsigned	start = pos;
	      NSString	*attr;

	      [self skipBlock];	// Skip the attributes
	      attr = [NSString stringWithCharacters: buffer + start
					     length: pos - start];
	      if ([attr rangeOfString: @"deprecated"].length > 0)
		{
		  [self appendComment: @"<em>Warning</em> this is "
		    @"<em>deprecated</em> and may be removed in "
		    @"future versions"
		    to: nil];
		}
	    }
	  else
	    {
	      [self log: @"strange format function attributes"];
	    }
	  continue;
	}
      if ([self parseSpace] >= length)
	{
	  [self log: @"error at method name component"];
	  goto fail;
	}
      if (buffer[pos] == ':')
	{
	  NSString	*arg;
	  NSString	*type = @"id";

	  pos++;
	  if (token == nil)
	    {
	      [sels addObject: @":"];
	    }
	  else
	    {
	      [mname appendString: token];
	      [sels addObject: [token stringByAppendingString: @":"]];
	    }
	  [mname appendString: @":"];
	  if ([self parseSpace] >= length)
	    {
	      [self log: @"error parsing method argument"];
	      goto fail;
	    }
	  if (buffer[pos] == '(')
	    {
	      if ((type = [self parseMethodType]) == nil
		|| [self parseSpace] >= length)
		{
		  [self log: @"error parsing method arguument type"];
		  goto fail;
		}
	    }
	  if ((arg = [self parseIdentifier]) == nil
	    || [self parseSpace] >= length)
	    {
	      [self log: @"error parsing method argument name"];
	      goto fail;
	    }

	  if (types == nil)
	    {
	      types = [NSMutableArray arrayWithCapacity: 2];
	      [method setObject: types forKey: @"Types"];
	    }
	  [types addObject: type];

	  if (args == nil)
	    {
	      args = [NSMutableArray arrayWithCapacity: 2];
	      [method setObject: args forKey: @"Args"];
	    }
	  [args addObject: arg];

	  if (buffer[pos] == ',')
	    {
	      [method setObject: @"YES" forKey: @"VarArgs"];
	      [mname appendString: @",..."];
	      while ([self parseSpace] < length)
		{
		  if (buffer[pos] == term)
		    {
		      break;
		    }
		  pos++;
		}
	      if (buffer[pos] != term)
		{
		  [self log: @"error skipping varargs"];
		  goto fail;
		}
	    }
	}
      else if (token != nil)
	{
	  [sels addObject: token];
	  [mname appendString: token];
	  if (buffer[pos] != term)
	    {
	      unsigned	saved = pos;

	      /*
	       * As a special case, try to cope with a method name separated
	       * from its body by a semicolon ... a common bug since the
	       * compiler doesn't pick it up!
	       */
	      if (term == '{' && buffer[pos] == ';')
		{
		  pos++;
		  if ([self parseSpace] >= length || buffer[pos] != term)
		    {
		      pos = saved;
		    }
		}
	      if (buffer[pos] == term)
		{
		  [self log: @"error in method definition ... "
		    @"semicolon after name"];
		}
	      else
		{
		  [self log: @"error parsing method name"];
		  goto fail;
		}
	    }
	}
      else
	{
	  unsigned	saved = pos;

	  /*
	   * As a special case, try to cope with a method name separated
	   * from its body by a semicolon ... a common bug since the
	   * compiler doesn't pick it up!
	   */
	  if (term == '{' && buffer[pos] == ';')
	    {
	      pos++;
	      if ([self parseSpace] >= length || buffer[pos] != term)
		{
		  pos = saved;
		}
	    }
	  if (buffer[pos] == term)
	    {
	      [self log: @"error in method definition ... "
		@"semicolon after name"];
	    }
	  else
	    {
	      [self log: @"error parsing method name"];
	      goto fail;
	    }
	}
    }

  [method setObject: mname forKey: @"Name"];
  if (flag == YES)
    {
      [self setStandards: method];
    }
  ASSIGNCOPY(itemName, mname);

  if (term == ';')
    {
      /*
       * Skip past the closing semicolon of the method declaration,
       * and read in any comment on the same line in case it
       * contains documentation for the method.
       */
      pos++;
      if ([self skipSpaces] < length && buffer[pos] == '/')
	{
	  [self parseComment];
	}
    }
  else if (term == '{')
    {
      BOOL	isEmpty;

      [self skipBlock: &isEmpty];
      if (isEmpty == YES)
	{
	  [method setObject: @"YES" forKey: @"Empty"];
	}
      else
	{
	  [method setObject: @"NO" forKey: @"Empty"];
	}
    }

  /*
   * Store any available documentation information in the method.
   * If the method is already documented, append new information.
   */
  if (comment != nil)
    {
      [self appendComment: comment to: method];
      DESTROY(comment);
    }
  if (flag == YES
    && [itemName length] > 1 && [itemName characterAtIndex: 1] == '_')
    {
      NSString	*c;

      c = @"<em>Warning</em> the underscore at the start of the name "
	@"of this method indicates that it is private, for internal use only, "
	@" and you should not use the method in your code.";
      [self appendComment: c to: method];
    }

  DESTROY(itemName);
  [arp drain];
  IF_NO_GC([method autorelease];)
  return method;

fail:
  DESTROY(itemName);
  DESTROY(comment);
  [arp drain];
  RELEASE(method);
  return nil;
}

- (void) addOrderedSymbolDeclaration: (NSString *)aMethodOrFunc toUnit: (NSString *)aUnitName
{
  NSMutableArray *orderedSymbolDecls = [orderedSymbolDeclsByUnit objectForKey: aUnitName];

  if (orderedSymbolDecls == nil)
    {
      orderedSymbolDecls = [NSMutableArray array];
      [orderedSymbolDeclsByUnit setObject: orderedSymbolDecls
				   forKey: aUnitName];
    }
  [orderedSymbolDecls addObject: aMethodOrFunc];
}

- (NSMutableDictionary*) parseMethodsAreDeclarations: (BOOL)flag
{
  NSMutableDictionary	*methods;
  NSMutableDictionary	*method;
  NSMutableDictionary	*exist;
  NSString		*token;
  BOOL			optionalMethods = NO;

  if (flag == YES)
    {
      methods = [NSMutableDictionary dictionaryWithCapacity: 8];
    }
  else
    {
      /*
       * Get a list of known methods.
       */
      if ([unitName hasPrefix: @"("])
	{
	  exist = nil;	// A protocol ... no method implementations.
	}
      else if ([unitName hasSuffix: @")"])
	{
	  exist = [info objectForKey: @"Categories"];
	}
      else
	{
	  exist = [info objectForKey: @"Classes"];
	}
      exist = [exist objectForKey: unitName];
      exist = [exist objectForKey: @"Methods"];
      /*
       * If there were no methods in the interface, we can't
       * document any now so we may as well skip to the end.
       */
      if (exist == nil)
	{
	  [self skipUnit];
	  DESTROY(comment);
	  return [NSMutableDictionary dictionary];	// Empty dictionary.
	}
      methods = exist;
    }

  while ([self parseSpace] < length)
    {
      unichar	c = buffer[pos++];

      switch (c)
	{
	  case '-':
	  case '+':
	    pos--;
	    method = [self parseMethodIsDeclaration: flag];
	    if (method == nil)
	      {
		return nil;
	      }
	    if (YES == optionalMethods)
	      {
		[method setObject: @"YES" forKey: @"Optional"];
	      }
	    token = [method objectForKey: @"Name"];
	    if (flag == YES)
	      {
		/*
		 * Just record the method.
		 */
		[methods setObject: method forKey: token];
                [self addOrderedSymbolDeclaration: token toUnit: unitName];
	      }
	    else if ((exist = [methods objectForKey: token]) != nil)
	      {
		NSArray		*a0;
		NSArray		*a1;
		NSString	*c0;
		NSString	*c1;
		NSString	*e;

		/*
		 * Merge info from implementation into existing version.
		 */

		a0 = [exist objectForKey: @"Args"];
		a1 = [method objectForKey: @"Args"];
		if (a0 != nil)
		  {
		    if ([a0 isEqual: a1] == NO)
		      {
			ASSIGNCOPY(itemName, token);
			[self log: @"method args in interface %@ don't match "
			  @"those in implementation %@", a0, a1];
			DESTROY(itemName);
			[exist setObject: a1 forKey: @"Args"];
		      }
		  }

		a0 = [exist objectForKey: @"Types"];
		a1 = [method objectForKey: @"Types"];
		if (a0 != nil)
		  {
		    if ([a0 isEqual: a1] == NO)
		      {
			ASSIGNCOPY(itemName, token);
			[self log: @"method types in interface %@ don't match "
			  @"those in implementation %@", a0, a1];
			DESTROY(itemName);
			[exist setObject: a1 forKey: @"Types"];
		      }
		  }

		/*
		 * If the old comment from the header parsing is
		 * the same as the new comment from the source
		 * parsing, assume we parsed the same file as both
		 * source and header ... otherwise append the new
		 * comment.
		 */
		c0 = [exist objectForKey: @"Comment"];
		c1 = [method objectForKey: @"Comment"];
		if ([c0 isEqual: c1] == NO)
		  {
		    [self appendComment: c1 to: exist];
		  }
		[exist setObject: @"YES" forKey: @"Implemented"];

		/*
		 * Record if the implementation is not empty.
		 */
		e = [method objectForKey: @"Empty"];
		if (e != nil)
		  {
		    [exist setObject: e forKey: @"Empty"];
		  }
	      }
	    DESTROY(comment);	// Don't want this.
	    break;

	  case '@':
	    if ((token = [self parseIdentifier]) == nil)
	      {
		[self log: @"method list with error after '@'"];
		[self skipStatementLine];
		return nil;
	      }
	    if ([token isEqual: @"end"] == YES)
	      {
		return methods;
	      }
	    else if ([token isEqual: @"optional"] == YES)
	      {
	        /* marking remaining methods as optional.
	         */
		optionalMethods = YES;
		continue;
	      }
	    else if ([token isEqual: @"required"] == YES)
	      {
	        /* marking remaining methods as required.
	         */
		optionalMethods = NO;
		continue;
	      }
	    else if ([token isEqual: @"class"] == YES)
	      {
		/*
		 * Pre-declaration of one or more classes ... rather like a
		 * normal C statement, it ends with a semicolon.
		 */
		[self skipStatementLine];
	      }
	    else
	      {
		[self log: @"@method list with unknown directive '%@'", token];
		[self skipStatementLine];
	      }
	    DESTROY(comment);	// Don't want this.
	    break;

	  case '#':
	    /*
	     * Some preprocessor directive ... must be on one line ... skip
	     * past it and delete any comment accumulated while doing so.
	     */
	    [self parsePreprocessor];
	    DESTROY(comment);
	    break;

	  default:
	    /*
	     * Some statement other than a method ... skip and delete comments.
	     */
	    if (flag == YES)
	      {
		[self log: @"interface with bogus line ... we expect methods"];
		[self skipStatementLine];
	      }
	    else
	      {
		pos--;
		[self parseDeclaration];
	      }
	    DESTROY(comment);	// Don't want this.
	    break;
	}
    }

  [self log: @"method list prematurely ended"];
  return nil;
}

- (NSString*) parseMethodType
{
  unichar	*start;
  unichar	*ptr;
  unsigned	nest = 0;

  pos++;
  if ([self parseSpace] >= length)
    {
      return nil;
    }
  ptr = start = &buffer[pos];
  while (pos < length)
    {
      unichar	c = buffer[pos++];

      if (c == '(')
	{
	  /*
	   * Remove any whitespace before an opening bracket.
	   */
	  if (ptr > start && ptr[-1] == ' ')
	    {
	      ptr--;
	    }
	  *ptr++ = '(';
	  nest++;
	}
      else if (c == ')')
	{
	  /*
	   * Remove any whitespace before a closing bracket.
	   */
	  if (ptr > start && ptr[-1] == ' ')
	    {
	      ptr--;
	    }
	  if (nest > 0)
	    {
	      *ptr++ = ')';
	      nest--;
	    }
	  else
	    {
	      break;
	    }
	}
      else if ([spacenl characterIsMember: c] == NO)
	{
	  /*
	   * If this character is not part of a name, and the previous
	   * character written was a space, we know we can get rid of
	   * the space to standardise the type format to use a minimal
	   * number of spaces.
	   */
	  if (ptr > start && ptr[-1] == ' ')
	    {
	      if ([identifier characterIsMember: c] == NO)
		{
		  ptr--;
		}
	    }
	  *ptr++ = c;
	}
      else
	{
	  /*
	   * Don't retain whitespace if we know we don't need it
	   * because the previous character was not part of a name.
	   */
	  if (ptr > start && [identifier characterIsMember: ptr[-1]] == YES)
	    {
	      *ptr++ = ' ';
	    }
	}
    }

  if ([self parseSpace] >= length)
    {
      return nil;
    }

  /*
   * Strip trailing space ... leading space we never copied in the
   * first place.
   */
  if (ptr > start && [spacenl characterIsMember: ptr[-1]] == YES)
    {
      ptr--;
    }

  if (ptr > start)
    {
      return [NSString stringWithCharacters: start length: ptr - start];
    }
  else
    {
      return nil;
    }
}

/**
 * Parse a preprocessor statement, handling preprocessor
 * conditionals in a rudimentary way.  We keep track of the
 * level of conditional nesting, and we also track the use of
 * #ifdef and #ifndef with some well-known constants to tell
 * us which standards are currently supported.
 */
- (unsigned) parsePreprocessor
{
  [self parseSpace: spaces];
  if (pos < length && buffer[pos] != '\n')
    {
      NSString	*directive = [self parseIdentifier];

      if ([directive isEqual: @"define"] == YES && inHeader == YES)
        {
	  NSMutableDictionary		*defn;

	  defn = [self parseMacro];
	  if (defn != nil)
	    {
	      NSMutableDictionary	*dict = [info objectForKey: @"Macros"];
	      NSString			*name = [defn objectForKey: @"Name"];
	      NSMutableDictionary	*odef;

	      odef = [dict objectForKey: name];
	      if (odef == nil)
		{
		  if (dict == nil)
		    {
		      dict = [[NSMutableDictionary alloc] initWithCapacity: 8];
		      [info setObject: dict forKey: @"Macros"];
		      RELEASE(dict);
		    }
		  [dict setObject: defn forKey: name];
		}
	      else
		{
		  NSString	*oc = [odef objectForKey: @"Comment"];
		  NSString	*nc = [defn objectForKey: @"Comment"];

		  /*
		   * If the old comment from the header parsing is
		   * the same as the new comment from the source
		   * parsing, assume we parsed the same file as both
		   * source and header ... otherwise append the new
		   * comment.
		   */
		  if ([oc isEqual: nc] == NO)
		    {
		      [self appendComment: nc to: odef];
		    }
		}
	    }
	}
      else if ([directive isEqual: @"endif"] == YES)
	{
	  if ([ifStack count] <= 1)
	    {
	      [self log: @"Unexpected #endif (no matching #if)"];
	    }
	  else
	    {
	      [ifStack removeLastObject];
	    }
	}
      else if ([directive isEqual: @"elif"] == YES)
	{
	  if ([ifStack count] <= 1)
	    {
	      [self log: @"Unexpected #else (no matching #if)"];
	    }
	  else
	    {
	      [ifStack removeLastObject];
	      [ifStack addObject: [ifStack lastObject]];
	    }
	}
      else if ([directive isEqual: @"else"] == YES)
	{
	  if ([ifStack count] <= 1)
	    {
	      [self log: @"Unexpected #else (no matching #if)"];
	    }
	  else
	    {
	      [ifStack removeLastObject];
	      [ifStack addObject: [ifStack lastObject]];
	    }
	}
      else if ([directive isEqual: @"if"] == YES)
	{
	  NSMutableDictionary	*top;
	  NSString		*arg;
	  BOOL			hadOstep = NO;
	  BOOL			hadGstep = NO;

	  top = [[ifStack lastObject] mutableCopy];

	  while ((arg = [self parseIdentifier]) != nil)
	  {
	    BOOL	openstep;
	    NSString	*ver;

	    if ([arg isEqual: @"OS_API_VERSION"] == YES)
	      {
		openstep = YES;
		if (hadOstep == YES)
		  {
		    [self log: @"multiple grouped OS_API_VERSION() calls"];
		    return [self skipRemainderOfLine];
		  }
		hadOstep = YES;
		[top removeObjectForKey: @"ovadd"];
		[top removeObjectForKey: @"ovdep"];
		[top removeObjectForKey: @"ovrem"];
	      }
	    else if ([arg isEqual: @"GS_API_VERSION"] == YES)
	      {
		openstep = NO;
		if (hadGstep == YES)
		  {
		    [self log: @"multiple grouped GS_API_VERSION() calls"];
		    return [self skipRemainderOfLine];
		  }
		hadGstep = YES;
		[top removeObjectForKey: @"gvadd"];
		[top removeObjectForKey: @"gvdep"];
		[top removeObjectForKey: @"gvrem"];
	      }
	    else
	      {
		break;
	      }

	    [self parseSpace: spaces];
	    if (pos < length && buffer[pos] == '(')
	      {
		pos++;
	      }
	    ver = [self parseVersion];
	    if ([ver length] == 0)
	      {
		ver = @"1.0.0";
	      }
	    if (openstep)
	      {
		[top setObject: ver forKey: @"ovadd"];
	      }
	    else
	      {
		[top setObject: ver forKey: @"gvadd"];
	      }

	    [self parseSpace: spaces];
	    if (pos < length && buffer[pos] == ',')
	      {
		pos++;
	      }
	    ver = [self parseVersion];
	    if ([ver length] == 0)
	      {
		ver = @"99.99.99";
	      }
	    if ([ver isEqualToString: @"99.99.99"] == NO)
	      {
		if (openstep)
		  {
		    [top setObject: ver forKey: @"ovrem"];
		  }
		else
		  {
		    [top setObject: ver forKey: @"gvrem"];
		  }
	      }

	    [self parseSpace: spaces];
	    if (pos < length && buffer[pos] == ',')
	      {
		pos++;
		ver = [self parseVersion];
		if ([ver length] == 0)
		  {
		    ver = @"99.99.99";
		  }
		if ([ver isEqualToString: @"99.99.99"] == NO)
		  {
		    if (openstep)
		      {
			[top setObject: ver forKey: @"ovdep"];
		      }
		    else
		      {
			[top setObject: ver forKey: @"gvdep"];
		      }
		  }
		[self parseSpace: spaces];
	      }

	    if (pos < length && buffer[pos] == ')')
	      {
		pos++;
	      }

	    [self parseSpace: spaces];
	    if (pos < length-1 && buffer[pos] == '&' && buffer[pos+1] == '&')
	      {
		pos += 2;
	      }
	    else
	      {
		break;	// may only join version macros with &&
	      }
	  }
	  [ifStack addObject: top];
	  RELEASE(top);
	}
      else if ([directive hasPrefix: @"if"] == YES)
	{
	  BOOL	isIfDef = [directive isEqual: @"ifdef"];

	  [self parseSpace: spaces];
	  if (pos < length && buffer[pos] != '\n')
	    {
	      NSMutableDictionary	*top;
	      NSString			*arg;

	      top = [[ifStack lastObject] mutableCopy];
	      arg = [self parseIdentifier];
	      if ([arg isEqual: @"NO_GNUSTEP"] == YES)
		{
		  if (isIfDef == YES)
		    {
		      [self log: @"Unexpected #ifdef NO_GNUSTEP (nonsense)"];
		    }
		  else
		    {
		      [top removeObjectForKey: @"MacOS-X"];
		      [top setObject: @"NotMacOS-X" forKey: @"NotMacOS-X"];
		      [top removeObjectForKey: @"OpenStep"];
		      [top setObject: @"NotOpenStep" forKey: @"NotOpenStep"];
		    }
		}
	      else if ([arg isEqual: @"STRICT_MACOS_X"] == YES)
		{
		  if (isIfDef == YES)
		    {
		      [top removeObjectForKey: @"NotMacOS-X"];
		      [top setObject: @"MacOS-X" forKey: @"MacOS-X"];
		    }
		  else
		    {
		      [top removeObjectForKey: @"MacOS-X"];
		      [top setObject: @"NotMacOS-X" forKey: @"NotMacOS-X"];
		    }
		}
	      else if ([arg isEqual: @"STRICT_OPENSTEP"] == YES)
		{
		  if (isIfDef == YES)
		    {
		      [top removeObjectForKey: @"NotOpenStep"];
		      [top setObject: @"OpenStep" forKey: @"OpenStep"];
		    }
		  else
		    {
		      [top removeObjectForKey: @"OpenStep"];
		      [top setObject: @"NotOpenStep" forKey: @"NotOpenStep"];
		    }
		}
	      [ifStack addObject: top];
	      RELEASE(top);
	    }
	}
    }
  return [self skipRemainderOfLine];
}

- (NSMutableDictionary*) parseProtocol
{
  NSString		*name;
  NSDictionary		*methods = nil;
  NSMutableDictionary	*dict;
  NSMutableDictionary	*d;
  CREATE_AUTORELEASE_POOL(arp);

  dict = [[NSMutableDictionary alloc] initWithCapacity: 8];

  /*
   * Record any protocol documentation for this protocol
   */
  if (comment != nil)
    {
      [dict setObject: comment forKey: @"Comment"];
      DESTROY(comment);
    }

  if ((name = [self parseIdentifier]) == nil
    || [self parseSpace] >= length)
    {
      [self log: @"protocol with bad name"];
      goto fail;
    }

  /*
   * If there is a comma, this must be a forward declaration of a list
   * of protocols ... so we can ignore it.  Otherwise, if we found a
   * semicolon, we have a single forward declaration to ignore.
   */
  if (pos < length && (buffer[pos] == ',' || buffer[pos] == ';'))
    {
      [self skipStatement];
      return nil;
    }

  [dict setObject: name forKey: @"Name"];
  [self setStandards: dict];
  DESTROY(unitName);
  unitName = [[NSString alloc] initWithFormat: @"(%@)", name];

  /*
   * Protocols may themselves conform to protocols.
   */
  if (buffer[pos] == '<')
    {
      NSArray	*protocols = [self parseProtocolList];

      if (protocols == nil)
	{
	  goto fail;
	}
      else if ([protocols count] > 0)
	{
	  [dict setObject: protocols forKey: @"Protocols"];
	}
    }
  [dict setObject: @"protocol" forKey: @"Type"];

  methods = [self parseMethodsAreDeclarations: YES];
  if (methods != nil && [methods count] > 0)
    {
      NSEnumerator		*e = [methods objectEnumerator];
      NSMutableDictionary	*m;

      /*
       * Mark methods as implemented because protocol methods have no
       * implementation separate from their declaration.
       */
      while ((m = [e nextObject]) != nil)
      	{
	  [m setObject: @"YES" forKey: @"Implemented"];
	}
      [dict setObject: methods forKey: @"Methods"];
    }

  [dict setObject: declared forKey: @"Declared"];

  d = [info objectForKey: @"Protocols"];
  if (d == nil)
    {
      d = [[NSMutableDictionary alloc] initWithCapacity: 4];
      [info setObject: d forKey: @"Protocols"];
      RELEASE(d);
    }
  /*
   * A protocol has no separate implementation, so mark it as implemented.
   */
  [dict setObject: @"YES" forKey: @"Implemented"];
  [d setObject: dict forKey: unitName];

  // [self log: @"Found protocol %@", dict];

  DESTROY(unitName);
  DESTROY(comment);
  [arp drain];
  IF_NO_GC([dict autorelease];)
  return dict;

fail:
  DESTROY(unitName);
  DESTROY(comment);
  [arp drain];
  RELEASE(dict);
  return nil;
}

- (NSMutableArray*) parseProtocolList
{
  NSMutableArray	*protocols;
  NSString		*p;

  protocols = [NSMutableArray arrayWithCapacity: 2];
  pos++;
  while ((p = [self parseIdentifier]) != nil
    && [self parseSpace] < length)
    {
      if ([protocols containsObject: p] == NO)
	{
	  [protocols addObject: p];
	}
      if (buffer[pos] == ',')
	{
	  pos++;
	}
      else
	{
	  break;
	}
    }
  if (pos >= length || buffer[pos] != '>' || ++pos >= length
    || [self parseSpace] >= length || [protocols count] == 0)
    {
      [self log: @"bad protocol list"];
      return nil;
    }
  return protocols;
}

/**
 * Skip past any whitespace characters (as defined by the supplied set)
 * including comments.<br />
 * Calls parseComment if neccesary, ensuring that any documentation
 * in comments is appended to our 'comment' ivar.
 */
- (unsigned) parseSpace: (NSCharacterSet*)spaceSet
{
  BOOL		tryAgain;

  do
    {
      unsigned	start;

      tryAgain = NO;
      while (pos < length)
	{
	  unichar	c = buffer[pos];

	  if (c == '/')
	    {
	      unsigned	old = pos;

	      if ([self parseComment] > old)
		{
		  continue;	// Found a comment ... act as if it was a space.
		}
	      break;
	    }
	  if ([spaceSet characterIsMember: c] == NO)
	    {
	      break;		// Not whitespace ... done.
	    }
	  pos++;		// Step past space character.
	}
      start = pos;
      if (pos < length && [identifier characterIsMember: buffer[pos]] == YES)
	{
	  while (pos < length)
	    {
	      if ([identifier characterIsMember: buffer[pos]] == NO)
		{
		  NSString	*tmp;
		  NSString	*val;

		  tmp = [[NSString alloc] initWithCharacters: &buffer[start]
						      length: pos - start];
                  if ([tmp isEqualToString: @"NS_FORMAT_ARGUMENT"]
                    || [tmp isEqualToString: @"NS_FORMAT_FUNCTION"])
                    {
                      /* These macros need to be skipped as they appear inside
                       * method declarations.
                       */
                      val = @"";
                      [self skipSpaces];
                      [self skipBlock];
                    }
                  else
                    {
                      val = [wordMap objectForKey: tmp];
                    }
		  RELEASE(tmp);
		  if (val == nil)
		    {
		      pos = start;	// No mapping found
		    }
		  else if ([val length] > 0)
		    {
		      if ([val isEqualToString: @"//"] == YES)
			{
			  [self skipToEndOfLine];
			  tryAgain = YES;
			}
		      else
			{
			  pos = start;	// Not mapped to a comment.
			}
		    }
		  else
		    {
		      tryAgain = YES;	// Identifier ignored.
		    }
		  break;
		}
	      pos++;
	    }
	}
    }
  while (tryAgain == YES);

  return pos;
}

- (unsigned) parseSpace
{
  return [self parseSpace: spacenl];
}

- (NSString*) parseVersion
{
  static NSDictionary   *known = nil;
  unsigned	        i;
  NSString	        *str;
  NSString	        *tmp;

  while (pos < length && [spaces characterIsMember: buffer[pos]] == YES)
    {
      pos++;
    }
  if (pos >= length || buffer[pos] == '\n')
    {
      return nil;
    }
  if (!isdigit(buffer[pos]))
    {
      str = [self parseIdentifier];
    }
  else
    {
      i = pos;
      while (pos < length)
	{
	  if (!isdigit(buffer[pos]))
	    {
	      break;
	    }
	  pos++;
	}
      str = [NSString stringWithCharacters: &buffer[i] length: pos - i];
    }

  if (nil == known)
    {
      known = [[NSDictionary alloc] initWithObjectsAndKeys:
	@"0", @"GS_API_NONE",
	@"999999", @"GS_API_LATEST",
	@"10000", @"GS_API_OSSPEC",
	@"40000", @"GS_API_OPENSTEP",
	@"100000", @"GS_API_MACOSX",
	@"100100", @"MAC_OS_X_VERSION_10_1",
	@"100200", @"MAC_OS_X_VERSION_10_2",
	@"100300", @"MAC_OS_X_VERSION_10_3",
	@"100400", @"MAC_OS_X_VERSION_10_4",
	@"100500", @"MAC_OS_X_VERSION_10_5",
	@"100600", @"MAC_OS_X_VERSION_10_6",
	@"100700", @"MAC_OS_X_VERSION_10_7",
	@"100800", @"MAC_OS_X_VERSION_10_8",
	@"100800", @"MAC_OS_X_VERSION_10_8",
        nil];
    }
  tmp = [known objectForKey: str];
  if (nil != tmp)
    {
      str = tmp;
    }

  i = [str intValue];
  return [NSString stringWithFormat: @"%d.%d.%d",
    i/10000, (i/100)%100, i%100];
}

- (void) reset
{
  [source removeAllObjects];
  [info removeAllObjects];
  haveOutput = NO;
  haveSource = NO;
  DESTROY(declared);
  DESTROY(comment);
  DESTROY(fileName);
  DESTROY(unitName);
  DESTROY(itemName);
  DESTROY(lines);
  buffer = 0;
  length = 0;
  pos = 0;
}

/**
 * Set the name of the file in which classes are to be documented as
 * being declared.  The default value of this is the last part of the
 * path of the source file being parsed.
 */
- (void) setDeclared: (NSString*)name
{
  ASSIGN(declared, name);
}

/**
 * This method is used to enable (or disable) documentation of all
 * instance variables.  If it is turned off, only those instance
 * variables that are explicitly declared 'public' or 'protected'
 * will be documented.
 */
- (void) setDocumentAllInstanceVariables: (BOOL)flag
{
  documentAllInstanceVariables = flag;
}

/**
 * This method is used to enable (or disable) documentation of instance
 * variables.  If it is turned off, instance variables will not be documented.
 */
- (void) setDocumentInstanceVariables: (BOOL)flag
{
  documentInstanceVariables = flag;
}

/**
 * Turn on or off parsing of preprocessor conditional compilation info
 * indicating the standards complied with.  When this is turned on, we
 * assume that all standards are complied with by default.<br />
 * You should only turn this on while parsing the GNUstep source code.
 */
- (void) setGenerateStandards: (BOOL)flag
{
  if (flag == YES)
    {
      [ifStack replaceObjectAtIndex: 0 withObject:
	[NSDictionary dictionaryWithObjectsAndKeys:
	@"OpenStep", @"OpenStep",
	@"MacOS-X", @"MacOS-X",
	@"GNUstep", @"GNUstep",
	nil]];
    }
  standards = flag;
}

/**
 * Store the current standards information derived from preprocessor
 * conditionals in the supplied dictionary ... this will be used by
 * the AGSOutput class to put standards markup in the gsdoc output.
 */
- (void) setStandards: (NSMutableDictionary*)dict
{
  if (standards == YES)
    {
      NSDictionary	*top = [ifStack lastObject];

      if ([top count] > 0)
	{
	  NSString	*vInfo = nil;
	  NSString	*gvadd = [top objectForKey: @"gvadd"];
	  NSString	*ovadd = [top objectForKey: @"ovadd"];

	  if (ovadd != nil || gvadd != nil)
	    {
	      NSMutableString	*m = [NSMutableString stringWithCapacity: 64];
	      NSString		*s;

	      if (ovadd != nil)
		{
		  [m appendFormat: @" ovadd=\"%@\"", ovadd];
		  if ((s = [top objectForKey: @"ovdep"]) != nil)
		    {
		      [m appendFormat: @" ovdep=\"%@\"", s];
		    }
		  if ((s = [top objectForKey: @"ovrem"]) != nil)
		    {
		      [m appendFormat: @" ovrem=\"%@\"", s];
		    }
		}
	      if (gvadd != nil)
		{
		  [m appendFormat: @" gvadd=\"%@\"", gvadd];
		  if ((s = [top objectForKey: @"gvdep"]) != nil)
		    {
		      [m appendFormat: @" gvdep=\"%@\"", s];
		    }
		  if ((s = [top objectForKey: @"gvrem"]) != nil)
		    {
		      [m appendFormat: @" gvrem=\"%@\"", s];
		    }
		}
	      vInfo = m;
	    }
	  else if ([top objectForKey: @"NotOpenStep"]
	    && [top objectForKey: @"NotMacOS-X"])
	    {
	      vInfo = @" gvadd=\"0.0.0\"";	// GNUstep
	    }
	  else if ([top objectForKey: @"NotOpenStep"]
	    && ![top objectForKey: @"NotMacOS-X"])
	    {
	      vInfo = @" ovadd=\"10.0.0\"";	// MacOS-X
	    }
	  else if (![top objectForKey: @"NotOpenStep"]
	    && [top objectForKey: @"NotMacOS-X"])
	    {
	      vInfo = @" ovadd=\"1.0.0\" ovrem=\"4.0.0\"";	// OpenStep only
	    }
	  else if ([top objectForKey: @"OpenStep"]
	    && ![top objectForKey: @"NotMacOS-X"])
	    {
	      vInfo = @" ovadd=\"1.0.0\"";	// OpenStep
	    }
	  if (vInfo != nil)
	    {
	      [dict setObject: vInfo forKey: @"Versions"];
	    }
	}
    }
}

/**
 * Sets up a dictionary used for mapping identifiers/keywords to other
 * words.  This is used to help cope with cases where C preprocessor
 * definitions are confusing the parsing process.
 */
- (void) setWordMap: (NSDictionary*)map
{
  ASSIGNCOPY(wordMap, map);
}

/**
 * Read in the file to be parsed and store it in a temporary unicode
 * buffer.  Perform basic transformations on the buffer to simplify
 * the parsing process later - including stripping out of escaped
 * end-of-line sequences.  Create mapping information to convert
 * positions in the new character buffer to line numbers in the
 * original data (for logging purposes).
 */
- (void) setupBuffer
{
  NSString		*contents;
  NSMutableData		*data;
  unichar		*end;
  unichar		*inptr;
  unichar		*outptr;
  NSMutableArray	*a;
  CREATE_AUTORELEASE_POOL(arp);

  contents = [NSString stringWithContentsOfFile: fileName];
  length = [contents length];
  data = [[NSMutableData alloc] initWithLength: length * sizeof(unichar)];
  buffer = [data mutableBytes];
  [contents getCharacters: buffer];
  outptr = buffer;
  end = &buffer[length];

  a = [NSMutableArray arrayWithCapacity: 1024];
  for (inptr = buffer; inptr < end; outptr++, inptr++)
    {
      unichar	c = *inptr;

      *outptr = c;

      /*
       * Perform ansi trigraph substitution.
       * Don't know why I bothered ... will probably never be used.
       */
      if (c == '?' && (inptr < end - 2) && inptr[1] == '?')
	{
	  BOOL	changed = YES;

	  switch (inptr[2])
	    {
	      case '=':		*outptr = '#';	break;
	      case '/':		*outptr = '\\';	break;
	      case '\'':	*outptr = '^';	break;
	      case '(':		*outptr = '[';	break;
	      case ')':		*outptr = ']';	break;
	      case '!':		*outptr = '|';	break;
	      default:		*outptr = '?'; changed = NO; break;
	    }
	  if (changed == YES)
	    {
	      inptr += 2;
	    }
	}
      else if (c == '\\')
	{
	  /*
	   * Backslash-end-of-line sequences are removed.
           */
	  if (inptr < end - 1)
	    {
	      if (inptr[1] == '\n')
		{
		  inptr++;
		  outptr--;
		  [a addObject: [NSNumber numberWithInt: outptr - buffer]];
		}
	      else if (inptr[1] == '\r')
		{
		  inptr++;
		  outptr--;
		  if (inptr[1] == '\n')
		    {
		      inptr++;
		    }
		  [a addObject: [NSNumber numberWithInt: outptr - buffer]];
		}
	    }
	}
      else if (c == '\r')
	{
	  /*
	   * Convert cr-fl or single cr to single lf
	   */
	  if (inptr < end - 1)
	    {
	      if (inptr[1] == '\n')
		{
		  inptr++;
		}
	      *outptr = '\n';
	    }
	  else
	    {
	      outptr--;		// Ignore trailing carriage return.
	    }
	  [a addObject: [NSNumber numberWithInt: outptr - buffer]];
	}
      else if (c == '\n')
	{
	  [a addObject: [NSNumber numberWithInt: outptr - buffer]];
	}
    }
  length = outptr - buffer;
  [data setLength: length*sizeof(unichar)];
  buffer = [data mutableBytes];
  pos = 0;
  ASSIGN(lines, [NSArray arrayWithArray: a]);
  [arp drain];
  IF_NO_GC([data autorelease];)
}

/**
 * Skip until we encounter an ']' marking the end of an array.
 * Expect the current character position to be pointing to the
 * '[' at the start of an array.
 */
- (unsigned) skipArray
{
  pos++;
  while ([self parseSpace] < length)
    {
      unichar	c = buffer[pos++];

      switch (c)
	{
	  case '#':		// preprocessor directive.
	    [self parsePreprocessor];
	    break;

	  case '\'':
	  case '"':
	    pos--;
	    [self skipLiteral];
	    break;

	  case '[':
	    pos--;
	    [self skipArray];
	    break;

	  case ']':
	    return pos;
        }
    }
  return pos;
}

/**
 * Skip a bracketed block.
 * Expect the current character position to be pointing to the
 * bracket at the start of a block.
 */
- (unsigned) skipBlock
{
  return [self skipBlock: 0];
}

- (unsigned) skipBlock: (BOOL*)isEmpty
{
  unichar	term = '}';
  BOOL		empty = YES;

  if (buffer[pos] == '(')
    {
      term = ')';
    }
  else if (buffer[pos] == '[')
    {
      term = ']';
    }
  pos++;
  while ([self parseSpace] < length)
    {
      unichar	c = buffer[pos++];

      switch (c)
	{
	  case '#':		// preprocessor directive.
	    [self parsePreprocessor];
	    break;

	  case '\'':
	  case '"':
	    empty = NO;
	    pos--;
	    [self skipLiteral];
	    break;

	  case '{':
	    empty = NO;
	    pos--;
	    [self skipBlock];
	    break;

	  case '(':
	    empty = NO;
	    pos--;
	    [self skipBlock];
	    break;

	  case '[':
	    empty = NO;
	    pos--;
	    [self skipBlock];
	    break;

	  default:
	    if (c == term)
	      {
		if (isEmpty != 0)
		  {
		    *isEmpty = empty;
		  }
		return pos;
	      }
	    empty = NO;
        }
    }
  if (isEmpty != 0)
    {
      *isEmpty = empty;
    }
  return pos;
}

- (unsigned) skipLiteral
{
  unichar	term = buffer[pos++];

  while (pos < length)
    {
      unichar	c = buffer[pos++];

      if (c == '\\')
	{
	  pos++;
	}
      else if (c == term)
	{
	  break;
	}
    }
  return pos;
}

- (unsigned) skipRemainderOfLine
{
  while (pos < length)
    {
      if (buffer[pos++] == '\n')
	{
	  break;
	}
    }
  return pos;
}

- (unsigned) skipSpaces
{
  while (pos < length)
    {
      unichar	c = buffer[pos];

      if ([spaces characterIsMember: c] == NO)
	{
	  break;
	}
      pos++;
    }
  return pos;
}

/**
 * Skip until we encounter a semicolon or closing brace.
 * Strictly speaking, we don't skip all statements that way,
 * since we only skip part of an if...else statement.
 */
- (unsigned) skipStatement
{
  while ([self parseSpace] < length)
    {
      unichar	c = buffer[pos++];

      switch (c)
	{
	  case '#':		// preprocessor directive.
	    [self parsePreprocessor];
	    break;

	  case '\'':
	  case '"':
	    pos--;
	    [self skipLiteral];
	    break;

	  case '{':
	    pos--;
	    [self skipBlock];
	    return pos;

	  case ';':
	    return pos;		// At end of statement

	  case '}':
	    [self log: @"Argh ... read '}' when looking for ';'"];
	    return --pos;	// No statement to skip.
	    break;
        }
    }
  return pos;
}

/**
 * Special method to skip a statement and up to the end of the last
 * line it was on, discarding any comments so they don't get used by
 * the next construct that actually needs documenting.
 */
- (unsigned) skipStatementLine
{
  [self skipStatement];
  if (buffer[pos-1] == ';' || buffer[pos-1] == '}')
    {
      [self skipRemainderOfLine];
    }
  DESTROY(comment);
  return pos;
}

- (unsigned) skipToEndOfLine
{
  while (pos < length)
    {
      if (buffer[pos++] == '\n')
	{
	  pos--;
	  break;
	}
    }
  return pos;
}

/**
 * Skip until we encounter an '@end' marking the end of an interface,
 * implementation, or protocol.
 */
- (unsigned) skipUnit
{
  while ([self parseSpace] < length)
    {
      unichar	c = buffer[pos++];

      switch (c)
	{
	  case '#':		// preprocessor directive.
	    [self parsePreprocessor];
	    break;

	  case '\'':
	  case '"':
	    pos--;
	    [self skipLiteral];
	    break;

	  case '@':
	    [self parseSpace];
	    if (pos < length - 3)
	      {
		if (buffer[pos] == 'e' && buffer[pos+1] == 'n'
		  && buffer[pos+2] == 'd')
		  {
		    pos += 3;
		    return pos;
		  }
	      }
	    break;
        }
    }
  return pos;
}

- (NSMutableArray*) sources
{
  return AUTORELEASE([source mutableCopy]);
}
@end

