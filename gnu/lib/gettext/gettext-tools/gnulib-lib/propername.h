/* Localization of proper names.  -*- coding: utf-8 -*-
   Copyright (C) 2006, 2008-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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

/* INTRODUCTION

   What do

      Torbjörn Granlund    (coreutils)
      François Pinard      (coreutils)
      Danilo Šegan         (gettext)

   have in common?

   A non-ASCII name. This causes trouble in the --version output. The simple
   "solution" unfortunately mutilates the name.

     $ du --version | grep Granlund
     Écrit par Torbjorn Granlund, David MacKenzie, Paul Eggert et Jim Meyering.

     $ ptx --version | grep Pinard
     Écrit par F. Pinard.

   What is desirable, is to print the full name if the output character set
   allows it, and the ASCIIfied name only as a fallback.

     $ recode-sr-latin --version
     ...
     Written by Danilo Šegan and Bruno Haible.

     $ LC_ALL=C recode-sr-latin --version
     ...
     Written by Danilo Segan and Bruno Haible.

   The 'propername' and 'propername-lite' modules do this. Plus, for
   languages that do not use the Latin alphabet, they allow a translator
   to write the name using that different writing system. In that case the
   propername and propername_utf8 output will look like this:
      <translated name> (<original name in English>)
   whereas the propername_lite output will just be the translated name
   if available, otherwise the original name (in UTF-8 if possible and
   in ASCII if not).

   To use the 'propername' module requires two simple steps:

     1) Add it to the list of gnulib modules to import,

     2) Change the arguments of version_etc(),

          from "Paul Eggert"
          to   proper_name ("Paul Eggert")

          from "Torbjorn Granlund"
          to   proper_name_utf8 ("Torbjorn Granlund", "Torbj\303\266rn Granlund")
          or   proper_name_lite ("Torbjorn Granlund", "Torbj\303\266rn Granlund")

          from "F. Pinard"
          to   proper_name_utf8 ("Franc,ois Pinard", "Fran\303\247ois Pinard")
          or   proper_name_lite ("Franc,ois Pinard", "Fran\303\247ois Pinard")

        In source code, the second argument of proper_name_lite and
        proper_name_utf8 should use octal escapes, not UTF-8 - e.g.,
        "Fran\303\247ois Pinard", not "François Pinard".  Doing it
        this way can avoid mishandling non-ASCII characters if the
        source is recoded to non-UTF-8, or if the compiler does not
        treat UTF-8 as-is in character string contents.

        (Optionally, here you can also add / * TRANSLATORS: ... * / comments
        explaining how the name is written or pronounced.)

   Here is an example in context.

              char const *author_names[2] = {
                / * TRANSLATORS: This is the proper name "Danilo Šegan".
                    In the original Cyrillic it is "Данило Шеган".  * /
                proper_name_utf8 ("Danilo Segan", "Danilo \305\240egan"),
                proper_name ("Bruno Haible")
              };

   Differences between proper_name_utf8 and proper_name_lite:
   * proper_name_lite uses the localization provided by the translator.
     If there is no localization, it uses the name with Unicode characters
     only in UTF-8 locales, otherwise it uses the original name in English.
   * proper_name_utf8 is more elaborate:
     - It uses the name with Unicode characters also when the locale encoding
       is not UTF-8 but contains the necessary characters (e.g. ISO-8859-x or
       GB18030).
     - If there is a localization, it produces a better result when the
       translator has given a poor localization.
 */

#ifndef _PROPERNAME_H
#define _PROPERNAME_H


#ifdef __cplusplus
extern "C" {
#endif

/* Return the localization of NAME.  NAME is written in ASCII.  */
extern const char * proper_name (const char *name) /* NOT attribute const */;

/* Return the localization of a name whose original writing is not ASCII.
   NAME_UTF8 is the real name, written in UTF-8 with octal or hexadecimal
   escape sequences.  NAME_ASCII is a fallback written only with ASCII
   characters.  */
extern const char * proper_name_utf8 (const char *name_ascii,
                                      const char *name_utf8);

/* Return the localization of the name spelled NAME_ASCII in ASCII,
   and NAME_UTF8 in UTF-8.  This function needs less infrastructure
   than proper_name and proper_name_utf8.  */
extern const char *proper_name_lite (const char *name_ascii,
                                     const char *name_utf8);

#ifdef __cplusplus
}
#endif


#endif /* _PROPERNAME_H */
