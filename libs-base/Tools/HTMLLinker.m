/** The GNUstep HTML Linker

   <title>HTMLLinker. A tool to fix up href references in html files</title>
   Copyright (C) 2002,2007 Free Software Foundation, Inc.

   Written by: Nicola Pero <nicola@brainstorm.co.uk>
   Date: January 2002

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

/*
 * See the HTMLLinker.html file for documentation on how to use the tool.
 */

#import	"common.h"

#import	"Foundation/NSArray.h"
#import	"Foundation/NSAutoreleasePool.h"
#import	"Foundation/NSFileManager.h"
#import	"Foundation/NSPathUtilities.h"
#import	"Foundation/NSProcessInfo.h"
#import	"Foundation/NSUserDefaults.h"

/* For convenience, cached for the whole tool.  */

/* [NSFileManager defaultManager]  */
static NSFileManager *fileManager = nil;

/* [[NSFileManager defaulManager] currentDirectoryPath]  */
static NSString *currentPath = nil;

static int verbose = 0;

/* Enumerate all .html (or .htmlink) files in a directory and
   subdirectories.  */
@interface HTMLDirectoryEnumerator : NSEnumerator
{
  NSDirectoryEnumerator *e;
  NSString *basePath;
  BOOL looksForHTMLLinkFiles;
  BOOL returnsAbsolutePaths;
}

- (id)initWithBasePath: (NSString *)path;

- (void)setReturnsAbsolutePaths: (BOOL)flag;

- (void)setLooksForHTMLLinkFiles: (BOOL)flag;

@end

@implementation HTMLDirectoryEnumerator : NSEnumerator

- (id)initWithBasePath: (NSString *)path
{
  ASSIGN (e, [fileManager enumeratorAtPath: path]);
  ASSIGN (basePath, path);
  return [super init];
}

- (void)dealloc
{
  RELEASE (e);
  RELEASE (basePath);
  [super dealloc];
}

- (void)setReturnsAbsolutePaths: (BOOL)flag
{
  returnsAbsolutePaths = flag;
}

- (void)setLooksForHTMLLinkFiles: (BOOL)flag
{
  looksForHTMLLinkFiles = YES;
}

- (id)nextObject
{
  NSString *s;
  
  while ((s = [e nextObject]) != nil)
    {
      BOOL found = NO;
      NSString *extension = [s pathExtension];

      if (looksForHTMLLinkFiles)
	{
	  if ([extension isEqualToString: @"htmlink"])
	    {
	      found = YES;
	    }
	}
      else if ([extension isEqualToString: @"html"]  
	       || [extension isEqualToString: @"HTML"]
	       || [extension isEqualToString: @"htm"]  
	       || [extension isEqualToString: @"HTM"])
	{
	  found = YES;
	}
      
      if ([[[e fileAttributes] fileType] isEqual: NSFileTypeDirectory]
      	  && verbose)
        {
	  GSPrintf(stdout, @"    traversing %@\n", s);
	}

      if (found)
	{
	  if (returnsAbsolutePaths)
	    {
	      /* NSDirectoryEnumerator returns the relative path, we
		 return the absolute.  */
	      return [basePath stringByAppendingPathComponent: s];
	    }
	  else
	    {
	      return s;
	    }
	}
    }

  return nil;
}

@end

/* The HTMLLinker class is very simple and is the core of the linker.
   It just keeps a relocation, and is able to fixup a link by using
   the relocation table.  */
@interface HTMLLinker : NSObject
{
  BOOL warn;
  BOOL hasPathMappings;
  NSMutableDictionary *pathMappings;
  NSMutableDictionary *relocationTable;
}

- (id)initWithWarnFlag: (BOOL)v;

- (void)registerRelocationFile: (NSString *)pathOnDisk;

- (void)registerDestinationFile: (NSString *)pathOnDisk;

/* Register a new path mapping.  */
- (void)registerPathMappings: (NSDictionary *)dict;

/* Resolve the link 'link' by fixing it up using the relocation table.
   Return the resolved link.  'logFile' is only used to print error
   messages.  It is the file in which the link is originally found; if
   there is problem resolving the link, the warning message printed
   out states that the problem is in file 'logFile'.  */
- (NSString *)resolveLink: (NSString *)link
		  logFile: (NSString *)logFile;

@end

/* All the parsing code is in the following class.  It's not a real
   parser in the sense that it is just performing its minimal duty in
   the quickest possible way, so calling this a parser is a bit of a
   exaggeration ... this code can run very quickly through an HTML
   string, extracting the <a name="yyy"> tags or fixing up the <a
   href="xxx" rel="dynamical"> tags.  No more HTML parsing than this
   is done.  Remarkably, this does not need XML support in the base
   library, so you can use the HTML linker on any system.  This class
   was written in order to perform its trivial, mechanical duty /very
   fast/.  You want to be able to run the linker often and on a lot of
   files and still be happy.  FIXME - Need to implement support for
   newer HTML where you can use id="name" in any tag.  */
