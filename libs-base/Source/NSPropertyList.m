/** Interface for NSPropertyList for GNUstep
   Copyright (C) 2003,2004 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   		Fred Kiefer <FredKiefer@gmx.de>

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.

   */

#import "common.h"
#import "GNUstepBase/GSMime.h"

#import "Foundation/NSArray.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSByteOrder.h"
#import "Foundation/NSCalendarDate.h"
#import "Foundation/NSCharacterSet.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSError.h"
#import "Foundation/NSException.h"
#import "Foundation/NSHashTable.h"
#import "Foundation/NSPropertyList.h"
#import "Foundation/NSSerialization.h"
#import "Foundation/NSStream.h"
#import "Foundation/NSTimeZone.h"
#import "Foundation/NSUserDefaults.h"
#import "Foundation/NSValue.h"
#import "Foundation/NSNull.h"
#import "Foundation/NSXMLParser.h"
#import "GNUstepBase/Unicode.h"
#import "GNUstepBase/NSProcessInfo+GNUstepBase.h"
#import "GNUstepBase/NSString+GNUstepBase.h"

#import "GSPrivate.h"

static id       boolN = nil;
static id       boolY = nil;

static const char *prefix =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
  "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
  "<plist version=\"1.0\">\n";

@class  GSSloppyXMLParser;

#define inrange(ch,min,max) ((ch)>=(min) && (ch)<=(max))
#define char2num(ch) \
inrange(ch,'0','9') \
? ((ch)-0x30) \
: (inrange(ch,'a','f') \
? ((ch)-0x57) : ((ch)-0x37))

/*
 * Cache classes.
 */
static Class	NSArrayClass;
static Class	NSDataClass;
static Class	NSDateClass;
static Class	NSDictionaryClass;
static Class	NSNumberClass;
static Class	NSStringClass;
static Class	NSMutableStringClass;
static Class	GSStringClass;
static Class	GSMutableStringClass;

@class	GSMutableDictionary;
@interface GSMutableDictionary : NSObject	// Help the compiler
@end


@interface GSXMLPListParser : NSObject
{
  NSXMLParser				*theParser;
  NSMutableString			*value;
  NSMutableArray			*stack;
  id					key;
  BOOL					inArray;
  BOOL					inDictionary;
  BOOL					inPCData;
  BOOL					parsed;
  BOOL					success;
  id					plist;
  NSPropertyListMutabilityOptions	opts;
}

- (id) initWithData: (NSData*)data
	 mutability: (NSPropertyListMutabilityOptions)options;
- (BOOL) parse;
- (void) parser: (NSXMLParser *)parser
  foundCharacters: (NSString *)string;
- (void) parser: (NSXMLParser *)parser
  didStartElement: (NSString *)elementName
  namespaceURI: (NSString *)namespaceURI
  qualifiedName: (NSString *)qualifiedName
  attributes: (NSDictionary *)attributeDict;
- (void) parser: (NSXMLParser *)parser
  didEndElement: (NSString *)elementName
  namespaceURI: (NSString *)namespaceURI
  qualifiedName: (NSString *)qName;
- (id) result;
- (void) unescape;
@end

@interface	GSSloppyXMLParser : NSXMLParser
@end

@implementation GSXMLPListParser

- (void) dealloc
{
  RELEASE(key);
  RELEASE(stack);
  RELEASE(plist);
  RELEASE(value);
  RELEASE(theParser);
  [super dealloc];
}

- (id) initWithData: (NSData*)data
	 mutability: (NSPropertyListMutabilityOptions)options
{
  if ((self = [super init]) != nil)
    {
      theParser = [[GSSloppyXMLParser alloc] initWithData: data];
      [theParser setDelegate: self];
      opts = options;
    }
  return self;
}

- (void) parser: (NSXMLParser *)parser
foundCharacters: (NSString *)string
{
  if (YES == inPCData)
    {
      [value appendString: string];
    }
  else
    {
      [value appendString: [string stringByTrimmingSpaces]];
    }
}

- (void) parser: (NSXMLParser *)parser
foundIgnorableWhitespace: (NSString *)string
{
  if (YES == inPCData)
    {
      [value appendString: string];
    }
}

- (void) parser: (NSXMLParser *)parser
  didStartElement: (NSString *)elementName
  namespaceURI: (NSString *)namespaceURI
  qualifiedName: (NSString *)qualifiedName
  attributes: (NSDictionary *)attributeDict
{
  if ([elementName isEqualToString: @"dict"] == YES)
    {
      NSMutableDictionary	*d;

      if (key == nil)
        {
          key = [[NSNull null] retain];
        }
      [stack addObject: key];
      DESTROY(key);
      d = [[NSMutableDictionary alloc] initWithCapacity: 10];
      [stack addObject: d];
      RELEASE(d);
      inDictionary = YES;
      inArray = NO;
    }
  else if ([elementName isEqualToString: @"array"] == YES)
    {
      NSMutableArray	*a;

      if (key == nil)
        {
          key = [[NSNull null] retain];
        }
      [stack addObject: key];
      DESTROY(key);
      a = [[NSMutableArray alloc] initWithCapacity: 10];
      [stack addObject: a];
      RELEASE(a);
      inArray = YES;
      inDictionary = NO;
    }
  else if ([elementName isEqualToString: @"plist"] == NO)
    {
      inPCData = YES;
    }
}

- (void) parser: (NSXMLParser *)parser
  didEndElement: (NSString *)elementName
  namespaceURI: (NSString *)namespaceURI
  qualifiedName: (NSString *)qName
{
  BOOL	inContainer = NO;

  inPCData = NO;
  if ([elementName isEqualToString: @"dict"] == YES)
    {
      inContainer = YES;
    }
  else if ([elementName isEqualToString: @"array"] == YES)
    {
      inContainer = YES;
    }

  if (inContainer)
    {
      if (opts != NSPropertyListImmutable)
	{
	  ASSIGN(plist, [stack lastObject]);
	}
      else
        {
          NSObject      *o = [stack lastObject];

          if ([o makeImmutable] == YES)
            {
              ASSIGN(plist, o);
            }
          else
            {
              ASSIGNCOPY(plist, o);
            }
	}
      [stack removeLastObject];
      inArray = NO;
      inDictionary = NO;
      ASSIGN(key, [stack lastObject]);
      [stack removeLastObject];
      if ((id)key == (id)[NSNull null])
        {
          DESTROY(key);
        }
      if ([stack count] > 0)
        {
	  id	last;

	  last = [stack lastObject];
	  if ([last isKindOfClass: NSArrayClass] == YES)
	    {
	      inArray = YES;
	    }
	  else if ([last isKindOfClass: NSDictionaryClass] == YES)
	    {
	      inDictionary = YES;
	    }
	}
    }
  else if ([elementName isEqualToString: @"key"] == YES)
    {
      [self unescape];
      ASSIGNCOPY(key, value);
      [value setString: @""];
      return;
    }
  else if ([elementName isEqualToString: @"data"])
    {
      NSData	*d;

      d = [GSMimeDocument decodeBase64:
	     [value dataUsingEncoding: NSASCIIStringEncoding]];
      if (opts == NSPropertyListMutableContainersAndLeaves)
	{
	  d = AUTORELEASE([d mutableCopy]);
	}
      ASSIGN(plist, d);
      if (d == nil)
	{
	  [parser abortParsing];
	  return;
	}
    }
  else if ([elementName isEqualToString: @"date"])
    {
      id	result;

      if ([value hasSuffix: @"Z"] == YES && [value length] == 20)
	{
	  result = [NSCalendarDate dateWithString: value
				   calendarFormat: @"%Y-%m-%dT%H:%M:%SZ"];
	}
      else
	{
	  result = [NSCalendarDate dateWithString: value
				   calendarFormat: @"%Y-%m-%d %H:%M:%S %z"];
	}
      ASSIGN(plist, result);
    }
  else if ([elementName isEqualToString: @"string"])
    {
      id	o;

      [self unescape];
      if (opts == NSPropertyListMutableContainersAndLeaves)
        {
	  o = [value mutableCopy];
	}
      else
        {
	  o = [value copy];
	}
      ASSIGN(plist, o);
      [o release];
    }
  else if ([elementName isEqualToString: @"integer"])
    {
      if ([value hasPrefix: @"-"])
        {
          ASSIGN(plist, [NSNumber numberWithLongLong: [value longLongValue]]);
        }
      else
        {
          ASSIGN(plist, [NSNumber numberWithUnsignedLongLong:
            (unsigned long long)[value longLongValue]]);
        }
    }
  else if ([elementName isEqualToString: @"real"])
    {
      ASSIGN(plist, [NSNumber numberWithDouble: strtod([value cString], NULL)]);
    }
  else if ([elementName isEqualToString: @"true"])
    {
      ASSIGN(plist, boolY);
    }
  else if ([elementName isEqualToString: @"false"])
    {
      ASSIGN(plist, boolN);
    }
  else if ([elementName isEqualToString: @"plist"])
    {
      [value setString: @""];
      return;
    }
  else // invalid tag
    {
      // NSLog(@"unrecognized tag <%@>", elementName);
      [parser abortParsing];
      return;
    }

  if (inArray == YES)
    {
      [[stack lastObject] addObject: plist];
    }
  else if (inDictionary == YES)
    {
      if (key == nil)
        {
	  [parser abortParsing];
	  return;
	}
      [(NSMutableDictionary*)[stack lastObject] setObject: plist forKey: key];
      DESTROY(key);
    }
  [value setString: @""];
}

- (BOOL) parse
{
  if (parsed == NO)
    {
      parsed = YES;
      stack = [[NSMutableArray alloc] initWithCapacity: 10];
      value = [[NSMutableString alloc] initWithCapacity: 50];
      success = [theParser parse];
    }
  return success;
}

- (id) result
{
  return plist;
}

- (void) unescape
{
  id	o;
  NSRange	r;

  /* Convert any \Uxxxx sequences to unicode characters.
   */
  r = NSMakeRange(0, [value length]);
  while (r.length >= 6)
    {
      r = [value rangeOfString: @"\\U" options: NSLiteralSearch range: r];
      if (r.length == 2 && [value length] >= r.location + 6)
	{
	  unichar	c;
	  unichar	v;

	  c = [value characterAtIndex: r.location + 2];
	  if (isxdigit(c))
	    {
	      v = char2num(c);
	      c = [value characterAtIndex: r.location + 3];
	      if (isxdigit(c))
		{
		  v <<= 4;
		  v |= char2num(c);
		  c = [value characterAtIndex: r.location + 4];
		  if (isxdigit(c))
		    {
		      v <<= 4;
		      v |= char2num(c);
		      c = [value characterAtIndex: r.location + 5];
		      if (isxdigit(c))
			{
			  v <<= 4;
			  v |= char2num(c);
			  o = [NSString alloc];
			  o = [o initWithCharacters: &v length: 1];
			  r.length += 4;
			  [value replaceCharactersInRange: r withString: o];
			  [o release];
			  r.location++;
			  r.length = 0;
			}
		    }
		}
	    }
	  r = NSMakeRange(NSMaxRange(r), [value length] - NSMaxRange(r));
	}
    }
}
@end




@interface GSBinaryPLParser : NSObject
{
  NSPropertyListMutabilityOptions	mutability;
  unsigned              _length;
  const unsigned char	*_bytes;
  NSData		*data;
  unsigned		offset_size;	// Number of bytes per table entry
  unsigned		index_size;	// Number of bytes per table entry
  unsigned		object_count;	// Number of objects
  unsigned		root_index;	// Index of root object
  unsigned		table_start;	// Start address of object table
  NSHashTable           *_stack; // The stack of objects we are currently parsing
}

- (id) initWithData: (NSData*)plData
	 mutability: (NSPropertyListMutabilityOptions)m;
- (id) rootObject;
- (id) objectAtIndex: (NSUInteger)index;

@end

@interface GSBinaryPLGenerator : NSObject
{
  NSMutableData *dest;
  NSMapTable 	*objectList;
  NSMutableArray *objectsToDoList;
  id root;

  // Number of bytes per object table index
  unsigned int index_size;
  // Number of bytes per object table entry
  unsigned int offset_size;

  unsigned int table_start;
  unsigned int table_size;
  unsigned int *table;
}

+ (void) serializePropertyList: (id)aPropertyList
                      intoData: (NSMutableData *)destination;
- (id) initWithPropertyList: (id)aPropertyList
                   intoData: (NSMutableData *)destination;
- (void) generate;
- (BOOL) storeObject: (id)object;
- (void) cleanup;

@end


static Class	plArray;
static id	(*plAdd)(id, SEL, id) = 0;

static Class	plDictionary;
static id	(*plSet)(id, SEL, id, id) = 0;

/* Bitmap of 'quotable' characters ... those characters which must be
 * inside a quoted string if written to an old style property list.
 */
static const unsigned char quotables[32] = {
  '\xff',
  '\xff',
  '\xff',
  '\xff',
  '\x85',
  '\x13',
  '\x00',
  '\x78',
  '\x00',
  '\x00',
  '\x00',
  '\x38',
  '\x01',
  '\x00',
  '\x00',
  '\xa8',
  '\xff',
  '\xff',
  '\xff',
  '\xff',
  '\xff',
  '\xff',
  '\xff',
  '\xff',
  '\xff',
  '\xff',
  '\xff',
  '\xff',
  '\xff',
  '\xff',
  '\xff',
  '\xff',
};

