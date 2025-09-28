/* findfre-1.c -- Test for sframe_find_fre in libsframe.

   Copyright (C) 2023 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "sframe-api.h"

/* DejaGnu should not use gnulib's vsnprintf replacement here.  */
#undef vsnprintf
#include <dejagnu.h>

static int
add_fde1 (sframe_encoder_ctx *encode, int idx)
{
  int i, err;
  /* A contiguous block containing 4 FREs.  */
  sframe_frame_row_entry fres[]
    = { {0x0, {0x1, 0, 0}, 0x3},
	{0x1, {0x2, 0xf0, 0}, 0x5},
	{0x10, {0x3, 0xf0, 0}, 0x4},
	{0x38, {0x8, 0xf0, 0}, 0x5}
      };

  unsigned char finfo = sframe_fde_create_func_info (SFRAME_FRE_TYPE_ADDR1,
						     SFRAME_FDE_TYPE_PCINC);
  err = sframe_encoder_add_funcdesc (encode, 0xfffff03e, 0x40, finfo, 4);
  if (err == -1)
    return err;

  for (i = 0; i < 4; i++)
    if (sframe_encoder_add_fre (encode, idx,fres+i) == SFRAME_ERR)
      return -1;

  return 0;
}

static int
add_fde2 (sframe_encoder_ctx *encode, int idx)
{
  int i, err;
  /* A contiguous block containing 4 FREs.  */
  sframe_frame_row_entry fres[]
    = { {0x0, {0x10, 0, 0}, 0x3},
	{0x10, {0x12, 0xf0, 0}, 0x5},
	{0x14, {0x14, 0xf0, 0}, 0x4},
	{0x20, {0x15, 0xf0, 0}, 0x5}
      };

  unsigned char finfo = sframe_fde_create_func_info (SFRAME_FRE_TYPE_ADDR1,
						     SFRAME_FDE_TYPE_PCINC);
  err = sframe_encoder_add_funcdesc (encode, 0xfffff08e, 0x60, finfo, 4);
  if (err == -1)
    return err;

  for (i = 0; i < 4; i++)
    if (sframe_encoder_add_fre (encode, idx, fres+i) == SFRAME_ERR)
      return -1;

  return 0;
}

int main (void)
{
  sframe_encoder_ctx *encode;
  sframe_decoder_ctx *dctx;
  sframe_frame_row_entry frep;
  char *sframe_buf;
  size_t sf_size;
  int err = 0;
  unsigned int fde_cnt = 0;

#define TEST(name, cond)                                                      \
  do                                                                          \
    {                                                                         \
      if (cond)                                                               \
	pass (name);                                                          \
      else                                                                    \
	fail (name);                                                          \
    }                                                                         \
    while (0)

  encode = sframe_encode (SFRAME_VERSION, 0,
			  SFRAME_ABI_AMD64_ENDIAN_LITTLE,
			  SFRAME_CFA_FIXED_FP_INVALID,
			  -8, /* Fixed RA offset for AMD64.  */
			  &err);

  err = add_fde1 (encode, 0);
  TEST ("findfre-1: Adding FDE1", err == 0);

  err = add_fde2 (encode, 1);
  TEST ("findfre-1: Adding FDE2", err == 0);

  fde_cnt = sframe_encoder_get_num_fidx (encode);
  TEST ("findfre-1: Test FDE count", fde_cnt == 2);

  sframe_buf = sframe_encoder_write (encode, &sf_size, &err);
  TEST ("findfre-1: Encoder write", err == 0);

  dctx = sframe_decode (sframe_buf, sf_size, &err);
  TEST("findfre-1: Decoder setup", dctx != NULL);

  /* Find the third FRE in first FDE.  */
  err = sframe_find_fre (dctx, (0xfffff03e + 0x15), &frep);
  TEST("findfre-1: Find third FRE",
       ((err == 0) && (sframe_fre_get_cfa_offset(dctx, &frep, &err) == 0x3)));

  /* Find an FRE for PC at the end of range covered by FRE.  */
  err = sframe_find_fre (dctx, (0xfffff03e + 0x9), &frep);
  TEST("findfre-1: Find FRE for last PC covered by FRE",
       ((err == 0) && (sframe_fre_get_cfa_offset(dctx, &frep, &err) == 0x2)));

  /* Find the last FRE in first FDE.  */
  err = sframe_find_fre (dctx, (0xfffff03e + 0x39), &frep);
  TEST("findfre-1: Find last FRE",
       ((err == 0) && (sframe_fre_get_cfa_offset(dctx, &frep, &err) == 0x8)));

  /* Find the second FRE in second FDE.  */
  err = sframe_find_fre (dctx, (0xfffff08e + 0x11), &frep);
  TEST("findfre-1: Find second FRE",
       ((err == 0) && (sframe_fre_get_cfa_offset(dctx, &frep, &err) == 0x12)));

  /* Find the first FRE in second FDE.  */
  err = sframe_find_fre (dctx, (0xfffff08e + 0x0), &frep);
  TEST("findfre-1: Find first FRE",
       ((err == 0) && (sframe_fre_get_cfa_offset(dctx, &frep, &err) == 0x10)));

  /* Find FRE for PC out of range.  Expect error code.  */
  err = sframe_find_fre (dctx, (0xfffff03e + 0x40), &frep);
  TEST("findfre-1: Find FRE for out of range PC",
       (err == SFRAME_ERR));

  sframe_encoder_free (&encode);
  sframe_decoder_free (&dctx);

  return 0;
}
