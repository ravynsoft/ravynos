/* SHA-1 thunks.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is part of libctf.

   libctf is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not see
   <http://www.gnu.org/licenses/>.  */

#include <ctf-impl.h>
#include <ctf-sha1.h>

static const char hex[] = "0123456789abcdef";

char *
ctf_sha1_fini (ctf_sha1_t *sha1, char *buf)
{
  size_t i;

  /* Alignment suitable for a uint32_t. */
  union
  {
    uint32_t align;
    unsigned char digest[((CTF_SHA1_SIZE - 1) / 2) + 1];
  } align;

  sha1_finish_ctx (sha1, align.digest);

  if (!buf)
    return NULL;

  buf[CTF_SHA1_SIZE - 1] = '\0';

  for (i = 0; i < (CTF_SHA1_SIZE - 1) / 2; i++)
    {
      buf[2 * i] = hex[align.digest[i] >> 4];
      buf[2 * i + 1] = hex[align.digest[i] & 0xf];
    }
  return buf;
}
