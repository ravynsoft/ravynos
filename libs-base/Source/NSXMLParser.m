/** Implementation for NSXMLParser for GNUStep
   Copyright (C) 2004 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date: May 2004

   SloppyParser additions based on code by Nikolaus Schaller

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
#define	EXPOSE_NSXMLParser_IVARS	1
#import "Foundation/NSArray.h"
#import "Foundation/NSError.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSException.h"
#import "Foundation/NSXMLParser.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSNull.h"
#import "GNUstepBase/GSMime.h"

@interface GSMimeDocument (internal)
+ (NSString*) charsetForXml: (NSData*)xml;
@end

NSString* const NSXMLParserErrorDomain = @"NSXMLParserErrorDomain";

static  NSNull  *null = nil;

/* We always have the native (sloppy) parser, and can get that behavior
 * by using the GSSloppyXMLParser class.
 * With libxml2 we have a stricter parser (which will break on some OSX
 * property lists) available using GSStrictXMLParser.
 */
@interface      GSSloppyXMLParser : NSXMLParser
@end
@implementation GSSloppyXMLParser
@end
@interface      GSStrictXMLParser : NSXMLParser
@end


@implementation NSString (NSXMLParser)

- (NSString *) _stringByExpandingXMLEntities
{
  NSMutableString       *t = [NSMutableString stringWithString: self];

  [t replaceOccurrencesOfString: @"&"
                     withString: @"&amp;"
                        options: 0
                          range: NSMakeRange(0, [t length])];  // must be first!
  [t replaceOccurrencesOfString: @"<"
                     withString: @"&lt;"
                        options: 0
                          range: NSMakeRange(0, [t length])];
  [t replaceOccurrencesOfString: @">"
                     withString: @"&gt;"
                        options: 0
                          range: NSMakeRange(0, [t length])];
  [t replaceOccurrencesOfString: @"\""
                     withString: @"&quot;"
                        options: 0
                          range: NSMakeRange(0, [t length])];
  [t replaceOccurrencesOfString: @"'"
                     withString: @"&apos;"
                        options: 0
                          range: NSMakeRange(0, [t length])];
  return t;
}

@end

static inline NSString *
NewUTF8STR(const void *ptr, int len)
{
  NSString	*s;

  s = [[NSString alloc] initWithBytes: ptr
			       length: len
			     encoding: NSUTF8StringEncoding];
  if (s == nil)
    NSLog(@"could not convert to UTF8 string! bytes=%p len=%d", ptr, len);
  return s;
}

@interface GSXMLParserIvars : NSObject
{
@public
  NSMutableArray        *tagPath;	// hierarchy of tags
  NSMutableArray        *namespaces;
  NSMutableDictionary	*defaults;
  NSData                *data;
  NSError               *error;
  const unsigned char	*bytes;
  NSUInteger		cp;		// character position
  NSUInteger		cend;		// end of data
  int line;				// current line (counts from 0)
  int column;				// current column (counts from 0)
  BOOL abort;				// abort parse loop
  BOOL ignorable;			// whitespace is ignorable
  BOOL whitespace;			// had only whitespace in current data
  BOOL shouldProcessNamespaces;
  BOOL shouldReportNamespacePrefixes;
  BOOL shouldResolveExternalEntities;
  BOOL acceptHTML;			// be lazy with bad tag nesting
  BOOL hasStarted;
  BOOL hasElement;
  IMP	didEndElement;
  IMP	didEndMappingPrefix;
  IMP	didStartElement;
  IMP	didStartMappingPrefix;
  IMP	foundCDATA;
  IMP	foundCharacters;
  IMP	foundComment;
  IMP	foundIgnorable;
} 
@end
@implementation	GSXMLParserIvars
- (NSString*) description
{
  return [[super description] stringByAppendingFormat:
    @" shouldProcessNamespaces: %d"
    @" shouldReportNamespacePrefixes: %d"
    @" shouldResolveExternalEntities: %d"
    @" acceptHTML: %d"
    @" hasStarted: %d"
    @" hasElement: %d",
    shouldProcessNamespaces,
    shouldReportNamespacePrefixes,
    shouldResolveExternalEntities,
    acceptHTML,
    hasStarted,
    hasElement];
}
@end

static SEL	didEndElementSel = 0;
static SEL	didEndMappingPrefixSel;
static SEL	didStartElementSel;
static SEL	didStartMappingPrefixSel;
static SEL	foundCDATASel;
static SEL	foundCharactersSel;
static SEL	foundCommentSel;
static SEL	foundIgnorableSel;

@interface	NSXMLParser (Private)
- (NSString *) _newQarg;
@end

@implementation NSXMLParser

#define EXTRA_DEBUG     0

#define _parser (self->_parser)
#define _handler (self->_handler)
#define	this	((GSXMLParserIvars*)_parser)
#define	_del	((id)_handler)

static	Class	sloppy = Nil;
static	Class	strict = Nil;

+ (id) allocWithZone: (NSZone*)z
{
  if (self == [NSXMLParser class])
    {
#if	 defined(HAVE_LIBXML)
      return NSAllocateObject(strict, 0, z);
#else
      return NSAllocateObject(sloppy, 0, z);
#endif
    }
  return NSAllocateObject(self, 0, z);
}

+ (void) initialize
{
  sloppy = [GSSloppyXMLParser class];
  strict = [GSStrictXMLParser class];
  if (null == nil)
    {
      null = RETAIN([NSNull null]);
      RELEASE([NSObject leakAt: &null]);
    }
  if (didEndElementSel == 0)
    {
      didEndElementSel
	= @selector(parser:didEndElement:namespaceURI:qualifiedName:);
      didEndMappingPrefixSel
        = @selector(parser:didEndMappingPrefix:);
      didStartElementSel
= @selector(parser:didStartElement:namespaceURI:qualifiedName:attributes:);
      didStartMappingPrefixSel
	= @selector(parser:didStartMappingPrefix:toURI:);
      foundCDATASel
	= @selector(parser:foundCDATA:);
      foundCharactersSel
	= @selector(parser:foundCharacters:);
      foundCommentSel
	= @selector(parser:foundComment:);
      foundIgnorableSel
	= @selector(parser:foundIgnorableWhitespace:);
    }
}

- (void) abortParsing
{
  this->abort = YES;
}

- (NSInteger) columnNumber
{
  return this->column;
}

- (void) dealloc
{
  if (this != 0)
    {
      RELEASE(this->data);
      RELEASE(this->error);
      RELEASE(this->tagPath);
      RELEASE(this->namespaces);
      RELEASE(this->defaults);
      RELEASE(this);
      _parser = 0;
      _handler = 0;
    }
  [super dealloc];
}

- (id) delegate
{
  return _del;
}

- (id) initWithContentsOfURL: (NSURL *)anURL
{
  return [self initWithData: [NSData dataWithContentsOfURL: anURL]];
}

#define	addr(x) (this->bytes + (x))

