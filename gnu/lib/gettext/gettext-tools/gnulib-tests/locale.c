/* Program that prints the names of the categories of the current locale.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2019.  */

#include <config.h>

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

/* We want to use the system's setlocale() function here, not the gnulib
   override.  */
#undef setlocale

/* Specification:
   <https://pubs.opengroup.org/onlinepubs/9699919799/utilities/locale.html>
   Here we implement only the invocation without any command-line options.  */

static const char *
defaulted_getenv (const char *variable)
{
  const char *value = getenv (variable);
  return (value != NULL ? value : "");
}

static void
print_category (int category, const char *variable)
{
  const char *value = defaulted_getenv (variable);
  if (value[0] != '\0' && defaulted_getenv ("LC_ALL")[0] == '\0')
    /* The variable is set in the environment and not overridden by LC_ALL.  */
    printf ("%s=%s\n", variable, value);
  else
    printf ("%s=\"%s\"\n", variable, setlocale (category, NULL));
}

int
main (void)
{
  setlocale (LC_ALL, "");

  printf ("LANG=%s\n", defaulted_getenv ("LANG"));
  print_category (LC_CTYPE, "LC_CTYPE");
  print_category (LC_NUMERIC, "LC_NUMERIC");
  print_category (LC_TIME, "LC_TIME");
  print_category (LC_COLLATE, "LC_COLLATE");
  print_category (LC_MONETARY, "LC_MONETARY");
  print_category (LC_MESSAGES, "LC_MESSAGES");
#ifdef LC_PAPER
  print_category (LC_PAPER, "LC_PAPER");
#endif
#ifdef LC_NAME
  print_category (LC_NAME, "LC_NAME");
#endif
#ifdef LC_ADDRESS
  print_category (LC_ADDRESS, "LC_ADDRESS");
#endif
#ifdef LC_TELEPHONE
  print_category (LC_TELEPHONE, "LC_TELEPHONE");
#endif
#ifdef LC_MEASUREMENT
  print_category (LC_MEASUREMENT, "LC_MEASUREMENT");
#endif
#ifdef LC_IDENTIFICATION
  print_category (LC_IDENTIFICATION, "LC_IDENTIFICATION");
#endif

  printf ("LC_ALL=%s\n", defaulted_getenv ("LC_ALL"));

  return 0;
}
