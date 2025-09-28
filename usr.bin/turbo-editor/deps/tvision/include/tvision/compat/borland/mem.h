#ifdef __BORLANDC__
#include <mem.h>
#else

#ifndef TVISION_COMPAT_MEM_H
#define TVISION_COMPAT_MEM_H

#ifdef _MSC_VER
#include <corecrt.h>
#endif

#include "_defs.h"
#include "_null.h"

#include <stddef.h>
#include <string.h>

inline void movmem(const void *src, void *dest, unsigned length) noexcept
{
    memmove(dest, src, length);
}

#endif // TVISION_COMPAT_MEM_H

#endif // __BORLANDC__
