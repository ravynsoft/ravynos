/* sframe-dump.c - Textual dump of .sframe.

   Copyright (C) 2022-2023 Free Software Foundation, Inc.

   This file is part of libsframe.

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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "sframe-impl.h"

#define SFRAME_HEADER_FLAGS_STR_MAX_LEN 50

/* Return TRUE if the SFrame section is associated with the aarch64 ABIs.  */

static bool
is_sframe_abi_arch_aarch64 (sframe_decoder_ctx *sfd_ctx)
{
  bool aarch64_p = false;

  uint8_t abi_arch = sframe_decoder_get_abi_arch (sfd_ctx);
  if (abi_arch == SFRAME_ABI_AARCH64_ENDIAN_BIG
      || abi_arch == SFRAME_ABI_AARCH64_ENDIAN_LITTLE)
    aarch64_p = true;

  return aarch64_p;
}

static void
dump_sframe_header (sframe_decoder_ctx *sfd_ctx)
{
  uint8_t ver;
  uint8_t flags;
  char *flags_str;
  const char *ver_str = NULL;
  const sframe_header *header = &(sfd_ctx->sfd_header);

  /* Prepare SFrame section version string.  */
  const char *version_names[]
    = { "NULL",
	"SFRAME_VERSION_1",
	"SFRAME_VERSION_2" };

  /* PS: Keep SFRAME_HEADER_FLAGS_STR_MAX_LEN in sync if adding more members to
     this array.  */
  const char *flag_names[]
    = { "SFRAME_F_FDE_SORTED",
	"SFRAME_F_FRAME_POINTER" };

  ver = sframe_decoder_get_version (sfd_ctx);
  if (ver <= SFRAME_VERSION)
    ver_str = version_names[ver];

  /* Prepare SFrame section flags string.  */
  flags = header->sfh_preamble.sfp_flags;
  flags_str = (char*) calloc (sizeof (char), SFRAME_HEADER_FLAGS_STR_MAX_LEN);
  if (flags)
    {
      if (flags & SFRAME_F_FDE_SORTED)
	strcpy (flags_str, flag_names[0]);
      if (flags & SFRAME_F_FRAME_POINTER)
	{
	  if (strlen (flags_str) > 0)
	    strcpy (flags_str, ",");
	  strcpy (flags_str, flag_names[1]);
	}
    }
  else
    strcpy (flags_str, "NONE");

  const char* subsec_name = "Header";
  printf ("\n");
  printf ("  %s :\n", subsec_name);
  printf ("\n");
  printf ("    Version: %s\n", ver_str);
  printf ("    Flags: %s\n", flags_str);
  printf ("    Num FDEs: %d\n", sframe_decoder_get_num_fidx (sfd_ctx));
  printf ("    Num FREs: %d\n", header->sfh_num_fres);

  free (flags_str);
}

