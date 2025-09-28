/* Mach-O support for BFD.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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

#ifndef _MACH_O_CODESIGN_H
#define _MACH_O_CODESIGN_H

/* Codesign blob magics.  */

/* Superblob containing all the components.  */
#define BFD_MACH_O_CS_MAGIC_EMBEDDED_SIGNATURE 0xfade0cc0

/* Individual code requirement.  */
#define BFD_MACH_O_CS_MAGIC_REQUIREMENT 0xfade0c00

/* Collection of code requirements, indexed by type.  */
#define BFD_MACH_O_CS_MAGIC_REQUIREMENTS 0xfade0c01

/* Directory.  */
#define BFD_MACH_O_CS_MAGIC_CODEDIRECTORY 0xfade0c02

/* Entitlements blob.  */
#define BFD_MACH_O_CS_MAGIC_EMBEDDED_ENTITLEMENTS 0xfade7171

/* Blob container.  */
#define BFD_MACH_O_CS_MAGIC_BLOB_WRAPPER 0xfade0b01

struct mach_o_codesign_codedirectory_external_v1
{
  /* All the fields are in network byte order (big endian).  */
  unsigned char version[4];
  unsigned char flags[4];
  unsigned char hash_offset[4];
  unsigned char ident_offset[4];
  unsigned char nbr_special_slots[4];
  unsigned char nbr_code_slots[4];
  unsigned char code_limit[4];
  unsigned char hash_size[1];
  unsigned char hash_type[1];
  unsigned char spare1[1];
  unsigned char page_size[1];
  unsigned char spare2[4];
};

struct mach_o_codesign_codedirectory_v1
{
  unsigned int version;
  unsigned int flags;
  unsigned int hash_offset;
  unsigned int ident_offset;
  unsigned int nbr_special_slots;
  unsigned int nbr_code_slots;
  unsigned int code_limit;
  unsigned char hash_size;
  unsigned char hash_type;
  unsigned char spare1;
  unsigned char page_size;
  unsigned int spare2;
};

/* Value for hash_type.  */
#define BFD_MACH_O_CS_NO_HASH 0
#define BFD_MACH_O_CS_HASH_SHA1 1
#define BFD_MACH_O_CS_HASH_SHA256 2
#define BFD_MACH_O_CS_HASH_PRESTANDARD_SKEIN_160x256 32 /* Skein, 160 bits */
#define BFD_MACH_O_CS_HASH_PRESTANDARD_SKEIN_256x512 33 /* Skein, 256 bits */

#endif /* _MACH_O_CODESIGN_H */
