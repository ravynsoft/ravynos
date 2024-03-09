#ifndef TVISION_COMPAT_MALLOC_H
#define TVISION_COMPAT_MALLOC_H

#include <stdlib.h>

#if __has_include(<alloca.h>)
#include <alloca.h>
#endif

// Also include extensions, just in case.

#if __has_include(<malloc_np.h>)
#include <malloc_np.h>
#endif

#endif // TVISION_COMPAT_MALLOC_H
