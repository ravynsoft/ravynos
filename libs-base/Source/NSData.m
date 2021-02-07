/**
   Copyright (C) 1995-2015 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Date: March 1995
   Rewritten by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: September 1997

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

   <title>NSData class reference</title>
   $Date$ $Revision$
   */

/* NOTES	-	Richard Frith-Macdonald 1997
 *
 *	Rewritten to use the class cluster architecture as in OPENSTEP.
 *
 *	NB. In our implementaion we require an extra primitive for the
 *	    NSMutableData subclasses.  This new primitive method is the
 *	    [-setCapacity:] method, and it differs from [-setLength:]
 *	    as follows -
 *
 *		[-setLength:]
 *			clears bytes when the allocated buffer grows
 *			never shrinks the allocated buffer capacity
 *		[-setCapacity:]
 *			doesn't clear newly allocated bytes
 *			sets the size of the allocated buffer.
 *
 *	The actual class hierarchy is as follows -
 *
 *	NSData					Abstract base class.
 *	    NSDataStatic			Concrete class static buffers.
 *	        NSDataEmpty			Concrete class static buffers.
 *		NSDataMalloc			Concrete class.
 *		    NSDataMappedFile		Memory mapped files.
 *		    NSDataShared		Extension for shared memory.
 *		    NSDataFinalized		For GC of non-GC data.
 *          NSDataWithDeallocatorBlock Adds custom deallocation behaviour
 *	    NSMutableData			Abstract base class.
 *		NSMutableDataMalloc		Concrete class.
 *		    NSMutableDataShared		Extension for shared memory.
 *		    NSDataMutableFinalized	For GC of non-GC data.
 *          NSMutableDataWithDeallocatorBlock Adds custom deallocation behaviour
 *
 *	NSMutableDataMalloc MUST share it's initial instance variable layout
 *	with NSDataMalloc so that it can use the 'behavior' code to inherit
 *	methods from NSDataMalloc.
 *
 *	Since all the other subclasses are based on NSDataMalloc or
 *	NSMutableDataMalloc, we can put most methods in here and not
 *	bother with duplicating them in the other classes.
 *
 */

#import "common.h"

#if !defined (__GNU_LIBOBJC__)
#  include <objc/encoding.h>
#endif

#import "GNUstepBase/GSObjCRuntime.h"
#import "Foundation/NSByteOrder.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSData.h"
#import "Foundation/NSException.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSPathUtilities.h"
#import "Foundation/NSRange.h"
#import "Foundation/NSURL.h"
#import "Foundation/NSValue.h"
#import "GSPrivate.h"
#include <stdio.h>

#ifdef	HAVE_MMAP
#include <sys/mman.h>

#if	defined(HAVE_SYS_FCNTL_H)
#  include	<sys/fcntl.h>
#elif	defined(HAVE_FCNTL_H)
#  include	<fcntl.h>
#endif

#ifndef	MAP_FAILED
#define	MAP_FAILED	((void*)-1)	/* Failure address.	*/
#endif
@class	NSDataMappedFile;
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef	HAVE_SHMCTL
#include	<sys/ipc.h>
#include	<sys/shm.h>

#define	VM_RDONLY	0644		/* self read/write - other readonly */
#define	VM_ACCESS	0666		/* read/write access for all */
@class	NSDataShared;
@class	NSMutableDataShared;
#endif

@class	NSDataMalloc;
@class	NSDataStatic;
@class	NSMutableDataMalloc;

/*
 *	Some static variables to cache classes and methods for quick access -
 *	these are set up at process startup or in [NSData +initialize]
 */
static Class	dataStatic;
static Class	dataMalloc;
static Class	mutableDataMalloc;
static Class	dataBlock;
static Class	mutableDataBlock;
static Class	NSDataAbstract;
static Class	NSMutableDataAbstract;
static SEL	appendSel;
static IMP	appendImp;

static inline void
decodebase64(unsigned char *dst, const unsigned char *src)
{
  dst[0] =  (src[0]         << 2) | ((src[1] & 0x30) >> 4);
  dst[1] = ((src[1] & 0x0F) << 4) | ((src[2] & 0x3C) >> 2);
  dst[2] = ((src[2] & 0x03) << 6) |  (src[3] & 0x3F);
}

static char b64[]
  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const int crlf64 = NSDataBase64EncodingEndLineWithCarriageReturn
  | NSDataBase64EncodingEndLineWithLineFeed;

static NSUInteger
encodebase64(unsigned char **dstRef,
  const unsigned char *src,
  NSUInteger length,
  NSDataBase64EncodingOptions options)
{
  unsigned char *dst;
  NSUInteger dIndex = 0;
  NSUInteger dIndexNoNewLines = 0;
  NSUInteger sIndex;
  NSUInteger lineLength;
  NSUInteger destLen;

  lineLength = 0;
  if (options & NSDataBase64Encoding64CharacterLineLength)
    lineLength = 64;
  else if (options & NSDataBase64Encoding76CharacterLineLength)
    lineLength = 76;

  /* if no EndLine options are set but a line length is given,
   * CR+LF is implied
   */
  if (lineLength && !(options & crlf64))
    {
      options |= crlf64;
    }

  /* estimate destination length */
  destLen = 4 * ((length + 2) / 3);
  /* we need to take in account line-endings */
  if (lineLength)
    {
      if ((options & crlf64) == crlf64)
        destLen += (destLen / lineLength)*2;    // CR and LF
      else
        destLen += (destLen / lineLength);      // CR or LF
    }

  dst = NSZoneMalloc(NSDefaultMallocZone(), destLen);

  for (sIndex = 0; sIndex < length; sIndex += 3)
    {
      int	c0 = src[sIndex];
      int	c1 = (sIndex+1 < length) ? src[sIndex+1] : 0;
      int	c2 = (sIndex+2 < length) ? src[sIndex+2] : 0;

      dst[dIndex++] = b64[(c0 >> 2) & 077];
      dst[dIndex++] = b64[((c0 << 4) & 060) | ((c1 >> 4) & 017)];
      dst[dIndex++] = b64[((c1 << 2) & 074) | ((c2 >> 6) & 03)];
      dst[dIndex++] = b64[c2 & 077];
      dIndexNoNewLines += 4;
      if (lineLength && !(dIndexNoNewLines % lineLength) )
        {
          if (options & NSDataBase64EncodingEndLineWithCarriageReturn)
            dst[dIndex++] = '\r';
          if (options & NSDataBase64EncodingEndLineWithLineFeed)
            dst[dIndex++] = '\n';
        }
    }

   /* If len was not a multiple of 3, then we have encoded too
    * many characters.  Adjust appropriately.
    */
   if (sIndex == length + 1)
     {
       /* There were only 2 bytes in that last group */
       dst[dIndex - 1] = '=';
     }
   else if (sIndex == length + 2)
     {
       /* There was only 1 byte in that last group */
       dst[dIndex - 1] = '=';
       dst[dIndex - 2] = '=';
     }

  *dstRef = dst;
  return dIndex;
}

static BOOL
readContentsOfFile(NSString *path, void **buf, off_t *len, NSZone *zone)
{
  NSFileManager	*mgr = [NSFileManager defaultManager];
  NSDictionary	*att;
#if defined(_WIN32)
  const unichar	*thePath = 0;
#else
  const char	*thePath = 0;
#endif
  FILE		*theFile = 0;
  void		*tmp = 0;
  int		c;
  off_t         fileLength;
  
#ifdef __ANDROID__
  // Android: try using asset manager if path is in main bundle resources
  AAsset *asset = [NSBundle assetForPath: path withMode: AASSET_MODE_BUFFER];
  if (asset)
    {
      fileLength = AAsset_getLength(asset);

      tmp = NSZoneMalloc(zone, fileLength);
      if (tmp == 0)
	{
	  NSLog(@"Malloc failed for file (%@) of length %jd - %@", path,
	    (intmax_t)fileLength, [NSError _last]);
	  AAsset_close(asset);
	  goto failure;
	}

      int result = AAsset_read(asset, tmp, fileLength);
      AAsset_close(asset);
      
      if (result < 0)
	{
	  NSWarnFLog(@"read of file (%@) contents failed - %@", path,
	    [NSError errorWithDomain: NSPOSIXErrorDomain
				code: result
			    userInfo: nil]);
	  goto failure;
	}
      
      *buf = tmp;
      *len = fileLength;
      return YES;
    }
#endif /* __ANDROID__ */

#if defined(_WIN32)
  thePath = (const unichar*)[path fileSystemRepresentation];
#else
  thePath = [path fileSystemRepresentation];
#endif
  if (thePath == 0)
    {
      NSWarnFLog(@"Open (%@) attempt failed - bad path", path);
      return NO;
    }

  att = [mgr fileAttributesAtPath: path traverseLink: YES];
  if (nil == att)
    {
      return NO;        // No such file ... fail quietly
    }

  if ([att fileType] != NSFileTypeRegular)
    {
      NSWarnFLog(@"Open (%@) attempt failed - not a regular file", path);
      return NO;
    }

#if defined(_WIN32)
  theFile = _wfopen(thePath, L"rb");
#else
  theFile = fopen(thePath, "rb");
#endif

  if (theFile == 0)		/* We failed to open the file. */
    {
      NSDebugFLog(@"Open (%@) attempt failed - %@", path, [NSError _last]);
      goto failure;
    }

  /*
   *	Seek to the end of the file.
   */
  c = fseeko(theFile, 0, SEEK_END);
  if (c != 0)
    {
      NSWarnFLog(@"Seek to end of file (%@) failed - %@", path,
	[NSError _last]);
      goto failure;
    }

  /*
   *	Determine the length of the file (having seeked to the end of the
   *	file) by calling ftello().
   */
  fileLength = ftello(theFile);
  if (fileLength == (off_t)-1)
    {
      NSWarnFLog(@"Ftell on %@ failed - %@", path, [NSError _last]);
      goto failure;
    }
  if (fileLength >= 2147483647)
    {
      fileLength = 0;
    }

  /*
   *	Rewind the file pointer to the beginning, preparing to read in
   *	the file.
   */
  c = fseeko(theFile, 0, SEEK_SET);
  if (c != 0)
    {
      NSWarnFLog(@"Fseek to start of file (%@) failed - %@", path,
	[NSError _last]);
      goto failure;
    }

  clearerr(theFile);
  if (fileLength == 0)
    {
      unsigned char	buf[BUFSIZ];

      /*
       * Special case ... a file of length zero may be a named pipe or some
       * file in the /proc filesystem, which will return us data if we read
       * from it ... so we try reading as much as we can.
       */
      while ((c = fread(buf, 1, BUFSIZ, theFile)) != 0)
	{
	  if (tmp == 0)
	    {
	      tmp = NSZoneMalloc(zone, c);
	    }
	  else
	    {
	      tmp = NSZoneRealloc(zone, tmp, fileLength + c);
	    }
	  if (tmp == 0)
	    {
	      NSLog(@"Malloc failed for file (%@) of length %jd - %@", path,
		(intmax_t)fileLength + c, [NSError _last]);
	      goto failure;
	    }
	  memcpy(tmp + fileLength, buf, c);
	  fileLength += c;
	}
    }
  else
    {
      off_t	offset = 0;

      tmp = NSZoneMalloc(zone, fileLength);
      if (tmp == 0)
	{
	  NSLog(@"Malloc failed for file (%@) of length %jd - %@", path,
	    (intmax_t)fileLength, [NSError _last]);
	  goto failure;
	}

      while (offset < fileLength
	&& (c = fread(tmp + offset, 1, fileLength - offset, theFile)) != 0)
	{
	  offset += c;
	}
      if (offset < fileLength)
	{
          fileLength = offset;
	  tmp = NSZoneRealloc(zone, tmp, fileLength);
	}
    }
  if (ferror(theFile))
    {
      NSWarnFLog(@"read of file (%@) contents failed - %@", path,
        [NSError _last]);
      goto failure;
    }

  *buf = tmp;
  *len = fileLength;
  fclose(theFile);
  return YES;

  /*
   *	Just in case the failure action needs to be changed.
   */
failure:
    {
      NSZoneFree(zone, tmp);
    }
  if (theFile != 0)
    {
      fclose(theFile);
    }
  return NO;
}

/*
 *	NB, The start of the NSMutableDataMalloc instance variables must be
 *	identical to that of NSDataMalloc in order to inherit its methods.
 */
@interface	NSDataStatic : NSData
{
  NSUInteger	length;
  __strong void	*bytes;
  /**
   * This is a GSDataDeallocatorBlock instance, stored as an id for backwards
   * compatibility.
   */
  id            deallocator;
}
@end

@interface	NSDataEmpty: NSDataStatic
@end

@interface	NSDataMalloc : NSDataStatic
@end

@interface NSDataWithDeallocatorBlock : NSDataMalloc
@end

@interface	NSMutableDataMalloc : NSMutableData
{
  NSUInteger	length;
  __strong void	*bytes;
  /**
   * This is a GSDataDeallocatorBlock instance, stored as an id for backwards
   * compatibility.
   */
  id            deallocator;
  NSZone	*zone;
  NSUInteger	capacity;
  NSUInteger	growth;
}
/* Increase capacity to at least the specified minimum value.	*/
- (void) _grow: (NSUInteger)minimum;
@end

@interface NSMutableDataWithDeallocatorBlock : NSMutableDataMalloc
@end

#ifdef	HAVE_MMAP
@interface	NSDataMappedFile : NSDataMalloc
@end
#endif

#ifdef	HAVE_SHMCTL
@interface	NSDataShared : NSDataMalloc
{
  int		shmid;
}
- (id) initWithShmID: (int)anId length: (NSUInteger)bufferSize;
@end

@interface	NSMutableDataShared : NSMutableDataMalloc
{
  int		shmid;
}
- (id) initWithShmID: (int)anId length: (NSUInteger)bufferSize;
@end
#endif



/**
 *  <p>Class for storing a byte array.  Methods for initializing from memory a
 *  file, or the network are provided, as well as the ability to write to a
 *  file or the network.  If desired, object can take over management of a
 *  pre-allocated buffer (with malloc or similar), free'ing it when deallocated.
 *  </p>
 *  <p>The data buffer at any given time has a <em>capacity</em>, which is the
 *  size of its allocated memory area, in bytes, and a <em>length</em>, which
 *  is the length of data it is currently storing.</p>
 */
@implementation NSData

- (NSUInteger) sizeOfContentExcluding: (NSHashTable*)exclude
{
  return [self length];
}

+ (void) initialize
{
  if (self == [NSData class])
    {
      NSDataAbstract = self;
      NSMutableDataAbstract = [NSMutableData class];
      dataStatic = [NSDataStatic class];
      dataMalloc = [NSDataMalloc class];
      dataBlock = [NSDataWithDeallocatorBlock class];
      mutableDataMalloc = [NSMutableDataMalloc class];
      mutableDataBlock = [NSMutableDataWithDeallocatorBlock class];
      appendSel = @selector(appendBytes:length:);
      appendImp = [mutableDataMalloc instanceMethodForSelector: appendSel];
    }
}

+ (id) allocWithZone: (NSZone*)z
{
  if (self == NSDataAbstract)
    {
      return NSAllocateObject(dataMalloc, 0, z);
    }
  else
    {
      return NSAllocateObject(self, 0, z);
    }
}

/**
 * Returns an empty data object.
 */
+ (id) data
{
  static NSData	*empty = nil;

  if (empty == nil)
    {
      empty = [dataStatic allocWithZone: NSDefaultMallocZone()];
      empty = [empty initWithBytesNoCopy: 0 length: 0 freeWhenDone: NO];
    }
  return empty;
}

/**
 * Returns an autoreleased data object containing data copied from bytes
 * and with the specified length.  Invokes -initWithBytes:length:
 */
