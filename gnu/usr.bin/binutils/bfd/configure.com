$!
$! This file configures the bfd library for use with openVMS.
$!
$! We do not use the configure script, since we do not have /bin/sh
$! to execute it.
$!
$! Written by Klaus K"ampf (kkaempf@rmi.de)
$! Rewritten by Tristan Gingold (gingold@adacore.com)
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
$if arch .eqs. "alpha" then target = "alpha"
$if arch .eqs. "ia64" then target = "ia64"
$!
$if (arch .eqs. "alpha") .or. (arch .eqs. "ia64")
$then
$!
$ write sys$output "Configuring BFD for ''target' target"
$!
$!
$! copy bfd-in2.h to bfd.h, replacing @ macros
$!
$ edit/tpu/nojournal/nosection/nodisplay/command=sys$input -
        []bfd-in2.h /output=[]bfd.h
$DECK
!
!  Copy file, changing lines with macros (@@)
!
!
   set (success,off);

   file := CREATE_BUFFER("file", GET_INFO(COMMAND_LINE, "file_name"));
   rang := CREATE_RANGE(BEGINNING_OF(file), END_OF(file));

   match_pos := SEARCH_QUIETLY('@wordsize@', FORWARD, EXACT, rang);
   IF match_pos <> 0 THEN;
      POSITION(BEGINNING_OF(match_pos));
      ERASE(match_pos);
      COPY_TEXT('64');
   ENDIF;
   match_pos := SEARCH_QUIETLY('@bfd_default_target_size@', FORWARD, EXACT, rang);
   IF match_pos <> 0 THEN;
      POSITION(BEGINNING_OF(match_pos));
      ERASE(match_pos);
      COPY_TEXT('64');
   ENDIF;
   match_pos := SEARCH_QUIETLY('@bfd_file_ptr@', FORWARD, EXACT, rang);
   IF match_pos <> 0 THEN;
      POSITION(BEGINNING_OF(match_pos));
      ERASE(match_pos);
      COPY_TEXT('bfd_signed_vma');
   ENDIF;
   match_pos := SEARCH_QUIETLY('@bfd_ufile_ptr@', FORWARD, EXACT, rang);
   IF match_pos <> 0 THEN;
      POSITION(BEGINNING_OF(match_pos));
      ERASE(match_pos);
      COPY_TEXT('bfd_vma');
   ENDIF;
   match_pos := SEARCH_QUIETLY('@supports_plugins@', FORWARD, EXACT, rang);
   IF match_pos <> 0 THEN;
      POSITION(BEGINNING_OF(match_pos));
      ERASE(match_pos);
      COPY_TEXT('0');
   ENDIF;
   WRITE_FILE(file, GET_INFO(COMMAND_LINE, "output_file"));
   QUIT
$  EOD
$
$else
$
$ write sys$output "Configuring for Vax target"
$ target = "vax"
$!
$! copy bfd-in2.h to bfd.h, replacing @ macros
$!
$ write sys$output "Generated `bfd.h' from `bfd-in2.h'."
$ edit/tpu/nojournal/nosection/nodisplay/command=sys$input -
        []bfd-in2.h /output=[]bfd.h
$DECK
!
!  Copy file, changing lines with macros (@@)
!
!
   set (success,off);

   file := CREATE_BUFFER("file", GET_INFO(COMMAND_LINE, "file_name"));
   rang := CREATE_RANGE(BEGINNING_OF(file), END_OF(file));

   match_pos := SEARCH_QUIETLY('@wordsize@', FORWARD, EXACT, rang);
   IF match_pos <> 0 THEN;
      POSITION(BEGINNING_OF(match_pos));
      ERASE(match_pos);
      COPY_TEXT('32');
   ENDIF;
   match_pos := SEARCH_QUIETLY('@bfd_default_target_size@', FORWARD, EXACT, rang);
   IF match_pos <> 0 THEN;
      POSITION(BEGINNING_OF(match_pos));
      ERASE(match_pos);
      COPY_TEXT('32');
   ENDIF;
   match_pos := SEARCH_QUIETLY('@bfd_file_ptr@', FORWARD, EXACT, rang);
   IF match_pos <> 0 THEN;
      POSITION(BEGINNING_OF(match_pos));
      ERASE(match_pos);
      COPY_TEXT('bfd_signed_vma');
   ENDIF;
   match_pos := SEARCH_QUIETLY('@bfd_ufile_ptr@', FORWARD, EXACT, rang);
   IF match_pos <> 0 THEN;
      POSITION(BEGINNING_OF(match_pos));
      ERASE(match_pos);
      COPY_TEXT('bfd_vma');
   ENDIF;
   match_pos := SEARCH_QUIETLY('@supports_plugins@', FORWARD, EXACT, rang);
   IF match_pos <> 0 THEN;
      POSITION(BEGINNING_OF(match_pos));
      ERASE(match_pos);
      COPY_TEXT('0');
   ENDIF;
   WRITE_FILE(file, GET_INFO(COMMAND_LINE, "output_file"));
   QUIT
