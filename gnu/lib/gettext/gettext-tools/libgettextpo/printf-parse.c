/* Formatted output to strings.
   Copyright (C) 1999-2000, 2002-2003, 2006-2023 Free Software Foundation, Inc.

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

/* This file can be parametrized with the following macros:
     CHAR_T             The element type of the format string.
     CHAR_T_ONLY_ASCII  Set to 1 to enable verification that all characters
                        in the format string are ASCII.
     DIRECTIVE          Structure denoting a format directive.
                        Depends on CHAR_T.
     DIRECTIVES         Structure denoting the set of format directives of a
                        format string.  Depends on CHAR_T.
     PRINTF_PARSE       Function that parses a format string.
                        Depends on CHAR_T.
     STATIC             Set to 'static' to declare the function static.
     ENABLE_UNISTDIO    Set to 1 to enable the unistdio extensions.  */

#ifndef PRINTF_PARSE
# include <config.h>
#endif

/* Specification.  */
#ifndef PRINTF_PARSE
# include "printf-parse.h"
#endif

/* Default parameters.  */
#ifndef PRINTF_PARSE
# define PRINTF_PARSE printf_parse
# define CHAR_T char
# define DIRECTIVE char_directive
# define DIRECTIVES char_directives
#endif

/* Get size_t, NULL.  */
#include <stddef.h>

/* Get intmax_t.  */
#include <stdint.h>

/* malloc(), realloc(), free().  */
#include <stdlib.h>

/* memcpy().  */
#include <string.h>

/* errno.  */
#include <errno.h>

/* Checked size_t computations.  */
#include "xsize.h"

#if CHAR_T_ONLY_ASCII
/* c_isascii().  */
# include "c-ctype.h"
#endif

