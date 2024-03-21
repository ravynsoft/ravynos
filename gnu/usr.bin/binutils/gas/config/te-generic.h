/* Copyright (C) 2007-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3,
   or (at your option) any later version.

   GAS is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* This file is te-generic.h and is intended to be a template for
   target environment specific header files.

   It is my intent that this file will evolve into a file suitable for config,
   compile, and copying as an aid for testing and porting.  xoxorich.  */

/* Added these, because if we don't know what we're targeting we may
   need an assembler version of libgcc, and that will use local
   labels.  */
#define LOCAL_LABELS_DOLLAR 1
#define LOCAL_LABELS_FB 1

/* These define interfaces.  */
#ifdef   OBJ_HEADER
#include OBJ_HEADER
#else
#include "obj-format.h"
#endif