/* Bitmap of characters considered white space if in an old style property
 * list. This is the same as the set given by the isspace() function in the
 * POSIX locale, but (for cross-locale portability of property list files)
 * is fixed, rather than locale dependent.
 */
static const unsigned char whitespace[32] = {
  '\x00',
  '\x3f',
  '\x00',
  '\x00',
  '\x01',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
  '\x00',
};

#define IS_BIT_SET(a,i) ((((a) & (1<<(i)))) > 0)

#define GS_IS_QUOTABLE(X) IS_BIT_SET(quotables[(X)/8], (X) % 8)

#define GS_IS_WHITESPACE(X) IS_BIT_SET(whitespace[(X)/8], (X) % 8)

static NSCharacterSet *oldQuotables = nil;
static NSCharacterSet *xmlQuotables = nil;

typedef	struct	{
  const unsigned char	*ptr;
  unsigned	end;
  unsigned	pos;
  unsigned	lin;
  NSString	*err;
  NSPropertyListMutabilityOptions opt;
  BOOL		key;
  BOOL		old;
} pldata;

/*
 *	Property list parsing - skip whitespace keeping count of lines and
 *	regarding objective-c style comments as whitespace.
 *	Returns YES if there is any non-whitespace text remaining.
 */
static BOOL skipSpace(pldata *pld)
{
  unsigned char	c;

  while (pld->pos < pld->end)
    {
      c = pld->ptr[pld->pos];

      if (GS_IS_WHITESPACE(c) == NO)
	{
	  if (c == '/' && pld->pos < pld->end - 1)
	    {
	      /*
	       * Check for comments beginning '/' followed by '/' or '*'
	       */
	      if (pld->ptr[pld->pos + 1] == '/')
		{
		  pld->pos += 2;
		  while (pld->pos < pld->end)
		    {
		      c = pld->ptr[pld->pos];
		      if (c == '\n')
			{
			  break;
			}
		      pld->pos++;
		    }
		  if (pld->pos >= pld->end)
		    {
		      pld->err = @"reached end of string in comment";
		      return NO;
		    }
		}
	      else if (pld->ptr[pld->pos + 1] == '*')
		{
		  pld->pos += 2;
		  while (pld->pos < pld->end)
		    {
		      c = pld->ptr[pld->pos];
		      if (c == '\n')
			{
			  pld->lin++;
			}
		      else if (c == '*' && pld->pos < pld->end - 1
			&& pld->ptr[pld->pos+1] == '/')
			{
			  pld->pos++; /* Skip past '*'	*/
			  break;
			}
		      pld->pos++;
		    }
		  if (pld->pos >= pld->end)
		    {
		      pld->err = @"reached end of string in comment";
		      return NO;
		    }
		}
	      else
		{
		  return YES;
		}
	    }
	  else
	    {
	      return YES;
	    }
	}
      if (c == '\n')
	{
	  pld->lin++;
	}
      pld->pos++;
    }
  pld->err = @"reached end of string";
  return NO;
}

static inline id parseQuotedString(pldata* pld)
{
  unsigned	start = ++pld->pos;
  unsigned	escaped = 0;
  unsigned	shrink = 0;
  BOOL		hex = NO;
  NSString	*obj;

  while (pld->pos < pld->end)
    {
      unsigned char	c = pld->ptr[pld->pos];

      if (escaped)
	{
	  if (escaped == 1 && c >= '0' && c <= '7')
	    {
	      escaped = 2;
	      hex = NO;
	    }
	  else if (escaped == 1 && (c == 'u' || c == 'U'))
	    {
	      escaped = 2;
	      hex = YES;
	    }
	  else if (escaped > 1)
	    {
	      if (hex && isxdigit(c))
		{
		  shrink++;
		  escaped++;
		  if (escaped == 6)
		    {
		      escaped = 0;
		    }
		}
	      else if (c >= '0' && c <= '7')
		{
		  shrink++;
		  escaped++;
		  if (escaped == 4)
		    {
		      escaped = 0;
		    }
		}
	      else
		{
		  pld->pos--;
		  escaped = 0;
		}
	    }
	  else
	    {
	      escaped = 0;
	    }
	}
      else
	{
	  if (c == '\\')
	    {
	      escaped = 1;
	      shrink++;
	    }
	  else if (c == '"')
	    {
	      break;
	    }
	}
      if (c == '\n')
	pld->lin++;
      pld->pos++;
    }
  if (pld->pos >= pld->end)
    {
      pld->err = @"reached end of string while parsing quoted string";
      return nil;
    }
  if (pld->pos - start - shrink == 0)
    {
      obj = @"";
    }
  else
    {
      unsigned	length;
      unichar	*chars;
      unichar	*temp = NULL;
      unsigned	int temp_length = 0;
      unsigned	j;
      unsigned	k;

      if (!GSToUnicode(&temp, &temp_length, &pld->ptr[start],
		       pld->pos - start, NSUTF8StringEncoding,
		       NSDefaultMallocZone(), 0))
	{
	  pld->err = @"invalid utf8 data while parsing quoted string";
	  return nil;
	}
      length = temp_length - shrink;
      chars = NSAllocateCollectable(sizeof(unichar) * length, 0);
      escaped = 0;
      hex = NO;
      for (j = 0, k = 0; j < temp_length; j++)
	{
	  unichar c = temp[j];

	  if (escaped)
	    {
	      if (escaped == 1 && c >= '0' && c <= '7')
		{
		  chars[k] = c - '0';
		  hex = NO;
		  escaped++;
		}
	      else if (escaped == 1 && (c == 'u' || c == 'U'))
		{
		  chars[k] = 0;
		  hex = YES;
		  escaped++;
		}
	      else if (escaped > 1)
		{
		  if (hex && isxdigit(c))
		    {
		      chars[k] <<= 4;
		      chars[k] |= char2num(c);
		      escaped++;
		      if (escaped == 6)
			{
			  escaped = 0;
			  k++;
			}
		    }
		  else if (c >= '0' && c <= '7')
		    {
		      chars[k] <<= 3;
		      chars[k] |= (c - '0');
		      escaped++;
		      if (escaped == 4)
			{
			  escaped = 0;
			  k++;
			}
		    }
		  else
		    {
		      escaped = 0;
		      j--;
		      k++;
		    }
		}
	      else
		{
		  escaped = 0;
		  switch (c)
		    {
		      case 'a' : chars[k] = '\a'; break;
		      case 'b' : chars[k] = '\b'; break;
		      case 't' : chars[k] = '\t'; break;
		      case 'r' : chars[k] = '\r'; break;
		      case 'n' : chars[k] = '\n'; break;
		      case 'v' : chars[k] = '\v'; break;
		      case 'f' : chars[k] = '\f'; break;
		      default  : chars[k] = c; break;
		    }
		  k++;
		}
	    }
	  else
	    {
	      chars[k] = c;
	      if (c == '\\')
		{
		  escaped = 1;
		}
	      else
		{
		  k++;
		}
	    }
	}

      NSZoneFree(NSDefaultMallocZone(), temp);
      length = k;

      if (pld->key == NO
        && pld->opt == NSPropertyListMutableContainersAndLeaves)
	{
	  obj = [GSMutableString alloc];
	  obj = [obj initWithCharactersNoCopy: chars
				       length: length
				 freeWhenDone: YES];
	}
      else
	{
	  obj = [NSStringClass allocWithZone: NSDefaultMallocZone()];
	  obj = [obj initWithCharactersNoCopy: chars
				       length: length
				 freeWhenDone: YES];
	}
    }
  pld->pos++;
  return obj;
}

static inline id parseUnquotedString(pldata *pld)
{
  unsigned	start = pld->pos;
  unsigned	i;
  unsigned	length;
  id		obj;
  unichar	*chars;

  while (pld->pos < pld->end)
    {
      if (GS_IS_QUOTABLE(pld->ptr[pld->pos]) == YES)
	break;
      pld->pos++;
    }

  length = pld->pos - start;
  chars = NSAllocateCollectable(sizeof(unichar) * length, 0);
  for (i = 0; i < length; i++)
    {
      chars[i] = pld->ptr[start + i];
    }

  if (pld->key == NO
    && pld->opt == NSPropertyListMutableContainersAndLeaves)
    {
      obj = [GSMutableString alloc];
      obj = [obj initWithCharactersNoCopy: chars
				   length: length
			     freeWhenDone: YES];
    }
  else
    {
      obj = [NSStringClass allocWithZone: NSDefaultMallocZone()];
      obj = [obj initWithCharactersNoCopy: chars
				   length: length
			     freeWhenDone: YES];
    }
  return obj;
}