+ (id) dataWithBytes: (const void*)bytes
	      length: (NSUInteger)length
{
  NSData	*d;

  d = [dataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithBytes: bytes length: length];
  return AUTORELEASE(d);
}

/**
 * Returns an autoreleased data object encapsulating the data at bytes
 * and with the specified length.  Invokes
 * -initWithBytesNoCopy:length:freeWhenDone: with YES
 */
+ (id) dataWithBytesNoCopy: (void*)bytes
		    length: (NSUInteger)length
{
  NSData	*d;

  d = [dataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithBytesNoCopy: bytes length: length freeWhenDone: YES];
  return AUTORELEASE(d);
}

/**
 * Returns an autoreleased data object encapsulating the data at bytes
 * and with the specified length.  Invokes
 * -initWithBytesNoCopy:length:freeWhenDone:
 */
+ (id) dataWithBytesNoCopy: (void*)aBuffer
		    length: (NSUInteger)bufferSize
	      freeWhenDone: (BOOL)shouldFree
{
  NSData	*d;

  if (shouldFree == YES)
    {
      d = [dataMalloc allocWithZone: NSDefaultMallocZone()];
    }
  else
    {
      d = [dataStatic allocWithZone: NSDefaultMallocZone()];
    }
  d = [d initWithBytesNoCopy: aBuffer
		      length: bufferSize
		freeWhenDone: shouldFree];
  return AUTORELEASE(d);
}

/**
 * Returns a data object encapsulating the contents of the specified file.
 * Invokes -initWithContentsOfFile:
 */
+ (id) dataWithContentsOfFile: (NSString*)path
{
  NSData	*d;

  d = [dataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithContentsOfFile: path];
  return AUTORELEASE(d);
}

/**
 * Returns a data object encapsulating the contents of the specified
 * file mapped directly into memory.
 * Invokes -initWithContentsOfMappedFile:
 */
+ (id) dataWithContentsOfMappedFile: (NSString*)path
{
  NSData	*d;

#ifdef	HAVE_MMAP
  d = [NSDataMappedFile allocWithZone: NSDefaultMallocZone()];
  d = [d initWithContentsOfMappedFile: path];
#else
  d = [dataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithContentsOfMappedFile: path];
#endif
  return AUTORELEASE(d);
}

/**
 * Retrieves the information at the specified url and returns an NSData
 * instance encapsulating it.
 */
+ (id) dataWithContentsOfURL: (NSURL*)url
{
  NSData	*d;

  d = [url resourceDataUsingCache: YES];
  return d;
}

/**
 * Returns an autoreleased instance initialised by copying the contents of data.
 */
+ (id) dataWithData: (NSData*)data
{
  NSData	*d;

  d = [dataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithBytes: [data bytes] length: [data length]];
  return AUTORELEASE(d);
}

/**
 * Returns a new empty data object.
 */
+ (id) new
{
  NSData	*d;

  d = [dataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithBytesNoCopy: 0 length: 0 freeWhenDone: YES];
  return d;
}

- (id) init
{
  return [self initWithBytesNoCopy: 0 length: 0 freeWhenDone: YES];
}

- (id) initWithBase64EncodedData: (NSData*)base64Data
                         options: (NSDataBase64DecodingOptions)options
{
  NSUInteger	length;
  NSUInteger	declen;
  const unsigned char	*src;
  const unsigned char	*end;
  unsigned char *result;
  unsigned char	*dst;
  unsigned char	buf[4];
  unsigned	pos = 0;
  NSZone	*zone = [self zone];

  if (nil == base64Data)
    {
      AUTORELEASE(self);
      [NSException raise: NSInvalidArgumentException
	format: @"[%@-initWithBase64EncodedData:options:] called with "
	@"nil data", NSStringFromClass([self class])];
      return nil;
    }
  length = [base64Data length];
  if (length == 0)
    {
      return [self initWithBytesNoCopy: 0 length: 0 freeWhenDone: YES];
    }
  declen = ((length + 3) * 3)/4;
  src = (const unsigned char*)[base64Data bytes];
  end = &src[length];

  result = NSZoneMalloc(zone, declen);
  dst = result;

  while (src != end)
    {
      int	c = *src++;

      if (isupper(c))
	{
	  c -= 'A';
	}
      else if (islower(c))
	{
	  c = c - 'a' + 26;
	}
      else if (isdigit(c))
	{
	  c = c - '0' + 52;
	}
      else if (c == '/')
	{
	  c = 63;
	}
      else if (c == '+')
	{
	  c = 62;
	}
      else if  (c == '=')
        {
          /* Only legal as padding at end of string */
          while (src != end)
            {
              c = *src++;
              if (c != '=')
                {
                  if (options & NSDataBase64DecodingIgnoreUnknownCharacters)
                    {
                      if (!isupper(c) && !islower(c) && !isdigit(c)
                        && c != '/' && c != '+')
                        {
                          continue;     // An unknown character
                        }
                    }
                  free(result);
                  DESTROY(self);
                  return nil;
                }
            }
          /* For OSX compatibility, if we have unnecessary padding at the
           * end of a string, we treat it as representing a zero byte.
           */
          if (0 == pos)
            {
              *dst++ = '\0';
            }
          c = -1;
        }
      else if (options & NSDataBase64DecodingIgnoreUnknownCharacters)
        {
          c = -1;               /* ignore */
        }
      else
        {
          free(result);
          DESTROY(self);
          return nil;
        }

      if (c >= 0)
	{
	  buf[pos++] = c;
	  if (pos == 4)
	    {
	      pos = 0;
	      decodebase64(dst, buf);
	      dst += 3;
	    }
	}
    }

  if (pos > 0)
    {
      unsigned	i;

      for (i = pos; i < 4; i++)
	{
	  buf[i] = '\0';
	}
      pos--;
      if (pos > 0)
	{
	  unsigned char	tail[3];
	  decodebase64(tail, buf);
	  memcpy(dst, tail, pos);
	  dst += pos;
	}
    }
  length = dst - result;
  if (options & NSDataBase64DecodingIgnoreUnknownCharacters)
    {
      /* If the decoded length is a lot shorter than expected (because we
       * ignored a lot of characters), reallocate to get smaller memory.
       */
      if ((((declen - length) * 100) / declen) > 5)
        {
	  result = NSZoneRealloc(zone, result, length);
        }
    }
  return [self initWithBytesNoCopy: result length: length freeWhenDone: YES];
}

- (id) initWithBase64EncodedString: (NSString*)base64String
                           options: (NSDataBase64DecodingOptions)options
{
  NSData        *data;

  if (nil == base64String)
    {
      AUTORELEASE(self);
      [NSException raise: NSInvalidArgumentException
	format: @"[%@-initWithBase64EncodedString:options:] called with "
	@"nil string", NSStringFromClass([self class])];
      return nil;
    }
  data = [base64String dataUsingEncoding: NSUTF8StringEncoding];
  return [self initWithBase64EncodedData: data options: options];
}

/**
 * Makes a copy of bufferSize bytes of data at aBuffer, and passes it to
 * -initWithBytesNoCopy:length:freeWhenDone: with a YES argument in order
 * to initialise the receiver.  Returns the result.
 */
- (id) initWithBytes: (const void*)aBuffer
	      length: (NSUInteger)bufferSize
{
  void	*ptr = 0;

  if (bufferSize > 0)
    {
      if (aBuffer == 0)
	{
	  [NSException raise: NSInvalidArgumentException
	    format: @"[%@-initWithBytes:length:] called with "
	    @"length but null bytes", NSStringFromClass([self class])];
	}
      ptr = NSAllocateCollectable(bufferSize, 0);
      if (ptr == 0)
	{
	  DESTROY(self);
	  return nil;
	}
      memcpy(ptr, aBuffer, bufferSize);
    }
  return [self initWithBytesNoCopy: ptr
			    length: bufferSize
		      freeWhenDone: YES];
}

/**
 * Invokes -initWithBytesNoCopy:length:freeWhenDone: with the last argument
 * set to YES.  Returns the resulting initialised data object (which may not
 * be the receiver).
 */
- (id) initWithBytesNoCopy: (void*)aBuffer
		    length: (NSUInteger)bufferSize
{
  return [self initWithBytesNoCopy: aBuffer
			    length: bufferSize
		      freeWhenDone: YES];
}

/** <init /><override-subclass />
 * Initialises the receiver.<br />
 * The value of aBuffer is a pointer to something to be stored.<br />
 * The value of bufferSize is the number of bytes to use.<br />
 * The value of shouldFree specifies whether the receiver should
 * attempt to free the memory pointer to by aBuffer when the receiver
 * is deallocated ... ie. it says whether the receiver <em>owns</em>
 * the memory.  Supplying the wrong value here will lead to memory
 * leaks or crashes.
 */
- (id) initWithBytesNoCopy: (void*)aBuffer
		    length: (NSUInteger)bufferSize
	      freeWhenDone: (BOOL)shouldFree
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (instancetype) initWithBytesNoCopy: (void*)bytes
                              length: (NSUInteger)length
                         deallocator: (GSDataDeallocatorBlock)deallocator
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/**
 * Initialises the receiver with the contents of the specified file.<br />
 * Returns the resulting object.<br />
 * Returns nil if the file does not exist or can not be read for some reason.
 */
- (id) initWithContentsOfFile: (NSString*)path
{
  void		*fileBytes;
  off_t         fileLength;

  if (readContentsOfFile(path, &fileBytes, &fileLength, [self zone]) == NO)
    {
      DESTROY(self);
      return nil;
    }
  self = [self initWithBytesNoCopy: fileBytes
			    length: (NSUInteger)fileLength
		      freeWhenDone: YES];
  return self;
}

/**
 *  Initialize with data pointing to contents of file at path.  Bytes are
 *  only "swapped in" as needed.  File should not be moved or deleted for
 *  the life of this object.
 */
- (id) initWithContentsOfMappedFile: (NSString *)path
{
#ifdef	HAVE_MMAP
  NSZone	*z = [self zone];
  DESTROY(self);
  self = [NSDataMappedFile allocWithZone: z];
  return [self initWithContentsOfMappedFile: path];
#else
  return [self initWithContentsOfFile: path];
#endif
}

/**
 *  Initialize with data pointing to contents of URL, which will be
 *  retrieved immediately in a blocking manner.
 */
- (id) initWithContentsOfURL: (NSURL*)url
{
  NSData	*data = [url resourceDataUsingCache: YES];

  return [self initWithData: data];
}

/**
 *  Initializes by copying data's bytes into a new buffer.
 */
- (id) initWithData: (NSData*)data
{
  if (data == nil)
    {
      return [self initWithBytesNoCopy: 0 length: 0 freeWhenDone: YES];
    }
  if ([data isKindOfClass: [NSData class]] == NO)
    {
      NSLog(@"-initWithData: passed a non-data object");
      DESTROY(self);
      return nil;
    }
  return [self initWithBytes: [data bytes] length: [data length]];
}


// Accessing Data

/** <override-subclass>
 * Returns a pointer to the data encapsulated by the receiver.
 */
- (const void*) bytes
{
  [self subclassResponsibility: _cmd];
  return 0;
}

/**
 *  Returns a short description of this object.
 */
- (NSString*) description
{
  extern void     GSPropertyListMake(id,NSDictionary*,BOOL,BOOL,unsigned,id*);
  NSMutableString       *result = nil;

  GSPropertyListMake(self, nil, NO, YES, 0, &result);
  return result;
}

/**
 * Copies the contents of the memory encapsulated by the receiver into
 * the specified buffer.  The buffer must be large enough to contain
 * -length bytes of data ... if it isn't then a crash is likely to occur.<br />
 * Invokes -getBytes:range: with the range set to the whole of the receiver.
 */
- (void) getBytes: (void*)buffer
{
  [self getBytes: buffer range: NSMakeRange(0, [self length])];
}

/**
 * Copies length bytes of data from the memory encapsulated by the receiver
 * into the specified buffer.  The buffer must be large enough to contain
 * length bytes of data ... if it isn't then a crash is likely to occur.<br />
 * If length is greater than the size of the receiver, only the available
 * bytes are copied.
 */
- (void) getBytes: (void*)buffer length: (NSUInteger)length
{
  NSUInteger	l = [self length];

  [self getBytes: buffer range: NSMakeRange(0,  l < length ? l : length)];
}

/**
 * Copies data from the memory encapsulated by the receiver (in the range
 * specified by aRange) into the specified buffer.<br />
 * The buffer must be large enough to contain the data ... if it isn't then
 * a crash is likely to occur.<br />
 * If aRange specifies a range which does not entirely lie within the
 * receiver, an exception is raised.
 */
- (void) getBytes: (void*)buffer range: (NSRange)aRange
{
  NSUInteger	size = [self length];

  GS_RANGE_CHECK(aRange, size);
  if (aRange.length > 0)
    {
      const void	*bytes = [self bytes];

      if (0 == bytes)
	{
	  [NSException raise: NSInternalInconsistencyException
		      format: @"missing bytes in getBytes:range:"];
	}
      memcpy(buffer, bytes + aRange.location, aRange.length);
    }
}

- (id) replacementObjectForPortCoder: (NSPortCoder*)aCoder
{
  return self;
}

/**
 * Returns an NSData instance encapsulating the memory from the receiver
 * specified by the range aRange.<br />
 * If aRange specifies a range which does not entirely lie within the
 * receiver, an exception is raised.
 */
- (NSData*) subdataWithRange: (NSRange)aRange
{
  void		*buffer;
  NSUInteger	l = [self length];

  GS_RANGE_CHECK(aRange, l);

  buffer = NSZoneMalloc(NSDefaultMallocZone(), aRange.length);
  if (buffer == 0)
    {
      [NSException raise: NSMallocException
		  format: @"No memory for subdata of NSData object"];
    }
  [self getBytes: buffer range: aRange];

  return [NSData dataWithBytesNoCopy: buffer length: aRange.length];
}

/**
 * Finds and returns the range of the first occurrence of the given data, within the given range, subject to given options.
 */
- (NSRange) rangeOfData: (NSData *)dataToFind
                options: (NSDataSearchOptions)mask
                  range: (NSRange)searchRange
{
  NSUInteger  length = [self length];
  NSUInteger  countOther = [dataToFind length];
  const void* bytesSelf = [self bytes];
  const void* bytesOther = [dataToFind bytes];
  NSRange     result;

  GS_RANGE_CHECK(searchRange, length);
  if (dataToFind == nil)
    [NSException raise: NSInvalidArgumentException format: @"range of nil"];

  /* Zero length data is always found at the start of the given range.
   */
  if (0 == countOther)
    {
      if ((mask & NSDataSearchBackwards) == NSDataSearchBackwards)
        {
          searchRange.location += searchRange.length;
        }
      searchRange.length = 0;
      return searchRange;
    }

  if (searchRange.length < countOther)
    {
      /* Range to search is smaller than data to look for.
       */
      result = NSMakeRange(NSNotFound, 0);
    }
  else
    {
      if ((mask & NSDataSearchAnchored) == NSDataSearchAnchored
        || searchRange.length == countOther)
        {
          /* Range to search is same size as data to look for.
           */
          if ((mask & NSDataSearchBackwards) == NSDataSearchBackwards)
            {
              searchRange.location = NSMaxRange(searchRange) - countOther;
              searchRange.length = countOther;
            }
          else
            {
              searchRange.length = countOther;
            }
          if (memcmp(bytesSelf + searchRange.location, bytesOther,
	    countOther) == 0)
            {
              result = searchRange;
            }
          else
            {
              result = NSMakeRange(NSNotFound, 0);
            }
        }
      else
        {
          /* Range to search is bigger than data to look for.
           */

          NSUInteger pos;
          NSUInteger end;

          end = searchRange.length - countOther + 1;
          if ((mask & NSDataSearchBackwards) == NSDataSearchBackwards)
            {
              pos = end;
            }
          else
            {
              pos = 0;
            }

          if ((mask & NSDataSearchBackwards) == NSDataSearchBackwards)
            {
              while (pos-- > 0)
                {
                  if (memcmp(bytesSelf + searchRange.location + pos,
		    bytesOther, countOther) == 0)
                    {
                      break;
                    }
                }
            }
          else
            {
              while (pos < end)
                {
                  if (memcmp(bytesSelf + searchRange.location + pos,
		    bytesOther, countOther) == 0)
                    {
                      break;
                    }
                  pos++;
                }
            }

          if (pos >= end)
            {
              result = NSMakeRange(NSNotFound, 0);
            }
          else
            {
              result = NSMakeRange(searchRange.location + pos, countOther);
            }
        }
    }
  return result;
}