@interface HTMLParser : NSObject
{
  /* The HTML code that we work on.  */
  unichar *chars;
  unsigned length;
}
/* Init with some HTML code to parse.  */
- (id)initWithCode: (NSString *)HTML;

/* Extract all the <a name="xxx"> tags from the HTML code, and return
   a list of them.  */
- (NSArray *)names;

/* Fix up all the links in the HTML code by feeding each of them to
   the provided HTMLLinker; return the fixed up HTML code.  If
   linksMarker is nil, attempts to fix up all links in the HTML code;
   if it is not-nil, only attempt to fixup links with rel=marker.
   logFile is the file we are fixing up; it's only used when a warning
   is issued because there is problem in the linking - the warning
   message is displayed as being about links in the file logFile.  */
- (NSString *)resolveLinksUsingHTMLLinker: (HTMLLinker *)linker
				  logFile: (NSString *)logFile
			      linksMarker: (NSString *)marker;
@end


@implementation HTMLParser

- (id)initWithCode: (NSString *)HTML
{
  length = [HTML length];
  chars = malloc (sizeof(unichar) * length);
  [HTML getCharacters: chars];

  return [super init];
}

- (void)dealloc
{
  free (chars);
  [super dealloc];
}

- (NSArray *)names
{
  NSMutableArray *names = AUTORELEASE ([NSMutableArray new]);
  unsigned i = 0;

  while (i + 3 < length)
    {
      /* We ignore anything except stuff which begins with "<a ". */
      if ((chars[i] == '<') 
	  && (chars[i + 1] == 'A'  ||  chars[i + 1] == 'a')
	  && (chars[i + 2] == ' '))
	{
	  /* Ok - we got the '<a ' tag, now parse it ... we're
             searching for a name attribute.  */
	  NSString *name = nil;

	  i += 3;
	  
	  while (1)
	    {
	      /* A marker for the start of strings.  */
	      unsigned s;

	      /* If this is not a 'name' attribute, setting this to YES
		 cause us to ignore it and go on to the next one.  */
	      BOOL isNameAttribute = NO;

	      /* Read in an attribute, of the form xxx="yyy" or
                 xxx=yyy or similar, and save it if it is a name
                 attribute.  */

	      /* Skip spaces.  */
	      while (i < length  &&  (chars[i] == ' '
				      || chars[i] == '\n'
				      || chars[i] == '\r'
				      || chars[i] == '\t'))
		{ i++; }
	      
	      if (i == length) { break; }
     	      
	      /* Read the attribute.  */
	      s = i;
	      
	      while (i < length  &&  (chars[i] != ' '  
				      && chars[i] != '\n'
				      && chars[i] != '\r'
				      && chars[i] != '\t'
				      && chars[i] != '='
				      && chars[i] != '>'))
		{ i++; }

	      if (i == length) { break; }
	      if (chars[i] == '>') { break; }


	      /* I suppose i == s might happen if the file contains <a
                 ="nicola"> */
	      if (i != s)
		{
		  /* If name != nil we already found it so don't bother.  */
		  if (name == nil)
		    {
		      NSString *attribute;
		      
		      attribute = [NSString stringWithCharacters: &chars[s]  
					    length: (i - s)];
		      /* Lowercase name so that eg, HREF and href are the
			 same.  */
		      attribute = [attribute lowercaseString];
		      
		      if ([attribute isEqualToString: @"name"])
			{
			  isNameAttribute = YES;
			}
		    }
		}
	      
	      /* Skip spaces.  */
	      while (i < length  &&  (chars[i] == ' '
				      || chars[i] == '\n'
				      || chars[i] == '\r'
				      || chars[i] == '\t'))
		{ i++; }
	      
	      if (i == length) { break; }

	      /* Read the '='  */
	      if (chars[i] == '=')
		{ 
		  i++; 
		}
	      else
		{
		  /* No '=' -- go on with the next attribute.  */
		  continue; 
		}
	      
	      if (i == length) { break; }

	      /* Skip spaces.  */
	      while (i < length &&  (chars[i] == ' '
				     || chars[i] == '\n'
				     || chars[i] == '\r'
				     || chars[i] == '\t'))
		{ i++; }
	      
	      if (i == length) { break; }
     	      
	      /* Read the value.  */
	      if (chars[i] == '"')
		{
		  /* Skip the '"', then read up to a '"'.  */
		  i++;
		  if (i == length) { break; }
		  
		  s = i;

		  while (i < length   &&  (chars[i] != '"'))
		    { i++; }
		}
	      else if (chars[i] == '\'')
		{
		  /* Skip the '\'', then read up to a '\''.  */
		  i++;
		  if (i == length) { break; }
		  
		  s = i;
		  
		  while (i < length   &&  (chars[i] != '\''))
		    { i++; }
		}
	      else
		{
		  /* Read up to a space or '>'.  */
		  s = i;

		  while (i < length
			 &&  (chars[i] != ' '  
			      && chars[i] != '\n'
			      && chars[i] != '\r'
			      && chars[i] != '\t'
			      && chars[i] != '>'))
		    { i++; }
		}

	      if (name == nil  &&  isNameAttribute)
		{
		  if (i == s)
		    {
		      /* I suppose this might happen if the file
			 contains <a name=> */
		      name = @"";
		    }
		  else
		    {
		      name = [NSString stringWithCharacters: &chars[s]  
				       length: (i - s)];
		    }
		}
	    }

	  if (name != nil)
	    {
	      [names addObject: name];
	    }
	}
      i++;
    }

  return names;
}