- (id) initWithData: (NSData *)data
{
  if (data == nil)
    {
      DESTROY(self);
    }
  else
    {
      self = [super init];
      if (self)
	{
	  NSStringEncoding	enc;

	  _parser = [GSXMLParserIvars new];
	  /* Determine character encoding and convert to utf-8 if needed.
	   */
	  enc = [GSMimeDocument encodingFromCharset:
	    [GSMimeDocument charsetForXml: data]];
	  if (enc == NSUTF8StringEncoding
	    || enc == NSASCIIStringEncoding
	    || enc == GSUndefinedEncoding)
	    {
	      this->data = [data copy];
	    }
	  else
	    {
	      NSString	*tmp;

	      tmp = [[NSString alloc] initWithData: data encoding: enc];
	      this->data
		= [[tmp dataUsingEncoding: NSUTF8StringEncoding] retain];
	      RELEASE(tmp);
	    }
	  this->tagPath = [[NSMutableArray alloc] init];
	  this->namespaces = [[NSMutableArray alloc] init];
	  this->bytes = [this->data bytes];
	  this->cp = 0;
	  this->cend = [this->data length];
	  /* If the data contained utf-8 with a BOM, we must skip it.
	   */
	  if ((this->cend - this->cp) > 2 && addr(this->cp)[0] == 0xef
	    && addr(this->cp)[1] == 0xbb && addr(this->cp)[2] == 0xbf)
	    {
	      this->cp += 3;	// Skip BOM
	    }
	}
    }
  return self;
}

- (id) initWithStream: (NSInputStream*)stream
{
  RELEASE(self);	// FIXME
  return nil;
}

- (NSInteger) lineNumber
{
  return this->line;
}

- (void) setDelegate: (id)delegate
{
  if (_handler != delegate)
    {
      _handler = delegate;

      if ([_del respondsToSelector: didEndElementSel])
	{
	  this->didEndElement = [_del methodForSelector: didEndElementSel];
	}
      else
	{
	  this->didEndElement = 0;
	}

      if ([_del respondsToSelector: didEndMappingPrefixSel])
	{
	  this->didEndMappingPrefix
	    = [_del methodForSelector: didEndMappingPrefixSel];
	}
      else
	{
	  this->didEndMappingPrefix = 0;
	}

      if ([_del respondsToSelector: didStartElementSel])
	{
	  this->didStartElement = [_del methodForSelector: didStartElementSel];
	}
      else
	{
	  this->didStartElement = 0;
	}

      if ([_del respondsToSelector: didStartMappingPrefixSel])
	{
	  this->didStartMappingPrefix
	    = [_del methodForSelector: didStartMappingPrefixSel];
	}
      else
	{
	  this->didStartMappingPrefix = 0;
	}

      if ([_del respondsToSelector: foundCDATASel])
	{
	  this->foundCDATA
	    = [_del methodForSelector: foundCDATASel];
	}
      else
	{
	  this->foundCDATA = 0;
	}

      if ([_del respondsToSelector: foundCharactersSel])
	{
	  this->foundCharacters
	    = [_del methodForSelector: foundCharactersSel];
	}
      else
	{
	  this->foundCharacters = 0;
	}

      if ([_del respondsToSelector: foundCommentSel])
	{
	  this->foundComment
	    = [_del methodForSelector: foundCommentSel];
	}
      else
	{
	  this->foundComment = 0;
	}

      if ([_del respondsToSelector: foundIgnorableSel])
	{
/* It seems OX reports ignorable whitespace as characters,
 * so we disable this ... FIXME can this really be right?
 */
#if 0
	  this->foundIgnorable
	    = [_del methodForSelector: foundIgnorableSel];
#else
	  this->foundIgnorable = 0;
#endif
	}
      else
	{
	  this->foundIgnorable = 0;
	}
    }
}

- (NSError *) parserError
{
  return this->error;
}

- (NSArray *) _tagPath
{
  return this->tagPath;
}

#define cget() (\
(this->cp < this->cend) \
  ? (this->column++, *addr(this->cp++)) \
  : -1)

- (BOOL) _parseError: (NSString *)message code: (NSInteger)code
{
  NSDictionary *info = nil;

  message = [NSString stringWithFormat: @"line %d, column %d ... %@",
    this->line, this->column, message];
#if EXTRA_DEBUG
  NSLog(@"XML parseError: %@", message);
#endif

  RELEASE(this->error);
  if (message != nil)
    {
      info = [[NSDictionary alloc] initWithObjectsAndKeys:
	message, NSLocalizedFailureReasonErrorKey, nil];
    }
  this->error = [[NSError alloc] initWithDomain: NSXMLParserErrorDomain
					   code: code
				       userInfo: info];
  RELEASE(info);
  this->abort = YES;
  if ([_del respondsToSelector: @selector(parser:parseErrorOccurred:)])
    [_del parser: self parseErrorOccurred: this->error];
  return NO;
}

/* Go up the namespace stack looking for a mapping from p to
 * a URI.  Return the first URI found (or an empty string if none is found).
 */
- (NSString*) _uriForPrefix: (NSString*)p
{
  unsigned      i = [this->namespaces count];
  NSString      *uri = nil;

  while (uri == nil && i-- > 0)
    {
      id        o = [this->namespaces objectAtIndex: i];

      if (o != (id)null)
        {
          uri = [(NSDictionary*)o objectForKey: p];
        }
    }
  return (nil == uri) ? @"" : uri;
}

- (void) _closeLastTag
{
  NSString      *tag = [this->tagPath lastObject];

  if (this->didEndElement != 0)
    {
      NSString  *qualified = tag;
      NSString  *uri = @"";

      if (this->shouldProcessNamespaces)
        {
          NSRange   r;
          NSString  *p = @"";

          r = [tag rangeOfString: @":" options: NSLiteralSearch];
          if (r.length > 0)
            {
              p = [tag substringToIndex: r.location];
              tag = [tag substringFromIndex: NSMaxRange(r)];
            }
          uri = [self _uriForPrefix: p];
        }
      (*this->didEndElement)(_del,
	didEndElementSel, self, tag, uri, qualified);
    }

  if (this->shouldReportNamespacePrefixes)
    {
      if (this->didEndMappingPrefix != 0)
        {
          id    d = [this->namespaces lastObject];

          if (d != (id)null)
            {
              NSEnumerator  *e = [(NSDictionary*)d keyEnumerator];
              NSString      *k;

              while ((k = [e nextObject]) != nil)
                {
                  (*this->didEndMappingPrefix)(_del,
		    didEndMappingPrefixSel, self, k);
                }
            }
        }
    }

  [this->tagPath removeLastObject];
  [this->namespaces removeLastObject];
}

