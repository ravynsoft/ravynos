/* Test of macros for declaring functions as non-returning.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2017.  */

#include <config.h>

#include <noreturn.h>

/* Test _GL_NORETURN_FUNC on function declarations.  */

_GL_NORETURN_FUNC extern void func1 (void);
extern _GL_NORETURN_FUNC void func2 (void);
extern void _GL_NORETURN_FUNC func3 (void);

/* Test _GL_NORETURN_FUNC on function definitions.  */

_GL_NORETURN_FUNC void funcd (void)
{
  for (;;)
    ;
}

/* Test _GL_NORETURN_FUNCPTR.  */

_GL_NORETURN_FUNCPTR void (*func1_ptr) (void) = func1;
_GL_NORETURN_FUNCPTR void (*func2_ptr) (void) = func2;
_GL_NORETURN_FUNCPTR void (*func3_ptr) (void) = func3;
_GL_NORETURN_FUNCPTR void (*funcd_ptr) (void) = funcd;

/* These could also be defined in a separate compilation unit.  */

void func1 (void)
{
  for (;;)
    ;
}

void func2 (void)
{
  for (;;)
    ;
}

void func3 (void)
{
  for (;;)
    ;
}


int
main ()
{
  return 0;
}
