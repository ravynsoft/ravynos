/* Example program for GNU libtextstyle.
   Copyright (C) 2018-2019 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2018.

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

/* Source code of the C program.  */

#include <textstyle.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>

#include "names.c"

/* Returns the record for a given first name, or NULL if not found.  */
static const struct first_name *
find_first_name (const char *name)
{
  size_t i = 0;
  size_t j = sizeof (names) / sizeof (names[0]);
  /* Loop invariants:
     1. j > i,
     2. If name is in the table, then at an index >= i, < j.  */
  while (j - i > 1)
    {
      size_t k = i + (j - i) / 2;
      if (strcmp (names[k].name, name) <= 0)
        i = k;
      else
        j = k;
    }
  /* Here j = i + 1.  */
  if (strcmp (names[i].name, name) == 0)
    return &names[i];
  else
    return NULL;
}

enum sex
{
  UNKNOWN,
  MALE,
  FEMALE
};

/* Returns the known sex for a given first name, or UNKNOWN if unknown.  */
static enum sex
classify_first_name (const char *name)
{
  const struct first_name *p = find_first_name (name);
  if (p)
    {
      if (p->p_boy >= 0.99f)
        return MALE;
      if (p->p_boy <= 0.01f)
        return FEMALE;
    }
  return UNKNOWN;
}

/* Returns the full name of the current user, or NULL if unknown.  */
static const char *
get_fullname (void)
{
  const char *name = getlogin ();
  if (name != NULL)
    {
      struct passwd *pwd = getpwnam (name);
      if (pwd != NULL)
        {
          const char *gecos = pwd->pw_gecos;
          if (gecos != NULL)
            {
              /* Use the part before the first comma.
                 See <https://en.wikipedia.org/wiki/Gecos_field>.  */
              const char *comma = strchr (gecos, ',');
              if (comma != NULL)
                {
                  char *part = (char *) malloc (comma - gecos + 1);
                  if (part != NULL)
                    {
                      memcpy (part, gecos, comma - gecos);
                      part[comma - gecos] = '\0';
                      return part;
                    }
                }
              else
                return gecos;
            }
        }
    }
  return NULL;
}

int
main (int argc, char *argv[])
{
  const char *program_name = argv[0];
  const char *fullname = NULL;
  int i;

  /* Parse the command-line arguments.  */
  for (i = 1; i < argc; i++)
    {
      const char *arg = argv[i];
      if (strncmp (arg, "--color=", 8) == 0)
        handle_color_option (arg + 8);
      else if (strncmp (arg, "--style=", 8) == 0)
        handle_style_option (arg + 8);
      else if (arg[0] == '-')
        {
          fprintf (stderr, "%s: invalid argument: %s\n", program_name, arg);
          exit (1);
        }
      else
        fullname = arg;
    }

  /* Handle the --color=test special argument.  */
  if (color_test_mode)
    {
      print_color_test ();
      exit (0);
    }

  if (color_mode == color_yes
      || (color_mode == color_tty
          && isatty (STDOUT_FILENO)
          && getenv ("NO_COLOR") == NULL)
      || color_mode == color_html)
    {
      /* Find the style file.  */
      style_file_prepare ("HELLO_STYLE", "HELLO_STYLESDIR", STYLESDIR,
                          "hello-default.css");
      /* As a fallback, use the default in the current directory.  */
      {
        struct stat statbuf;

        if (style_file_name == NULL || stat (style_file_name, &statbuf) < 0)
          style_file_name = "hello-default.css";
      }
    }
  else
    /* No styling.  */
    style_file_name = NULL;

  /* Create a terminal output stream that uses this style file.  */
  styled_ostream_t stream =
    (color_mode == color_html
     ? html_styled_ostream_create (file_ostream_create (stdout),
                                   style_file_name)
     : styled_ostream_create (STDOUT_FILENO, "(stdout)", TTYCTL_AUTO,
                              style_file_name));

  /* Determine the full name of the user.  */
  if (fullname == NULL)
    fullname = get_fullname ();
  if (fullname != NULL)
    {
      ostream_write_str (stream, "Hello ");

      /* Associate the entire full name in CSS class 'name'.  */
      styled_ostream_begin_use_class (stream, "name");

      const char *fullname_end = fullname + strlen (fullname);

      /* Determine the extent of the first name in the full name.  */
      const char *firstname_start;
      const char *firstname_end;
      {
        /* The full name can be of the form "FAMILYNAME, FIRSTNAME".  */
        const char *comma = strchr (fullname, ',');
        if (comma != NULL)
          {
            firstname_start = comma + 1;
            while (*firstname_start == ' ')
              firstname_start++;
            firstname_end = fullname_end;
          }
        else
          {
            /* Or it can be of the form "X. FIRSTNAME Y. FAMILYNAME".  */
            firstname_start = fullname;
            for (;;)
              {
                const char *space = strchr (firstname_start, ' ');
                if (space == NULL)
                  {
                    firstname_end = fullname_end;
                    break;
                  }
                if (space == firstname_start || space[-1] == '.')
                  firstname_start = space + 1;
                else
                  {
                    firstname_end = space;
                    break;
                  }
              }
          }
        while (firstname_end > firstname_start && firstname_end[-1] == ' ')
          firstname_end--;
      }

      /* Output the part of the full name before the first name.  */
      ostream_write_mem (stream, fullname, firstname_start - fullname);

      /* Guess the sex, based on the first name.  */
      char *firstname = (char *) malloc (firstname_end - firstname_start + 1);
      memcpy (firstname, firstname_start, firstname_end - firstname_start);
      firstname[firstname_end - firstname_start] = '\0';
      enum sex guessed_sex = classify_first_name (firstname);
      free (firstname);

      /* Associate the first name with the appropriate CSS class.  */
      switch (guessed_sex)
        {
        case MALE:
          styled_ostream_begin_use_class (stream, "boy-name");
          break;
        case FEMALE:
          styled_ostream_begin_use_class (stream, "girl-name");
          break;
        default:
          break;
        }

      /* Output the first name.  */
      ostream_write_mem (stream, firstname_start,
                         firstname_end - firstname_start);

      /* Terminate the first name.  */
      switch (guessed_sex)
        {
        case MALE:
          styled_ostream_end_use_class (stream, "boy-name");
          break;
        case FEMALE:
          styled_ostream_end_use_class (stream, "girl-name");
          break;
        default:
          break;
        }

      /* Output the part of the full name after the first name.  */
      ostream_write_mem (stream, firstname_end, fullname_end - firstname_end);

      /* Terminate the name.  */
      styled_ostream_end_use_class (stream, "name");

      ostream_write_str (stream, "!\n");
    }
  else
    ostream_write_str (stream, "Hello!\n");

  /* Flush and close the terminal stream.  */
  styled_ostream_free (stream);

  return 0;
}