- (NSData *) base64EncodedDataWithOptions: (NSDataBase64EncodingOptions)options
{
  void          *srcBytes = (void*)[self bytes];
  NSUInteger    length = [self length];
  NSUInteger    resLen;
  unsigned char *resBytes;
  NSData        *result;

  resLen = encodebase64(&resBytes, srcBytes, length, options);
  result = [[NSData alloc] initWithBytesNoCopy: resBytes
                                        length: resLen
                                  freeWhenDone: YES];
  return AUTORELEASE(result);
}

- (NSString *) base64EncodedStringWithOptions:
  (NSDataBase64EncodingOptions)options
{
  void          *srcBytes = (void*)[self bytes];
  NSUInteger    length = [self length];
  NSUInteger    resLen;
  unsigned char *resBytes;
  NSString      *result;

  resLen = encodebase64(&resBytes, srcBytes, length, options);
  result = [[NSString alloc] initWithBytesNoCopy: resBytes
                                          length: resLen
                                        encoding: NSASCIIStringEncoding
                                    freeWhenDone: YES];
  return AUTORELEASE(result);
}

- (NSUInteger) hash
{
  unsigned char	buf[64];
  NSUInteger	l = [self length];
  NSUInteger	ret =0;

  l = MIN(l,64);

  /*
   * hash for empty data matches hash for empty string
   */
  if (l == 0)
    {
      return 0xfffffffe;
    }

  [self getBytes: &buf range: NSMakeRange(0, l)];

  while (l-- > 0)
    {
      ret = (ret << 5) + ret + buf[l];
    }
  // Again, match NSString
  if (ret == 0)
    {
       ret = 0xffffffff;
    }
  return ret;
}

- (BOOL) isEqual: anObject
{
  if ([anObject isKindOfClass: [NSData class]])
    return [self isEqualToData: anObject];
  return NO;
}

/**
 * Returns a boolean value indicating if the receiver and other contain
 * identical data (using a byte by byte comparison).  Assumes that the
 * other object is an NSData instance ... may raise an exception if it isn't.
 */
- (BOOL) isEqualToData: (NSData*)other
{
  NSUInteger len;

  if (other == self)
    {
      return YES;
    }
  if ((len = [self length]) != [other length])
    {
      return NO;
    }
  return (memcmp([self bytes], [other bytes], len) ? NO : YES);
}

/** <override-subclass>
 * Returns the number of bytes of data encapsulated by the receiver.
 */
- (NSUInteger) length
{
  /* This is left to concrete subclasses to implement. */
  [self subclassResponsibility: _cmd];
  return 0;
}

- (BOOL) writeToFile: (NSString*)path atomically: (BOOL)useAuxiliaryFile
{
  if (useAuxiliaryFile)
    {
      return [self writeToFile: path options: NSDataWritingAtomic error: 0];
    }
  else
    {
      return [self writeToFile: path options: 0 error: 0];
    }
}

- (BOOL) writeToURL: (NSURL*)anURL atomically: (BOOL)flag
{
  if (flag)
    {
      return [self writeToURL: anURL options: NSDataWritingAtomic error: 0];
    }
  else
    {
      return [self writeToURL: anURL options: 0 error: 0];
    }
}


// Deserializing Data

/**
 *  Copies data from buffer starting from cursor.  <strong>Deprecated</strong>.
 *  Use [-getBytes:] and related methods instead.
 */
- (unsigned int) deserializeAlignedBytesLengthAtCursor: (unsigned int*)cursor
{
  return (unsigned)[self deserializeIntAtCursor: cursor];
}

/**
 *  Copies data from buffer starting from cursor.  <strong>Deprecated</strong>.
 *  Use [-getBytes:] and related methods instead.
 */
- (void) deserializeBytes: (void*)buffer
		   length: (unsigned int)bytes
		 atCursor: (unsigned int*)cursor
{
  NSRange	range = { *cursor, bytes };

  [self getBytes: buffer range: range];
  *cursor += bytes;
}

- (void) deserializeDataAt: (void*)data
	        ofObjCType: (const char*)type
		  atCursor: (unsigned int*)cursor
		   context: (id <NSObjCTypeSerializationCallBack>)callback
{
  if (!type || !data)
    return;

  switch (*type)
    {
      case _C_ID:
	{
	  [callback deserializeObjectAt: data
			     ofObjCType: type
			       fromData: self
			       atCursor: cursor];
	  return;
	}
      case _C_CHARPTR:
	{
	  int32_t length;

	  [self deserializeBytes: &length
			  length: sizeof(length)
			atCursor: cursor];
	  length = GSSwapBigI32ToHost(length);
	  if (length == -1)
	    {
	      *(const char**)data = 0;
	      return;
	    }
	  else
	    {
	      unsigned	len = (length+1)*sizeof(char);

	      *(char**)data = (char*)NSZoneMalloc(NSDefaultMallocZone(), len);
	      if (*(char**)data == 0)
	        {
		  [NSException raise: NSMallocException
			      format: @"out of memory to deserialize bytes"];
		}
	    }

	  [self deserializeBytes: *(char**)data
			  length: length
			atCursor: cursor];
	  (*(char**)data)[length] = '\0';
	  return;
	}
      case _C_ARY_B:
	{
	  unsigned	offset = 0;
	  unsigned	size;
	  unsigned	count = atoi(++type);
	  unsigned	i;

	  while (isdigit(*type))
	    {
	      type++;
	    }
	  size = objc_sizeof_type(type);

	  for (i = 0; i < count; i++)
	    {
	      [self deserializeDataAt: (char*)data + offset
			   ofObjCType: type
			     atCursor: cursor
			      context: callback];
	      offset += size;
	    }
	  return;
	}
      case _C_STRUCT_B:
	{
	  struct objc_struct_layout layout;

	  objc_layout_structure (type, &layout);
	  while (objc_layout_structure_next_member (&layout))
	    {
	      unsigned		offset;
	      unsigned		align;
	      const char	*ftype;

	      objc_layout_structure_get_info (&layout, &offset, &align, &ftype);

	      [self deserializeDataAt: ((char*)data) + offset
			   ofObjCType: ftype
			     atCursor: cursor
			      context: callback];
	    }
	  return;
        }
      case _C_PTR:
	{
	  unsigned	len = objc_sizeof_type(++type);

	  *(char**)data = (char*)NSZoneMalloc(NSDefaultMallocZone(), len);
	  if (*(char**)data == 0)
	    {
	      [NSException raise: NSMallocException
			  format: @"out of memory to deserialize bytes"];
	    }
	  [self deserializeDataAt: *(char**)data
		       ofObjCType: type
			 atCursor: cursor
			  context: callback];
	  return;
        }
      case _C_CHR:
      case _C_UCHR:
	{
	  [self deserializeBytes: data
			  length: sizeof(unsigned char)
			atCursor: cursor];
	  return;
	}
      case _C_SHT:
      case _C_USHT:
	{
	  unsigned short ns;

	  [self deserializeBytes: &ns
			  length: sizeof(unsigned short)
			atCursor: cursor];
	  *(unsigned short*)data = NSSwapBigShortToHost(ns);
	  return;
	}
      case _C_INT:
      case _C_UINT:
	{
	  unsigned ni;

	  [self deserializeBytes: &ni
			  length: sizeof(unsigned)
			atCursor: cursor];
	  *(unsigned*)data = NSSwapBigIntToHost(ni);
	  return;
	}
      case _C_LNG:
      case _C_ULNG:
	{
	  unsigned long nl;

	  [self deserializeBytes: &nl
			  length: sizeof(unsigned long)
			atCursor: cursor];
	  *(unsigned long*)data = NSSwapBigLongToHost(nl);
	  return;
	}
      case _C_LNG_LNG:
      case _C_ULNG_LNG:
	{
	  unsigned long long nl;

	  [self deserializeBytes: &nl
			  length: sizeof(unsigned long long)
			atCursor: cursor];
	  *(unsigned long long*)data = NSSwapBigLongLongToHost(nl);
	  return;
	}
      case _C_FLT:
	{
	  NSSwappedFloat nf;

	  [self deserializeBytes: &nf
			  length: sizeof(NSSwappedFloat)
			atCursor: cursor];
	  *(float*)data = NSSwapBigFloatToHost(nf);
	  return;
	}
      case _C_DBL:
	{
	  NSSwappedDouble nd;

	  [self deserializeBytes: &nd
			  length: sizeof(NSSwappedDouble)
			atCursor: cursor];
	  *(double*)data = NSSwapBigDoubleToHost(nd);
	  return;
	}
#if __GNUC__ > 2 && defined(_C_BOOL)
      case _C_BOOL:
	{
	  [self deserializeBytes: data
			  length: sizeof(_Bool)
			atCursor: cursor];
	  return;
	}
#endif
      case _C_CLASS:
	{
	  uint16_t ni;

	  [self deserializeBytes: &ni
			  length: sizeof(ni)
			atCursor: cursor];
	  ni = GSSwapBigI16ToHost(ni);
	  if (ni == 0)
	    {
	      *(Class*)data = 0;
	    }
	  else
	    {
	      char	name[ni+1];
	      Class	c;

	      [self deserializeBytes: name
			      length: ni
			    atCursor: cursor];
	      name[ni] = '\0';
	      c = objc_lookUpClass(name);
	      if (c == 0)
		{
		  NSLog(@"[%s %s] can't find class - %s",
		    class_getName([self class]),
		    sel_getName(_cmd), name);
		}
	      *(Class*)data = c;
	    }
	  return;
	}
      case _C_SEL:
	{
	  uint16_t	ln;
	  uint16_t	lt;

	  [self deserializeBytes: &ln
			  length: sizeof(ln)
			atCursor: cursor];
	  ln = GSSwapBigI16ToHost(ln);
	  [self deserializeBytes: &lt
			  length: sizeof(lt)
			atCursor: cursor];
	  lt = GSSwapBigI16ToHost(lt);
	  if (ln == 0)
	    {
	      *(SEL*)data = 0;
	    }
	  else
	    {
	      char	name[ln+1];
	      char	types[lt+1];
	      SEL	sel;

	      [self deserializeBytes: name
			      length: ln
			    atCursor: cursor];
	      name[ln] = '\0';
	      [self deserializeBytes: types
			      length: lt
			    atCursor: cursor];
	      types[lt] = '\0';

	      if (lt)
		{
		  sel = GSSelectorFromNameAndTypes(name, types);
		}
	      else
		{
		  sel = sel_registerName(name);
		}
	      if (sel == 0)
		{
		      [NSException raise: NSInternalInconsistencyException
				  format: @"can't make sel with name '%s' "
					      @"and types '%s'", name, types];
		}
	      *(SEL*)data = sel;
	    }
	  return;
	}
      default:
	[NSException raise: NSGenericException
		    format: @"Unknown type to deserialize - '%s'", type];
    }
}

/**
 *  Retrieve an int from this data, which is assumed to be in network
 *  (big-endian) byte order.  Cursor refers to byte position.
 */
- (int) deserializeIntAtCursor: (unsigned int*)cursor
{
  unsigned ni, result;

  [self deserializeBytes: &ni length: sizeof(unsigned) atCursor: cursor];
  result = NSSwapBigIntToHost(ni);
  return result;
}

/**
 *  Retrieve an int from this data, which is assumed to be in network
 *  (big-endian) byte order.  Index refers to byte position.
 */
- (int) deserializeIntAtIndex: (unsigned int)index
{
  unsigned ni, result;

  [self deserializeBytes: &ni length: sizeof(unsigned) atCursor: &index];
  result = NSSwapBigIntToHost(ni);
  return result;
}

/**
 *  Retrieve ints from intBuffer, which is assumed to be in network (big-endian)
 *  byte order.  Count refers to number of ints, but index refers to byte
 *  position.
 */
- (void) deserializeInts: (int*)intBuffer
		   count: (unsigned int)numInts
	        atCursor: (unsigned int*)cursor
{
  unsigned i;

  [self deserializeBytes: &intBuffer
		  length: numInts * sizeof(unsigned)
		atCursor: cursor];
  for (i = 0; i < numInts; i++)
    intBuffer[i] = NSSwapBigIntToHost(intBuffer[i]);
}

/**
 *  Retrieve ints from intBuffer, which is assumed to be in network (big-endian)
 *  byte order.  Count refers to number of ints, but index refers to byte
 *  position.
 */
- (void) deserializeInts: (int*)intBuffer
		   count: (unsigned int)numInts
		 atIndex: (unsigned int)index
{
  unsigned i;

  [self deserializeBytes: &intBuffer
		  length: numInts * sizeof(int)
		atCursor: &index];
  for (i = 0; i < numInts; i++)
    {
      intBuffer[i] = NSSwapBigIntToHost(intBuffer[i]);
    }
}

- (id) copyWithZone: (NSZone*)z
{
  if (NSShouldRetainWithZone(self, z) &&
	[self isKindOfClass: [NSMutableData class]] == NO)
    return RETAIN(self);
  else
    return [[dataMalloc allocWithZone: z]
	initWithBytes: [self bytes] length: [self length]];
}

- (id) mutableCopyWithZone: (NSZone*)zone
{
  return [[mutableDataMalloc allocWithZone: zone]
	initWithBytes: [self bytes] length: [self length]];
}

- (void) encodeWithCoder: (NSCoder*)coder
{
  if ([coder allowsKeyedCoding])
    {
      [coder encodeObject: self forKey: @"NS.bytes"];
    }
  else
    {
      [coder encodeDataObject: self];
    }
}

- (id) initWithCoder: (NSCoder*)coder
{
  id obj = nil;

  if ([coder allowsKeyedCoding])
    {
      obj = [coder decodeObjectForKey: @"NS.bytes"];
    }
  else
    {
      obj = [coder decodeDataObject];
    }

  if (obj != self)
    {
      ASSIGN(self, obj);
    }
  return self;
}

