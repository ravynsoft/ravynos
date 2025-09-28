/* xgettext C/C++/ObjectiveC backend.
   Copyright (C) 2001-2003, 2006, 2009, 2014-2015, 2018, 2020, 2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

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


#include <stdio.h>

#include "message.h"
#include "xg-arglist-context.h"


#ifdef __cplusplus
extern "C" {
#endif


#define EXTENSIONS_C \
  { "c",      "C"     },                                                \
  { "h",      "C"     },                                                \
  { "C",      "C++"   },                                                \
  { "c++",    "C++"   },                                                \
  { "cc",     "C++"   },                                                \
  { "cxx",    "C++"   },                                                \
  { "cpp",    "C++"   },                                                \
  { "hh",     "C++"   },                                                \
  { "hxx",    "C++"   },                                                \
  { "hpp",    "C++"   },                                                \
  { "m",      "ObjectiveC" },                                           \

#define SCANNERS_C \
  { "C",                extract_c, NULL,                                \
                        &flag_table_c,                                  \
                        &formatstring_c, NULL },                        \
  { "C++",              extract_cxx, NULL,                              \
                        &flag_table_c,                                  \
                        &formatstring_c, &formatstring_cplusplus_brace }, \
  { "ObjectiveC",       extract_objc, NULL,                             \
                        &flag_table_objc,                               \
                        &formatstring_c, &formatstring_objc },          \
  { "GCC-source",       extract_c, NULL,                                \
                        &flag_table_gcc_internal,                       \
                        &formatstring_gcc_internal, &formatstring_gfc_internal }, \

/* Scan a C file and add its translatable strings to mdlp.  */
extern void extract_c (FILE *fp, const char *real_filename,
                       const char *logical_filename,
                       flag_context_list_table_ty *flag_table,
                       msgdomain_list_ty *mdlp);
/* Scan a C++ file and add its translatable strings to mdlp.  */
extern void extract_cxx (FILE *fp, const char *real_filename,
                         const char *logical_filename,
                         flag_context_list_table_ty *flag_table,
                         msgdomain_list_ty *mdlp);
/* Scan an ObjectiveC file and add its translatable strings to mdlp.  */
extern void extract_objc (FILE *fp, const char *real_filename,
                          const char *logical_filename,
                          flag_context_list_table_ty *flag_table,
                          msgdomain_list_ty *mdlp);


/* Handling of options specific to this language.  */

extern void x_c_extract_all (void);

extern void x_c_keyword (const char *name);
extern void x_objc_keyword (const char *name);

extern void x_c_trigraphs (void);

extern void activate_additional_keywords_kde (void);

extern void init_flag_table_c (void);
extern void init_flag_table_objc (void);
extern void init_flag_table_gcc_internal (void);
extern void init_flag_table_kde (void);


#ifdef __cplusplus
}
#endif
