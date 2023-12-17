/* Macros for declaring functions as non-returning.

   Copyright (C) 2017-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Paul Eggert and Bruno Haible.  */

#ifndef _NORETURN_H
#define _NORETURN_H 1

/* A "non-returning" function is a function which cannot return normally.
   It can transfer control only through longjmp(), throw (in C++), or similar
   mechanisms.

   This file defines two macros _GL_NORETURN_FUNC and _GL_NORETURN_FUNCPTR,
   that declare a function to be non-returning.
   _GL_NORETURN_FUNC is for use in function declarations and function
   definitions.
   _GL_NORETURN_FUNCPTR is for use on function pointers.

   Comparison of this file with <stdnoreturn.h>:
   <stdnoreturn.h> defines a macro (or keyword) _Noreturn that declares
   a function to be non-returning.  _Noreturn is only for use in function
   declarations and function definitions.
   Therefore, if the non-returning functions you have to declare are unlikely
   to be accessed through function pointers, and if the efficiency with C++
   compilers other than g++, clang, MSVC++ is not an issue to you, you can use
   module 'stdnoreturn' instead of this one, and _Noreturn instead of
   _GL_NORETURN_FUNC.
 */

/* Declares that a function is nonreturning.
   Use in C only code:
     _GL_NORETURN_FUNC extern void func (void);
     extern _GL_NORETURN_FUNC void func (void);
     extern void _GL_NORETURN_FUNC func (void);
   Use in C++ only code for a function with C linkage:
     extern "C" { _GL_NORETURN_FUNC void func (void); }
   Use in C++ only code for a function with C++ linkage:
     _GL_NORETURN_FUNC extern void func (void);
   Use in C & C++ code for a function with C linkage:
     #ifdef __cplusplus
     extern "C" {
     #endif
     _GL_NORETURN_FUNC void func (void);
     #ifdef __cplusplus
     }
     #endif
   Use in C & C++ code for a function with current language linkage:
     _GL_NORETURN_FUNC extern void func (void);
 */
#if (3 <= __GNUC__ || (__GNUC__ == 2 && 8 <= __GNUC_MINOR__)) \
    || defined __clang__ \
    || (0x5110 <= __SUNPRO_C)
  /* For compatibility with _GL_NORETURN_FUNCPTR on clang, use
     __attribute__((__noreturn__)), not _Noreturn.  */
# define _GL_NORETURN_FUNC __attribute__ ((__noreturn__))
#elif 1200 <= _MSC_VER
  /* Use MSVC specific syntax.  */
# define _GL_NORETURN_FUNC __declspec (noreturn)
#elif defined __cplusplus
  /* Use ISO C++11 syntax when the compiler supports it.  */
# if (__cplusplus >= 201103 && !(__GNUC__ == 4 && __GNUC_MINOR__ == 7)) \
     || (_MSC_VER >= 1900)
#  define _GL_NORETURN_FUNC [[noreturn]]
  /* clang++ supports the _Noreturn keyword, but g++ doesn't.  */
# elif defined __clang__
#  define _GL_NORETURN_FUNC _Noreturn
# else
#  define _GL_NORETURN_FUNC /* empty */
# endif
#else
  /* Use ISO C11 syntax when the compiler supports it.  */
# if __STDC_VERSION__ >= 201112 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)
#  define _GL_NORETURN_FUNC _Noreturn
# else
#  define _GL_NORETURN_FUNC /* empty */
# endif
#endif

/* Declares that a function is nonreturning.
   Use in types and declarations that involve function pointers:
     _GL_NORETURN_FUNCPTR void (*funcptr) (void);
 */
#if (3 <= __GNUC__ || (__GNUC__ == 2 && 8 <= __GNUC_MINOR__)) \
    || defined __clang__ \
    || (0x5110 <= __SUNPRO_C)
# define _GL_NORETURN_FUNCPTR __attribute__ ((__noreturn__))
#else
# define _GL_NORETURN_FUNCPTR /* empty */
#endif

/* Comments about the compiler dependent language features:
   - '_Noreturn'    - standardized by ISO C11, available in C only (not in C++),
                      and not applicable to pointers.
   - '[[noreturn]]' - standardized by ISO C++11, available in C++ only,
                      and not applicable to pointers.
   - '__attribute__((__noreturn__))' - available in GCC and clang only,
                                       both in C and C++.
   - '__declspec(noreturn)' - available in MSVC only,
                              both in C and C++, not applicable to pointers.
 */

#endif /* _NORETURN_H */
