/* Macros for the 'type' part of an fopen, freopen or fdopen. 

	<Read|Write>[Update]<Binary file|text file>

   This version is for "same" systems, where text and binary files are
   the same.  An example is Unix.  Many Unix systems could also add a
   "b" to the string, indicating binary files, but some reject this
   (and thereby don't conform to ANSI C, but what else is new?).

   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* This file is designed for inclusion by host-dependent .h files.  No
   user application should include it directly, since that would make
   the application unable to be configured for both "same" and "binary"
   variant systems.  */

#define FOPEN_RB	"r"
#define FOPEN_WB 	"w"
#define FOPEN_AB 	"a"
#define FOPEN_RUB 	"r+"
#define FOPEN_WUB 	"w+"
#define FOPEN_AUB 	"a+"

#define FOPEN_RT	"r"
#define FOPEN_WT 	"w"
#define FOPEN_AT 	"a"
#define FOPEN_RUT 	"r+"
#define FOPEN_WUT 	"w+"
#define FOPEN_AUT 	"a+"
