/* Color and styling handling.
   Copyright (C) 2006-2008, 2019-2020, 2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Specification.  */
#include "color.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "term-ostream.h"
#include "xalloc.h"
#include "filename.h"
#include "concat-filename.h"


/* Whether to output a test page.  */
bool color_test_mode;

/* Color option.  */
enum color_option color_mode = color_tty;

/* Style to use when coloring.  */
const char *style_file_name;

/* --color argument handling.  Return an error indicator.  */
bool
handle_color_option (const char *option)
{
  if (option != NULL)
    {
      if (strcmp (option, "never") == 0 || strcmp (option, "no") == 0)
        color_mode = color_no;
      else if (strcmp (option, "auto") == 0 || strcmp (option, "tty") == 0)
        color_mode = color_tty;
      else if (strcmp (option, "always") == 0 || strcmp (option, "yes") == 0)
        color_mode = color_yes;
      else if (strcmp (option, "html") == 0)
        color_mode = color_html;
      else if (strcmp (option, "test") == 0)
        color_test_mode = true;
      else
        {
          fprintf (stderr, "invalid --color argument: %s\n", option);
          return true;
        }
    }
  else
    /* --color is equivalent to --color=yes.  */
    color_mode = color_yes;
  return false;
}

/* --style argument handling.  */
void
handle_style_option (const char *option)
{
  style_file_name = option;
}

