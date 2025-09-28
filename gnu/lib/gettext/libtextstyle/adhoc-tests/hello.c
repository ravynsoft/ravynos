/* Ad-hoc testing program for GNU libtextstyle.
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

#include <config.h>

#include <textstyle.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main (int argc, char *argv[])
{
  const char *program_name = argv[0];
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
        /* Handle non-option arguments here.  */
        ;
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
      /* If no style file is explicitly specified, use the default in the
         source directory.  */
      if (style_file_name == NULL)
        style_file_name = SRCDIR "hello-default.css";
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

  ostream_write_str (stream, "Hello ");

  /* Associate the entire full name in CSS class 'name'.  */
  styled_ostream_begin_use_class (stream, "name");

  ostream_write_str (stream, "Dr. ");
  styled_ostream_begin_use_class (stream, "boy-name");
  /* Start a hyperlink.  */
  styled_ostream_set_hyperlink (stream, "https://en.wikipedia.org/wiki/Linus_Pauling", NULL);
  ostream_write_str (stream, "Linus");
  styled_ostream_end_use_class (stream, "boy-name");
  ostream_write_str (stream, " Pauling");
  /* End the current hyperlink.  */
  styled_ostream_set_hyperlink (stream, NULL, NULL);

  /* Terminate the name.  */
  styled_ostream_end_use_class (stream, "name");

  ostream_write_str (stream, "!\n");

  /* Flush and close the terminal stream.  */
  styled_ostream_free (stream);

  return 0;
}