static id parsePlItem(pldata* pld)
{
  id	result = nil;
  BOOL	start = (pld->pos == 0 ? YES : NO);

  if (skipSpace(pld) == NO)
    {
      return nil;
    }
  switch (pld->ptr[pld->pos])
    {
      case '{':
	{
	  NSMutableDictionary	*dict;

	  dict = [[plDictionary allocWithZone: NSDefaultMallocZone()]
	    initWithCapacity: 0];
	  pld->pos++;
	  while (skipSpace(pld) == YES && pld->ptr[pld->pos] != '}')
	    {
	      id	key;
	      id	val;

	      pld->key = YES;
	      key = parsePlItem(pld);
	      pld->key = NO;
	      if (key == nil)
		{
		  return nil;
		}
	      if (skipSpace(pld) == NO)
		{
		  RELEASE(key);
		  RELEASE(dict);
		  return nil;
		}
	      if (pld->ptr[pld->pos] != '=')
		{
		  pld->err = @"unexpected character (wanted '=')";
		  RELEASE(key);
		  RELEASE(dict);
		  return nil;
		}
	      pld->pos++;
	      val = parsePlItem(pld);
	      if (val == nil)
		{
		  RELEASE(key);
		  RELEASE(dict);
		  return nil;
		}
	      if (skipSpace(pld) == NO)
		{
		  RELEASE(key);
		  RELEASE(val);
		  RELEASE(dict);
		  return nil;
		}
	      if (pld->ptr[pld->pos] == ';')
		{
		  pld->pos++;
		}
	      else if (pld->ptr[pld->pos] == '}')
		{
		  if (GSPrivateDefaultsFlag(GSMacOSXCompatible))
		    {
		      pld->err = @"unexpected character '}' (wanted ';')";
		      RELEASE(key);
		      RELEASE(val);
		      RELEASE(dict);
		      return nil;
		    }
		  else
		    {
	              NSWarnFLog(
		        @"Missing semicolon in dictionary at line %d char %d",
		        pld->lin + 1, pld->pos + 1);
		    }
		}
	      else
		{
		  pld->err = @"unexpected character (wanted ';' or '}')";
		  RELEASE(key);
		  RELEASE(val);
		  RELEASE(dict);
		  return nil;
		}
	      (*plSet)(dict, @selector(setObject:forKey:), val, key);
	      RELEASE(key);
	      RELEASE(val);
	    }
	  if (pld->pos >= pld->end)
	    {
	      pld->err = @"unexpected end of string when parsing dictionary";
	      RELEASE(dict);
	      return nil;
	    }
	  pld->pos++;
	  result = dict;
	  if (pld->opt == NSPropertyListImmutable)
	    {
              result = GS_IMMUTABLE(result);
	    }
	}
	break;

      case '(':
	{
	  NSMutableArray	*array;

	  array = [[plArray allocWithZone: NSDefaultMallocZone()]
	    initWithCapacity: 0];
	  pld->pos++;
	  while (skipSpace(pld) == YES && pld->ptr[pld->pos] != ')')
	    {
	      id	val;

	      val = parsePlItem(pld);
	      if (val == nil)
		{
		  RELEASE(array);
		  return nil;
		}
	      if (skipSpace(pld) == NO)
		{
		  RELEASE(val);
		  RELEASE(array);
		  return nil;
		}
	      if (pld->ptr[pld->pos] == ',')
		{
		  pld->pos++;
		}
	      else if (pld->ptr[pld->pos] != ')')
		{
		  pld->err = @"unexpected character (wanted ',' or ')')";
		  RELEASE(val);
		  RELEASE(array);
		  return nil;
		}
	      (*plAdd)(array, @selector(addObject:), val);
	      RELEASE(val);
	    }
	  if (pld->pos >= pld->end)
	    {
	      pld->err = @"unexpected end of string when parsing array";
	      RELEASE(array);
	      return nil;
	    }
	  pld->pos++;
	  result = array;
	  if (pld->opt == NSPropertyListImmutable)
	    {
              result = GS_IMMUTABLE(result);
	    }
	}
	break;

      case '<':
	pld->pos++;
	if (pld->pos < pld->end && pld->ptr[pld->pos] == '*')
	  {
	    const unsigned char	*ptr;
	    unsigned		min;
	    unsigned		len = 0;
	    unsigned		i;

	    pld->old = NO;
	    pld->pos++;
	    min = pld->pos;
	    ptr = &(pld->ptr[min]);
	    while (pld->pos < pld->end && pld->ptr[pld->pos] != '>')
	      {
		pld->pos++;
	      }
	    len = pld->pos - min;
	    if (len > 1)
	      {
		unsigned char	type = *ptr++;

		len--;
		// Allow for quoted values.
		if (len > 2 && '"' == ptr[0] && '"' == ptr[len - 1])
		  {
		    len -= 2;
		    ptr++;
		  }
		if (type == 'I')
		  {
		    char	buf[len+1];

		    for (i = 0; i < len; i++) buf[i] = (char)ptr[i];
		    buf[len] = '\0';
                    if ('-' == buf[0])
                      {
                        result = [[NSNumber alloc]
                          initWithLongLong: atoll(buf)];
                      }
                    else
                      {
                        result = [[NSNumber alloc]
                          initWithUnsignedLongLong: strtoull(buf, NULL, 10)];
                      }
		  }
		else if (type == 'B')
		  {
		    if (ptr[0] == 'Y')
		      {
			result = [boolY retain];
		      }
		    else if (ptr[0] == 'N')
		      {
			result = [boolN retain];
		      }
		    else
		      {
			pld->err = @"bad value for bool";
			return nil;
		      }
		  }
		else if (type == 'D')
		  {
		    unichar	buf[len];
		    unsigned	i;
		    NSString	*str;

		    for (i = 0; i < len; i++) buf[i] = ptr[i];
		    str = [[NSString alloc] initWithCharacters: buf
							length: len];
		    result = [[NSCalendarDate alloc] initWithString: str
		      calendarFormat: @"%Y-%m-%d %H:%M:%S %z"];
		    RELEASE(str);
		  }
		else if (type == 'R')
		  {
		    char	buf[len+1];

		    for (i = 0; i < len; i++) buf[i] = ptr[i];
		    buf[len] = '\0';
		    result = [[NSNumber alloc]
		      initWithDouble: strtod(buf, NULL)];
		  }
		else
		  {
		    pld->err = @"unrecognized type code after '<*'";
		    return nil;
		  }
	      }
	    else
	      {
		pld->err = @"missing type code after '<*'";
		return nil;
	      }
	    if (pld->pos >= pld->end)
	      {
		pld->err = @"unexpected end of string when parsing data";
		return nil;
	      }
	    if (pld->ptr[pld->pos] != '>')
	      {
		pld->err = @"unexpected character (wanted '>')";
		return nil;
	      }
	    pld->pos++;
	  }
	else if (pld->pos < pld->end && pld->ptr[pld->pos] == '[')
	  {
	    const unsigned char	*ptr;
	    unsigned		min;
	    unsigned		len;

	    pld->old = NO;
	    pld->pos++;
	    min = pld->pos;
	    ptr = &(pld->ptr[min]);
	    while (pld->pos < pld->end && pld->ptr[pld->pos] != ']')
	      {
		pld->pos++;
	      }
	    len = pld->pos - min;
	    if (pld->pos >= pld->end)
	      {
		pld->err = @"unexpected end of string when parsing data";
		return nil;
	      }
	    pld->pos++;
	    if (pld->pos >= pld->end)
	      {
		pld->err = @"unexpected end of string when parsing ']>'";
		return nil;
	      }
	    if (pld->ptr[pld->pos] != '>')
	      {
		pld->err = @"unexpected character (wanted '>')";
		return nil;
	      }
	    pld->pos++;
            if (0 == len)
              {
                if (pld->key == NO
                  && pld->opt == NSPropertyListMutableContainersAndLeaves)
                  {
                    result = [NSMutableData new];
                  }
                else
                  {
                    result = [NSData new];
                  }
              }
            else
              {
                NSData  *d;

                d = [[NSData alloc] initWithBytesNoCopy: (void*)ptr
                                                 length: len
                                           freeWhenDone: NO];
                NS_DURING
                  {
                    if (pld->key == NO
                      && pld->opt == NSPropertyListMutableContainersAndLeaves)
                      {
                        result = [[NSMutableData alloc]
                          initWithBase64EncodedData: d
                          options: NSDataBase64DecodingIgnoreUnknownCharacters];
                      }
                    else
                      {
                        result = [[NSData alloc]
                          initWithBase64EncodedData: d
                          options: NSDataBase64DecodingIgnoreUnknownCharacters];
                      }
                  }
                NS_HANDLER
                  {
                    pld->err = @"invalid base64 data";
                    result = nil;
                  }
                NS_ENDHANDLER
                RELEASE(d);
              }
	  }
	else
	  {
	    unsigned	        max = pld->pos;
	    unsigned char	*buf;
	    unsigned	        len = 0;

	    while (max < pld->end && pld->ptr[max] != '>')
	      {
                if (isxdigit(pld->ptr[max]))
                  {
                    len++;
                  }
		max++;
	      }
            if (max >= pld->end)
              {
		pld->err = @"unexpected end of string when parsing data";
		return nil;
              }
            buf = NSZoneMalloc(NSDefaultMallocZone(), (len + 1) / 2);
            // We permit (but do not require) space before hex octets
	    (void)skipSpace(pld);
            len = 0;
	    while (pld->pos < max
	      && isxdigit(pld->ptr[pld->pos])
	      && isxdigit(pld->ptr[pld->pos+1]))
	      {
		unsigned char	byte;

		byte = (char2num(pld->ptr[pld->pos])) << 4;
		pld->pos++;
		byte |= char2num(pld->ptr[pld->pos]);
		pld->pos++;
		buf[len++] = byte;
                // We permit (but do not require) space between/after hex octets
		(void)skipSpace(pld);
	      }
            if (pld->ptr[pld->pos] != '>')
              {
                NSZoneFree(NSDefaultMallocZone(), buf);
		pld->err = @"unexpected character (wanted '>')";
		return nil;
              }
	    pld->pos++;
            if (pld->key == NO
              && pld->opt == NSPropertyListMutableContainersAndLeaves)
              {
                result = [[NSMutableData alloc] initWithBytesNoCopy: buf
                                                             length: len
                                                       freeWhenDone: YES];
              }
            else
              {
                result = [[NSData alloc] initWithBytesNoCopy: buf
                                                      length: len
                                                freeWhenDone: YES];
              }
	  }
	break;

      case '"':
	result = parseQuotedString(pld);
	break;

      default:
	result = parseUnquotedString(pld);
	break;
    }
  if (YES == start && result != nil && nil == pld->err)
    {
      if (skipSpace(pld) == YES)
	{
	  pld->err = @"extra data after parsed string";
	  result = nil;		// Not at end of string.
	}
      else
        {
	  pld->err = nil;       // end expcted
        }
    }
  return result;
}

id
GSPropertyListFromStringsFormat(NSString *string)
{
  NSMutableDictionary	*dict;
  pldata		_pld;
  pldata		*pld = &_pld;
  NSData		*d;

  /*
   * An empty string is a nil property list.
   */
  if ([string length] == 0)
    {
      return nil;
    }

  d = [string dataUsingEncoding: NSUTF8StringEncoding];
  NSCAssert(d, @"Couldn't get utf8 data from string.");
  _pld.ptr = (unsigned char*)[d bytes];
  _pld.pos = 0;
  _pld.end = [d length];
  _pld.err = nil;
  _pld.lin = 0;
  _pld.opt = NSPropertyListImmutable;
  _pld.key = NO;
  _pld.old = YES;	// OpenStep style
  [NSPropertyListSerialization class];	// initialise

  dict = [[plDictionary allocWithZone: NSDefaultMallocZone()]
    initWithCapacity: 0];
  while (skipSpace(pld) == YES)
    {
      id	key;
      id	val;

      if (pld->ptr[pld->pos] == '"')
	{
	  key = parseQuotedString(pld);
	}
      else
	{
	  key = parseUnquotedString(pld);
	}
      if (key == nil)
	{
	  DESTROY(dict);
	  break;
	}
      if (skipSpace(pld) == NO)
	{
	  pld->err = @"incomplete final entry (no semicolon?)";
	  RELEASE(key);
	  DESTROY(dict);
	  break;
	}
      if (pld->ptr[pld->pos] == ';')
	{
	  pld->pos++;
	  (*plSet)(dict, @selector(setObject:forKey:), @"", key);
	  RELEASE(key);
	}
      else if (pld->ptr[pld->pos] == '=')
	{
	  pld->pos++;
	  if (skipSpace(pld) == NO)
	    {
	      RELEASE(key);
	      DESTROY(dict);
	      break;
	    }
	  if (pld->ptr[pld->pos] == '"')
	    {
	      val = parseQuotedString(pld);
	    }
	  else
	    {
	      val = parseUnquotedString(pld);
	    }
	  if (val == nil)
	    {
	      RELEASE(key);
	      DESTROY(dict);
	      break;
	    }
	  if (skipSpace(pld) == NO)
	    {
	      pld->err = @"missing final semicolon";
	      RELEASE(key);
	      RELEASE(val);
	      DESTROY(dict);
	      break;
	    }
	  (*plSet)(dict, @selector(setObject:forKey:), val, key);
	  RELEASE(key);
	  RELEASE(val);
	  if (pld->ptr[pld->pos] == ';')
	    {
	      pld->pos++;
	    }
	  else
	    {
	      pld->err = @"unexpected character (wanted ';')";
	      DESTROY(dict);
	      break;
	    }
	}
      else
	{
	  pld->err = @"unexpected character (wanted '=' or ';')";
	  RELEASE(key);
	  DESTROY(dict);
	  break;
	}
    }
  if (dict == nil && _pld.err != nil)
    {
      RELEASE(dict);
      [NSException raise: NSGenericException
		  format: @"Parse failed at line %d (char %d) - %@",
	_pld.lin + 1, _pld.pos + 1, _pld.err];
    }
  return AUTORELEASE(dict);
}



#include <math.h>

static void
encodeBase64(NSData *source, NSMutableData *dest)
{
  NSUInteger	length = [source length];

  if (length > 0)
    {
      NSUInteger	base = [dest length];
      NSUInteger	destlen = 4 * ((length + 2) / 3);

      [dest setLength: base + destlen];
      GSPrivateEncodeBase64((const uint8_t*)[source bytes],
        length, (uint8_t*)[dest mutableBytes] + base);
    }
}

/*
 * Output a string escaped for OpenStep style property lists.
 * The result is ascii data.
 */
static void
PString(NSString *obj, NSMutableData *output)
{
  unsigned	length;

  if ((length = [obj length]) == 0)
    {
      [output appendBytes: "\"\"" length: 2];
    }
  else if ([obj rangeOfCharacterFromSet: oldQuotables].length > 0
    || [obj characterAtIndex: 0] == '/')
    {
      unichar		*from;
      unichar		*end;
      unsigned char	*ptr;
      int		base = [output length];
      int		len = 0;
      GS_BEGINITEMBUF(ustring, (length * sizeof(unichar)), unichar)

      end = &ustring[length];
      [obj getCharacters: ustring];
      for (from = ustring; from < end; from++)
	{
	  switch (*from)
	    {
	      case '\t':
	      case '\r':
	      case '\n':
		len++;
		break;

	      case '\a':
	      case '\b':
	      case '\v':
	      case '\f':
	      case '\\':
	      case '"' :
		len += 2;
		break;

	      default:
		if (*from < 128)
		  {
		    if (isprint(*from) || *from == ' ')
		      {
			len++;
		      }
		    else
		      {
			len += 4;
		      }
		  }
		else
		  {
		    len += 6;
		  }
		break;
	    }
	}

      [output setLength: base + len + 2];
      ptr = [output mutableBytes] + base;
      *ptr++ = '"';
      for (from = ustring; from < end; from++)
	{
	  switch (*from)
	    {
	      case '\t':
	      case '\r':
	      case '\n':
		*ptr++ = *from;
		break;

	      case '\a': 	*ptr++ = '\\'; *ptr++ = 'a';  break;
	      case '\b': 	*ptr++ = '\\'; *ptr++ = 'b';  break;
	      case '\v': 	*ptr++ = '\\'; *ptr++ = 'v';  break;
	      case '\f': 	*ptr++ = '\\'; *ptr++ = 'f';  break;
	      case '\\': 	*ptr++ = '\\'; *ptr++ = '\\'; break;
	      case '"' : 	*ptr++ = '\\'; *ptr++ = '"';  break;

	      default:
		if (*from < 128)
		  {
		    if (isprint(*from) || *from == ' ')
		      {
			*ptr++ = *from;
		      }
		    else
		      {
			unichar	c = *from;

			*ptr++ = '\\';
			ptr[2] = (c & 7) + '0';
			c >>= 3;
			ptr[1] = (c & 7) + '0';
			c >>= 3;
			ptr[0] = (c & 7) + '0';
			ptr += 3;
		      }
		  }
		else
		  {
		    unichar	c = *from;

		    *ptr++ = '\\';
		    *ptr++ = 'U';
		    ptr[3] = (c & 15) > 9 ? (c & 15) + 55 : (c & 15) + 48;
		    c >>= 4;
		    ptr[2] = (c & 15) > 9 ? (c & 15) + 55 : (c & 15) + 48;
		    c >>= 4;
		    ptr[1] = (c & 15) > 9 ? (c & 15) + 55 : (c & 15) + 48;
		    c >>= 4;
		    ptr[0] = (c & 15) > 9 ? (c & 15) + 55 : (c & 15) + 48;
		    ptr += 4;
		  }
		break;
	    }
	}
      *ptr = '"';

      GS_ENDITEMBUF();
    }
  else
    {
      NSData	*d = [obj dataUsingEncoding: NSASCIIStringEncoding];

      [output appendData: d];
    }
}