- (BOOL) writeToFile: (NSString *)path
             options: (NSUInteger)writeOptionsMask
               error: (NSError **)errorPtr
{
#if defined(_WIN32)
  NSUInteger	length = [path length];
  unichar	wthePath[length + 100];
  unichar	wtheRealPath[length + 100];
  int		c;
  FILE		*theFile;
  BOOL		useAuxiliaryFile = NO;
  BOOL		error_BadPath = YES;

  if (writeOptionsMask & NSDataWritingAtomic)
    {
      useAuxiliaryFile = YES;
    }
  [path getCharacters: wtheRealPath];
  wtheRealPath[length] = L'\0';
  error_BadPath = (length <= 0);
  if (error_BadPath)
    {
      NSWarnMLog(@"Open (%@) attempt failed - bad path",path);
      return NO;
    }

  if (useAuxiliaryFile)
    {
      /* Use the path name of the destination file as a prefix for the
       * _wmktemp() call so that we can be sure that both files are on
       * the same filesystem and the subsequent rename() will work. */
      wcscpy(wthePath, wtheRealPath);
      wcscat(wthePath, L"XXXXXX");
      if (_wmktemp(wthePath) == 0)
	{
	  NSWarnMLog(@"mktemp (%@) failed - %@",
	  [NSString stringWithCharacters: wthePath length: wcslen(wthePath)],
	    [NSError _last]);
	  goto failure;
	}
    }
  else
    {
      wcscpy(wthePath,wtheRealPath);
    }
  theFile = _wfopen(wthePath, L"wb");

  if (theFile == 0)
    {
      /* Something went wrong; we weren't
       * even able to open the file. */
      NSWarnMLog(@"Open (%@) failed - %@",
	[NSString stringWithCharacters: wthePath length: wcslen(wthePath)],
	  [NSError _last]);
      goto failure;
    }

  /* Now we try and write the NSData's bytes to the file.  Here `c' is
   * the number of bytes which were successfully written to the file
   * in the fwrite() call. */
  c = fwrite([self bytes], sizeof(char), [self length], theFile);

  if (c < (int)[self length])        /* We failed to write everything for
                                 * some reason. */
    {
      NSWarnMLog(@"Fwrite (%@) failed - %@",
	[NSString stringWithCharacters: wthePath length: wcslen(wthePath)],
	[NSError _last]);
      goto failure;
    }

  /* We're done, so close everything up. */
  c = fclose(theFile);

  if (c != 0)                   /* I can't imagine what went wrong
                                 * closing the file, but we got here,
                                 * so we need to deal with it. */
    {
      NSWarnMLog(@"Fclose (%@) failed - %@",
	[NSString stringWithCharacters: wthePath length: wcslen(wthePath)],
	[NSError _last]);
      goto failure;
    }

  /* If we used a temporary file, we still need to rename() it be the
   * real file.  Also, we need to try to retain the file attributes of
   * the original file we are overwriting (if we are) */
  if (useAuxiliaryFile)
    {
      NSFileManager		*mgr = [NSFileManager defaultManager];
      NSMutableDictionary	*att = nil;
      NSUInteger		perm;

      if ([mgr fileExistsAtPath: path])
	{
	  att = [[mgr fileAttributesAtPath: path
			      traverseLink: YES] mutableCopy];
	  IF_NO_GC(AUTORELEASE(att));
	}

      /* To replace the existing file on windows, it must be writable.
       */
      perm = [att filePosixPermissions];
      if (perm != NSNotFound && (perm & 0200) == 0)
	{
          [mgr changeFileAttributes: [NSDictionary dictionaryWithObjectsAndKeys:
	    [NSNumber numberWithUnsignedInt: 0777], NSFilePosixPermissions,
	    nil] atPath: path];
	}
      /*
       * The windoze implementation of the POSIX rename() function is buggy
       * and doesn't work if the destination file already exists ... so we
       * use a windoze specific move file function instead.
       */
      if (MoveFileExW(wthePath, wtheRealPath, MOVEFILE_REPLACE_EXISTING) != 0)
	{
	  c = 0;
	}
	/* Windows 9x does not support MoveFileEx */
      else if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{
	  unichar	secondaryFile[length + 100];

	  wcscpy(secondaryFile, wthePath);
	  wcscat(secondaryFile, L"-delete");
	  // Delete the intermediate name just in case
	  DeleteFileW(secondaryFile);
	  // Move the existing file to the temp name
	  if (MoveFileW(wtheRealPath, secondaryFile) != 0)
	    {
	      if (MoveFileW(wthePath, wtheRealPath) != 0)
		{
		  c = 0;
		  // Delete the old file if possible
		  DeleteFileW(secondaryFile);
		}
	      else
		{
		  c = -1; // failure, restore the old file if possible
		  MoveFileW(secondaryFile, wtheRealPath);
		}
	    }
	  else
	    {
	      c = -1; // failure
	    }
	}
      else
	{
	  c = -1;
	}

      if (c != 0)               /* Many things could go wrong, I guess. */
        {
          NSWarnMLog(@"Rename ('%@' to '%@') failed - %@",
	    [NSString stringWithCharacters: wthePath
				    length: wcslen(wthePath)],
	    [NSString stringWithCharacters: wtheRealPath
				    length: wcslen(wtheRealPath)],
	    [NSError _last]);
          goto failure;
        }

      if (att != nil)
	{
	  /*
	   * We have created a new file - so we attempt to make it's
	   * attributes match that of the original (except for those
	   * we can't reasonably set).
	   */
	  [att removeObjectForKey: NSFileSize];
	  [att removeObjectForKey: NSFileCreationDate];
	  [att removeObjectForKey: NSFileModificationDate];
	  [att removeObjectForKey: NSFileReferenceCount];
	  [att removeObjectForKey: NSFileSystemNumber];
	  [att removeObjectForKey: NSFileSystemFileNumber];
	  [att removeObjectForKey: NSFileDeviceIdentifier];
	  [att removeObjectForKey: NSFileType];
	  if ([mgr changeFileAttributes: att atPath: path] == NO)
	    {
	      NSWarnMLog(@"Unable to correctly set attributes for '%@' to %@",
		path, att);
	    }
	}
#ifdef HAVE_GETEUID
      else if (geteuid() == 0 && [@"root" isEqualToString: NSUserName()] == NO)
	{
	  att = [NSDictionary dictionaryWithObjectsAndKeys:
	    NSFileOwnerAccountName, NSUserName(), nil];
	  if ([mgr changeFileAttributes: att atPath: path] == NO)
	    {
	      NSWarnMLog(@"Unable to correctly set ownership for '%@'", path);
	    }
	}
#endif
    }

  /* success: */
  return YES;

  /* Just in case the failure action needs to be changed. */
failure:
  /*
   * Attempt to tidy up by removing temporary file on failure.
   */
  if (useAuxiliaryFile)
    {
      _wunlink(wthePath);
    }
  return NO;

#else

  char		thePath[BUFSIZ*2+8];
  char		theRealPath[BUFSIZ*2];
  int		c;
  FILE		*theFile;
  BOOL		useAuxiliaryFile = NO;
  BOOL		error_BadPath = YES;

  if (writeOptionsMask & NSDataWritingAtomic)
    {
      useAuxiliaryFile = YES;
    }
  if ([path canBeConvertedToEncoding: [NSString defaultCStringEncoding]])
    {
      const char *local_c_path = [path cString];

      if (local_c_path != 0 && strlen(local_c_path) < (BUFSIZ*2))
	{
	  strncpy(theRealPath, local_c_path, sizeof(theRealPath) - 1);
	  theRealPath[sizeof(theRealPath) - 1] = '\0';
	  error_BadPath = NO;
	}
    }
  if (error_BadPath)
    {
      NSWarnMLog(@"Open (%@) attempt failed - bad path",path);
      return NO;
    }

#  if   defined(HAVE_MKSTEMP)
  if (useAuxiliaryFile)
    {
      int	desc;
      int	mask;
      int	length;

      length = strlen(theRealPath);
      if (length > sizeof(thePath) - 7)
	{
	  length = sizeof(thePath) - 7;
	} 
      memcpy(thePath, theRealPath, length);
      memcpy(thePath + length, "XXXXXX", 6);
      thePath[length + 6] = '\0';
      if ((desc = mkstemp(thePath)) < 0)
	{
          NSWarnMLog(@"mkstemp (%s) failed - %@", thePath, [NSError _last]);
          goto failure;
	}
      mask = umask(0);
      umask(mask);
      fchmod(desc, 0644 & ~mask);
      if ((theFile = fdopen(desc, "w")) == 0)
	{
	  close(desc);
	}
    }
  else
    {
      strncpy(thePath, theRealPath, sizeof(thePath) - 1);
      thePath[sizeof(thePath) - 1] = '\0';
      theFile = fopen(thePath, "wb");
    }
#  else
  if (useAuxiliaryFile)
    {
      /* Use the path name of the destination file as a prefix for the
       * mktemp() call so that we can be sure that both files are on
       * the same filesystem and the subsequent rename() will work. */
      strncpy(thePath, theRealPath, sizeof(thePath) - 1);
      thePath[sizeof(thePath) - 1] = '\0';
      strncat(thePath, "XXXXXX", 6);
      if (mktemp(thePath) == 0)
	{
          NSWarnMLog(@"mktemp (%s) failed - %@", thePath, [NSError _last]);
          goto failure;
	}
    }
  else
    {
      strncpy(thePath, theRealPath, sizeof(thePath) - 1);
      thePath[sizeof(thePath) - 1] = '\0';
    }
  theFile = fopen(thePath, "wb");
#  endif

  if (theFile == 0)
    {
      /* Something went wrong; we weren't
       * even able to open the file. */
      NSWarnMLog(@"Open (%s) failed - %@", thePath, [NSError _last]);
      goto failure;
    }

  /* Now we try and write the NSData's bytes to the file.  Here `c' is
   * the number of bytes which were successfully written to the file
   * in the fwrite() call. */
  c = fwrite([self bytes], sizeof(char), [self length], theFile);

  if (c < (int)[self length])        /* We failed to write everything. */
    {
      NSWarnMLog(@"Fwrite (%s) failed - %@", thePath, [NSError _last]);
      fclose(theFile);
      goto failure;
    }

  /* We're done, so close everything up. */
  c = fclose(theFile);

  if (c != 0)                   /* I can't imagine what went wrong
                                 * closing the file, but we got here,
                                 * so we need to deal with it. */
    {
      NSWarnMLog(@"Fclose (%s) failed - %@", thePath, [NSError _last]);
      goto failure;
    }

  /* If we used a temporary file, we still need to rename() it be the
   * real file.  Also, we need to try to retain the file attributes of
   * the original file we are overwriting (if we are) */
  if (useAuxiliaryFile)
    {
      NSFileManager		*mgr = [NSFileManager defaultManager];
      NSDictionary	  *att = nil;

      if ([mgr fileExistsAtPath: path])
	{
	  att = [mgr fileAttributesAtPath: path traverseLink: YES];
	}

      c = rename(thePath, theRealPath);
      if (c != 0)               /* Many things could go wrong, I guess. */
        {
	  NSWarnMLog(@"Rename ('%s' to '%s') failed - %@",
	    thePath, theRealPath, [NSError _last]);
          goto failure;
        }

      if (att != nil)
	{
          NSMutableDictionary *mAtt = [att mutableCopy];

          IF_NO_GC(AUTORELEASE(mAtt));
	  /*
	   * We have created a new file - so we attempt to make it's
	   * attributes match that of the original.
	   */
	  [mAtt removeObjectForKey: NSFileSize];
	  [mAtt removeObjectForKey: NSFileCreationDate];
	  [mAtt removeObjectForKey: NSFileModificationDate];
	  [mAtt removeObjectForKey: NSFileReferenceCount];
	  [mAtt removeObjectForKey: NSFileSystemNumber];
	  [mAtt removeObjectForKey: NSFileSystemFileNumber];
	  [mAtt removeObjectForKey: NSFileDeviceIdentifier];
	  [mAtt removeObjectForKey: NSFileType];
	  if ([mgr changeFileAttributes: mAtt atPath: path] == NO)
	    {
	      NSWarnMLog(@"Unable to correctly set attributes for '%@' to %@",
		path, mAtt);
	    }
	}
#ifdef HAVE_GETEUID
      else if (geteuid() == 0 && [@"root" isEqualToString: NSUserName()] == NO)
	{
	  att = [NSDictionary dictionaryWithObjectsAndKeys:
	    NSFileOwnerAccountName, NSUserName(), nil];
	  if ([mgr changeFileAttributes: att atPath: path] == NO)
	    {
	      NSWarnMLog(@"Unable to correctly set ownership for '%@'", path);
	    }
	}
#endif
    }

  /* success: */
  return YES;

  /* Just in case the failure action needs to be changed. */
failure:
  /*
   * Attempt to tidy up by removing temporary file on failure.
   */
  if (useAuxiliaryFile)
    {
      unlink(thePath);
    }
  return NO;
#endif
}

- (BOOL) writeToURL: (NSURL *)url
            options: (NSUInteger)writeOptionsMask
              error: (NSError **)errorPtr
{
  if ([url isFileURL] == YES)
    {
      return [self writeToFile: [url path]
		       options: writeOptionsMask
			 error: errorPtr];
    }
  else
    {
      return [url setResourceData: self];
    }
  return NO;
}

@end

/**
 *  Provides some shared-memory extensions to [NSData].
 */
@implementation	NSData (GNUstepExtensions)

/**
 *  New instance with given shared memory ID.
 */
+ (id) dataWithShmID: (int)anID length: (NSUInteger)length
{
#ifdef	HAVE_SHMCTL
  NSDataShared	*d;

  d = [NSDataShared allocWithZone: NSDefaultMallocZone()];
  d = [d initWithShmID: anID length: length];
  return AUTORELEASE(d);
#else
  NSLog(@"[NSData -dataWithSmdID:length:] no shared memory support");
  return nil;
#endif
}

/**
 *  New instance with given bytes in shared memory.
 */
+ (id) dataWithSharedBytes: (const void*)bytes length: (NSUInteger)length
{
  NSData	*d;

#ifdef	HAVE_SHMCTL
  d = [NSDataShared allocWithZone: NSDefaultMallocZone()];
  d = [d initWithBytes: bytes length: length];
#else
  d = [dataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithBytes: bytes length: length];
#endif
  return AUTORELEASE(d);
}

+ (id) dataWithStaticBytes: (const void*)bytes length: (NSUInteger)length
{
  NSDataStatic	*d;

  d = [dataStatic allocWithZone: NSDefaultMallocZone()];
  d = [d initWithBytesNoCopy: (void*)bytes length: length freeWhenDone: NO];
  return AUTORELEASE(d);
}

- (void) deserializeTypeTag: (unsigned char*)tag
		andCrossRef: (unsigned int*)ref
		   atCursor: (unsigned int*)cursor
{
  [self deserializeDataAt: (void*)tag
	       ofObjCType: @encode(uint8_t)
		 atCursor: cursor
		  context: nil];
  if (*tag & _GSC_MAYX)
    {
      switch (*tag & _GSC_SIZE)
	{
	  case _GSC_X_0:
	    {
	      return;
	    }
	  case _GSC_X_1:
	    {
	      uint8_t	x;

	      [self deserializeDataAt: (void*)&x
			   ofObjCType: @encode(uint8_t)
			     atCursor: cursor
			      context: nil];
	      *ref = (unsigned int)x;
	      return;
	    }
	  case _GSC_X_2:
	    {
	      uint16_t	x;

	      [self deserializeDataAt: (void*)&x
			   ofObjCType: @encode(uint16_t)
			     atCursor: cursor
			      context: nil];
	      *ref = (unsigned int)x;
	      return;
	    }
	  default:
	    {
	      uint32_t	x;

	      [self deserializeDataAt: (void*)&x
			   ofObjCType: @encode(uint32_t)
			     atCursor: cursor
			      context: nil];
	      *ref = (unsigned int)x;
	      return;
	    }
	}
    }
}

@end


/**
 *  Mutable version of [NSData].  Methods are provided for appending and
 *  replacing bytes in the buffer, which will be grown as needed.
 */
@implementation NSMutableData
+ (id) allocWithZone: (NSZone*)z
{
  if (self == NSMutableDataAbstract)
    {
      return NSAllocateObject(mutableDataMalloc, 0, z);
    }
  else
    {
      return NSAllocateObject(self, 0, z);
    }
}

- (NSUInteger) sizeOfContentExcluding: (NSHashTable*)exclude
{
  return [self capacity];
}

+ (id) data
{
  NSMutableData	*d;

  d = [mutableDataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithCapacity: 0];
  return AUTORELEASE(d);
}

+ (id) dataWithBytes: (const void*)bytes
	      length: (NSUInteger)length
{
  NSData	*d;

  d = [mutableDataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithBytes: bytes length: length];
  return AUTORELEASE(d);
}