- (NSString *)resolveLinksUsingHTMLLinker: (HTMLLinker *)linker
				  logFile: (NSString *)logFile
			      linksMarker: (NSString *)marker
{
  /* We represent the output as a linked list.  Each element in the
     linked list represents a string; concatenating all the strings in
     the linked list, you obtain the output.  The trick is that these
     strings in the linked list might actually be pointers inside the
     chars array ... we are never copying stuff from the chars array -
     just keeping pointers to substrings inside it - till we generate
     the final string at the end ... for speed and efficiency reasons
     of course.  */
  struct stringFragment
    {
      unichar *chars;
      unsigned length;
      BOOL needsFreeing;
      struct stringFragment *next;
    } *head, *tail;

  /* The index of the beginning of the last string fragment (the tail).  */
  unsigned tailIndex = 0;

  /* The temporary index.  */
  unsigned i = 0;

  /* The total number of chars in the output string.  We don't know
     this beforehand because each time we fix up a link, we might add
     or remove characters from the output.  We update
     totalNumberOfChars each time we close a stringFragment.  */
  unsigned totalNumberOfChars = 0;
  

  /* Initialize the linked list.  */
  head = malloc (sizeof (struct stringFragment));
  head->chars = chars;
  head->length = 0;
  head->needsFreeing = NO;
  head->next = NULL;

  /* The last string fragment is the first one at the beginning.  */
  tail = head;
  
  while (i + 3 < length)
    {
      /* We ignore anything except stuff which begins with "<a ". */
      if ((chars[i] == '<') 
	  && (chars[i + 1] == 'A'  ||  chars[i + 1] == 'a')
	  && (chars[i + 2] == ' '))
	{
	  /* Ok - we got the '<a ' tag, now parse it ... we're
             searching for a href and a rel attributes.  */
	  NSString *href = nil;
	  NSString *rel = nil;

	  /* We also need to keep track of where the href starts and
             where it ends, because we are going to replace it with a
             different one (the fixed up one) later on if we determine
             we should do it.  */
	  unsigned hrefStart = 0, hrefEnd = 0;

	  i += 3;
	  
	  while (1)
	    {
	      /* A marker for the start of strings.  */
	      unsigned s;

	      /* If this is an interesting (href/rel) attribute or
		 not, and which one.  */
	      BOOL isHrefAttribute = NO;
	      BOOL isRelAttribute = NO;

	      /* Read in an attribute, of the form xxx="yyy" or
                 xxx=yyy or similar, and save it if it is a name
                 attribute.  */

	      /* Skip spaces.  */
	      while (i < length  &&  (chars[i] == ' '
				      || chars[i] == '\n'
				      || chars[i] == '\r'
				      || chars[i] == '\t'))
		{ i++; }
	      
	      if (i == length) { break; }
     	      
	      /* Read the attribute.  */
	      s = i;
	      
	      while (i < length  &&  (chars[i] != ' '  
				      && chars[i] != '\n'
				      && chars[i] != '\r'
				      && chars[i] != '\t'
				      && chars[i] != '='
				      && chars[i] != '>'))
		{ i++; }

	      if (i == length) { break; }
	      if (chars[i] == '>') { break; }


	      /* I suppose i == s might happen if the file contains <a
                 ="nicola"> */
	      if (i != s)
		{
		  /* If href != nil && rel != nil we already found it
                     so don't bother.  */
		  if (href == nil  ||  rel == nil)
		    {
		      NSString *attribute;
		      
		      attribute = [NSString stringWithCharacters: &chars[s]  
					    length: (i - s)];
		      /* Lowercase name so that eg, HREF and href are the
			 same.  */
		      attribute = [attribute lowercaseString];
		      
		      if (href == nil 
			  && [attribute isEqualToString: @"href"])
			{
			  isHrefAttribute = YES;
			}
		      else if (rel == nil 
			       && [attribute isEqualToString: @"rel"])
			{
			  isRelAttribute = YES;
			}
		    }
		}
	      
	      /* Skip spaces.  */
	      while (i < length  &&  (chars[i] == ' '
				      || chars[i] == '\n'
				      || chars[i] == '\r'
				      || chars[i] == '\t'))
		{ i++; }
	      
	      if (i == length) { break; }

	      /* Read the '='  */
	      if (chars[i] == '=')
		{ 
		  i++; 
		}
	      else
		{
		  /* No '=' -- go on with the next attribute.  */
		  continue; 
		}
	      
	      if (i == length) { break; }

	      /* Skip spaces.  */
	      while (i < length &&  (chars[i] == ' '
				     || chars[i] == '\n'
				     || chars[i] == '\r'
				     || chars[i] == '\t'))
		{ i++; }
	      
	      if (i == length) { break; }
     	      
	      /* Read the value.  */
	      if (isHrefAttribute)
		{
		  /* Remeber that href starts here.  */
		  hrefStart = i;
		}	      

	      if (chars[i] == '"')
		{
		  /* Skip the '"', then read up to a '"'.  */
		  i++;
		  if (i == length) { break; }
		  
		  s = i;

		  while (i < length   &&  (chars[i] != '"'))
		    { i++; }

		  if (isHrefAttribute)
		    {
		      /* Remeber that href ends here.  We don't want
			 the ending " because we already insert those
			 by our own.  */
		      hrefEnd = i + 1;
		    }
		}
	      else if (chars[i] == '\'')
		{
		  /* Skip the '\'', then read up to a '\''.  */
		  i++;
		  if (i == length) { break; }
		  
		  s = i;
		  
		  while (i < length   &&  (chars[i] != '\''))
		    { i++; }

		  if (isHrefAttribute)
		    {
		      hrefEnd = i + 1;
		    }
		}
	      else
		{
		  /* Read up to a space or '>'.  */
		  s = i;

		  while (i < length
			 &&  (chars[i] != ' '  
			      && chars[i] != '\n'
			      && chars[i] != '\r'
			      && chars[i] != '\t'
			      && chars[i] != '>'))
		    { i++; }
		  if (isHrefAttribute)
		    {
		      /* We do want the ending space.  */
		      hrefEnd = i;
		    }
		}

	      if (i == length)
		{
		  break;
		}

	      if (hrefEnd >= length)
		{
		  hrefEnd = length - 1;
		}
	      
	      if (isRelAttribute)
		{
		  if (i == s)
		    {
		      /* I suppose this might happen if the file
			 contains <a rel=> */
		      rel = @"";
		    }
		  else
		    {
		      rel = [NSString stringWithCharacters: &chars[s]  
				       length: (i - s)];
		    }
		}

	      if (isHrefAttribute)
		{
		  if (i == s)
		    {
		      /* I suppose this might happen if the file
			 contains <a href=> */
		      href = @"";
		    }
		  else
		    {
		      href = [NSString stringWithCharacters: &chars[s]  
				       length: (i - s)];
		    }
		}
	    }
	  if (href != nil  &&  ((marker == nil)
				|| [rel isEqualToString: marker]))
	    {
	      /* Ok - fixup the link.  */
	      NSString *link;
	      struct stringFragment *s;

	      link = [linker resolveLink: href  logFile: logFile];

	      /* Add " before and after the link.  */
	      link = [NSString stringWithFormat: @"\"%@\"", link];
	      
	      /* Close the previous string fragment at hrefStart.  */
	      tail->length = hrefStart - tailIndex;

	      totalNumberOfChars += tail->length;

	      /* Insert immediately afterwards a string fragment containing
		 the fixed up link.  */
	      s = malloc (sizeof (struct stringFragment));
	      s->length = [link length];
	      
	      s->chars = malloc (sizeof(unichar) * s->length);
	      [link getCharacters: s->chars];
	      
	      s->needsFreeing = YES;
	      s->next = NULL;

	      tail->next = s;
	      tail = s;

	      totalNumberOfChars += tail->length;

	      /* Now prepare the new tail to start just after the end
                 of the original href in the original HTML code.  */
	      s = malloc (sizeof (struct stringFragment));
	      s->length = 0;
	      s->chars = &chars[hrefEnd];
	      s->needsFreeing = NO;
	      s->next = NULL;
	      tail->next = s;
	      tail = s;

	      tailIndex = hrefEnd;
	    }
	}
      i++;
    }

  /* Close the last open string fragment.  */
  tail->length = length - tailIndex;
  totalNumberOfChars += tail->length;

  /* Generate the output.  */
  {
    NSString *result;
    /* Allocate space for the whole output in a single chunk now that
       we know how big it should be.  */
    unichar *outputChars = malloc (sizeof(unichar) * totalNumberOfChars);
    unsigned j = 0;
    
    /* Copy into the output all the string fragments, destroying each
       of them as we go on.  */
    while (head != NULL)
      {
	struct stringFragment *s;
	
	memcpy (&outputChars[j], head->chars, 
		sizeof(unichar) * head->length);

	j += head->length;
	
	if (head->needsFreeing)
	  {
	    free (head->chars);
	  }
	
	s = head->next;
	free (head);
	head = s;
      }

    result = [NSString stringWithCharacters: outputChars
				     length: totalNumberOfChars];
    free(outputChars);
    return result;
  }
}