/*
 * Output a string escaped for use in xml.
 * Result is utf8 data.
 */
static void
XString(NSString* obj, NSMutableData *output)
{
  static const char	*hexdigits = "0123456789ABCDEF";
  unsigned	end;

  end = [obj length];
  if (end == 0)
    {
      return;
    }

  if ([obj rangeOfCharacterFromSet: xmlQuotables].length > 0)
    {
      unichar	*base;
      unichar	*map;
      unichar	c;
      unsigned	len;
      unsigned	rpos;
      unsigned	wpos;
      BOOL	osx;

      osx = GSPrivateDefaultsFlag(GSMacOSXCompatible);

      base = NSAllocateCollectable(sizeof(unichar) * end, 0);
      [obj getCharacters: base];
      for (len = rpos = 0; rpos < end; rpos++)
	{
	  c = base[rpos];
	  switch (c)
	    {
	      case '&':
		len += 5;
		break;
	      case '<':
	      case '>':
		len += 4;
		break;
	      case '\'':
	      case '"':
		len += 6;
		break;

	      default:
		if ((c < 0x20 && (c != 0x09 && c != 0x0A && c != 0x0D))
		  || (c > 0xD7FF && c < 0xE000) || c > 0xFFFD)
		  {
		    if (osx)
		      {
			len += 8;	// Illegal in XML
		      }
		    else
		      {
			len += 6;	// Non-standard escape
		      }
		  }
		else
		  {
		    len++;
		  }
		break;
	    }
	}
      map = NSAllocateCollectable(sizeof(unichar) * len, 0);
      for (wpos = rpos = 0; rpos < end; rpos++)
	{
	  c = base[rpos];
	  switch (c)
	    {
	      case '&':
		map[wpos++] = '&';
		map[wpos++] = 'a';
		map[wpos++] = 'm';
		map[wpos++] = 'p';
		map[wpos++] = ';';
		break;
	      case '<':
		map[wpos++] = '&';
		map[wpos++] = 'l';
		map[wpos++] = 't';
		map[wpos++] = ';';
		break;
	      case '>':
		map[wpos++] = '&';
		map[wpos++] = 'g';
		map[wpos++] = 't';
		map[wpos++] = ';';
		break;
	      case '\'':
		map[wpos++] = '&';
		map[wpos++] = 'a';
		map[wpos++] = 'p';
		map[wpos++] = 'o';
		map[wpos++] = 's';
		map[wpos++] = ';';
		break;
	      case '"':
		map[wpos++] = '&';
		map[wpos++] = 'q';
		map[wpos++] = 'u';
		map[wpos++] = 'o';
		map[wpos++] = 't';
		map[wpos++] = ';';
		break;

	      default:
		if ((c < 0x20 && (c != 0x09 && c != 0x0A && c != 0x0D))
		  || (c > 0xD7FF && c < 0xE000) || c > 0xFFFD)
		  {
		    if (osx)
		      {
			/* Use XML style character entity references for
			 * OSX compatibility, even though this is an
			 * illegal character code and a standards complient
			 * XML parser will barf when it tries to read it.
			 * The OSX property list parser does not implement
		         * the XML standard and accepts at least some
			 * illegal characters.
			 */
			map[wpos++] = '&';
			map[wpos++] = '#';
			map[wpos++] = 'x';
			map[wpos++] = hexdigits[(c>>12) & 0xf];
			map[wpos++] = hexdigits[(c>>8) & 0xf];
			map[wpos++] = hexdigits[(c>>4) & 0xf];
			map[wpos++] = hexdigits[c & 0xf];
			map[wpos++] = ';';
		      }
		    else
		      {
			/* We need to be able to encode characters in a
			 * property list which are illegal in XML (even
			 * when encoded as numeric entities with the
			 * &#...; format.  So we use the same \Uxxxx
			 * format is in old style property lists.
			 */
			map[wpos++] = '\\';
			map[wpos++] = 'U';
			map[wpos++] = hexdigits[(c>>12) & 0xf];
			map[wpos++] = hexdigits[(c>>8) & 0xf];
			map[wpos++] = hexdigits[(c>>4) & 0xf];
			map[wpos++] = hexdigits[c & 0xf];
		      }
		  }
		else
		  {
		    map[wpos++] = c;
		  }
		break;
	    }
	}
      NSZoneFree(NSDefaultMallocZone(), base);
      obj = [[NSString alloc] initWithCharacters: map length: len];
      NSZoneFree(NSDefaultMallocZone(), map);
      [output appendData: [obj dataUsingEncoding: NSUTF8StringEncoding]];
      RELEASE(obj);
    }
  else
    {
      [output appendData: [obj dataUsingEncoding: NSUTF8StringEncoding]];
    }
}


static const char	*indentStrings[] = {
  "",
  "  ",
  "    ",
  "      ",
  "\t",
  "\t  ",
  "\t    ",
  "\t      ",
  "\t\t",
  "\t\t  ",
  "\t\t    ",
  "\t\t      ",
  "\t\t\t",
  "\t\t\t  ",
  "\t\t\t    ",
  "\t\t\t      ",
  "\t\t\t\t",
  "\t\t\t\t  ",
  "\t\t\t\t    ",
  "\t\t\t\t      ",
  "\t\t\t\t\t",
  "\t\t\t\t\t  ",
  "\t\t\t\t\t    ",
  "\t\t\t\t\t      ",
  "\t\t\t\t\t\t"
};

/**
 * obj is the object to be written out<br />
 * loc is the locale for formatting (or nil to indicate no formatting)<br />
 * lev is the level of indentation to use<br />
 * step is the indentation step (0 == 0, 1 = 2, 2 = 4, 3 = 8)<br />
 * x is an indicator for xml or old/new openstep property list format<br />
 * dest is the output buffer.
 */