+ (id) dataWithBytesNoCopy: (void*)bytes
		    length: (NSUInteger)length
{
  NSData	*d;

  d = [mutableDataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithBytesNoCopy: bytes length: length];
  return AUTORELEASE(d);
}

+ (id) dataWithBytesNoCopy: (void*)aBuffer
		    length: (NSUInteger)bufferSize
	      freeWhenDone: (BOOL)shouldFree
{
  NSData	*d;

  d = [mutableDataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithBytesNoCopy: aBuffer
		      length: bufferSize
		freeWhenDone: shouldFree];
  return AUTORELEASE(d);
}

/**
 *  New instance with buffer of given numBytes with length of valid data set
 *  to zero.  Note that capacity will be automatically increased as necessary.
 */
+ (id) dataWithCapacity: (NSUInteger)numBytes
{
  NSMutableData	*d;

  d = [mutableDataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithCapacity: numBytes];
  return AUTORELEASE(d);
}

+ (id) dataWithContentsOfFile: (NSString*)path
{
  NSMutableData	*d;

  d = [mutableDataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithContentsOfFile: path];
  return AUTORELEASE(d);
}

+ (id) dataWithContentsOfMappedFile: (NSString*)path
{
  NSMutableData	*d;

  d = [mutableDataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithContentsOfMappedFile: path];
  return AUTORELEASE(d);
}

+ (id) dataWithContentsOfURL: (NSURL*)url
{
  NSMutableData	*d;
  NSData	*data;

  d = [mutableDataMalloc allocWithZone: NSDefaultMallocZone()];
  data = [url resourceDataUsingCache: YES];
  d = [d initWithBytes: [data bytes] length: [data length]];
  return AUTORELEASE(d);
}

+ (id) dataWithData: (NSData*)data
{
  NSMutableData	*d;

  d = [mutableDataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithBytes: [data bytes] length: [data length]];
  return AUTORELEASE(d);
}

/**
 *  New instance with buffer of capacity and valid data size equal to given
 *  length in bytes.  The buffer contents are set to zero.  The length of
 *  valid data is set to zero.  Note that buffer will be automatically
 *  increased as necessary.
 */
+ (id) dataWithLength: (NSUInteger)length
{
  NSMutableData	*d;

  d = [mutableDataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithLength: length];
  return AUTORELEASE(d);
}

+ (id) new
{
  NSMutableData	*d;

  d = [mutableDataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithCapacity: 0];
  return d;
}

- (const void*) bytes
{
  return [self mutableBytes];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  NSUInteger	length = [self length];
  void		*bytes = [self mutableBytes];

  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeBytes: bytes
                   length: length
                   forKey: @"NS.data"];
    }
  else
    {
      [aCoder encodeValueOfObjCType: @encode(NSUInteger)
			         at: &length];
      if (length)
        {
          [aCoder encodeArrayOfObjCType: @encode(unsigned char)
				  count: length
				     at: bytes];
        }
    }
}

/**
 *  Initialize with buffer capable of holding size bytes.  The length of valid
 *  data is initially set to zero.
 *  <init/>
 */
- (id) initWithCapacity: (NSUInteger)capacity
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      const uint8_t	*data;
      NSUInteger	l;


      data = [aCoder decodeBytesForKey: @"NS.data"
                        returnedLength: &l];
      self = [self initWithBytes: data length: l];
    }
  else
    {
      NSUInteger  l;

      [aCoder decodeValueOfObjCType: @encode(NSUInteger) at: &l];
      if (l)
        {
          void *b;

          b = NSZoneMalloc([self zone], l);
          if (b == 0)
            {
              NSLog(@"[NSDataMalloc -initWithCoder:] unable to get %u bytes",
                (unsigned int)l);
              DESTROY(self);
              return nil;
            }
          [aCoder decodeArrayOfObjCType: @encode(unsigned char) count: l at: b];
          self = [self initWithBytesNoCopy: b length: l];
        }
      else
        {
          self = [self initWithBytesNoCopy: 0 length: 0];
        }
    }
  return self;
}

/**
 *  Initialize with buffer of capacity equal to length, and with the length
 *  of valid data set to length.  Data is set to zero.
 */
- (id) initWithLength: (NSUInteger)length
{
  [self subclassResponsibility: _cmd];
  return nil;
}

// Adjusting Capacity
/**
 *  Increases buffer length by given number of bytes, filling the new space
 *  with zeros.
 */
- (void) increaseLengthBy: (NSUInteger)extraLength
{
  [self setLength: [self length]+extraLength];
}

/**
 * <p>
 *   Sets the length of the NSMutableData object.
 *   If the length is increased, the newly allocated data area
 *   is filled with zero bytes.
 * </p>
 * <p>
 *   This is a 'primitive' method ... you need to implement it
 *   if you write a subclass of NSMutableData.
 * </p>
 */
- (void) setLength: (NSUInteger)size
{
  [self subclassResponsibility: _cmd];
}

/**
 * <p>
 *   Returns a pointer to the data storage of the receiver.<br />
 *   Modifications to the memory pointed to by this pointer will
 *   change the contents of the object.  It is important that
 *   your code should not try to modify the memory beyond the
 *   number of bytes given by the <code>-length</code> method.
 * </p>
 * <p>
 *   NB. if the object is released, or any method that changes its
 *   size or content is called, then the pointer previously returned
 *   by this method may cease to be valid.
 * </p>
 * <p>
 *   This is a 'primitive' method ... you need to implement it
 *   if you write a subclass of NSMutableData.
 * </p>
 */
- (void*) mutableBytes
{
  [self subclassResponsibility: _cmd];
  return 0;
}

// Appending Data

/**
 *  Appends bufferSize bytes from aBuffer to data, increasing capacity if
 *  necessary.
 */
- (void) appendBytes: (const void*)aBuffer
	      length: (NSUInteger)bufferSize
{
  NSUInteger	oldLength = [self length];
  void		*buffer;

  [self setLength: oldLength + bufferSize];
  buffer = [self mutableBytes];
  if (0 == buffer)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"missing bytes in appendBytes:length:"];
    }
  memcpy(buffer + oldLength, aBuffer, bufferSize);
}

/**
 *  Copies and appends data from other to data, increasing capacity if
 *  necessary.
 */
- (void) appendData: (NSData*)other
{
  [self appendBytes: [other bytes] length: [other length]];
}

/**
 * Replaces the bytes of data in the specified range with a
 * copy of the new bytes supplied.<br />
 * If the location of the range specified lies beyond the end
 * of the data (<code>[self length] &lt; range.location</code>)
 * then a range exception is raised.<br />
 * Otherwise, if the range specified extends beyond the end
 * of the data, then the size of the data is increased to
 * accommodate the new bytes.<br />
 */
- (void) replaceBytesInRange: (NSRange)aRange
		   withBytes: (const void*)bytes
{
  NSUInteger	size = [self length];
  NSUInteger	need = NSMaxRange(aRange);

  if (aRange.location > size)
    {
      [NSException raise: NSRangeException
                  format: @"location bad in %@", NSStringFromSelector(_cmd)];
    }
  if (aRange.length > 0)
    {
      void	*buf;

      if (need > size)
	{
	  [self setLength: need];
	}
      buf = [self mutableBytes];
      if (0 == buf)
	{
	  [NSException raise: NSInternalInconsistencyException
            format: @"missing bytes in %@", NSStringFromSelector(_cmd)];
	}
      memmove(buf + aRange.location, bytes, aRange.length);
    }
}

/**
 * Replace the content of the receiver which lies in aRange with
 * the specified length of data from the buffer pointed to by bytes.<br />
 * The size of the receiver is adjusted to allow for the change.
 */
- (void) replaceBytesInRange: (NSRange)aRange
		   withBytes: (const void*)bytes
		      length: (NSUInteger)length
{
  NSUInteger	size = [self length];
  NSUInteger	end = NSMaxRange(aRange);
  NSInteger    	shift = length - aRange.length;
  NSUInteger	need = size + shift;
  void		*buf;

  if (aRange.location > size)
    {
      [NSException raise: NSRangeException
                  format: @"location bad in %@", NSStringFromSelector(_cmd)];
    }
  if (0 == length && 0 == shift)
    {
      return;   // Nothing to do
    }
  if (need > size)
    {
      [self setLength: need];
    }
  buf = [self mutableBytes];
  if (0 == buf)
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"missing bytes in %@", NSStringFromSelector(_cmd)];
    }
  if (shift < 0)
    {
      if (length > 0)
	{
	  // Copy bytes into place.
	  memmove(buf + aRange.location, bytes, length);
	}
      // Fill gap
      memmove(buf + end + shift, buf + end, size - end);
    }
  else
    {
      if (shift > 0)
	{
	  // Open space
	  memmove(buf + end + shift, buf + end, size - end);
	}
      if (length > 0)
	{
	  // Copy bytes into place.
	  memmove(buf + aRange.location, bytes, length);
	}
    }
  if (need < size)
    {
      [self setLength: need];
    }
}

/**
 *  Set bytes in aRange to 0.
 */
- (void) resetBytesInRange: (NSRange)aRange
{
  NSUInteger	size = [self length];
  void		*bytes = [self mutableBytes];

  GS_RANGE_CHECK(aRange, size);
  if (0 == bytes)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"missing bytes in resetBytesInRange:"];
    }
  memset((char*)bytes + aRange.location, 0, aRange.length);
}

/**
 *  Replaces contents of buffer with contents of data's buffer, increasing
 *  or shrinking capacity to match.
 */
- (void) setData: (NSData*)data
{
  NSRange	r = NSMakeRange(0, [data length]);

  [self setCapacity: r.length];
  [self replaceBytesInRange: r withBytes: [data bytes]];
}

// Serializing Data

/**
 *  Does not act as the name suggests.  Instead, serializes length itself
 *  as an int into buffer.
 */
- (void) serializeAlignedBytesLength: (unsigned int)length
{
  [self serializeInt: length];
}

- (void) serializeDataAt: (const void*)data
	      ofObjCType: (const char*)type
		 context: (id <NSObjCTypeSerializationCallBack>)callback
{
  if (!data || !type)
    return;

  switch (*type)
    {
      case _C_ID:
	[callback serializeObjectAt: (id*)data
			 ofObjCType: type
			   intoData: self];
	return;

      case _C_CHARPTR:
	{
	  uint32_t	len;
	  uint32_t	ni;

	  if (!*(void**)data)
	    {
	      ni = (uint32_t)-1;
	      ni = GSSwapHostI32ToBig(ni);
	      [self appendBytes: (void*)&ni length: sizeof(ni)];
	      return;
	    }
	  len = (uint32_t)strlen(*(void**)data);
	  ni = GSSwapHostI32ToBig(len);
	  [self appendBytes: (void*)&ni length: sizeof(ni)];
	  [self appendBytes: *(void**)data length: len];
	  return;
	}
      case _C_ARY_B:
	{
	  unsigned	offset = 0;
	  unsigned	size;
	  unsigned	count = atoi(++type);
	  unsigned	i;

	  while (isdigit(*type))
	    {
	      type++;
	    }
	  size = objc_sizeof_type(type);

	  for (i = 0; i < count; i++)
	    {
	      [self serializeDataAt: (char*)data + offset
			 ofObjCType: type
			    context: callback];
	      offset += size;
	    }
	  return;
        }
      case _C_STRUCT_B:
	{
	  struct objc_struct_layout layout;

	  objc_layout_structure (type, &layout);
	  while (objc_layout_structure_next_member (&layout))
	    {
	      unsigned		offset;
	      unsigned		align;
	      const char	*ftype;

	      objc_layout_structure_get_info (&layout, &offset, &align, &ftype);

	      [self serializeDataAt: ((char*)data) + offset
			 ofObjCType: ftype
			    context: callback];
	    }
	  return;
        }
      case _C_PTR:
	[self serializeDataAt: *(char**)data
		   ofObjCType: ++type
		      context: callback];
	return;
      case _C_CHR:
      case _C_UCHR:
	[self appendBytes: data length: sizeof(unsigned char)];
	return;
      case _C_SHT:
      case _C_USHT:
	{
	  unsigned short ns = NSSwapHostShortToBig(*(unsigned short*)data);
	  [self appendBytes: &ns length: sizeof(unsigned short)];
	  return;
	}
      case _C_INT:
      case _C_UINT:
	{
	  unsigned ni = NSSwapHostIntToBig(*(unsigned int*)data);
	  [self appendBytes: &ni length: sizeof(unsigned)];
	  return;
	}
      case _C_LNG:
      case _C_ULNG:
	{
	  unsigned long nl = NSSwapHostLongToBig(*(unsigned long*)data);
	  [self appendBytes: &nl length: sizeof(unsigned long)];
	  return;
	}
      case _C_LNG_LNG:
      case _C_ULNG_LNG:
	{
	  unsigned long long nl;

	  nl = NSSwapHostLongLongToBig(*(unsigned long long*)data);
	  [self appendBytes: &nl length: sizeof(unsigned long long)];
	  return;
	}
      case _C_FLT:
	{
	  NSSwappedFloat nf = NSSwapHostFloatToBig(*(float*)data);

	  [self appendBytes: &nf length: sizeof(NSSwappedFloat)];
	  return;
	}
      case _C_DBL:
	{
	  NSSwappedDouble nd = NSSwapHostDoubleToBig(*(double*)data);

	  [self appendBytes: &nd length: sizeof(NSSwappedDouble)];
	  return;
	}
#if __GNUC__ > 2 && defined(_C_BOOL)
      case _C_BOOL:
	[self appendBytes: data length: sizeof(_Bool)];
	return;
#endif
      case _C_CLASS:
	{
	  const char  *name = *(Class*)data?class_getName(*(Class*)data):"";
	  uint16_t	ln = (uint16_t)strlen(name);
	  uint16_t	ni;

	  ni = GSSwapHostI16ToBig(ln);
	  [self appendBytes: &ni length: sizeof(ni)];
	  if (ln)
	    {
	      [self appendBytes: name length: ln];
	    }
	  return;
	}
      case _C_SEL:
	{
	  const char  *name = *(SEL*)data?sel_getName(*(SEL*)data):"";
	  uint16_t	ln = (name == 0) ? 0 : (uint16_t)strlen(name);
	  const char  *types = *(SEL*)data?GSTypesFromSelector(*(SEL*)data):"";
	  uint16_t	lt = (types == 0) ? 0 : (uint16_t)strlen(types);
	  uint16_t	ni;

	  ni = GSSwapHostI16ToBig(ln);
	  [self appendBytes: &ni length: sizeof(ni)];
	  ni = GSSwapHostI16ToBig(lt);
	  [self appendBytes: &ni length: sizeof(ni)];
	  if (ln)
	    {
	      [self appendBytes: name length: ln];
	    }
	  if (lt)
	    {
	      [self appendBytes: types length: lt];
	    }
	  return;
	}
      default:
	[NSException raise: NSGenericException
		    format: @"Unknown type to serialize - '%s'", type];
    }
}

/**
 * Serialize an int into this object's data buffer, swapping it to network
 * (big-endian) byte order first.
 */
- (void) serializeInt: (int)value
{
  unsigned ni = NSSwapHostIntToBig(value);
  [self appendBytes: &ni length: sizeof(unsigned)];
}

/**
 * Serialize an int into this object's data buffer at index (replacing
 * anything there currently), swapping it to network (big-endian) byte order
 * first.
 */
- (void) serializeInt: (int)value atIndex: (unsigned int)index
{
  unsigned ni = NSSwapHostIntToBig(value);
  NSRange range = { index, sizeof(int) };

  [self replaceBytesInRange: range withBytes: &ni];
}

/**
 * Serialize one or more ints into this object's data buffer, swapping them to
 * network (big-endian) byte order first.
 */
