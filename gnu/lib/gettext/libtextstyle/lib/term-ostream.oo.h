/* Output stream for attributed text, producing ANSI escape sequences.
   Copyright (C) 2006, 2019-2020 Free Software Foundation, Inc.
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

#ifndef _TERM_OSTREAM_H
#define _TERM_OSTREAM_H

#include <stdbool.h>

#include "ostream.h"


/* Querying and setting of text attributes.
   The stream has a notion of the current text attributes; they apply
   implicitly to all following output.  The attributes are automatically
   reset when the stream is closed.
   Note: Not all terminal types can actually render all attributes adequately.
   For example, xterm cannot render POSTURE_ITALIC nor the combination of
   WEIGHT_BOLD and UNDERLINE_ON.  */

/* Colors are represented by indices >= 0 in a stream dependent format.  */
typedef int term_color_t;
/* The value -1 denotes the default (foreground or background) color.  */
enum
{
  COLOR_DEFAULT = -1  /* unknown */
};

typedef enum
{
  WEIGHT_NORMAL = 0,
  WEIGHT_BOLD,
  WEIGHT_DEFAULT = WEIGHT_NORMAL
} term_weight_t;

typedef enum
{
  POSTURE_NORMAL = 0,
  POSTURE_ITALIC, /* same as oblique */
  POSTURE_DEFAULT = POSTURE_NORMAL
} term_posture_t;

typedef enum
{
  UNDERLINE_OFF = 0,
  UNDERLINE_ON,
  UNDERLINE_DEFAULT = UNDERLINE_OFF
} term_underline_t;

/* Get ttyctl_t.  */
#define term_style_user_data term_ostream_representation
#include "term-style-control.h"

struct term_ostream : struct ostream
{
methods:

  /* Convert an RGB value (red, green, blue in [0..255]) to a color, valid
     for this stream only.  */
  term_color_t rgb_to_color (term_ostream_t stream,
                             int red, int green, int blue);

  /* Get/set the text color.  */
  term_color_t get_color (term_ostream_t stream);
  void         set_color (term_ostream_t stream, term_color_t color);

  /* Get/set the background color.  */
  term_color_t get_bgcolor (term_ostream_t stream);
  void         set_bgcolor (term_ostream_t stream, term_color_t color);

  /* Get/set the font weight.  */
  term_weight_t get_weight (term_ostream_t stream);
  void          set_weight (term_ostream_t stream, term_weight_t weight);

  /* Get/set the font posture.  */
  term_posture_t get_posture (term_ostream_t stream);
  void           set_posture (term_ostream_t stream, term_posture_t posture);

  /* Get/set the text underline decoration.  */
  term_underline_t get_underline (term_ostream_t stream);
  void             set_underline (term_ostream_t stream,
                                  term_underline_t underline);

  /* Get/set the hyperlink attribute and its id.  */
  const char * get_hyperlink_ref (term_ostream_t stream);
  const char * get_hyperlink_id (term_ostream_t stream);
  void         set_hyperlink (term_ostream_t stream,
                              const char *ref, const char *id);

  /* Like term_ostream_flush (first_arg, FLUSH_THIS_STREAM), except that it
     leaves the terminal with the current text attributes enabled, instead of
     with the default text attributes.
     After calling this function, you can output strings without newlines(!)
     to the underlying file descriptor, and they will be rendered like strings
     passed to 'ostream_write_mem', 'ostream_write_str', or
     'ostream_write_printf'.  */
  void flush_to_current_style (term_ostream_t stream);

  /* Accessors.  */
  int          get_descriptor (term_ostream_t stream);
  const char * get_filename (term_ostream_t stream);
  ttyctl_t     get_tty_control (term_ostream_t stream);
  ttyctl_t     get_effective_tty_control (term_ostream_t stream);
};


#ifdef __cplusplus
extern "C" {
#endif


/* Create an output stream referring to the file descriptor FD.
   FILENAME is used only for error messages.
   TTY_CONTROL specifies the amount of control to take over the underlying tty.
   The resulting stream will be line-buffered.
   Note that the resulting stream must be closed before FD can be closed.  */
extern term_ostream_t
       term_ostream_create (int fd, const char *filename, ttyctl_t tty_control);


/* Test whether a given output stream is a term_ostream.  */
extern bool is_instance_of_term_ostream (ostream_t stream);


#ifdef __cplusplus
}
#endif

#endif /* _TERM_OSTREAM_H */