static void
OAppend(id obj, NSDictionary *loc, unsigned lev, unsigned step,
  NSPropertyListFormat x, NSMutableData *dest)
{
  if (step > 3)
    {
      step = 3;
    }
  if (NSStringClass == 0)
    {
      [NSPropertyListSerialization class];      // Force initialisation
    }
  if ([obj isKindOfClass: NSStringClass])
    {
      if (x == NSPropertyListXMLFormat_v1_0)
	{
	  [dest appendBytes: "<string>" length: 8];
	  XString(obj, dest);
	  [dest appendBytes: "</string>\n" length: 10];
	}
      else
	{
	  PString(obj, dest);
	}
    }
  else if (obj == boolY)
    {
      if (x == NSPropertyListXMLFormat_v1_0)
        {
          [dest appendBytes: "<true/>\n" length: 8];
        }
      else if (x == NSPropertyListGNUstepFormat)
        {
          [dest appendBytes: "<*BY>" length: 5];
        }
      else
        {
          PString([obj description], dest);
        }
    }
  else if (obj == boolN)
    {
      if (x == NSPropertyListXMLFormat_v1_0)
        {
          [dest appendBytes: "<false/>\n" length: 9];
        }
      else if (x == NSPropertyListGNUstepFormat)
        {
          [dest appendBytes: "<*BN>" length: 5];
        }
      else
        {
          PString([obj description], dest);
        }
    }
  else if ([obj isKindOfClass: NSNumberClass])
    {
      const char	*t = [obj objCType];

      if (strchr("cCsSiIlLqQ", *t) != 0)
	{
	  if (x == NSPropertyListXMLFormat_v1_0)
	    {
	      [dest appendBytes: "<integer>" length: 9];
	      XString([obj stringValue], dest);
	      [dest appendBytes: "</integer>\n" length: 11];
	    }
	  else if (x == NSPropertyListGNUstepFormat)
	    {
	      [dest appendBytes: "<*I" length: 3];
	      [dest appendData:
	        [[obj stringValue] dataUsingEncoding: NSASCIIStringEncoding]];
	      [dest appendBytes: ">" length: 1];
	    }
	  else
	    {
	      PString([obj description], dest);
	    }
	}
      else
	{
	  if (x == NSPropertyListXMLFormat_v1_0)
	    {
	      [dest appendBytes: "<real>" length: 6];
	      XString([obj stringValue], dest);
	      [dest appendBytes: "</real>\n" length: 8];
	    }
	  else if (x == NSPropertyListGNUstepFormat)
	    {
	      [dest appendBytes: "<*R" length: 3];
	      [dest appendData:
	        [[obj stringValue] dataUsingEncoding: NSASCIIStringEncoding]];
	      [dest appendBytes: ">" length: 1];
	    }
	  else
	    {
	      PString([obj description], dest);
	    }
	}
    }
  else if ([obj isKindOfClass: NSDataClass])
    {
      if (NSPropertyListXMLFormat_v1_0 == x)
	{
	  [dest appendBytes: "<data>\n" length: 7];
	  encodeBase64(obj, dest);
	  [dest appendBytes: "</data>\n" length: 8];
	}
      else if (NSPropertyListGNUstepFormat == x)
        {
          [dest appendBytes: "<[" length: 2];
	  encodeBase64(obj, dest);
          [dest appendBytes: "]>" length: 2];
        }
      else
	{
	  const unsigned char	*src;
	  unsigned char		*dst;
	  int		length;
	  int		i;
	  int		j;

	  src = [obj bytes];
	  length = [obj length];
	  #define num2char(num) ((num) < 0xa ? ((num)+'0') : ((num)+0x57))

	  j = [dest length];
	  [dest setLength: j + 2*length+(length > 4 ? (length-1)/4+2 : 2)];
	  dst = [dest mutableBytes];
	  dst[j++] = '<';
	  for (i = 0; i < length; i++, j++)
	    {
	      dst[j++] = num2char((src[i]>>4) & 0x0f);
	      dst[j] = num2char(src[i] & 0x0f);
	      if ((i & 3) == 3 && i < length-1)
		{
		  /* if we've just finished a 32-bit int, print a space */
		  dst[++j] = ' ';
		}
	    }
	  dst[j] = '>';
	}
    }
  else if ([obj isKindOfClass: NSDateClass])
    {
      static NSTimeZone	*z = nil;

      if (z == nil)
	{
	  z = RETAIN([NSTimeZone timeZoneForSecondsFromGMT: 0]);
	}
      if (x == NSPropertyListXMLFormat_v1_0)
	{
	  [dest appendBytes: "<date>" length: 6];
	  obj = [obj descriptionWithCalendarFormat: @"%Y-%m-%dT%H:%M:%SZ"
	    timeZone: z locale: nil];
	  obj = [obj dataUsingEncoding: NSASCIIStringEncoding];
	  [dest appendData: obj];
	  [dest appendBytes: "</date>\n" length: 8];
	}
      else if (x == NSPropertyListGNUstepFormat)
	{
	  [dest appendBytes: "<*D" length: 3];
	  obj = [obj descriptionWithCalendarFormat: @"%Y-%m-%d %H:%M:%S %z"
	    timeZone: z locale: nil];
	  obj = [obj dataUsingEncoding: NSASCIIStringEncoding];
	  [dest appendData: obj];
	  [dest appendBytes: ">" length: 1];
	}
      else
	{
	  PString([obj description], dest);
	}
    }
  else if ([obj isKindOfClass: NSArrayClass])
    {
      const char	*iBaseString;
      const char	*iSizeString;
      unsigned	level = lev;

      if (level*step < sizeof(indentStrings)/sizeof(id))
	{
	  iBaseString = indentStrings[level*step];
	}
      else
	{
	  iBaseString
	    = indentStrings[sizeof(indentStrings)/sizeof(id)-1];
	}
      level++;
      if (level*step < sizeof(indentStrings)/sizeof(id))
	{
	  iSizeString = indentStrings[level*step];
	}
      else
	{
	  iSizeString
	    = indentStrings[sizeof(indentStrings)/sizeof(id)-1];
	}

      if (x == NSPropertyListXMLFormat_v1_0)
	{
	  NSEnumerator	*e;

	  [dest appendBytes: "<array>\n" length: 8];
	  e = [obj objectEnumerator];
	  while ((obj = [e nextObject]))
	    {
	      [dest appendBytes: iSizeString length: strlen(iSizeString)];
	      OAppend(obj, loc, level, step, x, dest);
	    }
	  [dest appendBytes: iBaseString length: strlen(iBaseString)];
	  [dest appendBytes: "</array>\n" length: 9];
	}
      else
	{
	  unsigned		count = [obj count];
	  unsigned		last = count - 1;
	  NSString		*plists[count];
	  unsigned		i;

	  if ([obj isProxy] == YES)
	    {
	      for (i = 0; i < count; i++)
		{
		  plists[i] = [obj objectAtIndex: i];
		}
	    }
	  else
	    {
	      [obj getObjects: plists];
	    }

	  if (loc == nil)
	    {
	      [dest appendBytes: "(" length: 1];
	      for (i = 0; i < count; i++)
		{
		  id	item = plists[i];

		  OAppend(item, nil, 0, step, x, dest);
		  if (i != last)
		    {
		      [dest appendBytes: ", " length: 2];
		    }
		}
	      [dest appendBytes: ")" length: 1];
	    }
	  else
	    {
	      [dest appendBytes: "(\n" length: 2];
	      for (i = 0; i < count; i++)
		{
		  id	item = plists[i];

		  [dest appendBytes: iSizeString length: strlen(iSizeString)];
		  OAppend(item, loc, level, step, x, dest);
		  if (i == last)
		    {
		      [dest appendBytes: "\n" length: 1];
		    }
		  else
		    {
		      [dest appendBytes: ",\n" length: 2];
		    }
		}
	      [dest appendBytes: iBaseString length: strlen(iBaseString)];
	      [dest appendBytes: ")" length: 1];
	    }
	}
    }
  else if ([obj isKindOfClass: NSDictionaryClass])
    {
      const char	*iBaseString;
      const char	*iSizeString;
      SEL		objSel = @selector(objectForKey:);
      IMP		myObj = [obj methodForSelector: objSel];
      unsigned		i;
      NSArray		*keyArray = [obj allKeys];
      unsigned		numKeys = [keyArray count];
      NSString		*plists[numKeys];
      NSString		*keys[numKeys];
      BOOL		canCompare = YES;
      Class		lastClass = 0;
      unsigned		level = lev;
      BOOL		isProxy = [obj isProxy];

      if (level*step < sizeof(indentStrings)/sizeof(id))
	{
	  iBaseString = indentStrings[level*step];
	}
      else
	{
	  iBaseString
	    = indentStrings[sizeof(indentStrings)/sizeof(id)-1];
	}
      level++;
      if (level*step < sizeof(indentStrings)/sizeof(id))
	{
	  iSizeString = indentStrings[level*step];
	}
      else
	{
	  iSizeString
	    = indentStrings[sizeof(indentStrings)/sizeof(id)-1];
	}

      if (isProxy == YES)
	{
	  for (i = 0; i < numKeys; i++)
	    {
	      keys[i] = [keyArray objectAtIndex: i];
	      plists[i] = [(NSDictionary*)obj objectForKey: keys[i]];
	    }
	}
      else
	{
	  [keyArray getObjects: keys];
	  for (i = 0; i < numKeys; i++)
	    {
	      plists[i] = (*myObj)(obj, objSel, keys[i]);
	    }
	}

      if (x == NSPropertyListXMLFormat_v1_0)
        {
	  /* This format can only use strings as keys.
	   */
	  for (i = 0; i < numKeys; i++)
	    {
	      if ([keys[i] isKindOfClass: NSStringClass] == NO)
	        {
		  [NSException raise: NSInvalidArgumentException
		    format: @"Bad key (%@) in property list: '%@'",
		    NSStringFromClass([keys[i] class]), keys[i]];
		}
	    }
	}
      else if (numKeys == 0)
	{
	  canCompare = NO;
	}
      else
	{
	  /* All keys must respond to -compare: for sorting.
	   */
	  lastClass = NSStringClass;
	  for (i = 0; i < numKeys; i++)
	    {
	      if (object_getClass(keys[i]) == lastClass)
		continue;
	      if ([keys[i] isKindOfClass: NSStringClass] == NO)
		{
		  canCompare = NO;
		  break;
		}
	      lastClass = object_getClass(keys[i]);
	    }
	}

      if (canCompare == YES)
	{
	  #define STRIDE_FACTOR 3
	  unsigned	c,d, stride;
	  BOOL		found;
	  NSComparisonResult	(*comp)(id, SEL, id) = 0;
	  unsigned int	count = numKeys;
	  #ifdef	GSWARN
	  BOOL		badComparison = NO;
	  #endif

	  stride = 1;
	  while (stride <= count)
	    {
	      stride = stride * STRIDE_FACTOR + 1;
	    }
	  lastClass = 0;
	  while (stride > (STRIDE_FACTOR - 1))
	    {
	      // loop to sort for each value of stride
	      stride = stride / STRIDE_FACTOR;
	      for (c = stride; c < count; c++)
		{
		  found = NO;
		  if (stride > c)
		    {
		      break;
		    }
		  d = c - stride;
		  while (!found)
		    {
		      id			a = keys[d + stride];
		      id			b = keys[d];
		      Class			x;
		      NSComparisonResult	r;

		      x = object_getClass(a);
		      if (x != lastClass)
			{
			  lastClass = x;
			  comp = (NSComparisonResult (*)(id, SEL, id))
			    [a methodForSelector: @selector(compare:)];
			}
		      r = (0 == comp) ? 0 : (*comp)(a, @selector(compare:), b);
		      if (r < 0)
			{
			  #ifdef	GSWARN
			  if (r != NSOrderedAscending)
			    {
			      badComparison = YES;
			    }
			  #endif

			  /* Swap keys and values.
			   */
			  keys[d + stride] = b;
			  keys[d] = a;
		          a = plists[d + stride];
		          b = plists[d];
			  plists[d + stride] = b;
			  plists[d] = a;

			  if (stride > d)
			    {
			      break;
			    }
			  d -= stride;
			}
		      else
			{
			  #ifdef	GSWARN
			  if (r != NSOrderedDescending
			    && r != NSOrderedSame)
			    {
			      badComparison = YES;
			    }
			  #endif
			  found = YES;
			}
		    }
		}
	    }
	  #ifdef	GSWARN
	  if (badComparison == YES)
	    {
	      NSWarnFLog(@"Detected bad return value from comparison");
	    }
	  #endif
	}

      if (x == NSPropertyListXMLFormat_v1_0)
	{
	  [dest appendBytes: "<dict>\n" length: 7];
	  for (i = 0; i < numKeys; i++)
	    {
	      [dest appendBytes: iSizeString length: strlen(iSizeString)];
	      [dest appendBytes: "<key>" length: 5];
	      XString(keys[i], dest);
	      [dest appendBytes: "</key>\n" length: 7];
	      [dest appendBytes: iSizeString length: strlen(iSizeString)];
	      OAppend(plists[i], loc, level, step, x, dest);
	    }
	  [dest appendBytes: iBaseString length: strlen(iBaseString)];
	  [dest appendBytes: "</dict>\n" length: 8];
	}
      else if (loc == nil)
	{
	  [dest appendBytes: "{" length: 1];
	  for (i = 0; i < numKeys; i++)
	    {
	      OAppend(keys[i], nil, 0, step, x, dest);
	      [dest appendBytes: " = " length: 3];
	      OAppend(plists[i], nil, 0, step, x, dest);
	      [dest appendBytes: "; " length: 2];
	    }
	  [dest appendBytes: "}" length: 1];
	}
      else
	{
	  [dest appendBytes: "{\n" length: 2];
	  for (i = 0; i < numKeys; i++)
	    {
	      [dest appendBytes: iSizeString length: strlen(iSizeString)];
	      OAppend(keys[i], loc, level, step, x, dest);
	      [dest appendBytes: " = " length: 3];
	      OAppend(plists[i], loc, level, step, x, dest);
	      [dest appendBytes: ";\n" length: 2];
	    }
	  [dest appendBytes: iBaseString length: strlen(iBaseString)];
	  [dest appendBytes: "}" length: 1];
	}
    }
  else
    {
      if (nil == obj)
	{
	  obj = @"(nil)";
	}
      if (x == NSPropertyListXMLFormat_v1_0)
	{
	  [dest appendBytes: "<string>" length: 8];
	  XString([obj description], dest);
	  [dest appendBytes: "</string>" length: 9];
	}
      else
	{
	  PString([obj description], dest);
	}
    }
}



static inline NSError*
create_error(int code, NSString* desc)
{
  return [NSError errorWithDomain: @"NSPropertyListSerialization"
                  code: code 
                  userInfo: [NSDictionary 
                                dictionaryWithObjectsAndKeys: desc,
                                NSLocalizedDescriptionKey, nil]];
}


@implementation	NSPropertyListSerialization

static BOOL	classInitialized = NO;

+ (void) initialize
{
  if (classInitialized == NO)
    {
      NSMutableCharacterSet	*s;

      classInitialized = YES;

      NSStringClass = [NSString class];
      NSMutableStringClass = [NSMutableString class];
      NSDataClass = [NSData class];
      NSDateClass = [NSDate class];
      NSNumberClass = [NSNumber class];
      NSArrayClass = [NSArray class];
      NSDictionaryClass = [NSDictionary class];
      GSStringClass = [GSString class];
      GSMutableStringClass = [GSMutableString class];

      plArray = [GSMutableArray class];
      plAdd = (id (*)(id, SEL, id))
	[plArray instanceMethodForSelector: @selector(addObject:)];

      plDictionary = [GSMutableDictionary class];
      plSet = (id (*)(id, SEL, id, id))
	[plDictionary instanceMethodForSelector: @selector(setObject:forKey:)];

      /* The '$', '.', '/' and '_' characters used to be OK to use in
       * property lists, but OSX now quotes them, so we follow suite.
       */
      s = [NSMutableCharacterSet new];
      [s addCharactersInString:
	@"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	@"abcdefghijklmnopqrstuvwxyz"];
      [s invert];
      oldQuotables = s;
      [[NSObject leakAt: &oldQuotables] release];

      s = [NSMutableCharacterSet new];
      [s addCharactersInString: @"&<>'\\\""];
      [s addCharactersInRange: NSMakeRange(0x0001, 0x001f)];
      [s removeCharactersInRange: NSMakeRange(0x0009, 0x0002)];
      [s removeCharactersInRange: NSMakeRange(0x000D, 0x0001)];
      [s addCharactersInRange: NSMakeRange(0xD800, 0x07FF)];
      [s addCharactersInRange: NSMakeRange(0xFFFE, 0x0002)];
      xmlQuotables = s;
      [[NSObject leakAt: &xmlQuotables] release];

      boolN = [[NSNumber numberWithBool: NO] retain];
      [[NSObject leakAt: &boolN] release];

      boolY = [[NSNumber numberWithBool: YES] retain];
      [[NSObject leakAt: &boolY] release];
    }
}

+ (NSData*) dataFromPropertyList: (id)aPropertyList
			  format: (NSPropertyListFormat)aFormat
		errorDescription: (NSString**)anErrorString
{
  NSError *error = nil;
  NSData *data = [self dataWithPropertyList: aPropertyList
                                     format: aFormat
                                    options: 0
                                      error: &error];

  if ((error != nil) && (anErrorString != NULL))
    {
      *anErrorString = [error description];
    }

  return data;
}

+ (NSData *) dataWithPropertyList: (id)aPropertyList
                           format: (NSPropertyListFormat)aFormat
                          options: (NSPropertyListWriteOptions)anOption
                            error: (out NSError**)error
{
  NSMutableData	*dest;
  NSDictionary	*loc;
  int		step = 2;

  if (nil == aPropertyList)
    {
      [NSException raise: NSInvalidArgumentException
                  format: @"[%@ +%@]: nil property list",
        NSStringFromClass(self), NSStringFromSelector(_cmd)];
    }

  loc = [[NSUserDefaults standardUserDefaults] dictionaryRepresentation];
  dest = [NSMutableData dataWithCapacity: 1024];

  if (aFormat == NSPropertyListXMLFormat_v1_0)
    {
      [dest appendBytes: prefix length: strlen(prefix)];
      OAppend(aPropertyList, loc, 0, step, aFormat, dest);
      [dest appendBytes: "</plist>" length: 8];
    }
  else if (aFormat == NSPropertyListGNUstepBinaryFormat)
    {
      [NSSerializer serializePropertyList: aPropertyList intoData: dest];
    }
  else if (aFormat == NSPropertyListBinaryFormat_v1_0)
    {
      [GSBinaryPLGenerator serializePropertyList: aPropertyList intoData: dest];
    }
  else
    {
      OAppend(aPropertyList, loc, 0, step, aFormat, dest);
    }
  return dest;
}

/**
 * <p>Make <var>obj</var> into a plist in <var>str</var>, using the locale <var>loc</var>.</p>
 *
 * <p>If <var>*str</var> is <code>nil</code>, create a <ref>GSMutableString</ref>.
 * Otherwise <var>*str</var> must be a GSMutableString.</p>
 * 
 * <p>Options:</p><ul>
 * <li><var>step</var> is the indent level.</li>
 * <li><var>forDescription</var> enables OpenStep formatting.</li>
 * <li><var>xml</var> enables XML formatting.</li>
 * </ul>
 */
GS_DECLARE void
GSPropertyListMake(id obj, NSDictionary *loc, BOOL xml,
  BOOL forDescription, unsigned step, id *str)
{
  NSString		*tmp;
  NSPropertyListFormat	style;
  NSMutableData		*dest;

  if (classInitialized == NO)
    {
      [NSPropertyListSerialization class];
    }

  if (*str == nil)
    {
      *str = AUTORELEASE([GSMutableString new]);
    }
  else if (object_getClass(*str) != [GSMutableString class])
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Illegal object (%@) at argument 0", *str];
    }

  if (forDescription)
    {
      style = NSPropertyListOpenStepFormat;
    }
  else if (xml == YES)
    {
      style = NSPropertyListXMLFormat_v1_0;
    }
  else if (GSPrivateDefaultsFlag(NSWriteOldStylePropertyLists) == YES)
    {
      style = NSPropertyListOpenStepFormat;
    }
  else
    {
      style = NSPropertyListGNUstepFormat;
    }

  dest = [NSMutableData dataWithCapacity: 1024];

  if (style == NSPropertyListXMLFormat_v1_0)
    {
      [dest appendBytes: prefix length: strlen(prefix)];
      OAppend(obj, loc, 0, step, style, dest);
      [dest appendBytes: "</plist>" length: 8];
    }
  else
    {
      OAppend(obj, loc, 0, step, style, dest);
    }
  tmp = [[NSString alloc] initWithData: dest encoding: NSASCIIStringEncoding];
  [*str appendString: tmp];
  RELEASE(tmp);
}