- (void) _processDeclaration
{
  NSUInteger	tp;
  NSString	*decl;
  NSString	*name;
  int		c;

  if (NO == this->hasStarted)
    {
#if EXTRA_DEBUG
NSLog(@"parserDidStartDocument: ");
#endif
      this->hasStarted = YES;
      if ([_del respondsToSelector: @selector(parserDidStartDocument:)])
	{
	  [_del parserDidStartDocument: self];
	}
    }

  c = cget();
  while (isspace(c))
    {
      c = cget();
    }
  tp = this->cp - 1;
  while (c != EOF && !isspace(c) && c != '>')
    {
      c = cget(); // scan name to delimiting character
    }
  decl = NewUTF8STR(addr(tp), this->cp - tp - 1);
  if (nil == decl)
    {
      [self _parseError: @"invalid character in declaraction"
                   code: NSXMLParserInvalidCharacterError];
      return;
    }
#if EXTRA_DEBUG
  NSLog(@"decl=%@ - %02x %c", decl, c, isprint(c)?c: ' ');
#endif

  while (isspace(c))
    {
      c = cget();
    }
  tp = this->cp - 1;
  while (c != EOF && !isspace(c) && c != '>')
    {
      c = cget(); // scan name to delimiting character
    }
  name = NewUTF8STR(addr(tp), this->cp - tp - 1);
  if (nil == name)
    {
      [self _parseError: @"invalid character in declaraction name"
                   code: NSXMLParserInvalidCharacterError];
      RELEASE(decl);
      return;
    }
#if EXTRA_DEBUG
  NSLog(@"name=%@ - %02x %c", name, c, isprint(c)?c: ' ');
#endif

  if ([decl isEqualToString: @"ATTLIST"])
    {
      NSMutableDictionary	*d;
      NSString			*attr;
      NSString			*type;
      NSString			*def;

#if EXTRA_DEBUG
NSLog(@"_processDeclaration <%@%@ %@>", flag?@"/": @"", decl, name);
#endif

      /* Get the dictionary  of attribute defaults for this element.
       */
      d = [this->defaults objectForKey: name];
      if (nil == d)
	{
	  if (nil == this->defaults)
	    {
	      this->defaults = [NSMutableDictionary new];
	    }
	  d = [NSMutableDictionary new];
	  [this->defaults setObject: d forKey: name];
	  RELEASE(d);
	}

      while (c != EOF && c != '>')
	{
	  while (isspace(c))
	    {
	      c = cget();
	    }
	  tp = this->cp - 1;
	  while (c != EOF && !isspace(c) && c != '>')
	    {
	      c = cget(); // scan name to delimiting character
	    }
	  attr = NewUTF8STR(addr(tp), this->cp - tp - 1);
          if (nil == attr)
            {
              [self _parseError: @"invalid character in declaration attr"
                           code: NSXMLParserInvalidCharacterError];
	      RELEASE(decl);
	      RELEASE(name);
              return;
            }
#if 1 || EXTRA_DEBUG
NSLog(@"attr=%@ - %02x %c", attr, c, isprint(c)?c: ' ');
#endif

	  while (isspace(c))
	    {
	      c = cget();
	    }
	  tp = this->cp - 1;
	  while (c != EOF && !isspace(c) && c != '>')
	    {
	      c = cget(); // scan name to delimiting character
	    }
	  type = NewUTF8STR(addr(tp), this->cp - tp - 1);
          if (nil == type)
            {
              [self _parseError: @"invalid character in declaration type"
                           code: NSXMLParserInvalidCharacterError];
	      RELEASE(name);
	      RELEASE(decl);
	      RELEASE(attr);
              return;
            }
#if 1 || EXTRA_DEBUG
NSLog(@"type=%@ - %02x %c", type, c, isprint(c)?c: ' ');
#endif
	  /* OSX reports a CDATA type as an empty string.
	   */
	  if ([type isEqualToString: @"CDATA"])
	    {
	      RELEASE(type);
	      type = @"";
	    }

	  while (isspace(c))
	    {
	      c = cget();
	    }
	  /* OSX reports a default as nil if it's not a quoted string.
	   */
	  if (c == '#')
	    {
	      while (c != EOF && !isspace(c) && c != '>')
		{
		  c = cget();
		}
	      def = nil;
	    }
	  else
	    {
	      def = [self _newQarg];
	      c = cget();  // get character behind qarg value
	    }
	  while (isspace(c))
	    {
	      c = cget();
	    }

	  /* Record default value (if any) for this attribute.
	   */
	  if (nil != def)
	    {
	      [d setObject: def forKey: attr];
	    }

	  if ([_del respondsToSelector: @selector(parser:foundAttributeDeclarationWithName:forElement:type:defaultValue:)])
	    {
	      [_del parser: self
		foundAttributeDeclarationWithName: attr
		forElement: name
		type: type
		defaultValue: def];
	    }
	  RELEASE(attr);
	  RELEASE(type);
	  RELEASE(def);
	}
    }
  else if ([decl isEqualToString: @"DOCTYPE"])
    {
#if EXTRA_DEBUG
NSLog(@"_processDeclaration <%@%@ %@>", flag?@"/": @"", decl, name);
#endif
      while (isspace(c))
	{
	  c = cget();
	}
      while (c != EOF && c != '[' && c != '>')
	{
	  c = cget();
	}
      if (c == '[')
	{
	  /* Got inline docuent declaration.  Scan to ']'
	   */
	  while (c != EOF && c != ']')
	    {
	      if (c == '<')
		{
	          c = cget();
		  if (c == '!')
		    {
		      [self _processDeclaration];
	              c = cget();
		    }
		}
	      else
		{
	          c = cget();
		}
	    }
	  /* Skip to trailing '>' in DOCTYPE declration.
	   */
	  if (c == ']')
	    {
	      while (c != EOF && c != '>')
		{
		  c = cget();
		}
	    }
	}
    }
  else if ([decl isEqualToString: @"ELEMENT"])
    {
#if EXTRA_DEBUG
NSLog(@"_processDeclaration <%@%@ %@>", flag?@"/": @"", decl, name);
#endif
      while (c != EOF &&  c != '>')
	{
	  c = cget();
	}
      if ([_del respondsToSelector:
	@selector(parser:foundElementDeclarationWithName:model:)])
	{
	  [_del parser: self
	    foundElementDeclarationWithName: name
	    model: @""];
	}
    }
  else if ([decl isEqualToString: @"ENTITY"])
    {
#if EXTRA_DEBUG
NSLog(@"_processDeclaration <%@%@ %@>", flag?@"/": @"", decl, name);
#endif
      while (c != EOF &&  c != '>')
	{
	  c = cget();
	}
    }
  RELEASE(decl);
  RELEASE(name);
}

- (void) _processTag: (NSString *)tag
	       isEnd: (BOOL)flag
      withAttributes: (NSDictionary *)attributes
{
  if (this->acceptHTML)
    {
      tag = [tag lowercaseString];  // not case sensitive
    }
  if (!flag)
    {
      if (NO == this->hasStarted)
	{
#if EXTRA_DEBUG
NSLog(@"parserDidStartDocument: ");
#endif
	  this->hasStarted = YES;
	  if ([_del respondsToSelector: @selector(parserDidStartDocument:)])
            {
              [_del parserDidStartDocument: self];
            }
	}
      if ([tag isEqualToString: @"?xml"])
	{
	  return;
	}
      else if ([tag hasPrefix: @"?"])
	{
#if EXTRA_DEBUG
NSLog(@"_processTag <%@%@ %@>", flag?@"/": @"", tag, attributes);
#endif
	  // parser: foundProcessingInstructionWithTarget: data: 
	  return;
	}
      else
        {
          NSMutableDictionary   *ns = nil;
          NSMutableDictionary   *attr = nil;
          NSEnumerator          *enumerator = [attributes keyEnumerator];
          NSString              *k;
          NSString              *uri;
          NSString              *qualified;

	  this->hasElement = YES;
          while ((k = [enumerator nextObject]) != nil)
            {
              NSString  *prefix = nil;

              if ([k isEqualToString: @"xmlns"] == YES)
                {
                  prefix = @"";
                }
              else if ([k hasPrefix: @"xmlns:"] == YES)
                {
                  prefix = [k substringFromIndex: 6];
                }
              if (prefix != nil)
                {
                  if (nil == ns)
                    {
                      ns = [NSMutableDictionary new];
                      if (this->shouldProcessNamespaces)
                        {
                          attr = [attributes mutableCopy];
                        }
                    }
                  uri = [attributes objectForKey: k];
                  [ns setObject: uri forKey: prefix];
                  if (attr != nil)
                    {
                      [attr removeObjectForKey: k];
                    }
                  if (this->shouldReportNamespacePrefixes)
                    {
                      if (this->didStartMappingPrefix != 0)
                        {
			  (*this->didStartMappingPrefix)(_del,
			    didStartMappingPrefixSel, self, prefix, uri);
                        }
                    }
                }
            }
          if ([attr count] > 0)
            {
              attributes = attr;
            }
          else if (nil == attributes)
            {
              attr = [NSMutableDictionary new];
              attributes = attr;
            }

          [this->tagPath addObject: tag];
          [this->namespaces addObject: ((ns == nil) ? (id)null : (id)ns)];

	  if (this->didStartElement != 0)
	    {
	      qualified = tag;
              if (this->shouldProcessNamespaces)
                {
                  NSRange   r;
                  NSString  *p = @"";

                  r = [tag rangeOfString: @":" options: NSLiteralSearch];
                  if (r.length > 0)
                    {
                      p = [tag substringToIndex: r.location];
                      tag = [tag substringFromIndex: NSMaxRange(r)];
                    }
                  uri = [self _uriForPrefix: p];
                }
              else
                {
                  uri = @"";
                }
	      (*this->didStartElement)(_del,
		didStartElementSel, self, tag, uri, qualified, attributes);
            }
	  TEST_RELEASE(ns);
	  TEST_RELEASE(attr);
        }
    }
  else
    {
      // closing tag
      if (this->acceptHTML)
	{
	  // lazily close any missing tags on stack
	  while ([this->tagPath count] > 0
	    && ![[this->tagPath lastObject] isEqualToString: tag])
	    {
              [self _closeLastTag];
	    }
	  if ([this->tagPath count] == 0)
            {
              return;  // ignore closing tag without matching open...
            }
	}
      else if (![[this->tagPath lastObject] isEqualToString: tag])
	{
	  [self _parseError: [NSString stringWithFormat:
	    @"tag nesting error (</%@> expected, </%@> found)",
	    [this->tagPath lastObject], tag]
	    code: NSXMLParserNotWellBalancedError];
	  return;
	}
      [self _closeLastTag];
    }
}

