/* Abstract output stream for CSS styled text.
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

#ifndef _STYLED_OSTREAM_H
#define _STYLED_OSTREAM_H

#include <stdbool.h>

#include "ostream.h"


/* A styled output stream is an object to which one can feed a sequence of
   bytes, marking some runs of text as belonging to specific CSS classes,
   where the rendering of the CSS classes is defined through a CSS (cascading
   style sheet).  */

struct styled_ostream : struct ostream
{
methods:

  /* Start a run of text belonging to CLASSNAME.  The CLASSNAME is the name
     of a CSS class.  It can be chosen arbitrarily and customized through
     an inline or external CSS.  */
  void begin_use_class (styled_ostream_t stream, const char *classname);

  /* End a run of text belonging to CLASSNAME.
     The begin_use_class / end_use_class calls must match properly.  */
  void end_use_class (styled_ostream_t stream, const char *classname);

  /* Get/set the hyperlink attribute and its id.  */
  const char * get_hyperlink_ref (styled_ostream_t stream);
  const char * get_hyperlink_id (styled_ostream_t stream);
  void         set_hyperlink (styled_ostream_t stream,
                              const char *ref, const char *id);

  /* Like styled_ostream_flush (first_arg, FLUSH_THIS_STREAM), except that it
     leaves the destination with the current text style enabled, instead
     of with the default text style.
     After calling this function, you can output strings without newlines(!)
     to the underlying stream, and they will be rendered like strings passed
     to 'ostream_write_mem', 'ostream_write_str', or 'ostream_write_printf'.  */
  void flush_to_current_style (styled_ostream_t stream);
};

#ifdef __cplusplus
extern "C" {
#endif


/* Test whether a given output stream is a styled_ostream.  */
extern bool is_instance_of_styled_ostream (ostream_t stream);


#ifdef __cplusplus
}
#endif

#endif /* _STYLED_OSTREAM_H */
