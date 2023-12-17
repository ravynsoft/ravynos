/* Reading XML files.
   Copyright (C) 1995-1998, 2000-2003, 2005-2006, 2008-2009, 2014-2016 Free
   Software Foundation, Inc.
   This file was written by Daiki Ueno <ueno@gnu.org>.

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

#ifndef _WRITE_XML_H
#define _WRITE_XML_H

#include "its.h"
#include "msgfmt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Write an XML file.  mlp is a list containing the messages
   to be output.  locale_name is the locale name.  template_file_name
   is the template file.  file_name is the output file.  Return 0 if
   ok, nonzero on error.  */
extern int
       msgdomain_write_xml (message_list_ty *mlp,
                            const char *canon_encoding,
                            const char *locale_name,
                            const char *template_file_name,
                            its_rule_list_ty *its_rules,
                            const char *file_name);

extern int
       msgdomain_write_xml_bulk (msgfmt_operand_list_ty *operands,
                                 const char *template_file_name,
                                 its_rule_list_ty *its_rules,
                                 const char *file_name);

#ifdef __cplusplus
}
#endif


#endif /* _WRITE_XML_H */