@end


@implementation HTMLLinker

- (id)initWithWarnFlag: (BOOL)v
{
  warn = v;
  relocationTable = [NSMutableDictionary new];
  pathMappings = [NSMutableDictionary new];
  return [super init];
}

- (void)dealloc
{
  RELEASE (relocationTable);
  RELEASE (pathMappings);
  [super dealloc];
}

- (void)registerRelocationFile: (NSString *)pathOnDisk
{
  /* We only accept absolute paths.  */
  if (![pathOnDisk isAbsolutePath])
    {
      pathOnDisk = [currentPath stringByAppendingPathComponent: pathOnDisk];
    }

  /* Check if it's a directory; if it is, enumerate all .htmlink files
     inside it, and add all of them.  */
  {
    BOOL isDir;
    
    if (![fileManager fileExistsAtPath: pathOnDisk  isDirectory: &isDir])
      {
	NSLog (@"Warning - relocation file '%@' not found - ignored",
	       pathOnDisk);
	return;
      }
    else
      {
	if (isDir)
	  {
	    HTMLDirectoryEnumerator *e;
	    NSString *filename;
	   
	    e = [HTMLDirectoryEnumerator alloc];
	    e = [[e initWithBasePath: pathOnDisk] autorelease];
	    [e setLooksForHTMLLinkFiles: YES];
	    [e setReturnsAbsolutePaths: YES];

	    while ((filename = [e nextObject]) != nil)
	      {
		[self registerRelocationFile: filename];
	      }
	    return;
	  }
      }
  }

  /* Now, read the mappings in the file.  */
  {
    NSString *file = [NSString stringWithContentsOfFile: pathOnDisk];
    NSString *path = [pathOnDisk stringByDeletingLastPathComponent];
    NSDictionary *d = [file propertyList];
    NSEnumerator *e = [d keyEnumerator];
    NSString *name;
    
    while ((name = [e nextObject]) != nil)
      {
	NSString *v = [d objectForKey: name];
	NSString *filePath;

	filePath = [path stringByAppendingPathComponent: v];
	
	if (hasPathMappings)
	  {
	    /* Manage pathMappings: try to match any of the
	       pathMappings against pathOnDisk, and perform the path
	       mapping if we can match.  */
	    NSEnumerator *en = [pathMappings keyEnumerator];
	    NSString *key;
	    while ((key = [en nextObject]))
	      {
		if ([filePath hasPrefix: key])
		  {
		    NSString *value = [pathMappings objectForKey: key];

		    filePath = [filePath substringFromIndex: [key length]];
		    filePath = [value stringByAppendingPathComponent: 
					filePath];
		    break;
		  }
	      }
	  }
	
	[relocationTable setObject: filePath  forKey: name];
      }
  }  
}


