/* Test for the term-ostream API.  */

#include <config.h>

#include "term-ostream.h"

#include <stdlib.h>
#include <string.h>

int
main ()
{
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
      { "default", COLOR_DEFAULT }
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

  ostream_free (stream);

  return 0;
}
