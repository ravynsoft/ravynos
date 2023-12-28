/* compress-debug.c - compress debug sections
   Copyright (C) 2010-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#if HAVE_ZSTD
#include <zstd.h>
#endif
#include "ansidecl.h"
#include "compress-debug.h"

/* Initialize the compression engine.  */

void *
compress_init (bool use_zstd)
{
  if (use_zstd) {
#if HAVE_ZSTD
    return ZSTD_createCCtx ();
#endif
  }

  static struct z_stream_s strm;
  memset (&strm, 0, sizeof (strm));
  deflateInit (&strm, Z_DEFAULT_COMPRESSION);
  return &strm;
}

/* Stream the contents of a frag to the compression engine.  Output
   from the engine goes into the current frag on the obstack.  */

int
compress_data (bool use_zstd, void *ctx, const char **next_in, int *avail_in,
	       char **next_out, int *avail_out)
{
  if (use_zstd)
    {
#if HAVE_ZSTD
      ZSTD_outBuffer ob = { *next_out, *avail_out, 0 };
      ZSTD_inBuffer ib = { *next_in, *avail_in, 0 };
      size_t ret = ZSTD_compressStream2 (ctx, &ob, &ib, ZSTD_e_continue);
      *next_in += ib.pos;
      *avail_in -= ib.pos;
      *next_out += ob.pos;
      *avail_out -= ob.pos;
      if (ZSTD_isError (ret))
	return -1;
      return (int)ob.pos;
#endif
    }

  struct z_stream_s *strm = ctx;

  strm->next_in = (Bytef *) (*next_in);
  strm->avail_in = *avail_in;
  strm->next_out = (Bytef *) (*next_out);
  strm->avail_out = *avail_out;

  int x = deflate (strm, Z_NO_FLUSH);
  if (x != Z_OK)
    return -1;

  int out_size = *avail_out - strm->avail_out;
  *next_in = (char *) (strm->next_in);
  *avail_in = strm->avail_in;
  *next_out = (char *) (strm->next_out);
  *avail_out = strm->avail_out;

  return out_size;
}

/* Finish the compression and consume the remaining compressed output.
   Returns -1 for error, 0 when done, 1 when more output buffer is
   needed.  */

int
compress_finish (bool use_zstd, void *ctx, char **next_out,
		 int *avail_out, int *out_size)
{
  if (use_zstd)
    {
#if HAVE_ZSTD
      ZSTD_outBuffer ob = { *next_out, *avail_out, 0 };
      ZSTD_inBuffer ib = { 0, 0, 0 };
      size_t ret = ZSTD_compressStream2 (ctx, &ob, &ib, ZSTD_e_end);
      *out_size = ob.pos;
      *next_out += ob.pos;
      *avail_out -= ob.pos;
      if (ZSTD_isError (ret))
	return -1;
      if (ret == 0)
	ZSTD_freeCCtx (ctx);
      return ret ? 1 : 0;
#endif
    }

  int x;
  struct z_stream_s *strm = ctx;

  strm->avail_in = 0;
  strm->next_out = (Bytef *) (*next_out);
  strm->avail_out = *avail_out;

  x = deflate (strm, Z_FINISH);

  *out_size = *avail_out - strm->avail_out;
  *next_out = (char *) (strm->next_out);
  *avail_out = strm->avail_out;

  if (x == Z_STREAM_END)
    {
      deflateEnd (strm);
      return 0;
    }
  if (strm->avail_out != 0)
    return -1;
  return 1;
}
