/** NSURL.m - Class NSURL
   Copyright (C) 1999 Free Software Foundation, Inc.

   Written by: 	Manuel Guesdon <mguesdon@sbuilders.com>
   Date: 	Jan 1999

   Rewrite by: 	Richard Frith-Macdonald <rfm@gnu.org>
   Date: 	Jun 2002

   Add'l by:    Gregory John Casamento <greg.casamento@gmail.com>  
   Date: 	Jan 2020

   This file is part of the GNUstep Library.

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

   <title>NSURL class reference</title>
   $Date$ $Revision$
*/

/*
Note from Manuel Guesdon:
* I've made some test to compare apple NSURL results
and GNUstep NSURL results but as there this class is not very documented, some
function may be incorrect
* I've put 2 functions to make tests. You can add your own tests
* Some functions are not implemented
*/

#define	GS_NSURLQueryItem_IVARS \
  NSString *_name; \
  NSString *_value; 

#define	GS_NSURLComponents_IVARS \
  NSString *_string; \
  NSString *_fragment; \
  NSString *_host; \
  NSString *_password; \
  NSString *_path; \
  NSNumber *_port; \
  NSArray  *_queryItems; \
  NSString *_scheme; \
  NSString *_user; \
  NSRange   _rangeOfFragment; \
  NSRange   _rangeOfHost; \
  NSRange   _rangeOfPassword; \
  NSRange   _rangeOfPath; \
  NSRange   _rangeOfPort; \
  NSRange   _rangeOfQuery; \
  NSRange   _rangeOfQueryItems; \
  NSRange   _rangeOfScheme; \
  NSRange   _rangeOfUser; \
  BOOL      _dirty;

#import "common.h"
#define	EXPOSE_NSURL_IVARS	1
#import "Foundation/NSArray.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSError.h"
#import "Foundation/NSException.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSMapTable.h"
#import "Foundation/NSPortCoder.h"
#import "Foundation/NSRunLoop.h"
#import "Foundation/NSURL.h"
#import "Foundation/NSURLHandle.h"
#import "Foundation/NSValue.h"
#import "Foundation/NSCharacterSet.h"
#import "Foundation/NSString.h"

#import "GNUstepBase/NSURL+GNUstepBase.h"

NSString * const NSURLErrorDomain = @"NSURLErrorDomain";
NSString * const NSErrorFailingURLStringKey = @"NSErrorFailingURLStringKey";

@interface	NSURL (GSPrivate)
- (NSURL*) _URLBySettingPath: (NSString*)newPath; 
@end

@implementation	NSURL (GSPrivate)

- (NSURL*) _URLBySettingPath: (NSString*)newPath 
{
  if ([self isFileURL]) 
    {
      return [NSURL fileURLWithPath: newPath];
    }
  else
    {
      NSURL	*u;

      u = [[NSURL alloc] initWithScheme: [self scheme]
				   user: [self user]
			       password: [self password]
				   host: [self host]
				   port: [self port]
			       fullPath: newPath
			parameterString: [self parameterString]
				  query: [self query]
			       fragment: [self fragment]];
      return [u autorelease];
    }
}

@end

/*
 * Structure describing a URL.
 * All the char* fields may be NULL pointers, except path, which
 * is *always* non-null (though it may be an empty string).
 */
typedef struct {
  id	absolute;		// Cache absolute string or nil
  char	*scheme;
  char	*user;
  char	*password;
  char	*host;
  char	*port;
  char	*path;			// May never be NULL
  char	*parameters;
  char	*query;
  char	*fragment;
  BOOL	pathIsAbsolute;
  BOOL	emptyPath;
  BOOL	hasNoPath;
  BOOL	isGeneric;
  BOOL	isFile;
} parsedURL;

#define	myData ((parsedURL*)(self->_data))
#define	baseData ((self->_baseURL == 0)?0:((parsedURL*)(self->_baseURL->_data)))

static NSLock	*clientsLock = nil;

/*
 * Local utility functions.
 */
static char *buildURL(parsedURL *base, parsedURL *rel, BOOL standardize);
static id clientForHandle(void *data, NSURLHandle *hdl);
static char *findUp(char *str);
static char *unescape(const char *from, char * to);

/**
 * Build an absolute URL as a C string
 */
static char *buildURL(parsedURL *base, parsedURL *rel, BOOL standardize)
{
  const char	*rpath;
  char		*buf;
  char		*ptr;
  char		*tmp;
  int		l;
  unsigned int	len = 1;

  if (NO == rel->hasNoPath)
    {
      len += 1;                         // trailing '/' to be added
    }
  if (rel->scheme != 0)
    {
      len += strlen(rel->scheme) + 3;	// scheme://
    }
  else if (YES == rel->isGeneric)
    {
      len += 2;                         // need '//' even if no scheme
    }
  if (rel->user != 0)
    {
      len += strlen(rel->user) + 1;	// user...@
    }
  if (rel->password != 0)
    {
      len += strlen(rel->password) + 1;	// :password
    }
  if (rel->host != 0)
    {
      len += strlen(rel->host) + 1;	// host.../
    }
  if (rel->port != 0)
    {
      len += strlen(rel->port) + 1;	// :port
    }
  if (rel->path != 0)
    {
      rpath = rel->path;
    }
  else
    {
      rpath = "";
    }
  len += strlen(rpath) + 1;	// path
  if (base != 0 && base->path != 0)
    {
      len += strlen(base->path) + 1;	// path
    }
  if (rel->parameters != 0)
    {
      len += strlen(rel->parameters) + 1;	// ;parameters
    }
  if (rel->query != 0)
    {
      len += strlen(rel->query) + 1;		// ?query
    }
  if (rel->fragment != 0)
    {
      len += strlen(rel->fragment) + 1;		// #fragment
    }

  ptr = buf = (char*)NSZoneMalloc(NSDefaultMallocZone(), len);

  if (rel->scheme != 0)
    {
      l = strlen(rel->scheme);
      memcpy(ptr, rel->scheme, l);
      ptr += l;
      *ptr++ = ':';
    }
  if (rel->isGeneric == YES
    || rel->user != 0 || rel->password != 0 || rel->host != 0 || rel->port != 0)
    {
      *ptr++ = '/';
      *ptr++ = '/';
      if (rel->user != 0 || rel->password != 0)
	{
	  if (rel->user != 0)
	    {
	      l = strlen(rel->user);
	      memcpy(ptr, rel->user, l);
	      ptr += l;
	    }
	  if (rel->password != 0)
	    {
	      *ptr++ = ':';
	      l = strlen(rel->password);
	      memcpy(ptr, rel->password, l);
	      ptr += l;
	    }
	  if (rel->host != 0 || rel->port != 0)
	    {
	      *ptr++ = '@';
	    }
	}
      if (rel->host != 0)
	{
	  l = strlen(rel->host);
	  memcpy(ptr, rel->host, l);
	  ptr += l;
	}
      if (rel->port != 0)
	{
	  *ptr++ = ':';
	  l = strlen(rel->port);
	  memcpy(ptr, rel->port, l);
	  ptr += l;
	}
    }

  /*
   * Now build path.
   */

  tmp = ptr;
  if (rel->pathIsAbsolute == YES)
    {
      if (rel->hasNoPath == NO)
	{
	  *tmp++ = '/';
	}
      l = strlen(rpath);
      memcpy(tmp, rpath, l);
      tmp += l;
    }
  else if (base == 0)
    {
      l = strlen(rpath);
      memcpy(tmp, rpath, l);
      tmp += l;
    }
  else if (rpath[0] == 0)
    {
      if (base->hasNoPath == NO)
	{
	  *tmp++ = '/';
	}
      if (base->path)
	{
	  l = strlen(base->path);
	  memcpy(tmp, base->path, l);
	  tmp += l;
	}
    }
  else
    {
      char	*start = base->path;

      if (start != 0)
        {
          char	*end = strrchr(start, '/');

          if (end != 0)
            {
              *tmp++ = '/';
              memcpy(tmp, start, end - start);
              tmp += (end - start);
            }
        }
      *tmp++ = '/';
      l = strlen(rpath);
      memcpy(tmp, rpath, l);
      tmp += l;
    }
  *tmp = '\0';

  if (standardize == YES)
    {
      /*
       * Compact '/./'  to '/' and strip any trailing '/.'
       */
      tmp = ptr;
      while (*tmp != '\0')
	{
	  if (tmp[0] == '/' && tmp[1] == '.'
	    && (tmp[2] == '/' || tmp[2] == '\0'))
	    {
	      /*
	       * Ensure we don't remove the leading '/'
	       */
	      if (tmp == ptr && tmp[2] == '\0')
		{
		  tmp[1] = '\0';
		}
	      else
		{
		  l = strlen(&tmp[2]) + 1;
		  memmove(tmp, &tmp[2], l);
		}
	    }
	  else
	    {
	      tmp++;
	    }
	}
      /*
       * Reduce any sequence of '/' characters to a single '/'
       */
      tmp = ptr;
      while (*tmp != '\0')
	{
	  if (tmp[0] == '/' && tmp[1] == '/')
	    {
	      l = strlen(&tmp[1]) + 1;
	      memmove(tmp, &tmp[1], l);
	    }
	  else
	    {
	      tmp++;
	    }
	}
      /*
       * Reduce any '/something/../' sequence to '/' and a trailing
       * "/something/.." to ""
       */
      tmp = ptr;
      while ((tmp = findUp(tmp)) != 0)
	{
	  char	*next = &tmp[3];

	  while (tmp > ptr)
	    {
	      if (*--tmp == '/')
		{
		  break;
		}
	    }
	  /*
	   * Ensure we don't remove the leading '/'
	   */
	  if (tmp == ptr && *next == '\0')
	    {
	      tmp[1] = '\0';
	    }
	  else
	    {
	      l = strlen(next) + 1;
	      memmove(tmp, next, l);
	    }
	}
      /*
       * if we have an empty path, we standardize to a single slash.
       */
      tmp = ptr;
      if (*tmp == '\0')
	{
	  memcpy(tmp, "/", 2);
	}
    }
  ptr = &ptr[strlen(ptr)];

  if (rel->parameters != 0)
    {
      *ptr++ = ';';
      l = strlen(rel->parameters);
      memcpy(ptr, rel->parameters, l);
      ptr += l;
    }
  if (rel->query != 0)
    {
      *ptr++ = '?';
      l = strlen(rel->query);
      memcpy(ptr, rel->query, l);
      ptr += l;
    }
  if (rel->fragment != 0)
    {
      *ptr++ = '#';
      l = strlen(rel->fragment);
      memcpy(ptr, rel->fragment, l);
      ptr += l;
    }
  *ptr = '\0';
  return buf;
}

