#ifndef __LIBOBJC_SAFEWINDOWS_H_INCLUDED__
#define __LIBOBJC_SAFEWINDOWS_H_INCLUDED__

#pragma push_macro("BOOL")

#ifdef BOOL
#undef BOOL
#endif
#define BOOL _WINBOOL

#include <Windows.h>

// Windows.h defines interface -> struct
#ifdef interface
#undef interface
#endif

#pragma pop_macro("BOOL")

#endif // __LIBOBJC_SAFEWINDOWS_H_INCLUDED__