- (void)registerDestinationFile: (NSString *)pathOnDisk
{
  NSString *fullPath = pathOnDisk;

  /* We only accept absolute paths.  */
  if (![pathOnDisk isAbsolutePath])
    {
      pathOnDisk = [currentPath stringByAppendingPathComponent: pathOnDisk];
    }

  /* Check if it's a directory; if it is, enumerate all HTML files
     inside it, and add all of them.  */
  {
    BOOL isDir;
    
    if (![fileManager fileExistsAtPath: pathOnDisk  isDirectory: &isDir])
      {
	NSLog (@"Warning - destination file '%@' not found - ignored", 
	       pathOnDisk);
	return;
      }
    else
      {
	if (isDir)
	  {
	    HTMLDirectoryEnumerator *e;
	    NSString *filename;
	   
	    e = [HTMLDirectoryEnumerator alloc];
	    e = [[e initWithBasePath: pathOnDisk] autorelease];
	    [e setReturnsAbsolutePaths: YES];

	    while ((filename = [e nextObject]) != nil)
	      {
		[self registerDestinationFile: filename];
	      }
	    return;
	  }
      }
  }

  if (hasPathMappings)
    {
      /* Manage pathMappings: try to match any of the pathMappings
	 against pathOnDisk, and perform the path mapping if we can
	 match.  */
      NSEnumerator *e = [pathMappings keyEnumerator];
      NSString *key;
      
      while ((key = [e nextObject]))
	{
	  if ([pathOnDisk hasPrefix: key])
	    {
	      NSString *value = [pathMappings objectForKey: key];
	      
	      fullPath = [pathOnDisk substringFromIndex: [key length]];
	      fullPath = [value stringByAppendingPathComponent: fullPath];
	      break;
	    }
	}
    }

  /* Now, read all the names from the file.  */
  {
    NSString *file = [NSString stringWithContentsOfFile: pathOnDisk];
    HTMLParser *p = [[HTMLParser alloc] initWithCode: file];
    NSArray *names = [p names];
    unsigned i, count;
    
    RELEASE (p);

    count = [names count];
  
    for (i = 0; i < count; i++)
      {
	NSString *name = [names objectAtIndex: i];
	[relocationTable setObject: fullPath  forKey: name];
      }
  }
}

