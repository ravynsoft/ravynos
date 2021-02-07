/** <title>NSAttributedStringAdditions</title>

   <abstract>Categories which add capabilities to NSAttributedString</abstract>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: July 1999
   Modifications: Fred Kiefer <FredKiefer@gmx.de>
   Date: June 2000
   
   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/ 

#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSCharacterSet.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSError.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSRange.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>

#import "AppKit/NSAttributedString.h"
#import "AppKit/NSDocumentController.h"
#import "AppKit/NSParagraphStyle.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSTextAttachment.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSFileWrapper.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSFontDescriptor.h"
#import "AppKit/NSFontManager.h"
// For the colour name spaces
#import "AppKit/NSGraphics.h"
#import "AppKit/NSTextTable.h"

#import "GNUstepGUI/GSTextConverter.h"
#import "GSGuiPrivate.h"

/* Cache class pointers to avoid the expensive lookup by string. */ 
static Class dictionaryClass = nil;
static Class stringClass = nil;

/* A character set containing characters that separate words.  */
static NSCharacterSet *wordBreakCSet = nil;
/* A character set containing characters that are legal within words.  */
static NSCharacterSet *wordCSet = nil;
/* Character sets containing characters that are white space and
   not white space */
static NSCharacterSet *whiteCSet = nil;
static NSCharacterSet *nonWhiteCSet = nil;
/* A String containing the attachment character */
static NSString *attachmentString = nil;


/* This function initializes all the previous cached values. */
static void cache_init_real(void)
{
  NSMutableCharacterSet *m;
  NSCharacterSet *cset;
  unichar ch = NSAttachmentCharacter;
  
  /* Initializes Class pointer cache */
  dictionaryClass = [NSDictionary class];
  stringClass = [NSString class];
  
  /* Initializes wordBreakCSet */
  m = [NSMutableCharacterSet new];
  cset = [NSCharacterSet whitespaceAndNewlineCharacterSet];
  [m formUnionWithCharacterSet: cset];
  cset = [NSCharacterSet punctuationCharacterSet];
  [m formUnionWithCharacterSet: cset];
  cset = [NSCharacterSet controlCharacterSet];
  [m formUnionWithCharacterSet: cset];
  cset = [NSCharacterSet illegalCharacterSet];
  [m formUnionWithCharacterSet: cset];
  [m addCharactersInString: @"<>"];
  [m removeCharactersInString: @"_"];
  wordBreakCSet = [m copy];
  RELEASE (m);
  
  /* Initializes wordCSet */
  wordCSet = [[wordBreakCSet invertedSet] copy];

  /* Initializes white space and non-white space character sets */
  whiteCSet = [[NSCharacterSet whitespaceCharacterSet] copy];
  nonWhiteCSet = [[whiteCSet invertedSet] copy];
  
  /* Initializes attachmentString */
  attachmentString = [stringClass stringWithCharacters: &ch length: 1];
  RETAIN (attachmentString);  
}

/* This inline function calls cache_init_real () the first time it is
   invoked, and does nothing afterwards.  Thus we get both speed
   (cache_init is inlined and only compares a pointer to nil when the
   cache has been initialized) and limit memory consumption (we are
   not copying everywhere the real initialization code, which is in
   cache_real_init (), which is not inlined.).*/
static inline void cache_init(void)
{
  if (dictionaryClass == nil)
    {
      cache_init_real ();
    }
}

/* Return the class that handles format from the first bundle it finds */
static 
Class converter_bundles(NSString *format, BOOL producer)
{
  Class converter_class = Nil;
  NSEnumerator *benum;
  NSString *dpath;

  /* Find the bundle paths */
  benum = [NSStandardLibraryPaths() objectEnumerator];
  while ((dpath = [benum nextObject]))
    {
      NSEnumerator *direnum;
      NSString *path;
      dpath = [dpath stringByAppendingPathComponent: @"Bundles"];
      dpath = [dpath stringByAppendingPathComponent: @"TextConverters"];
      if ([[NSFileManager defaultManager] fileExistsAtPath: dpath])
        direnum = [[NSFileManager defaultManager] enumeratorAtPath: dpath];
      else
        direnum = nil;
      while (direnum && (path = [direnum nextObject]))
	{
	  Class bclass;
	  NSString *full_path;
	  NSBundle *aBundle;
	  if ([[path pathExtension] isEqual: @"bundle"] == NO)
	    continue;
	  full_path = [dpath stringByAppendingPathComponent: path];
	  aBundle = [NSBundle bundleWithPath: full_path];
	  if (aBundle && ((bclass = [aBundle principalClass])))
	    {
	      if ([bclass respondsToSelector: 
			    @selector(classForFormat: producer: )])
		{
		  converter_class = (Class)[bclass classForFormat: format
						   producer: producer];
		}
	      else
		{
		  NSString *converter_name;
		  if (producer)
		    {
		      converter_name
			= [format stringByAppendingString: @"Producer"];
		    }
		  else
		    {
		      converter_name
			= [format stringByAppendingString: @"Consumer"];
		    }
		  converter_class = [aBundle classNamed: converter_name];
		}
	    }	 
	  if (converter_class)
	    break;
	}
      if (converter_class)
	break;
    }
  return converter_class;
}

/*
  Return a suitable converter for the text format supplied as argument.
  If producer is YES a class capable of writing that format is returned,
  otherwise a class able to read the format is returned.
 */
static Class converter_class(NSString *format, BOOL producer)
{
  static NSMutableDictionary *p_classes = nil;
  static NSMutableDictionary *c_classes = nil;
  Class found;

  if (producer)
    {
      if (p_classes == nil)
	p_classes = [NSMutableDictionary new];

      found = [p_classes objectForKey: format];
      if (found == Nil)
        {
	  found = converter_bundles(format, producer);
	  if (found != Nil)
	    NSDebugLog(@"Found converter %@ for format %@", found, format);
	  if (found != Nil)
	    [p_classes setObject: found forKey: format];
	}
      return found;
    }
  else 
    {
      if (c_classes == nil)
	c_classes = [NSMutableDictionary new];

      found = [c_classes objectForKey: format];
      if (found == Nil)
        {
	  found = converter_bundles(format, producer);
	  if (found != Nil)
	    NSDebugLog(@"Found converter %@ for format %@", found, format);
	  if (found != Nil)
	    [c_classes setObject: found forKey: format];
	}
      return found;
    }

  return Nil;
}

static inline NSError*
create_error(int code, NSString* desc)
{
  return [NSError errorWithDomain: @"NSAttributedString"
                  code: code 
                  userInfo: [NSDictionary 
                                dictionaryWithObjectsAndKeys: desc,
                                NSLocalizedDescriptionKey, nil]];
}

@implementation NSAttributedString (AppKit)

+ (NSArray *) textFileTypes
{
  // FIXME: Apply service filters
  return [self textUnfilteredFileTypes];
}