- (NSString*) _newEntity: (const unsigned char *)ep length: (int)len
{
  NSString      *entity;

  if (*ep == '#')
    {
      if (len < 8)
        {
          uint32_t val;
          char  buf[8];

          memcpy(buf, ep + 1, len - 1);
          buf[len - 1] = '\0';
          // &#ddd; or &#xhh;
          if (sscanf(buf, "x%x;", &val) || sscanf(buf, "%d;", &val))
            {
              // &#xhh; hex value or &ddd; decimal value
              if (val > 0xffff)
                {
                  unichar       buf[2];

                  /* Convert codepoint outside base plane to surrogate pair
                   */
                  val -= 0x010000;
                  buf[0] = (val / 0x400) + 0xd800;
                  buf[1] = (val % 0x400) + 0xdc00;
                  return [[NSString alloc] initWithCharacters: buf length: 2];
                }
              else
                {
                  unichar       c = (unichar)val;

                  return [[NSString alloc] initWithCharacters: &c length: 1];
                }
            }
        }
    }
  else
    {
      // the five predefined entities
      if (len == 3 && strncmp((char *)ep, "amp", len) == 0)
	{
	  return @"&";
	}
      else if (len == 2 && strncmp((char *)ep, "lt", len) == 0)
	{
	  return @"<";
	}
      else if (len == 2 && strncmp((char *)ep, "gt", len) == 0)
	{
	  return @">";
	}
      else if (len == 4 && strncmp((char *)ep, "quot", len) == 0)
	{
	  return @"\"";
	}
      else if (len == 4 && strncmp((char *)ep, "apos", len) == 0)
	{
	  return @"'";
	}
    }
  entity = NewUTF8STR(ep, len);
  if (nil == entity)
    {
      [self _parseError: @"invalid character in entity name"
                   code: NSXMLParserInvalidCharacterError];
    }

  if (this->shouldResolveExternalEntities)
    {
#if 1
      NSLog(@"NSXMLParser: unrecognized entity: &%@;", entity);
#endif
    //  entity=[entitiesTable objectForKey: entity];  // look up string in entity translation table

      if (nil == entity)
        {
          entity = @"&??;";  // unknown entity
        }
    }
  else
    {
      entity = @"";             // not resolved
    }
  return entity;
}

- (BOOL) _parseEntity: (NSString**)result
{
  int 		c;
  NSUInteger	ep = this->cp;  // should be position behind &
  int 		len;
  NSString	*entity;

  if (0 == result) result = &entity;
  do {
    c = cget();
  } while (c != EOF && c != '<' && c != ';');

  if (c != ';')
    {
      // invalid sequence - end of file or missing ; before next tag
      return NO;
    }
  len = this->cp - ep - 1;

  *result = [self _newEntity: addr(ep) length: len];
  if (&entity == result)
    {
      RELEASE(*result); // Won't be used
    }
  return YES;
}

- (NSString *) _newQarg
{
// get argument (might be quoted)
  NSUInteger	ap = --this->cp;	// argument start pointer
  int 		c = cget();		// refetch first character
  int 		len;
  BOOL 		containsEntity = NO;
  NSString 	*qs;

#if EXTRA_DEBUG
  NSLog(@"_newQarg: %02x %c", c, isprint(c)?c: ' ');
#endif
  if (c == '\"')
    {
      do
	{
	  c = cget();
	  if (c == EOF)
	    {
	      return nil;  // unterminated!
	    }
	  if ('&' == c)
            {
              containsEntity = YES;
            }
	}
      while (c != '\"');
      len = this->cp - ap - 2;
      ap++;
    }
  else if (c == '\'')
    {
      do
	{
	  c = cget();
	  if (c == EOF)
	    {
	      return nil;  // unterminated!
	    }
	  if ('&' == c)
            {
              containsEntity = YES;
            }
	}
      while (c != '\'');
      len = this->cp - ap - 2;
      ap++;
    }
  else
    {
      /* strict XML requires quoting (?)
      if (!this->acceptHTML)
        ;
      */
      while (!isspace(c)
        && c != '>' && c != '/' && c != '?' && c != '=' && c != EOF)
        {
          if ('&' == c)
            {
              containsEntity = YES;
            }
          c = cget();
        }
      this->cp--;  // go back to terminating character
      len = this->cp - ap;
    }
  if (YES == containsEntity)
    {
      NSString                  *seg;
      NSMutableString           *m;
      const unsigned char       *start = addr(ap);
      const unsigned char       *end = start + len;
      const unsigned char       *ptr = start;

      m = [[NSMutableString alloc] initWithCapacity: len];
      while (ptr < end)
        {
          while (ptr < end && *ptr != '&')
            {
              ptr++;
            }
          if (ptr > start)
            {
              seg = NewUTF8STR(start, ptr - start);
              if (nil == seg)
                {
                  RELEASE(m);
                  [self _parseError: @"invalid character in quoted string"
                               code: NSXMLParserInvalidCharacterError];
                  return nil;
                }
              [m appendString: seg];
              RELEASE(seg);
              start = ptr;
            }
          else
            {
              while (ptr < end && *ptr != ';')
                {
                  ptr++;
                }
              seg = [self _newEntity: start + 1 length: ptr - start - 1];
              [m appendString: seg];
              RELEASE(seg);
              if (ptr < end)
                {
                  ptr++;        // Step past trailing semicolon
                }
              start = ptr;
            }
        }
      return m;
    }
  qs = NewUTF8STR(addr(ap), len);
  if (nil == qs)
    {
      [self _parseError: @"invalid character in quoted string"
                   code: NSXMLParserInvalidCharacterError];
      return nil;
    }
  return qs;
}

