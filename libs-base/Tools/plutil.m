/** Property list utility.
   Copyright (C) 2020 Free Software Foundation, Inc.

   Written by:  Mingye Wang
   Created: feb 2020

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

// #import "common.h"
#include <string.h>

#import "Foundation/NSArray.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSException.h"
#import "Foundation/NSFileHandle.h"
#import "Foundation/NSProcessInfo.h"
#import "Foundation/NSPropertyList.h"
#import "Foundation/NSString.h"
#import "Foundation/NSUserDefaults.h"
#import "Foundation/NSValue.h"
#import "NSPropertyList+PLUtil.h"

// From NSPropertyList.m
GS_EXPORT void
GSPropertyListMake(id, NSDictionary *, BOOL, BOOL, unsigned, id *);

// We don't have @[] on gcc
#define NARRAY(...) [NSArray arrayWithObjects: __VA_ARGS__, nil]
// And no @() or @123
#define NINT(Num) [NSNumber numberWithInt: Num]
// Unfortunately we have to define @() for @"" too because a macro later
#define NSTR(Str) [NSString stringWithCString: Str]

/* Bitmap of 'quotable' characters ... those characters which must be
 * inside a quoted string if written to an old style property list.
 * Taken from NSPropertyList.m.
 */
static const unsigned char quotables[32] = {
  '\xff', '\xff', '\xff', '\xff', '\x85', '\x13', '\x00', '\x78',
  '\x00', '\x00', '\x00', '\x38', '\x01', '\x00', '\x00', '\xa8',
  '\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff',
  '\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff',
};
#define IS_BIT_SET(a, i) ((((a) & (1 << (i)))) > 0)
#define GS_IS_QUOTABLE(X) IS_BIT_SET(quotables[(X) / 8], (X) % 8)

/**
 * Indexes a NSArray or a NSDictionary.
 */
id
plIndex(id obj, NSString *key)
{
  const char	*ckey;
  char 		*endptr = NULL;
  NSInteger   	res;

  if ([obj isKindOfClass: [NSDictionary class]])
    {
      return [(NSDictionary *) obj objectForKey: key];
    }
  else if ([obj isKindOfClass: [NSArray class]])
    {
      ckey = [key cStringUsingEncoding: [NSString defaultCStringEncoding]];
      res = strtoll(ckey, &endptr, 10);
      if (endptr && *endptr != '\0')
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"%@ is not a valid integer", key];
	}
      return [(NSArray *) obj objectAtIndex: res];
    }
  else
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"%@ is not indexible", obj];
      return nil;
    }
}

/**
 * Mutate obj[key] to leaf.
 * If leaf is nil, remove.
 * Else if replace is NO, insert.
 * Otherwise replace.
 *
 * Inserting and replacing are the same for NSMutableDictionary.
 */
void
mutate(id obj, NSString *key, id leaf, BOOL replace)
{
  const char	*ckey;
  char 		*endptr = NULL;
  NSInteger   	res;

  if ([obj isKindOfClass: [NSMutableDictionary class]])
    {
      if (!leaf)
	[(NSMutableDictionary *) obj removeObjectForKey: key];
      else
	[(NSMutableDictionary *) obj setObject: leaf forKey: key];
    }
  else if ([obj isKindOfClass: [NSMutableArray class]])
    {
      ckey = [key cStringUsingEncoding: [NSString defaultCStringEncoding]];
      res = strtoll(ckey, &endptr, 10);
      if (endptr && *endptr != '\0')
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"%@ is not a valid integer", key];
	}
      if (!leaf)
	[(NSMutableArray *) obj removeObjectAtIndex: res];
      else if (replace)
	[(NSMutableArray *) obj replaceObjectAtIndex: res withObject: leaf];
      else
	[(NSMutableArray *) obj insertObject: leaf atIndex: res];
    }
  else
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"%@ is not indexible", obj];
    }
}

#define KEYPATH_SEP '.'
NSString *
parseQuotedString(const char *, size_t *);
/**
 * Parses a keypath to its component keys. ISO/IEC 14977 EBNF:
 * <pre>
 * keypath = '' | keypath '.' key;
 * key = quotedString | unquotedStringNotAllowingPeriod;
 * </pre>
 * The definitions of strings follow plist conventions.
 * The use of quoted strings is a GNUstep extension (I think).
 */
