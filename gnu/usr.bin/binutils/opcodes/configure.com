$!
$! This file configures the opcodes library for use with openVMS.
$!
$! We do not use the configure script, since we do not have /bin/sh
$! to execute it.
$!
$! Written by Tristan Gingold (gingold@adacore.com)
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
$ arch=F$GETSYI("ARCH_NAME")
$ arch=F$EDIT(arch,"LOWERCASE")

$!
$ write sys$output "Generate opcodes/build.com"
$!
$ if arch.eqs."ia64"
$ then
$   create build.com
$DECK
$ FILES="ia64-dis,ia64-opc"
$ DEFS="""ARCH_ia64"""
$EOD
$ endif
$ if arch.eqs."alpha"
$ then
$   create build.com
$DECK
$ FILES="alpha-dis,alpha-opc"
$ DEFS="""ARCH_alpha"""
$EOD
$ endif
$!
$ append sys$input build.com
$DECK
$ FILES=FILES + ",dis-init,dis-buf,disassemble"
$ OPT="/noopt/debug"
$ CFLAGS=OPT + "/include=([],""../include"",[-.bfd])/name=(as_is,shortened)" + -
  "/define=(" + DEFS + ")"
$ write sys$output "CFLAGS=",CFLAGS
$ NUM = 0
$ LOOP:
$   F = F$ELEMENT(NUM,",",FILES)
$   IF F.EQS."," THEN GOTO END
$   write sys$output "Compiling ", F, ".c"
$   cc 'CFLAGS 'F.c
$   NUM = NUM + 1
$   GOTO LOOP
$ END:
$ purge
$ lib/create libopcodes 'FILES
$EOD
$exit