- (BOOL) parse
{
// read XML (or HTML) file
  NSUInteger	vp = this->cp;	// value position
  int 		c;

  /* Start by accumulating ignorable whitespace.
   */
  this->ignorable = YES;
  this->whitespace = YES;
  c = cget();  // get first character
  while (!this->abort)
    {
#if EXTRA_DEBUG
    NSLog(@"_nextelement %02x %c", c, isprint(c)?c: ' ');
#endif
      switch (c)
        {
          case '\r': 
            this->column = 0;
            break;

          case '\n': 
            this->line++;
            this->column = 0;
	    break;

          case '<':
	    /* Whitespace immediately before an element is always ignorable.
	     */
	    this->ignorable = YES; /* Fall through to push out data */
          case EOF: 
          case '&': 
            {
              /* push out any characters that have been collected so far
               */
              if (this->cp - vp > 1)
                {
		  NSUInteger	p;
		  NSString	*s;

		  p = this->cp - 1;
		  if (YES == this->ignorable)
		    {
		      if (YES == this->whitespace)
			{
			  p = vp;	// all whitespace
			}
		      else
			{
			  /* step through trailing whitespace (if any)
			   */
			  while (p > vp && isspace(addr(p)[-1]))
			    {
			      p--;
			    }
			}
		    }
		  if (YES == this->hasElement)
		    {
		      if (p - vp > 0)
			{
			  if (this->foundCharacters != 0)
			    {
			      s = NewUTF8STR(addr(vp), p - vp);
                              if (nil == s)
                                {
                                  [self _parseError: @"invalid character data"
                                     code: NSXMLParserInvalidCharacterError];
                                  continue;
                                }
                              else
                                {
                                  /* Process this data as characters
                                   */
                                  (*this->foundCharacters)(_del,
                                    foundCharactersSel, self, s);
                                  RELEASE(s);
                                }
			    }
			}
		      if (p < this->cp - 1)
			{
			  if (this->foundIgnorable != 0)
			    {
			      s = NewUTF8STR(addr(p), this->cp - p - 1);
                              if (nil == s)
                                {
                                  [self _parseError: @"invalid whitespace data"
                                     code: NSXMLParserInvalidCharacterError];
                                }
                              else
                                {
                                  /* Process data as ignorable whitespace
                                   */
                                  (*this->foundIgnorable)(_del,
                                    foundIgnorableSel, self, s);
                                  RELEASE(s);
                                }
			    }
			  else if (this->foundCharacters != 0)
			    {
			      s = NewUTF8STR(addr(p), this->cp - p - 1);
                              if (nil == s)
                                {
                                  [self _parseError: @"invalid character data"
                                     code: NSXMLParserInvalidCharacterError];
                                }
                              else
                                {
                                  /* Process data as characters
                                   */
                                  (*this->foundCharacters)(_del,
                                    foundCharactersSel, self, s);
                                  RELEASE(s);
                                }
			    }
			}
		    }
                  vp = this->cp;
                }
            }
        }

      switch (c)
        {
          default: 
	    if (YES == this->whitespace && !isspace(c))
	      {
		if (YES == this->ignorable && this->cp - vp > 1)
		  {
		    NSString	*s;

		    /* We have accumulated ignorable whitespace ...
		     * push it out.
		     */
		    if (this->foundIgnorable != 0)
		      {
			s = NewUTF8STR(addr(vp), this->cp - vp - 1);
                        if (nil == s)
                          {
                            [self _parseError: @"invalid whitespace data"
                               code: NSXMLParserInvalidCharacterError];
                          }
                        else
                          {
                            (*this->foundIgnorable)(_del,
                              foundIgnorableSel, self, s);
			    RELEASE(s);
                          }
		      }
		    else if (this->foundCharacters != 0)
		      {
			s = NewUTF8STR(addr(vp), this->cp - vp - 1);
                        if (nil == s)
                          {
                            [self _parseError: @"invalid character data"
                               code: NSXMLParserInvalidCharacterError];
                          }
                        else
                          {
                            (*this->foundCharacters)(_del,
                              foundCharactersSel, self, s);
                            RELEASE(s);
			  }
		      }
		    vp = this->cp - 1;
		  }
		/* We have read non-space data, so whitespace is no longer
		 * ignorable, and the buffer no longer contains only space.
		 */
		this->ignorable = NO;
		this->whitespace = NO;
	      }
            c = cget();  // just collect until we push out (again)
            continue;

          case EOF:
            {
              if ([this->tagPath count] != 0)
                {
                  if (!this->acceptHTML)
                    {
                      /* strict XML nesting error
                       */
                      return [self _parseError: @"unexpected end of file"
			code: NSXMLParserNotWellBalancedError];
                    }
                while ([this->tagPath count] > 0)
                  {
                    // lazily close all open tags
                    if (this->didEndElement != 0)
                      {
                        (*this->didEndElement)(_del,
			  didEndElementSel, self,
                          [this->tagPath lastObject], nil, nil);
                      }
                    [this->tagPath removeLastObject];  // pop from stack
                  }
                }
#if EXTRA_DEBUG
              NSLog(@"parserDidEndDocument: ");
#endif
              
              if ([_del respondsToSelector: @selector(parserDidEndDocument:)])
                {
                  [_del parserDidEndDocument: self];
                }
              return YES;
            }

          case '&': 
            {
              NSString  *entity;

	      /* After any entity, whitespace is no longer ignorable, but
	       * we will have an empty buffer to accumulate it.
	       */
	      this->ignorable = NO;
	      this->whitespace = YES;

              if ([self _parseEntity: &entity] == NO)
                {
                  return [self _parseError: @"empty entity name"
		    code: NSXMLParserEntityRefAtEOFError];
                }
	      if (this->foundCharacters != 0)
		{
		  (*this->foundCharacters)(_del,
		    foundCharactersSel, self, entity);
                }
	      RELEASE(entity);
              vp = this->cp;  // next value sequence starts here
              c = cget();  // first character behind ;
              continue;
            }

          case '<': 
            {
              NSString                  *tag;
              NSMutableDictionary       *attributes;
              NSString                  *arg;
              NSUInteger		tp = this->cp;  // tag position
	      NSUInteger		sp = tp - 1;	// Open angle bracket

	      /* After processing a tag, whitespace will be ignorable and
	       * we can start accumulating it in our buffer.
	       */
	      this->ignorable = YES;
	      this->whitespace = YES;

              if (this->cp < this->cend-3
                && strncmp((char *)addr(this->cp), "!--", 3) == 0)
                {
                  /* start of comment skip all characters until "-->"
                   */
                  this->cp += 3;
		  tp = this->cp;
                  while (this->cp < this->cend-3
                         && strncmp((char *)addr(this->cp), "-->", 3) != 0)
                    {
                      this->cp++;  // search
                    }
		  if (this->foundComment != 0)
		    {
		      NSString	*c = NewUTF8STR(addr(tp), this->cp - tp);

                      if (nil == c)
                        {
                          [self _parseError: @"invalid comment data"
                             code: NSXMLParserInvalidCharacterError];
                        }
                      else
                        {
                          (*this->foundComment)(_del,
                            foundCommentSel, self, c);
			  RELEASE(c);
                        }
		    }
                  this->cp += 3;	// might go beyond cend ... ok
                  vp = this->cp;	// value might continue
                  c = cget();		// get first character after comment
                  continue;
                }
              if (this->cp < this->cend-8
                && strncmp((char *)addr(this->cp), "![CDATA[", 8) == 0)
		{
                  /* start of CDATA skip all characters until "]>"
                   */
                  this->cp += 8;
		  tp = this->cp;
                  while (this->cp < this->cend-3
                    && strncmp((char *)addr(this->cp), "]]>", 3) != 0)
                    {
                      this->cp++;  // search
                    }
		  if (this->foundCDATA != 0)
		    {
		      NSData	*d;

		      d = [[NSData alloc] initWithBytes: addr(tp)
						 length: this->cp - tp];
		      (*this->foundCDATA)(_del,
			foundCDATASel, self, d);
		      RELEASE(d);
		    }
                  this->cp += 3;	// might go beyond cend ... ok
                  vp = this->cp;	// value might continue
                  c = cget();		// get first character after CDATA
                  continue;
                }

              c = cget(); // get first character of tag
              if (c == '/')
                {
                  c = cget(); // closing tag </tag begins
                }
              else if (c == '?')
                {
                  /* special tag <?tag begins
                   */
                  c = cget();  // include in tag string
                  //  NSLog(@"special tag <? found");
                  /* FIXME: this->should process this tag in a special
                   * way so that e.g. <?php any PHP script ?> is read
                   * as a single tag!
                   * to do this properly, we need a notion of comments
                   * and quoted string constants...
                   */
                }
              else if (c == '!')
                {
                  /* declaration <!tag begins
                   */
		  [self _processDeclaration];
		  vp = this->cp;    // prepare for next value
		  c = cget();  // fetch next character
		  continue;
		}

              while (c != EOF && !isspace(c)
                && c != '>' && c != '/'  && c != '?')
                {
                  c = cget(); // scan tag until we find a delimiting character
                }
              if (*addr(tp) == '/')
                {
                  tag = NewUTF8STR(addr(tp + 1), this->cp - tp - 2);
                }
              else
                {
                  tag = NewUTF8STR(addr(tp), this->cp - tp - 1);
                }
              if (nil == tag)
                {
                  [self _parseError: @"invalid character in tag"
                     code: NSXMLParserInvalidCharacterError];
                }
#if EXTRA_DEBUG
              NSLog(@"tag=%@ - %02x %c", tag, c, isprint(c)?c: ' ');
#endif

	      /* Create an attributes dictionary for this tag,
	       * using default values if available.
	       */
	      attributes = [[this->defaults objectForKey: tag] mutableCopy];
	      if (nil == attributes)
		{
                  attributes
		    = [[NSMutableDictionary alloc] initWithCapacity: 5];
		}

              while (isspace(c))
                {
                  c = cget();
                }
              while (c != EOF)
                {
                  if (c == '/' && *addr(tp) != '/')
                    {
                      // appears to be a />
                      c = cget();
                      if (c != '>')
                        {
			  RELEASE(attributes);
			  RELEASE(tag);
                          return [self _parseError: @"<tag/ is missing the >"
			    code: NSXMLParserGTRequiredError];
                        }
                      [self _processTag: tag
                                  isEnd: NO
                         withAttributes: attributes];
                      [self _processTag: tag isEnd: YES withAttributes: nil];
                      break;
                    }

                  if (c == '?' && *addr(tp) == '?')
                    {
                      // appears to be a ?>
                      c = cget();
                      if (c != '>')
                        {
			  RELEASE(attributes);
			  RELEASE(tag);
                          return [self _parseError:
                            @"<?tag ...? is missing the >"
			    code: NSXMLParserGTRequiredError];
                        }
		      /* If this is the <?xml  header, the opening angle
		       * bracket MUST be at the start of the data.
		       */
		      if ([tag isEqualToString: @"?xml"] && sp != 0)
			{
			  RELEASE(attributes);
			  RELEASE(tag);
			  return [self _parseError: @"bad <?xml > preamble"
			    code: NSXMLParserDocumentStartError];
			}
                      [self _processTag: tag
                                  isEnd: NO
                         withAttributes: attributes];  // single <?tag ...?>
                      break; // done
                    }
                  // this should also allow for line break and tab
                  while (isspace(c))
                    {
                      c = cget();
                    }
                  if (c == '>')
                    {
                      [self _processTag: tag
                                  isEnd: (*addr(tp) == '/')
                         withAttributes: attributes];
                      break;
                    }
                  /* get next argument (eats up to /, ?, >, =, space)
                   */
                  arg = [self _newQarg];
#if EXTRA_DEBUG
                  NSLog(@"arg=%@", arg);
#endif
                  if (!this->acceptHTML && [arg length] == 0)
                    {
                      RELEASE(arg);
                      RELEASE(tag);
                      RELEASE(attributes);
                      return [self _parseError: @"empty attribute name"
			code: NSXMLParserAttributeNotStartedError];
                    }
                  c = cget();
                  while (isspace(c))
                    {
                      c = cget();
                    }
                  if (c == '=')
                    {
		      NSString	*val;

                      // explicit assignment
                      c = cget();  // skip =
		      while (isspace(c))
			{
			  c = cget();
			}
		      val = [self _newQarg];
                      [attributes setObject: val forKey: arg];
		      RELEASE(val);
                      c = cget();  // get character behind qarg value
		      while (isspace(c))
			{
			  c = cget();
			}
                    }
                  else  // implicit
                    {
                      [attributes setObject: @"" forKey: arg];
                    }                    
		  RELEASE(arg);
                }
	      RELEASE(attributes);
	      RELEASE(tag);
              vp = this->cp;    // prepare for next value
              c = cget();  // skip > and fetch next character
            }
        }
    }
  return [self _parseError: @"aborted"
    code: NSXMLParserDelegateAbortedParseError];
}

