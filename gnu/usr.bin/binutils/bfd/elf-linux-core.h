/* Definitions for PRPSINFO structures under ELF on GNU/Linux.
   Copyright (C) 2013-2023 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef ELF_LINUX_CORE_H
#define ELF_LINUX_CORE_H

/* External 32-bit structure for PRPSINFO.  This structure is
   ABI-defined, thus we choose to use char arrays here in order to
   avoid dealing with different types in different architectures.

   This is the variant for targets which use a 32-bit data type for
   UID and GID, as all modern Linux ports do.  Some older ports use
   a 16-bit data type instead; see below for the alternative variant.

   This structure will ultimately be written in the corefile's note
   section, as the PRPSINFO.  */

struct elf_external_linux_prpsinfo32_ugid32
  {
    char pr_state;			/* Numeric process state.  */
    char pr_sname;			/* Char for pr_state.  */
    char pr_zomb;			/* Zombie.  */
    char pr_nice;			/* Nice val.  */
    char pr_flag[4];			/* Flags.  */
    char pr_uid[4];
    char pr_gid[4];
    char pr_pid[4];
    char pr_ppid[4];
    char pr_pgrp[4];
    char pr_sid[4];
    char pr_fname[16] ATTRIBUTE_NONSTRING;  /* Filename of executable.  */
    char pr_psargs[80] ATTRIBUTE_NONSTRING; /* Initial part of arg list.  */
  };

/* Helper function to copy an elf_internal_linux_prpsinfo in host
   endian to an elf_external_linux_prpsinfo32_ugid32 in target endian.  */

static inline void
swap_linux_prpsinfo32_ugid32_out
  (bfd *obfd,
   const struct elf_internal_linux_prpsinfo *from,
   struct elf_external_linux_prpsinfo32_ugid32 *to)
{
  bfd_put_8 (obfd, from->pr_state, &to->pr_state);
  bfd_put_8 (obfd, from->pr_sname, &to->pr_sname);
  bfd_put_8 (obfd, from->pr_zomb, &to->pr_zomb);
  bfd_put_8 (obfd, from->pr_nice, &to->pr_nice);
  bfd_put_32 (obfd, from->pr_flag, to->pr_flag);
  bfd_put_32 (obfd, from->pr_uid, to->pr_uid);
  bfd_put_32 (obfd, from->pr_gid, to->pr_gid);
  bfd_put_32 (obfd, from->pr_pid, to->pr_pid);
  bfd_put_32 (obfd, from->pr_ppid, to->pr_ppid);
  bfd_put_32 (obfd, from->pr_pgrp, to->pr_pgrp);
  bfd_put_32 (obfd, from->pr_sid, to->pr_sid);
  strncpy (to->pr_fname, from->pr_fname, sizeof (to->pr_fname));
  strncpy (to->pr_psargs, from->pr_psargs, sizeof (to->pr_psargs));
}

/* External 32-bit structure for PRPSINFO.  This structure is
   ABI-defined, thus we choose to use char arrays here in order to
   avoid dealing with different types in different architectures.

   This is the variant for targets which use a 16-bit data type for
   UID and GID, as some older Linux ports do.  All modern ports use
   a 32-bit data type instead; see above for the alternative variant.

   This structure will ultimately be written in the corefile's note
   section, as the PRPSINFO.  */

struct elf_external_linux_prpsinfo32_ugid16
  {
    char pr_state;			/* Numeric process state.  */
    char pr_sname;			/* Char for pr_state.  */
    char pr_zomb;			/* Zombie.  */
    char pr_nice;			/* Nice val.  */
    char pr_flag[4];			/* Flags.  */
    char pr_uid[2];
    char pr_gid[2];
    char pr_pid[4];
    char pr_ppid[4];
    char pr_pgrp[4];
    char pr_sid[4];
    char pr_fname[16] ATTRIBUTE_NONSTRING;  /* Filename of executable.  */
    char pr_psargs[80] ATTRIBUTE_NONSTRING; /* Initial part of arg list.  */
  };

/* Helper function to copy an elf_internal_linux_prpsinfo in host
   endian to an elf_external_linux_prpsinfo32_ugid16 in target endian.  */

static inline void
swap_linux_prpsinfo32_ugid16_out
  (bfd *obfd,
   const struct elf_internal_linux_prpsinfo *from,
   struct elf_external_linux_prpsinfo32_ugid16 *to)
{
  bfd_put_8 (obfd, from->pr_state, &to->pr_state);
  bfd_put_8 (obfd, from->pr_sname, &to->pr_sname);
  bfd_put_8 (obfd, from->pr_zomb, &to->pr_zomb);
  bfd_put_8 (obfd, from->pr_nice, &to->pr_nice);
  bfd_put_32 (obfd, from->pr_flag, to->pr_flag);
  bfd_put_16 (obfd, from->pr_uid, to->pr_uid);
  bfd_put_16 (obfd, from->pr_gid, to->pr_gid);
  bfd_put_32 (obfd, from->pr_pid, to->pr_pid);
  bfd_put_32 (obfd, from->pr_ppid, to->pr_ppid);
  bfd_put_32 (obfd, from->pr_pgrp, to->pr_pgrp);
  bfd_put_32 (obfd, from->pr_sid, to->pr_sid);
  strncpy (to->pr_fname, from->pr_fname, sizeof (to->pr_fname));
  strncpy (to->pr_psargs, from->pr_psargs, sizeof (to->pr_psargs));
}

