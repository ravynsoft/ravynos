/*  dir.h

    Defines structures, macros, and functions for dealing with
    directories and pathnames.

*/

/*
 *      C/C++ Run Time Library - Version 6.0
 *
 *      Copyright (c) 1987, 1993 by Borland International
 *      All Rights Reserved.
 *
 */

#ifdef __BORLANDC__
#include <dir.h>
#else

#ifndef TVISION_COMPAT_DIR_H
#define TVISION_COMPAT_DIR_H

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "_defs.h"

#include <stdint.h>

#define WILDCARDS 0x01
#define EXTENSION 0x02
#define FILENAME  0x04
#define DIRECTORY 0x08
#define DRIVE     0x10

#define MAXDRIVE  3

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
#endif

#define MAXPATH   260
#define MAXDIR    256
#define MAXFILE   256
#define MAXEXT    256

int findfirst( const char *__path, struct ffblk *__ffblk, int __attrib ) noexcept;
int findnext( struct ffblk *__ffblk ) noexcept;
void fnmerge( char *__path,
              const char *__drive,
              const char *__dir,
              const char *__name,
              const char *__ext ) noexcept;
int fnsplit( const char *__path,
             char *__drive,
             char *__dir,
             char *__name,
             char *__ext ) noexcept;
int getcurdir( int __drive, char *__directory ) noexcept;
int getdisk( void ) noexcept;
int setdisk( int __drive ) noexcept;

#endif // TVISION_COMPAT_DIR_H

#endif // __BORLANDC__
