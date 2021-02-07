/** This tool mimics the OPENSTEP command line tool for handling defaults.
   Copyright (C) 1997 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: January 1998

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

#include <string.h>

#import "common.h"

#import	"Foundation/NSArray.h"
#import	"Foundation/NSDictionary.h"
#import	"Foundation/NSEnumerator.h"
#import	"Foundation/NSException.h"
#import	"Foundation/NSProcessInfo.h"
#import	"Foundation/NSUserDefaults.h"
#import	"Foundation/NSAutoreleasePool.h"
#import	"Foundation/NSPathUtilities.h"

#define GSEXIT_SUCCESS EXIT_SUCCESS
#define GSEXIT_FAILURE EXIT_FAILURE
#define GSEXIT_NOTFOUND 2

static NSString *input(char **ptr)
{
  NSString	*result = nil;
  char		*tmp = *ptr;
  char		*start;

  while (*tmp != '\0' && isspace(*tmp))
    {
      tmp++;
    }
  start = tmp;
  if (*start == '\'')
    {
      start = ++tmp;
      while (*tmp != '\0')
	{
	  if (*tmp++ == '\'')
	    {
	      if (*tmp == '\'')
		{
		  memmove(&tmp[-1], tmp, strlen(tmp) + 1);
		}
	      else
		{
		  tmp[-1] = '\0';
		  *ptr = tmp;
		  result = [NSString stringWithUTF8String: start];
		  break;
		}
	    }
	}
    }
  else
    {
      while (*tmp != '\0' && !isspace(*tmp))
	{
	  tmp++;
	}
      *tmp++ = '\0';
      *ptr = tmp;
      result = [NSString stringWithUTF8String: start];
    }
  return result;
}


static void output(const char *ptr)
{
  const char	*tmp;

  for (tmp = ptr; *tmp; tmp++)
    {
      if (isspace(*tmp))
	{
	  break;
	}
    }
  if (tmp == ptr || *tmp != '\0')
    {
      putchar('\'');
      while (*ptr)
	{
	  if (*ptr == '\'')
	    {
	      putchar('\'');
	    }
	  putchar(*ptr);
	  ptr++;
	}
      putchar('\'');
    }
  else
    {
      fputs(ptr, stdout);
    }
}

/** <p>This tool mimics the OPENSTEP command line tool for handling defaults.
       Please see the man page for more information.
 </p>*/