+ (NSArray *) textPasteboardTypes
{
  // FIXME: Apply service filters
  return [self textUnfilteredPasteboardTypes];
}

+ (NSArray *) textTypes
{
  // FIXME: Apply service filters
  return [self textUnfilteredTypes];
}

+ (NSArray *) textUnfilteredFileTypes
{
  return [NSArray arrayWithObjects: @"txt",  @"rtf", @"rtfd", @"html", nil];
}

+ (NSArray *) textUnfilteredPasteboardTypes
{
  return [NSArray arrayWithObjects: NSStringPboardType, NSRTFPboardType, 
		  NSRTFDPboardType, NSHTMLPboardType, nil];
}

+ (NSArray *) textUnfilteredTypes
{
  return [NSArray arrayWithObjects: @"public.plain-text",
		  @"public.rtf",
		  @"com.apple.rtfd",
		  @"public.html",
		  /*
		  @"public.xml",
		  @"com.apple.webarchive",
		  @"com.microsoft.word.doc",
		  @"com.microsoft.word.wordml",
		  @"org.openxmlformats.wordprocessingml.document",
		  @"org.oasis-open.opendocument.text",
		  @"com.apple.traditional-mac-plain-text",
		  */
		  nil];
}

+ (NSAttributedString *) attributedStringWithAttachment: 
                                            (NSTextAttachment *)attachment
{
  NSDictionary *attributes;

  cache_init ();

  attributes = [dictionaryClass dictionaryWithObject: attachment
				forKey: NSAttachmentAttributeName];
  
  return AUTORELEASE ([[self alloc] initWithString: attachmentString
				    attributes: attributes]);
}