$  EOD
$endif
$
$!
$! create bfdver.h
$!
$ write sys$output "Generate `bfdver.h' from 'version.h' and `configure.in'."
$ edit/tpu/nojournal/nosection/nodisplay/command=sys$input -
        []version.h /output=[]bfdver.h
$DECK
!
!  Copy file, changing lines with macros (@@)
!
!
   set (success,off);
   vfile := CREATE_BUFFER("vfile", "configure.in");
   rang := CREATE_RANGE(BEGINNING_OF(vfile), END_OF(vfile));
   match_pos := SEARCH_QUIETLY('AC_INIT([bfd], [', FORWARD, EXACT, rang);
   IF match_pos <> 0 THEN;
     POSITION(BEGINNING_OF(match_pos));
     ERASE(match_pos);
     vers := CURRENT_LINE-"])";
   ELSE;
     vers := "unknown";
   ENDIF;
   versnum := vers - "." - ".";

   file := CREATE_BUFFER("file", GET_INFO(COMMAND_LINE, "file_name"));
   rang := CREATE_RANGE(BEGINNING_OF(file), END_OF(file));

   match_pos := SEARCH_QUIETLY('@bfd_version@', FORWARD, EXACT, rang);
   IF match_pos <> 0 THEN;
      POSITION(BEGINNING_OF(match_pos));
      ERASE(match_pos);
      COPY_TEXT(versnum);
   ENDIF;
   match_pos := SEARCH_QUIETLY('@bfd_version_string@', FORWARD, EXACT, rang);
   IF match_pos <> 0 THEN;
      POSITION(BEGINNING_OF(match_pos));
      ERASE(match_pos);
      COPY_TEXT('"');
      COPY_TEXT(vers);
      COPY_TEXT('"');
   ENDIF;
   match_pos := SEARCH_QUIETLY('@bfd_version_package@', FORWARD, EXACT, rang);
   IF match_pos <> 0 THEN;
      POSITION(BEGINNING_OF(match_pos));
      ERASE(match_pos);
      COPY_TEXT('"(GNU Binutils) "');
   ENDIF;
   match_pos := SEARCH_QUIETLY('@report_bugs_to@', FORWARD, EXACT, rang);
   IF match_pos <> 0 THEN;
      POSITION(BEGINNING_OF(match_pos));
      ERASE(match_pos);
      COPY_TEXT('"<https://www.sourceware.org/bugzilla/>"');
   ENDIF;
   WRITE_FILE(file, GET_INFO(COMMAND_LINE, "output_file"));
   QUIT
$  EOD
$!
$!
$! create targmatch.h
$!
$ write sys$output "Generate `targmatch.h'"
$ open/write tfile []targmatch.h
$ write tfile "{ """ + target + "-*-*vms*""" + ","
$ write tfile "#if defined (SELECT_VECS)"
$ write tfile "SELECT_VECS"
$ write tfile "#else"
$ write tfile "UNSUPPORTED_TARGET"
$ write tfile "#endif"
$ write tfile "},"
$ close tfile
$!
$!
$! create config.h
$!
$ write sys$output "Generate `config.h'"
$ create []config.h
/* config.h-vms.  Generated by hand by Klaus Kämpf, kkaempf@didymus.rmi.de.  */
/* config.in.  Generated automatically from configure.in by autoheader.  */
/* Whether malloc must be declared even if <stdlib.h> is included.  */
/* #undef NEED_DECLARATION_MALLOC */
/* Whether free must be declared even if <stdlib.h> is included.  */
/* #undef NEED_DECLARATION_FREE */
/* Define if you have a working `mmap' system call.  */
/* #define HAVE_MMAP 1 */
/* Do we need to use the b modifier when opening binary files?  */
/* #undef USE_BINARY_FOPEN */
/* Name of host specific header file to include in trad-core.c.  */
/* #undef TRAD_HEADER */
/* Define only if <sys/procfs.h> is available *and* it defines prstatus_t.  */
/* #undef HAVE_SYS_PROCFS_H */
/* Do we really want to use mmap if it's available?  */
/* #undef USE_MMAP */
/* Define if you have the fcntl function.  */
#define HAVE_FCNTL 1
/* Define if you have the getpagesize function.  */
#define HAVE_GETPAGESIZE 1
/* Define if you have the madvise function.  */
#define HAVE_MADVISE 1
/* Define if you have the mprotect function.  */
#define HAVE_MPROTECT 1
/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1
/* Define if you have the <stddef.h> header file.  */
#define HAVE_STDDEF_H 1
/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1
/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1
/* Define if you have the <strings.h> header file.  */
#define HAVE_STRINGS_H 1
/* Define if you have the <sys/file.h> header file.  */
#define HAVE_SYS_FILE_H 1
/* Define if you have the <time.h> header file.  */
#define HAVE_TIME_H 1
/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1
/* Disable NLS  */
#undef ENABLE_NLS
/* Name of package */
#define PACKAGE "bfd"
/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""
/* Define to the full name of this package. */
#define PACKAGE_NAME "bfd"
/* Define to the full name and version of this package. */
#define PACKAGE_STRING "bfd"
/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "bfd"
/* Define to the home page for this package. */
#define PACKAGE_URL ""
/* Define to the version of this package. */
#define PACKAGE_VERSION "(package version)"
$!
$ write sys$output "Copy sysdep.h"
$ copy [.hosts]alphavms.h sysdep.h
$
$ write sys$output "Generate build.com"
$!
$ if ARCH.eqs."alpha"
$ then
$   create build.com
$DECK
$ DEFS="""SELECT_VECS=&alpha_vms_vec"","+-
  """SELECT_ARCHITECTURES=&bfd_alpha_arch"""
$ FILES="cpu-alpha,vms,vms-hdr,vms-gsd,vms-tir,vms-misc,"
$EOD
$ endif
$ if ARCH.eqs."ia64"
$ then
$   create build.com
$DECK
$ DEFS="""SELECT_VECS=&ia64_elf64_vms_vec"","+-
  """SELECT_ARCHITECTURES=&bfd_ia64_arch"""
