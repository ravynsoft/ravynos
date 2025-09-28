/* GNU gettext - internationalization aids
   Copyright (C) 1995-1998, 2000-2003, 2006, 2008, 2019, 2021 Free Software Foundation, Inc.

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

#ifndef _WRITE_CATALOG_H
#define _WRITE_CATALOG_H

#include <stdbool.h>

#include <textstyle.h>

#include "message.h"


#ifdef __cplusplus
extern "C" {
#endif


/* This structure describes a textual catalog output format.  */
struct catalog_output_format
{
  /* Outputs a list of domains of messages to a stream.  */
  void (*print) (msgdomain_list_ty *mdlp, ostream_t stream, size_t page_width, bool debug);

  /* Whether the print function requires the MDLP to be encoded in UTF-8
     encoding.  */
  bool requires_utf8;

  /* Whether the print function uses Unicode control characters to protect
     against filenames with spaces.  */
  bool requires_utf8_for_filenames_with_spaces;

  /* Whether the print function supports styled output.  */
  bool supports_color;

  /* Whether the format supports multiple domains in a single file.  */
  bool supports_multiple_domains;

  /* Whether the format supports contexts.  */
  bool supports_contexts;

  /* Whether the format supports plurals.  */
  bool supports_plurals;

  /* Whether the formats sorts obsolete messages at the end.  */
  bool sorts_obsoletes_to_end;

  /* Whether the PO file format is a suitable alternative output format for
     this one.  */
  bool alternative_is_po;

  /* Whether a Java class is a suitable alternative output format for this
     one.  */
  bool alternative_is_java_class;
};

typedef const struct catalog_output_format * catalog_output_format_ty;

/* These functions set some parameters for use by 'msgdomain_list_print'.  */
extern void
       message_page_width_set (size_t width);

/* Output MDLP into a PO file with the given FILENAME, according to the
   parameters set by the functions above.  */
extern void
       msgdomain_list_print (msgdomain_list_ty *mdlp,
                             const char *filename,
                             catalog_output_format_ty output_syntax,
                             bool force, bool debug);

/* Sort MDLP destructively according to the given criterion.  */
extern void
       msgdomain_list_sort_by_msgid (msgdomain_list_ty *mdlp);
extern void
       msgdomain_list_sort_by_filepos (msgdomain_list_ty *mdlp);


#ifdef __cplusplus
}
#endif


#endif /* _WRITE_CATALOG_H */
