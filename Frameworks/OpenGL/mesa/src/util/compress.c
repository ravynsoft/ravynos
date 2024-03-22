/*
 * Copyright © 2021 Valve Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifdef HAVE_COMPRESSION

#include <assert.h>

/* Ensure that zlib uses 'const' in 'z_const' declarations. */
#ifndef ZLIB_CONST
#define ZLIB_CONST
#endif

#ifdef HAVE_ZLIB
#include "zlib.h"
#endif

#ifdef HAVE_ZSTD
#include "zstd.h"
#endif

#include "util/compress.h"
#include "util/perf/cpu_trace.h"
#include "macros.h"

/* 3 is the recomended level, with 22 as the absolute maximum */
#define ZSTD_COMPRESSION_LEVEL 3

size_t
util_compress_max_compressed_len(size_t in_data_size)
{
#ifdef HAVE_ZSTD
   /* from the zstd docs (https://facebook.github.io/zstd/zstd_manual.html):
    * compression runs faster if `dstCapacity` >= `ZSTD_compressBound(srcSize)`.
    */
   return ZSTD_compressBound(in_data_size);
#elif defined(HAVE_ZLIB)
   /* From https://zlib.net/zlib_tech.html:
    *
    *    "In the worst possible case, where the other block types would expand
    *    the data, deflation falls back to stored (uncompressed) blocks. Thus
    *    for the default settings used by deflateInit(), compress(), and
    *    compress2(), the only expansion is an overhead of five bytes per 16 KB
    *    block (about 0.03%), plus a one-time overhead of six bytes for the
    *    entire stream."
    */
   size_t num_blocks = (in_data_size + 16383) / 16384; /* round up blocks */
   return in_data_size + 6 + (num_blocks * 5);
#else
   STATIC_ASSERT(false);
#endif
}

/* Compress data and return the size of the compressed data */
size_t
util_compress_deflate(const uint8_t *in_data, size_t in_data_size,
                      uint8_t *out_data, size_t out_buff_size)
{
   MESA_TRACE_FUNC();
#ifdef HAVE_ZSTD
   size_t ret = ZSTD_compress(out_data, out_buff_size, in_data, in_data_size,
                              ZSTD_COMPRESSION_LEVEL);
   if (ZSTD_isError(ret))
      return 0;

   return ret;
#elif defined(HAVE_ZLIB)
   size_t compressed_size = 0;

   /* allocate deflate state */
   z_stream strm;
   strm.zalloc = Z_NULL;
   strm.zfree = Z_NULL;
   strm.opaque = Z_NULL;
   strm.next_in = in_data;
   strm.next_out = out_data;
   strm.avail_in = in_data_size;
   strm.avail_out = out_buff_size;

   int ret = deflateInit(&strm, Z_BEST_COMPRESSION);
   if (ret != Z_OK) {
       (void) deflateEnd(&strm);
       return 0;
   }

   /* compress until end of in_data */
   ret = deflate(&strm, Z_FINISH);

   /* stream should be complete */
   assert(ret == Z_STREAM_END);
   if (ret == Z_STREAM_END) {
       compressed_size = strm.total_out;
   }

   /* clean up and return */
   (void) deflateEnd(&strm);
   return compressed_size;
#else
   STATIC_ASSERT(false);
# endif
}

/**
 * Decompresses data, returns true if successful.
 */
bool
util_compress_inflate(const uint8_t *in_data, size_t in_data_size,
                      uint8_t *out_data, size_t out_data_size)
{
   MESA_TRACE_FUNC();
#ifdef HAVE_ZSTD
   size_t ret = ZSTD_decompress(out_data, out_data_size, in_data, in_data_size);
   return !ZSTD_isError(ret);
#elif defined(HAVE_ZLIB)
   z_stream strm;

   /* allocate inflate state */
   strm.zalloc = Z_NULL;
   strm.zfree = Z_NULL;
   strm.opaque = Z_NULL;
   strm.next_in = in_data;
   strm.avail_in = in_data_size;
   strm.next_out = out_data;
   strm.avail_out = out_data_size;

   int ret = inflateInit(&strm);
   if (ret != Z_OK)
      return false;

   ret = inflate(&strm, Z_NO_FLUSH);
   assert(ret != Z_STREAM_ERROR);  /* state not clobbered */

   /* Unless there was an error we should have decompressed everything in one
    * go as we know the uncompressed file size.
    */
   if (ret != Z_STREAM_END) {
      (void)inflateEnd(&strm);
      return false;
   }
   assert(strm.avail_out == 0);

   /* clean up and return */
   (void)inflateEnd(&strm);
   return true;
#else
   STATIC_ASSERT(false);
#endif
}

#endif
