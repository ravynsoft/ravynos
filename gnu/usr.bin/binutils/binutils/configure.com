$!
$! This file configures binutils for use with openVMS/Alpha
$! We do not use the configure script, since we do not have /bin/sh
$! to execute it.
$!
$!   Copyright (C) 2012-2023 Free Software Foundation, Inc.
$!
$! This file is free software; you can redistribute it and/or modify
$! it under the terms of the GNU General Public License as published by
$! the Free Software Foundation; either version 3 of the License, or
$! (at your option) any later version.
$!
$! This program is distributed in the hope that it will be useful,
$! but WITHOUT ANY WARRANTY; without even the implied warranty of
$! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
$! GNU General Public License for more details.
$!
$! You should have received a copy of the GNU General Public License
$! along with this program; see the file COPYING3.  If not see
$! <http://www.gnu.org/licenses/>.
$!
$! Written by Klaus K"ampf (kkaempf@rmi.de)
$!
$ arch=F$GETSYI("ARCH_NAME")
$ arch=F$EDIT(arch,"LOWERCASE")
$ write sys$output "Configuring binutils for ''arch' target"
$!
$! Generate config.h
$!
$ create config.h
/* config.h.  Generated automatically by configure.com  */
/* Is the type time_t defined in <time.h>?  */
#define HAVE_TIME_T_IN_TIME_H 1
/* Is the type time_t defined in <sys/types.h>?  */
#define HAVE_TIME_T_IN_TYPES_H 1
/* Does <utime.h> define struct utimbuf?  */
#define HAVE_GOOD_UTIME_H 1
/* Whether fprintf must be declared even if <stdio.h> is included.  */
#define NEED_DECLARATION_FPRINTF 1
/* Do we need to use the b modifier when opening binary files?  */
/* #undef USE_BINARY_FOPEN */
/* Define if you have the utimes function.  */
#define HAVE_UTIMES 1
/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1
/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1
/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1
/* Define if you have the <strings.h> header file.  */
#define HAVE_STRINGS_H 1
/* Define if you have the <sys/file.h> header file.  */
#define HAVE_SYS_FILE_H 1
/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Alloca.  */
#ifdef __DECC
#include <builtins.h>
#define C_alloca(x) __ALLOCA(x)
#endif
/* Name of package */
#define PACKAGE "binutils"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""
$!
$! Add TARGET.
$!
$ if arch .eqs. "ia64" then target = "elf64-ia64-vms"
$ if arch .eqs. "alpha" then target = "vms-alpha"
$ if arch .eqs. "vax" then target = "vms-vax"
$!
$ open/append tfile config.h
$ write tfile "#define TARGET """ + target + """"
$ close tfile
$ write sys$output "Created `config.h'"
$!
$ write sys$output "Generate binutils build.com"
$!
$ create build.com
$DECK
$ DEFS=""
$ OPT="/noopt/debug"
$ CFLAGS=OPT + "/include=([],""../include"",[-.bfd])" +-
 "/name=(as_is,shortened)" +-
 "/prefix=(all,exc=(""getopt"",""optarg"",""optopt"",""optind"",""opterr""))"
$ BFDLIB = ",[-.bfd]libbfd.olb/lib"
$ LIBIBERTY = ",[-.libiberty]libiberty.olb/lib"
$ OPCODES = ",[-.opcodes]libopcodes.olb/lib"
$ DEBUG_FILES = ",rddbg,debug,stabs,ieee,rdcoff,dwarf"
$ BULIBS_FILES = ",bucomm,version,filemode"
$ ALL_FILES="nm,strings,addr2line,size,objdump,prdbg" +-
   BULIBS_FILES + DEBUG_FILES
$!
$ write sys$output "CFLAGS=",CFLAGS
$ if p1.nes."LINK"
$ then
$   NUM = 0
$   LOOP:
$     F = F$ELEMENT(NUM,",",ALL_FILES)
$     IF F.EQS."," THEN GOTO END
$     write sys$output "Compiling ", F, ".c"
$     cc 'CFLAGS 'F.c
$     NUM = NUM + 1
$     GOTO LOOP
$   END:
$ endif
$ purge
$!
$ write sys$output "Building nm.exe"
$ NM_OBJS="nm.obj" + BULIBS_FILES + BFDLIB + LIBIBERTY
$ link/exe=nm 'NM_OBJS
$!
$ write sys$output "Building strings.exe"
$ STRINGS_OBJS="strings.obj" + BULIBS_FILES + BFDLIB + LIBIBERTY
$ link/exe=strings 'STRINGS_OBJS
$!
$ write sys$output "Building size.exe"
$ SIZE_OBJS="size.obj" + BULIBS_FILES + BFDLIB + LIBIBERTY
$ link/exe=size 'SIZE_OBJS
$!
$ write sys$output "Building addr2line.exe"
$ ADDR2LINE_OBJS="addr2line.obj" + BULIBS_FILES + BFDLIB + LIBIBERTY
$ link/exe=addr2line 'ADDR2LINE_OBJS
$!
$ write sys$output "Building objdump.exe"
$ OBJDUMP_OBJS="objdump.obj,prdbg.obj" + DEBUG_FILES + BULIBS_FILES +-
   BFDLIB + OPCODES + LIBIBERTY
$ link/exe=objdump 'OBJDUMP_OBJS
$EOD
