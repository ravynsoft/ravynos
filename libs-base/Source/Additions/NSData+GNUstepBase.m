/* Implementation of extension methods to base additions

   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>

   This file is part of the GNUstep Base Library.

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

*/
#import "common.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSByteOrder.h"
#import "Foundation/NSException.h"
#import "GNUstepBase/NSData+GNUstepBase.h"
#import "GNUstepBase/NSString+GNUstepBase.h"

#include <ctype.h>

#if     USE_ZLIB
#include <zlib.h>
#endif

#if	defined(_WIN32)
#include <wincrypt.h>
#else
#include <fcntl.h>
#endif

static int
randombytes(uint8_t *buf, unsigned len)
{
#if	defined(_WIN32)

  HCRYPTPROV hProvider = 0;

  if (!CryptAcquireContextW(&hProvider, 0, 0,
    PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
    {
      return -1;
    }

  if (!CryptGenRandom(hProvider, len, buf))
    {
      CryptReleaseContext(hProvider, 0);
      return -1;
    }

  CryptReleaseContext(hProvider, 0);

#else

  int		devUrandom;
  ssize_t	bytesRead;

  devUrandom = open("/dev/urandom", O_RDONLY);
  if (devUrandom == -1)
    {
      return -1;
    }
  bytesRead = read(devUrandom, buf, len);
  close(devUrandom);
  if (bytesRead != len)
    {
      return -1;
    }

#endif
  return 0;
}

/**
 * Extension methods for the NSData class.
 */
@implementation NSData (GNUstepBase)

+ (id) dataWithRandomBytesOfLength: (NSUInteger)length
{
  uint8_t       *buf = 0;
  NSData        *d;

  if (0 == length || length > 0xffffffff)
    {
      return nil;       // Unreasonable length for random data
    }
  buf = malloc(length);
  if (0 == buf)
    {
      return nil;       // Not enough memory for random data
    }
  if (randombytes(buf, (unsigned)length) < 0)
    {
      free(buf);
      return nil;       // Unable to generate the random data
    }
  d = [[self alloc] initWithBytesNoCopy: buf length: length freeWhenDone: YES];
  if (nil == d)
    {
      free(buf);
      return nil;       // Unable to create NSData instance
    }
  return AUTORELEASE(d);
}

- (NSString*) escapedRepresentation
{
  char          *buf;
  NSUInteger    len;
  NSString      *string;

  buf = [self escapedRepresentation: &len];
  string = [[NSString alloc] initWithBytesNoCopy: buf
                                          length: len
                                        encoding: NSASCIIStringEncoding
                                    freeWhenDone: YES];
  return AUTORELEASE(string);
}

- (char*) escapedRepresentation: (NSUInteger*)length
{
  const uint8_t *bytes = (const uint8_t*)[self bytes];
  uint8_t       *buf;
  NSUInteger    count = [self length];
  NSUInteger    size = count + 1;
  NSUInteger    index;
  NSUInteger    pos;

  for (index = 0; index < count; index++)
    {
      uint8_t   b = bytes[index];

      if ('\n' == b) size++;
      else if ('\r' == b) size++;
      else if ('\t' == b) size++;
      else if ('\\' == b) size++;
      else if (b < 32 || b > 126) size += 3;
    }
  buf = (uint8_t*)malloc(size);
  for (pos = index = 0; index < count; index++)
    {
      uint8_t   b = bytes[index];

      if ('\n' == b)
        {
          buf[pos++] = '\\';
          buf[pos++] = 'n';
        }
      else if ('\r' == b)
        {
          buf[pos++] = '\\';
          buf[pos++] = 'r';
        }
      else if ('\t' == b)
        {
          buf[pos++] = '\\';
          buf[pos++] = 't';
        }
      else if ('\\' == b)
        {
          buf[pos++] = '\\';
          buf[pos++] = '\\';
        }
      else if (b < 32 || b > 126)
        {
          sprintf((char*)&buf[pos], "\\x%02x", b);
          pos += 4;
        }
      else
        {
          buf[pos++] = b;
        }
    }
  buf[pos] = '\0';
  if (0 != length)
    {
      *length = pos;
    }
  return (char*)buf;
}

- (NSString*) hexadecimalRepresentation
{
  char          *buf;
  NSUInteger    len;
  NSString      *string;

  buf = [self hexadecimalRepresentation: &len];
  string = [[NSString alloc] initWithBytesNoCopy: buf
                                          length: len
                                        encoding: NSASCIIStringEncoding
                                    freeWhenDone: YES];
  return AUTORELEASE(string);
}

- (char*) hexadecimalRepresentation: (NSUInteger*)length
{
  static const char	*hexChars = "0123456789ABCDEF";
  unsigned		slen = [self length];
  unsigned		dlen = slen * 2;
  const unsigned char	*src = (const unsigned char *)[self bytes];
  char			*dst;
  unsigned		spos = 0;
  unsigned		dpos = 0;

  dst = (char*)malloc(dlen + 1);
  while (spos < slen)
    {
      unsigned char	c = src[spos++];

      dst[dpos++] = hexChars[(c >> 4) & 0x0f];
      dst[dpos++] = hexChars[c & 0x0f];
    }
  dst[dpos] = '\0';
  if (0 != length)
    {
      *length = dpos;
    }
  return dst;
}

- (NSData*) gunzipped
{
#if     USE_ZLIB
  NSUInteger    length = [self length];
  z_stream      stream;

  if (NO == [self isGzipped])
    {
      return self;
    }

  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.avail_in = (unsigned)length;
  stream.next_in = (uint8_t *)[self bytes];
  stream.total_out = 0;
  stream.avail_out = 0;

  if (inflateInit2(&stream, 15 + 32) == Z_OK)   // inflate or gzip
    {
      NSZone    *zone = NSDefaultMallocZone();
      uint8_t   *dst;
      unsigned  capacity;
      int       status = Z_OK;

      capacity = 2 * length;
      dst = NSZoneMalloc(zone, capacity);
      while (Z_OK == status)
        {
          if (stream.total_out >= capacity)
            {
              capacity += length / 2;
              dst = NSZoneRealloc(zone, dst, capacity);
            }
          stream.next_out = dst + stream.total_out;
          stream.avail_out = (unsigned)(capacity - stream.total_out);
          status = inflate(&stream, Z_SYNC_FLUSH);
        }
      if (inflateEnd(&stream) == Z_OK)
        {
          if (Z_STREAM_END == status)
            {
              NSMutableData     *result;

              result = [NSMutableData alloc];
              result = [result initWithBytesNoCopy: dst
                                            length: stream.total_out
                                      freeWhenDone: YES];
              return AUTORELEASE(result);
            }
        }
      NSZoneFree(zone, dst);
    }
#else
  [NSException raise: NSGenericException
              format: @"library was configured without zlib support"];
#endif
  return nil;
}

- (NSData*) gzipped: (int)compressionLevel
{
#if     USE_ZLIB
  NSUInteger    length = [self length];
  z_stream      stream;

  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  stream.avail_in = (unsigned)length;
  stream.next_in = (uint8_t*)[self bytes];
  stream.total_out = 0;
  stream.avail_out = 0;

  if (compressionLevel < 0 || compressionLevel > 9)
    {
      compressionLevel = Z_DEFAULT_COMPRESSION;
    }
  if (deflateInit2(&stream,
    compressionLevel,
    Z_DEFLATED,
    31,
    8,
    Z_DEFAULT_STRATEGY) == Z_OK)
    {
      NSMutableData     *result;
      NSZone            *zone = NSDefaultMallocZone();
      unsigned          capacity = 1024 * 16;
      uint8_t           *dst;

      dst = NSZoneMalloc(zone, capacity);
      while (stream.avail_out == 0)
        {
          if (stream.total_out >= capacity)
            {
              capacity += 1024 * 16;
              dst = NSZoneRealloc(zone, dst, capacity);
            }
          stream.next_out = dst + stream.total_out;
          stream.avail_out = (unsigned)(capacity - stream.total_out);
          (void)deflate(&stream, Z_FINISH);
        }
      deflateEnd(&stream);
      result = [NSMutableData alloc];
      result = [result initWithBytesNoCopy: dst
                                    length: stream.total_out
                              freeWhenDone: YES];
      return AUTORELEASE(result);
    }
#else
  [NSException raise: NSGenericException
              format: @"library was configured without zlib support"];
#endif
  return nil;
}

/**
 * Initialises the receiver with the supplied string data which contains
 * a hexadecimal coding of the bytes.  The parsing of the string is
 * fairly tolerant, ignoring whitespace and permitting both upper and
 * lower case hexadecimal digits (the -hexadecimalRepresentation method
 * produces a string using only uppercase digits with no white space).<br />
 * If the string does not contain one or more pairs of hexadecimal digits
 * then an exception is raised.
 */
- (id) initWithHexadecimalRepresentation: (NSString*)string
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  NSData		*d;
  const char		*src;
  const char		*end;
  unsigned char		*dst;
  unsigned int		pos = 0;
  unsigned char		byte = 0;
  BOOL			high = NO;

  d = [string dataUsingEncoding: NSASCIIStringEncoding
	   allowLossyConversion: YES];
  src = (const char*)[d bytes];
  end = src + [d length];
  dst = NSZoneMalloc(NSDefaultMallocZone(), [d length]/2 + 1);

  while (src < end)
    {
      char		c = *src++;
      unsigned char	v;

      if (isspace(c))
	{
	  continue;
	}
      if (c >= '0' && c <= '9')
	{
	  v = c - '0';
	}
      else if (c >= 'A' && c <= 'F')
	{
	  v = c - 'A' + 10;
	}
      else if (c >= 'a' && c <= 'f')
	{
	  v = c - 'a' + 10;
	}
      else
	{
	  pos = 0;
	  break;
	}
      if (high == NO)
	{
	  byte = v << 4;
	  high = YES;
	}
      else
	{
	  byte |= v;
	  high = NO;
	  dst[pos++] = byte;
	}
    }
  if (pos > 0 && high == NO)
    {
      self = [self initWithBytes: dst length: pos];
    }
  else
    {
      DESTROY(self);
    }
  NSZoneFree(NSDefaultMallocZone(), dst);
  [arp drain];
  if (self == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"%@: invalid hexadecimal string data",
	NSStringFromSelector(_cmd)];
    }
  return self;
}