static id clientForHandle(void *data, NSURLHandle *hdl)
{
  id	client = nil;

  if (data != 0)
    {
      [clientsLock lock];
      client = (id)NSMapGet((NSMapTable*)data, hdl);
      [clientsLock unlock];
    }
  return client;
}

/**
 * Locate a '/../ or trailing '/..'
 */
static char *findUp(char *str)
{
  while (*str != '\0')
    {
      if (str[0] == '/' && str[1] == '.' && str[2] == '.'
	&& (str[3] == '/' || str[3] == '\0'))
	{
	  return str;
	}
      str++;
    }
  return 0;
}

/*
 * Check a string to see if it contains only legal data characters
 * or percent escape sequences.
 */
static BOOL legal(const char *str, const char *extras)
{
  const char	*mark = "-_.!~*'()";

  if (str != 0)
    {
      while (*str != 0)
	{
	  if (*str == '%' && isxdigit(str[1]) && isxdigit(str[2]))
	    {
	      str += 3;
	    }
	  else if (isalnum(*str))
	    {
	      str++;
	    }
	  else if (strchr(mark, *str) != 0)
	    {
	      str++;
	    }
	  else if (strchr(extras, *str) != 0)
	    {
	      str++;
	    }
	  else
	    {
	      return NO;
	    }
	}
    }
  return YES;
}

/*
 * Convert percent escape sequences to individual characters.
 */
static char *unescape(const char *from, char * to)
{
  while (*from != '\0')
    {
      if (*from == '%')
	{
	  unsigned char	c;

	  from++;
	  if (isxdigit(*from))
	    {
	      if (*from <= '9')
		{
		  c = *from - '0';
		}
	      else if (*from <= 'F')
		{
		  c = *from - 'A' + 10;
		}
	      else
		{
		  c = *from - 'a' + 10;
		}
	      from++;
	    }
	  else
	    {
	      c = 0;	// Avoid compiler warning
	      [NSException raise: NSGenericException
			  format: @"Bad percent escape sequence in URL string"];
	    }
	  c <<= 4;
	  if (isxdigit(*from))
	    {
	      if (*from <= '9')
		{
		  c |= *from - '0';
		}
	      else if (*from <= 'F')
		{
		  c |= *from - 'A' + 10;
		}
	      else
		{
		  c |= *from - 'a' + 10;
		}
	      from++;
	      *to++ = c;
	    }
	  else
	    {
	      [NSException raise: NSGenericException
			  format: @"Bad percent escape sequence in URL string"];
	    }
	}
      else
	{
	  *to++ = *from++;
	}
    }
  *to = '\0';
  return to;
}



@implementation NSURL

static NSCharacterSet	*fileCharSet = nil;
static NSUInteger	urlAlign;

+ (id) fileURLWithPath: (NSString*)aPath
{
  return AUTORELEASE([[NSURL alloc] initFileURLWithPath: aPath]);
}

+ (id) fileURLWithPath: (NSString*)aPath isDirectory: (BOOL)isDir
{
  return AUTORELEASE([[NSURL alloc] initFileURLWithPath: aPath
					    isDirectory: isDir]);
}

+ (id) fileURLWithPathComponents: (NSArray*)components
{
  return [self fileURLWithPath: [NSString pathWithComponents: components]];
}

+ (void) initialize
{
  if (clientsLock == nil)
    {
      NSGetSizeAndAlignment(@encode(parsedURL), &urlAlign, 0);
      clientsLock = [NSLock new];
      [[NSObject leakAt: &clientsLock] release];
      ASSIGN(fileCharSet, [NSCharacterSet characterSetWithCharactersInString:
        @"!$&'()*+,-./0123456789:=@ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~"]);
    }
}

+ (id) URLWithString: (NSString*)aUrlString
{
  return AUTORELEASE([[NSURL alloc] initWithString: aUrlString]);
}

+ (id) URLWithString: (NSString*)aUrlString
       relativeToURL: (NSURL*)aBaseUrl
{
  return AUTORELEASE([[NSURL alloc] initWithString: aUrlString
				     relativeToURL: aBaseUrl]);
}

+ (id) URLByResolvingAliasFileAtURL: (NSURL*)url 
                            options: (NSURLBookmarkResolutionOptions)options 
                              error: (NSError**)error
{
  // TODO: unimplemented
  return nil;
}

- (id) initFileURLWithPath: (NSString*)aPath
{
  NSFileManager	*mgr = [NSFileManager defaultManager];
  BOOL		flag = NO;

  if (nil == aPath)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"[%@ %@] nil string parameter",
	NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  if ([aPath isAbsolutePath] == NO)
    {
      aPath = [[mgr currentDirectoryPath]
	stringByAppendingPathComponent: aPath];
    }
  if ([mgr fileExistsAtPath: aPath isDirectory: &flag] == YES)
    {
      if ([aPath isAbsolutePath] == NO)
	{
	  aPath = [aPath stringByStandardizingPath];
	}
      if (flag == YES && [aPath hasSuffix: @"/"] == NO)
	{
	  aPath = [aPath stringByAppendingString: @"/"];
	}
    }
  self = [self initWithScheme: NSURLFileScheme
			 host: @""
			 path: aPath];
  return self;
}

- (id) initFileURLWithPath: (NSString*)aPath isDirectory: (BOOL)isDir
{
  NSFileManager	*mgr = [NSFileManager defaultManager];
  BOOL		flag = NO;

  if (nil == aPath)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"[%@ %@] nil string parameter",
	NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  if ([aPath isAbsolutePath] == NO)
    {
      aPath = [[mgr currentDirectoryPath]
	stringByAppendingPathComponent: aPath];
    }
  if ([mgr fileExistsAtPath: aPath isDirectory: &flag] == YES)
    {
      if ([aPath isAbsolutePath] == NO)
	{
	  aPath = [aPath stringByStandardizingPath];
	}
      isDir = flag;
    }
  if (isDir == YES && [aPath hasSuffix: @"/"] == NO)
    {
      aPath = [aPath stringByAppendingString: @"/"];
    }
  self = [self initWithScheme: NSURLFileScheme
			 host: @""
			 path: aPath];
  return self;
}