NSArray *
parseKeyPath(const char *keypath)
{
  NSMutableArray	*res = [[NSMutableArray alloc] init];
  NSString		*key = nil;
  size_t	  	i;
  size_t	  	j;

  for (i = 0; keypath[i]; i++)
    {
      switch (keypath[i])
	{
	  case KEYPATH_SEP:
	    if (key != nil)
	      {
		[res addObject: key];
		key = nil;
	      }
	    break;

	  case '"':
	    key = parseQuotedString(keypath, &i);
	    break;

	  default:
	    j = i;
	    while (keypath[j]
	      && !GS_IS_QUOTABLE(keypath[j])
	      && keypath[j] != KEYPATH_SEP)
	      {
		j++;
	      }
	    key = [NSString stringWithCString: &keypath[i] length: (j - i)];
	    i = j - 1;
	    break;
	}
    }
  return [res copy];
}

/**
 * Parse a quoted string by pretending it is a plist.
 */
NSString *
parseQuotedString(const char *keypath, size_t *i)
{
  const char *begin = &keypath[*i];
  const char *end;
  id	  parsed;

  // Select the part of the quoted string that looks like a plist string
  for (end = begin + 1; *end && *end != '"'; end++)
    {
      if (*end == '\\')
	{
	  if (end[1])
	    {
	      end += 1;
	    }
	  else
	    {
	      [NSException raise: NSInvalidArgumentException
			  format: @"Premature EOF in keypath"];
	    }
	}
    }

  if (!*end)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Premature EOF in keypath"];
    }
  *i = end - keypath + 1;
  parsed = [[NSString stringWithCString: begin length: end-begin] propertyList];
  return (NSString *)parsed;
}

/**
 * Index by keypath.
 */
id
plIndexKeypath(id obj, NSString *keypath, int depthOffset)
{
  NSArray *parsedPath = parseKeyPath([keypath cString]);
  int      count = [parsedPath count];
  int      i;

  for (i = 0; i < count - depthOffset; i++)
    obj = plIndex(obj, [parsedPath objectAtIndex: i]);

  return obj;
}

/**
 * Interpret -type value options.
 *
 * The -plist type is a GNUStep extension.
 */
id
parseValue(NSString *type, NSString *value)
{
  if ([type isEqual: @"-plist"])
    return [value propertyList];
  else if ([type isEqual: @"-xml"] || [type isEqual: @"-date"])
    {
      NSData *mydata =
	[value dataUsingEncoding: [NSString defaultCStringEncoding]];
      NSPropertyListFormat aFormat;
      NSError *		   anError;
      id		   result = [NSPropertyListSerialization
	propertyListWithData: mydata
		       options: NSPropertyListMutableContainersAndLeaves
			format: &aFormat
			 error: &anError];
      if (result == nil)
	{
	  GSPrintf(stderr, @"Parsing plist %@: %@\n", value, anError);
	  return nil;
	}
      else if ([type isEqual: @"-xml"]
	       && aFormat != NSPropertyListXMLFormat_v1_0)
	GSPrintf(stderr,
		 @"Warning: parsing XML plist %@: Not an XML (fmt %d)\n", value,
		 aFormat);
      else if ([type isEqual: @"-date"]
	       && ![result isKindOfClass: [NSDate class]])
	GSPrintf(stderr, @"Warning: parsing date %@: Not a date (got %@)\n",
		 value, result);
      return result;
    }
  else if ([type isEqual: @"-bool"])
    return [NSNumber
      numberWithBool: ([value isEqual: @"YES"] || [value isEqual: @"true"])];
  else if ([type isEqual: @"-integer"])
    return [[NSNumber alloc] initWithLongLong: [value longLongValue]];
  else if ([type isEqual: @"-float"])
    // We do a step further than NSPropertyList.m and probably Apple,
    // since notsupporting inf and nan is a bad look
    // (No hex literals for now unless someone really wants it)
    return [[NSNumber alloc] initWithDouble: strtod([value cString], 0)];
  else if ([type isEqual: @"-data"])
    return [[NSData alloc]
      initWithBase64EncodedData: [value
				  dataUsingEncoding: [NSString
						      defaultCStringEncoding]]
			options: NSDataBase64DecodingIgnoreUnknownCharacters];
  else
    GSPrintf(stderr, @"Unrecognized type %@\n", type);

  return nil;
}

