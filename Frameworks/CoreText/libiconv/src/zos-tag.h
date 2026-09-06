/* Copyright (C) 2023 Free Software Foundation, Inc.
   This file is part of the GNU LIBICONV Library.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Mike Fulton.  */

#ifndef __ZOS_TAG__
#define __ZOS_TAG__ 1

#include <_Ccsid.h>

#ifdef __MVS__
/* See: https://www.ibm.com/docs/en/zos/latest?topic=lf-toccsid-convert-codeset-name-coded-character-set-id */

static void chgpfx (char* encoding, size_t enclen, size_t pfxlen, const char* normpfx, size_t normpfxlen)
{
  /* assertion: enclen >= pfxlen >= normpfxlen */
  if (normpfxlen > 0) {
    memcpy(encoding, normpfx, normpfxlen);
  }
  memcpy(&encoding[normpfxlen], &encoding[pfxlen], enclen-pfxlen+1);
}

static __ccsid_t map_encoding_to_ccsid (const char* encoding)
{
  size_t enclen = strlen(encoding);
  char* updtenc = (char*) xmalloc(enclen+1);
  memcpy(updtenc, encoding, enclen+1);

  /*
   * Some strings are known to gnu iconv but not z/OS __toCcsid.
   * Examples are ISO-8859-1, ISO_8859-2, CP819 (which map to ISO8859-1, ISO8859-2, 819)
   *
   * The following are supported encodings and corresponding output CCSIDs
   *
   * CCSID Encoding
   * 819  ISO8859-1
   * 912  ISO8859-2
   * 914  ISO8859-4
   * 915  ISO8859-5
   * 1089 ISO8859-6
   * 813  ISO8859-7
   * 916  ISO8859-8
   * 920  ISO8859-9
   * 921  ISO8859-13
   * 923  ISO8859-15
   */
  #define ISO8859      "ISO8859"
  #define ISO8859_LEN (sizeof(ISO8859)-1)
  #define ISO8859_DASH "ISO-8859"
  #define ISO8859_DASH_LEN (sizeof(ISO8859_DASH)-1)
  #define ISO8859_UL "ISO_8859"
  #define ISO8859_UL_LEN (sizeof(ISO8859_UL)-1)
  #define CP "CP"
  #define CP_LEN (sizeof(CP)-1)
  #define NO_PFX ""
  #define NO_PFX_LEN (0)

  if (enclen > ISO8859_DASH_LEN && !memcmp(ISO8859_DASH, encoding, ISO8859_DASH_LEN)) {
    chgpfx(updtenc, enclen, ISO8859_DASH_LEN, ISO8859, ISO8859_LEN);
  } else if (enclen > ISO8859_UL_LEN && !memcmp(ISO8859_UL, encoding, ISO8859_UL_LEN)) {
    chgpfx(updtenc, enclen, ISO8859_UL_LEN, ISO8859, ISO8859_LEN);
  } else if (enclen > CP_LEN && !memcmp(CP, encoding, CP_LEN)) {
    chgpfx(updtenc, enclen, CP_LEN, NO_PFX, NO_PFX_LEN);
  }
  return __toCcsid(updtenc);
}

static int tagfile(int filedes, const char* tocode)
{
  int status = 0;

  __ccsid_t newccsid = map_encoding_to_ccsid(tocode);
  if (newccsid) {
    attrib_t attr = {0};
    attr.att_filetagchg = 1;
    attr.att_filetag.ft_ccsid = newccsid;
    attr.att_filetag.ft_txtflag = 1;
    status = __fchattr(filedes, &attr, sizeof(attr));
  }
  return status;
}
#endif
#endif
