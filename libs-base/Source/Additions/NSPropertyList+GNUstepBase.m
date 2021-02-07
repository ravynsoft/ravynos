/** Permit writing of OpenStep property lists.
   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by: Richard Frith-Macdonald <richard@brainstorm.co.uk>

   This file is part of the GNUstep Objective-C Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

   $Date: 2010-02-17 11:47:06 +0000 (Wed, 17 Feb 2010) $ $Revision: 29657 $
*/

#import "common.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSCharacterSet.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDate.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSPropertyList.h"
#import "Foundation/NSString.h"
#import "Foundation/NSTimeZone.h"
#import "Foundation/NSUserDefaults.h"
#import "Foundation/NSValue.h"

#import "GNUstepBase/GSObjCRuntime.h"

#if	defined(NeXT_Foundation_LIBRARY)

static IMP		originalImp = 0;

static NSCharacterSet	*quotables = nil;
static Class		NSArrayClass;
static Class		NSDataClass;
static Class		NSDateClass;
static Class		NSDictionaryClass;
static Class		NSNumberClass;
static Class		NSStringClass;

static void
setup()
{
  if (quotables == nil)
    {
      NSMutableCharacterSet	*s;

      s = [[NSCharacterSet characterSetWithCharactersInString:
	@"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	@"abcdefghijklmnopqrstuvwxyz$./_"]
	mutableCopy];
      [s invert];
      quotables = [s copy];
      [s release];

      NSArrayClass = [NSArray class];
      NSDataClass = [NSData class];
      NSDateClass = [NSDate class];
      NSDictionaryClass = [NSDictionary class];
      NSNumberClass = [NSNumber class];
      NSStringClass = [NSString class];
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
  else if ([obj rangeOfCharacterFromSet: quotables].length > 0
    || [obj characterAtIndex: 0] == '/')
    {
      unichar		tmp[length <= 1024 ? length : 0];
      unichar		*ustring;
      unichar		*from;
      unichar		*end;
      unsigned char	*ptr;
      int		base = [output length];
      int		len = 0;

      if (length <= 1024)
	{
	  ustring = tmp;
	}
      else
	{
	  ustring = NSAllocateCollectable(sizeof(unichar) * length, 0);
	}
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
      *ptr++ = '"';

      if (ustring != tmp)
	{
	  NSZoneFree(NSDefaultMallocZone(), ustring);
	}
    }
  else
    {
      NSData	*d = [obj dataUsingEncoding: NSASCIIStringEncoding];

      [output appendData: d];
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
 * dest is the output buffer.
 */
static void
OAppend(id obj, NSDictionary *loc, unsigned lev, unsigned step,
  NSMutableData *dest)
{
  if ([obj isKindOfClass: NSStringClass])
    {
      PString(obj, dest);
    }
  else if ([obj isKindOfClass: NSNumberClass])
    {
      const char	*t = [obj objCType];

      if (*t ==  'c' || *t == 'C')
	{
	  BOOL	val = [obj boolValue];

	  if (val == YES)
	    {
	      PString([obj description], dest);
	    }
	  else
	    {
	      PString([obj description], dest);
	    }
	}
      else if (strchr("sSiIlLqQ", *t) != 0)
	{
	  PString([obj description], dest);
	}
      else
	{
	  PString([obj description], dest);
	}
    }
  else if ([obj isKindOfClass: NSDataClass])
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
      dst[j++] = '>';
    }
  else if ([obj isKindOfClass: NSDateClass])
    {
      static NSTimeZone	*z = nil;

      if (z == nil)
	{
	  z = RETAIN([NSTimeZone timeZoneForSecondsFromGMT: 0]);
	}
      obj = [obj descriptionWithCalendarFormat: @"%Y-%m-%d %H:%M:%S %z"
	timeZone: z locale: nil];
      PString(obj, dest);
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

		OAppend(item, nil, 0, step, dest);
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
		OAppend(item, loc, level, step, dest);
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
      id (*myObj)(id,SEL,id)
        = (id(*)(id,SEL,id))[obj methodForSelector: objSel];
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

      if (numKeys == 0)
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
		      r = (*comp)(a, @selector(compare:), b);
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
	}

      if (loc == nil)
	{
	  [dest appendBytes: "{" length: 1];
	  for (i = 0; i < numKeys; i++)
	    {
	      OAppend(keys[i], nil, 0, step, dest);
	      [dest appendBytes: " = " length: 3];
	      OAppend(plists[i], nil, 0, step, dest);
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
	      OAppend(keys[i], loc, level, step, dest);
	      [dest appendBytes: " = " length: 3];
	      OAppend(plists[i], loc, level, step, dest);
	      [dest appendBytes: ";\n" length: 2];
	    }
	  [dest appendBytes: iBaseString length: strlen(iBaseString)];
	  [dest appendBytes: "}" length: 1];
	}
    }
  else
    {
      NSString	*cls;

      if (obj == nil)
	{
	  obj = @"(nil)";
	  cls = @"(nil)";
	}
      else
	{
	  cls = NSStringFromClass([obj class]);
	}
      PString([obj description], dest);
    }
}


@interface NSPropertyListSerialization (GNUstepAdditions)
+ (NSData*) _dataFromPropertyList: (id)aPropertyList
			   format: (NSPropertyListFormat)aFormat
		 errorDescription: (NSString**)anErrorString;
+ (void) load;
@end

@implementation NSPropertyListSerialization (GNUstepAdditions)
+ (NSData*) _dataFromPropertyList: (id)aPropertyList
			   format: (NSPropertyListFormat)aFormat
		 errorDescription: (NSString**)anErrorString
{
  if (aFormat == NSPropertyListOpenStepFormat)
    {
      NSMutableData	*dest;
      NSDictionary	*loc;
      int		step = 2;

      loc = [[NSUserDefaults standardUserDefaults] dictionaryRepresentation];
      dest = [NSMutableData dataWithCapacity: 1024];

      if (nil == quotables)
	{
	  setup();
	}
      OAppend(aPropertyList, loc, 0, step > 3 ? 3 : step, dest);
      return dest;
    }
  return (*(id(*)(id,SEL,id,id,id))originalImp)
    (self, _cmd, aPropertyList, aFormat, anErrorString);
}

+ (void) load
{
  Method	replacementMethod;

  replacementMethod = class_getClassMethod(self,
    @selector(_dataFromPropertyList:format:errorDescription:));

  originalImp = class_replaceMethod(object_getClass(self), 
    @selector(dataFromPropertyList:format:errorDescription:),
    method_getImplementation(replacementMethod),
    method_getTypeEncoding(replacementMethod));
}
@end

#endif