- (id) initWithScheme: (NSString*)aScheme
		 host: (NSString*)aHost
		 path: (NSString*)aPath
{
  NSRange	r = NSMakeRange(NSNotFound, 0);
  NSString	*auth = nil;
  NSString	*aUrlString = [NSString alloc];

  if ([aScheme isEqualToString: @"file"])
    {
      aPath = [aPath stringByAddingPercentEncodingWithAllowedCharacters:
	fileCharSet];
    }
  else
    {
      aPath = [aPath
	stringByAddingPercentEscapesUsingEncoding: NSUTF8StringEncoding];
    }

  r = [aHost rangeOfString: @"@"];

  /* Allow for authentication (username:password) before actual host.
   */
  if (r.length > 0)
    {
      auth = [aHost substringToIndex: r.location];
      aHost = [aHost substringFromIndex: NSMaxRange(r)];
    }

  /* Add square brackets around ipv6 address if necessary
   */
  if ([[aHost componentsSeparatedByString: @":"] count] > 2
    && [aHost hasPrefix: @"["] == NO)
    {
      aHost = [NSString stringWithFormat: @"[%@]", aHost];
    }

  if (auth != nil)
    {
      aHost = [NSString stringWithFormat: @"%@@%@", auth, aHost];
    }

  if ([aPath length] > 0)
    {
      /*
       * For MacOS-X compatibility, assume a path component with
       * a leading slash is intended to have that slash separating
       * the host from the path as specified in the RFC1738
       */
      if ([aPath hasPrefix: @"/"] == YES)
        {
          aUrlString = [aUrlString initWithFormat: @"%@://%@%@",
            aScheme, aHost, aPath];
        }
      else
        {
          aUrlString = [aUrlString initWithFormat: @"%@://%@/%@",
            aScheme, aHost, aPath];
        }
    }
  else
    {
      aUrlString = [aUrlString initWithFormat: @"%@://%@/",
        aScheme, aHost];
    }
  self = [self initWithString: aUrlString relativeToURL: nil];
  RELEASE(aUrlString);
  return self;
}

- (id) initWithString: (NSString*)aUrlString
{
  self = [self initWithString: aUrlString relativeToURL: nil];
  return self;
}

- (id) initWithString: (NSString*)aUrlString
	relativeToURL: (NSURL*)aBaseUrl
{
  /* RFC 2396 'reserved' characters ...
   * as modified by RFC2732
   * static const char *reserved = ";/?:@&=+$,[]";
   */
  /* Same as reserved set but allow the hash character in a path too.
   */
  static const char *filepath = ";/?:@&=+$,[]#";

  if (nil == aUrlString)
    {
      RELEASE(self);
      return nil;       // OSX behavior is to give up.
    }
  if ([aUrlString isKindOfClass: [NSString class]] == NO)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"[%@ %@] bad string parameter",
	NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  if (aBaseUrl != nil
    && [aBaseUrl isKindOfClass: [NSURL class]] == NO)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"[%@ %@] bad base URL parameter",
	NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  ASSIGNCOPY(_urlString, aUrlString);
  ASSIGN(_baseURL, [aBaseUrl absoluteURL]);
  NS_DURING
    {
      parsedURL	*buf;
      parsedURL	*base = baseData;
      unsigned	size = [_urlString length];
      char	*end;
      char	*start;
      char	*ptr;
      BOOL	usesFragments = YES;
      BOOL	usesParameters = YES;
      BOOL	usesQueries = YES;
      BOOL	canBeGeneric = YES;

      size += sizeof(parsedURL) + urlAlign + 1;
      buf = _data = (parsedURL*)NSZoneMalloc(NSDefaultMallocZone(), size);
      memset(buf, '\0', size);
      start = end = ptr = (char*)&buf[1];
      NS_DURING
        {
          [_urlString getCString: start
                       maxLength: size
                        encoding: NSASCIIStringEncoding];
        }
      NS_HANDLER
        {
          /* OSX behavior when given non-ascii text is to return nil.
           */
          RELEASE(self);
          return nil;
        }
      NS_ENDHANDLER

      /*
       * Parse the scheme if possible.
       */
      ptr = start;
      if (isalpha(*ptr))
	{
	  ptr++;
	  while (isalnum(*ptr) || *ptr == '+' || *ptr == '-' || *ptr == '.')
	    {
	      ptr++;
	    }
	  if (*ptr == ':')
	    {
	      buf->scheme = start;		// Got scheme.
	      *ptr = '\0';			// Terminate it.
	      end = &ptr[1];
	      /*
	       * Standardise uppercase to lower.
	       */
	      while (--ptr > start)
		{
		  if (isupper(*ptr))
		    {
		      *ptr = tolower(*ptr);
		    }
		}
	    }
	}
      start = end;

      if (buf->scheme != 0 && base != 0
        && 0 != strcmp(buf->scheme, base->scheme))
        {
          /* The relative URL is of a different scheme to the base ...
           * so it's actually an absolute URL without a base.
           */
          DESTROY(_baseURL);
          base = 0;
        }

      if (buf->scheme == 0 && base != 0)
	{
	  buf->scheme = base->scheme;
	}

      /*
       * Set up scheme specific parsing options.
       */
      if (buf->scheme != 0)
        {
          if (strcmp(buf->scheme, "file") == 0)
	    {
	      buf->isFile = YES;
	    }
	  else if (strcmp(buf->scheme, "data") == 0)
            {
	      canBeGeneric = NO;
              DESTROY(_baseURL);
              base = 0;
            }
          else if (strcmp(buf->scheme, "mailto") == 0)
	    {
	      usesFragments = NO;
	      usesParameters = NO;
	      usesQueries = NO;
	    }
          else if (strcmp(buf->scheme, "http") == 0
            || strcmp(buf->scheme, "https") == 0)
	    {
	      buf->emptyPath = YES;
	    }
        }

      if (canBeGeneric == YES)
	{
	  /*
	   * Parse the 'authority'
	   * //user:password@host:port
	   */
	  if (start[0] == '/' && start[1] == '/')
	    {
	      buf->isGeneric = YES;
	      start = end = &end[2];

	      /*
	       * Set 'end' to point to the start of the path, or just past
	       * the 'authority' if there is no path.
	       */
	      end = strchr(start, '/');
	      if (end == 0)
		{
		  buf->hasNoPath = YES;
		  end = &start[strlen(start)];
		}
	      else
		{
		  *end++ = '\0';
		}

	      /*
	       * Parser username:password part
	       */
	      ptr = strchr(start, '@');
	      if (ptr != 0)
		{
		  buf->user = start;
		  *ptr++ = '\0';
		  start = ptr;
		  if (legal(buf->user, ";:&=+$,") == NO)
		    {
		      [NSException raise: NSInvalidArgumentException
                        format: @"[%@ %@](%@, %@) "
			@"illegal character in user/password part",
                        NSStringFromClass([self class]),
                        NSStringFromSelector(_cmd),
                        aUrlString, aBaseUrl];
		    }
		  ptr = strchr(buf->user, ':');
		  if (ptr != 0)
		    {
		      *ptr++ = '\0';
		      buf->password = ptr;
		    }
		}

	      /*
	       * Parse host:port part
	       */
	      buf->host = start;
	      if (*start == '[')
		{
	          ptr = strchr(buf->host, ']');
		  if (ptr == 0)
		    {
		      [NSException raise: NSInvalidArgumentException
			format: @"[%@ %@](%@, %@) "
			@"illegal ipv6 host address",
			NSStringFromClass([self class]),
			NSStringFromSelector(_cmd),
			aUrlString, aBaseUrl];
		    }
		  else
		    {
		      ptr = start + 1;
		      while (*ptr != ']')
			{
			  if (*ptr != ':' && *ptr != '.' && !isxdigit(*ptr))
			    {
			      [NSException raise: NSInvalidArgumentException
				format: @"[%@ %@](%@, %@) "
				@"illegal ipv6 host address",
				NSStringFromClass([self class]),
				NSStringFromSelector(_cmd),
				aUrlString, aBaseUrl];
			    }
			  ptr++;
			}
		    }
	          ptr = strchr(ptr, ':');
		}
	      else
		{
	          ptr = strchr(buf->host, ':');
		}
	      if (ptr != 0)
		{
		  const char	*str;

		  *ptr++ = '\0';
		  buf->port = ptr;
		  str = buf->port;
		  while (*str != 0)
		    {
		      if (*str == '%' && isxdigit(str[1]) && isxdigit(str[2]))
			{
			  unsigned char	c;

			  str++;
			  if (*str <= '9')
			    {
			      c = *str - '0';
			    }
			  else if (*str <= 'F')
			    {
			      c = *str - 'A' + 10;
			    }
			  else
			    {
			      c = *str - 'a' + 10;
			    }
			  c <<= 4;
			  str++;
			  if (*str <= '9')
			    {
			      c |= *str - '0';
			    }
			  else if (*str <= 'F')
			    {
			      c |= *str - 'A' + 10;
			    }
			  else
			    {
			      c |= *str - 'a' + 10;
			    }

			  if (isdigit(c))
			    {
			      str++;
			    }
			  else
			    {
			      [NSException raise: NSInvalidArgumentException
                                format: @"[%@ %@](%@, %@) "
				@"illegal port part",
                                NSStringFromClass([self class]),
                                NSStringFromSelector(_cmd),
                                aUrlString, aBaseUrl];
			    }
			}
		      else if (isdigit(*str))
			{
			  str++;
			}
		      else
			{
			  [NSException raise: NSInvalidArgumentException
                            format: @"[%@ %@](%@, %@) "
			    @"illegal character in port part",
                            NSStringFromClass([self class]),
                            NSStringFromSelector(_cmd),
                            aUrlString, aBaseUrl];
			}
		    }
		}
	      start = end;
	      /* Check for a legal host, unless it's an ipv6 address
	       * (which would have been checked earlier).
	       */
	      if (*buf->host != '[' && legal(buf->host, "-") == NO)
		{
		  [NSException raise: NSInvalidArgumentException
                    format: @"[%@ %@](%@, %@) "
		    @"illegal character in host part",
                    NSStringFromClass([self class]),
                    NSStringFromSelector(_cmd),
                    aUrlString, aBaseUrl];
		}

	      /*
	       * If we have an authority component,
	       * this must be an absolute URL
	       */
	      buf->pathIsAbsolute = YES;
	      base = 0;
	    }
	  else
	    {
	      if (base != 0)
		{
		  buf->isGeneric = base->isGeneric;
		}
	      if (*start == '/')
		{
		  buf->pathIsAbsolute = YES;
		  start++;
		}
	    }

	  if (usesFragments == YES)
	    {
	      /*
	       * Strip fragment string from end of url.
	       */
	      ptr = strchr(start, '#');
	      if (ptr != 0)
		{
		  *ptr++ = '\0';
		  if (*ptr != 0)
		    {
		      buf->fragment = ptr;
		    }
		}
	      if (buf->fragment == 0 && base != 0)
		{
		  buf->fragment = base->fragment;
		}
	      if (legal(buf->fragment, filepath) == NO)
		{
		  [NSException raise: NSInvalidArgumentException
		    format: @"[%@ %@](%@, %@) "
		    @"illegal character in fragment part",
		    NSStringFromClass([self class]),
		    NSStringFromSelector(_cmd),
		    aUrlString, aBaseUrl];
		}
	    }

	  if (usesQueries == YES)
	    {
	      /*
	       * Strip query string from end of url.
	       */
	      ptr = strchr(start, '?');
	      if (ptr != 0)
		{
		  *ptr++ = '\0';
		  if (*ptr != 0)
		    {
		      buf->query = ptr;
		    }
		}
	      if (buf->query == 0 && base != 0)
		{
		  buf->query = base->query;
		}
	      if (legal(buf->query, filepath) == NO)
		{
		  [NSException raise: NSInvalidArgumentException
		    format: @"[%@ %@](%@, %@) "
		    @"illegal character in query part",
		    NSStringFromClass([self class]),
		    NSStringFromSelector(_cmd),
		    aUrlString, aBaseUrl];
		}
	    }

	  if (usesParameters == YES)
	    {
	      /*
	       * Strip parameters string from end of url.
	       */
	      ptr = strchr(start, ';');
	      if (ptr != 0)
		{
		  *ptr++ = '\0';
		  if (*ptr != 0)
		    {
		      buf->parameters = ptr;
		    }
		}
	      if (buf->parameters == 0 && base != 0)
		{
		  buf->parameters = base->parameters;
		}
	      if (legal(buf->parameters, filepath) == NO)
		{
		  [NSException raise: NSInvalidArgumentException
		    format: @"[%@ %@](%@, %@) "
		    @"illegal character in parameters part",
		    NSStringFromClass([self class]),
		    NSStringFromSelector(_cmd),
		    aUrlString, aBaseUrl];
		}
	    }

	  if (buf->isFile == YES)
	    {
	      buf->user = 0;
	      buf->password = 0;
	      if (base != 0 && base->host != 0)
		{
		  buf->host = base->host;
		}
	      else if (buf->host != 0 && *buf->host == 0)
		{
		  buf->host = 0;
		}
	      buf->port = 0;
	      buf->isGeneric = YES;
	    }
	  else if (base != 0
	    && buf->user == 0 && buf->password == 0
	    && buf->host == 0 && buf->port == 0)
	    {
	      buf->user = base->user;
	      buf->password = base->password;
	      buf->host = base->host;
	      buf->port = base->port;
	    }
	}
      /*
       * Store the path.
       */
      buf->path = start;
      if (0 == base && '\0' == *buf->path && NO == buf->pathIsAbsolute)
	{
	  buf->hasNoPath = YES;
	}
      if (legal(buf->path, filepath) == NO)
	{
	  [NSException raise: NSInvalidArgumentException
            format: @"[%@ %@](%@, %@) "
	    @"illegal character in path part",
            NSStringFromClass([self class]),
            NSStringFromSelector(_cmd),
            aUrlString, aBaseUrl];
	}
    }
  NS_HANDLER
    {
      NSDebugLog(@"%@", localException);
      DESTROY(self);
    }
  NS_ENDHANDLER
  return self;
}

