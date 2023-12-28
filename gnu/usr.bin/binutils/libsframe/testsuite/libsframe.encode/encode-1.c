/* encode-1.c -- Test for encoder in libsframe.

   Copyright (C) 2022-2023 Free Software Foundation, Inc.

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
    = { {0x0, {0x8, 0, 0}, 0x3},
	{0x1, {0x10, 0xf0, 0}, 0x5},
	{0x4, {0x10, 0xf0, 0}, 0x4},
	{0x1a, {0x8, 0xf0, 0}, 0x5}
      };

  unsigned char finfo = sframe_fde_create_func_info (SFRAME_FRE_TYPE_ADDR1,
						     SFRAME_FDE_TYPE_PCINC);
  err = sframe_encoder_add_funcdesc (encode, 0xfffff03e, 0x1b, finfo, 4);
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
    = { {0x0, {0x8, 0, 0}, 0x3},
	{0x1, {0x10, 0xf0, 0}, 0x5},
	{0x4, {0x10, 0xf0, 0}, 0x4},
	{0xf, {0x8, 0xf0, 0}, 0x5}
      };

  unsigned char finfo = sframe_fde_create_func_info (SFRAME_FRE_TYPE_ADDR1,
						     SFRAME_FDE_TYPE_PCINC);
  err = sframe_encoder_add_funcdesc (encode, 0xfffff059, 0x10, finfo, 4);
  if (err == -1)
    return err;

  for (i = 0; i < 4; i++)
    if (sframe_encoder_add_fre (encode, idx, fres+i) == SFRAME_ERR)
      return -1;

  return 0;
}

/*
 * SFrame info from the following source (2 fdes, 4 fres in each fde):
 * static int cnt;
 * int foo() { return ++cnt; }
 * int main() { return foo(); }
 */
#define DATA    "DATA2"

static int
data_match (char *sframe_buf, size_t sz)
{
  FILE *fp;
  struct stat st;
  char *sf_buf;
  size_t sf_size;
  int diffs;

  fp = fopen (DATA, "r");
  if (fp == NULL)
    return 0;
  if (fstat (fileno (fp), &st) < 0)
    {
      perror ("fstat");
      fclose (fp);
      return 0;
    }
  sf_buf = malloc (st.st_size);
  if (sf_buf == NULL)
    {
      perror ("malloc");
      return 0;
    }
  sf_size = fread (sf_buf, 1, st.st_size, fp);
  fclose (fp);
  if (sf_size == 0 || sf_buf == NULL)
    {
      fprintf (stderr, "Encode: Read section failed\n");
      return 0;
    }
  if (sf_size != sz)
    return 0;

  diffs = memcmp (sf_buf, sframe_buf, sz);

  free (sf_buf);
  return diffs == 0;
}

int main (void)
{
  sframe_encoder_ctx *encode;
  sframe_frame_row_entry frep;
  char *sframe_buf;
  size_t sf_size;
  int err = 0;
  unsigned int fde_cnt = 0;
  int match_p = 0;

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

  fde_cnt = sframe_encoder_get_num_fidx (encode);
  TEST ("encode-1: Encoder FDE count", fde_cnt == 0);

  err = sframe_encoder_add_fre (encode, 1, &frep);
  TEST ("encode-1: Encoder update workflow", err == SFRAME_ERR);

  err = add_fde1 (encode, 0);
  TEST ("encode-1: Encoder adding FDE1", err == 0);

  err = add_fde2 (encode, 1);
  TEST ("encode-1: Encoder adding FDE2", err == 0);

  fde_cnt = sframe_encoder_get_num_fidx (encode);
  TEST ("encode-1: Encoder FDE count", fde_cnt == 2);

  sframe_buf = sframe_encoder_write (encode, &sf_size, &err);
  TEST ("encode-1: Encoder write", err == 0);

  match_p = data_match (sframe_buf, sf_size);
  TEST ("encode-1: Encode buffer match", match_p == 1);

  sframe_encoder_free (&encode);
  return 0;
}