/* Print a color test page.  */
void
print_color_test ()
{
  /* Code copied from test-term-ostream.c.  */
  static struct { const char *name; term_color_t c; int r; int g; int b; }
         colors[] =
    {
      { "black",   -2,   0,   0,   0 },
      { "blue",    -2,   0,   0, 255 },
      { "green",   -2,   0, 255,   0 },
      { "cyan",    -2,   0, 255, 255 },
      { "red",     -2, 255,   0,   0 },
      { "magenta", -2, 255,   0, 255 },
      { "yellow",  -2, 255, 255,   0 },
      { "white",   -2, 255, 255, 255 },
      { "default", COLOR_DEFAULT, /* unused: */ -1, -1, -1 }
    };
  term_ostream_t stream;
  int i, row, col;

  stream = term_ostream_create (1, "stdout", TTYCTL_AUTO);

  for (i = 0; i < 8; i++)
    colors[i].c =
      term_ostream_rgb_to_color (stream, colors[i].r, colors[i].g, colors[i].b);

  ostream_write_str (stream, "Colors (foreground/background):\n");
  ostream_write_str (stream, "       ");
  for (col = 0; col <= 8; col++)
    {
      const char *name = colors[col].name;
      ostream_write_str (stream, "|");
      ostream_write_str (stream, name);
      ostream_write_mem (stream, "        ", 7 - strlen (name));
    }
  ostream_write_str (stream, "\n");
  for (row = 0; row <= 8; row++)
    {
      const char *name = colors[row].name;
      ostream_write_str (stream, name);
      ostream_write_mem (stream, "        ", 7 - strlen (name));
      for (col = 0; col <= 8; col++)
        {
          term_color_t row_color = colors[row].c;
          term_color_t col_color = colors[col].c;

          ostream_write_str (stream, "|");
          term_ostream_set_color (stream, row_color);
          term_ostream_set_bgcolor (stream, col_color);
          if (!(term_ostream_get_color (stream) == row_color
                && term_ostream_get_bgcolor (stream) == col_color))
            abort ();
          ostream_write_str (stream, " Words ");
          term_ostream_set_color (stream, COLOR_DEFAULT);
          term_ostream_set_bgcolor (stream, COLOR_DEFAULT);
          if (!(term_ostream_get_color (stream) == COLOR_DEFAULT
                && term_ostream_get_bgcolor (stream) == COLOR_DEFAULT))
            abort ();
        }
      ostream_write_str (stream, "\n");
    }
  ostream_write_str (stream, "\n");

  ostream_write_str (stream, "Colors (hue/saturation):\n");
  /* Hue from 0 to 1.  */
  for (row = 0; row <= 17; row++)
    {
      ostream_write_str (stream, row == 0 ? "red:     " : "         ");
      for (col = 0; col <= 64; col++)
        {
          int r = 255;
          int b = (int) (255.0f / 64.0f * col + 0.5f);
          int g = b + (int) (row / 17.0f * (r - b) + 0.5f);
          term_color_t c = term_ostream_rgb_to_color (stream, r, g, b);
          term_ostream_set_bgcolor (stream, c);
          ostream_write_str (stream, " ");
          term_ostream_set_bgcolor (stream, COLOR_DEFAULT);
        }
      ostream_write_str (stream, "\n");
    }
  /* Hue from 1 to 2.  */
  for (row = 17; row >= 0; row--)
    {
      ostream_write_str (stream, row == 17 ? "yellow:  " : "         ");
      for (col = 0; col <= 64; col++)
        {
          int g = 255;
          int b = (int) (255.0f / 64.0f * col + 0.5f);
          int r = b + (int) (row / 17.0f * (g - b) + 0.5f);
          term_color_t c = term_ostream_rgb_to_color (stream, r, g, b);
          term_ostream_set_bgcolor (stream, c);
          ostream_write_str (stream, " ");
          term_ostream_set_bgcolor (stream, COLOR_DEFAULT);
        }
      ostream_write_str (stream, "\n");
    }
  /* Hue from 2 to 3.  */
  for (row = 0; row <= 17; row++)
    {
      ostream_write_str (stream, row == 0 ? "green:   " : "         ");
      for (col = 0; col <= 64; col++)
        {
          int g = 255;
          int r = (int) (255.0f / 64.0f * col + 0.5f);
          int b = r + (int) (row / 17.0f * (g - r) + 0.5f);
          term_color_t c = term_ostream_rgb_to_color (stream, r, g, b);
          term_ostream_set_bgcolor (stream, c);
          ostream_write_str (stream, " ");
          term_ostream_set_bgcolor (stream, COLOR_DEFAULT);
        }
      ostream_write_str (stream, "\n");
    }
  /* Hue from 3 to 4.  */
  for (row = 17; row >= 0; row--)
    {
      ostream_write_str (stream, row == 17 ? "cyan:    " : "         ");
      for (col = 0; col <= 64; col++)
        {
          int b = 255;
          int r = (int) (255.0f / 64.0f * col + 0.5f);
          int g = r + (int) (row / 17.0f * (b - r) + 0.5f);
          term_color_t c = term_ostream_rgb_to_color (stream, r, g, b);
          term_ostream_set_bgcolor (stream, c);
          ostream_write_str (stream, " ");
          term_ostream_set_bgcolor (stream, COLOR_DEFAULT);
        }
      ostream_write_str (stream, "\n");
    }
  /* Hue from 4 to 5.  */
  for (row = 0; row <= 17; row++)
    {
      ostream_write_str (stream, row == 0 ? "blue:    " : "         ");
      for (col = 0; col <= 64; col++)
        {
          int b = 255;
          int g = (int) (255.0f / 64.0f * col + 0.5f);
          int r = g + (int) (row / 17.0f * (b - g) + 0.5f);
          term_color_t c = term_ostream_rgb_to_color (stream, r, g, b);
          term_ostream_set_bgcolor (stream, c);
          ostream_write_str (stream, " ");
          term_ostream_set_bgcolor (stream, COLOR_DEFAULT);
        }
      ostream_write_str (stream, "\n");
    }
  /* Hue from 5 to 6.  */
  for (row = 17; row >= 0; row--)
    {
      ostream_write_str (stream, row == 17 ? "magenta: " :
                                 row == 0 ? "red:     " : "         ");
      for (col = 0; col <= 64; col++)
        {
          int r = 255;
          int g = (int) (255.0f / 64.0f * col + 0.5f);
          int b = g + (int) (row / 17.0f * (r - g) + 0.5f);
          term_color_t c = term_ostream_rgb_to_color (stream, r, g, b);
          term_ostream_set_bgcolor (stream, c);
          ostream_write_str (stream, " ");
          term_ostream_set_bgcolor (stream, COLOR_DEFAULT);
        }
      ostream_write_str (stream, "\n");
    }
  ostream_write_str (stream, "\n");

  ostream_write_str (stream, "Weights:\n");
  term_ostream_set_weight (stream, WEIGHT_NORMAL);
  if (term_ostream_get_weight (stream) != WEIGHT_NORMAL)
    abort ();
  ostream_write_str (stream, "normal, ");
  term_ostream_set_weight (stream, WEIGHT_BOLD);
  if (term_ostream_get_weight (stream) != WEIGHT_BOLD)
    abort ();
  ostream_write_str (stream, "bold, ");
  term_ostream_set_weight (stream, WEIGHT_DEFAULT);
  if (term_ostream_get_weight (stream) != WEIGHT_DEFAULT)
    abort ();
  ostream_write_str (stream, "default \n");
  ostream_write_str (stream, "\n");

  ostream_write_str (stream, "Postures:\n");
  term_ostream_set_posture (stream, POSTURE_NORMAL);
  if (term_ostream_get_posture (stream) != POSTURE_NORMAL)
    abort ();
  ostream_write_str (stream, "normal, ");
  term_ostream_set_posture (stream, POSTURE_ITALIC);
  if (term_ostream_get_posture (stream) != POSTURE_ITALIC)
    abort ();
  ostream_write_str (stream, "italic, ");
  term_ostream_set_posture (stream, POSTURE_DEFAULT);
  if (term_ostream_get_posture (stream) != POSTURE_DEFAULT)
    abort ();
  ostream_write_str (stream, "default \n");
  ostream_write_str (stream, "\n");

  ostream_write_str (stream, "Text decorations:\n");
  term_ostream_set_underline (stream, UNDERLINE_OFF);
  if (term_ostream_get_underline (stream) != UNDERLINE_OFF)
    abort ();
  ostream_write_str (stream, "normal, ");
  term_ostream_set_underline (stream, UNDERLINE_ON);
  if (term_ostream_get_underline (stream) != UNDERLINE_ON)
    abort ();
  ostream_write_str (stream, "underlined, ");
  term_ostream_set_underline (stream, UNDERLINE_DEFAULT);
  if (term_ostream_get_underline (stream) != UNDERLINE_DEFAULT)
    abort ();
  ostream_write_str (stream, "default \n");
  ostream_write_str (stream, "\n");

  ostream_write_str (stream, "Colors (foreground) mixed with attributes:\n");
  for (row = 0; row <= 8; row++)
    {
      const char *name = colors[row].name;
      ostream_write_str (stream, name);
      ostream_write_mem (stream, "        ", 7 - strlen (name));
      term_ostream_set_color (stream, colors[row].c);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_weight (stream, WEIGHT_BOLD);
      ostream_write_str (stream, "bold");
      term_ostream_set_weight (stream, WEIGHT_NORMAL);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_posture (stream, POSTURE_ITALIC);
      ostream_write_str (stream, "italic");
      term_ostream_set_posture (stream, POSTURE_NORMAL);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_underline (stream, UNDERLINE_ON);
      ostream_write_str (stream, "underlined");
      term_ostream_set_underline (stream, UNDERLINE_OFF);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_color (stream, COLOR_DEFAULT);
      ostream_write_str (stream, "\n       ");
      term_ostream_set_color (stream, colors[row].c);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_weight (stream, WEIGHT_BOLD);
      term_ostream_set_posture (stream, POSTURE_ITALIC);
      ostream_write_str (stream, "bold+italic");
      term_ostream_set_weight (stream, WEIGHT_NORMAL);
      term_ostream_set_posture (stream, POSTURE_NORMAL);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_weight (stream, WEIGHT_BOLD);
      term_ostream_set_underline (stream, UNDERLINE_ON);
      ostream_write_str (stream, "bold+underl");
      term_ostream_set_weight (stream, WEIGHT_NORMAL);
      term_ostream_set_underline (stream, UNDERLINE_OFF);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_posture (stream, POSTURE_ITALIC);
      term_ostream_set_underline (stream, UNDERLINE_ON);
      ostream_write_str (stream, "italic+underl");
      term_ostream_set_posture (stream, POSTURE_NORMAL);
      term_ostream_set_underline (stream, UNDERLINE_OFF);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_color (stream, COLOR_DEFAULT);
      ostream_write_str (stream, "\n");
    }
  ostream_write_str (stream, "\n");

  ostream_write_str (stream, "Colors (background) mixed with attributes:\n");
  for (row = 0; row <= 8; row++)
    {
      const char *name = colors[row].name;
      ostream_write_str (stream, name);
      ostream_write_mem (stream, "        ", 7 - strlen (name));
      term_ostream_set_bgcolor (stream, colors[row].c);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_weight (stream, WEIGHT_BOLD);
      ostream_write_str (stream, "bold");
      term_ostream_set_weight (stream, WEIGHT_NORMAL);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_posture (stream, POSTURE_ITALIC);
      ostream_write_str (stream, "italic");
      term_ostream_set_posture (stream, POSTURE_NORMAL);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_underline (stream, UNDERLINE_ON);
      ostream_write_str (stream, "underlined");
      term_ostream_set_underline (stream, UNDERLINE_OFF);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_bgcolor (stream, COLOR_DEFAULT);
      ostream_write_str (stream, "\n       ");
      term_ostream_set_bgcolor (stream, colors[row].c);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_weight (stream, WEIGHT_BOLD);
      term_ostream_set_posture (stream, POSTURE_ITALIC);
      ostream_write_str (stream, "bold+italic");
      term_ostream_set_weight (stream, WEIGHT_NORMAL);
      term_ostream_set_posture (stream, POSTURE_NORMAL);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_weight (stream, WEIGHT_BOLD);
      term_ostream_set_underline (stream, UNDERLINE_ON);
      ostream_write_str (stream, "bold+underl");
      term_ostream_set_weight (stream, WEIGHT_NORMAL);
      term_ostream_set_underline (stream, UNDERLINE_OFF);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_posture (stream, POSTURE_ITALIC);
      term_ostream_set_underline (stream, UNDERLINE_ON);
      ostream_write_str (stream, "italic+underl");
      term_ostream_set_posture (stream, POSTURE_NORMAL);
      term_ostream_set_underline (stream, UNDERLINE_OFF);
      ostream_write_str (stream, "|normal|");
      term_ostream_set_bgcolor (stream, COLOR_DEFAULT);
      ostream_write_str (stream, "\n");
    }
  ostream_write_str (stream, "\n");

  ostream_free (stream);
}