static void
dump_sframe_func_with_fres (sframe_decoder_ctx *sfd_ctx,
			    unsigned int funcidx,
			    uint64_t sec_addr)
{
  uint32_t j = 0;
  uint32_t num_fres = 0;
  uint32_t func_size = 0;
  int32_t func_start_address = 0;
  unsigned char func_info = 0;

  uint64_t func_start_pc_vma = 0;
  uint64_t fre_start_pc_vma = 0;
  const char *base_reg_str[] = {"fp", "sp"};
  int32_t cfa_offset = 0;
  int32_t fp_offset = 0;
  int32_t ra_offset = 0;
  uint8_t base_reg_id = 0;
  int err[3] = {0, 0, 0};

  sframe_frame_row_entry fre;

  /* Get the SFrame function descriptor.  */
  sframe_decoder_get_funcdesc (sfd_ctx, funcidx, &num_fres,
			       &func_size, &func_start_address, &func_info);
  /* Calculate the virtual memory address for function start pc.  */
  func_start_pc_vma = func_start_address + sec_addr;

  /* Mark FDEs with [m] where the FRE start address is interpreted as a
     mask.  */
  int fde_type_addrmask_p = (SFRAME_V1_FUNC_FDE_TYPE (func_info)
			     == SFRAME_FDE_TYPE_PCMASK);
  const char *fde_type_marker
    = (fde_type_addrmask_p ? "[m]" : "   ");

  printf ("\n    func idx [%d]: pc = 0x%"PRIx64 ", size = %d bytes",
	  funcidx,
	  func_start_pc_vma,
	  func_size);

  if (is_sframe_abi_arch_aarch64 (sfd_ctx)
      && (SFRAME_V1_FUNC_PAUTH_KEY (func_info) == SFRAME_AARCH64_PAUTH_KEY_B))
    printf (", pauth = B key");

  char temp[100];

  printf ("\n    %-7s%-8s %-10s%-10s%-13s",
	  "STARTPC", fde_type_marker, "CFA", "FP", "RA");
  for (j = 0; j < num_fres; j++)
    {
      sframe_decoder_get_fre (sfd_ctx, funcidx, j, &fre);

      fre_start_pc_vma = (fde_type_addrmask_p
			  ? fre.fre_start_addr
			  : func_start_pc_vma + fre.fre_start_addr);

      /* FIXME - fixup the err caching in array.
	 assert no error for base reg id.  */
      base_reg_id = sframe_fre_get_base_reg_id (&fre, &err[0]);
      cfa_offset = sframe_fre_get_cfa_offset (sfd_ctx, &fre, &err[0]);
      fp_offset = sframe_fre_get_fp_offset (sfd_ctx, &fre, &err[1]);
      ra_offset = sframe_fre_get_ra_offset (sfd_ctx, &fre, &err[2]);

      /* Dump CFA info.  */
      printf ("\n");
      printf ("    %016"PRIx64, fre_start_pc_vma);
      sprintf (temp, "%s+%d", base_reg_str[base_reg_id], cfa_offset);
      printf ("  %-10s", temp);

      /* Dump SP/FP info.  */
      if (err[1] == 0)
	sprintf (temp, "c%+d", fp_offset);
      else
	strcpy (temp, "u");
      printf ("%-10s", temp);

      /* Dump RA info.
	 If an ABI does not track RA offset, e.g., AMD64, display a 'u',
	 else display the offset d as 'c+-d'.  */
      if (sframe_decoder_get_fixed_ra_offset(sfd_ctx)
	  != SFRAME_CFA_FIXED_RA_INVALID)
	strcpy (temp, "u");
      else if (err[2] == 0)
	sprintf (temp, "c%+d", ra_offset);

      /* Mark SFrame FRE's RA information with "[s]" if the RA is mangled
	 with signature bits.  */
      const char *ra_mangled_p_str
	= ((sframe_fre_get_ra_mangled_p (sfd_ctx, &fre, &err[2]))
	   ? "[s]" : "   ");
      strcat (temp, ra_mangled_p_str);
      printf ("%-13s", temp);
    }
}

static void
dump_sframe_functions (sframe_decoder_ctx *sfd_ctx, uint64_t sec_addr)
{
  uint32_t i;
  uint32_t num_fdes;

  const char* subsec_name = "Function Index";
  printf ("\n  %s :\n", subsec_name);

  num_fdes = sframe_decoder_get_num_fidx (sfd_ctx);
  for (i = 0; i < num_fdes; i++)
    {
      dump_sframe_func_with_fres (sfd_ctx, i, sec_addr);
      printf ("\n");
    }
}

void
dump_sframe (sframe_decoder_ctx *sfd_ctx, uint64_t sec_addr)
{
  uint8_t ver;

  dump_sframe_header (sfd_ctx);

  ver = sframe_decoder_get_version (sfd_ctx);
  if (ver == SFRAME_VERSION)
    dump_sframe_functions (sfd_ctx, sec_addr);
  else
    printf ("\n No further information can be displayed.  %s",
	    "SFrame version not supported\n");
}