- (void)registerPathMappings: (NSDictionary *)dict
{
  NSEnumerator *e = [dict keyEnumerator];
  NSString *key;
  
  while ((key = [e nextObject])) 
    {
      NSString *value = [dict objectForKey: key];
      [pathMappings setObject: value  forKey: key];
    }
  hasPathMappings = YES;
}

- (NSString *)resolveLink: (NSString *)link
		  logFile: (NSString *)logFile
{
  NSString *fileLink;
  NSString *nameLink;
  NSString *relocatedFileLink;
  NSString *file;

  /* Do nothing if this is evidently *not* a dynamical link to fixup.  */
  if ([link hasPrefix: @"mailto:"] || [link hasPrefix: @"news:"])
    {
      return link;
    }
  
  {
    /* Break the link string into fileLink (everything which is before
       the `#'), and nameLink (everything which is after the `#', `#'
       not included).  For example, if link is
       'NSObject_Class.html#isa', then fileLink is
       'NSObject_Class.html' and nameLink is 'isa'.  */
  
    /* Look for the #.  */
    NSRange hashRange = [link rangeOfString: @"#"];
    
    if (hashRange.location == NSNotFound)
      {
	fileLink = link;
	nameLink = nil;
      }
    else
      {
	fileLink = [link substringToIndex: hashRange.location];

	if (hashRange.location + 1 < [link length])
	  {
	    nameLink = [link substringFromIndex: (hashRange.location + 1)];
	  }
	else
	  {
	    nameLink = nil;
	  }
      }
  }
  
  /* Now lookup nameLink.  */
  
  /* If it's "", it is not something we can fixup.  */
  if (nameLink == nil  ||  [nameLink isEqualToString: @""])
    {
      relocatedFileLink = fileLink;
    }
  else
    {
      /* Now simply look it up in our relocation table.  */
      file = [relocationTable objectForKey: nameLink];
      
      /* Not found - leave it unfixed.  */
      if (file == nil)
	{
	  if (warn && [fileLink length] > 0)
	    {
	      GSPrintf(stderr, @"%@: Unresolved reference to '%@'\n", 
			    logFile, nameLink);
	    }
	  
	  relocatedFileLink = fileLink;
	}
      else
	{
	  relocatedFileLink = file;
	}
    }
  
  /* Now build up the final relocated link, and return it.  */
  if (nameLink != nil)
    {
      return [NSString stringWithFormat: @"%@#%@", relocatedFileLink,
		       nameLink];
    }
  else
    {
      return relocatedFileLink;
    }
}

@end