- (void) dealloc
{
  if (_clients != 0)
    {
      NSFreeMapTable(_clients);
      _clients = 0;
    }
  if (_data != 0)
    {
      DESTROY(myData->absolute);
      NSZoneFree([self zone], _data);
      _data = 0;
    }
  DESTROY(_urlString);
  DESTROY(_baseURL);
  [super dealloc];
}

- (id) copyWithZone: (NSZone*)zone
{
  if (NSShouldRetainWithZone(self, zone) == NO)
    {
      return [[[self class] allocWithZone: zone] initWithString: _urlString
						  relativeToURL: _baseURL];
    }
  else
    {
      return RETAIN(self);
    }
}

- (NSString*) description
{
  NSString	*dscr = _urlString;

  if (_baseURL != nil)
    {
      dscr = [dscr stringByAppendingFormat: @" -- %@", _baseURL];
    }
  return dscr;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: _baseURL forKey: @"NS.base"];
      [aCoder encodeObject: _urlString forKey: @"NS.relative"];
    }
  else
    {
      [aCoder encodeObject: _urlString];
      [aCoder encodeObject: _baseURL];
    }
}

- (NSUInteger) hash
{
  return [[self absoluteString] hash];
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  NSURL		*base;
  NSString	*rel;

  if ([aCoder allowsKeyedCoding])
    {
      base = [aCoder decodeObjectForKey: @"NS.base"];
      rel = [aCoder decodeObjectForKey: @"NS.relative"];
    }
  else
    {
      rel = [aCoder decodeObject];
      base = [aCoder decodeObject];
    }
  if (nil == rel)
    {
      rel = @"";
    }
  self = [self initWithString: rel relativeToURL: base];
  return self;
}

- (BOOL) isEqual: (id)other
{
  if (other == nil || [other isKindOfClass: [NSURL class]] == NO)
    {
      return NO;
    }
  return [[self absoluteString] isEqualToString: [other absoluteString]];
}

- (NSString*) absoluteString
{
  NSString	*absString = myData->absolute;

  if (absString == nil)
    {
      char	*url = buildURL(baseData, myData, NO);
      unsigned	len = strlen(url);

      absString = [[NSString alloc] initWithCStringNoCopy: url
						   length: len
					     freeWhenDone: YES];
      myData->absolute = absString;
    }
  return absString;
}

- (NSURL*) absoluteURL
{
  if (_baseURL == nil)
    {
      return self;
    }
  else
    {
      return [NSURL URLWithString: [self absoluteString]];
    }
}

- (NSURL*) baseURL
{
  return _baseURL;
}

- (BOOL) checkResourceIsReachableAndReturnError: (NSError **)error
{
  NSString *errorStr = nil;

  if ([self isFileURL])
    {
      NSFileManager *mgr = [NSFileManager defaultManager];
      NSString *path = [self path];
      
      if ([mgr fileExistsAtPath: path])
        {
          if (![mgr isReadableFileAtPath: path])
            {
              errorStr = @"File not readable";
            }
        }
      else
        {
          errorStr = @"File does not exist";
        }
    }
  else
    {
      errorStr = @"No file URL";
    }

  if ((errorStr != nil) && (error != NULL))
    {
      NSDictionary	*info;

      info = [NSDictionary dictionaryWithObjectsAndKeys:
	errorStr, NSLocalizedDescriptionKey, nil];
      *error = [NSError errorWithDomain: @"NSURLError"
                                   code: 0 
                               userInfo: info];
    }
  return nil == errorStr ? YES : NO;
}

- (NSString*) fragment
{
  NSString	*fragment = nil;

  if (myData->fragment != 0)
    {
      fragment = [NSString stringWithUTF8String: myData->fragment];
    }
  return fragment;
}

