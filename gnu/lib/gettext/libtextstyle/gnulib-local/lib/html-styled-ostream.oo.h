/* Output stream for CSS styled text, producing HTML output.
   Copyright (C) 2006, 2020 Free Software Foundation, Inc.
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

#ifndef _HTML_STYLED_OSTREAM_H
#define _HTML_STYLED_OSTREAM_H

#include <stdbool.h>

#include "styled-ostream.h"
#include "html-ostream.h"


struct html_styled_ostream : struct styled_ostream
{
methods:
  /* Accessors.  */
  ostream_t      get_destination (html_styled_ostream_t stream);
  html_ostream_t get_html_destination (html_styled_ostream_t stream);
  const char *   get_css_filename (html_styled_ostream_t stream);
};


#ifdef __cplusplus
extern "C" {
#endif


/* Create an output stream that takes input in the UTF-8 encoding and
   writes it in HTML form on DESTINATION, styled with the file CSS_FILENAME.
   Note that the resulting stream must be closed before DESTINATION can be
   closed.  */
extern html_styled_ostream_t
       html_styled_ostream_create (ostream_t destination,
                                   const char *css_filename);


/* Test whether a given output stream is a html_styled_ostream.  */
extern bool is_instance_of_html_styled_ostream (ostream_t stream);


#ifdef __cplusplus
}
#endif

#endif /* _HTML_STYLED_OSTREAM_H */