int
main(int argc, char** argv, char **env)
{
  NSAutoreleasePool	*pool;
  NSUserDefaults	*defs;
  NSProcessInfo		*proc;
  NSArray		*args;
  NSArray		*domains;
  NSMutableDictionary	*domain;
  NSString		*owner = nil;
  NSString		*name = nil;
  NSString		*value;
  NSString		*user = nil;
  BOOL			found = NO;
  unsigned int		i;
  int derror = 0;

#ifdef GS_PASS_ARGUMENTS
  GSInitializeProcess(argc, argv, env);
#endif
//[NSObject enableDoubleReleaseCheck: YES];
  pool = [NSAutoreleasePool new];
  proc = [NSProcessInfo processInfo];
  if (proc == nil)
    {
      GSPrintf(stderr, @"defaults: unable to get process information!\n");
      [pool release];
      exit(GSEXIT_FAILURE);
    }

  args = [proc arguments];

  for (i = 1; i < [args count]; i++)
    {
      if ([[args objectAtIndex: i] isEqual: @"--help"] ||
	    [[args objectAtIndex: i] isEqual: @"help"])
	{
	  printf(
"The 'defaults' command lets you to read and modify a user's defaults.\n\n"
"This program replaces the old NeXTstep style dread, dwrite, and dremove\n"
"programs.\n\n"
"If you have access to another user's defaults database, you may include\n"
"'-u username' before any other options to use that user's database rather\n"
"than your own.\n\n");
	  printf(
"defaults read [ domain [ key] ]\n"
"    read the named default from the specified domain.\n"
"    If no 'key' is given - read all defaults from the domain.\n"
"    If no 'domain' is given - read all defaults from all domains.\n\n");
	  printf(
"defaults readkey key\n"
"    read the named default from all domains.\n\n");
	  printf(
"defaults write domain key value\n"
"    write 'value' as default 'key' in the specified domain.\n"
"    'value' must be a property list in single quotes.\n\n");
	  printf(
"defaults write domain dictionary\n"
"    write 'dictionary' as a replacement for the specified domain.\n"
"    'dictionary' must be a property list in single quotes.\n\n");
	  printf(
"defaults write\n"
"    reads standard input for defaults in the format produced by\n"
"    'defaults read' and writes them to the database.\n\n");
	  printf(
"defaults delete [ domain [ key] ]\n"
"    remove the specified default(s) from the domain.\n"
"    If no 'key' is given - delete the entire domain.\n\n");
	  printf(
"defaults delete\n"
"    read standard input for a series of lines containing pairs of domains\n"
"    and keys for defaults to be deleted.\n\n");
	  printf(
"defaults domains\n"
"    lists the domains in the database (one per line)\n\n");
	  printf(
"defaults find word\n"
"    searches domain names, default names, and default value strings for\n"
"    those equal to the specified word and lists them on standard output.\n\n");
	  printf(
"defaults plist\n"
"    output some information about property lists\n\n");
	  printf(
"defaults help\n"
"    list options for the defaults command.\n\n");
	  [pool release];
	  exit(GSEXIT_SUCCESS);
	}
      else if ([[args objectAtIndex: i] isEqual: @"plist"])
	{
	  printf(
"A property list is a method of providing structured information consisting\n"
"of strings, arrays, dictionaries, and binary data.\n\n"
"The defaults system allows you to work with a human-readable form of a\n"
"property list which is set as the value of a default.\n\n");
	  printf(
"In a property list, strings appear as plain text (as long as they contain\n"
"no special characters), and inside quotation marks otherwise.\n"
"Special characters inside a quoted string are 'escaped' by a backslash.\n"
"This escape mechanism is used to permit the double quote mark to appear\n"
"inside a quoted string.\n"
"Unicode characters are represented as four digit hexadecimal numbers\n"
"prefixed by \\U\n"
"Arrays appear as a comma separated list of items delimited by brackets.\n"
"Dictionaries appear as a series of key-value pairs, each pair is followed\n"
"by a semicolon and the whole dictionary is delimited by curly brackets.\n"
"Data is encoded as hexadecimal digits delimited by angle brackets.\n\n");
	  printf(
"In output from 'defaults read' the defaults values are represented as\n"
"property lists enclosed in single quotes.  If a value actually contains\n"
"a string with a single quite mark in it, that quote is repeated.\n"
"Similarly, if 'defaults write' is reading a defaults value from stdin\n"
"it expects to receive the value in single quotes with any internal\n"
"single quote marks repeated.\n\n");
	  printf(
"Here is an example of a dictionary encoded as a text property list -\n\n");
	  printf(
"{\n"
"    Name = \"My Application\";\n"
"    Author = \"Just me and \\\"my other half\\\"\";\n"
"    Modules = (\n"
"	Main,\n"
"	\"'Input output'\",\n"
"	Computation\n"
"    );\n"
"    Checksum = <01014b5b 123a8b20>\n"
"}\n\n");
	  printf(
"And as output from the command 'defaults read foo bar' -\n\n");
	  printf(
"foo bar '{\n"
"    Name = \"My Application\";\n"
"    Author = \"Just me and \\\"my other half\\\"\";\n"
"    Modules = (\n"
"	Main,\n"
"	\"''Input output''\",\n"
"	Computation\n"
"    );\n"
"    Checksum = <01014b5b 123a8b20>\n"
"}'\n\n");
	  [pool release];
	  exit(GSEXIT_SUCCESS);
	}
    }

  i = 1;
  if ([args count] <= i)
    {
      GSPrintf(stderr, @"defaults: too few arguments supplied!\n");
      [pool release];
      exit(GSEXIT_FAILURE);
    }
  if ([[args objectAtIndex: i] isEqual: @"-u"])
    {
      if ([args count] > ++i)
	{
	  user = [args objectAtIndex: i++];
	}
      else
	{
	  GSPrintf(stderr, @"defaults: no name supplied for -u option!\n");
	  [pool release];
	  exit(GSEXIT_FAILURE);
	}
    }
  if (user)
    {
      GSSetUserName(user);
      defs = [[NSUserDefaults alloc] initWithUser: user];
    }
  else
    {
      defs = [NSUserDefaults standardUserDefaults];
    }
  if (defs == nil)
    {
      GSPrintf(stderr, @"defaults: unable to access defaults database!\n");
      [pool release];
      exit(GSEXIT_FAILURE);
    }
  /* We don't want this tool in the defaults database - so remove it. */
  [defs removePersistentDomainForName: [proc processName]];

  if ([args count] <= i)
    {
      GSPrintf(stderr, @"defaults: too few arguments supplied!\n");
      [pool release];
      exit(GSEXIT_FAILURE);
    }

  if ([[args objectAtIndex: i] isEqual: @"read"] ||
      [[args objectAtIndex: i] isEqual: @"readkey"])
    {
      NSDictionary	*locale = [defs dictionaryRepresentation];

      if ([[args objectAtIndex: i] isEqual: @"read"])
	{
	  if ([args count] == ++i)
	    {
	      name = nil;
	      owner = nil;
	    }
	  else
	    {
	      owner = [args objectAtIndex: i++];
	      if ([args count] > i)
		{
		  name = [args objectAtIndex: i];
		}
	    }
	}
      else
	{
	  if ([args count] == ++i)
	    {
	      GSPrintf(stderr, @"defaults: too few arguments supplied!\n");
	      [pool release];
	      exit(GSEXIT_FAILURE);
	    }
	  owner = nil;
	  name = [args objectAtIndex: i];
	}

      domains = [defs persistentDomainNames];
      for (i = 0; i < [domains count]; i++)
	{
	  NSString	*domainName = [domains objectAtIndex: i];

	  if (owner == nil || [owner isEqual: domainName])
	    {
	      NSDictionary	*dom;

	      dom = [defs persistentDomainForName: domainName];
	      if (dom)
		{
		  if (name == nil)
		    {
		      NSEnumerator	*enumerator;
		      NSString		*key;

		      enumerator = [dom keyEnumerator];
		      while ((key = [enumerator nextObject]) != nil)
			{
			  id		obj = [dom objectForKey: key];
			  const char	*ptr;

			  ptr = [domainName UTF8String];
			  output(ptr);
			  putchar(' ');

			  ptr = [key UTF8String];
			  output(ptr);
			  putchar(' ');

			  ptr = [[obj descriptionWithLocale: locale
			    indent: 0] UTF8String];
			  output(ptr);
			  putchar('\n');
			}
		    }
		  else
		    {
		      id	obj = [dom objectForKey: name];

		      if (obj)
			{
			  const char      *ptr;

			  ptr = [domainName UTF8String];
			  output(ptr);
			  putchar(' ');
			  ptr = [name UTF8String];
			  output(ptr);
			  putchar(' ');
			  ptr = [[obj descriptionWithLocale: locale indent: 0]
			    UTF8String];
			  output(ptr);
			  putchar('\n');
			  found = YES;
			}
		    }
		}
	    }
	}

      domains = [defs volatileDomainNames];
      for (i = 0; i < [domains count]; i++)
	{
	  NSString	*domainName = [domains objectAtIndex: i];

#if 0
	  if (owner == nil || [owner isEqual: domainName])
#else	
	  if ([owner isEqual: domainName])
#endif
	    {
	      NSDictionary	*dom;

	      dom = [defs volatileDomainForName: domainName];
	      if (dom)
		{
		  if (name == nil)
		    {
		      NSEnumerator	*enumerator;
		      NSString		*key;

		      enumerator = [dom keyEnumerator];
		      while ((key = [enumerator nextObject]) != nil)
			{
			  id		obj = [dom objectForKey: key];
			  const char	*ptr;

			  ptr = [domainName UTF8String];
			  output(ptr);
			  putchar(' ');

			  ptr = [key UTF8String];
			  output(ptr);
			  putchar(' ');

			  ptr = [[obj descriptionWithLocale: locale
			    indent: 0] UTF8String];
			  output(ptr);
			  putchar('\n');
			}
		    }
		  else
		    {
		      id	obj = [dom objectForKey: name];

		      if (obj)
			{
			  const char      *ptr;

			  ptr = [domainName UTF8String];
			  output(ptr);
			  putchar(' ');
			  ptr = [name UTF8String];
			  output(ptr);
			  putchar(' ');
			  ptr = [[obj descriptionWithLocale: locale indent: 0]
			    UTF8String];
			  output(ptr);
			  putchar('\n');
			  found = YES;
			}
		    }
		}
	    }
	}

      if (found == NO && name != nil)
	{
	  GSPrintf(stderr, @"defaults read: couldn't read default\n");
	  derror = GSEXIT_NOTFOUND;
	}
    }
  else if ([[args objectAtIndex: i] isEqual: @"write"])
    {
      id	obj;

      if ([args count] == ++i)
	{
	  int	size = BUFSIZ;
	  int	got;
	  int	off = 0;
	  char	*buf = malloc(size);
	  char	*ptr;

	  /*
	   *	Read from stdin - grow buffer as necessary since defaults
	   *	values are quoted property lists which may be huge.
	   */
	  while ((got = fread(buf + off, 1, BUFSIZ, stdin)) == BUFSIZ)
	    {
	      off += BUFSIZ;
	      size += BUFSIZ;
	      buf = realloc(buf, size);
	      if (buf == 0)
		{
		  GSPrintf(stderr,
		    @"defaults write: out of memory loading defaults\n");
		  [pool release];
		  exit(GSEXIT_FAILURE);
		}
	    }
	  buf[off + got] = '\0';

	  ptr = buf;

	  for (;;)
	    {
	      unichar	c;

	      /*
	       *	Expect domain name as a space delimited string.
	       */
	      while (isspace(*ptr))
		{
		  ptr++;
		}
	      if (*ptr == '\0')
		{
		  break;	/* At end ... quit. */
		}

	      owner = input(&ptr);
	      if ([owner length] == 0)
		{
		  GSPrintf(stderr,
		    @"defaults write: invalid input - nul domain name\n");
		  [pool release];
		  exit(GSEXIT_FAILURE);
		}

	      name = input(&ptr);
	      if ([name length] == 0)
		{
		  GSPrintf(stderr,
		    @"defaults write: invalid input - nul default name.\n");
		  [pool release];
		  exit(GSEXIT_FAILURE);
		}

	      /*
	       * Expect defaults value as a quoted property list which
	       * may cover multiple lines.
	       */
	      obj = input(&ptr);
	      if (obj == nil)
		{
		  GSPrintf(stderr,
		    @"defaults write: invalid input - "
		    @"empty property list\n");
		  [pool release];
		  exit(GSEXIT_FAILURE);
		}

	      c = 0;
	      if ([obj length] > 0)
		{
		  c = [obj characterAtIndex: 0];
		}
	      if (c == '(' || c == '{' || c == '<' || c == '"')
		{
		  id	tmp;

		  NS_DURING
		    tmp = [obj propertyList];
		  NS_HANDLER
		    NSLog(@"Failed to parse '%@' ... '%@'",
		      obj, localException);
		    tmp = nil;
		  NS_ENDHANDLER
		  if (tmp == nil)
		    {
		      GSPrintf(stderr,
		        @"defaults write: invalid input - "
			@"bad property list\n");
		      [pool release];
		      exit(GSEXIT_FAILURE);
		    }
		  else
		    {
		      obj = tmp;
		    }
		}

	      domain = [[defs persistentDomainForName: owner] mutableCopy];
	      if (domain == nil)
		{
		  domain = [NSMutableDictionary dictionaryWithCapacity:1];
		}
	      [domain setObject: obj forKey: name];
	      [defs setPersistentDomain: domain forName: owner];
	    }
	}
      else
	{
	  owner = [args objectAtIndex: i++];
	  if ([args count] <= i)
	    {
	      GSPrintf(stderr, @"defaults: no dictionary or key for write!\n");
	      [pool release];
	      exit(GSEXIT_FAILURE);
	    }
	  name = [args objectAtIndex: i++];
	  if ([args count] > i)
	    {
	      const char	*ptr;

	      value = [args objectAtIndex: i];
	      ptr = [value UTF8String];

	      if (*ptr == '(' || *ptr == '{' || *ptr == '<' || *ptr == '"')
		{
		  NS_DURING
		    obj = [value propertyList];
		  NS_HANDLER
		    NSLog(@"Failed to parse '%@' ... '%@'",
		      value, localException);
		    obj = nil;
		  NS_ENDHANDLER

		  if (obj == nil)
		    {
		      GSPrintf(stderr, @"defaults write: invalid input - "
			@"bad property list\n");
		      [pool release];
		      exit(GSEXIT_FAILURE);
		    }
		}
	      else
		{
		  obj = value;
		}

	      domain = [[defs persistentDomainForName: owner] mutableCopy];
	      if (domain == nil)
		{
		  domain = [NSMutableDictionary dictionaryWithCapacity:1];
		}
	      [domain setObject: obj forKey: name];
	      [defs setPersistentDomain: domain forName: owner];
	    }
	  else
	    {
	      domain = [name propertyList];
	      if (domain == nil ||
			[domain isKindOfClass: [NSDictionary class]] == NO)
		{
		  GSPrintf(stderr,
		    @"defaults write: domain is not a dictionary!\n");
		  [pool release];
		  exit(GSEXIT_FAILURE);
		}
	    }
	}

      if ([defs synchronize] == NO)
	{
	  GSPrintf(stderr,
	    @"defaults: unable to write to defaults database\n");
	}
    }
  else if ([[args objectAtIndex: i] isEqual: @"delete"])
    {
      if ([args count] == ++i)
	{
	  int	size = BUFSIZ;
	  int	got;
	  int	off = 0;
	  char	*buf = malloc(size);
	  char	*ptr;

	  while ((got = fread(buf + off, 1, BUFSIZ, stdin)) == BUFSIZ)
	    {
	      off += BUFSIZ;
	      size += BUFSIZ;
	      buf = realloc(buf, size);
	      if (buf == 0)
		{
		  GSPrintf(stderr,
		    @"defaults write: out of memory reading domains\n");
		  [pool release];
		  exit(GSEXIT_FAILURE);
		}
	    }
	  buf[off + got] = '\0';

	  ptr = buf;

	  for (;;)
	    {
	      while (*ptr && isspace(*ptr))
		{
		  ptr++;
		}
	      if (*ptr == '\0')
		{
		  break;	// Read all
		}
	      owner = input(&ptr);
	      if ([owner length] == 0)
		{
		  GSPrintf(stderr,
		    @"defaults delete: invalid input - empty domain name\n");
		  [pool release];
		  exit(GSEXIT_FAILURE);
		}
	      name = input(&ptr);
	      if ([name length] == 0)
		{
		  GSPrintf(stderr,
		    @"defaults delete: invalid input - empty key\n");
		  [pool release];
		  exit(GSEXIT_FAILURE);
		}
	      domain = [[defs persistentDomainForName: owner] mutableCopy];
	      if (domain == nil)
	        {
		  GSPrintf(stderr, @"defaults delete: couldn't remove "
		    @"value from non-existent domain %s\n", [owner UTF8String]);
		}
	      else if ([domain objectForKey: name] == nil)
		{
		  GSPrintf(stderr,
		    @"defaults delete: couldn't remove non-existent value %s "
		    @"from domain %s\n",
		    [name UTF8String], [owner UTF8String]);
		}
	      else
		{
		  [domain removeObjectForKey: name];
		  [defs setPersistentDomain: domain forName: owner];
		}
	    }
	}
      else
	{
	  owner = [args objectAtIndex: i++];
	  if ([args count] > i)
	    {
	      name = [args objectAtIndex: i];
	    }
	  else
	    {
	      name = nil;
	    }
	  if (name)
	    {
	      domain = [[defs persistentDomainForName: owner] mutableCopy];
	      if (domain == nil)
	        {
		  GSPrintf(stderr, @"defaults delete: couldn't remove "
		    @"value from non-existent domain %s\n", [owner UTF8String]);
		}
	      else if ([domain objectForKey: name] == nil)
		{
		  GSPrintf(stderr,
		    @"defaults delete: couldn't remove non-existent value %s "
		    @"from domain %s\n",
		    [name UTF8String], [owner UTF8String]);
		}
	      else
		{
		  [domain removeObjectForKey: name];
		  [defs setPersistentDomain: domain forName: owner];
		}
	    }
	  else
	    {
	      if ([defs persistentDomainForName: owner]  == nil)
	        {
		  GSPrintf(stderr, @"defaults delete: couldn't remove "
		    @"non-existent domain %s\n", [owner UTF8String]);
		}
	      else
	        {
	          [defs removePersistentDomainForName: owner];
		}
	    }
	}
      if ([defs synchronize] == NO)
	{
	  GSPrintf(stderr,
	    @"defaults: unable to write to defaults database\n");
	}
    }
  else if ([[args objectAtIndex: i] isEqual: @"domains"])
    {
      domains = [defs persistentDomainNames];
      for (i = 0; i < [domains count]; i++)
	{
	  NSString	*domainName = [domains objectAtIndex: i];

	  output([domainName UTF8String]);
	  putchar('\n');
	}
    }
  else if ([[args objectAtIndex: i] isEqual: @"find"])
    {
      if ([args count] == ++i)
	{
	  GSPrintf(stderr, @"defaults: no arguments for find!\n");
	  [pool release];
	  exit(GSEXIT_FAILURE);
	}
      name = [args objectAtIndex: i];

      domains = [defs persistentDomainNames];
      for (i = 0; i < [domains count]; i++)
	{
	  NSString	*domainName = [domains objectAtIndex: i];
	  NSDictionary	*dom;

	  if ([domainName isEqual: name])
	    {
	      GSPrintf(stderr, @"%s\n", [domainName UTF8String]);
	      found = YES;
	    }

	  dom = [defs persistentDomainForName: domainName];
	  if (dom)
	    {
	      NSEnumerator	*enumerator;
	      NSString		*key;

	      enumerator = [dom keyEnumerator];
	      while ((key = [enumerator nextObject]) != nil)
		{
		  id	obj = [dom objectForKey: key];

		  if ([key isEqual: name])
		    {
		      GSPrintf(stderr, @"%s %s\n",
		        [domainName UTF8String], [key UTF8String]);
		      found = YES;
		    }
		  if ([obj isKindOfClass: [NSString class]])
		    {
		      if ([obj isEqual: name])
			{
			  GSPrintf(stderr, @"%s %s %s\n",
			    [domainName UTF8String],
			    [key UTF8String],
			    [obj UTF8String]);
			  found = YES;
			}
		    }
		}
	    }
	}

      if (found == NO)
	{
	  GSPrintf(stderr, @"defaults find: couldn't find value\n");
	  derror = GSEXIT_NOTFOUND;
	}
    }
  else
    {
      GSPrintf(stderr, @"defaults: unknown option supplied!\n");
      derror = GSEXIT_FAILURE;
    }

  [pool release];
  exit(derror);
}