$ FILES="cpu-ia64,elf64-ia64,elf-strtab,corefile,stabs,merge,elf-eh-frame,"+-
  "elflink,elf-attrs,dwarf1,elf64,"
$EOD
$ create substxx.tpu
$DECK
   set (success,off);
   file := CREATE_BUFFER("file", GET_INFO(COMMAND_LINE, "file_name"));
   found_range := CREATE_RANGE(BEGINNING_OF(file), BEGINNING_OF(file));

   LOOP
     rang := CREATE_RANGE (END_OF(found_range),END_OF(file));
     match_pos := SEARCH_QUIETLY('NN', FORWARD, EXACT, rang);
     EXITIF match_pos = 0;
     POSITION(BEGINNING_OF(match_pos));
     ERASE(match_pos);
     COPY_TEXT('64');
   ENDLOOP;
   WRITE_FILE(file, GET_INFO(COMMAND_LINE, "output_file"));
   QUIT
$  EOD
$ write sys$output "Generate elf64-target.h from elfxx-target.h"
$ edit/tpu/nojournal/nosection/nodisplay/command=substxx.tpu -
        []elfXX-target.h /output=[]elf64-target.h
$ del substxx.tpu;*
$ endif
$ append sys$input build.com
$DECK
$ DEFS=DEFS + ",""unlink=remove"",""DEBUGDIR=""""GNU$DEBUGDIR:"""""""
$ OPT="/noopt/debug"
$ CFLAGS="/name=(as_is,shortened)" + -
  "/include=([],""../"",""../include"")" + -
  "/define=(" + DEFS + ")" + OPT
$ FILES=FILES + "archive,archive64,archures,bfd,bfdio,binary,cache,coffgen,"+-
  "compress,corefile,dwarf2,elf,format,hash,ihex,init,libbfd,linker,"+-
  "opncls,reloc,section,simple,srec,stab-syms,syms,targets,tekhex,verilog"
$ write sys$output "CFLAGS=",CFLAGS
$ cflags_libbfd="/warning=(disable=missingreturn)"
$ cflags_nil=""
$ NUM = 0
$ OBJS=""
$ LOOP:
$   F = F$ELEMENT(NUM,",",FILES)
$   IF F.EQS."," THEN GOTO END
$   eflags_name="cflags_''f'"
$   name_len=f$length(eflags_name)
$   dash_pos=f$locate("-",eflags_name)
$   if dash_pos.ne.name_len
$   then
$     eflags_name['dash_pos,1]:="_"
$     dash_pos=f$locate("-",eflags_name)
$     if dash_pos.ne.name_len then eflags_name['dash_pos,1]:="_"
$   endif
$   if f$type('eflags_name).eqs."" then eflags_name="cflags_nil"
$   eflags='eflags_name
$   write sys$output "Compiling ", F, ".c", eflags
$   cc 'CFLAGS 'eflags 'F.c
$   IF OBJS.NES."" THEN OBJS=OBJS + ","
$   OBJS=OBJS + F + ".obj"
$   NUM = NUM + 1
$   GOTO LOOP
$ END:
$ purge
$ lib/create libbfd 'OBJS
$EOD