- (char*) _path: (char*)buf withEscapes: (BOOL)withEscapes
{
  char	*ptr = buf;
  char	*tmp = buf;
  int	l;

  if (myData->pathIsAbsolute == YES)
    {
      if (myData->hasNoPath == NO)
	{
	  *tmp++ = '/';
	}
      if (myData->path != 0)
	{
	  l = strlen(myData->path);
          memcpy(tmp, myData->path, l + 1);
	}
    }
  else if (nil == _baseURL)
    {
      if (myData->path != 0)
	{
	  l = strlen(myData->path);
          memcpy(tmp, myData->path, l + 1);
	}
    }
  else if (0 == myData->path || 0 == *myData->path)
    {
      if (baseData->hasNoPath == NO)
	{
	  *tmp++ = '/';
	}
      if (baseData->path != 0)
	{
	  l = strlen(baseData->path);
          memcpy(tmp, baseData->path, l + 1);
	}
    }
  else
    {
      char	*start = baseData->path;
      char	*end = (start == 0) ? 0 : strrchr(start, '/');

      if (end != 0)
	{
	  *tmp++ = '/';
	  strncpy(tmp, start, end - start);
	  tmp += end - start;
	}
      *tmp++ = '/';
      if (myData->path != 0)
	{
	  l = strlen(myData->path);
          memcpy(tmp, myData->path, l + 1);
	}
    }

  if (!withEscapes)
    {
      unescape(buf, buf);
    }

#if	defined(_WIN32)
  /* On windows a file URL path may be of the form C:\xxx (ie we should
   * not insert the leading slash).
   * Also the vertical bar symbol may have been used instead of the
   * colon, so we need to convert that.
   */
  if (myData->isFile == YES)
    {
      if (ptr[1] && isalpha(ptr[1]))
	{
	  if (ptr[2] == ':' || ptr[2] == '|')
	    {
	      if (ptr[3] == '\0' || ptr[3] == '/' || ptr[3] == '\\')
		{
		  ptr[2] = ':';
		  ptr++;
		}
	    }
	}
    }
#endif
  return ptr;
}

- (NSString*) host
{
  NSString	*host = nil;

  if (myData->host != 0)
    {
      char	buf[strlen(myData->host)+1];

      if (*myData->host == '[')
	{
	  char	*end = unescape(myData->host + 1, buf);

	  if (end[-1] == ']')
	    {
	      end[-1] = '\0';
	    }
	}
      else
	{
          unescape(myData->host, buf);
	}
      host = [NSString stringWithUTF8String: buf];
    }
  return host;
}

- (BOOL) isFileURL
{
  return myData->isFile;
}

- (NSString*) lastPathComponent
{
  return [[self path] lastPathComponent];
}

- (BOOL) isFileReferenceURL
{
  return NO;
}

- (NSURL *) fileReferenceURL
{
  if ([self isFileURL]) 
    {
      return self;
    }
  return nil;
}

- (NSURL *) filePathURL
{
  if ([self isFileURL]) 
    {
      return self;
    }
  return nil;
}

- (BOOL) getResourceValue: (id*)value 
                   forKey: (NSString *)key 
                    error: (NSError**)error
{
  // TODO: unimplemented
  return NO;
}

- (void) loadResourceDataNotifyingClient: (id)client
			      usingCache: (BOOL)shouldUseCache
{
  NSURLHandle	*handle = [self URLHandleUsingCache: YES];
  NSData	*d;

  if (shouldUseCache == YES && (d = [handle availableResourceData]) != nil)
    {
      /*
       * We already have cached data we should use.
       */
      if ([client respondsToSelector:
	@selector(URL:resourceDataDidBecomeAvailable:)])
	{
	  [client URL: self resourceDataDidBecomeAvailable: d];
	}
      if ([client respondsToSelector: @selector(URLResourceDidFinishLoading:)])
	{
	  [client URLResourceDidFinishLoading: self];
	}
    }
  else
    {
      if (client != nil)
	{
	  [clientsLock lock];
	  if (_clients == 0)
	    {
	      _clients = NSCreateMapTable (NSObjectMapKeyCallBacks,
		NSNonRetainedObjectMapValueCallBacks, 0);
	    }
	  NSMapInsert((NSMapTable*)_clients, (void*)handle, (void*)client);
	  [clientsLock unlock];
	  [handle addClient: self];
	}

      /*
       * Kick off the load process.
       */
      [handle loadInBackground];
    }
}

- (NSString*) parameterString
{
  NSString	*parameters = nil;

  if (myData->parameters != 0)
    {
      parameters = [NSString stringWithUTF8String: myData->parameters];
    }
  return parameters;
}

- (NSString*) password
{
  NSString	*password = nil;

  if (myData->password != 0)
    {
      char	buf[strlen(myData->password)+1];

      unescape(myData->password, buf);
      password = [NSString stringWithUTF8String: buf];
    }
  return password;
}

- (NSString*) _pathWithEscapes: (BOOL)withEscapes
{
  NSString	*path = nil;

  if (YES == myData->isGeneric || 0 == myData->scheme)
    {
      unsigned int	len = 3;

      if (_baseURL != nil)
        {
          if (baseData->path && *baseData->path)
            {
              len += strlen(baseData->path);
            }
          else if (baseData->hasNoPath == NO)
            {
              len++;
            }
        }
      if (myData->path && *myData->path)
        {
          len += strlen(myData->path);
        }
      else if (myData->hasNoPath == NO)
        {
          len++;
        }
      if (len > 3)
        {
          char		buf[len];
          char		*ptr;
          char		*tmp;

          ptr = [self _path: buf withEscapes: withEscapes];

          /* Remove any trailing '/' from the path for MacOS-X compatibility.
           */
          tmp = ptr + strlen(ptr) - 1;
          if (tmp > ptr && *tmp == '/')
            {
              *tmp = '\0';
            }

          path = [NSString stringWithUTF8String: ptr];
        }
      else if (YES == myData->emptyPath)
        {
          /* OSX seems to use an empty string for some schemes,
           * though it normally uses nil.
           */
          path = @"";
        }
    }
  return path;
}

- (NSString*) path
{
  return [self _pathWithEscapes: NO];
}

- (NSArray*) pathComponents 
{
  return [[self path] pathComponents];
}

- (NSString*) pathExtension 
{
  return [[self path] pathExtension];
}

- (NSNumber*) port
{
  NSNumber	*port = nil;

  if (myData->port != 0)
    {
      char	buf[strlen(myData->port)+1];

      unescape(myData->port, buf);
      port = [NSNumber numberWithUnsignedShort: atol(buf)];
    }
  return port;
}

- (id) propertyForKey: (NSString*)propertyKey
{
  NSURLHandle	*handle = [self URLHandleUsingCache: YES];

  return [handle propertyForKey: propertyKey];
}

- (NSString*) query
{
  NSString	*query = nil;

  if (myData->query != 0)
    {
      query = [NSString stringWithUTF8String: myData->query];
    }
  return query;
}

- (NSString*) relativePath
{
  if (nil == _baseURL)
    {
      return [self path];
    }
  else
    {
      NSString	*path = nil;

      if (myData->path != 0)
	{
          char		buf[strlen(myData->path) + 1];

          strcpy(buf, myData->path);
          unescape(buf, buf);
	  path = [NSString stringWithUTF8String: buf];
	}
      return path;
    }
}

- (NSString*) relativeString
{
  return _urlString;
}

/* Encode bycopy unless explicitly requested otherwise.
 */
- (id) replacementObjectForPortCoder: (NSPortCoder*)aCoder
{
  if ([aCoder isByref] == NO)
    return self;
  return [super replacementObjectForPortCoder: aCoder];
}

- (NSData*) resourceDataUsingCache: (BOOL)shouldUseCache
{
  NSURLHandle	*handle = [self URLHandleUsingCache: YES];
  NSData	*data = nil;

  if ([handle status] == NSURLHandleLoadSucceeded)
    {
      data = [handle availableResourceData];
    }
  if (shouldUseCache == NO || [handle status] != NSURLHandleLoadSucceeded)
    {
      data = [handle loadInForeground];
    }
  if (nil == data)
    {
      data = [handle availableResourceData];
    }
  return data;
}

- (NSString*) resourceSpecifier
{
  if (YES == myData->isGeneric)
    {
      NSRange	range = [_urlString rangeOfString: @"://"];

      if (range.length > 0)
        {
          NSString *specifier;

          /* MacOSX compatibility - in the case where there is no
           * host in the URL, just return the path (without the "//").
           * For all other cases we return the whole specifier.
           */
          if (nil == [self host])
            {
              specifier = [_urlString substringFromIndex: NSMaxRange(range)];
            }
          else
            {
              specifier = [_urlString substringFromIndex: range.location+1];
            }
          return specifier;
        }
      else
        {
          /*
           * Cope with URLs missing net_path info -  <scheme>:/<path>...
           */
          range = [_urlString rangeOfString: @":"];
          if (range.length > 0)
            {
              return [_urlString substringFromIndex: range.location + 1];
            }
          else
            {
              return _urlString;
            }
        }
    }
  else
    {
      return [NSString stringWithUTF8String: myData->path];
    }
}

- (NSString*) scheme
{
  NSString	*scheme = nil;

  if (myData->scheme != 0)
    {
      scheme = [NSString stringWithUTF8String: myData->scheme];
    }
  return scheme;
}

- (BOOL) setProperty: (id)property
	      forKey: (NSString*)propertyKey
{
  NSURLHandle	*handle = [self URLHandleUsingCache: YES];

  return [handle writeProperty: property forKey: propertyKey];
}