- (void) serializeInts: (int*)intBuffer
		 count: (unsigned int)numInts
{
  unsigned	i;
  SEL		sel = @selector(serializeInt:);
  IMP		imp = [self methodForSelector: sel];

  for (i = 0; i < numInts; i++)
    {
      (*imp)(self, sel, intBuffer[i]);
    }
}

/**
 * Serialize one or more ints into this object's data buffer at index
 * (replacing anything there currently), swapping them to network (big-endian)
 * byte order first.
 */
- (void) serializeInts: (int*)intBuffer
		 count: (unsigned int)numInts
	       atIndex: (unsigned int)index
{
  unsigned	i;
  SEL		sel = @selector(serializeInt:atIndex:);
  IMP		imp = [self methodForSelector: sel];

  for (i = 0; i < numInts; i++)
    {
      (*imp)(self, sel, intBuffer[i], index++);
    }
}

@end


/**
 *  Provides some additional methods to [NSData].
 */
@implementation	NSMutableData (GNUstepExtensions)

/**
 *  New instance with given shared memory ID.
 */
+ (id) dataWithShmID: (int)anID length: (NSUInteger)length
{
#ifdef	HAVE_SHMCTL
  NSMutableDataShared	*d;

  d = [NSMutableDataShared allocWithZone: NSDefaultMallocZone()];
  d = [d initWithShmID: anID length: length];
  return AUTORELEASE(d);
#else
  NSLog(@"[NSMutableData -dataWithSmdID:length:] no shared memory support");
  return nil;
#endif
}

/**
 *  New instance with given bytes in shared memory.
 */
+ (id) dataWithSharedBytes: (const void*)bytes length: (NSUInteger)length
{
  NSData	*d;

#ifdef	HAVE_SHMCTL
  d = [NSMutableDataShared allocWithZone: NSDefaultMallocZone()];
  d = [d initWithBytes: bytes length: length];
#else
  d = [mutableDataMalloc allocWithZone: NSDefaultMallocZone()];
  d = [d initWithBytes: bytes length: length];
#endif
  return AUTORELEASE(d);
}

/**
 *  Returns current capacity of data buffer.
 */
- (NSUInteger) capacity
{
  [self subclassResponsibility: _cmd];
  return 0;
}

/**
 *  Sets current capacity of data buffer.  Unlike [-setLength:], this will
 *  shrink the buffer if requested.
 */
- (id) setCapacity: (NSUInteger)newCapacity
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/**
 *  Return shared memory ID, if using one, else -1.
 */
- (int) shmID
{
  return -1;
}

- (void) serializeTypeTag: (unsigned char)tag
{
  [self serializeDataAt: (void*)&tag
	     ofObjCType: @encode(unsigned char)
		context: nil];
}

- (void) serializeTypeTag: (unsigned char)tag
	      andCrossRef: (unsigned int)xref
{
  if (xref <= 0xff)
    {
      uint8_t	x = (uint8_t)xref;

      tag = (tag & ~_GSC_SIZE) | _GSC_X_1;
      [self serializeDataAt: (void*)&tag
		 ofObjCType: @encode(unsigned char)
		    context: nil];
      [self serializeDataAt: (void*)&x
		 ofObjCType: @encode(uint8_t)
		    context: nil];
    }
  else if (xref <= 0xffff)
    {
      uint16_t	x = (uint16_t)xref;

      tag = (tag & ~_GSC_SIZE) | _GSC_X_2;
      [self serializeDataAt: (void*)&tag
		 ofObjCType: @encode(unsigned char)
		    context: nil];
      [self serializeDataAt: (void*)&x
		 ofObjCType: @encode(uint16_t)
		    context: nil];
    }
  else
    {
      uint32_t	x = (uint32_t)xref;

      tag = (tag & ~_GSC_SIZE) | _GSC_X_4;
      [self serializeDataAt: (void*)&tag
		 ofObjCType: @encode(unsigned char)
		    context: nil];
      [self serializeDataAt: (void*)&x
		 ofObjCType: @encode(uint32_t)
		    context: nil];
    }
}
@end


/*
 *	This is the top of the hierarchy of concrete implementations.
 *	As such, it contains efficient implementations of most methods.
 */
@implementation	NSDataStatic

+ (id) allocWithZone: (NSZone*)z
{
  return NSAllocateObject(self, 0, z);
}

/*	Creation and Destruction of objects.	*/

- (id) copyWithZone: (NSZone*)z
{
  return RETAIN(self);
}

- (id) mutableCopyWithZone: (NSZone*)z
{
  return [[mutableDataMalloc allocWithZone: z]
    initWithBytes: bytes length: length];
}

- (void) dealloc
{
  bytes = 0;
  length = 0;
  [super dealloc];
}

- (id) initWithBytesNoCopy: (void*)aBuffer
		    length: (NSUInteger)bufferSize
	      freeWhenDone: (BOOL)shouldFree
{
  if (aBuffer == 0 && bufferSize > 0)
    {
      [NSException raise: NSInvalidArgumentException
	format: @"[%@-initWithBytesNoCopy:length:freeWhenDone:] called with "
	@"length but null bytes", NSStringFromClass([self class])];
    }
  bytes = aBuffer;
  length = bufferSize;
  return self;
}

- (Class) classForCoder
{
  return NSDataAbstract;
}

/* Basic methods	*/

- (const void*) bytes
{
  return bytes;
}

- (void) getBytes: (void*)buffer
	    range: (NSRange)aRange
{
  GS_RANGE_CHECK(aRange, length);
  memcpy(buffer, bytes + aRange.location, aRange.length);
}

- (NSUInteger) length
{
  return length;
}

static inline void
getBytes(void* dst, void* src, unsigned len, unsigned limit, unsigned *pos)
{
  if (*pos > limit || len > limit || len+*pos > limit)
    {
      [NSException raise: NSRangeException
		  format: @"Range: (%u, %u) Size: %d",
			*pos, len, limit];
    }
  memcpy(dst, src + *pos, len);
  *pos += len;
}

- (void) deserializeDataAt: (void*)data
	        ofObjCType: (const char*)type
		  atCursor: (unsigned int*)cursor
		   context: (id <NSObjCTypeSerializationCallBack>)callback
{
  if (data == 0 || type == 0)
    {
      if (data == 0)
	{
	  NSLog(@"attempt to deserialize to a null pointer");
	}
      if (type == 0)
	{
            NSLog(@"attempt to deserialize with a null type encoding");
	}
      return;
    }

  switch (*type)
    {
      case _C_ID:
	{
	  [callback deserializeObjectAt: data
			     ofObjCType: type
			       fromData: self
			       atCursor: cursor];
	  return;
	}
      case _C_CHARPTR:
	{
	  int32_t len;

	  [self deserializeBytes: &len
			  length: sizeof(len)
			atCursor: cursor];
	  len = GSSwapBigI32ToHost(len);
	  if (len == -1)
	    {
	      *(const char**)data = 0;
	      return;
	    }
	  else
	    {
	      *(char**)data = (char*)NSZoneMalloc(NSDefaultMallocZone(), len+1);
	      if (*(char**)data == 0)
	        {
		  [NSException raise: NSMallocException
			      format: @"out of memory to deserialize bytes"];
		}
	    }
	  getBytes(*(void**)data, bytes, len, length, cursor);
	  (*(char**)data)[len] = '\0';
	  return;
	}
      case _C_ARY_B:
	{
	  unsigned	offset = 0;
	  unsigned	size;
	  unsigned	count = atoi(++type);
	  unsigned	i;

	  while (isdigit(*type))
	    {
	      type++;
	    }
	  size = objc_sizeof_type(type);

	  for (i = 0; i < count; i++)
	    {
	      [self deserializeDataAt: (char*)data + offset
			   ofObjCType: type
			     atCursor: cursor
			      context: callback];
	      offset += size;
	    }
	  return;
	}
      case _C_STRUCT_B:
	{
	  struct objc_struct_layout layout;

	  objc_layout_structure (type, &layout);
	  while (objc_layout_structure_next_member (&layout))
	    {
	      unsigned		offset;
	      unsigned		align;
	      const char	*ftype;

	      objc_layout_structure_get_info (&layout, &offset, &align, &ftype);

	      [self deserializeDataAt: ((char*)data) + offset
			   ofObjCType: ftype
			     atCursor: cursor
			      context: callback];
	    }
	  return;
        }
      case _C_PTR:
	{
	  unsigned	len = objc_sizeof_type(++type);

	  *(char**)data = (char*)NSZoneMalloc(NSDefaultMallocZone(), len);
	  if (*(char**)data == 0)
	    {
	      [NSException raise: NSMallocException
			  format: @"out of memory to deserialize bytes"];
	    }
	  [self deserializeDataAt: *(char**)data
		       ofObjCType: type
			 atCursor: cursor
			  context: callback];
	  return;
        }
      case _C_CHR:
      case _C_UCHR:
	{
	  getBytes(data, bytes, sizeof(unsigned char), length, cursor);
	  return;
	}
      case _C_SHT:
      case _C_USHT:
	{
	  unsigned short ns;

	  getBytes((void*)&ns, bytes, sizeof(ns), length, cursor);
	  *(unsigned short*)data = NSSwapBigShortToHost(ns);
	  return;
	}
      case _C_INT:
      case _C_UINT:
	{
	  unsigned ni;

	  getBytes((void*)&ni, bytes, sizeof(ni), length, cursor);
	  *(unsigned*)data = NSSwapBigIntToHost(ni);
	  return;
	}
      case _C_LNG:
      case _C_ULNG:
	{
	  unsigned long nl;

	  getBytes((void*)&nl, bytes, sizeof(nl), length, cursor);
	  *(unsigned long*)data = NSSwapBigLongToHost(nl);
	  return;
	}
      case _C_LNG_LNG:
      case _C_ULNG_LNG:
	{
	  unsigned long long nl;

	  getBytes((void*)&nl, bytes, sizeof(nl), length, cursor);
	  *(unsigned long long*)data = NSSwapBigLongLongToHost(nl);
	  return;
	}
      case _C_FLT:
	{
	  NSSwappedFloat nf;

	  getBytes((void*)&nf, bytes, sizeof(nf), length, cursor);
	  *(float*)data = NSSwapBigFloatToHost(nf);
	  return;
	}
      case _C_DBL:
	{
	  NSSwappedDouble nd;

	  getBytes((void*)&nd, bytes, sizeof(nd), length, cursor);
	  *(double*)data = NSSwapBigDoubleToHost(nd);
	  return;
	}
#if __GNUC__ > 2 && defined(_C_BOOL)
      case _C_BOOL:
	{
	  getBytes(data, bytes, sizeof(_Bool), length, cursor);
	  return;
	}
#endif
      case _C_CLASS:
	{
	  uint16_t	ni;

	  getBytes((void*)&ni, bytes, sizeof(ni), length, cursor);
	  ni = GSSwapBigI16ToHost(ni);
	  if (ni == 0)
	    {
	      *(Class*)data = 0;
	    }
	  else
	    {
	      char	name[ni+1];
	      Class	c;

	      getBytes((void*)name, bytes, ni, length, cursor);
	      name[ni] = '\0';
	      c = objc_lookUpClass(name);
	      if (c == 0)
		{
		  NSLog(@"[%s %s] can't find class - %s",
		    class_getName([self class]),
		    sel_getName(_cmd), name);
		}
	      *(Class*)data = c;
	    }
	  return;
	}
      case _C_SEL:
	{
	  uint16_t	ln;
	  uint16_t	lt;

	  getBytes((void*)&ln, bytes, sizeof(ln), length, cursor);
	  ln = GSSwapBigI16ToHost(ln);
	  getBytes((void*)&lt, bytes, sizeof(lt), length, cursor);
	  lt = GSSwapBigI16ToHost(lt);
	  if (ln == 0)
	    {
	      *(SEL*)data = 0;
	    }
	  else
	    {
	      char	name[ln+1];
	      char	types[lt+1];
	      SEL	sel;

	      getBytes((void*)name, bytes, ln, length, cursor);
	      name[ln] = '\0';
	      getBytes((void*)types, bytes, lt, length, cursor);
	      types[lt] = '\0';

	      if (lt)
		{
		  sel = GSSelectorFromNameAndTypes(name, types);
		}
	      else
		{
		  sel = sel_registerName(name);
		}
	      if (sel == 0)
		{
		      [NSException raise: NSInternalInconsistencyException
				  format: @"can't make sel with name '%s' "
					      @"and types '%s'", name, types];
		}
	      *(SEL*)data = sel;
	    }
	  return;
	}
      default:
	[NSException raise: NSGenericException
		    format: @"Unknown type to deserialize - '%s'", type];
    }
}

- (void) deserializeTypeTag: (unsigned char*)tag
		andCrossRef: (unsigned int*)ref
		   atCursor: (unsigned int*)cursor
{
  if (*cursor >= length)
    {
      [NSException raise: NSRangeException
		  format: @"Range: (%u, 1) Size: %"PRIuPTR, *cursor, length];
    }
  *tag = *((unsigned char*)bytes + (*cursor)++);
  if (*tag & _GSC_MAYX)
    {
      switch (*tag & _GSC_SIZE)
	{
	  case _GSC_X_0:
	    {
	      return;
	    }
	  case _GSC_X_1:
	    {
	      if (*cursor >= length)
		{
		  [NSException raise: NSRangeException
			      format: @"Range: (%u, 1) Size: %"PRIuPTR,
			  *cursor, length];
		}
	      *ref = (unsigned int)*((unsigned char*)bytes + (*cursor)++);
	      return;
	    }
	  case _GSC_X_2:
	    {
	      uint16_t	x;

	      if (*cursor >= length-1)
		{
		  [NSException raise: NSRangeException
			      format: @"Range: (%u, 1) Size: %"PRIuPTR,
			  *cursor, length];
		}
#if NEED_WORD_ALIGNMENT
	      if ((*cursor % __alignof__(uint16_t)) != 0)
		memcpy(&x, (bytes + *cursor), 2);
	      else
#endif
	      x = *(uint16_t*)(bytes + *cursor);
	      *cursor += 2;
	      *ref = (unsigned int)GSSwapBigI16ToHost(x);
	      return;
	    }
	  default:
	    {
	      uint32_t	x;

	      if (*cursor >= length-3)
		{
		  [NSException raise: NSRangeException
			      format: @"Range: (%u, 1) Size: %"PRIuPTR,
			  *cursor, length];
		}
#if NEED_WORD_ALIGNMENT
	      if ((*cursor % __alignof__(uint32_t)) != 0)
		memcpy(&x, (bytes + *cursor), 4);
	      else
#endif
	      x = *(uint32_t*)(bytes + *cursor);
	      *cursor += 4;
	      *ref = (unsigned int)GSSwapBigI32ToHost(x);
	      return;
	    }
	}
    }
}

@end


@implementation NSDataEmpty
- (void) dealloc
{
  GSNOSUPERDEALLOC;
}
@end


@implementation	NSDataMalloc

- (id) copyWithZone: (NSZone*)z
{
  if (NSShouldRetainWithZone(self, z))
    return RETAIN(self);
  else
    return [[dataMalloc allocWithZone: z]
      initWithBytes: bytes length: length];
}

- (void) dealloc
{
  if (bytes != 0)
    {
      NSZoneFree(NSZoneFromPointer(bytes), bytes);
      bytes = 0;
    }
  [super dealloc];
}

- (id) initWithBytesNoCopy: (void*)aBuffer
		    length: (NSUInteger)bufferSize
	      freeWhenDone: (BOOL)shouldFree
{
  if (aBuffer == 0 && bufferSize > 0)
    {
      [NSException raise: NSInvalidArgumentException
	format: @"[%@-initWithBytesNoCopy:length:freeWhenDone:] called with "
	@"length but null bytes", NSStringFromClass([self class])];
    }
  if (shouldFree == NO)
    {
      GSClassSwizzle(self, dataStatic);
    }
  bytes = aBuffer;
  length = bufferSize;
  return self;
}