static NSDictionary *
build_relocation_table_for_directory (NSString *dir)
{
  BOOL isDir;

  if (verbose)
    GSPrintf(stdout, @"  Building relcation table for %@\n", dir);

  /* Check if it's a directory; if it is, enumerate all HTML files
     inside it, and add all of them.  */
  if (![fileManager fileExistsAtPath: dir  isDirectory: &isDir])
    {
      NSLog (@"%@ does not exist - exiting", dir);
      exit (1);
    }
  else if (!isDir)
    {
      NSLog (@"%@ is not a directory - exiting", dir);
      exit (1);
    }
  else
    {
      HTMLDirectoryEnumerator *e;
      NSString *filename;
      NSMutableDictionary *relocationTable;
      
      relocationTable = [NSMutableDictionary new];
      IF_NO_GC ([relocationTable autorelease];)

      e = [HTMLDirectoryEnumerator alloc];
      e = [[e initWithBasePath: dir] autorelease];
      /* The relocation table for a directory is relative to the
	 directory top, so that the whole directory can be moved
	 around without having to regenerate the .htmlink file.  */
      [e setReturnsAbsolutePaths: NO];

      while ((filename = [e nextObject]) != nil)
	{
	  NSString *fullPath;
	  NSString *file;
	  HTMLParser *p;
	  NSArray *names;
	  unsigned i, count;

	  fullPath = [dir stringByAppendingPathComponent: filename];	  

	  file = [NSString stringWithContentsOfFile: fullPath];
	  
	  p = [[HTMLParser alloc] initWithCode: file];
	  names = [p names];
	  RELEASE (p);
	  count = [names count];
	  
	  for (i = 0; i < count; i++)
	    {
	      NSString *name = [names objectAtIndex: i];
	      [relocationTable setObject: filename  forKey: name];
	    }
	}
      return relocationTable;
    }
}


static void print_help_and_exit ()
{
  printf ("GNUstep HTMLLinker\n");
  printf ("Usage: HTMLLinker [options] input_files [-l relocation_file] [-d destination_file]\n");
  printf ("Multiple input files, and multiple -l and -d options are allowed.\n");
  printf (" `options' include:\n");
  printf ("  --help: print this message;\n");
  printf ("  --version: print version information;\n");
  printf ("  --verbose: print information while processing;\n");
  printf ("  -Warn NO: do not print warnings about unresolved links;\n");
  printf ("  -LinksMarker xxx: only fixup links with attribute rel=xxx;\n");
  printf ("  -FixupAllLinks YES: attempt to fixup all links (not only ones with the marker);\n");
  printf ("  -PathMappingsFile file: read path mappings from file (a dictionary);\n");
  printf ("  -PathMappings '{\"/usr/doc\"=\"/Doc\";}': use the supplied path mappings;\n");
  printf ("  -BuildRelocationFileForDir yyy: build a relocation file for the dir yyy\n");
  printf ("                                  and save it into yyy/table.htmlink.  This option is special\n");
  printf ("                                  and prevents any other processing by the linker.\n");
  exit (0);
}

static void print_version_and_exit ()
{
  printf ("GNUstep HTMLLinker (gnustep-base version %d.%d.%d)\n", 
	  GNUSTEP_BASE_MAJOR_VERSION,
	  GNUSTEP_BASE_MINOR_VERSION,
	  GNUSTEP_BASE_SUBMINOR_VERSION);
  exit (0);
}

