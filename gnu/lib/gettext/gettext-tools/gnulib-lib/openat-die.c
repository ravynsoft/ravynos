/* Report a save- or restore-cwd failure in our openat replacement and then exit.

   Copyright (C) 2005-2006, 2008-2023 Free Software Foundation, Inc.

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

#include <config.h>

#include "openat.h"

#include <stdlib.h>

#ifndef GNULIB_LIBPOSIX
# include "error.h"
#endif

#include "exitfail.h"

#include "gettext.h"
#define _(msgid) gettext (msgid)

_Noreturn void
openat_save_fail (int errnum)
{
#ifndef GNULIB_LIBPOSIX
  error (exit_failure, errnum,
         _("unable to record current working directory"));
#endif
  /* _Noreturn cannot be applied to error, since it returns
     when its first argument is 0.  To help compilers understand that this
     function does not return, call abort.  Also, the abort is a
     safety feature if exit_failure is 0 (which shouldn't happen).  */
  abort ();
}


/* Exit with an error about failure to restore the working directory
   during an openat emulation.  The caller must ensure that fd 2 is
   not a just-opened fd, even when openat_safer is not in use.  */

_Noreturn void
openat_restore_fail (int errnum)
{
#ifndef GNULIB_LIBPOSIX
  error (exit_failure, errnum,
         _("failed to return to initial working directory"));
#endif

  /* As above.  */
  abort ();
}