+ (BOOL) propertyList: (id)aPropertyList
     isValidForFormat: (NSPropertyListFormat)aFormat
{
// FIXME ... need to check properly.
  switch (aFormat)
    {
      case NSPropertyListGNUstepFormat:
	return YES;

      case NSPropertyListGNUstepBinaryFormat:
	return YES;

      case NSPropertyListOpenStepFormat:
	return YES;

      case NSPropertyListXMLFormat_v1_0:
	return YES;

      case NSPropertyListBinaryFormat_v1_0:
	return YES;

      default:
	[NSException raise: NSInvalidArgumentException
		    format: @"[%@ +%@]: unsupported format",
	  NSStringFromClass(self), NSStringFromSelector(_cmd)];
	return NO;
    }
}

+ (id) propertyListFromData: (NSData*)data
	   mutabilityOption: (NSPropertyListMutabilityOptions)anOption
		     format: (NSPropertyListFormat*)aFormat
	   errorDescription: (NSString**)anErrorString
{
  NSError *error = nil;
  id prop = [self propertyListWithData: data
                               options: anOption
                                format: aFormat
                                 error: &error];

  if ((error != nil) && (anErrorString != NULL))
    {
      *anErrorString = [error description];
    }

  return prop;
}

+ (id) propertyListWithData: (NSData*)data
                    options: (NSPropertyListReadOptions)anOption
                     format: (NSPropertyListFormat*)aFormat
                      error: (out NSError**)error
{
  NSPropertyListFormat	format = 0;
  NSString           *errorStr = nil;
  id			result = nil;
  const unsigned char	*bytes = 0;
  unsigned int		length = 0;

  if (data == nil)
    {
      errorStr = @"nil data argument passed to method";
    }
  else if ([data isKindOfClass: NSDataClass] == NO)
    {
      errorStr = @"non-NSData data argument passed to method";
    }
  else if ([data length] == 0)
    {
      errorStr = @"empty data argument passed to method";
    }
  else
    {
      bytes = [data bytes];
      length = [data length];
      if (length >= 8 && memcmp(bytes, "bplist00", 8) == 0)
        {
          format = NSPropertyListBinaryFormat_v1_0;
        }
      else if (bytes[0] == 0 || bytes[0] == 1)
        {
          format = NSPropertyListGNUstepBinaryFormat;
        }
      else
        {
          unsigned int index = 0;

          // Skip any leading white space.
          while (index < length && GS_IS_WHITESPACE(bytes[index]) == YES)
            {
              index++;
            }
          
          if (length - index > 2
            && bytes[index] == '<' && bytes[index+1] == '?')
            {
              // It begins with '<?' so it is xml
              format = NSPropertyListXMLFormat_v1_0;
            }
          else
            {
              // Assume openstep format unless we find otherwise.
              format = NSPropertyListOpenStepFormat;
            }
        }
    }

  if (errorStr == nil)
    {
      switch (format)
        {
        case NSPropertyListXMLFormat_v1_0:
          {
            GSXMLPListParser *parser;
            
            parser = [GSXMLPListParser alloc];
            parser = AUTORELEASE([parser initWithData: data
                                           mutability: anOption]);
            if ([parser parse] == YES)
              {
                result = AUTORELEASE(RETAIN([parser result]));
              }
            else
              { 
                errorStr = @"failed to parse as XML property list";
              }
          }
          break;
          
        case NSPropertyListOpenStepFormat:
          {
            pldata	_pld;
            
            _pld.ptr = bytes;
            _pld.pos = 0;
            _pld.end = length;
            _pld.err = nil;
            _pld.lin = 0;
            _pld.opt = anOption;
            _pld.key = NO;
            _pld.old = YES;	// OpenStep style
            
            result = AUTORELEASE(parsePlItem(&_pld));
            if (_pld.old == NO)
              {
                // Found some modern GNUstep extension in data.
                format = NSPropertyListGNUstepFormat;
              }
            if (_pld.err != nil)
              {
                errorStr = [NSString stringWithFormat:
		  @"Parse failed at line %d (char %d) - %@",
		  _pld.lin + 1, _pld.pos + 1, _pld.err];
              }
          }
          break;

        case NSPropertyListGNUstepBinaryFormat:
          if (anOption == NSPropertyListImmutable)
            {
              result = [NSDeserializer deserializePropertyListFromData: data
                                                     mutableContainers: NO];
            }
          else
            {
              result = [NSDeserializer deserializePropertyListFromData: data
                                                     mutableContainers: YES];
            }
          break;
          
        case NSPropertyListBinaryFormat_v1_0:
          {
            GSBinaryPLParser	*p = [GSBinaryPLParser alloc];
            
            p = [p initWithData: data mutability: anOption];
            result = [p rootObject];
            RELEASE(p);
          }
          break;
          
        default:
          errorStr = @"format not supported";
          break;
        }
    }

  /*
   * Done ... return all values.
   */
  if ((errorStr != nil) && (error != NULL))
    {
      *error = create_error(0, errorStr);
    }
  if (aFormat != 0)
    {
      *aFormat = format;
    }
  return result;
}

+ (id) propertyListWithStream: (NSInputStream*)stream
                      options: (NSPropertyListReadOptions)anOption
                       format: (NSPropertyListFormat*)aFormat
                        error: (out NSError**)error
{
  // FIXME
  return nil;
}

+ (NSInteger) writePropertyList: (id)aPropertyList
                       toStream: (NSOutputStream*)stream
                         format: (NSPropertyListFormat)aFormat
                        options: (NSPropertyListWriteOptions)anOption
                          error: (out NSError**)error
{
  // FIXME: The NSData operations should be implemented on top of this method, 
  // not the other way round,
  NSData *data = [self dataWithPropertyList: aPropertyList
                                     format: aFormat
                                    options: anOption
                                      error: error];

  return [stream write: [data bytes] maxLength: [data length]];
}

@end



@interface NSPropertyListSerialization (JavaCompatibility)
+ (NSData*) dataFromPropertyList: (id)anObject;
+ (id) propertyListFromData: (NSData*)aData;
+ (id) propertyListFromString: (NSString*)aString;
+ (NSString*) stringFromPropertyList: (id)anObject;
@end

@implementation NSPropertyListSerialization (JavaCompatibility)
+ (NSData*) dataFromPropertyList: (id)anObject
{
  NSString	*dummy;

  if (anObject == nil)
    {
      return nil;
    }
  return [self dataFromPropertyList: anObject
                             format: NSPropertyListGNUstepBinaryFormat
		   errorDescription: &dummy];
}
+ (id) propertyListFromData: (NSData*)aData
{
  NSPropertyListFormat	format;
  NSString		*dummy;

  if (aData == nil)
    {
      return nil;
    }
  return [self propertyListFromData: aData
		   mutabilityOption: NSPropertyListImmutable
			     format: &format
		   errorDescription: &dummy];
}
+ (id) propertyListFromString: (NSString*)aString
{
  NSData		*aData;
  NSPropertyListFormat	format;
  NSString		*dummy;

  aData = [aString dataUsingEncoding: NSUTF8StringEncoding];
  if (aData == nil)
    {
      return nil;
    }
  return [self propertyListFromData: aData
		   mutabilityOption: NSPropertyListImmutable
			     format: &format
		   errorDescription: &dummy];
}
+ (NSString*) stringFromPropertyList: (id)anObject
{
  NSString	*string;
  NSData	*aData;

  if (anObject == nil)
    {
      return nil;
    }
  aData = [self dataFromPropertyList: anObject
			      format: NSPropertyListGNUstepFormat
		    errorDescription: &string];
  string = [NSString alloc];
  string = [string initWithData: aData encoding: NSASCIIStringEncoding];
  return AUTORELEASE(string);
}
@end





@implementation GSBinaryPLParser
#define PUSH_OBJ(index) if (NO == [self _pushObject: index]) \
        { \
          [NSException raise: NSGenericException \
                      format: @"Cyclic object graph"]; \
        } 

#define POP_OBJ(index) do { [self _popObject: index]; } while (0)

- (void) dealloc
{
  DESTROY(data);
  DESTROY(_stack);
  [super dealloc];
}

- (id) initWithData: (NSData*)plData
	 mutability: (NSPropertyListMutabilityOptions)m
{
  _length = [plData length];
  if (_length < 32)
    {
      DESTROY(self);
    }
  else
    {
      unsigned char postfix[32];

      [plData getBytes: postfix range: NSMakeRange(_length - 32, 32)];
      offset_size = postfix[6];
      index_size = postfix[7];
      // FIXME: Looks like the following are actually 8 byte values.
      // But taking the lower 4 bytes is currently sufficient.
      object_count = (postfix[12] << 24) + (postfix[13] << 16)
	+ (postfix[14] << 8) + postfix[15];
      root_index = (postfix[20] << 24) + (postfix[21] << 16)
	+ (postfix[22] << 8) + postfix[23];
      table_start = (postfix[28] << 24) + (postfix[29] << 16)
	+ (postfix[30] << 8) + postfix[31];

      if (offset_size < 1 || offset_size > 4)
	{
	  unsigned saved = offset_size;

	  DESTROY(self);	// Bad format
	  [NSException raise: NSGenericException
		      format: @"Unknown offset size %d", saved];
	}
      else if (index_size < 1 || index_size > 4)
	{
	  unsigned saved = index_size;

	  DESTROY(self);	// Bad format
	  [NSException raise: NSGenericException
		      format: @"Unknown table size %d", saved];
	}
      else if (table_start + object_count * offset_size > _length)
        {
	  DESTROY(self);	// Bad format
	  [NSException raise: NSGenericException
		      format: @"Table size larger than supplied data"];
        }
      else if (root_index >= object_count)
	{
	  DESTROY(self);	// Bad format
	}
      else if (table_start > _length - 32)
	{
	  DESTROY(self);	// Bad format
	}
      else
	{
	  ASSIGN(data, plData);
	  _bytes = (const unsigned char*)[data bytes];
	  mutability = m;
	}
    }

  return self;
}

- (unsigned long) offsetForIndex: (unsigned)index
{
  if (index >= object_count)
    {
      [NSException raise: NSRangeException
		   format: @"Object table index out of bounds %d.", index];
      return 0; /* Not reached */
    }
  else
    {
      unsigned long     offset;
      unsigned          count;
      unsigned          pos;

      /* An offset is stored in big-endian byte order, so we can simply
       * read it byte by byte.
       */
      pos = table_start + index * offset_size;
      offset = _bytes[pos++];
      for (count = 1; count < offset_size; count++)
        {
          offset = (offset << 8) + _bytes[pos++];
        }
      return offset;
    }
}

- (unsigned) readObjectIndexAt: (unsigned*)counter
{
  unsigned      index;
  unsigned      count;
  unsigned      pos;

NSAssert(0 != counter, NSInvalidArgumentException);
  pos = *counter;
NSAssert(pos + index_size < _length, NSInvalidArgumentException);
  index = _bytes[pos++];
  for (count = 1; count < index_size; count++)
    {
      index = (index << 8) + _bytes[pos++];
    }
  *counter = pos;
  return index;
}

- (unsigned long) readCountAt: (unsigned*) counter
{
  unsigned long count;
  unsigned      pos;
  unsigned char c;

NSAssert(0 != counter, NSInvalidArgumentException);
  pos = *counter;
NSAssert(pos <= _length, NSInvalidArgumentException);
  c = _bytes[pos++];

  if (c == 0x10)
    {
NSAssert(pos + 1 < _length, NSInvalidArgumentException);
      count = _bytes[pos++];
      *counter = pos;
      return count;
    }
  else if (c == 0x11)
    {
NSAssert(pos + 2 < _length, NSInvalidArgumentException);
      count = _bytes[pos++];
      count = (count << 8) + _bytes[pos++];
      *counter = pos;
      return count;
    }
  // FIXME: Handling for 0x13 is wrong, but this value will only
  // show up for incorrect old GNUstep property lists.
  else if ((c == 0x12) || (c == 0x13))
    {
      unsigned len = 4;

NSAssert(pos + 4 < _length, NSInvalidArgumentException);
      count = _bytes[pos++];
      while (--len > 0)
        {
          count = (count << 8) + _bytes[pos++];
        }
      *counter = pos;
      return count;
    }
  else
    {
      //FIXME
      [NSException raise: NSGenericException
		   format: @"Unknown count type %d", c];
      return 0;
    }
}

- (id) rootObject
{
  return [self objectAtIndex: root_index];
}

