/* Macro for checking that a function declaration is compliant.
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

#ifndef SIGNATURE_CHECK

/* Check that the function FN takes the specified arguments ARGS with
   a return type of RET.  This header is designed to be included after
   <config.h> and the one system header that is supposed to contain
   the function being checked, but prior to any other system headers
   that are necessary for the unit test.  Therefore, this file does
   not include any system headers, nor reference anything outside of
   the macro arguments.  For an example, if foo.h should provide:

   extern int foo (char, float);

   then the unit test named test-foo.c would start out with:

   #include <config.h>
   #include <foo.h>
   #include "signature.h"
   SIGNATURE_CHECK (foo, int, (char, float));
   #include <other.h>
   ...
*/
# define SIGNATURE_CHECK(fn, ret, args) \
  SIGNATURE_CHECK1 (fn, ret, args, __LINE__)

/* Necessary to allow multiple SIGNATURE_CHECK lines in a unit test.
   Note that the checks must not occupy the same line.  */
# define SIGNATURE_CHECK1(fn, ret, args, id) \
  SIGNATURE_CHECK2 (fn, ret, args, id) /* macroexpand line */
# define SIGNATURE_CHECK2(fn, ret, args, id) \
  _GL_UNUSED static ret (*signature_check ## id) args = fn

#endif /* SIGNATURE_CHECK */
