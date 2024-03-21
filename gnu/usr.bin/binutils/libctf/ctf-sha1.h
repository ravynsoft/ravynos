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

#ifndef _CTF_SHA1_H
#define _CTF_SHA1_H

#include "config.h"
#include "sha1.h"

#define CTF_SHA1_SIZE 41

typedef struct sha1_ctx ctf_sha1_t;

static inline void
ctf_sha1_init (ctf_sha1_t *sha1)
{
  sha1_init_ctx (sha1);
}

static inline void
ctf_sha1_add (ctf_sha1_t *sha1, const void *buf, size_t len)
{
  sha1_process_bytes (buf, len, sha1);
}
#endif
