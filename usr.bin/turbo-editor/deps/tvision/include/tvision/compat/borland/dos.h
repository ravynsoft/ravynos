/*  dos.h

    Defines structs, unions, macros, and functions for dealing
    with MSDOS and the Intel iAPX86 microprocessor family.

*/
/*
 *      C/C++ Run Time Library - Version 6.0
 *
 *      Copyright (c) 1987, 1993 by Borland International
 *      All Rights Reserved.
 *
 */

#ifdef __BORLANDC__
#include <dos.h>
#else

#ifdef TVISION_COMPAT_DOS_INCNEXT
#undef TVISION_COMPAT_DOS_INCNEXT
#include_next <dos.h>
#endif // TVISION_COMPAT_DOS_INCNEXT

#ifndef TVISION_COMPAT_DOS_H
#define TVISION_COMPAT_DOS_H

#include "_defs.h"

#ifdef _MSC_VER
#include <corecrt.h>
#elif defined(__MINGW32__)
#define TVISION_COMPAT_DOS_INCNEXT
#include <dos.h>
#undef TVISION_COMPAT_DOS_INCNEXT
#endif

#include <errno.h>
#include <stdint.h>

#define FA_NORMAL   0x00        /* Normal file, no attributes */
#define FA_RDONLY   0x01        /* Read only attribute */
#define FA_HIDDEN   0x02        /* Hidden file */
#define FA_SYSTEM   0x04        /* System file */
#define FA_LABEL    0x08        /* Volume label */
#define FA_DIREC    0x10        /* Directory */
#define FA_ARCH     0x20        /* Archive */

/* MSC names for file attributes */

#define _A_NORMAL   0x00        /* Normal file, no attributes */
#define _A_RDONLY   0x01        /* Read only attribute */
#define _A_HIDDEN   0x02        /* Hidden file */
#define _A_SYSTEM   0x04        /* System file */
#define _A_VOLID    0x08        /* Volume label */
#define _A_SUBDIR   0x10        /* Directory */
#define _A_ARCH     0x20        /* Archive */

#ifndef _FFBLK_DEF
#define _FFBLK_DEF
struct  ffblk   {
    int32_t         ff_reserved;
    int32_t         ff_fsize;
    uint32_t        ff_attrib;
    unsigned short  ff_ftime;
    unsigned short  ff_fdate;
    char            ff_name[256];
};
#endif  /* __FFBLK_DEF */

/* The MSC find_t structure corresponds exactly to the ffblk structure */
struct  find_t {
    int32_t         reserved;
    int32_t         size;              /* size of file */
    uint32_t        attrib;            /* attribute byte for matched file */
    unsigned short  wr_time;           /* time of last write to file */
    unsigned short  wr_date;           /* date of last write to file */
    char            name[256];         /* asciiz name of matched file */
};

#include <stdio.h> // SEEK_SET, SEEK_CUR, SEEK_END

unsigned _dos_findfirst( const char * __path, unsigned __attrib, struct find_t *__finfo ) noexcept;
unsigned _dos_findnext( struct find_t *__finfo ) noexcept;

#endif // TVISION_COMPAT_DOS_H

#endif // __BORLANDC__
