/* Copyright (c) 1997 John D. Polstra.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.  */

#ifndef	_ELF_HINTS_H_
#define	_ELF_HINTS_H_

#include <stdint.h>

/* Hints file produced by ldconfig.  */
struct elfhints_hdr
{
  uint32_t magic;		/* Magic number.  */
  uint32_t version;		/* File version (1).  */
  uint32_t strtab;		/* Offset of string table in file.  */
  uint32_t strsize;		/* Size of string table.  */
  uint32_t dirlist;		/* Offset of directory list in string table.  */
  uint32_t dirlistlen;		/* strlen(dirlist).  */
  uint32_t spare[26];		/* Room for expansion.  */
};

#define ELFHINTS_MAGIC	0x746e6845

#define _PATH_ELF_HINTS	"/var/run/ld-elf.so.hints"

#endif /* !_ELF_HINTS_H_ */