#ifdef STATIC
STATIC
#endif
int
PRINTF_PARSE (const CHAR_T *format, DIRECTIVES *d, arguments *a)
{
  const CHAR_T *cp = format;    /* pointer into format */
  size_t arg_posn = 0;          /* number of regular arguments consumed */
  size_t d_allocated;           /* allocated elements of d->dir */
  size_t a_allocated;           /* allocated elements of a->arg */
  size_t max_width_length = 0;
  size_t max_precision_length = 0;

  d->count = 0;
  d_allocated = N_DIRECT_ALLOC_DIRECTIVES;
  d->dir = d->direct_alloc_dir;

  a->count = 0;
  a_allocated = N_DIRECT_ALLOC_ARGUMENTS;
  a->arg = a->direct_alloc_arg;

#define REGISTER_ARG(_index_,_type_) \
  {                                                                     \
    size_t n = (_index_);                                               \
    if (n >= a_allocated)                                               \
      {                                                                 \
        size_t memory_size;                                             \
        argument *memory;                                               \
                                                                        \
        a_allocated = xtimes (a_allocated, 2);                          \
        if (a_allocated <= n)                                           \
          a_allocated = xsum (n, 1);                                    \
        memory_size = xtimes (a_allocated, sizeof (argument));          \
        if (size_overflow_p (memory_size))                              \
          /* Overflow, would lead to out of memory.  */                 \
          goto out_of_memory;                                           \
        memory = (argument *) (a->arg != a->direct_alloc_arg            \
                               ? realloc (a->arg, memory_size)          \
                               : malloc (memory_size));                 \
        if (memory == NULL)                                             \
          /* Out of memory.  */                                         \
          goto out_of_memory;                                           \
        if (a->arg == a->direct_alloc_arg)                              \
          memcpy (memory, a->arg, a->count * sizeof (argument));        \
        a->arg = memory;                                                \
      }                                                                 \
    while (a->count <= n)                                               \
      a->arg[a->count++].type = TYPE_NONE;                              \
    if (a->arg[n].type == TYPE_NONE)                                    \
      a->arg[n].type = (_type_);                                        \
    else if (a->arg[n].type != (_type_))                                \
      /* Ambiguous type for positional argument.  */                    \
      goto error;                                                       \
  }

  while (*cp != '\0')
    {
      CHAR_T c = *cp++;
      if (c == '%')
        {
          size_t arg_index = ARG_NONE;
          DIRECTIVE *dp = &d->dir[d->count]; /* pointer to next directive */

          /* Initialize the next directive.  */
          dp->dir_start = cp - 1;
          dp->flags = 0;
          dp->width_start = NULL;
          dp->width_end = NULL;
          dp->width_arg_index = ARG_NONE;
          dp->precision_start = NULL;
          dp->precision_end = NULL;
          dp->precision_arg_index = ARG_NONE;
          dp->arg_index = ARG_NONE;

          /* Test for positional argument.  */
          if (*cp >= '0' && *cp <= '9')
            {
              const CHAR_T *np;

              for (np = cp; *np >= '0' && *np <= '9'; np++)
                ;
              if (*np == '$')
                {
                  size_t n = 0;

                  for (np = cp; *np >= '0' && *np <= '9'; np++)
                    n = xsum (xtimes (n, 10), *np - '0');
                  if (n == 0)
                    /* Positional argument 0.  */
                    goto error;
                  if (size_overflow_p (n))
                    /* n too large, would lead to out of memory later.  */
                    goto error;
                  arg_index = n - 1;
                  cp = np + 1;
                }
            }

          /* Read the flags.  */
          for (;;)
            {
              if (*cp == '\'')
                {
                  dp->flags |= FLAG_GROUP;
                  cp++;
                }
              else if (*cp == '-')
                {
                  dp->flags |= FLAG_LEFT;
                  cp++;
                }
              else if (*cp == '+')
                {
                  dp->flags |= FLAG_SHOWSIGN;
                  cp++;
                }
              else if (*cp == ' ')
                {
                  dp->flags |= FLAG_SPACE;
                  cp++;
                }
              else if (*cp == '#')
                {
                  dp->flags |= FLAG_ALT;
                  cp++;
                }
              else if (*cp == '0')
                {
                  dp->flags |= FLAG_ZERO;
                  cp++;
                }
#if __GLIBC__ >= 2 && !defined __UCLIBC__
              else if (*cp == 'I')
                {
                  dp->flags |= FLAG_LOCALIZED;
                  cp++;
                }
#endif
              else
                break;
            }

          /* Parse the field width.  */
          if (*cp == '*')
            {
              dp->width_start = cp;
              cp++;
              dp->width_end = cp;
              if (max_width_length < 1)
                max_width_length = 1;

              /* Test for positional argument.  */
              if (*cp >= '0' && *cp <= '9')
                {
                  const CHAR_T *np;

                  for (np = cp; *np >= '0' && *np <= '9'; np++)
                    ;
                  if (*np == '$')
                    {
                      size_t n = 0;

                      for (np = cp; *np >= '0' && *np <= '9'; np++)
                        n = xsum (xtimes (n, 10), *np - '0');
                      if (n == 0)
                        /* Positional argument 0.  */
                        goto error;
                      if (size_overflow_p (n))
                        /* n too large, would lead to out of memory later.  */
                        goto error;
                      dp->width_arg_index = n - 1;
                      cp = np + 1;
                    }
                }
              if (dp->width_arg_index == ARG_NONE)
                {
                  dp->width_arg_index = arg_posn++;
                  if (dp->width_arg_index == ARG_NONE)
                    /* arg_posn wrapped around.  */
                    goto error;
                }
              REGISTER_ARG (dp->width_arg_index, TYPE_INT);
            }
          else if (*cp >= '0' && *cp <= '9')
            {
              size_t width_length;

              dp->width_start = cp;
              for (; *cp >= '0' && *cp <= '9'; cp++)
                ;
              dp->width_end = cp;
              width_length = dp->width_end - dp->width_start;
              if (max_width_length < width_length)
                max_width_length = width_length;
            }

          /* Parse the precision.  */
          if (*cp == '.')
            {
              cp++;
              if (*cp == '*')
                {
                  dp->precision_start = cp - 1;
                  cp++;
                  dp->precision_end = cp;
                  if (max_precision_length < 2)
                    max_precision_length = 2;

                  /* Test for positional argument.  */
                  if (*cp >= '0' && *cp <= '9')
                    {
                      const CHAR_T *np;

                      for (np = cp; *np >= '0' && *np <= '9'; np++)
                        ;
                      if (*np == '$')
                        {
                          size_t n = 0;

                          for (np = cp; *np >= '0' && *np <= '9'; np++)
                            n = xsum (xtimes (n, 10), *np - '0');
                          if (n == 0)
                            /* Positional argument 0.  */
                            goto error;
                          if (size_overflow_p (n))
                            /* n too large, would lead to out of memory
                               later.  */
                            goto error;
                          dp->precision_arg_index = n - 1;
                          cp = np + 1;
                        }
                    }
                  if (dp->precision_arg_index == ARG_NONE)
                    {
                      dp->precision_arg_index = arg_posn++;
                      if (dp->precision_arg_index == ARG_NONE)
                        /* arg_posn wrapped around.  */
                        goto error;
                    }
                  REGISTER_ARG (dp->precision_arg_index, TYPE_INT);
                }
              else
                {
                  size_t precision_length;

                  dp->precision_start = cp - 1;
                  for (; *cp >= '0' && *cp <= '9'; cp++)
                    ;
                  dp->precision_end = cp;
                  precision_length = dp->precision_end - dp->precision_start;
                  if (max_precision_length < precision_length)
                    max_precision_length = precision_length;
                }
            }

          {
            arg_type type;

            /* Parse argument type/size specifiers.  */
            /* Relevant for the conversion characters d, i.  */
            arg_type signed_type = TYPE_INT;
            /* Relevant for the conversion characters b, o, u, x, X.  */
            arg_type unsigned_type = TYPE_UINT;
            /* Relevant for the conversion characters n.  */
            arg_type pointer_type = TYPE_COUNT_INT_POINTER;
            /* Relevant for the conversion characters a, A, e, E, f, F, g, G.  */
            arg_type floatingpoint_type = TYPE_DOUBLE;

            if (*cp == 'h')
              {
                if (cp[1] == 'h')
                  {
                    signed_type = TYPE_SCHAR;
                    unsigned_type = TYPE_UCHAR;
                    pointer_type = TYPE_COUNT_SCHAR_POINTER;
                    cp += 2;
                  }
                else
                  {
                    signed_type = TYPE_SHORT;
                    unsigned_type = TYPE_USHORT;
                    pointer_type = TYPE_COUNT_SHORT_POINTER;
                    cp++;
                  }
              }
            else if (*cp == 'l')
              {
                if (cp[1] == 'l')
                  {
                    signed_type = TYPE_LONGLONGINT;
                    unsigned_type = TYPE_ULONGLONGINT;
                    pointer_type = TYPE_COUNT_LONGLONGINT_POINTER;
                    /* For backward compatibility only.  */
                    floatingpoint_type = TYPE_LONGDOUBLE;
                    cp += 2;
                  }
                else
                  {
                    signed_type = TYPE_LONGINT;
                    unsigned_type = TYPE_ULONGINT;
                    pointer_type = TYPE_COUNT_LONGINT_POINTER;
                    cp++;
                  }
              }
            else if (*cp == 'j')
              {
                if (sizeof (intmax_t) > sizeof (long))
                  {
                    /* intmax_t = long long */
                    signed_type = TYPE_LONGLONGINT;
                    unsigned_type = TYPE_ULONGLONGINT;
                    pointer_type = TYPE_COUNT_LONGLONGINT_POINTER;
                    /* For backward compatibility only.  */
                    floatingpoint_type = TYPE_LONGDOUBLE;
                  }
                else if (sizeof (intmax_t) > sizeof (int))
                  {
                    /* intmax_t = long */
                    signed_type = TYPE_LONGINT;
                    unsigned_type = TYPE_ULONGINT;
                    pointer_type = TYPE_COUNT_LONGINT_POINTER;
                  }
                cp++;
              }
            else if (*cp == 'z' || *cp == 'Z')
              {
                /* 'z' is standardized in ISO C 99, but glibc uses 'Z'
                   because the warning facility in gcc-2.95.2 understands
                   only 'Z' (see gcc-2.95.2/gcc/c-common.c:1784).  */
                if (sizeof (size_t) > sizeof (long))
                  {
                    /* size_t = unsigned long long */
                    signed_type = TYPE_LONGLONGINT;
                    unsigned_type = TYPE_ULONGLONGINT;
                    pointer_type = TYPE_COUNT_LONGLONGINT_POINTER;
                    /* For backward compatibility only.  */
                    floatingpoint_type = TYPE_LONGDOUBLE;
                  }
                else if (sizeof (size_t) > sizeof (int))
                  {
                    /* size_t = unsigned long */
                    signed_type = TYPE_LONGINT;
                    unsigned_type = TYPE_ULONGINT;
                    pointer_type = TYPE_COUNT_LONGINT_POINTER;
                  }
                cp++;
              }
            else if (*cp == 't')
              {
                if (sizeof (ptrdiff_t) > sizeof (long))
                  {
                    /* ptrdiff_t = long long */
                    signed_type = TYPE_LONGLONGINT;
                    unsigned_type = TYPE_ULONGLONGINT;
                    pointer_type = TYPE_COUNT_LONGLONGINT_POINTER;
                    /* For backward compatibility only.  */
                    floatingpoint_type = TYPE_LONGDOUBLE;
                  }
                else if (sizeof (ptrdiff_t) > sizeof (int))
                  {
                    /* ptrdiff_t = long */
                    signed_type = TYPE_LONGINT;
                    unsigned_type = TYPE_ULONGINT;
                    pointer_type = TYPE_COUNT_LONGINT_POINTER;
                  }
                cp++;
              }
            else if (*cp == 'w')
              {
                /* wN and wfN are standardized in ISO C 23.  */
                if (cp[1] == 'f')
                  {
                    if (cp[2] == '8')
                      {
                        signed_type = TYPE_INT_FAST8_T;
                        unsigned_type = TYPE_UINT_FAST8_T;
                        pointer_type = TYPE_COUNT_INT_FAST8_T_POINTER;
                        cp += 3;
                      }
                    else if (cp[2] == '1' && cp[3] == '6')
                      {
                        signed_type = TYPE_INT_FAST16_T;
                        unsigned_type = TYPE_UINT_FAST16_T;
                        pointer_type = TYPE_COUNT_INT_FAST16_T_POINTER;
                        cp += 4;
                      }
                    else if (cp[2] == '3' && cp[3] == '2')
                      {
                        signed_type = TYPE_INT_FAST32_T;
                        unsigned_type = TYPE_UINT_FAST32_T;
                        pointer_type = TYPE_COUNT_INT_FAST32_T_POINTER;
                        cp += 4;
                      }
                    else if (cp[2] == '6' && cp[3] == '4')
                      {
                        signed_type = TYPE_INT_FAST64_T;
                        unsigned_type = TYPE_UINT_FAST64_T;
                        pointer_type = TYPE_COUNT_INT_FAST64_T_POINTER;
                        cp += 4;
                      }
                  }
                else
                  {
                    if (cp[1] == '8')
                      {
                        signed_type = TYPE_INT8_T;
                        unsigned_type = TYPE_UINT8_T;
                        pointer_type = TYPE_COUNT_INT8_T_POINTER;
                        cp += 2;
                      }
                    else if (cp[1] == '1' && cp[2] == '6')
                      {
                        signed_type = TYPE_INT16_T;
                        unsigned_type = TYPE_UINT16_T;
                        pointer_type = TYPE_COUNT_INT16_T_POINTER;
                        cp += 3;
                      }
                    else if (cp[1] == '3' && cp[2] == '2')
                      {
                        signed_type = TYPE_INT32_T;
                        unsigned_type = TYPE_UINT32_T;
                        pointer_type = TYPE_COUNT_INT32_T_POINTER;
                        cp += 3;
                      }
                    else if (cp[1] == '6' && cp[2] == '4')
                      {
                        signed_type = TYPE_INT64_T;
                        unsigned_type = TYPE_UINT64_T;
                        pointer_type = TYPE_COUNT_INT64_T_POINTER;
                        cp += 3;
                      }
                  }
              }
            else if (*cp == 'L')
              {
                signed_type = TYPE_LONGLONGINT;
                unsigned_type = TYPE_ULONGLONGINT;
                pointer_type = TYPE_COUNT_LONGLONGINT_POINTER;
                floatingpoint_type = TYPE_LONGDOUBLE;
                cp++;
              }
#if defined __APPLE__ && defined __MACH__
            /* On Mac OS X 10.3, PRIdMAX is defined as "qd".
               We cannot change it to "lld" because PRIdMAX must also
               be understood by the system's printf routines.  */
            else if (*cp == 'q')
              {
                if (64 / 8 > sizeof (long))
                  {
                    /* int64_t = long long */
                    signed_type = TYPE_LONGLONGINT;
                    unsigned_type = TYPE_ULONGLONGINT;
                    pointer_type = TYPE_COUNT_LONGLONGINT_POINTER;
                    /* For backward compatibility only.  */
                    floatingpoint_type = TYPE_LONGDOUBLE;
                  }
                else
                  {
                    /* int64_t = long */
                    signed_type = TYPE_LONGINT;
                    unsigned_type = TYPE_ULONGINT;
                    pointer_type = TYPE_COUNT_LONGINT_POINTER;
                  }
                cp++;
              }
#endif
#if defined _WIN32 && ! defined __CYGWIN__
            /* On native Windows, PRIdMAX is defined as "I64d".
               We cannot change it to "lld" because PRIdMAX must also
               be understood by the system's printf routines.  */
            else if (*cp == 'I' && cp[1] == '6' && cp[2] == '4')
              {
                if (64 / 8 > sizeof (long))
                  {
                    /* __int64_t = long long */
                    signed_type = TYPE_LONGLONGINT;
                    unsigned_type = TYPE_ULONGLONGINT;
                    pointer_type = TYPE_COUNT_LONGLONGINT_POINTER;
                    /* For backward compatibility only.  */
                    floatingpoint_type = TYPE_LONGDOUBLE;
                  }
                else
                  {
                    /* __int64_t = long */
                    signed_type = TYPE_LONGINT;
                    unsigned_type = TYPE_ULONGINT;
                    pointer_type = TYPE_COUNT_LONGINT_POINTER;
                  }
                cp += 3;
              }
#endif

            /* Read the conversion character.  */
            c = *cp++;
            switch (c)
              {
              case 'd': case 'i':
                type = signed_type;
                break;
              case 'b': case 'o': case 'u': case 'x': case 'X':
              #if SUPPORT_GNU_PRINTF_DIRECTIVES \
                  || (__GLIBC__ + (__GLIBC_MINOR__ >= 35) > 2)
              case 'B':
              #endif
                type = unsigned_type;
                break;
              case 'f': case 'F': case 'e': case 'E': case 'g': case 'G':
              case 'a': case 'A':
                type = floatingpoint_type;
                break;
              case 'c':
                if (signed_type == TYPE_LONGINT
                    /* For backward compatibility only.  */
                    || signed_type == TYPE_LONGLONGINT)
#if HAVE_WINT_T
                  type = TYPE_WIDE_CHAR;
#else
                  goto error;
#endif
                else
                  type = TYPE_CHAR;
                break;
#if HAVE_WINT_T
              case 'C':
                type = TYPE_WIDE_CHAR;
                c = 'c';
                break;
#endif
              case 's':
                if (signed_type == TYPE_LONGINT
                    /* For backward compatibility only.  */
                    || signed_type == TYPE_LONGLONGINT)
#if HAVE_WCHAR_T
                  type = TYPE_WIDE_STRING;
#else
                  goto error;
#endif
                else
                  type = TYPE_STRING;
                break;
#if HAVE_WCHAR_T
              case 'S':
                type = TYPE_WIDE_STRING;
                c = 's';
                break;
#endif
              case 'p':
                type = TYPE_POINTER;
                break;
              case 'n':
                type = pointer_type;
                break;
#if ENABLE_UNISTDIO
              /* The unistdio extensions.  */
              case 'U':
                if (signed_type == TYPE_LONGLONGINT)
                  type = TYPE_U32_STRING;
                else if (signed_type == TYPE_LONGINT)
                  type = TYPE_U16_STRING;
                else
                  type = TYPE_U8_STRING;
                break;
#endif
              case '%':
                type = TYPE_NONE;
                break;
              default:
                /* Unknown conversion character.  */
                goto error;
              }

            if (type != TYPE_NONE)
              {
                dp->arg_index = arg_index;
                if (dp->arg_index == ARG_NONE)
                  {
                    dp->arg_index = arg_posn++;
                    if (dp->arg_index == ARG_NONE)
                      /* arg_posn wrapped around.  */
                      goto error;
                  }
                REGISTER_ARG (dp->arg_index, type);
              }
            dp->conversion = c;
            dp->dir_end = cp;
          }

          d->count++;
          if (d->count >= d_allocated)
            {
              size_t memory_size;
              DIRECTIVE *memory;

              d_allocated = xtimes (d_allocated, 2);
              memory_size = xtimes (d_allocated, sizeof (DIRECTIVE));
              if (size_overflow_p (memory_size))
                /* Overflow, would lead to out of memory.  */
                goto out_of_memory;
              memory = (DIRECTIVE *) (d->dir != d->direct_alloc_dir
                                      ? realloc (d->dir, memory_size)
                                      : malloc (memory_size));
              if (memory == NULL)
                /* Out of memory.  */
                goto out_of_memory;
              if (d->dir == d->direct_alloc_dir)
                memcpy (memory, d->dir, d->count * sizeof (DIRECTIVE));
              d->dir = memory;
            }
        }
#if CHAR_T_ONLY_ASCII
      else if (!c_isascii (c))
        {
          /* Non-ASCII character.  Not supported.  */
          goto error;
        }
#endif
    }
  d->dir[d->count].dir_start = cp;

  d->max_width_length = max_width_length;
  d->max_precision_length = max_precision_length;
  return 0;

error:
  if (a->arg != a->direct_alloc_arg)
    free (a->arg);
  if (d->dir != d->direct_alloc_dir)
    free (d->dir);
  errno = EINVAL;
  return -1;

out_of_memory:
  if (a->arg != a->direct_alloc_arg)
    free (a->arg);
  if (d->dir != d->direct_alloc_dir)
    free (d->dir);
  errno = ENOMEM;
  return -1;
}

#undef PRINTF_PARSE
#undef DIRECTIVES
#undef DIRECTIVE
#undef CHAR_T_ONLY_ASCII
#undef CHAR_T