- (BOOL) acceptsHTML
{
  return this->acceptHTML;
}

- (BOOL) shouldProcessNamespaces
{
  return this->shouldProcessNamespaces;
}

- (BOOL) shouldReportNamespacePrefixes
{
  return this->shouldReportNamespacePrefixes;
}

- (BOOL) shouldResolveExternalEntities
{
  return this->shouldResolveExternalEntities;
}

- (void) setShouldProcessNamespaces: (BOOL)aFlag
{
  this->shouldProcessNamespaces = aFlag;
}

- (void) setShouldReportNamespacePrefixes: (BOOL)aFlag
{
  this->shouldReportNamespacePrefixes = aFlag;
}

- (void) setShouldResolveExternalEntities: (BOOL)aFlag
{
  this->shouldResolveExternalEntities = aFlag;
}

- (void) _setAcceptHTML: (BOOL) flag
{
  this->acceptHTML = flag;
}

- (NSString *) publicID
{
  return [self notImplemented: _cmd];
}

- (NSString *) systemID
{
  return [self notImplemented: _cmd];
}

@end

@implementation NSObject (NSXMLParserDelegateEventAdditions)
- (NSData*) parser: (NSXMLParser*)aParser
  resolveExternalEntityName: (NSString*)aName
  systemID: (NSString*)aSystemID
{
  return nil;
}

- (void) parser: (NSXMLParser*)aParser
  didEndElement: (NSString*)anElementName
  namespaceURI: (NSString*)aNamespaceURI
  qualifiedName: (NSString*)aQualifierName
{
}

- (void) parser: (NSXMLParser*)aParser
  didEndMappingPrefix: (NSString*)aPrefix
{
}

- (void) parser: (NSXMLParser*)aParser
  didStartElement: (NSString*)anElementName
  namespaceURI: (NSString*)aNamespaceURI
  qualifiedName: (NSString*)aQualifierName
  attributes: (NSDictionary*)anAttributeDict
{
}

- (void) parser: (NSXMLParser*)aParser
  didStartMappingPrefix: (NSString*)aPrefix
  toURI: (NSString*)aNamespaceURI
{
}