- (BOOL) containsAttachments
{
  NSRange aRange;

  cache_init ();

  aRange = [[self string] rangeOfString: attachmentString];

  if (aRange.length > 0)
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

- (NSDictionary *) fontAttributesInRange: (NSRange)range
{
  NSDictionary	*all;
  static SEL	sel = 0;
  IMP		objForKey;
  id		objects[8];
  id		keys[8];
  int		count = 0;

  if (NSMaxRange(range) > [self length])
    {
      [NSException raise: NSRangeException
		  format: @"RangeError in method -fontAttributesInRange: "];
    }
  all = [self attributesAtIndex: range.location
	      effectiveRange: &range];

  if (sel == 0)
    {
      sel = @selector (objectForKey: );
    }
  objForKey = [all methodForSelector: sel];
  
#define NSATT_GET_ATTRIBUTE(attribute) \
  keys[count] = attribute; \
  objects[count] = (*objForKey) (all, sel, keys[count]); \
  if (objects[count] != nil) count++; 

  NSATT_GET_ATTRIBUTE (NSFontAttributeName);
  NSATT_GET_ATTRIBUTE (NSForegroundColorAttributeName);
  NSATT_GET_ATTRIBUTE (NSBackgroundColorAttributeName);
  NSATT_GET_ATTRIBUTE (NSUnderlineStyleAttributeName);
  NSATT_GET_ATTRIBUTE (NSSuperscriptAttributeName);
  NSATT_GET_ATTRIBUTE (NSBaselineOffsetAttributeName);
  NSATT_GET_ATTRIBUTE (NSKernAttributeName);
  NSATT_GET_ATTRIBUTE (NSLigatureAttributeName);

#undef NSATT_GET_ATTRIBUTE

  cache_init ();
  
  return [dictionaryClass dictionaryWithObjects: objects
			  forKeys: keys
			  count: count];
}

- (NSDictionary*) rulerAttributesInRange: (NSRange)range
{
  id style;

  cache_init ();

  if (NSMaxRange (range) > [self length])
    {
      [NSException raise: NSRangeException
		   format: @"RangeError in method -rulerAttributesInRange: "];
    }
  
  style = [self attribute: NSParagraphStyleAttributeName
		atIndex: range.location
		effectiveRange: &range];

  if (style != nil)
    {
      return [dictionaryClass dictionaryWithObject: style
			      forKey: NSParagraphStyleAttributeName];
    }
  
  return [dictionaryClass dictionary];
}

- (NSUInteger) lineBreakByHyphenatingBeforeIndex: (NSUInteger)location
				     withinRange: (NSRange)aRange
{
  // FIXME
  return [self lineBreakBeforeIndex: location
                        withinRange: aRange];
}

- (NSUInteger) lineBreakBeforeIndex: (NSUInteger)location
			withinRange: (NSRange)aRange
{
  NSString *str = [self string];
  NSUInteger length = [str length];
  NSRange scanRange;
  NSRange startRange;
  
  cache_init ();

  if (NSMaxRange (aRange) > length || location > length)
    {
      [NSException raise: NSRangeException
	format: @"RangeError in method -lineBreakBeforeIndex: withinRange: "];
    }

  if (!NSLocationInRange (location, aRange))
    {
      return NSNotFound;
    }
  
  scanRange = NSMakeRange (aRange.location, location - aRange.location);
  startRange = [str rangeOfCharacterFromSet: wordBreakCSet
		    options: NSBackwardsSearch | NSLiteralSearch
		    range: scanRange];
  while (startRange.length > 0 && startRange.location > 0
    && [str characterAtIndex: startRange.location] == '\''
    && [wordCSet characterIsMember: 
      [str characterAtIndex: startRange.location-1]])
    {
      location = startRange.location - 1;
      scanRange = NSMakeRange (0, location);
      startRange = [str rangeOfCharacterFromSet: wordBreakCSet
	options: NSBackwardsSearch|NSLiteralSearch range: scanRange];
    }
  if (startRange.length == 0)
    {
      return NSNotFound;
    }
  else
    {
      return NSMaxRange (startRange);
    }
}

- (NSRange) doubleClickAtIndex: (NSUInteger)location
{
  NSString *str = [self string];
  NSUInteger length = [str length];
  NSRange  scanRange;
  NSRange  startRange;
  NSRange  endRange;
  NSCharacterSet *breakCSet;

  cache_init ();

  if (location > length)
    {
      [NSException raise: NSRangeException
		  format: @"RangeError in method -doubleClickAtIndex: "];
    }

  /*
   * Double clicking on a white space character selects all surrounding
   * white space. Otherwise, if the location lies between words, a double
   * click selects only the character actually clicked on.
   */
  if ([whiteCSet characterIsMember: [str characterAtIndex: location]])
    {
      breakCSet = nonWhiteCSet;
    }
  else if ([wordBreakCSet characterIsMember: [str characterAtIndex: location]])
    {
      if (location == 0 || location == length - 1
	|| [str characterAtIndex: location] != '\''
	|| ! [wordCSet characterIsMember: [str characterAtIndex: location - 1]]
	|| ! [wordCSet characterIsMember: [str characterAtIndex: location + 1]])
	{
	  return NSMakeRange(location, 1);
	}
      breakCSet = wordBreakCSet;
    }
  else
    {
      breakCSet = wordBreakCSet;
    }

  scanRange = NSMakeRange (0, location);
  startRange = [str rangeOfCharacterFromSet: breakCSet
				    options: NSBackwardsSearch|NSLiteralSearch
				      range: scanRange];
  /*
   * Don't treat single quotes embedded within a word as break characters.
   * Note: The loop condition is always false when breakCSet==nonWhiteSetCSet.
   */
  while (startRange.length > 0
    && startRange.location > 0 && startRange.location < length - 1
    && [str characterAtIndex: startRange.location] == '\''
    && [wordCSet characterIsMember: 
      [str characterAtIndex: startRange.location - 1]]
    && [wordCSet characterIsMember: 
      [str characterAtIndex: startRange.location + 1]])
    {
      location = startRange.location - 1;
      scanRange = NSMakeRange (0, location);
      startRange = [str rangeOfCharacterFromSet: wordBreakCSet
	options: NSBackwardsSearch|NSLiteralSearch range: scanRange];
    }

  scanRange = NSMakeRange (location, length - location);
  endRange = [str rangeOfCharacterFromSet: breakCSet
				  options: NSLiteralSearch
				    range: scanRange];
  /*
   * Don't treat single quotes embedded within a word as break characters.
   * Note: The loop condition is always false when breakCSet==nonWhiteSetCSet.
   */
  while (endRange.length > 0
    && endRange.location > 0 && endRange.location < length - 1
    && [str characterAtIndex: endRange.location] == '\''
    && [wordCSet characterIsMember: 
      [str characterAtIndex: endRange.location - 1]]
    && [wordCSet characterIsMember: 
      [str characterAtIndex: endRange.location + 1]])
    {
      location = endRange.location + 1;
      scanRange = NSMakeRange (location, length - location);
      endRange = [str rangeOfCharacterFromSet: wordBreakCSet
	options: NSLiteralSearch range: scanRange];
    }

  if (startRange.length == 0)
    {
      location = 0;
    }
  else
    {
      location = NSMaxRange (startRange);
    }

  if (endRange.length == 0)
    {
      length = length - location;
    }
  else
    {
      length = endRange.location - location;
    }
  return NSMakeRange (location, length);
}

- (NSUInteger) nextWordFromIndex: (NSUInteger)location
                         forward: (BOOL)isForward
{
  NSString *str = [self string];
  NSUInteger length = [str length];
  NSRange range;

  if (location > length)
    {
      [NSException raise: NSRangeException
	format: @"RangeError in method -nextWordFromIndex: forward: "];
    }

  /* Please note that we consider ' a valid word separator.  This is
     what Emacs does and is perfectly correct.  If you want to change
     the word separators, the right approach is to use a different
     character set for word separators - the following code should be
     unchanged whatever characters you use to separate words.  */
  cache_init ();

  if (isForward)
    {
      /* What we want to do is: move forward to the next chunk of
	 non-word separator characters, skip them all, and return the
	 location just after them.  */

      if (location == length)
	{
	  return length;
	}

      /* Move forward to the next non-word separator.  */
      range = NSMakeRange (location, length - location);
      range = [str rangeOfCharacterFromSet: wordCSet
		                   options: NSLiteralSearch
                          	     range: range];
      if (range.location == NSNotFound)
	{
	  return length;
	}
      /* rangeOfCharacterFromSet: options: range: only returns the range
	 of the first non-word-separator character ... we want to skip
	 them all!  So we need to search again, this time for the
	 first word-separator character, and return the first such
	 character.  */
      range = NSMakeRange (range.location, length - range.location);
      range = [str rangeOfCharacterFromSet: wordBreakCSet
		                   options: NSLiteralSearch
                          	     range: range];
      if (range.location == NSNotFound)
	{
	  return length;
	}

      return range.location;
    }
  else
    {
      /* What we want to do is: move backward to the next chunk of
	 non-word separator characters, skip them all, and return the
	 location just at the beginning of the chunk.  */

      if (location == 0)
	{
	  return 0;
	}

      /* Move backward to the next non-word separator.  */
      range = NSMakeRange (0, location);
      range = [str rangeOfCharacterFromSet: wordCSet
		                   options: NSBackwardsSearch | NSLiteralSearch
		                     range: range];
      if (range.location == NSNotFound)
	{
	  return 0;
	}

      /* rangeOfCharacterFromSet: options: range: only returns the range
	 of the first non-word-separator character ... we want to skip
	 them all!  So we need to search again, this time for the
	 first word-separator character. */
      range = NSMakeRange (0, range.location);
      range = [str rangeOfCharacterFromSet: wordBreakCSet
		                   options: NSBackwardsSearch | NSLiteralSearch
                          	     range: range];
      if (range.location == NSNotFound)
	{
	  return 0;
	}
      
      return NSMaxRange (range);
    }
}

- (id) initWithRTFDFileWrapper: (NSFileWrapper *)wrapper
            documentAttributes: (NSDictionary **)dict
{
  return [self initWithRTFD: [wrapper serializedRepresentation]
                 documentAttributes: dict];
}

- (id) initWithRTFD: (NSData*)data
 documentAttributes: (NSDictionary**)dict
{
  NSDictionary *options;

  options = [NSDictionary dictionaryWithObject: NSRTFDTextDocumentType
                          forKey: NSDocumentTypeDocumentOption];
  return [self initWithData: data
               options: options
               documentAttributes: dict
               error: NULL];
}

- (id) initWithRTF: (NSData *)data
  documentAttributes: (NSDictionary **)dict
{
  NSDictionary *options;

  options = [NSDictionary dictionaryWithObject: NSRTFTextDocumentType
                          forKey: NSDocumentTypeDocumentOption];
  return [self initWithData: data
               options: options
               documentAttributes: dict
               error: NULL];
}

- (id) initWithHTML: (NSData *)data
 documentAttributes: (NSDictionary **)dict
{
  return [self initWithHTML: data
	       baseURL: nil
	       documentAttributes: dict];
}

- (id) initWithHTML: (NSData *)data
            baseURL: (NSURL *)base
 documentAttributes: (NSDictionary **)dict
{
  NSDictionary *options = nil;

  if (base != nil)
    options = [NSDictionary dictionaryWithObject: base
					  forKey: NSBaseURLDocumentOption];

  return [self initWithHTML: data
               options: options
               documentAttributes: dict];
}

- (id) initWithHTML: (NSData *)data
            options: (NSDictionary *)options
 documentAttributes: (NSDictionary **)dict
{
  if (options == nil)
    {
      options = [NSDictionary dictionaryWithObject: NSHTMLTextDocumentType
                                 forKey: NSDocumentTypeDocumentOption];
    }
  else if ([options objectForKey: NSDocumentTypeDocumentOption] == nil)
    {
      options = AUTORELEASE([options mutableCopy]);
      [(NSMutableDictionary*)options setObject: NSHTMLTextDocumentType 
                             forKey: NSDocumentTypeDocumentOption];
    }

  /*
    The converter should support:
    NSHTMLTextDocumentType
    @"public.html"
    @"html"
   */
  return [self initWithData: data
               options: options
               documentAttributes: dict
               error: NULL];
}

- (id) initWithDocFormat: (NSData *)data
      documentAttributes: (NSDictionary **)dict
{
  NSDictionary *options;

  options = [NSDictionary dictionaryWithObject: NSDocFormatTextDocumentType
                          forKey: NSDocumentTypeDocumentOption];
  return [self initWithData: data
               options: options
               documentAttributes: dict
               error: NULL];
}

- (id) initWithData: (NSData *)data
            options: (NSDictionary *)options
 documentAttributes: (NSDictionary **)dict
              error: (NSError **)error
{
  NSString *type = [options objectForKey: NSDocumentTypeDocumentOption];
  Class converter;

  if (data == nil)
    {
      if (error)
	*error = create_error(0, NSLocalizedString(@"No data specified for data loading.",
						   @"Error description"));
      RELEASE(self);
      return nil;
    }

  if (type == nil)
    {
      // Make sure this buffer is long enough
      char prefix[14];
      NSUInteger l = [data length];
      if (l < sizeof(prefix))
	{
	  [data getBytes: prefix length: l];
	  prefix[l] = 0;
	}
      else
	{
	  [data getBytes: prefix length: sizeof(prefix)];
	}

      // The list of file types below was derived from Apache's conf/magic file
      // FIXME extend the list
      if (strncmp(prefix, "{\\rtf", 5) == 0)
	{
	  type = NSRTFTextDocumentType;
	}
      else if (strncasecmp(prefix, "<!doctype html", 14) == 0 ||
	       strncasecmp(prefix, "<head", 5) == 0 ||
	       strncasecmp(prefix, "<title", 6) == 0 ||
	       strncasecmp(prefix, "<html", 5) == 0 ||
	       strncmp(prefix, "<!--", 4) == 0 ||
	       strncasecmp(prefix, "<h1", 3) == 0)
	{
	  type = NSHTMLTextDocumentType;
	}
    }
  if (type == nil)
    {
      type = NSPlainTextDocumentType;
    }

  converter = converter_class(type, NO);
  if (converter != Nil)
    {
      NSAttributedString *new;

      new = [converter
              parseData: data
              options: options
              documentAttributes: dict
              error: error
              class: [self class]];
      // We do not return self but the newly created object
      RELEASE(self);
      return RETAIN(new); 
    }
  else if ([type isEqualToString: NSPlainTextDocumentType]
           || [type isEqualToString: @"public.plain-text"]
           || [type isEqualToString: @"text"])
    {
      // FIXME: Should we have a proper converter for this type?
      NSStringEncoding encoding = [[options objectForKey: @"CharacterEncoding"] 
				      intValue];
      NSDictionary *defaultAttrs = [options objectForKey: @"DefaultAttributes"];
      NSString *str;

      if (encoding == GSUndefinedEncoding)
	{
	  encoding = NSUTF8StringEncoding;

	  if ([data length] >= 2)
	    {	
	      static const unichar byteOrderMark = 0xFEFF;
	      static const unichar byteOrderMarkSwapped = 0xFFFE;
	      const unichar firstChar = ((const unichar *)[data bytes])[0];
	      if (firstChar == byteOrderMark
		  || firstChar == byteOrderMarkSwapped)
		{
		  encoding = NSUnicodeStringEncoding;
		}
	    }
	}

      if (dict != NULL)
	{
	  *dict = [NSDictionary dictionaryWithObjectsAndKeys:
				  NSPlainTextDocumentType, NSDocumentTypeDocumentAttribute,
			 [NSNumber numberWithUnsignedInteger: encoding], NSCharacterEncodingDocumentAttribute,
				nil];
	}

      str = [[NSString alloc] initWithData: data 
                              encoding: encoding];
      self = [self initWithString: str
                   attributes: defaultAttrs];
      RELEASE(str);
      return self;
    }

  if (error)
    *error = create_error(0, NSLocalizedString(@"Could not load data.", 
					       @"Error description"));
  RELEASE(self);
  return nil;
}

- (id) initWithPath: (NSString *)path
 documentAttributes: (NSDictionary **)dict
{
  BOOL isDir = NO;

  if (path == nil)
    {
      RELEASE (self);
      return nil;
    }

  if ([[NSFileManager defaultManager]
          fileExistsAtPath: path isDirectory: &isDir] && isDir)
    {
      // FIXME: This expects the file to be RTFD
      NSFileWrapper *fw;

      fw = [[NSFileWrapper alloc] initWithPath: path];
      AUTORELEASE (fw);
  
      return [self initWithRTFDFileWrapper: fw documentAttributes: dict];
    }
  else
   {
     return [self initWithURL: [NSURL fileURLWithPath: path]
		  documentAttributes: dict];
   }
}

- (id) initWithURL: (NSURL *)url 
documentAttributes: (NSDictionary **)dict
{
  NSURL *baseURL = [url baseURL];
  NSDictionary *options = nil;

  if (baseURL != nil)
    {
      [NSDictionary dictionaryWithObject: baseURL
				  forKey: NSBaseURLDocumentOption];
    }

  return [self initWithURL: url
               options: options
               documentAttributes: dict
               error: NULL];
}

- (id) initWithURL: (NSURL *)url
           options: (NSDictionary *)options
documentAttributes: (NSDictionary **)dict
             error: (NSError **)error
{
  NSURL *baseURL;
  NSData *data = [url resourceDataUsingCache: YES];

  if (data == nil)
    {
      if (error)
	*error = create_error(0, NSLocalizedString(@"Could not load data from URL.", 
						   @"Error description"));
      RELEASE(self);
      return nil;
    }

  // Pass on baseURL
  baseURL = [url baseURL];
  if (baseURL != nil)
    {
      if (options == nil)
	options = [NSDictionary dictionaryWithObject: baseURL
					      forKey: NSBaseURLDocumentOption];
      else if ([options objectForKey: NSBaseURLDocumentOption] == nil)
	{
	  options = AUTORELEASE([options mutableCopy]);
	  [(NSMutableDictionary*)options setObject: baseURL
					    forKey: NSBaseURLDocumentOption];
	}
    }

  return [self initWithData: data
               options: options
               documentAttributes: dict
               error: error];
}

- (NSData *) RTFFromRange: (NSRange)range
       documentAttributes: (NSDictionary *)dict
{
  if (dict == nil)
    {
      dict = [NSDictionary dictionaryWithObject: NSRTFTextDocumentType
                                 forKey: NSDocumentTypeDocumentOption];
    }
  else if ([dict objectForKey: NSDocumentTypeDocumentOption] == nil)
    {
      dict = AUTORELEASE([dict mutableCopy]);
      [(NSMutableDictionary*)dict setObject: NSRTFTextDocumentType 
                             forKey: NSDocumentTypeDocumentOption];
    }

  return [self dataFromRange: range
               documentAttributes: dict
               error: NULL];
}

- (NSData *) RTFDFromRange: (NSRange)range
	documentAttributes: (NSDictionary *)dict
{
  if (dict == nil)
    {
      dict = [NSDictionary dictionaryWithObject: NSRTFDTextDocumentType
                                 forKey: NSDocumentTypeDocumentOption];
    }
  else if ([dict objectForKey: NSDocumentTypeDocumentOption] == nil)
    {
      dict = AUTORELEASE([dict mutableCopy]);
      [(NSMutableDictionary*)dict setObject: NSRTFDTextDocumentType 
                             forKey: NSDocumentTypeDocumentOption];
    }

  return [self dataFromRange: range
               documentAttributes: dict
               error: NULL];
}

- (NSFileWrapper *) RTFDFileWrapperFromRange: (NSRange)range
			  documentAttributes: (NSDictionary *)dict
{
  return AUTORELEASE([[NSFileWrapper alloc]
                       initWithSerializedRepresentation: 
                         [self RTFDFromRange: range
                               documentAttributes: dict]]);
}

- (NSData *) docFormatFromRange: (NSRange)range
             documentAttributes: (NSDictionary *)dict
{
  if (dict == nil)
    {
      dict = [NSDictionary dictionaryWithObject: NSDocFormatTextDocumentType
                                 forKey: NSDocumentTypeDocumentOption];
    }
  else if ([dict objectForKey: NSDocumentTypeDocumentOption] == nil)
    {
      dict = AUTORELEASE([dict mutableCopy]);
      [(NSMutableDictionary*)dict setObject: NSDocFormatTextDocumentType 
                             forKey: NSDocumentTypeDocumentOption];
    }

  return [self dataFromRange: range
               documentAttributes: dict
               error: NULL];
}

- (NSData *) dataFromRange: (NSRange)range
        documentAttributes: (NSDictionary *)dict
                     error: (NSError **)error
{
  NSString *type = [dict objectForKey: NSDocumentTypeDocumentOption];
  Class converter;

  if (type == nil)
    {
      if (error)
	*error = create_error(0, NSLocalizedString(@"No type specified for data.",
						   @"Error description"));
      return nil;
    }

  converter = converter_class(type, YES);
  if (converter != Nil)
    {
      return [converter
               produceDataFrom: 
                 [self attributedSubstringFromRange: range]
               documentAttributes: dict
               error: error];
    }
  else if ([type isEqualToString: NSPlainTextDocumentType]
           || [type isEqualToString: @"public.plain-text"]
           || [type isEqualToString: @"text"])
    {
      NSStringEncoding encoding = [[dict objectForKey: @"CharacterEncoding"] 
                                      intValue];
      
      if (!encoding)
        encoding = [NSString defaultCStringEncoding];
      return [[self string] dataUsingEncoding: encoding];
    }

  if (error)
    *error = create_error(0, NSLocalizedString(@"Could not create data for type.",
					       @"Error description"));
  return nil;
}

- (NSFileWrapper *) fileWrapperFromRange: (NSRange)range
                      documentAttributes: (NSDictionary *)dict
                                   error: (NSError **)error
{
  NSFileWrapper *wrapper;
  NSData *data;

  data = [self dataFromRange: range
	  documentAttributes: dict
		       error: error];
  if (data != nil)
    {
      // FIXME: This wont work for directory bundles.
      wrapper = [[NSFileWrapper alloc] initRegularFileWithContents: data];
      return AUTORELEASE(wrapper);
    }

  if (error)
    *error = create_error(0, NSLocalizedString(@"Could not create data for type.",
					       @"Error description"));

  return nil;
}

- (NSInteger) itemNumberInTextList: (NSTextList *)list
                           atIndex: (NSUInteger)location
{
  NSParagraphStyle *style = [self attribute: NSParagraphStyleAttributeName
                                  atIndex: location
                                  effectiveRange: NULL];
  if (style != nil)
    {
      NSArray *textLists = [style textLists];

      if (textLists != nil)
        {
          return [textLists indexOfObject: list];
        }
    }

  return NSNotFound;
}

- (NSRange) rangeOfTextBlock: (NSTextBlock *)block
                     atIndex: (NSUInteger)location
{
  NSRange effRange;
  NSParagraphStyle *style = [self attribute: NSParagraphStyleAttributeName
                                  atIndex: location
                                  effectiveRange: &effRange];
  if (style != nil)
    {
      NSArray *textBlocks = [style textBlocks];

      if ((textBlocks != nil) && [textBlocks containsObject: block])
        {
          NSRange newEffRange;
          NSUInteger len = [self length];

          while ((effRange.location > 0) && style && textBlocks)
            {
              style = [self attribute: NSParagraphStyleAttributeName
                            atIndex: effRange.location - 1
                            effectiveRange: &newEffRange];
              if (style != nil)
                {
                  textBlocks = [style textBlocks];
                  
                  if ((textBlocks != nil) && [textBlocks containsObject: block])
                    {
                      effRange.location = newEffRange.location;
                      effRange.length += newEffRange.length;
                    }
                }
            }

          while (NSMaxRange(effRange) < len && style && textBlocks) 
            {
              style = [self attribute: NSParagraphStyleAttributeName
                            atIndex: NSMaxRange(effRange)
                            effectiveRange: &newEffRange];
              if (style != nil)
                {
                  textBlocks = [style textBlocks];
                  
                  if ((textBlocks != nil) && [textBlocks containsObject: block])
                    {
                      effRange.length += newEffRange.length;
                    }
                }
            }

          return effRange;
        }
    }

  return NSMakeRange(NSNotFound, 0);
}

- (NSRange) rangeOfTextList: (NSTextList *)list
                    atIndex: (NSUInteger)location
{
  NSRange effRange;
  NSParagraphStyle *style = [self attribute: NSParagraphStyleAttributeName
                                  atIndex: location
                                  effectiveRange: &effRange];
  if (style != nil)
    {
      NSArray *textLists = [style textLists];

      if ((textLists != nil) && [textLists containsObject: list])
        {
          NSRange newEffRange;
          NSUInteger len = [self length];

          while ((effRange.location > 0) && style && textLists)
            {
              style = [self attribute: NSParagraphStyleAttributeName
                            atIndex: effRange.location - 1
                            effectiveRange: &newEffRange];
              if (style != nil)
                {
                  textLists = [style textLists];
                  
                  if ((textLists != nil) && [textLists containsObject: list])
                    {
                      effRange.location = newEffRange.location;
                      effRange.length += newEffRange.length;
                    }
                }
            }

          while (NSMaxRange(effRange) < len && style && textLists) 
            {
              style = [self attribute: NSParagraphStyleAttributeName
                            atIndex: NSMaxRange(effRange)
                            effectiveRange: &newEffRange];
              if (style != nil)
                {
                  textLists = [style textLists];
                  
                  if ((textLists != nil) && [textLists containsObject: list])
                    {
                      effRange.length += newEffRange.length;
                    }
                }
            }

          return effRange;
        }
    }

  return NSMakeRange(NSNotFound, 0);
}

static inline
BOOL containsTable(NSArray *textBlocks, NSTextTable *table)
{
  NSEnumerator *benum = [textBlocks objectEnumerator];
  NSTextTableBlock *block;

  while ((block = [benum nextObject]))
    {
      if ([table isEqual: [block table]])
	{
	  return YES;
	}
    }
  return NO;
}

- (NSRange) rangeOfTextTable: (NSTextTable *)table
                     atIndex: (NSUInteger)location
{
  NSRange effRange;
  NSParagraphStyle *style = [self attribute: NSParagraphStyleAttributeName
                                  atIndex: location
                                  effectiveRange: &effRange];
  if (style != nil)
    {
      NSArray *textBlocks = [style textBlocks];

      if ((textBlocks != nil) && containsTable(textBlocks, table))
        {
	  NSRange newEffRange;
	  NSUInteger len = [self length];
	  
	  while ((effRange.location > 0) && style && textBlocks)
	    {
	      style = [self attribute: NSParagraphStyleAttributeName
			      atIndex: effRange.location - 1
		       effectiveRange: &newEffRange];
	      if (style != nil)
		{
		  textBlocks = [style textBlocks];
		  
		  if ((textBlocks != nil) && containsTable(textBlocks, table))
                    {
                      effRange.location = newEffRange.location;
                      effRange.length += newEffRange.length;
                    }
                }
            }

          while (NSMaxRange(effRange) < len && style && textBlocks) 
            {
              style = [self attribute: NSParagraphStyleAttributeName
                            atIndex: NSMaxRange(effRange)
                            effectiveRange: &newEffRange];
              if (style != nil)
                {
                  textBlocks = [style textBlocks];
                  
                  if ((textBlocks != nil) && containsTable(textBlocks, table))
                    {
                      effRange.length += newEffRange.length;
                    }
                }
            }

          return effRange;
        }
    }

  return NSMakeRange(NSNotFound, 0);
}

@end

@implementation NSMutableAttributedString (AppKit)
- (void) superscriptRange: (NSRange)range
{
  id value;
  int sValue;
  NSRange effRange;
  
  if (NSMaxRange (range) > [self length])
    {
      [NSException raise: NSRangeException
		   format: @"RangeError in method -superscriptRange: "];
    }
  
  // We take the value from the first character and use it for the whole range
  value = [self attribute: NSSuperscriptAttributeName
		  atIndex: range.location
	   effectiveRange: &effRange];

  if (value != nil)
    {
      sValue = [value intValue] + 1;
    }
  else
    {
      sValue = 1;
    }
  

  [self addAttribute: NSSuperscriptAttributeName
	value: [NSNumber numberWithInt: sValue]
	range: range];
}

- (void) subscriptRange: (NSRange)range
{
  id value;
  int sValue;
  NSRange effRange;

  if (NSMaxRange (range) > [self length])
    {
      [NSException raise: NSRangeException
		  format: @"RangeError in method -subscriptRange: "];
    }

  // We take the value form the first character and use it for the whole range
  value = [self attribute: NSSuperscriptAttributeName
		atIndex: range.location
		effectiveRange: &effRange];

  if (value != nil)
    {
      sValue = [value intValue] - 1;
    }
  else
    {
      sValue = -1;
    }

  [self addAttribute: NSSuperscriptAttributeName
	value: [NSNumber numberWithInt: sValue]
	range: range];
}

- (void) unscriptRange: (NSRange)range
{
  if (NSMaxRange (range) > [self length])
    {
      [NSException raise: NSRangeException
		  format: @"RangeError in method -unscriptRange: "];
    }

  [self removeAttribute: NSSuperscriptAttributeName
	range: range];
}

- (void) applyFontTraits: (NSFontTraitMask)traitMask
		   range: (NSRange)range
{
  NSFont *font;
  NSUInteger loc = range.location;
  NSRange effRange;
  NSFontManager *fm = [NSFontManager sharedFontManager];

  if (NSMaxRange (range) > [self length])
    {
      [NSException raise: NSRangeException
		   format: @"RangeError in method -applyFontTraits: range: "];
    }

  while (loc < NSMaxRange (range))
    {
      font = [self attribute: NSFontAttributeName
		   atIndex: loc
		   effectiveRange: &effRange];

      if (font != nil)
	{
	  font = [fm convertFont: font
		     toHaveTrait: traitMask];

	  if (font != nil)
	    {
	      [self addAttribute: NSFontAttributeName
		    value: font
		    range: NSIntersectionRange (effRange, range)];
	    }
	}
      loc = NSMaxRange(effRange);
    }
}

- (void) setAlignment: (NSTextAlignment)alignment
		range: (NSRange)range
{
  id		value;
  NSUInteger	loc = range.location;
  
  if (NSMaxRange(range) > [self length])
    {
      [NSException raise: NSRangeException
		  format: @"RangeError in method -setAlignment: range: "];
    }

  while (loc < NSMaxRange(range))
    {
      BOOL	copiedStyle = NO;
      NSRange	effRange;
      NSRange	newRange;

      value = [self attribute: NSParagraphStyleAttributeName
		      atIndex: loc
	       effectiveRange: &effRange];
      newRange = NSIntersectionRange (effRange, range);

      if (value == nil)
	{
	  value = [NSMutableParagraphStyle defaultParagraphStyle];
	}
      else
	{
	  value = [value mutableCopy];
	  copiedStyle = YES;
	}

      [value setAlignment: alignment];

      [self addAttribute: NSParagraphStyleAttributeName
		   value: value
		   range: newRange];
      if (copiedStyle == YES)
	{
	  RELEASE(value);
	}
      loc = NSMaxRange (effRange);
    }
}

- (void) fixAttributesInRange: (NSRange)range
{
  [self fixFontAttributeInRange: range];
  [self fixParagraphStyleAttributeInRange: range];
  [self fixAttachmentAttributeInRange: range];
}

static NSString *lastFont = nil;
static NSCharacterSet *lastSet = nil;
static NSMutableDictionary *cachedCSets = nil;

- (NSFont*)_substituteFontWithName: (NSString*)fontName 
                              font: (NSFont*)baseFont
{
  return [[NSFontManager sharedFontManager] convertFont: baseFont 
                                            toFace: fontName];
}

- (NSFont*)_substituteFontFor: (unichar)uchar 
                         font: (NSFont*)baseFont 
                     fromList: (NSArray *)fonts
{
  NSUInteger count;
  NSUInteger i;
      
  if (cachedCSets == nil)
    {
      cachedCSets = [NSMutableDictionary new];
    }

  count = [fonts count];
  for (i = 0; i < count; i++)
    {
      NSFont *newFont;
      NSString *fName;
      NSCharacterSet *newSet;

      fName = [fonts objectAtIndex: i];
      newSet = [cachedCSets objectForKey: fName];
      if (newSet == nil)
        { 
          newFont = [self _substituteFontWithName: fName font: baseFont];
          newSet = [newFont coveredCharacterSet];
          if ((newSet != nil) && ([cachedCSets count] < 10))
            {
              [cachedCSets setObject: newSet forKey: fName];
            }
        } 
      else
        {
          newFont = nil;
        }
      
      if ([newSet characterIsMember: uchar])
        {
          ASSIGN(lastFont, fName);
          ASSIGN(lastSet, newSet);
          if (newFont != nil)
            {
              return newFont;
            }
          else
            {
              return [self _substituteFontWithName: fName font: baseFont];      
            }
        }
    }

  return nil;
}

- (NSFontDescriptor*)_substituteFontDescriptorFor: (unichar)uchar
{
  NSString *chars = [NSString stringWithCharacters: &uchar length: 1];

  // If we cannot get a string from a single unichar, it most likely is part of a surrogate pair
  if (nil != chars)
    {
      NSCharacterSet *requiredCharacterSet = [NSCharacterSet characterSetWithCharactersInString: chars];
      NSDictionary *fontAttributes = [NSDictionary dictionaryWithObjectsAndKeys: requiredCharacterSet, NSFontCharacterSetAttribute, nil];
      NSSet *mandatoryKeys = [NSSet setWithObjects: NSFontCharacterSetAttribute, nil];
      NSFontDescriptor *fd = [NSFontDescriptor fontDescriptorWithFontAttributes: fontAttributes];
      return [fd matchingFontDescriptorWithMandatoryKeys: mandatoryKeys];
    }
  else
    {
      return nil;
    }
}

- (NSFont*)_substituteFontFor: (unichar)uchar font: (NSFont*)baseFont
{
  NSFont *subFont;
  NSFontDescriptor *descriptor;

  // Caching one font may lead to the selected substitution font not being
  // from the prefered list, although there is one there with this character.
  if (lastSet && [lastSet characterIsMember: uchar])
    {
      return [self _substituteFontWithName: lastFont font: baseFont];
    }

  subFont = [self _substituteFontFor: uchar 
                  font: baseFont 
                  fromList: [NSFont preferredFontNames]];
  if (subFont != nil)
    {
      return subFont;
    }

  // Fast way with font descriptors
  descriptor = [self _substituteFontDescriptorFor: uchar];
  if (descriptor != nil)
    {
      NSCharacterSet *newSet = [descriptor objectForKey: NSFontCharacterSetAttribute];
      if ([newSet characterIsMember: uchar])
        {
          NSString *fName = [descriptor objectForKey: NSFontFamilyAttribute];
 
          ASSIGN(lastFont, fName);
          ASSIGN(lastSet, newSet);
          return [self _substituteFontWithName: fName font: baseFont];
        }
    }

  
  subFont = [self _substituteFontFor: uchar font: baseFont fromList: 
                      [[NSFontManager sharedFontManager] availableFonts]];
  if (subFont != nil)
    {
      return subFont;
    }

  return nil;
}

- (void) fixFontAttributeInRange: (NSRange)range
{
  NSString *string;
  NSFont *font = nil;
  NSCharacterSet *charset = nil;
  NSRange fontRange = NSMakeRange(NSNotFound, 0);
  NSUInteger i;
  NSUInteger lastMax;
  NSUInteger start;
  unichar chars[64];
  CREATE_AUTORELEASE_POOL(pool);
  NSCharacterSet *controlset = [NSCharacterSet controlCharacterSet];
  
  if (NSMaxRange (range) > [self length])
    {
      [NSException raise: NSRangeException
		  format: @"RangeError in method -fixFontAttributeInRange: "];
    }
  // Check for each character if it is supported by the 
  // assigned font
  
  /*
  Note that this needs to be done on a script basis. Per-character checks
  are difficult to do at all, don't give reasonable results, and would have
  really poor performance.
  */
  string = [self string];
  lastMax = range.location;
  start = lastMax;
  for (i = range.location; i < NSMaxRange(range); i++)
    {
      unichar uchar;
  
      if (i >= lastMax)
        {
          NSUInteger dist;
          
          start = lastMax;
          dist = MIN(64, NSMaxRange(range) - start);
          lastMax = start + dist;
          [string getCharacters: chars range: NSMakeRange(start, dist)];
        }
      uchar = chars[i - start];
      if (uchar >= 0xd800 && uchar <= 0xdfff)
        {
          // Currently we don't handle surrogate pairs
          continue;
        }
      
      if (!NSLocationInRange(i, fontRange))
        {
          font = [self attribute: NSFontAttributeName
                       atIndex: i
                       effectiveRange: &fontRange];

          /* If we don't have an attribute for NSFontAttributeName,
          ** we take a default font so we can carry on with the
          ** substitution.
          */
          if (nil == font)
            {
              font = [NSFont userFontOfSize: 0.0];
            }

          charset = [font coveredCharacterSet];
        }
      
      if (charset != nil && ![charset characterIsMember: uchar]
          && (uchar != NSAttachmentCharacter)
          && ![controlset characterIsMember: uchar])
        {
          // Find a replacement font
          NSFont *subFont;
          
          subFont = [self _substituteFontFor: uchar font: font];
          if (subFont != nil)
            {
              // Set substitution font permanently
              [self addAttribute: NSFontAttributeName
                    value: subFont
                    range: NSMakeRange(i, 1)];
            }
        }
    }
  
  [pool drain];
}

- (void) fixParagraphStyleAttributeInRange: (NSRange)range
{
  NSString *str = [self string];
  NSUInteger loc = range.location;
  NSRange r;

  if (NSMaxRange (range) > [self length])
    {
      [NSException raise: NSRangeException
	format: @"RangeError in method -fixParagraphStyleAttributeInRange: "];
    }

  while (loc < NSMaxRange (range))
    {
      NSParagraphStyle	*style;
      NSRange		found;
      NSUInteger	end;

      /* Extend loc to take in entire paragraph if necessary.  */
      r = [str lineRangeForRange: NSMakeRange (loc, 1)];
      end = NSMaxRange (r);

      /* Get the style in effect at the paragraph start.  */
      style = [self attribute: NSParagraphStyleAttributeName
		    atIndex: r.location
		    longestEffectiveRange: &found
		    inRange: r];
      if (style == nil)
	{
	  /* No style found at the beginning of paragraph.  found is
             the range without the style set.  */
	  if ((NSMaxRange (found) + 1) < end)
	    {
	      /* There is a paragraph style for part of the paragraph. Set
	      this style for the entire paragraph.

	      Since NSMaxRange(found) + 1 is outside the longest effective
	      range for the nil style, it must be non-nil.
	      */
	      style = [self attribute: NSParagraphStyleAttributeName
			      atIndex: NSMaxRange(found) + 1
			      effectiveRange: NULL];
	      [self addAttribute: NSParagraphStyleAttributeName
		    value: style
		    range: r];
	    }
	  else
	    {
	      /* All the paragraph without a style ... too bad, fixup
		 the whole paragraph using the default paragraph style.  */
	      [self addAttribute: NSParagraphStyleAttributeName
		    value: [NSParagraphStyle defaultParagraphStyle]
		    range: r];
	    }
	}
      else
	{
	  if (NSMaxRange (found) < end)
	    {
	      /* Not the whole paragraph has the same style ... add
		 the style found at the beginning to the remainder of
		 the paragraph.  */
	      found.location = NSMaxRange (found);
	      found.length = end - found.location;
	      [self addAttribute: NSParagraphStyleAttributeName
		    value: style
		    range: found];
	    }
	}
      
      /* Move on to the next paragraph.  */
      loc = end;
    }
}

- (void) fixAttachmentAttributeInRange: (NSRange)range
{
  NSString *string = [self string];
  NSUInteger location = range.location;
  NSUInteger end = NSMaxRange (range);

  cache_init ();

  if (end > [self length])
    {
      [NSException raise: NSRangeException
	format: @"RangeError in method -fixAttachmentAttributeInRange: "];
    }

  // Check for attachments with the wrong character
  while (location < end)
    {
      NSDictionary	*attr;
      NSRange		eRange;

      attr = [self attributesAtIndex: location  effectiveRange: &eRange];
      if ([attr objectForKey: NSAttachmentAttributeName] != nil)
	{
	  unichar	buf[eRange.length];
	  NSUInteger	pos = 0;
	  NSUInteger	start = eRange.location;

	  // Leave only one character with the attachment
	  [string getCharacters: buf  range: eRange];
	  while (pos < eRange.length && buf[pos] != NSAttachmentCharacter)
	    pos++;
	  if (pos)
	    [self removeAttribute: NSAttachmentAttributeName
		  range: NSMakeRange (start, pos)];
	  pos++;
	  if (pos < eRange.length)
	    [self removeAttribute: NSAttachmentAttributeName
		  range: NSMakeRange (start + pos, eRange.length - pos)];
	}
      location = NSMaxRange (eRange);
    }

  // Check for attachment characters without attachments
  location = range.location;
  string = [self string];
  while (location < end)
    {
      NSRange eRange = [string rangeOfString: attachmentString
			      options: NSLiteralSearch 
			      range: NSMakeRange (location, end - location)];
      NSTextAttachment *attachment;

      if (!eRange.length)
        break;

      attachment = [self attribute: NSAttachmentAttributeName
			 atIndex: eRange.location
			 effectiveRange: NULL];

      if (attachment == nil)
        {
          [self deleteCharactersInRange: NSMakeRange (eRange.location, 1)];
          eRange.length--;
          end--;
          // Need to reset this after every character change
          string = [self string];
        }

      location = NSMaxRange (eRange);
    }
}

- (void) updateAttachmentsFromPath: (NSString *)path
{
  NSString *string = [self string];
  NSUInteger location = 0;
  NSUInteger end = [string length];

  cache_init ();

  while (location < end)
    {
      NSRange range = [string rangeOfString: attachmentString
			      options: NSLiteralSearch 
			      range: NSMakeRange (location, end - location)];
      NSTextAttachment *attachment;
      NSFileWrapper *fileWrapper;

      if (!range.length)
	break;

      attachment = [self attribute: NSAttachmentAttributeName
			 atIndex: range.location
			 effectiveRange: NULL];
      fileWrapper = [attachment fileWrapper];

      // FIXME: Is this the correct thing to do?
      [fileWrapper updateFromPath: [path stringByAppendingPathComponent: 
					     [fileWrapper filename]]];
      location = NSMaxRange (range);
    }
}

- (BOOL) readFromURL: (NSURL *)url
             options: (NSDictionary *)options
  documentAttributes: (NSDictionary**)documentAttributes
{
  return [self readFromURL: url
               options: options
               documentAttributes: documentAttributes
               error: NULL];
}

- (BOOL) readFromURL: (NSURL *)url
             options: (NSDictionary *)options
  documentAttributes: (NSDictionary **)documentAttributes
               error: (NSError **)error
{
  NSAttributedString *attr;

  attr = [[NSAttributedString alloc] 
                 initWithURL: url
		     options: options
	  documentAttributes: documentAttributes
		       error: error];
  if (attr != nil)
    {
      [self setAttributedString: attr];
      RELEASE(attr);
      return YES; 
    }

  return NO;
}

- (BOOL) readFromData: (NSData *)data
              options: (NSDictionary *)options
   documentAttributes: (NSDictionary **)documentAttributes
{
  return [self readFromData:  data
               options: options
               documentAttributes: documentAttributes
               error: NULL];
}

- (BOOL) readFromData: (NSData *)data
              options: (NSDictionary *)options
   documentAttributes: (NSDictionary **)documentAttributes
                error: (NSError **)error
{
  NSAttributedString *attr;

  attr = [[NSAttributedString alloc] 
             initWithData: data
             options: options
             documentAttributes: documentAttributes
             error: error];
  if (attr)
    {
      [self setAttributedString: attr];
      RELEASE(attr);
      return YES;
    }

  return NO;
}

- (void) setBaseWritingDirection: (NSWritingDirection)writingDirection
                           range: (NSRange)range
{
  id value;
  NSUInteger loc = range.location;
  
  if (NSMaxRange(range) > [self length])
    {
      [NSException raise: NSRangeException
		  format: @"RangeError in method -setBaseWritingDirection: range: "];
    }

  while (loc < NSMaxRange(range))
    {
      BOOL copiedStyle = NO;
      NSRange effRange;
      NSRange newRange;

      value = [self attribute: NSParagraphStyleAttributeName
		      atIndex: loc
	       effectiveRange: &effRange];
      newRange = NSIntersectionRange(effRange, range);

      if (value == nil)
	{
	  value = [NSMutableParagraphStyle defaultParagraphStyle];
	}
      else
	{
	  value = [value mutableCopy];
	  copiedStyle = YES;
	}

      [value setBaseWritingDirection: writingDirection];

      [self addAttribute: NSParagraphStyleAttributeName
		   value: value
		   range: newRange];
      if (copiedStyle == YES)
	{
	  RELEASE(value);
	}
      loc = NSMaxRange(effRange);
    }
}

@end