/* External 64-bit structure for PRPSINFO.  This structure is
   ABI-defined, thus we choose to use char arrays here in order to
   avoid dealing with different types in different architectures.

   This is the variant for targets which use a 32-bit data type for
   UID and GID, as most Linux ports do.  The SH64 port uses a 16-bit
   data type instead; see below for the alternative variant.

   This structure will ultimately be written in the corefile's note
   section, as the PRPSINFO.  */

struct elf_external_linux_prpsinfo64_ugid32
  {
    char pr_state;			/* Numeric process state.  */
    char pr_sname;			/* Char for pr_state.  */
    char pr_zomb;			/* Zombie.  */
    char pr_nice;			/* Nice val.  */
    char gap[4];
    char pr_flag[8];			/* Flags.  */
    char pr_uid[4];
    char pr_gid[4];
    char pr_pid[4];
    char pr_ppid[4];
    char pr_pgrp[4];
    char pr_sid[4];
    char pr_fname[16] ATTRIBUTE_NONSTRING;  /* Filename of executable.  */
    char pr_psargs[80] ATTRIBUTE_NONSTRING; /* Initial part of arg list.  */
  };

/* Helper function to copy an elf_internal_linux_prpsinfo in host
   endian to an elf_external_linux_prpsinfo64_ugid32 in target endian.  */

static inline void
swap_linux_prpsinfo64_ugid32_out
  (bfd *obfd,
   const struct elf_internal_linux_prpsinfo *from,
   struct elf_external_linux_prpsinfo64_ugid32 *to)
{
  bfd_put_8 (obfd, from->pr_state, &to->pr_state);
  bfd_put_8 (obfd, from->pr_sname, &to->pr_sname);
  bfd_put_8 (obfd, from->pr_zomb, &to->pr_zomb);
  bfd_put_8 (obfd, from->pr_nice, &to->pr_nice);
  bfd_put_64 (obfd, from->pr_flag, to->pr_flag);
  bfd_put_32 (obfd, from->pr_uid, to->pr_uid);
  bfd_put_32 (obfd, from->pr_gid, to->pr_gid);
  bfd_put_32 (obfd, from->pr_pid, to->pr_pid);
  bfd_put_32 (obfd, from->pr_ppid, to->pr_ppid);
  bfd_put_32 (obfd, from->pr_pgrp, to->pr_pgrp);
  bfd_put_32 (obfd, from->pr_sid, to->pr_sid);
  strncpy (to->pr_fname, from->pr_fname, sizeof (to->pr_fname));
  strncpy (to->pr_psargs, from->pr_psargs, sizeof (to->pr_psargs));
}

/* External 64-bit structure for PRPSINFO.  This structure is
   ABI-defined, thus we choose to use char arrays here in order to
   avoid dealing with different types in different architectures.

   This is the variant for the SH64 port which uses a 16-bit data
   type for UID and GID.  Most Linux ports use a 32-bit data type
   instead; see above for the alternative variant.

   This structure will ultimately be written in the corefile's note
   section, as the PRPSINFO.  */

struct elf_external_linux_prpsinfo64_ugid16
  {
    char pr_state;			/* Numeric process state.  */
    char pr_sname;			/* Char for pr_state.  */
    char pr_zomb;			/* Zombie.  */
    char pr_nice;			/* Nice val.  */
    char gap[4];
    char pr_flag[8];			/* Flags.  */
    char pr_uid[2];
    char pr_gid[2];
    char pr_pid[4];
    char pr_ppid[4];
    char pr_pgrp[4];
    char pr_sid[4];
    char pr_fname[16] ATTRIBUTE_NONSTRING;  /* Filename of executable.  */
    char pr_psargs[80] ATTRIBUTE_NONSTRING; /* Initial part of arg list.  */
  };

/* Helper function to copy an elf_internal_linux_prpsinfo in host
   endian to an elf_external_linux_prpsinfo64_ugid16 in target endian.  */

static inline void
swap_linux_prpsinfo64_ugid16_out
  (bfd *obfd,
   const struct elf_internal_linux_prpsinfo *from,
   struct elf_external_linux_prpsinfo64_ugid16 *to)
{
  bfd_put_8 (obfd, from->pr_state, &to->pr_state);
  bfd_put_8 (obfd, from->pr_sname, &to->pr_sname);
  bfd_put_8 (obfd, from->pr_zomb, &to->pr_zomb);
  bfd_put_8 (obfd, from->pr_nice, &to->pr_nice);
  bfd_put_64 (obfd, from->pr_flag, to->pr_flag);
  bfd_put_16 (obfd, from->pr_uid, to->pr_uid);
  bfd_put_16 (obfd, from->pr_gid, to->pr_gid);
  bfd_put_32 (obfd, from->pr_pid, to->pr_pid);
  bfd_put_32 (obfd, from->pr_ppid, to->pr_ppid);
  bfd_put_32 (obfd, from->pr_pgrp, to->pr_pgrp);
  bfd_put_32 (obfd, from->pr_sid, to->pr_sid);
  strncpy (to->pr_fname, from->pr_fname, sizeof (to->pr_fname));
  strncpy (to->pr_psargs, from->pr_psargs, sizeof (to->pr_psargs));
}

#endif