- (BOOL)_pushObject: (NSUInteger)index
{
  uintptr_t val;
  if (nil == _stack)
    {
      _stack = NSCreateHashTable(NSIntegerHashCallBacks,
                                 5);
    }
  val = (index == 0) ? UINTPTR_MAX : (uintptr_t)(void*)index;
  // NSHashInsertIfAbsent() returns NULL on success
  return (NULL == NSHashInsertIfAbsent(_stack, (void*)val) ? YES : NO);
}

- (void)_popObject: (NSUInteger)index
{
  if (_stack != nil)
    {
      uintptr_t val = (index == 0) ? UINTPTR_MAX : (uintptr_t)(void*)index;
      NSHashRemove(_stack, (void*)val);
    }
}

- (id) objectAtIndex: (NSUInteger)index
{
  unsigned char	next;
  unsigned counter = [self offsetForIndex: index];
  id	        result = nil;

  [data getBytes: &next range: NSMakeRange(counter,1)];
  //NSLog(@"read object %d at index %d type %d", index, counter, next);
  counter += 1;

  if (next == 0x08)
    {
      // NO
      result = boolN;
    }
  else if (next == 0x09)
    {
      // YES
      result = boolY;
    }
  else if ((next >= 0x10) && (next < 0x17))
    {
      // integer number
      unsigned		len = 1 << (next - 0x10);
      unsigned long long num = 0;
      unsigned		i;
      unsigned char	buffer[16];

      if (len > sizeof(unsigned long long))
        {
          [NSException raise: NSInvalidArgumentException
                      format: @"Stored number too long (%d bytes) in property list", len];
       }

      [data getBytes: buffer range: NSMakeRange(counter, len)];
      for (i = 0; i < len; i++)
        {
	  num = (num << 8) + buffer[i];
	}
      result = [NSNumber numberWithLongLong: (long long)num];
    }
  else if (next == 0x22)
    {
      // float number
      NSSwappedFloat in;

      [data getBytes: &in range: NSMakeRange(counter, sizeof(float))];
      result = [NSNumber numberWithFloat: NSSwapBigFloatToHost(in)];
    }
  else if (next == 0x23)
    {
      // double number
      NSSwappedDouble in;

      [data getBytes: &in range: NSMakeRange(counter, sizeof(double))];
      result = [NSNumber numberWithDouble: NSSwapBigDoubleToHost(in)];
    }
  else if (next == 0x33)
    {
      NSSwappedDouble in;
      // Date
      NSDate *date;
      [data getBytes: &in range: NSMakeRange(counter, sizeof(double))];
      date = [NSDate dateWithTimeIntervalSinceReferenceDate:
	NSSwapBigDoubleToHost(in)];
      result = date;
    }
  else if ((next >= 0x40) && (next < 0x4F))
    {
      // short data
      unsigned len = next - 0x40;

NSAssert(counter + len <= _length, NSInvalidArgumentException);
      if (mutability == NSPropertyListMutableContainersAndLeaves)
	{
	  result = [NSMutableData dataWithBytes: _bytes + counter
					 length: len];
	}
      else
	{
	  result = [NSData dataWithBytes: _bytes + counter
                                  length: len];
	}
    }
  else if (next == 0x4F)
    {
      // long data
      unsigned long len;

      len = [self readCountAt: &counter];
NSAssert(counter + len <= _length, NSInvalidArgumentException);
      if (mutability == NSPropertyListMutableContainersAndLeaves)
	{
	  result = [NSMutableData dataWithBytes: _bytes + counter
					 length: len];
	}
      else
	{
	  result = [NSData dataWithBytes: _bytes + counter
                                  length: len];
	}
    }
  else if ((next >= 0x50) && (next < 0x5F))
    {
      NSString  *s;     // Short utf8 string
      unsigned	len;

      if (mutability == NSPropertyListMutableContainersAndLeaves)
	{
	  s = [NSMutableString alloc];
	}
      else
	{
	  s = [NSString alloc];
	}
      len = next - 0x50;
      s = [s initWithBytes: _bytes + counter
                    length: len
                  encoding: NSUTF8StringEncoding];
      result = [s autorelease];
    }
  else if (next == 0x5F)
    {
      NSString  *s;     // Long utf8 string
      unsigned	len;

      if (mutability == NSPropertyListMutableContainersAndLeaves)
	{
	  s = [NSMutableString alloc];
	}
      else
	{
	  s = [NSString alloc];
	}
      len = [self readCountAt: &counter];
      s = [s initWithBytes: _bytes + counter
                    length: len
                  encoding: NSUTF8StringEncoding];
      result = [s autorelease];
    }
  else if ((next >= 0x60) && (next < 0x6F))
    {
      NSString  *s;     // Short unicode string
      unsigned	len;

      if (mutability == NSPropertyListMutableContainersAndLeaves)
	{
	  s = [NSMutableString alloc];
	}
      else
	{
	  s = [NSString alloc];
	}
      len = next - 0x60;
      s = [s initWithBytes: _bytes + counter
                    length: len * sizeof(unichar)
                  encoding: NSUTF16BigEndianStringEncoding];
      result = [s autorelease];
    }
  else if (next == 0x6F)
    {
      NSString          *s;     // Short unicode string
      unsigned long     len;

      if (mutability == NSPropertyListMutableContainersAndLeaves)
	{
	  s = [NSMutableString alloc];
	}
      else
	{
	  s = [NSString alloc];
	}
      len = [self readCountAt: &counter];
      s = [s initWithBytes: _bytes + counter
                    length: len * sizeof(unichar)
                  encoding: NSUTF16BigEndianStringEncoding];
      result = [s autorelease];
    }
  else if (next == 0x80)
    {
      unsigned char	index;

      [data getBytes: &index range: NSMakeRange(counter,1)];
      result = [NSDictionary dictionaryWithObject:
				 [NSNumber numberWithInt: index]
			     forKey: @"CF$UID"];
    }
  else if (next == 0x81)
    {
      unsigned short	index;

      [data getBytes: &index range: NSMakeRange(counter,2)];
      index = NSSwapBigShortToHost(index);
      result = [NSDictionary dictionaryWithObject:
				 [NSNumber numberWithInt: index]
			     forKey: @"CF$UID"];
    }
  else if ((next >= 0xA0) && (next < 0xAF))
    {
      // short array
      unsigned	len = next - 0xA0;
      unsigned	i;
      id	objects[len];
      PUSH_OBJ(index);
      for (i = 0; i < len; i++)
        {
	  int oid = [self readObjectIndexAt: &counter];

	  objects[i] = [self objectAtIndex: oid];
	}
      POP_OBJ(index);
      if (mutability == NSPropertyListMutableContainersAndLeaves
	|| mutability == NSPropertyListMutableContainers)
	{
	  result = [NSMutableArray arrayWithObjects: objects count: len];
	}
      else
	{
	  result = [NSArray arrayWithObjects: objects count: len];
	}
    }
  else if (next == 0xAF)
    {
      // big array
      unsigned	long len;
      unsigned	i;
      id	*objects;

      len = [self readCountAt: &counter];
      objects = NSAllocateCollectable(sizeof(id) * len, NSScannedOption);
      PUSH_OBJ(index);
      for (i = 0; i < len; i++)
        {
	  int oid = [self readObjectIndexAt: &counter];

	  objects[i] = [self objectAtIndex: oid];
	}
      POP_OBJ(index);
      if (mutability == NSPropertyListMutableContainersAndLeaves
	|| mutability == NSPropertyListMutableContainers)
	{
	  result = [NSMutableArray arrayWithObjects: objects count: len];
	}
      else
	{
	  result = [NSArray arrayWithObjects: objects count: len];
	}
      NSZoneFree(NSDefaultMallocZone(), objects);
    }
  else if ((next >= 0xD0) && (next < 0xDF))
    {
      // dictionary
      unsigned	len = next - 0xD0;
      unsigned	i;
      id	keys[len];
      id	values[len];
      PUSH_OBJ(index);
      for (i = 0; i < len; i++)
        {
	  int oid = [self readObjectIndexAt: &counter];

	  keys[i] = [self objectAtIndex: oid];
	}
      for (i = 0; i < len; i++)
        {
	  int oid = [self readObjectIndexAt: &counter];

	  values[i] = [self objectAtIndex: oid];
	}

      POP_OBJ(index);
      if (mutability == NSPropertyListMutableContainersAndLeaves
	|| mutability == NSPropertyListMutableContainers)
	{
	  result = [NSMutableDictionary dictionaryWithObjects: values
						      forKeys: keys
							count: len];
	}
      else
	{
	  result = [NSDictionary dictionaryWithObjects: values
					       forKeys: keys
						 count: len];
	}
    }
  else if (next == 0xDF)
    {
      // big dictionary
      unsigned	long len;
      unsigned	i;
      id	*keys;
      id	*values;

      len = [self readCountAt: &counter];
      keys = NSAllocateCollectable(sizeof(id) * len * 2, NSScannedOption);
      values = keys + len;
      PUSH_OBJ(index);
      for (i = 0; i < len; i++)
        {
	  int oid = [self readObjectIndexAt: &counter];

	  keys[i] = [self objectAtIndex: oid];
	}

      for (i = 0; i < len; i++)
        {
	  int oid = [self readObjectIndexAt: &counter];

	  values[i] = [self objectAtIndex: oid];
	}
      POP_OBJ(index);
      if (mutability == NSPropertyListMutableContainersAndLeaves
	|| mutability == NSPropertyListMutableContainers)
	{
	  result = [NSMutableDictionary dictionaryWithObjects: values
						      forKeys: keys
							count: len];
	}
      else
	{
	  result = [NSDictionary dictionaryWithObjects: values
					       forKeys: keys
						 count: len];
	}
      NSZoneFree(NSDefaultMallocZone(), keys);
    }
  else
    {
      [NSException raise: NSGenericException
		   format: @"Unknown control byte = %d", next];
    }

  return result;
}
#undef PUSH_OBJ
#undef POP_OBJ
@end

/* Test two items for equality ... both are objects.
 * If either is an NSNumber, we insist that they are the same class
 * so that numbers with the same numeric value but different classes
 * are not treated as the same number (that confuses OSXs decoding).
 */
static BOOL
isEqualFunc(const void *item1, const void *item2,
  NSUInteger (*size)(const void *item))
{
  id	o1 = (id)item1;
  id	o2 = (id)item2;

  if ([o1 isKindOfClass: [NSNumber class]]
    || [o2 isKindOfClass: [NSNumber class]])
    {
      if ([o1 class] != [o2 class])
	{
	  return NO;
	}
    }
  return [o1 isEqual: o2];
}

@implementation GSBinaryPLGenerator

+ (void) serializePropertyList: (id)aPropertyList
		      intoData: (NSMutableData *)destination
{
  GSBinaryPLGenerator *gen;

  gen = [[GSBinaryPLGenerator alloc]
    initWithPropertyList: aPropertyList intoData: destination];
  [gen generate];
  RELEASE(gen);
}

- (id) initWithPropertyList: (id) aPropertyList
		   intoData: (NSMutableData *)destination
{
  ASSIGN(root, aPropertyList);
  ASSIGN(dest, destination);
  [dest setLength: 0];

  return self;
}

- (void) dealloc
{
  DESTROY(root);
  [self cleanup];
  DESTROY(dest);
  [super dealloc];
}

- (NSData*) data
{
  return dest;
}

- (void) setup
{
  NSPointerFunctions	*k;
  NSPointerFunctions	*v;

  [dest setLength: 0];
  if (index_size == 1)
    {
      table_size = 256;
    }
  else if (index_size == 2)
    {
      table_size = 256 * 256;
    }
  else if (index_size == 3)
    {
      table_size = 256 * 256 * 256;
    }
  else if (index_size == 4)
    {
      table_size = UINT_MAX;
    }

  table = NSZoneMalloc(0, table_size * sizeof(int));

  objectsToDoList = [[NSMutableArray alloc] init];
  k = [NSPointerFunctions pointerFunctionsWithOptions:
    NSPointerFunctionsObjectPersonality];
  [k setIsEqualFunction: isEqualFunc];
  v = [NSPointerFunctions pointerFunctionsWithOptions:
    NSPointerFunctionsIntegerPersonality|NSPointerFunctionsOpaqueMemory];
  objectList = [[NSMapTable alloc] initWithKeyPointerFunctions: k
					 valuePointerFunctions: v
						      capacity: 1000];

  [objectsToDoList addObject: root];
  [objectList setObject: (id)1 forKey: root];
}

- (void) cleanup
{
  DESTROY(objectsToDoList);
  DESTROY(objectList);
  if (table != NULL)
    {
      NSZoneFree(0, table);
      table = NULL;
    }
}

- (BOOL) writeObjects
{
  id object;
  const char *prefix = "bplist00";

  [dest appendBytes: prefix length: strlen(prefix)];

  while ([objectsToDoList count] != 0)
    {
      object = [objectsToDoList objectAtIndex: 0];
      if (NO == [self storeObject: object])
        {
          return NO;
        }
      [objectsToDoList removeObjectAtIndex: 0];
    }
  return YES;
}

