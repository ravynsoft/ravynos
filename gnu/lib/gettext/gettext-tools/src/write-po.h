/* GNU gettext - internationalization aids
   Copyright (C) 1995-1998, 2000-2003, 2006, 2008, 2014, 2018-2019, 2021, 2023 Free Software Foundation, Inc.

   This file was written by Peter Miller <millerp@canb.auug.org.au>

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

#ifndef _WRITE_PO_H
#define _WRITE_PO_H

#include <stdbool.h>

#include <textstyle.h>

#include "attribute.h"
#include "message.h"


#ifdef __cplusplus
extern "C" {
#endif


enum filepos_comment_type
  {
    filepos_comment_none,
    filepos_comment_full,
    filepos_comment_file
  };

/* These functions are used to output a #, flags line.  */
extern const char *
       make_format_description_string (enum is_format is_format,
                                       const char *lang, bool debug);
extern bool
       significant_format_p (enum is_format is_format);

extern char *
       make_range_description_string (struct argument_range range)
       ATTRIBUTE_MALLOC;

/* These functions output parts of a message, as comments.  */
extern void
       message_print_comment (const message_ty *mp, ostream_t stream);
extern void
       message_print_comment_dot (const message_ty *mp, ostream_t stream);
extern void
       message_print_comment_filepos (const message_ty *mp, ostream_t stream,
                                      const char *charset, bool uniforum,
                                      size_t page_width);
extern void
       message_print_comment_flags (const message_ty *mp, ostream_t stream,
                                    bool debug);

/* These functions set some parameters for use by 'output_format_po.print'.  */
extern void
       message_page_width_ignore (void);
extern void
       message_print_style_indent (void);
extern void
       message_print_style_uniforum (void);
extern void
       message_print_style_escape (bool flag);
extern void
       message_print_style_comment (bool flag);
extern void
       message_print_style_filepos (enum filepos_comment_type type);

/* --add-location argument handling.  Return an error indicator.  */
extern bool handle_filepos_comment_option (const char *option);


/* Describes a PO file in .po syntax.  */
extern DLL_VARIABLE const struct catalog_output_format output_format_po;


#ifdef __cplusplus
}
#endif


#endif /* _WRITE_PO_H */
