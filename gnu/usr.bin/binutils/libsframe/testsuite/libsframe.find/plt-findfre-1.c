/* plt-findfre-1.c -- Test for sframe_find_fre for SFRAME_FDE_TYPE_PCMASK.

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
add_plt_fde1 (sframe_encoder_ctx *ectx, int idx)
{
  int i, err;
  /* A contiguous block containing 3 FREs.  The start_ip_offset must remain
     less than 16 bytes.  */
  sframe_frame_row_entry fres[]
    = { {0x0, {0x1, 0, 0}, 0x3},
	{0x6, {0x2, 0xf0, 0}, 0x5},
	{0xc, {0x3, 0xf0, 0}, 0x4}
      };

  unsigned char finfo = sframe_fde_create_func_info (SFRAME_FRE_TYPE_ADDR1,
						     SFRAME_FDE_TYPE_PCMASK);
  /* 5 pltN entries of 16 bytes each.  */
  err = sframe_encoder_add_funcdesc_v2 (ectx, 0x1000, 16*5, finfo, 16, 3);
  if (err == -1)
    return err;

  for (i = 0; i < 3; i++)
    if (sframe_encoder_add_fre (ectx, idx, fres+i) == SFRAME_ERR)
      return -1;

  return 0;
}

int main (void)
{
  sframe_encoder_ctx *ectx;
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

  ectx = sframe_encode (SFRAME_VERSION, 0, SFRAME_ABI_AMD64_ENDIAN_LITTLE,
			SFRAME_CFA_FIXED_FP_INVALID,
			-8, /* Fixed RA offset for AMD64.  */
			&err);

  err = add_plt_fde1 (ectx, 0);
  TEST ("plt-findfre-1: Adding FDE1 for plt", err == 0);

  fde_cnt = sframe_encoder_get_num_fidx (ectx);
  TEST ("plt-findfre-1: Test FDE count", fde_cnt == 1);

  sframe_buf = sframe_encoder_write (ectx, &sf_size, &err);
  TEST ("plt-findfre-1: Encoder write", err == 0);

  dctx = sframe_decode (sframe_buf, sf_size, &err);
  TEST("plt-findfre-1: Decoder setup", dctx != NULL);

  /* Find the first FRE in PLT1.  */
  err = sframe_find_fre (dctx, (0x1000 + 0x0), &frep);
  TEST("plt-findfre-1: Find first FRE in PLT1",
       ((err == 0) && (sframe_fre_get_cfa_offset (dctx, &frep, &err) == 0x1)));

  /* Find the second FRE.  */
  err = sframe_find_fre (dctx, (0x1000 + 0x6), &frep);
  TEST("plt-findfre-1: Find second FRE in PLT1",
       ((err == 0) && (sframe_fre_get_cfa_offset (dctx, &frep, &err) == 0x2)));

  /* Find the last FRE.  */
  err = sframe_find_fre (dctx, (0x1000 + 0xc), &frep);
  TEST("plt-findfre-1: Find last FRE in PLT1",
       ((err == 0) && (sframe_fre_get_cfa_offset (dctx, &frep, &err) == 0x3)));

  /* Find the first FRE in PLT4.  */
  err = sframe_find_fre (dctx, (0x1000 + 16*3 + 0x0), &frep);
  TEST("plt-findfre-1: Find first FRE in PLT4",
       ((err == 0) && (sframe_fre_get_cfa_offset (dctx, &frep, &err) == 0x1)));

  /* Find the second FRE in PLT4.  */
  err = sframe_find_fre (dctx, (0x1000 + 16*3 + 0x6), &frep);
  TEST("plt-findfre-1: Find second FRE in PLT4",
       ((err == 0) && (sframe_fre_get_cfa_offset (dctx, &frep, &err) == 0x2)));

  /* Find the last FRE in PLT4.  */
  err = sframe_find_fre (dctx, (0x1000 + 16*3 + 0xc), &frep);
  TEST("plt-findfre-1: Find last FRE in PLT4",
       ((err == 0) && (sframe_fre_get_cfa_offset (dctx, &frep, &err) == 0x3)));

  sframe_encoder_free (&ectx);
  sframe_decoder_free (&dctx);

  return 0;
}