int main (int argc, char** argv, char** env)
{
  NSUserDefaults *userDefs;
  NSArray *args;
  NSMutableArray *inputFiles;
  unsigned i, count;
  BOOL warn;
  NSString *linksMarker;
  HTMLLinker *linker;
  CREATE_AUTORELEASE_POOL(pool);

#ifdef GS_PASS_ARGUMENTS
  GSInitializeProcess(argc, argv, env);
#endif

  /* Set up the cache.  */
  fileManager = [NSFileManager defaultManager];
  currentPath = [fileManager currentDirectoryPath];

  /* Read basic defaults.  */
  userDefs = [NSUserDefaults standardUserDefaults];

  /* defaults are - 
     -Warn YES
     -LinksMarker dynamic
     -FixupAllLinks NO
  */
  [userDefs registerDefaults: [NSDictionary dictionaryWithObjectsAndKeys:
					      @"dynamic", @"LinksMarker", 
					    @"YES", @"Warn",
					    nil]];

  warn = [userDefs boolForKey: @"Warn"];
  linksMarker = [userDefs stringForKey: @"LinksMarker"];


  /* If -BuildRelocationFileForDir xxx is passed on the command line,
     build a relocation file for the directory xxx and save it in
     xxx/table.htmlink.  */
  {
    NSString *relFile; 
    relFile = [userDefs stringForKey: @"BuildRelocationFileForDir"];

    if (relFile != nil)
      {
	NSDictionary *table;
	NSString *outputFile;

	outputFile = [relFile stringByAppendingPathComponent: 
				@"table.htmlink"];

	table = build_relocation_table_for_directory (relFile);
	[table writeToFile: outputFile  atomically: YES];
	exit (0);
      }
  }

  /* Create the linker object.  */
  linker = [[HTMLLinker alloc] initWithWarnFlag: warn];

  /* First, read all path mappings (before reading any destination
     file / relocation file, so we can relocate properly.  */
  
  /* Read path mappings from PathMappingsFile if specified.  */
  {
    NSString *pathMapFile = [userDefs stringForKey: @"PathMappingsFile"];
    
    if (pathMapFile != nil)
      {
	NSDictionary *mappings;
	
	mappings = [NSDictionary dictionaryWithContentsOfFile: pathMapFile];
	
	if (mappings == nil)
	  {
	    NSLog (@"Warning - %@ does not contain a dictionary - ignored", 
		   pathMapFile);
	  }
	else
	  {
	    [linker registerPathMappings: mappings];
	  }
      }
  }
  
  /* Add PathMappings specified on the command line if any.  */
  {
    NSDictionary *paths = [userDefs dictionaryForKey: @"PathMappings"];
    
    if (paths != nil)
      {
	[linker registerPathMappings: paths];
      }
  }
  
  /* All non-options on the command line are:
     
     input files
     
     destination files if they come after a -d 

     relocation files if they come after a -l
     
     Directories as input files or destination files means 'all .html, .htm,
     .HTML, .HTM files in the directory and subdirectories'.
     
  */
  args = [[NSProcessInfo processInfo] arguments];

  count = [args count];
  
  inputFiles = AUTORELEASE ([NSMutableArray new]);

  for (i = 1; i < count; i++)
    {
      NSString *arg = [args objectAtIndex: i];
      if ([arg characterAtIndex: 0] == '-')
	{
	  NSString *opt;
	  opt = ([arg characterAtIndex: 1] == '-') ?
	      [arg substringFromIndex: 2] : [arg substringFromIndex: 1];
	  if ([opt isEqualToString: @"help"]
	      || [opt isEqualToString: @"h"])
	    {
	      print_help_and_exit ();
	    }
	  else if ([opt isEqualToString: @"version"]
		   || [opt isEqualToString: @"V"])
	    {
	      print_version_and_exit ();
	    }
	  else if ([opt isEqualToString: @"verbose"]
		   || [opt isEqualToString: @"v"])
	    {
	      verbose++;
	    }
	  else if ([opt isEqualToString: @"d"])
	    {
	      if ((i + 1) < count)
		{
		  i++;
		  /* Register a destination file.  */
		  [linker registerDestinationFile: [args objectAtIndex: i]];
		}
	      else
		{
		  NSLog (@"Missing argument to -d");
		}
	    }
	  else if ([opt isEqualToString: @"l"])
	    {
	      if ((i + 1) < count)
		{
		  i++;
		  /* Register a destination file.  */
		  [linker registerRelocationFile: [args objectAtIndex: i]];
		}
	      else
		{
		  NSLog (@"Missing argument to -l");
		}
	    }
	  else
	    {
	      /* A GNUstep default - skip it and the next argument.  */
	      if ((i + 1) < count)
		{
		  i++;
		  continue;
		}
	    }
	}
      else
	{
	  BOOL isDir;
	  
	  if (![fileManager fileExistsAtPath: arg  isDirectory: &isDir])
	    {
	      NSLog (@"Warning - input file '%@' not found - ignored", arg);
	    }
	  else
	    {
	      if (isDir)
		{
		  HTMLDirectoryEnumerator *e;
		  NSString *filename;
		  
		  e = [[[HTMLDirectoryEnumerator alloc]
		    initWithBasePath: arg] autorelease];
		  [e setReturnsAbsolutePaths: YES];
		  
		  while ((filename = [e nextObject]) != nil)
		    {
		      [inputFiles addObject: filename];
		    }
		}
	      else
		{
		  [inputFiles addObject: arg];
		}
	    }
	}
    }
  
  count = [inputFiles count];

  if (count == 0)
    {
      NSLog (@"No input files specified.");
    }
  

  for (i = 0; i < count; i++)
    {
      NSString *inputFile;
      NSString *inputFileContents;
      HTMLParser *parser;

      inputFile = [inputFiles objectAtIndex: i];
      if (verbose)
        GSPrintf(stdout, @"  %@\n", inputFile);
      inputFileContents = [NSString stringWithContentsOfFile: inputFile];
      
      parser = [[HTMLParser alloc] initWithCode: inputFileContents];
      inputFileContents = [parser resolveLinksUsingHTMLLinker: linker
				  logFile: inputFile
				  linksMarker: linksMarker];
      [inputFileContents writeToFile: inputFile  atomically: YES];
      RELEASE (parser);
    }

  RELEASE (linker);
  [pool drain];

  return 0;
}
