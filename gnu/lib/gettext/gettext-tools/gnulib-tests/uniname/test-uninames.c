/* Test the Unicode character name functions.
   Copyright (C) 2000-2003, 2005, 2007, 2009-2023 Free Software Foundation,
   Inc.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xalloc.h"
#include "uniname.h"

/* The names according to the UnicodeData.txt file, modified to contain the
   Hangul syllable names, as described in the Unicode 3.0 book.  */
static const char * unicode_names [0x110000];

/* Maximum entries in unicode_aliases.  */
#define ALIASLEN 0x200

/* The aliases according to the NameAliases.txt file.  */
struct unicode_alias
{
  const char *name;
  unsigned int uc;
};

static struct unicode_alias unicode_aliases [ALIASLEN];
static int aliases_count;

/* Stores in unicode_names[] the relevant contents of the UnicodeData.txt
   file.  */
static void
fill_names (const char *unicodedata_filename)
{
  FILE *stream;
  char *field0;
  char *field1;
  char line[1024];
  int lineno = 0;

  stream = fopen (unicodedata_filename, "r");
  if (stream == NULL)
    {
      fprintf (stderr, "error during fopen of '%s'\n", unicodedata_filename);
      exit (EXIT_FAILURE);
    }

  while (fgets (line, sizeof line, stream))
    {
      char *p;
      char *comment;
      unsigned long i;

      lineno++;

      comment = strchr (line, '#');
      if (comment != NULL)
        *comment = '\0';
      if (line[strspn (line, " \t\r\n")] == '\0')
        continue;

      field0 = p = line;
      p = strchr (p, ';');
      if (!p)
        {
          fprintf (stderr, "short line in '%s':%d\n",
                   unicodedata_filename, lineno);
          exit (EXIT_FAILURE);
        }
      *p++ = '\0';

      field1 = p;
      if (*field1 == '<')
        continue;
      p = strchr (p, ';');
      if (!p)
        {
          fprintf (stderr, "short line in '%s':%d\n",
                   unicodedata_filename, lineno);
          exit (EXIT_FAILURE);
        }
      *p = '\0';
      i = strtoul (field0, NULL, 16);
      if (i >= 0x110000)
        {
          fprintf (stderr, "index too large\n");
          exit (EXIT_FAILURE);
        }
      unicode_names[i] = xstrdup (field1);
    }
  if (ferror (stream) || fclose (stream))
    {
      fprintf (stderr, "error reading from '%s'\n", unicodedata_filename);
      exit (1);
    }
}

/* Stores in unicode_aliases[] the relevant contents of the NameAliases.txt
   file.  */
static void
fill_aliases (const char *namealiases_filename)
{
  FILE *stream;
  char *field0;
  char *field1;
  char line[1024];
  int lineno = 0;

  stream = fopen (namealiases_filename, "r");
  if (stream == NULL)
    {
      fprintf (stderr, "error during fopen of '%s'\n", namealiases_filename);
      exit (EXIT_FAILURE);
    }

  while (fgets (line, sizeof line, stream))
    {
      char *p;
      char *comment;
      unsigned long uc;

      comment = strchr (line, '#');
      if (comment != NULL)
        *comment = '\0';
      if (line[strspn (line, " \t\r\n")] == '\0')
        continue;

      lineno++;

      field0 = p = line;
      p = strchr (p, ';');
      if (!p)
        {
          fprintf (stderr, "short line in '%s':%d\n",
                   namealiases_filename, lineno);
          exit (EXIT_FAILURE);
        }
      *p++ = '\0';

      field1 = p;
      p = strchr (p, ';');
      if (!p)
        {
          fprintf (stderr, "short line in '%s':%d\n",
                   namealiases_filename, lineno);
          exit (EXIT_FAILURE);
        }
      *p = '\0';

      uc = strtoul (field0, NULL, 16);
      if (uc >= 0x110000)
        {
          fprintf (stderr, "index too large\n");
          exit (EXIT_FAILURE);
        }

      if (aliases_count == ALIASLEN)
        {
          fprintf (stderr, "too many aliases\n");
          exit (EXIT_FAILURE);
        }
      unicode_aliases[aliases_count].name = xstrdup (field1);
      unicode_aliases[aliases_count].uc = uc;
      aliases_count++;
    }
  if (ferror (stream) || fclose (stream))
    {
      fprintf (stderr, "error reading from '%s'\n", namealiases_filename);
      exit (1);
    }
}

static int
name_has_alias (unsigned int uc)
{
  int i;
  for (i = 0; i < ALIASLEN; i++)
    if (unicode_aliases[i].uc == uc)
      return 1;
  return 0;
}

