/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/*
 *  This file describes the enum's, etc. shared by the collector control
 *	class and libcollector and its modules.  It is #included in collctrl.h
 *	so any changes to it should follow the procedure described there.
 */

#ifndef _CC_LIBCOLLECTOR_H
#define _CC_LIBCOLLECTOR_H

/* definitions for synchronization tracing scope -- a bit mask */
#define SYNCSCOPE_NATIVE    0x1
#define SYNCSCOPE_JAVA      0x2

typedef enum
{
  FOLLOW_NONE  = 0x0,
  FOLLOW_EXEC  = 0x1,
  FOLLOW_FORK  = 0x2,
  FOLLOW_ON    = 0x3,
  FOLLOW_COMBO = 0x4,
  FOLLOW_ALL   = 0x7
} Follow_type;

#endif /* !__CC_LIBCOLLECTOR_H */
