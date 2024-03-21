/* ELF AArch64 mapping symbol support
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

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
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#define BFD_AARCH64_SPECIAL_SYM_TYPE_MAP	(1 << 0)
#define BFD_AARCH64_SPECIAL_SYM_TYPE_TAG	(1 << 1)
#define BFD_AARCH64_SPECIAL_SYM_TYPE_OTHER	(1 << 2)
#define BFD_AARCH64_SPECIAL_SYM_TYPE_ANY	(~0)
extern bool bfd_is_aarch64_special_symbol_name
  (const char * name, int type);