- (BOOL) setResourceData: (NSData*)data
{
  NSURLHandle	*handle = [self URLHandleUsingCache: YES];

  if (handle == nil)
    {
      return NO;
    }
  if ([handle writeData: data] == NO)
    {
      return NO;
    }
  if ([handle loadInForeground] == nil)
    {
      return NO;
    }
  return YES;
}

- (NSURL*) standardizedURL
{
  char		*url = buildURL(baseData, myData, YES);
  unsigned	len = strlen(url);
  NSString	*str;
  NSURL		*tmp;

  str = [[NSString alloc] initWithCStringNoCopy: url
					 length: len
				   freeWhenDone: YES];
  tmp = [NSURL URLWithString: str];
  RELEASE(str);
  return tmp;
}

- (NSURLHandle*) URLHandleUsingCache: (BOOL)shouldUseCache
{
  NSURLHandle	*handle = nil;

  if (shouldUseCache)
    {
      handle = [NSURLHandle cachedHandleForURL: self];
    }
  if (handle == nil)
    {
      Class	c = [NSURLHandle URLHandleClassForURL: self];

      if (c != 0)
	{
	  handle = [[c alloc] initWithURL: self cached: shouldUseCache];
	  IF_NO_GC([handle autorelease];)
	}
    }
  return handle;
}

- (NSString*) user
{
  NSString	*user = nil;

  if (myData->user != 0)
    {
      char	buf[strlen(myData->user)+1];

      unescape(myData->user, buf);
      user = [NSString stringWithUTF8String: buf];
    }
  return user;
}

- (NSURL*) URLByAppendingPathComponent: (NSString*)pathComponent 
{
  return [self _URLBySettingPath:
    [[self path] stringByAppendingPathComponent: pathComponent]];
}

- (NSURL*) URLByAppendingPathExtension: (NSString*)pathExtension
{
  return [self _URLBySettingPath:
    [[self path] stringByAppendingPathExtension: pathExtension]];
}

- (NSURL*) URLByDeletingLastPathComponent 
{
  return [self _URLBySettingPath:
    [[self path] stringByDeletingLastPathComponent]];
}

- (NSURL*) URLByDeletingPathExtension 
{
  return [self _URLBySettingPath:
    [[self path] stringByDeletingPathExtension]];
}

- (NSURL*) URLByResolvingSymlinksInPath 
{
  if ([self isFileURL]) 
    {
      return [NSURL fileURLWithPath:
	[[self path] stringByResolvingSymlinksInPath]];
    }
  return self;
}

- (NSURL*) URLByStandardizingPath 
{
  if ([self isFileURL]) 
    {
      return [NSURL fileURLWithPath: [[self path] stringByStandardizingPath]];
    }
  return self;
}

- (NSURL *) URLByAppendingPathComponent:(NSString *)pathComponent
                            isDirectory:(BOOL)isDirectory
{
  NSString *path = [[self path] stringByAppendingPathComponent: pathComponent];
  if (isDirectory)
    {
      path = [path stringByAppendingString: @"/"];
    }
  return [self _URLBySettingPath: path];
}

- (void) URLHandle: (NSURLHandle*)sender
  resourceDataDidBecomeAvailable: (NSData*)newData
{
  id	c = clientForHandle(_clients, sender);

  if ([c respondsToSelector: @selector(URL:resourceDataDidBecomeAvailable:)])
    {
      [c URL: self resourceDataDidBecomeAvailable: newData];
    }
}

- (void) URLHandle: (NSURLHandle*)sender
  resourceDidFailLoadingWithReason: (NSString*)reason
{
  id	c = clientForHandle(_clients, sender);

  if (c != nil)
    {
      if ([c respondsToSelector:
	@selector(URL:resourceDidFailLoadingWithReason:)])
	{
	  [c URL: self resourceDidFailLoadingWithReason: reason];
	}
      [clientsLock lock];
      NSMapRemove((NSMapTable*)_clients, (void*)sender);
      [clientsLock unlock];
    }
  [sender removeClient: self];
}

- (void) URLHandleResourceDidBeginLoading: (NSURLHandle*)sender
{
}

- (void) URLHandleResourceDidCancelLoading: (NSURLHandle*)sender
{
  id	c = clientForHandle(_clients, sender);

  if (c != nil)
    {
      if ([c respondsToSelector: @selector(URLResourceDidCancelLoading:)])
	{
	  [c URLResourceDidCancelLoading: self];
	}
      [clientsLock lock];
      NSMapRemove((NSMapTable*)_clients, (void*)sender);
      [clientsLock unlock];
    }
  [sender removeClient: self];
}

- (void) URLHandleResourceDidFinishLoading: (NSURLHandle*)sender
{
  id	c = clientForHandle(_clients, sender);

  IF_NO_GC([self retain];)
  [sender removeClient: self];
  if (c != nil)
    {
      if ([c respondsToSelector: @selector(URLResourceDidFinishLoading:)])
	{
	  [c URLResourceDidFinishLoading: self];
	}
      [clientsLock lock];
      NSMapRemove((NSMapTable*)_clients, (void*)sender);
      [clientsLock unlock];
    }
  RELEASE(self);
}

@end



/**
 * An informal protocol to which clients may conform if they wish to be
 * notified of the progress in loading a URL for them.  NSURL conforms to
 * this protocol but all methods are implemented as no-ops.  See also
 * the [(NSURLHandleClient)] protocol.
 */
@implementation NSObject (NSURLClient)

- (void) URL: (NSURL*)sender
  resourceDataDidBecomeAvailable: (NSData*)newBytes
{
}

- (void) URL: (NSURL*)sender
  resourceDidFailLoadingWithReason: (NSString*)reason
{
}

- (void) URLResourceDidCancelLoading: (NSURL*)sender
{
}

- (void) URLResourceDidFinishLoading: (NSURL*)sender
{
}

@end

@implementation NSURL (GNUstepBase)
- (NSString*) fullPath
{
  NSString	*path = nil;

  if (YES == myData->isGeneric || 0 == myData->scheme)
    {
      unsigned int	len = 3;

      if (_baseURL != nil)
        {
          if (baseData->path && *baseData->path)
            {
              len += strlen(baseData->path);
            }
          else if (baseData->hasNoPath == NO)
            {
              len++;
            }
        }
      if (myData->path && *myData->path)
        {
          len += strlen(myData->path);
        }
      else if (myData->hasNoPath == NO)
        {
          len++;
        }
      if (len > 3)
        {
          char		buf[len];
          char		*ptr;

          ptr = [self _path: buf withEscapes: NO];
          path = [NSString stringWithUTF8String: ptr];
        }
    }
  return path;
}

- (NSString*) pathWithEscapes
{
  return [self _pathWithEscapes: YES];
}
@end


#define	GSInternal	NSURLQueryItemInternal
#include	"GSInternal.h"
GS_PRIVATE_INTERNAL(NSURLQueryItem)


@implementation NSURLQueryItem

// Creating query items.
+ (instancetype)queryItemWithName:(NSString *)name 
                            value:(NSString *)value
{
  NSURLQueryItem *newQueryItem = [[NSURLQueryItem alloc] initWithName: name
                                                                value: value];
  return AUTORELEASE(newQueryItem);
}

- (instancetype) init
{
  self = [self initWithName:nil value:nil];
  if(self != nil)
    {
    
    }
  return self;
}

- (instancetype)initWithName:(NSString *)name 
                       value:(NSString *)value
{
  self = [super init];
  if(self != nil)
  {
    GS_CREATE_INTERNAL(NSURLQueryItem);
    if(name)
      {
        ASSIGNCOPY(internal->_name, name);
      }
    else
      {
        ASSIGN(internal->_name, @""); //OSX behaviour is to set an empty string for nil name property
      }
    ASSIGNCOPY(internal->_value, value);
  }
  return self;
}

- (void) dealloc
{
  RELEASE(internal->_name);
  RELEASE(internal->_value);
  GS_DESTROY_INTERNAL(NSURLQueryItem);
  [super dealloc];
}

// Reading a name and value from a query
- (NSString *) name
{
  return internal->_name;
}

- (NSString *) value
{
  return internal->_value;
}