#define SELFMAP(Name) NINT(Name), NSTR(#Name)
/**
 * Translates a string fmt to NSPropertyListFormat.
 */
NSPropertyListFormat
plFormatFromName(NSString *name)
{
  // clang-format off
  NSDictionary *nameMap = [NSDictionary dictionaryWithObjectsAndKeys:
    NINT(NSPropertyListXMLFormat_v1_0), @"xml1",
    NINT(NSPropertyListBinaryFormat_v1_0), @"binary1",
    NINT(NSPropertyListOpenStepFormat), @"openstep",
    NINT(NSPropertyListGNUstepFormat), @"gnustep",
    NINT(NSPropertyListGNUstepBinaryFormat), @"gsbinary",
    NINT(NSPropertyListJSONFormat), @"json",
    SELFMAP(NSPropertyListOpenStepFormat),
    SELFMAP(NSPropertyListXMLFormat_v1_0),
    SELFMAP(NSPropertyListBinaryFormat_v1_0),
    SELFMAP(NSPropertyListGNUstepFormat),
    SELFMAP(NSPropertyListGNUstepBinaryFormat),
    nil];
  id res = [nameMap objectForKey: name];
  // clang-format on
  if (!res)
    [NSException raise: NSInvalidArgumentException
		format: @"Invalid fmt %@", name];
  return [res intValue];
}

/**
 * Dumps obj to outfile.
 */
int
dumpToFile(id obj, NSPropertyListFormat fmt, NSString *outfile)
{
  NSString 	*errorString = nil;
  NSFileHandle 	*fh;
  NSData 	*outdata =
    [NSPropertyListSerialization dataFromPropertyList: obj
					       format: fmt
				     errorDescription: &errorString];
  if (errorString)
    {
      GSPrintf(stderr, @"Dumping %@ as format %@ - %@\n", obj, fmt,
	       errorString);
      return EXIT_FAILURE;
    }

  if ([outfile isEqual: @"-"])
    fh = [NSFileHandle fileHandleWithStandardOutput];
  else
    fh = [NSFileHandle fileHandleForWritingAtPath: outfile];
  [fh writeData: outdata];
  [fh synchronizeFile];
  return EXIT_SUCCESS;
}

int
plCmdConvert(id obj, NSArray *cmdargs, NSString *outfile)
{
  NSString *fmt = [cmdargs objectAtIndex: 0];
  return dumpToFile(obj, plFormatFromName(fmt), outfile);
}

int
plCmdExtract(id obj, NSArray *cmdargs, NSString *outfile)
{
  NSString *keypath = [cmdargs objectAtIndex: 0];
  NSString *fmt = [cmdargs objectAtIndex: 1];
  obj = plIndexKeypath(obj, keypath, 0);
  return dumpToFile(obj, plFormatFromName(fmt), outfile);
}

int
plCmdRemove(id obj, NSPropertyListFormat fmt, NSArray *cmdargs,
	    NSString *outfile)
{
  NSString *keypath = [cmdargs objectAtIndex: 0];

  NSArray *parsedPath = parseKeyPath([keypath cString]);
  id       leaf = plIndexKeypath(obj, keypath, 1);

  mutate(leaf, [parsedPath lastObject], nil, false);
  return dumpToFile(obj, fmt, outfile);
}

int
plCmdInsert(id obj, NSPropertyListFormat fmt, NSArray *cmdargs,
	    NSString *outfile)
{
  NSString *keypath = [cmdargs objectAtIndex: 0];
  id newleaf = parseValue([cmdargs objectAtIndex: 1], [cmdargs objectAtIndex: 2]);

  NSArray *parsedPath = parseKeyPath([keypath cString]);
  id       leaf = plIndexKeypath(obj, keypath, 1);

  if (!newleaf)
    return EXIT_FAILURE;

  mutate(leaf, [parsedPath lastObject], newleaf, false);
  return dumpToFile(obj, fmt, outfile);
}

