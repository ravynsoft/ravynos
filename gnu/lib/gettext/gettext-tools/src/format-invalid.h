/* Common reasons that make a format string invalid.
   Copyright (C) 2003 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* These macros return freshly allocated error message strings, intended
   to be stored in *invalid_reason.  */

#define INVALID_UNTERMINATED_DIRECTIVE() \
  xstrdup (_("The string ends in the middle of a directive."))

#define INVALID_MIXES_NUMBERED_UNNUMBERED() \
  xstrdup (_("The string refers to arguments both through absolute argument numbers and through unnumbered argument specifications."))

#define INVALID_ARGNO_0(directive_number) \
  xasprintf (_("In the directive number %u, the argument number 0 is not a positive integer."), directive_number)
#define INVALID_WIDTH_ARGNO_0(directive_number) \
  xasprintf (_("In the directive number %u, the width's argument number 0 is not a positive integer."), directive_number)
#define INVALID_PRECISION_ARGNO_0(directive_number) \
  xasprintf (_("In the directive number %u, the precision's argument number 0 is not a positive integer."), directive_number)

#define INVALID_CONVERSION_SPECIFIER(directive_number,conv_char) \
  (c_isprint (conv_char) \
   ? xasprintf (_("In the directive number %u, the character '%c' is not a valid conversion specifier."), directive_number, conv_char) \
   : xasprintf (_("The character that terminates the directive number %u is not a valid conversion specifier."), directive_number))

#define INVALID_INCOMPATIBLE_ARG_TYPES(arg_number) \
  xasprintf (_("The string refers to argument number %u in incompatible ways."), arg_number)