- (id) initWithCoder: (NSCoder *)acoder
{
  if ((self = [super init]) != nil)
    {
      if ([acoder allowsKeyedCoding])
        {
          internal->_name = [acoder decodeObjectForKey: @"NS.name"];
          internal->_value = [acoder decodeObjectForKey: @"NS.value"];
        }
      else
        {
          internal->_name = [acoder decodeObject];
          internal->_value = [acoder decodeObject];
        }
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)acoder
{
  if ([acoder allowsKeyedCoding])
    {
      [acoder encodeObject: internal->_name forKey: @"NS.name"];
      [acoder encodeObject: internal->_value forKey: @"NS.value"];
    }
  else
    {
      [acoder encodeObject: internal->_name];
      [acoder encodeObject: internal->_value];
    }
}

- (id) copyWithZone: (NSZone *)zone
{
    return [[[self class] allocWithZone: zone] initWithName: internal->_name
                                                      value: internal->_value];
}

@end


#undef	GSInternal
#define	GSInternal NSURLComponentsInternal
#include "GSInternal.h"
GS_PRIVATE_INTERNAL(NSURLComponents)


@implementation NSURLComponents 

static NSCharacterSet	*queryItemCharSet = nil;

+ (void) initialize
{
  if (nil == queryItemCharSet)
    {
      ENTER_POOL
      NSMutableCharacterSet	*m;

      m = [[NSCharacterSet URLQueryAllowedCharacterSet] mutableCopy];

      /* Rationale: if a query item contained an ampersand we would not be
       * able to tell where one name/value pair ends and the next starts,
       * so we cannot permit that character in an item.  Similarly, if a
       * query item contained an equals sign we would not be able to tell
       * where the name ends and the value starts, so we cannot permit that
       * character either.
       */
      [m removeCharactersInString: @"&="];
      queryItemCharSet = [m copy];
      LEAVE_POOL
    }
}

// Creating URL components...
+ (instancetype) componentsWithString: (NSString *)urlString
{
  return  AUTORELEASE([[NSURLComponents alloc] initWithString: urlString]);
}

+ (instancetype) componentsWithURL: (NSURL *)url 
           resolvingAgainstBaseURL: (BOOL)resolve
{
  return  AUTORELEASE([[NSURLComponents alloc] initWithURL: url
                      resolvingAgainstBaseURL: resolve]);
}

- (instancetype) init
{
  self = [super init];
  if (self != nil)
    {
      GS_CREATE_INTERNAL(NSURLComponents);
      
      internal->_rangeOfFragment = NSMakeRange(NSNotFound, 0);
      internal->_rangeOfHost     = NSMakeRange(NSNotFound, 0);
      internal->_rangeOfPassword = NSMakeRange(NSNotFound, 0);
      internal->_rangeOfPath     = NSMakeRange(NSNotFound, 0);
      internal->_rangeOfPort     = NSMakeRange(NSNotFound, 0);
      internal->_rangeOfQuery    = NSMakeRange(NSNotFound, 0);
      internal->_rangeOfScheme   = NSMakeRange(NSNotFound, 0);
      internal->_rangeOfUser     = NSMakeRange(NSNotFound, 0);
    }
  return self;
}

- (instancetype) initWithString: (NSString *)URLString
{
  //OSX behavior is to return nil for a string which cannot be used to initialize valid NSURL object
  NSURL* url = [NSURL URLWithString:URLString];
  if(url)
    {
      return [self initWithURL:url resolvingAgainstBaseURL:NO];
    }
  else
    {
      return nil;
    }
}

- (instancetype) initWithURL: (NSURL *)url 
     resolvingAgainstBaseURL: (BOOL)resolve
{
  self = [self init];
  if (self != nil)
    {
      NSURL *tempURL = url;

      if (resolve)
        {
          tempURL = [url absoluteURL];
        }
      [self setURL: tempURL];
    }
  return self;
}

- (void) dealloc
{
  RELEASE(internal->_string);
  RELEASE(internal->_fragment);
  RELEASE(internal->_host);
  RELEASE(internal->_password);
  RELEASE(internal->_path);
  RELEASE(internal->_port);
  RELEASE(internal->_queryItems);
  RELEASE(internal->_scheme);
  RELEASE(internal->_user);
  GS_DESTROY_INTERNAL(NSURLComponents);
  [super dealloc];
}

- (id) copyWithZone: (NSZone *)zone
{
  return [[NSURLComponents allocWithZone: zone] initWithURL: [self URL]
                                    resolvingAgainstBaseURL: NO];
}

// Regenerate URL when components are changed...
- (void) _regenerateURL
{
  NSMutableString	*urlString;
  NSString		*component;
  NSUInteger 	 	location;
  NSUInteger 		len;
  
  if (internal->_dirty == NO)
    {
      return;
    }

  urlString = [[NSMutableString alloc] initWithCapacity: 1000];
  location = 0;
  // Build up the URL from components...
  if (internal->_scheme != nil)
    {
      component = [self scheme];
      [urlString appendString: component];
      len = [component length];
      internal->_rangeOfScheme = NSMakeRange(location, len);
      [urlString appendString: @"://"];
      location += len + 3;
    }
  else
    {
      internal->_rangeOfScheme = NSMakeRange(NSNotFound, 0);
    }

  if (internal->_user != nil) 
    {
      if (internal->_password != nil)
        {
          component = [self percentEncodedUser];
	  len = [component length];
          [urlString appendString: component];
          internal->_rangeOfUser = NSMakeRange(location, len);
          [urlString appendString: @":"];
          location += len + 1;

          component = [self percentEncodedPassword];
	  len = [component length];
          [urlString appendString: component];
          internal->_rangeOfUser = NSMakeRange(location, len);
          [urlString appendString: @"@"];
          location += len + 1;
        }
      else
        {
          component = [self percentEncodedUser];
	  len = [component length];
          [urlString appendString: component];
          internal->_rangeOfUser = NSMakeRange(location, len);
          [urlString appendString: @"@"];
          location += len + 1;
        }
    }

  if (internal->_host != nil)
    {
      component = [self percentEncodedHost];
      len = [component length];
      [urlString appendString: component];
      internal->_rangeOfHost = NSMakeRange(location, len);
      location += len;
    }

  if (internal->_port != nil)
    {
      component = [[self port] stringValue];
      len = [component length];
      [urlString appendString: @":"];
      location += 1;
      [urlString appendString: component];
      internal->_rangeOfPort = NSMakeRange(location, len);
      location += len;
    }

  /* FIXME ... if the path is empty we still need a '/' do we not?
   */
  if (internal->_path != nil)
    {
      component = [self percentEncodedPath];
      len = [component length];
      [urlString appendString: component];
      internal->_rangeOfPath = NSMakeRange(location, len);
      location += len;
    }

  if ([internal->_queryItems count] > 0)
    {
      component = [self percentEncodedQuery];
      len = [component length];
      [urlString appendString: @"?"];
      location += 1;
      [urlString appendString: component];
      internal->_rangeOfQuery = NSMakeRange(location, len);
      location += len;
    }

  if (internal->_fragment != nil)
    {
      component = [self percentEncodedFragment];
      len = [component length];
      [urlString appendString: @"#"];
      location += 1;
      [urlString appendString: component];
      internal->_rangeOfFragment = NSMakeRange(location, len);
      location += len;
    }
    
  ASSIGNCOPY(internal->_string, urlString);
  RELEASE(urlString);
  internal->_dirty = NO;
}

// Getting the URL
- (NSString *) string
{
  [self _regenerateURL];
  return internal->_string;
}

- (void) setString: (NSString *)urlString
{
  NSURL *url = [NSURL URLWithString: urlString];
  [self setURL: url];
}

- (NSURL *) URL
{
  return AUTORELEASE([[NSURL alloc] initWithScheme: [self scheme]
                                              user: [self user]
                                          password: [self password]
                                              host: [self host]
                                              port: [self port]
                                          fullPath: [self path]
                                   parameterString: nil
                                             query: [self percentEncodedQuery]
                                          fragment: [self fragment]]);
}

- (void) setURL: (NSURL *)url
{
  // Set all the components...
  [self setScheme: [url scheme]];
  [self setHost: [url host]];
  [self setPort: [url port]];
  [self setUser: [url user]];
  [self setPassword: [url password]];
  [self setPath: [url path]];
  [self setPercentEncodedQuery:[url query]];
  [self setFragment: [url fragment]];
}

- (NSURL *) URLRelativeToURL: (NSURL *)baseURL
{
  return nil;
}

// Accessing Components in Native Format
- (NSString *) fragment
{
  return internal->_fragment;
}

- (void) setFragment: (NSString *)fragment
{
  ASSIGNCOPY(internal->_fragment, fragment);
  internal->_dirty = YES;
}

- (NSString *) host
{
  return internal->_host;
}

- (void) setHost: (NSString *)host
{
  ASSIGNCOPY(internal->_host, host);
  internal->_dirty = YES;
}

- (NSString *) password
{
  return internal->_password;
}

- (void) setPassword: (NSString *)password
{
  ASSIGNCOPY(internal->_password, password);
  internal->_dirty = YES;
}

- (NSString *) path
{
  return internal->_path;
}

- (void) setPath: (NSString *)path
{
  ASSIGNCOPY(internal->_path, path);
  internal->_dirty = YES;
}

- (NSNumber *) port
{
  return internal->_port;
}

- (void) setPort: (NSNumber *)port
{
  ASSIGNCOPY(internal->_port, port);
  internal->_dirty = YES;
}

- (NSString *) query
{
  NSString	*result = nil;

  if (internal->_queryItems != nil)
    {
      NSMutableString	*query = nil;
      NSURLQueryItem	*item = nil;
      NSEnumerator	*en;

      en = [internal->_queryItems objectEnumerator];
      while ((item = (NSURLQueryItem *)[en nextObject]) != nil)
	{
	  NSString	*name = [item name];
	  NSString	*value = [item value];

	  if (nil == query)
	    {
	      query = [[NSMutableString alloc] initWithCapacity: 1000];
	    }
	  else
	    {
	      [query appendString: @"&"];
	    }
	  [query appendString: name];
	  if (value != nil)
	    {
	      [query appendString: @"="];
	      [query appendString: value];
	    }
	}
      if (nil == query)
	{
	  result = @"";
	}
      else
	{
	  result = AUTORELEASE([query copy]);
	  RELEASE(query);
	}
    }
  return result;
}

- (void) _setQuery: (NSString *)query fromPercentEncodedString: (BOOL)encoded
{
  /* Parse according to https://developer.apple.com/documentation/foundation/nsurlcomponents/1407752-queryitems?language=objc
   */
  if (nil == query)
    {
      [self setQueryItems: nil];
    }
  else if ([query length] == 0)
    {
      [self setQueryItems: [NSArray array]];
    }
  else
    {
      NSMutableArray	*result = [NSMutableArray arrayWithCapacity: 5];
      NSArray 		*items = [query componentsSeparatedByString: @"&"];
      NSEnumerator	*en = [items objectEnumerator];
      id		item = nil;

      while ((item = [en nextObject]) != nil)
        {
          NSURLQueryItem	*qitem;
	  NSString		*name;
	  NSString		*value;

	  if ([item length] == 0)
	    {
	      name = @"";
	      value = nil;
	    }
	  else
	    {
	      NSRange	r = [item rangeOfString: @"="];

	      if (0 == r.length)
		{
		  /* No '=' found in query item.  */
		  name = item;
		  value = nil;
		}
	      else
		{
		  name = [item substringToIndex: r.location];
		  value = [item substringFromIndex: NSMaxRange(r)];
		}
	    }
	  if (encoded)
	    {
	      name = [name stringByRemovingPercentEncoding];
	      value = [value stringByRemovingPercentEncoding];
	    }
          qitem = [NSURLQueryItem queryItemWithName: name value: value];
          [result addObject: qitem];
        }
      [self setQueryItems: result];
    }
}

- (void) setQuery: (NSString *)query
{
  [self _setQuery: query fromPercentEncodedString: NO];
}

- (NSArray *) queryItems
{
  return AUTORELEASE(RETAIN(internal->_queryItems));
}

- (void) setQueryItems: (NSArray *)queryItems
{ 
  ASSIGNCOPY(internal->_queryItems, queryItems);
  internal->_dirty = YES;
}

- (NSString *) scheme
{
  return internal->_scheme;
}

- (void) setScheme: (NSString *)scheme
{
  ASSIGNCOPY(internal->_scheme, scheme);
  internal->_dirty = YES;
}

- (NSString *) user
{
  return internal->_user;
}

- (void) setUser: (NSString *)user
{
  ASSIGNCOPY(internal->_user, user);
  internal->_dirty = YES;
}

// Accessing Components in PercentEncoded Format
- (NSString *) percentEncodedFragment
{
  return [internal->_fragment
    stringByAddingPercentEncodingWithAllowedCharacters:
    [NSCharacterSet URLFragmentAllowedCharacterSet]];
}

- (void) setPercentEncodedFragment: (NSString *)fragment
{
  [self setFragment: [fragment stringByRemovingPercentEncoding]];
}

- (NSString *) percentEncodedHost
{
  return [internal->_host
    stringByAddingPercentEncodingWithAllowedCharacters:
    [NSCharacterSet URLHostAllowedCharacterSet]];
}

- (void) setPercentEncodedHost: (NSString *)host
{
  [self setHost: [host stringByRemovingPercentEncoding]];
}

- (NSString *) percentEncodedPassword
{
  return [internal->_password
    stringByAddingPercentEncodingWithAllowedCharacters:
    [NSCharacterSet URLPasswordAllowedCharacterSet]];
}

- (void) setPercentEncodedPassword: (NSString *)password
{
  [self setPassword: [password stringByRemovingPercentEncoding]];
}

- (NSString *) percentEncodedPath
{
  return [internal->_path
    stringByAddingPercentEncodingWithAllowedCharacters:
    [NSCharacterSet URLPathAllowedCharacterSet]];
}

- (void) setPercentEncodedPath: (NSString *)path
{
  [self setPath: [path stringByRemovingPercentEncoding]];
}

- (NSString *) percentEncodedQuery
{
  NSString	*result = nil;

  if (internal->_queryItems != nil)
    {
      NSMutableString	*query = nil;
      NSURLQueryItem	*item = nil;
      NSEnumerator	*en;

      en = [[self percentEncodedQueryItems] objectEnumerator];
      while ((item = (NSURLQueryItem *)[en nextObject]) != nil)
	{
	  NSString	*name = [item name];
	  NSString	*value = [item value];

	  if (nil == query)
	    {
	      query = [[NSMutableString alloc] initWithCapacity: 1000];
	    }
	  else
	    {
	      [query appendString: @"&"];
	    }
	  [query appendString: name];
	  if (value != nil)
	    {
	      [query appendString: @"="];
	      [query appendString: value];
	    }
	}
      if (nil == query)
	{
	  result = @"";
	}
      else
	{
	  result = AUTORELEASE([query copy]);
	  RELEASE(query);
	}
    }
  return result;
}

- (void) setPercentEncodedQuery: (NSString *)query
{
  [self _setQuery: query fromPercentEncodedString: YES];
}

- (NSArray *) percentEncodedQueryItems
{
  NSArray	*result = nil;

  if (internal->_queryItems != nil)
    {
      NSMutableArray	*items;
      NSEnumerator 	*en = [internal->_queryItems objectEnumerator];
      NSURLQueryItem	*i = nil;

      items = [[NSMutableArray alloc]
	initWithCapacity: [internal->_queryItems count]];
      while ((i = [en nextObject]) != nil)
	{
	  NSURLQueryItem	*ni;
	  NSString		*name = [i name];
	  NSString		*value = [i value];

	  name = [name stringByAddingPercentEncodingWithAllowedCharacters:
	    queryItemCharSet];
	  value = [value stringByAddingPercentEncodingWithAllowedCharacters:
	    queryItemCharSet];
	  ni = [NSURLQueryItem queryItemWithName: name
					   value: value];
	  [items addObject: ni];
	}
      result = AUTORELEASE([items copy]);
      RELEASE(items);
    }
  return result;
}

- (void) setPercentEncodedQueryItems: (NSArray *)queryItems
{
  NSMutableArray	*items = nil;

  if (queryItems != nil)
    {
      NSEnumerator	*en = [queryItems objectEnumerator];
      NSURLQueryItem 	*i = nil;

      items = [NSMutableArray arrayWithCapacity: [queryItems count]];
      while ((i = [en nextObject]) != nil)
	{
	  NSString		*name;
	  NSString		*value;
	  NSURLQueryItem	*ni;

	  name = [[i name] stringByRemovingPercentEncoding];
	  value = [[i value] stringByRemovingPercentEncoding];
	  ni = [NSURLQueryItem queryItemWithName: name value: value];
	  [items addObject: ni];
	}
    }

  [self setQueryItems: items];
}

- (NSString *) percentEncodedUser
{
  return [internal->_user stringByAddingPercentEncodingWithAllowedCharacters:
    [NSCharacterSet URLUserAllowedCharacterSet]];
}

- (void) setPercentEncodedUser: (NSString *)user
{
  [self setUser: [user stringByRemovingPercentEncoding]];
}

// Locating components of the URL string representation
- (NSRange) rangeOfFragment
{
  [self _regenerateURL];
  return internal->_rangeOfFragment;
}

- (NSRange) rangeOfHost
{
  [self _regenerateURL];
  return internal->_rangeOfHost;
}

- (NSRange) rangeOfPassword
{
  [self _regenerateURL];
  return internal->_rangeOfPassword;
}

- (NSRange) rangeOfPath
{
  [self _regenerateURL];
  return internal->_rangeOfPath;
}

- (NSRange) rangeOfPort
{
  [self _regenerateURL];
  return internal->_rangeOfPort;
}

- (NSRange) rangeOfQuery
{
  [self _regenerateURL];
  return internal->_rangeOfQuery;
}

- (NSRange) rangeOfScheme
{
  [self _regenerateURL];
  return internal->_rangeOfScheme;
}

- (NSRange) rangeOfUser
{
  [self _regenerateURL];
  return internal->_rangeOfUser;
}
  
@end