int
plCmdReplace(id obj, NSPropertyListFormat fmt, NSArray *cmdargs,
	     NSString *outfile)
{
  NSString *keypath = [cmdargs objectAtIndex: 0];
  id newleaf = parseValue([cmdargs objectAtIndex: 1], [cmdargs objectAtIndex: 2]);

  NSArray *parsedPath = parseKeyPath([keypath cString]);
  id       leaf = plIndexKeypath(obj, keypath, 1);

  if (!newleaf)
    return EXIT_FAILURE;

  mutate(leaf, [parsedPath lastObject], newleaf, true);
  return dumpToFile(obj, fmt, outfile);
}

static void
print_help(FILE *f)
{
  GSPrintf(f, @"Property list utility\n");
  GSPrintf(f, @"Usage: plutil [command] [options] file\n\n");
  GSPrintf(f, @"Accepted commands:\n");
  GSPrintf(
    f, @"  -p\tPrints the plists in a human-readable form (GNUstep ASCII).\n");
    GSPrintf(
    f, @"  -lint\tVerifies the plist can be parsed.\n");
    GSPrintf(
    f, @"  -convert FMT\tConverts the plist to another format.\n");
    GSPrintf(
    f, @"  -insert PATH KEY VALUE\tInsert KEY=VALUE to the object at PATH.\n");
    GSPrintf(
    f, @"  -replace PATH KEY VALUE\tReplace KEY=VALUE for the object at PATH.\n");
    GSPrintf(
    f, @"  -remove PATH KEY\tRemove KEY from the object at PATH.\n");
    GSPrintf(
    f, @"  -extract PATH KEY\tExtract the KEY from the object at PATH.\n");
  GSPrintf(f, @"Accepted options:\n");
  GSPrintf(
    f, @"  -s\t(No effect.)\n");
  GSPrintf(
    f, @"  -o OUTFILE\tOutput to the file given.\n");
  GSPrintf(
    f, @"  -e OUTEXT\tOutput to a file with the given extension.\n");
}

typedef enum _Action
{
  ACTION_LINT,
  ACTION_PRINT,
  ACTION_CONVERT,
  ACTION_REPLACE,
  ACTION_INSERT,
  ACTION_REMOVE,
  ACTION_EXTRACT,
} Action;

/** <p> Property list utility. Should act like macOS catalina plutil(1).
 * </p>
 */
