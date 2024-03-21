/* Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

/* Linux writes the task structure at the end of the core file.  Currently it
   is 2912 bytes.  It is possible that this should be a pickier check, but
   we should probably not be too picky (the size of the task structure might
   vary, and if it's not the length we expect it to be, it doesn't affect
   our ability to process the core file).  So allow 0-4096 extra bytes at
   the end.  */

#define TRAD_CORE_EXTRA_SIZE_ALLOWED 4096
