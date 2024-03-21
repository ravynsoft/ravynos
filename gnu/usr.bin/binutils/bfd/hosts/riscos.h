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

/* RISC/os 4.52C, and presumably other versions.  */

#include <bsd43/machine/machparam.h>
#include <bsd43/machine/vmparam.h>

#define NBPG BSD43_NBPG
#define UPAGES BSD43_UPAGES
#define HOST_TEXT_START_ADDR BSD43_USRTEXT
#define HOST_DATA_START_ADDR BSD43_USRDATA
#define HOST_STACK_END_ADDR BSD43_USRSTACK