int
main(int argc, char **argv, char **env)
{
  int status = EXIT_SUCCESS;

  do
    {
      ENTER_POOL
      NSProcessInfo	*proc;
      NSArray 		*args;
      NSString		*arg;
      NSString 		*inpath;
      NSArray 		*cmdargs = nil;
      NSDictionary 	*commands = nil;
      NSArray 		*command_rhs;
      NSString 		*outpath = nil;
      NSString 		*outext = nil;
      Action 		action = ACTION_LINT;
      int    		count = 0;
      int    		i = 1;

#ifdef GS_PASS_ARGUMENTS
      GSInitializeProcess(argc, argv, env);
#endif
      proc = [NSProcessInfo processInfo];
      if (proc == nil)
	{
	  NSLog(@"plutil: unable to get process information.");
	  status = EXIT_FAILURE;
	  break;
	}

      args = [proc arguments];
      if ((count = [args count]) <= 1)
	{
	  NSLog(@"plutil: no files given.");
	  status = EXIT_FAILURE;
	  break;
	}

      // Parse the COMMAND.
      // Maps number of args to commands.
      // clang-format off
      commands = [NSDictionary dictionaryWithObjectsAndKeys:
	NARRAY(NINT(ACTION_PRINT), NINT(0)), @"-p",
	NARRAY(NINT(ACTION_LINT), NINT(0)), @"-lint",
	NARRAY(NINT(ACTION_CONVERT), NINT(1)), @"-convert",
	NARRAY(NINT(ACTION_INSERT), NINT(3)), @"-insert",
	NARRAY(NINT(ACTION_REPLACE), NINT(3)), @"-replace",
	NARRAY(NINT(ACTION_REMOVE), NINT(1)), @"-remove",
	NARRAY(NINT(ACTION_EXTRACT), NINT(2)), @"-extract",
	nil];
      // clang-format on
      NS_DURING
      {
	NSData 			*fileData;
	NSPropertyListFormat 	aFormat;
	NSError 		*anError;
	id			result;
	NSMutableString 	*outStr = nil;
	NSDictionary 		*locale;

	arg = [args objectAtIndex: i];
	if (![arg hasPrefix: @"-"])
	  goto parse_file;

	command_rhs = [commands objectForKey: arg];
	if (command_rhs)
	  {
	    int     iwant;
	    NSRange argrange;

	    action = [[command_rhs objectAtIndex: 0] intValue];
	    iwant = [[command_rhs objectAtIndex: 1] intValue];

	    argrange.location = i + 1;
	    argrange.length = iwant;
	    cmdargs = [args subarrayWithRange: argrange];

	    i += 1 + iwant;
	  }

	// Parse options
	for (; i < count; i++)
	  {
	    arg = [args objectAtIndex: i];
	    if (![arg hasPrefix: @"-"]
	      || [arg isEqual: @"-"]
	      || [arg isEqual: @"--"])
	      {
		goto parse_file;
	      }
	    else if ([arg caseInsensitiveCompare: @"-help"] == NSOrderedSame
	      || [arg caseInsensitiveCompare: @"--help"] == NSOrderedSame
	      || [arg caseInsensitiveCompare: @"-h"] == NSOrderedSame)
	      {
		print_help(stdout);
		break;
	      }
	    else if ([arg isEqual: @"-s"])
	      {
	      /* NOOP: What the heck is being quiet? */;
	      }
	    else if ([arg isEqual: @"-o"])
	      {
		outpath = [args objectAtIndex: ++i];
	      }
	    else if ([arg isEqual: @"-e"])
	      {
		outext = [args objectAtIndex: ++i];
	      }
	    else
	      {
		GSPrintf(stderr, @"unrecognized option: %@\n", arg);
		return EXIT_FAILURE;
	      }
	  }

      parse_file:
	inpath = [args objectAtIndex: i];
	if (!outpath && !outext && action != ACTION_EXTRACT)
	  {
	    outpath = inpath;
	  }
	else if (outext)
	  {
	    NSRange	dot;

	    dot = [inpath rangeOfString: @"." options: NSBackwardsSearch];
	    if (dot.length == 0)
	      {
		dot.location = [inpath length];
	      }
	    outpath = [NSString stringWithFormat: @"%@.%@",
	      [inpath substringToIndex: dot.location], outext];
	  }

	// Open, read, do things.
	if ([inpath isEqual: @"-"])
	  {
	    NSFileHandle *fh = [NSFileHandle fileHandleWithStandardInput];
	    fileData = [fh readDataToEndOfFile];
	  }
	else
	  fileData = [NSData dataWithContentsOfFile: inpath];

	result = [NSPropertyListSerialization
	  propertyListWithData: fileData
		       options: NSPropertyListMutableContainersAndLeaves
			format: &aFormat
			 error: &anError];
	if (result == nil)
	  {
	    GSPrintf(stderr, @"Loading '%@' - %@\n", inpath, anError);
	    status = EXIT_FAILURE;
	    break;
	  }

	switch (action)
	  {
	  case ACTION_LINT:
	    break;
	  case ACTION_PRINT:
	    // Not using description because we can GS it
	    locale =
	      [[NSUserDefaults standardUserDefaults] dictionaryRepresentation];
	    GSPropertyListMake(result, locale, NO, NO, 2, &outStr);
	    GSPrintf(stdout, @"%@\n", outStr);
	    break;
	  case ACTION_CONVERT:
	    status = plCmdConvert(result, cmdargs, outpath);
	    break;
	  case ACTION_REPLACE:
	    status = plCmdReplace(result, aFormat, cmdargs, outpath);
	    break;
	  case ACTION_INSERT:
	    status = plCmdInsert(result, aFormat, cmdargs, outpath);
	    break;
	  case ACTION_REMOVE:
	    status = plCmdRemove(result, aFormat, cmdargs, outpath);
	    break;
	  case ACTION_EXTRACT:
	    if (!outpath)
	      outpath = @"-";
	    status = plCmdExtract(result, cmdargs, outpath);
	    break;
	  }
      }
      NS_HANDLER
      {
	NSLog(@"Problem: %@", localException);
	if ([[localException name] isEqual: NSInvalidArgumentException])
	  print_help(stderr);
	status = EXIT_FAILURE;
	break;
      }
      NS_ENDHANDLER

      LEAVE_POOL
    } while (0);
  return status;
}