- (BOOL) markOffset: (unsigned int) offset for: (id)object
{
  int oid;

  oid = (NSInteger)[objectList objectForKey: object];
  if (oid <= 0)
    {
      [NSException raise: NSGenericException
		   format: @"Unknown object %@.", object];
    }
  oid--;
  if (oid >= table_size)
    {
      return NO;
    }

  table[oid] = offset;
  return YES;
}

- (void) writeObjectTable
{
  unsigned int size;
  unsigned int len;
  unsigned int i;
  unsigned char *buffer;
  unsigned int last_offset;

  table_start = [dest length];
  // This is a bit too much, as the length
  // of the last object is added.
  last_offset = table_start;

  if (last_offset < 256)
    {
      offset_size = 1;
    }
  else if (last_offset < 256 * 256)
    {
      offset_size = 2;
    }
  else if (last_offset < 256 * 256 * 256)
    {
      offset_size = 3;
    }
  else
    {
      offset_size = 4;
    }

  len = [objectList count];
  size = offset_size * len;

  buffer = NSZoneMalloc(0, size);

  if (offset_size == 1)
    {
      for (i = 0; i < len; i++)
        {
	  unsigned char ci;

	  ci = table[i];
	  buffer[i] = ci;
	}
    }
  else if (offset_size == 2)
    {
      for (i = 0; i < len; i++)
        {
	  unsigned short si;

	  si = table[i];
	  buffer[2 * i] = (si >> 8);
	  buffer[2 * i + 1] = si % 256;
	}
    }
  else if (offset_size == 3)
    {
      for (i = 0; i < len; i++)
        {
	  unsigned int si;

	  si = table[i];
	  buffer[3 * i] = (si >> 16);
	  buffer[3 * i + 1] = (si >> 8) % 256;
	  buffer[3 * i + 2] = si % 256;
	}
    }
  else if (offset_size == 4)
    {
      for (i = 0; i < len; i++)
        {
	  unsigned int si;

	  si = table[i];
	  buffer[4 * i] = (si >> 24);
	  buffer[4 * i + 1] = (si >> 16) % 256;
	  buffer[4 * i + 2] = (si >> 8) % 256;
	  buffer[4 * i + 3] = si % 256;
	}
    }

  [dest appendBytes: buffer length: size];
  NSZoneFree(0, buffer);
}

- (void) writeMetaData
{
  unsigned char meta[32];
  unsigned int i;
  unsigned int len;

  for (i = 0; i < 32; i++)
    {
      meta[i] = 0;
    }

  meta[6] = offset_size;
  meta[7] = index_size;

  len = [objectList count];
  meta[12] = (len >> 24);
  meta[13] = (len >> 16) % 256;
  meta[14] = (len >> 8) % 256;
  meta[15] = len % 256;
  // root index is always 0, no need to write it
  meta[28] = (table_start >> 24);
  meta[29] = (table_start >> 16) % 256;
  meta[30] = (table_start >> 8) % 256;
  meta[31] = table_start % 256;

  [dest appendBytes: meta length: 32];
}

- (NSInteger) indexForObject: (id)object
{
  NSInteger index;

  index = (NSInteger)[objectList objectForKey: object];
  if (index <= 0)
    {
      index = [objectList count];
      [objectList setObject: (id)(++index) forKey: object];
      [objectsToDoList addObject: object];
    }

  return index - 1;
}

- (void) storeIndex: (NSInteger)index
{
  if (index_size == 1)
    {
      unsigned char oid;

      oid = index;
      [dest appendBytes: &oid length: 1];
    }
  else if (index_size == 2)
    {
      unsigned short oid;

      oid = NSSwapHostShortToBig(index);
      [dest appendBytes: &oid length: 2];
    }
  else if (index_size == 3)
    {
      unsigned char buffer[index_size];
      int i;
      unsigned num = index;

      for (i = index_size - 1; i >= 0; i--)
        {
	  buffer[i] = num & 0xFF;
          num >>= 8;
	}
      [dest appendBytes: buffer length: index_size];
    }
  else if (index_size == 4)
    {
      unsigned int oid;

      oid = NSSwapHostIntToBig(index);
      [dest appendBytes: &oid length: 4];
    }
  else
    {
      [NSException raise: NSGenericException
		   format: @"Unknown table size %d", index_size];
    }
}

- (void) storeCount: (unsigned int)count
{
  unsigned char code;

  if (count < 256)
    {
      unsigned char c;

      code = 0x10;
      [dest appendBytes: &code length: 1];
      c = count;
      [dest appendBytes: &c length: 1];
    }
  else if (count < 256 * 256)
    {
      unsigned short c;

      code = 0x11;
      [dest appendBytes: &code length: 1];
      c = count;
      c = NSSwapHostShortToBig(c);
      [dest appendBytes: &c length: 2];
    }
  else
    {
      code = 0x12;
      [dest appendBytes: &code length: 1];
      count = NSSwapHostIntToBig(count);
      [dest appendBytes: &count length: 4];
    }
}

- (void) storeData: (NSData*) data
{
  unsigned int len;
  unsigned char code;

  len = [data length];

  if (len < 0x0F)
    {
      code = 0x40 + len;
      [dest appendBytes: &code length: 1];
      [dest appendData: data];
    }
  else
    {
      code = 0x4F;
      [dest appendBytes: &code length: 1];
      [self storeCount: len];
      [dest appendData: data];
    }
}

- (void) storeString: (NSString*) string
{
  unsigned int len;
  NSData        *ascii;
  unsigned char code;

  len = [string length];

  ascii = [string dataUsingEncoding: NSASCIIStringEncoding
               allowLossyConversion: NO];
  if (ascii)
    {
      if (len < 0x0F)
	{
	  code = 0x50 + len;
	  [dest appendBytes: &code length: 1];
	  [dest appendData: ascii];
	}
      else
	{
	  code = 0x5F;
	  [dest appendBytes: &code length: 1];
	  [self storeCount: len];
	  [dest appendData: ascii];
	}
    }
  else
    {
      NSUInteger        offset;
      unichar           *buffer;

      if (len < 0x0F)
	{
	  code = 0x60 + len;
	  [dest appendBytes: &code length: 1];
	}
      else
        {
	  code = 0x6F;
	  [dest appendBytes: &code length: 1];
	  [self storeCount: len];
	}

      offset = [dest length];
      [dest setLength: offset + sizeof(unichar)*len];
      buffer = [dest mutableBytes] + offset;
      [string getCharacters: buffer];

#if     !GS_WORDS_BIGENDIAN
      /* Always store in big-endian, so if machine is little-endian,
       * perform byte-swapping.
       */
      {
        uint8_t *o = (uint8_t*)buffer;
	int     i;

	for (i = 0; i < len; i++)
	  {
	    uint8_t c = *o++;

	    o[-1] = *o;
            *o++ = c;
	  }
      }
#endif
    }
}

- (void) storeNumber: (NSNumber*) number
{
  const char *type;
  unsigned char code;

  type = [number objCType];

  switch (*type)
    {
      case 'c':
      case 'C':
      case 's':
      case 'S':
      case 'i':
      case 'I':
      case 'l':
      case 'L':
      case 'q':
      case 'Q':
        {
	  unsigned long long val;

	  val = [number unsignedLongLongValue];

	  // FIXME: We need a better way to determine boolean values!
	  if ((val == 0) && ((*type == 'c') || (*type == 'C')))
	    {
	      code = 0x08;
	      [dest appendBytes: &code length: 1];
	    }
	  else if ((val == 1) && ((*type == 'c') || (*type == 'C')))
	    {
	      code = 0x09;
	      [dest appendBytes: &code length: 1];
	    }
	  else if (val < 256)
	    {
	      unsigned char cval;

	      code = 0x10;
	      [dest appendBytes: &code length: 1];
	      cval = (unsigned char) val;
	      [dest appendBytes: &cval length: 1];
	    }
	  else if (val < 256 * 256)
	    {
	      unsigned short sval;

	      code = 0x11;
	      [dest appendBytes: &code length: 1];
	      sval = NSSwapHostShortToBig([number unsignedShortValue]);
	      [dest appendBytes: &sval length: 2];
	    }
	  else if (val <= UINT_MAX)
	    {
	      unsigned int ival;

	      code = 0x12;
	      [dest appendBytes: &code length: 1];
	      ival = NSSwapHostIntToBig([number unsignedIntValue]);
	      [dest appendBytes: &ival length: 4];
	    }
	  else
	    {
	      unsigned long long lval;

	      code = 0x13;
	      [dest appendBytes: &code length: 1];
	      lval = NSSwapHostLongLongToBig([number unsignedLongLongValue]);
	      [dest appendBytes: &lval length: 8];
	    }
	  break;
	}
      case 'f':
        {
	  NSSwappedFloat val = NSSwapHostFloatToBig([number floatValue]);

	  code = 0x22;
	  [dest appendBytes: &code length: 1];
	  [dest appendBytes: &val length: sizeof(float)];
	  break;
	}
      case 'd':
        {
	  NSSwappedDouble val = NSSwapHostDoubleToBig([number doubleValue]);

	  code = 0x23;
	  [dest appendBytes: &code length: 1];
	  [dest appendBytes: &val length: sizeof(double)];
	  break;
	}
      default:
	[NSException raise: NSGenericException
		    format: @"Attempt to store number with unknown ObjC type"];
    }
}

- (void) storeDate: (NSDate*) date
{
  unsigned char code;
  NSSwappedDouble out;

  code = 0x33;
  [dest appendBytes: &code length: 1];
  out = NSSwapHostDoubleToBig([date timeIntervalSinceReferenceDate]);
  [dest appendBytes: &out length: sizeof(double)];
}

- (void) storeArray: (NSArray*) array
{
  unsigned char code;
  unsigned int len;
  unsigned int i;

  len = [array count];

  if (len < 0x0F)
    {
      code = 0xA0 + len;
      [dest appendBytes: &code length: 1];
    }
  else
    {
      code = 0xAF;
      [dest appendBytes: &code length: 1];
      [self storeCount: len];
    }

  for (i = 0; i < len; i++)
    {
      id obj;
      NSInteger oid;

      obj = [array objectAtIndex: i];
      oid = [self indexForObject: obj];
      [self storeIndex: oid];
    }
}

- (void) storeDictionary: (NSDictionary*) dict
{
  unsigned char code;
  NSNumber *num;
  unsigned int i;

  num = [dict objectForKey: @"CF$UID"];
  if (num != nil)
    {
      // Special dictionary from keyed encoding
      unsigned int index;

      index = [num intValue];
      if (index < 256)
        {
	  unsigned char ci;

	  code = 0x80;
	  [dest appendBytes: &code length: 1];
	  ci = (unsigned char)index;
	  [dest appendBytes: &ci length: 1];
	}
      else
        {
	  unsigned short si;

	  code = 0x81;
	  [dest appendBytes: &code length: 1];
	  si = NSSwapHostShortToBig((unsigned short)index);
	  [dest appendBytes: &si length: 2];
	}
    }
  else
    {
      unsigned int len = [dict count];
      NSArray *keys = [dict allKeys];
      NSMutableArray *objects = [NSMutableArray arrayWithCapacity: len];
      id key;

      for (i = 0; i < len; i++)
        {
	  key = [keys objectAtIndex: i];
	  [objects addObject: [dict objectForKey: key]];
	}

      if (len < 0x0F)
        {
	  code = 0xD0 + len;
	  [dest appendBytes: &code length: 1];
	}
      else
        {
	  code = 0xDF;
	  [dest appendBytes: &code length: 1];
	  [self storeCount: len];
	}

      for (i = 0; i < len; i++)
        {
	  id obj;
	  NSInteger oid;

	  obj = [keys objectAtIndex: i];
	  oid = [self indexForObject: obj];
	  [self storeIndex: oid];
	}

      for (i = 0; i < len; i++)
        {
	  id obj;
	  NSInteger oid;

	  obj = [objects objectAtIndex: i];
	  oid = [self indexForObject: obj];
	  [self storeIndex: oid];
	}
    }
}

- (BOOL) storeObject: (id)object
{
  if (NO == [self markOffset: [dest length] for: object])
    {
      return NO;
    }

  if ([object isKindOfClass: NSStringClass])
    {
      [self storeString: object];
    }
  else if ([object isKindOfClass: NSDataClass])
    {
      [self storeData: object];
    }
  else if ([object isKindOfClass: NSNumberClass])
    {
      [self storeNumber: object];
    }
  else if ([object isKindOfClass: NSDateClass])
    {
      [self storeDate: object];
    }
  else if ([object isKindOfClass: NSArrayClass])
    {
      [self storeArray: object];
    }
  else if ([object isKindOfClass: NSDictionaryClass])
    {
      [self storeDictionary: object];
    }
  else
    {
      NSLog(@"Unknown object class %@", object);
    }
  return YES;
}

- (void) generate
{
  BOOL done = NO;

  index_size = 1;

  while (!done && (index_size <= 4))
    {
      NS_DURING
	{
	  [self setup];
	  done = [self writeObjects];
	}
      NS_HANDLER
	{
	}
      NS_ENDHANDLER
      if (NO == done)
        {
          [self cleanup];
          index_size += 1;
        }
    }

  [self writeObjectTable];
  [self writeMetaData];
}

@end