- (void) parser: (NSXMLParser*)aParser
  foundAttributeDeclarationWithName: (NSString*)anAttributeName
  forElement: (NSString*)anElementName
  type: (NSString*)aType
  defaultValue: (NSString*)aDefaultValue
{
}

- (void) parser: (NSXMLParser*)aParser
  foundCDATA: (NSData*)aBlock
{
}

- (void) parser: (NSXMLParser*)aParser
  foundCharacters: (NSString*)aString
{
}

- (void) parser: (NSXMLParser*)aParser
  foundComment: (NSString*)aComment
{
}

- (void) parser: (NSXMLParser*)aParser
  foundElementDeclarationWithName: (NSString*)anElementName
  model: (NSString*)aModel
{
}

- (void) parser: (NSXMLParser*)aParser
  foundExternalEntityDeclarationWithName: (NSString*)aName
  publicID: (NSString*)aPublicID
  systemID: (NSString*)aSystemID
{
}

- (void) parser: (NSXMLParser*)aParser
  foundIgnorableWhitespace: (NSString*)aWhitespaceString
{
}

- (void) parser: (NSXMLParser*)aParser
  foundInternalEntityDeclarationWithName: (NSString*)aName
  value: (NSString*)aValue
{
}

- (void) parser: (NSXMLParser*)aParser
  foundNotationDeclarationWithName: (NSString*)aName
  publicID: (NSString*)aPublicID
  systemID: (NSString*)aSystemID
{
}

- (void) parser: (NSXMLParser*)aParser
  foundProcessingInstructionWithTarget: (NSString*)aTarget
  data: (NSString*)aData
{
}

- (void) parser: (NSXMLParser*)aParser
  foundUnparsedEntityDeclarationWithName: (NSString*)aName
  publicID: (NSString*)aPublicID
  systemID: (NSString*)aSystemID
  notationName: (NSString*)aNotationName
{
}

- (void) parser: (NSXMLParser*)aParser
  parseErrorOccurred: (NSError*)anError
{
}

- (void) parser: (NSXMLParser*)aParser
  validationErrorOccurred: (NSError*)anError
{
}

- (void) parserDidEndDocument: (NSXMLParser*)aParser
{
}

- (void) parserDidStartDocument: (NSXMLParser*)aParser
{
}

@end

#if	 defined(HAVE_LIBXML)

#include <GNUstepBase/GSXML.h>

@interface	NSXMLSAXHandler : GSSAXHandler
{
@public
  id		_delegate;      // Not retained
  id		_owner;         // Not retained
  NSError	*_lastError;
  BOOL		_shouldProcessNamespaces;
  BOOL		_shouldReportNamespacePrefixes;
  BOOL		_shouldResolveExternalEntities;
  NSMutableArray        *_namespaces;
}
- (void) _setOwner: (id)owner;
@end

@implementation	NSXMLSAXHandler

+ (void) initialize
{
  GSMakeWeakPointer(self, "_delegate");
  GSMakeWeakPointer(self, "_owner");
}

- (void) dealloc
{
  DESTROY(_namespaces);
  DESTROY(_lastError);
  [super dealloc];
}

- (id) init
{
  if ((self = [super init]) != nil)
    {
      _namespaces = [NSMutableArray new];
    }
  return self;
}

- (void) endDocument
{
  [_delegate parserDidEndDocument: _owner];
}
- (void) startDocument
{
  [_delegate parserDidStartDocument: _owner];
}

- (void) startElement: (NSString*)elementName
	       prefix: (NSString*)prefix
		 href: (NSString*)href
	   attributes: (NSMutableDictionary*)elementAttributes
	   namespaces: (NSMutableDictionary*)elementNamespaces
{
  NSString      *qName = elementName;

  if ([prefix length] > 0)
    {
      qName = [NSString stringWithFormat: @"%@:%@", prefix, qName];
    }

  if (elementAttributes == nil)
    {
      elementAttributes = [NSMutableDictionary dictionary];
    }

  if ([elementNamespaces count] > 0)
    {
      [_namespaces addObject: [elementNamespaces allKeys]];
      if (_shouldReportNamespacePrefixes)
        {
          NSEnumerator  *e = [elementNamespaces keyEnumerator];
          NSString      *k;

          while ((k = [e nextObject]) != nil)
            {
              NSString  *v = [elementNamespaces objectForKey: k];

              [_delegate parser: _owner
                didStartMappingPrefix: k
                toURI: v];
            }
        }
    }
  else
    {
      [_namespaces addObject: null];
    }

  if (_shouldProcessNamespaces)
    {
      [_delegate parser: _owner
	didStartElement: elementName
	   namespaceURI: (nil == href) ? @"" : href
	  qualifiedName: qName
	     attributes: elementAttributes];
    }
  else
    {
      /* When we are not handling namespaces specially, any namespaces
       * should appear as attributes of the element.
       */
      if ([elementNamespaces count] > 0)
        {
          NSEnumerator  *e = [elementNamespaces keyEnumerator];
          NSString      *k;

          while ((k = [e nextObject]) != nil)
            {
              NSString  *v = [elementNamespaces objectForKey: k];

              if ([k length] == 0)
                {
                  [elementAttributes setObject: v forKey: @"xmlns"];
                }
              else
                {
                  k = [@"xmlns:" stringByAppendingString: k];
                  [elementAttributes setObject: v forKey: k];
                }
            }
        }
      [_delegate parser: _owner
	didStartElement: qName
	   namespaceURI: nil
	  qualifiedName: nil
	     attributes: elementAttributes];
    }
}

- (void) endElement: (NSString*)elementName
	     prefix: (NSString*)prefix
	       href: (NSString*)href
{
  NSString      *qName = elementName;

  if ([prefix length] > 0)
    {
      qName = [NSString stringWithFormat: @"%@:%@", prefix, qName];
    }
  if (_shouldProcessNamespaces)
    {
      [_delegate parser: _owner
	  didEndElement: elementName
	   namespaceURI: (nil == href) ? @"" : href
	  qualifiedName: qName];
    }
  else
    {
      [_delegate parser: _owner
	  didEndElement: qName
	   namespaceURI: nil
	  qualifiedName: nil];
    }

  if (_shouldReportNamespacePrefixes)
    {
      id        o = [_namespaces lastObject];

      if (o != (id)null)
        {
          NSEnumerator  *e = [(NSArray*)o objectEnumerator];
          NSString      *k;

          while ((k = [e nextObject]) != nil)
            {
              [_delegate parser: _owner didEndMappingPrefix: k];
            }
        }
    }
  [_namespaces removeLastObject];
}
- (void) attribute: (NSString*) name value: (NSString*)value
{
	// FIXME
}
- (void) characters: (NSString*)string
{
  [_delegate parser: _owner
    foundCharacters: string];
}
- (void) ignoreWhitespace: (NSString*) ch
{
  [_delegate parser: _owner
    foundIgnorableWhitespace: ch];
}
- (void) processInstruction: (NSString*)targetName data: (NSString*)PIdata
{
  [_delegate parser: _owner
    foundProcessingInstructionWithTarget: targetName
    data: PIdata];
}
- (void) comment: (NSString*) value
{
  [_delegate parser: _owner
    foundComment: value];
}
- (void) cdataBlock: (NSData*)value
{
  [_delegate parser: _owner
    foundCDATA: value];
}

/**
 * Called to return the filename from which an entity should be loaded.
 */
- (NSString*) loadEntity: (NSString*)publicId
		      at: (NSString*)location
{
  return nil;
}

/**
 * An old global namespace has been parsed.
 */