- (instancetype) initWithBytesNoCopy: (void*)buf
                              length: (NSUInteger)len
                         deallocator: (GSDataDeallocatorBlock)deallocBlock
{
  if (buf == NULL && len > 0)
    {
      [self release];
      [NSException raise: NSInvalidArgumentException
        format: @"[%@-initWithBytesNoCopy:length:deallocator:] called with "
          @"length but NULL bytes", NSStringFromClass([self class])];
    }
  else if (NULL == deallocBlock)
    {
      // For a nil deallocator we can just swizzle into a static data object
      GSClassSwizzle(self, dataStatic);
      bytes = buf;
      length = len;
      return self;
    }

  GSClassSwizzle(self, dataBlock);
  ASSIGN(deallocator, (id)deallocBlock);
  return self;
}

@end

@implementation NSDataWithDeallocatorBlock
- (instancetype) initWithBytesNoCopy: (void*)buf
                              length: (NSUInteger)len
                         deallocator: (GSDataDeallocatorBlock)deallocBlock
{
  if (buf == NULL && len > 0)
    {
      [self release];
      [NSException raise: NSInvalidArgumentException
        format: @"[%@-initWithBytesNoCopy:length:deallocator:] called with "
          @"length but NULL bytes", NSStringFromClass([self class])];
    }

  bytes = buf;
  length = len;
  ASSIGN(deallocator, (id)deallocBlock);
  return self;
}

- (void) dealloc
{
  if (deallocator != NULL)
    {
      CALL_BLOCK(((GSDataDeallocatorBlock)deallocator), bytes, length);
      DESTROY(deallocator);
    }
  // Clear out the ivars so that super doesn't double free.
  bytes = NULL;
  length = 0;
  [super dealloc];
}
@end


#ifdef	HAVE_MMAP
@implementation	NSDataMappedFile
+ (id) allocWithZone: (NSZone*)z
{
  return NSAllocateObject([NSDataMappedFile class], 0, z);
}

- (void) dealloc
{
  [self finalize];
  [super dealloc];
}

- (void) finalize
{
  if (bytes != 0)
    {
      munmap(bytes, length);
      bytes = 0;
    }
  [super finalize];
}

/**
 *  Initialize with data pointing to contents of file at path.  Bytes are
 *  only "swapped in" as needed.  File should not be moved or deleted for
 *  the life of this object.
 */
- (id) initWithContentsOfMappedFile: (NSString*)path
{
  off_t		off;
  int		fd;

#if defined(_WIN32)
  const unichar	*thePath = (const unichar*)[path fileSystemRepresentation];
#else
  const char	*thePath = [path fileSystemRepresentation];
#endif

  if (thePath == 0)
    {
      NSWarnMLog(@"Open (%@) attempt failed - bad path", path);
      DESTROY(self);
      return nil;
    }

#if defined(_WIN32)
  fd = _wopen(thePath, _O_RDONLY);
#else
  fd = open(thePath, O_RDONLY);
#endif
  if (fd < 0)
    {
      NSWarnMLog(@"unable to open %@ - %@", path, [NSError _last]);
      DESTROY(self);
      return nil;
    }
  /* Find size of file to be mapped. */
  off = lseek(fd, 0, SEEK_END);
  if (off < 0)
    {
      NSWarnMLog(@"unable to seek to eof %@ - %@", path, [NSError _last]);
      close(fd);
      DESTROY(self);
      return nil;
    }
  length = off;
  /* Position at start of file. */
  if (lseek(fd, 0, SEEK_SET) != 0)
    {
      NSWarnMLog(@"unable to seek to sof %@ - %@", path, [NSError _last]);
      close(fd);
      DESTROY(self);
      return nil;
    }
  bytes = mmap(0, length, PROT_READ, MAP_SHARED, fd, 0);
  if (bytes == MAP_FAILED)
    {
      NSWarnMLog(@"mapping failed for %@ - %@", path, [NSError _last]);
      DESTROY(self);
      self = [dataMalloc allocWithZone: NSDefaultMallocZone()];
      self = [self initWithContentsOfFile: path];
    }
  close(fd);
  return self;
}

@end
#endif	/* HAVE_MMAP	*/

#ifdef	HAVE_SHMCTL
@implementation	NSDataShared
+ (id) allocWithZone: (NSZone*)z
{
  return NSAllocateObject([NSDataShared class], 0, z);
}

- (void) dealloc
{
  if (bytes != 0)
    {
      struct shmid_ds	buf;

      if (shmctl(shmid, IPC_STAT, &buf) < 0)
        NSLog(@"[NSDataShared -dealloc] shared memory control failed - %@",
	  [NSError _last]);
      else if (buf.shm_nattch == 1)
	if (shmctl(shmid, IPC_RMID, &buf) < 0)	/* Mark for deletion. */
          NSLog(@"[NSDataShared -dealloc] shared memory delete failed - %@",
	    [NSError _last]);
      if (shmdt(bytes) < 0)
        NSLog(@"[NSDataShared -dealloc] shared memory detach failed - %@",
	  [NSError _last]);
      bytes = 0;
      length = 0;
      shmid = -1;
    }
  [super dealloc];
}

- (id) initWithBytes: (const void*)aBuffer length: (NSUInteger)bufferSize
{
  shmid = -1;
  if (bufferSize > 0)
    {
      if (aBuffer == 0)
	{
	  [NSException raise: NSInvalidArgumentException
	    format: @"[%@-initWithBytes:length:] called with "
	    @"length but null bytes", NSStringFromClass([self class])];
	}
      shmid = shmget(IPC_PRIVATE, bufferSize, IPC_CREAT|VM_RDONLY);
      if (shmid == -1)			/* Created memory? */
	{
	  NSLog(@"[-initWithBytes:length:] shared mem get failed for %"
	    PRIuPTR" - %@", bufferSize, [NSError _last]);
	  DESTROY(self);
	  self = [dataMalloc allocWithZone: NSDefaultMallocZone()];
	  return [self initWithBytes: aBuffer length: bufferSize];
	}

    bytes = shmat(shmid, 0, 0);
    if (bytes == (void*)-1)
      {
	NSLog(@"[-initWithBytes:length:] shared mem attach failed for %"
	  PRIuPTR" - %@", bufferSize, [NSError _last]);
	bytes = 0;
	DESTROY(self);
	self = [dataMalloc allocWithZone: NSDefaultMallocZone()];
	return [self initWithBytes: aBuffer length: bufferSize];
      }
      length = bufferSize;
    }
  return self;
}

- (id) initWithShmID: (int)anId length: (NSUInteger)bufferSize
{
  struct shmid_ds	buf;

  shmid = anId;
  if (shmctl(shmid, IPC_STAT, &buf) < 0)
    {
      NSLog(@"[NSDataShared -initWithShmID:length:] shared memory "
        @"control failed - %@", [NSError _last]);
      DESTROY(self);	/* Unable to access memory. */
      return nil;
    }
  if (buf.shm_segsz < bufferSize)
    {
      NSLog(@"[NSDataShared -initWithShmID:length:] shared memory "
        @"segment too small");
      DESTROY(self);	/* Memory segment too small. */
      return nil;
    }
  bytes = shmat(shmid, 0, 0);
  if (bytes == (void*)-1)
    {
      NSLog(@"[NSDataShared -initWithShmID:length:] shared memory "
        @"attach failed - %@", [NSError _last]);
      bytes = 0;
      DESTROY(self);	/* Unable to attach to memory. */
      return nil;
    }
  length = bufferSize;
  return self;
}

- (int) shmID
{
  return shmid;
}

@end
#endif	/* HAVE_SHMCTL	*/


@implementation	NSMutableDataMalloc
+ (void) initialize
{
  if (self == [NSMutableDataMalloc class])
    {
      GSObjCAddClassBehavior(self, [NSDataMalloc class]);
    }
}

+ (id) allocWithZone: (NSZone*)z
{
  return NSAllocateObject(mutableDataMalloc, 0, z);
}

- (Class) classForCoder
{
  return NSMutableDataAbstract;
}

- (id) copyWithZone: (NSZone*)z
{
  return [[dataMalloc allocWithZone: z]
    initWithBytes: bytes length: length];
}

- (void) dealloc
{
  if (bytes != 0)
    {
      if (zone != 0)
	{
	  NSZoneFree(zone, bytes);
	}
      bytes = 0;
    }
  [super dealloc];
}

- (id) initWithBytes: (const void*)aBuffer length: (NSUInteger)bufferSize
{
  self = [self initWithCapacity: bufferSize];
  if (self)
    {
      if (bufferSize > 0)
	{
	  if (aBuffer == 0)
	    {
	      [NSException raise: NSInvalidArgumentException
		format: @"[%@-initWithBytes:length:] called with "
		@"length but null bytes", NSStringFromClass([self class])];
	    }
	  length = bufferSize;
	  memcpy(bytes, aBuffer, length);
	}
    }
  return self;
}

- (id) initWithBytesNoCopy: (void*)aBuffer
		    length: (NSUInteger)bufferSize
	      freeWhenDone: (BOOL)shouldFree
{
  if (aBuffer == 0)
    {
      if (bufferSize > 0)
	{
	  [NSException raise: NSInvalidArgumentException format:
	    @"[%@-initWithBytesNoCopy:length:freeWhenDone:] called with "
	    @"length but null bytes", NSStringFromClass([self class])];
	}
      self = [self initWithCapacity: bufferSize];
      [self setLength: 0];
      return self;
    }
  self = [self initWithCapacity: 0];
  if (self)
    {
      if (shouldFree == NO)
	{
	  zone = 0;		// Don't free this memory.
	}
      else
	{
          zone = NSZoneFromPointer(aBuffer);
	}
      bytes = aBuffer;
      length = bufferSize;
      capacity = bufferSize;
      growth = capacity/2;
      if (growth == 0)
	{
	  growth = 1;
	}
    }
  return self;
}

- (instancetype) initWithBytesNoCopy: (void*)buf
                              length: (NSUInteger)len
                         deallocator: (GSDataDeallocatorBlock)deallocBlock;
{
  if (buf == NULL && len > 0)
    {
      [self release];
      [NSException raise: NSInvalidArgumentException
        format: @"[%@-initWithBytesNoCopy:length:deallocator:] called with "
          @"length but NULL bytes", NSStringFromClass([self class])];
    }
  else if (NULL == deallocBlock)
    {
      // Can reuse this class.
      return [self initWithBytesNoCopy: buf
                                length: len
                          freeWhenDone: NO];
    }

  /*
   * Custom deallocator. swizzle to NSMutableDataWithDeallocatorBlock
   */
  GSClassSwizzle(self, mutableDataBlock);
  if (nil == (self = [self initWithBytesNoCopy: buf
                                         length: len
                                   freeWhenDone: NO]))
    {
      return nil;
    }
  ASSIGN(deallocator, (id)deallocBlock);
  return self;
}

// THIS IS THE DESIGNATED INITIALISER
/**
 *  Initialize with buffer capable of holding size bytes.
 *  <init/>
 */
- (id) initWithCapacity: (NSUInteger)size
{
  if (size)
    {
      zone = [self zone];
      bytes = NSZoneMalloc(zone, size);
      if (bytes == 0)
	{
	  NSLog(@"[NSMutableDataMalloc -initWithCapacity:] out of memory "
	    @"for %"PRIuPTR" bytes - %@", size, [NSError _last]);
	  DESTROY(self);
	  return nil;
	}
    }
  capacity = size;
  growth = capacity/2;
  if (growth == 0)
    {
      growth = 1;
    }
  length = 0;

  return self;
}

/**
 *  Initialize with buffer capable of holding size bytes.  Buffer is zeroed
 *  out.
 */
- (id) initWithLength: (NSUInteger)size
{
  self = [self initWithCapacity: size];
  if (self)
    {
      memset(bytes, '\0', size);
      length = size;
    }
  return self;
}

- (id) initWithContentsOfMappedFile: (NSString *)path
{
  return [self initWithContentsOfFile: path];
}

- (void) appendBytes: (const void*)aBuffer
	      length: (NSUInteger)bufferSize
{
  if (bufferSize > 0)
    {
      NSUInteger oldLength = length;
      NSUInteger minimum = length + bufferSize;

      if (aBuffer == 0)
	{
	  [NSException raise: NSInvalidArgumentException
	    format: @"[%@-appendBytes:length:] called with "
	    @"length but null bytes", NSStringFromClass([self class])];
	}
      if (minimum > capacity)
	{
	  [self _grow: minimum];
	}
      memcpy(bytes + oldLength, aBuffer, bufferSize);
      length = minimum;
    }
}

- (NSUInteger) capacity
{
  return capacity;
}

- (void) _grow: (NSUInteger)minimum
{
  if (minimum > capacity)
    {
      NSUInteger nextCapacity = capacity + growth;
      NSUInteger nextGrowth = capacity ? capacity : 1;

      while (nextCapacity < minimum)
	{
	  NSUInteger tmp = nextCapacity + nextGrowth;

	  nextGrowth = nextCapacity;
	  nextCapacity = tmp;
	}
      [self setCapacity: nextCapacity];
      growth = nextGrowth;
    }
}

- (void*) mutableBytes
{
  return bytes;
}

- (void) replaceBytesInRange: (NSRange)aRange
		   withBytes: (const void*)moreBytes
{
  NSUInteger need = NSMaxRange(aRange);

  if (aRange.location > length)
    {
      [NSException raise: NSRangeException
		  format: @"location bad in replaceBytesInRange:withBytes:"];
    }
  if (aRange.length > 0)
    {
      if (moreBytes == 0)
	{
	  [NSException raise: NSInvalidArgumentException
	    format: @"[%@-replaceBytesInRange:withBytes:] called with "
	    @"range but null bytes", NSStringFromClass([self class])];
	}
      if (need > length)
	{
	  [self setCapacity: need];
	  length = need;
	}
      memcpy(bytes + aRange.location, moreBytes, aRange.length);
    }
}