/* Perform an exhaustive test of the unicode_character_name function.  */
static int
test_name_lookup ()
{
  int error = 0;
  unsigned int i;
  char buf[UNINAME_MAX];

  for (i = 0; i < 0x11000; i++)
    {
      char *result = unicode_character_name (i, buf);

      if (unicode_names[i] != NULL)
        {
          if (result == NULL)
            {
              fprintf (stderr, "\\u%04X name lookup failed!\n", i);
              error = 1;
            }
          else if (strcmp (result, unicode_names[i]) != 0)
            {
              fprintf (stderr, "\\u%04X name lookup returned wrong name: %s\n",
                               i, result);
              error = 1;
            }
        }
      else
        {
          if (result != NULL)
            {
              fprintf (stderr, "\\u%04X name lookup returned wrong name: %s\n",
                               i, result);
              error = 1;
            }
        }
    }

  for (i = 0x110000; i < 0x1000000; i++)
    {
      char *result = unicode_character_name (i, buf);

      if (result != NULL)
        {
          fprintf (stderr, "\\u%04X name lookup returned wrong name: %s\n",
                           i, result);
          error = 1;
        }
    }

  return error;
}

/* Perform a test of the unicode_name_character function.  */
static int
test_inverse_lookup ()
{
  int error = 0;
  unsigned int i;

  /* First, verify all valid character names are recognized.  */
  for (i = 0; i < 0x110000; i++)
    if (unicode_names[i] != NULL)
      {
        unsigned int result = unicode_name_character (unicode_names[i]);
        if (result != i)
          {
            if (result == UNINAME_INVALID)
              fprintf (stderr, "inverse name lookup of \"%s\" failed\n",
                       unicode_names[i]);
            else
              fprintf (stderr,
                       "inverse name lookup of \"%s\" returned 0x%04X\n",
                       unicode_names[i], result);
            error = 1;
          }
      }

  /* Second, generate random but likely names and verify they are not
     recognized unless really valid.  */
  for (i = 0; i < 10000; i++)
    {
      unsigned int i1, i2;
      const char *s1;
      const char *s2;
      unsigned int l1, l2, j1, j2;
      char buf[2*UNINAME_MAX];
      unsigned int result;

      do i1 = ((rand () % 0x11) << 16)
              + ((rand () & 0xff) << 8)
              + (rand () & 0xff);
      while (unicode_names[i1] == NULL);

      do i2 = ((rand () % 0x11) << 16)
              + ((rand () & 0xff) << 8)
              + (rand () & 0xff);
      while (unicode_names[i2] == NULL);

      s1 = unicode_names[i1];
      l1 = strlen (s1);
      s2 = unicode_names[i2];
      l2 = strlen (s2);

      /* Concatenate a starting piece of s1 with an ending piece of s2.  */
      for (j1 = 1; j1 <= l1; j1++)
        if (j1 == l1 || s1[j1] == ' ')
          for (j2 = 0; j2 < l2; j2++)
            if (j2 == 0 || s2[j2-1] == ' ')
              {
                memcpy (buf, s1, j1);
                buf[j1] = ' ';
                memcpy (buf + j1 + 1, s2 + j2, l2 - j2 + 1);

                result = unicode_name_character (buf);
                if (result != UNINAME_INVALID
                    && !name_has_alias (result)
                    && !(unicode_names[result] != NULL
                         && strcmp (unicode_names[result], buf) == 0))
                  {
                    fprintf (stderr,
                             "inverse name lookup of \"%s\" returned 0x%04X\n",
                             unicode_names[i], result);
                    error = 1;
                  }
              }
    }

  /* Third, some extreme case that used to loop.  */
  if (unicode_name_character ("A A") != UNINAME_INVALID)
    error = 1;

  return error;
}

/* Perform a test of the unicode_name_character function for aliases.  */
static int
test_alias_lookup ()
{
  int error = 0;
  unsigned int i;
  char buf[UNINAME_MAX];

  /* Verify all valid character names are recognized.  */
  for (i = 0; i < ALIASLEN; i++)
    if (unicode_aliases[i].uc != UNINAME_INVALID
        /* Skip if the character has no canonical name (e.g. control
           characters).  */
        && unicode_character_name (unicode_aliases[i].uc, buf))
      {
        unsigned int result = unicode_name_character (unicode_aliases[i].name);
        if (result != unicode_aliases[i].uc)
          {
            if (result == UNINAME_INVALID)
              fprintf (stderr, "inverse name lookup of \"%s\" failed\n",
                       unicode_aliases[i].name);
            else
              fprintf (stderr,
                       "inverse name lookup of \"%s\" returned 0x%04X\n",
                       unicode_aliases[i].name, result);
            error = 1;
          }
      }

  return error;
}

int
main (int argc, char *argv[])
{
  int error = 0;
  int i;

  for (i = 1; i < argc && strcmp (argv[i], "--") != 0; i++)
    fill_names (argv[i]);

  if (i < argc)
    {
      int j;
      for (j = 0; j < ALIASLEN; j++)
        unicode_aliases[j].uc = UNINAME_INVALID;

      i++;
      for (; i < argc; i++)
        fill_aliases (argv[i]);
    }

  error |= test_name_lookup ();
  error |= test_inverse_lookup ();

  if (aliases_count > 0)
    error |= test_alias_lookup ();

  return error;
}