- (BOOL) isGzipped
{
  NSUInteger    length = [self length];
  const uint8_t *bytes = (const uint8_t *)[self bytes];

  return (length >= 2 && bytes[0] == 0x1f && bytes[1] == 0x8b) ? YES : NO;
}

struct MD5Context
{
  uint32_t	buf[4];
  uint32_t	bits[2];
  uint8_t	in[64];
};
static void MD5Init (struct MD5Context *context);
static void MD5Update (struct MD5Context *context, unsigned char const *buf,
unsigned len);
static void MD5Final (unsigned char digest[16], struct MD5Context *context);
static void MD5Transform (uint32_t buf[4], uint32_t const in[16]);

/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 */

/*
 * Ensure data is little-endian
 */
static void littleEndian (void *buf, unsigned words)
{
  if (NSHostByteOrder() == NS_BigEndian)
    {
      while (words-- > 0)
        {
          union swap {
            uint32_t    num;
            uint8_t     byt[4];
          } tmp;
          uint8_t       b0;
          uint8_t       b1;

          tmp.num = ((uint32_t*)buf)[words];
          b0 = tmp.byt[0];
          b1 = tmp.byt[1];
          tmp.byt[0] = tmp.byt[3];
          tmp.byt[1] = tmp.byt[2];
          tmp.byt[2] = b1;
          tmp.byt[3] = b0;
          ((uint32_t*)buf)[words] = tmp.num;
        }
    }
}

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
static void MD5Init (struct MD5Context *ctx)
{
  ctx->buf[0] = 0x67452301;
  ctx->buf[1] = 0xefcdab89;
  ctx->buf[2] = 0x98badcfe;
  ctx->buf[3] = 0x10325476;

  ctx->bits[0] = 0;
  ctx->bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
static void MD5Update (struct MD5Context *ctx, unsigned char const *buf,
  unsigned len)
{
  uint32_t t;

  /* Update bitcount */

  t = ctx->bits[0];
  if ((ctx->bits[0] = t + ((uint32_t) len << 3)) < t)
    ctx->bits[1]++;		/* Carry from low to high */
  ctx->bits[1] += len >> 29;

  t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

  /* Handle any leading odd-sized chunks */

  if (t)
    {
      unsigned char *p = (unsigned char *) ctx->in + t;

      t = 64 - t;
      if (len < t)
	{
	  memcpy (p, buf, len);
	  return;
	}
      memcpy (p, buf, t);
      littleEndian (ctx->in, 16);
      MD5Transform (ctx->buf, (uint32_t *) ctx->in);
      buf += t;
      len -= t;
    }
  /* Process data in 64-byte chunks */

  while (len >= 64)
    {
      memcpy (ctx->in, buf, 64);
      littleEndian (ctx->in, 16);
      MD5Transform (ctx->buf, (uint32_t *) ctx->in);
      buf += 64;
      len -= 64;
    }

  /* Handle any remaining bytes of data. */

  memcpy (ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
static void MD5Final (unsigned char digest[16], struct MD5Context *ctx)
{
  unsigned count;
  unsigned char *p;

  /* Compute number of bytes mod 64 */
  count = (ctx->bits[0] >> 3) & 0x3F;

  /* Set the first char of padding to 0x80.  This is safe since there is
     always at least one byte free */
  p = ctx->in + count;
  *p++ = 0x80;

  /* Bytes of padding needed to make 64 bytes */
  count = 64 - 1 - count;

  /* Pad out to 56 mod 64 */
  if (count < 8)
    {
      /* Two lots of padding:  Pad the first block to 64 bytes */
      memset (p, 0, count);
      littleEndian (ctx->in, 16);
      MD5Transform (ctx->buf, (uint32_t *) ctx->in);

      /* Now fill the next block with 56 bytes */
      memset (ctx->in, 0, 56);
    }
  else
    {
      /* Pad block to 56 bytes */
      memset (p, 0, count - 8);
    }
  littleEndian (ctx->in, 14);

  /* Append length in bits and transform */
  ((uint32_t *) ctx->in)[14] = ctx->bits[0];
  ((uint32_t *) ctx->in)[15] = ctx->bits[1];

  MD5Transform (ctx->buf, (uint32_t *) ctx->in);
  littleEndian ((unsigned char *) ctx->buf, 4);
  memcpy (digest, ctx->buf, 16);
  memset (ctx, 0, sizeof (*ctx));	/* In case it's sensitive */
}

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
  (w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x)

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 43bit words of new data.  MD5Update blocks
 * the data and converts bytes into 43bit words for this routine.
 */
static void MD5Transform (uint32_t buf[4], uint32_t const in[16])
{
  register uint32_t a, b, c, d;

  a = buf[0];
  b = buf[1];
  c = buf[2];
  d = buf[3];

  MD5STEP (F1, a, b, c, d, in[0] + 0xd76aa478, 7);
  MD5STEP (F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
  MD5STEP (F1, c, d, a, b, in[2] + 0x242070db, 17);
  MD5STEP (F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
  MD5STEP (F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
  MD5STEP (F1, d, a, b, c, in[5] + 0x4787c62a, 12);
  MD5STEP (F1, c, d, a, b, in[6] + 0xa8304613, 17);
  MD5STEP (F1, b, c, d, a, in[7] + 0xfd469501, 22);
  MD5STEP (F1, a, b, c, d, in[8] + 0x698098d8, 7);
  MD5STEP (F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
  MD5STEP (F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
  MD5STEP (F1, b, c, d, a, in[11] + 0x895cd7be, 22);
  MD5STEP (F1, a, b, c, d, in[12] + 0x6b901122, 7);
  MD5STEP (F1, d, a, b, c, in[13] + 0xfd987193, 12);
  MD5STEP (F1, c, d, a, b, in[14] + 0xa679438e, 17);
  MD5STEP (F1, b, c, d, a, in[15] + 0x49b40821, 22);

  MD5STEP (F2, a, b, c, d, in[1] + 0xf61e2562, 5);
  MD5STEP (F2, d, a, b, c, in[6] + 0xc040b340, 9);
  MD5STEP (F2, c, d, a, b, in[11] + 0x265e5a51, 14);
  MD5STEP (F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
  MD5STEP (F2, a, b, c, d, in[5] + 0xd62f105d, 5);
  MD5STEP (F2, d, a, b, c, in[10] + 0x02441453, 9);
  MD5STEP (F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
  MD5STEP (F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
  MD5STEP (F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
  MD5STEP (F2, d, a, b, c, in[14] + 0xc33707d6, 9);
  MD5STEP (F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
  MD5STEP (F2, b, c, d, a, in[8] + 0x455a14ed, 20);
  MD5STEP (F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
  MD5STEP (F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
  MD5STEP (F2, c, d, a, b, in[7] + 0x676f02d9, 14);
  MD5STEP (F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

  MD5STEP (F3, a, b, c, d, in[5] + 0xfffa3942, 4);
  MD5STEP (F3, d, a, b, c, in[8] + 0x8771f681, 11);
  MD5STEP (F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
  MD5STEP (F3, b, c, d, a, in[14] + 0xfde5380c, 23);
  MD5STEP (F3, a, b, c, d, in[1] + 0xa4beea44, 4);
  MD5STEP (F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
  MD5STEP (F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
  MD5STEP (F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
  MD5STEP (F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
  MD5STEP (F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
  MD5STEP (F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
  MD5STEP (F3, b, c, d, a, in[6] + 0x04881d05, 23);
  MD5STEP (F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
  MD5STEP (F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
  MD5STEP (F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
  MD5STEP (F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

  MD5STEP (F4, a, b, c, d, in[0] + 0xf4292244, 6);
  MD5STEP (F4, d, a, b, c, in[7] + 0x432aff97, 10);
  MD5STEP (F4, c, d, a, b, in[14] + 0xab9423a7, 15);
  MD5STEP (F4, b, c, d, a, in[5] + 0xfc93a039, 21);
  MD5STEP (F4, a, b, c, d, in[12] + 0x655b59c3, 6);
  MD5STEP (F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
  MD5STEP (F4, c, d, a, b, in[10] + 0xffeff47d, 15);
  MD5STEP (F4, b, c, d, a, in[1] + 0x85845dd1, 21);
  MD5STEP (F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
  MD5STEP (F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
  MD5STEP (F4, c, d, a, b, in[6] + 0xa3014314, 15);
  MD5STEP (F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
  MD5STEP (F4, a, b, c, d, in[4] + 0xf7537e82, 6);
  MD5STEP (F4, d, a, b, c, in[11] + 0xbd3af235, 10);
  MD5STEP (F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
  MD5STEP (F4, b, c, d, a, in[9] + 0xeb86d391, 21);

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}

/**
 * Creates an MD5 digest of the information stored in the receiver and
 * returns it as an autoreleased 16 byte NSData object.<br />
 * If you need to produce a digest of string information, you need to
 * decide what character encoding is to be used and convert your string
 * to a data object of that encoding type first using the
 * [NSString-dataUsingEncoding:] method -
 * <example>
 *   myDigest = [[myString dataUsingEncoding: NSUTF8StringEncoding] md5Digest];
 * </example>
 * If you need to use the digest in a human readable form, you will
 * probably want it to be seen as 32 hexadecimal digits, and can do that
 * using the -hexadecimalRepresentation method.
 */
- (NSData*) md5Digest
{
  struct MD5Context	ctx;
  unsigned char		digest[16];

  MD5Init(&ctx);
  MD5Update(&ctx, [self bytes], [self length]);
  MD5Final(digest, &ctx);
  return [NSData dataWithBytes: digest length: 16];
}

/**
 * Decodes the source data from uuencoded and return the result.<br />
 * Returns the encoded file name in namePtr if it is not null.
 * Returns the encoded file mode in modePtr if it is not null.
 */
- (BOOL) uudecodeInto: (NSMutableData*)decoded
		 name: (NSString**)namePtr
		 mode: (NSInteger*)modePtr
{
  const unsigned char	*bytes = (const unsigned char*)[self bytes];
  unsigned		length = [self length];
  unsigned		decLength = [decoded length];
  unsigned		pos = 0;
  NSString		*name = nil;

  if (namePtr != 0)
    {
      *namePtr = nil;
    }
  if (modePtr != 0)
    {
      *modePtr = 0;
    }

#define DEC(c)	(((c) - ' ') & 077)

  for (pos = 0; pos < length; pos++)
    {
      if (bytes[pos] == '\n')
	{
	  if (name != nil)
	    {
	      unsigned		i = 0;
	      int		lineLength;
	      unsigned char	*decPtr;

	      lineLength = DEC(bytes[i++]);
	      if (lineLength <= 0)
		{
		  break;	// Got line length zero or less.
		}

	      [decoded setLength: decLength + lineLength];
	      decPtr = [decoded mutableBytes];

	      while (lineLength > 0)
		{
		  unsigned char	tmp[4];
		  int	c;

		  /*
		   * In case the data is corrupt, we need to copy into
		   * a temporary buffer avoiding buffer overrun in the
		   * main buffer.
		   */
		  tmp[0] = bytes[i++];
		  if (i < pos)
		    {
		      tmp[1] = bytes[i++];
		      if (i < pos)
			{
			  tmp[2] = bytes[i++];
			  if (i < pos)
			    {
			      tmp[3] = bytes[i++];
			    }
			  else
			    {
			      tmp[3] = 0;
			    }
			}
		      else
			{
			  tmp[2] = 0;
			  tmp[3] = 0;
			}
		    }
		  else
		    {
		      tmp[1] = 0;
		      tmp[2] = 0;
		      tmp[3] = 0;
		    }
		  if (lineLength >= 1)
		    {
		      c = DEC(tmp[0]) << 2 | DEC(tmp[1]) >> 4;
		      decPtr[decLength++] = (unsigned char)c;
		    }
		  if (lineLength >= 2)
		    {
		      c = DEC(tmp[1]) << 4 | DEC(tmp[2]) >> 2;
		      decPtr[decLength++] = (unsigned char)c;
		    }
		  if (lineLength >= 3)
		    {
		      c = DEC(tmp[2]) << 6 | DEC(tmp[3]);
		      decPtr[decLength++] = (unsigned char)c;
		    }
		  lineLength -= 3;
		}
	    }
	  else if (pos > 6 && strncmp((const char*)bytes, "begin ", 6) == 0)
	    {
	      unsigned	off = 6;
	      unsigned	end = pos;
	      int	mode = 0;
	      NSData	*d;

	      if (end > off && bytes[end-1] == '\r')
		{
		  end--;
		}
	      while (off < end && bytes[off] >= '0' && bytes[off] <= '7')
		{
		  mode *= 8;
		  mode += bytes[off] - '0';
		  off++;
		}
	      if (modePtr != 0)
		{
		  *modePtr = mode;
		}
	      while (off < end && bytes[off] == ' ')
		{
		  off++;
		}
	      d = [NSData dataWithBytes: &bytes[off] length: end - off];
	      name = [[NSString alloc] initWithData: d
					   encoding: NSASCIIStringEncoding];
	      IF_NO_GC(AUTORELEASE(name);)
	      if (namePtr != 0)
		{
		  *namePtr = name;
		}
	    }
	  pos++;
	  bytes += pos;
	  length -= pos;
	}
    }
  if (name == nil)
    {
      return NO;
    }
  return YES;
}

/**
 * Encode the source data to uuencoded.<br />
 * Uses the supplied name as the filename in the encoded data,
 * and says that the file mode is as specified.<br />
 * If no name is supplied, uses <code>untitled</code> as the name.
 */
- (BOOL) uuencodeInto: (NSMutableData*)encoded
		 name: (NSString*)name
		 mode: (NSInteger)mode
{
  const unsigned char	*bytes = (const unsigned char*)[self bytes];
  int			length = [self length];
  unsigned char		buf[64];
  unsigned		i;

  name = [name stringByTrimmingSpaces];
  if ([name length] == 0)
    {
      name = @"untitled";
    }
  /*
   * The header is a line of the form 'begin mode filename'
   */
  snprintf((char*)buf, sizeof(buf), "begin %03o ", (int)mode);
  [encoded appendBytes: buf length: strlen((const char*)buf)];
  [encoded appendData: [name dataUsingEncoding: NSASCIIStringEncoding]];
  [encoded appendBytes: "\n" length: 1];

#define ENC(c) ((c) > 0 ? ((c) & 077) + ' ': '`')

  while (length > 0)
    {
      int	count;
      unsigned	pos;

      /*
       * We want up to 45 bytes in a line ... and we record the
       * number of bytes as the initial output character.
       */
      count = length;
      if (count > 45)
	{
	  count = 45;
	}
      i = 0;
      buf[i++] = ENC(count);

      /*
       * Now we encode the actual data for the line.
       */
      for (pos = 0; count > 0; count -= 3)
	{
	  unsigned char	tmp[3];
	  int		c;

	  /*
	   * Copy data into a temporary buffer ensuring we don't
	   * overrun the end of the original buffer risking access
	   * violation.
	   */
	  tmp[0] = bytes[pos++];
	  if (pos < length)
	    {
	      tmp[1] = bytes[pos++];
	      if (pos < length)
		{
		  tmp[2] = bytes[pos++];
		}
	      else
		{
		  tmp[2] = 0;
		}
	    }
	  else
	    {
	      tmp[1] = 0;
	      tmp[2] = 0;
	    }

	  c = tmp[0] >> 2;
	  buf[i++] = ENC(c);
	  c = ((tmp[0] << 4) & 060) | ((tmp[1] >> 4) & 017);
	  buf[i++] = ENC(c);
	  c = ((tmp[1] << 2) & 074) | ((tmp[2] >> 6) & 03);
	  buf[i++] = ENC(c);
	  c = tmp[2] & 077;
	  buf[i++] = ENC(c);
	}
      bytes += pos;
      length -= pos;
      buf[i++] = '\n';
      [encoded appendBytes: buf length: i];
    }

  /*
   * Encode a line of length zero followed by 'end' as the terminator.
   */
  [encoded appendBytes: "`\nend\n" length: 6];
  return YES;
}

@end