- (void) serializeDataAt: (const void*)data
	      ofObjCType: (const char*)type
		 context: (id <NSObjCTypeSerializationCallBack>)callback
{
  if (data == 0 || type == 0)
    {
      if (data == 0)
	{
	  NSLog(@"attempt to serialize from a null pointer");
	}
      if (type == 0)
	{
	  NSLog(@"attempt to serialize with a null type encoding");
	}
      return;
    }
  switch (*type)
    {
      case _C_ID:
	[callback serializeObjectAt: (id*)data
			 ofObjCType: type
			   intoData: self];
	return;

      case _C_CHARPTR:
	{
	  unsigned	len;
	  int32_t	ni;
	  uint32_t	minimum;

	  if (!*(void**)data)
	    {
	      ni = -1;
	      ni = GSSwapHostI32ToBig(ni);
	      [self appendBytes: (void*)&len length: sizeof(len)];
	      return;
	    }
	  len = strlen(*(void**)data);
	  ni = GSSwapHostI32ToBig(len);
	  minimum = length + len + sizeof(ni);
	  if (minimum > capacity)
	    {
	      [self _grow: minimum];
	    }
	  memcpy(bytes+length, &ni, sizeof(ni));
	  length += sizeof(ni);
	  if (len)
	    {
	      memcpy(bytes+length, *(void**)data, len);
	      length += len;
	    }
	  return;
	}
      case _C_ARY_B:
	{
	  unsigned	offset = 0;
	  unsigned	size;
	  unsigned	count = atoi(++type);
	  unsigned	i;
	  uint32_t	minimum;

	  while (isdigit(*type))
	    {
	      type++;
	    }
	  size = objc_sizeof_type(type);

	  /*
	   *	Serialized objects are going to take up at least as much
	   *	space as the originals, so we can calculate a minimum space
	   *	we are going to need and make sure our buffer is big enough.
	   */
	  minimum = length + size*count;
	  if (minimum > capacity)
	    {
	      [self _grow: minimum];
	    }

	  for (i = 0; i < count; i++)
	    {
	      [self serializeDataAt: (char*)data + offset
			 ofObjCType: type
			    context: callback];
	      offset += size;
	    }
	  return;
	}
      case _C_STRUCT_B:
	{
	  struct objc_struct_layout layout;

	  objc_layout_structure (type, &layout);
	  while (objc_layout_structure_next_member (&layout))
	    {
	      unsigned		offset;
	      unsigned		align;
	      const char	*ftype;

	      objc_layout_structure_get_info (&layout, &offset, &align, &ftype);

	      [self serializeDataAt: ((char*)data) + offset
			 ofObjCType: ftype
			    context: callback];
	    }
	  return;
	}
      case _C_PTR:
	[self serializeDataAt: *(char**)data
		   ofObjCType: ++type
		      context: callback];
	return;
      case _C_CHR:
      case _C_UCHR:
	(*appendImp)(self, appendSel, data, sizeof(unsigned char));
	return;
      case _C_SHT:
      case _C_USHT:
	{
	  unsigned short ns = NSSwapHostShortToBig(*(unsigned short*)data);
	  (*appendImp)(self, appendSel, &ns, sizeof(unsigned short));
	  return;
	}
      case _C_INT:
      case _C_UINT:
	{
	  unsigned ni = NSSwapHostIntToBig(*(unsigned int*)data);
	  (*appendImp)(self, appendSel, &ni, sizeof(unsigned));
	  return;
	}
      case _C_LNG:
      case _C_ULNG:
	{
	  unsigned long nl = NSSwapHostLongToBig(*(unsigned long*)data);
	  (*appendImp)(self, appendSel, &nl, sizeof(unsigned long));
	  return;
	}
      case _C_LNG_LNG:
      case _C_ULNG_LNG:
	{
	  unsigned long long nl;

	  nl = NSSwapHostLongLongToBig(*(unsigned long long*)data);
	  (*appendImp)(self, appendSel, &nl, sizeof(unsigned long long));
	  return;
	}
      case _C_FLT:
	{
	  NSSwappedFloat nf = NSSwapHostFloatToBig(*(float*)data);
	  (*appendImp)(self, appendSel, &nf, sizeof(NSSwappedFloat));
	  return;
	}
      case _C_DBL:
	{
	  NSSwappedDouble nd = NSSwapHostDoubleToBig(*(double*)data);
	  (*appendImp)(self, appendSel, &nd, sizeof(NSSwappedDouble));
	  return;
	}
#if __GNUC__ > 2 && defined(_C_BOOL)
      case _C_BOOL:
	(*appendImp)(self, appendSel, data, sizeof(_Bool));
	return;
#endif
      case _C_CLASS:
	{
	  const char  *name = *(Class*)data?class_getName(*(Class*)data):"";
	  uint16_t	ln = (uint16_t)strlen(name);
	  uint32_t	minimum = length + ln + sizeof(uint16_t);
	  uint16_t	ni;

	  if (minimum > capacity)
	    {
	      [self _grow: minimum];
	    }
	  ni = GSSwapHostI16ToBig(ln);
	  memcpy(bytes+length, &ni, sizeof(ni));
	  length += sizeof(ni);
	  if (ln)
	    {
	      memcpy(bytes+length, name, ln);
	      length += ln;
	    }
	  return;
	}
      case _C_SEL:
	{
	  const char  *name = *(SEL*)data?sel_getName(*(SEL*)data):"";
	  uint16_t	ln = (name == 0) ? 0 : (uint16_t)strlen(name);
	  const char  *types = *(SEL*)data?GSTypesFromSelector(*(SEL*)data):"";
	  uint16_t	lt = (types == 0) ? 0 : (uint16_t)strlen(types);
	  uint32_t	minimum = length + ln + lt + 2*sizeof(uint16_t);
	  uint16_t	ni;

	  if (minimum > capacity)
	    {
	      [self _grow: minimum];
	    }
	  ni = GSSwapHostI16ToBig(ln);
	  memcpy(bytes+length, &ni, sizeof(ni));
	  length += sizeof(ni);
	  ni = GSSwapHostI16ToBig(lt);
	  memcpy(bytes+length, &ni, sizeof(ni));
	  length += sizeof(ni);
	  if (ln)
	    {
	      memcpy(bytes+length, name, ln);
	      length += ln;
	    }
	  if (lt)
	    {
	      memcpy(bytes+length, types, lt);
	      length += lt;
	    }
	  return;
	}
      default:
	[NSException raise: NSMallocException
		    format: @"Unknown type to serialize - '%s'", type];
    }
}

- (void) serializeTypeTag: (unsigned char)tag
{
  if (length == capacity)
    {
      [self _grow: length + 1];
    }
  ((unsigned char*)bytes)[length++] = tag;
}

- (void) serializeTypeTag: (unsigned char)tag
	      andCrossRef: (unsigned int)xref
{
  if (xref <= 0xff)
    {
      tag = (tag & ~_GSC_SIZE) | _GSC_X_1;
      if (length + 2 >= capacity)
	{
	  [self _grow: length + 2];
	}
      *(uint8_t*)(bytes + length++) = tag;
      *(uint8_t*)(bytes + length++) = xref;
    }
  else if (xref <= 0xffff)
    {
      uint16_t	x = (uint16_t)xref;

      tag = (tag & ~_GSC_SIZE) | _GSC_X_2;
      if (length + 3 >= capacity)
	{
	  [self _grow: length + 3];
	}
      *(uint8_t*)(bytes + length++) = tag;
#if NEED_WORD_ALIGNMENT
      if ((length % __alignof__(uint16_t)) != 0)
	{
	  x = GSSwapHostI16ToBig(x);
	  memcpy((bytes + length), &x, 2);
	}
      else
#endif
      *(uint16_t*)(bytes + length) = GSSwapHostI16ToBig(x);
      length += 2;
    }
  else
    {
      uint32_t	x = (uint32_t)xref;

      tag = (tag & ~_GSC_SIZE) | _GSC_X_4;
      if (length + 5 >= capacity)
	{
	  [self _grow: length + 5];
	}
      *(uint8_t*)(bytes + length++) = tag;
#if NEED_WORD_ALIGNMENT
      if ((length % __alignof__(uint32_t)) != 0)
	{
	  x = GSSwapHostI32ToBig(x);
	  memcpy((bytes + length), &x, 4);
	}
      else
#endif
      *(uint32_t*)(bytes + length) = GSSwapHostI32ToBig(x);
      length += 4;
    }
}

- (id) setCapacity: (NSUInteger)size
{
  if (size != capacity)
    {
      void	*tmp;

      tmp = NSZoneMalloc(zone, size);
      if (tmp == 0)
	{
	  [NSException raise: NSMallocException
	    format: @"Unable to set data capacity to '%"PRIuPTR"'", size];
	}
      if (bytes)
	{
	  memcpy(tmp, bytes, capacity < size ? capacity : size);
	  if (zone == 0)
	    {
	      zone = NSDefaultMallocZone();
	    }
	  else
	    {
	      NSZoneFree(zone, bytes);
	    }
	}
      else if (zone == 0)
	{
	  zone = NSDefaultMallocZone();
	}
      bytes = tmp;
      capacity = size;
      growth = capacity/2;
      if (growth == 0)
	{
	  growth = 1;
	}
    }
  if (size < length)
    {
      length = size;
    }
  return self;
}

- (void) setData: (NSData*)data
{
  NSUInteger l = [data length];

  [self setCapacity: l];
  length = l;
  memcpy(bytes, [data bytes], length);
}

- (void) setLength: (NSUInteger)size
{
  if (size > capacity)
    {
      NSUInteger    growTo = capacity + capacity / 2;

      if (size > growTo)
        {
          growTo = size;
        }
      [self setCapacity: growTo];
    }
  if (size > length)
    {
      memset(bytes + length, '\0', size - length);
    }
  length = size;
}

@end

@implementation NSMutableDataWithDeallocatorBlock

+ (id) allocWithZone: (NSZone*)z
{
  return NSAllocateObject(mutableDataBlock, 0, z);
}

- (instancetype) initWithBytesNoCopy: (void*)buf
                              length: (NSUInteger)len
                         deallocator: (GSDataDeallocatorBlock)deallocBlock
{
  if (buf == NULL && len > 0)
    {
      [self release];
      [NSException raise: NSInvalidArgumentException
        format: @"[%@-initWithBytesNoCopy:length:deallocator:] called with "
          @"length but NULL bytes", NSStringFromClass([self class])];
    }

  /* The assumption here is that the superclass if fully concrete and will
   * not return a different instance. This invariant holds for the current
   * implementation of NSMutableDataMalloc, but not NSDataMalloc.
   */
  if (nil == (self = [super initWithBytesNoCopy: buf
                                         length: len
                                   freeWhenDone: NO]))
    {
      return nil;
    }
  ASSIGN(deallocator, (id)deallocBlock);
  return self;
}

- (void) dealloc
{
  if (deallocator != NULL)
    {
      CALL_BLOCK(((GSDataDeallocatorBlock)deallocator), bytes, capacity);
      // Clear out the ivars so that super doesn't double free.
      bytes = NULL;
      length = 0;
      DESTROY(deallocator);
    }

  [super dealloc];
}

- (id) setCapacity: (NSUInteger)size
{
  /* We need to override capacity modification so that we correctly call the
   * block when we are operating on the initial allocation, usual malloc/free
   * machinery otherwise. */
  if (size != capacity)
    {
      void	*tmp;

      tmp = NSZoneMalloc(zone, size);
      if (tmp == 0)
	{
	  [NSException raise: NSMallocException
	    format: @"Unable to set data capacity to '%"PRIuPTR"'", size];
	}
      if (bytes)
	{
	  memcpy(tmp, bytes, capacity < size ? capacity : size);
	  if (deallocator != NULL)
	    {
          CALL_BLOCK(((GSDataDeallocatorBlock)deallocator), bytes, capacity);
          DESTROY(deallocator);
	      zone = NSDefaultMallocZone();
	    }
	  else
	    {
	      NSZoneFree(zone, bytes);
	    }
	}
      else if (deallocator != NULL)
	{
      CALL_BLOCK(((GSDataDeallocatorBlock)deallocator), bytes, capacity);
      DESTROY(deallocator);
	  zone = NSDefaultMallocZone();
	}
      bytes = tmp;
      capacity = size;
      growth = capacity/2;
      if (growth == 0)
	{
	  growth = 1;
	}
    }
  if (size < length)
    {
      length = size;
    }
  return self;
}

@end


#ifdef	HAVE_SHMCTL
@implementation	NSMutableDataShared
+ (id) allocWithZone: (NSZone*)z
{
  return NSAllocateObject([NSMutableDataShared class], 0, z);
}

- (void) dealloc
{
  [self finalize];
  [super dealloc];
}

- (void) finalize
{
  if (bytes != 0)
    {
      struct shmid_ds	buf;

      if (shmctl(shmid, IPC_STAT, &buf) < 0)
	{
	  NSLog(@"[NSMutableDataShared -dealloc] shared memory "
	    @"control failed - %@", [NSError _last]);
	}
      else if (buf.shm_nattch == 1)
	{
	  if (shmctl(shmid, IPC_RMID, &buf) < 0)	/* Mark for deletion. */
	    {
	      NSLog(@"[NSMutableDataShared -dealloc] shared memory "
		@"delete failed - %@", [NSError _last]);
	    }
	}
      if (shmdt(bytes) < 0)
	{
	  NSLog(@"[NSMutableDataShared -dealloc] shared memory "
	    @"detach failed - %@", [NSError _last]);
	}
      bytes = 0;
      length = 0;
      capacity = 0;
      shmid = -1;
    }
  [super finalize];
}

- (id) initWithCapacity: (NSUInteger)bufferSize
{
  shmid = shmget(IPC_PRIVATE, bufferSize, IPC_CREAT|VM_ACCESS);
  if (shmid == -1)			/* Created memory? */
    {
      NSLog(@"[NSMutableDataShared -initWithCapacity:] shared memory "
	@"get failed for %"PRIuPTR" - %@", bufferSize, [NSError _last]);
      DESTROY(self);
      self = [mutableDataMalloc allocWithZone: NSDefaultMallocZone()];
      return [self initWithCapacity: bufferSize];
    }

  bytes = shmat(shmid, 0, 0);
  if (bytes == (void*)-1)
    {
      NSLog(@"[NSMutableDataShared -initWithCapacity:] shared memory "
	@"attach failed for %"PRIuPTR" - %@", bufferSize, [NSError _last]);
      bytes = 0;
      DESTROY(self);
      self = [mutableDataMalloc allocWithZone: NSDefaultMallocZone()];
      return [self initWithCapacity: bufferSize];
    }
  length = 0;
  capacity = bufferSize;

  return self;
}

- (id) initWithShmID: (int)anId length: (NSUInteger)bufferSize
{
  struct shmid_ds	buf;

  shmid = anId;
  if (shmctl(shmid, IPC_STAT, &buf) < 0)
    {
      NSLog(@"[NSMutableDataShared -initWithShmID:length:] shared memory "
	@"control failed - %@", [NSError _last]);
      DESTROY(self);	/* Unable to access memory. */
      return nil;
    }
  if (buf.shm_segsz < bufferSize)
    {
      NSLog(@"[NSMutableDataShared -initWithShmID:length:] shared memory "
	@"segment too small");
      DESTROY(self);	/* Memory segment too small. */
      return nil;
    }
  bytes = shmat(shmid, 0, 0);
  if (bytes == (void*)-1)
    {
      NSLog(@"[NSMutableDataShared -initWithShmID:length:] shared memory "
	@"attach failed - %@", [NSError _last]);
      bytes = 0;
      DESTROY(self);	/* Unable to attach to memory. */
      return nil;
    }
  length = bufferSize;
  capacity = length;

  return self;
}

- (id) setCapacity: (NSUInteger)size
{
  if (size != capacity)
    {
      void	*tmp;
      int	newid;

      newid = shmget(IPC_PRIVATE, size, IPC_CREAT|VM_ACCESS);
      if (newid == -1)			/* Created memory? */
	{
	  [NSException raise: NSMallocException
	    format: @"Unable to create shared memory segment"
	    @" (size:%"PRIuPTR") - %@.", size, [NSError _last]];
	}
      tmp = shmat(newid, 0, 0);
      if ((intptr_t)tmp == -1)			/* Attached memory? */
	{
	  [NSException raise: NSMallocException
		      format: @"Unable to attach to shared memory segment."];
	}
      memcpy(tmp, bytes, length);
      if (bytes)
	{
          struct shmid_ds	buf;

          if (shmctl(shmid, IPC_STAT, &buf) < 0)
	    {
	      NSLog(@"[NSMutableDataShared -setCapacity:] shared memory "
		@"control failed - %@", [NSError _last]);
	    }
          else if (buf.shm_nattch == 1)
	    {
	      if (shmctl(shmid, IPC_RMID, &buf) < 0)	/* Mark for deletion. */
		{
		  NSLog(@"[NSMutableDataShared -setCapacity:] shared memory "
		    @"delete failed - %@", [NSError _last]);
		}
	    }
	  if (shmdt(bytes) < 0)				/* Detach memory. */
	    {
              NSLog(@"[NSMutableDataShared -setCapacity:] shared memory "
		@"detach failed - %@", [NSError _last]);
	    }
	}
      bytes = tmp;
      shmid = newid;
      capacity = size;
    }
  if (size < length)
    {
      length = size;
    }
  return self;
}

- (int) shmID
{
  return shmid;
}

@end
#endif	/* HAVE_SHMCTL	*/
