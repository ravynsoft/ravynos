/* Test of <alignof.h>.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2009.  */

#include <config.h>

#include <alignof.h>

#include <stddef.h>
#include <stdint.h>

typedef long double longdouble;
typedef struct { char a[1]; } struct1;
typedef struct { char a[2]; } struct2;
typedef struct { char a[3]; } struct3;
typedef struct { char a[4]; } struct4;

#define CHECK(type) \
  typedef struct { char slot1; type slot2; } type##_helper; \
  static_assert (alignof_slot (type) == offsetof (type##_helper, slot2)); \
  const int type##_slot_alignment = alignof_slot (type); \
  const int type##_type_alignment = alignof_type (type);

CHECK (char)
CHECK (short)
CHECK (int)
CHECK (long)
CHECK (float)
CHECK (double)
CHECK (longdouble)
#ifdef INT64_MAX
CHECK (int64_t)
#endif
CHECK (struct1)
CHECK (struct2)
CHECK (struct3)
CHECK (struct4)

int
main ()
{
  return 0;
}