- (void) namespaceDecl: (NSString*)name
		  href: (NSString*)href
		prefix: (NSString*)prefix
{
}

- (void) notationDecl: (NSString*)name
	       public: (NSString*)publicId
	       system: (NSString*)systemId
{
  [_delegate parser: _owner
    foundNotationDeclarationWithName: name
    publicID: publicId
    systemID: systemId];
}

/**
 * An entity definition has been parsed.
 */
- (void) entityDecl: (NSString*)name
	       type: (NSInteger)type
	     public: (NSString*)publicId
	     system: (NSString*)systemId
	    content: (NSString*)content
{
}

- (void) attributeDecl: (NSString*)nameElement
		  name: (NSString*)name
		  type: (NSInteger)type
	  typeDefValue: (NSInteger)defType
	  defaultValue: (NSString*)value
{
  [_delegate parser: _owner
    foundAttributeDeclarationWithName: name
    forElement: nameElement
    type: @""		// FIXME
    defaultValue: value];
}

- (void) elementDecl: (NSString*)name
		type: (NSInteger)type
{
  [_delegate parser: _owner
    foundElementDeclarationWithName: name
    model: @""];	// FIXME
}

/**
 * What to do when an unparsed entity declaration is parsed.
 */
- (void) unparsedEntityDecl: (NSString*)name
		     public: (NSString*)publicId
		     system: (NSString*)systemId
	       notationName: (NSString*)notation
{
}

/**
 * Called when an entity reference is detected.
 */
- (void) reference: (NSString*) name
{
}

/**
 * An old global namespace has been parsed.
 */
- (void) globalNamespace: (NSString*)name
		    href: (NSString*)href
		  prefix: (NSString*)prefix
{
}

/**
 * Called when a warning message needs to be output.
 */
- (void) warning: (NSString*)e
{
  GSPrintf(stderr, @"%@", e);
}

- (void) error: (NSString*)e
{
  NSError	*error;
  NSDictionary	*d;

  d = [NSDictionary dictionaryWithObjectsAndKeys:
    e, NSLocalizedDescriptionKey,
    nil];
  error = [NSError errorWithDomain: NSXMLParserErrorDomain
			      code: 0
			  userInfo: d];
  ASSIGN(_lastError, error);
  [_delegate parser: _owner
    parseErrorOccurred: error];
}
- (void) fatalError: (NSString*)e
{
  [self error: e];
}
- (void) warning: (NSString*)e
       colNumber: (NSInteger)colNumber
      lineNumber: (NSInteger)lineNumber
{
  e = [NSString stringWithFormat: @"at line: %d column: %d ... %@",
    (int)lineNumber, (int)colNumber, e];
  [self warning: e];
}
- (void) error: (NSString*)e
     colNumber: (NSInteger)colNumber
    lineNumber: (NSInteger)lineNumber
{
  e = [NSString stringWithFormat: @"at line: %d column: %d ... %@",
    (int)lineNumber, (int)colNumber, e];
  [self error: e];
}
- (void) fatalError: (NSString*)e
          colNumber: (NSInteger)colNumber
         lineNumber: (NSInteger)lineNumber
{
  e = [NSString stringWithFormat: @"at line: %d column: %d ... %@",
    (int)lineNumber, (int)colNumber, e];
  [self fatalError: e];
}
- (NSInteger) hasInternalSubset
{
  return 0;
}
- (BOOL) internalSubset: (NSString*)name
	     externalID: (NSString*)externalID
	       systemID: (NSString*)systemID
{
  return NO;
}
- (NSInteger) hasExternalSubset
{
  return 0;
}
- (BOOL) externalSubset: (NSString*)name
	     externalID: (NSString*)externalID
	       systemID: (NSString*)systemID
{
  return NO;
}
- (void*) getEntity: (NSString*)name
{
  return 0;
}
- (void*) getParameterEntity: (NSString*)name
{
  return 0;
}

- (void) _setOwner: (id)owner
{
  _owner = owner;
}

@end



@implementation GSStrictXMLParser

+ (void) initialize
{
  if (null == nil)
    {
      null = RETAIN([NSNull null]);
      RELEASE([NSObject leakAt: &null]);
    }
}

#define	myParser	((GSXMLParser*)_parser)
#define	myHandler	((NSXMLSAXHandler*)_handler)

- (void) abortParsing
{
  NSDictionary	*d;
  NSString	*e;
  NSError	*error;

  e = @"Parsing aborted";
  d = [NSDictionary dictionaryWithObjectsAndKeys:
    e, NSLocalizedDescriptionKey,
    nil];
  error = [NSError errorWithDomain: NSXMLParserErrorDomain
			      code: 0
			  userInfo: d];
  ASSIGN(myHandler->_lastError, error);
  [myHandler->_delegate parser: myHandler->_owner parseErrorOccurred: error];
  [myParser abortParsing];
}

- (void) dealloc
{
  DESTROY(_parser);
  DESTROY(_handler);
  [super dealloc];
}

- (id) delegate
{
  return myHandler->_delegate;
}

- (id) initWithContentsOfURL: (NSURL*)anURL
{
  _handler = [NSXMLSAXHandler new];
  [myHandler _setOwner: self];
  _parser = [[GSXMLParser alloc] initWithSAXHandler: myHandler
                                  withContentsOfURL: anURL];
  [(GSXMLParser*)_parser substituteEntities: YES];
  return self;
}

- (id) initWithData: (NSData*)data
{
  _handler = [NSXMLSAXHandler new];
  [myHandler _setOwner: self];
  _parser = [[GSXMLParser alloc] initWithSAXHandler: myHandler
                                           withData: data];
  [(GSXMLParser*)_parser substituteEntities: YES];
  return self;
}

- (id) initWithStream: (NSInputStream*)stream
{
  _handler = [NSXMLSAXHandler new];
  [myHandler _setOwner: self];
  _parser = [[GSXMLParser alloc] initWithSAXHandler: myHandler
                                    withInputStream: stream];
  [(GSXMLParser*)_parser substituteEntities: YES];
  return self;
}

- (BOOL) parse
{
  BOOL	result;

  result = [[myHandler parser] parse];
  return result;
}

- (NSError*) parserError
{
  return (nil == myHandler) ? nil : myHandler->_lastError;
}

- (void) setDelegate: (id)delegate
{
  myHandler->_delegate = delegate;
}

- (void) setShouldProcessNamespaces: (BOOL)aFlag
{
  myHandler->_shouldProcessNamespaces = aFlag;
}

- (void) setShouldReportNamespacePrefixes: (BOOL)aFlag
{
  myHandler->_shouldReportNamespacePrefixes = aFlag;
}

- (void) setShouldResolveExternalEntities: (BOOL)aFlag
{
  myHandler->_shouldResolveExternalEntities = aFlag;
}

- (BOOL) shouldProcessNamespaces
{
  return myHandler->_shouldProcessNamespaces;
}

- (BOOL) shouldReportNamespacePrefixes
{
  return myHandler->_shouldReportNamespacePrefixes;
}

- (BOOL) shouldResolveExternalEntities
{
  return myHandler->_shouldResolveExternalEntities;
}

@end

@implementation NSXMLParser (NSXMLParserLocatorAdditions)
- (NSInteger) columnNumber
{
  return [myParser columnNumber];
}

- (NSInteger) lineNumber
{
  return [myParser lineNumber];
}

- (NSString*) publicID
{
  return [myParser publicID];
}

- (NSString*) systemID
{
  return [myParser systemID];
}

@end

#else
@implementation GSStrictXMLParser
@end
#endif
