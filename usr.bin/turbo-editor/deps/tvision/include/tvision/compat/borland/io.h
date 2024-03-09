#ifdef __BORLANDC__
#include <io.h>
#else

#ifdef TVISION_COMPAT_IO_INCNEXT
#undef TVISION_COMPAT_IO_INCNEXT
#include_next <io.h>
#endif // TVISION_COMPAT_IO_INCNEXT

#ifndef TVISION_COMPAT_IO_H
#define TVISION_COMPAT_IO_H

struct  ftime   {
  unsigned    ft_tsec  : 5;   /* Two second interval */
  unsigned    ft_min   : 6;   /* Minutes */
  unsigned    ft_hour  : 5;   /* Hours */
  unsigned    ft_day   : 5;   /* Days */
  unsigned    ft_month : 4;   /* Months */
  unsigned    ft_year  : 7;   /* Year */
};

#ifdef _MSC_VER
#include <corecrt_io.h>
#elif defined(__MINGW32__)
#define TVISION_COMPAT_IO_INCNEXT
#include <io.h>
#undef TVISION_COMPAT_IO_INCNEXT
#elif !defined(_WIN32)

#include <unistd.h>
#include <sys/stat.h>

inline off_t filelength( int fd ) noexcept
{
    struct stat s;
    if ( fstat( fd, &s ) == (off_t) -1 )
        return -1;
    return s.st_size;
}

#endif // !_MSC_VER && !__MINGW32__ && !_WIN32

#endif // TVISION_COMPAT_IO_H

#endif // __BORLANDC__