/* Lookup the location of the style file.  */
static const char *
style_file_lookup (const char *file_name, const char *stylesdir_after_install)
{
  if (!IS_FILE_NAME_WITH_DIR (file_name))
    {
      /* It's a file name without a directory specification.
         If it does not exist in the current directory...  */
      struct stat statbuf;

      if (stat (file_name, &statbuf) < 0)
        {
          /* ... but it exists in the styles installation location...  */
          char *possible_file_name =
            xconcatenated_filename (stylesdir_after_install, file_name, NULL);

          if (stat (possible_file_name, &statbuf) >= 0)
            {
              /* ... then use the file in the styles installation directory.  */
              return possible_file_name;
            }
          free (possible_file_name);
        }

      /* Let the CSS library show a warning.  */
    }
  return file_name;
}

/* Assign a default value to style_file_name if necessary.  */
void
style_file_prepare (const char *style_file_envvar,
                    const char *stylesdir_envvar,
                    const char *stylesdir_after_install,
                    const char *default_style_file)
{
  if (style_file_name == NULL)
    {
      const char *user_preference = getenv (style_file_envvar);

      if (user_preference != NULL && user_preference[0] != '\0')
        style_file_name =
          style_file_lookup (xstrdup (user_preference),
                             stylesdir_after_install);
      else
        {
          const char *stylesdir;

          /* Make it possible to override the default style file location.  This
             is necessary for running the testsuite before "make install".  */
          stylesdir = getenv (stylesdir_envvar);
          if (stylesdir == NULL || stylesdir[0] == '\0')
            stylesdir = stylesdir_after_install;

          style_file_name =
            xconcatenated_filename (stylesdir, default_style_file,
                                   NULL);
        }
    }
  else
    style_file_name =
      style_file_lookup (style_file_name, stylesdir_after_install);
}
