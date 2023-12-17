/* Redirections from public function names to GNU libintl functions.
   Copyright (C) 1995, 2000-2003, 2005, 2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "gettextP.h"

/* @@ end of prolog @@ */


/* This file redirects the gettext functions (without prefix) to those
   defined in the included GNU libintl library (with "libintl_" prefix).
   It is compiled into libintl for three purposes:
     * Packages that bind libintl into other programming languages
       (Python, Perl, PHP, OCaml, Free Pascal Compiler, mailfromd's mail
       filtering language, and many others) bind to the symbols without
       prefix and at the linker level, i.e. without '#include <libintl.h>'.
       Only few packages bind to the symbols with "libintl_" prefix.
     * On glibc systems, we want that existing and future features of
       GNU gettext (such as the logging to $GETTEXT_LOG_UNTRANSLATED)
       become available when the program is linked against -lintl or
       when libintl.so is used LD_PRELOADable library.
     * In order to make the AM_GNU_GETTEXT test of gettext <= 0.11.2 work
       with the libintl library >= 0.11.3 which has the redirections
       primarily in the <libintl.h> include file.
 */

#undef gettext
#undef dgettext
#undef dcgettext
#undef ngettext
#undef dngettext
#undef dcngettext
#undef textdomain
#undef bindtextdomain
#undef bind_textdomain_codeset


/* When building a DLL, we must export some functions.  Note that because
   the functions are only defined for binary backward compatibility, we
   don't need to use __declspec(dllimport) in any case.  */
#if HAVE_VISIBILITY && BUILDING_DLL
# define DLL_EXPORTED __attribute__((__visibility__("default")))
#elif defined _MSC_VER && BUILDING_DLL
/* When building with MSVC, exporting a symbol means that the object file
   contains a "linker directive" of the form /EXPORT:symbol.  This can be
   inspected through the "objdump -s --section=.drectve FILE" or
   "dumpbin /directives FILE" commands.
   The symbols from this file should be exported if and only if the object
   file gets included in a DLL.  Libtool, on Windows platforms, defines
   the C macro DLL_EXPORT (together with PIC) when compiling for a DLL
   and does not define it when compiling an object file meant to be linked
   statically into some executable.  */
# if defined DLL_EXPORT
#  define DLL_EXPORTED __declspec(dllexport)
# else
#  define DLL_EXPORTED
# endif
#else
# define DLL_EXPORTED
#endif


DLL_EXPORTED
char *
gettext (const char *msgid)
{
  return libintl_gettext (msgid);
}


DLL_EXPORTED
char *
dgettext (const char *domainname, const char *msgid)
{
  return libintl_dgettext (domainname, msgid);
}


DLL_EXPORTED
char *
dcgettext (const char *domainname, const char *msgid, int category)
{
  return libintl_dcgettext (domainname, msgid, category);
}


DLL_EXPORTED
char *
ngettext (const char *msgid1, const char *msgid2, unsigned long int n)
{
  return libintl_ngettext (msgid1, msgid2, n);
}


DLL_EXPORTED
char *
dngettext (const char *domainname,
           const char *msgid1, const char *msgid2, unsigned long int n)
{
  return libintl_dngettext (domainname, msgid1, msgid2, n);
}


DLL_EXPORTED
char *
dcngettext (const char *domainname,
            const char *msgid1, const char *msgid2, unsigned long int n,
            int category)
{
  return libintl_dcngettext (domainname, msgid1, msgid2, n, category);
}


DLL_EXPORTED
char *
textdomain (const char *domainname)
{
  return libintl_textdomain (domainname);
}


DLL_EXPORTED
char *
bindtextdomain (const char *domainname, const char *dirname)
{
  return libintl_bindtextdomain (domainname, dirname);
}


DLL_EXPORTED
char *
bind_textdomain_codeset (const char *domainname, const char *codeset)
{
  return libintl_bind_textdomain_codeset (domainname, codeset);
}
