/* xgettext Ruby backend.
   Copyright (C) 2020 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2020.

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


#define EXTENSIONS_RUBY \
  { "rb",     "Ruby"  },                                                \

#define SCANNERS_RUBY \
  { "Ruby",             NULL, extract_ruby,                             \
                        &flag_table_ruby, &formatstring_ruby, NULL },   \

extern void extract_ruby (const char *found_in_dir, const char *real_filename,
                          const char *logical_filename,
                          flag_context_list_table_ty *flag_table,
                          msgdomain_list_ty *mdlp);

extern void x_ruby_keyword (const char *keyword);
extern void x_ruby_extract_all (void);

extern void init_flag_table_ruby (void);


#ifdef __cplusplus
}
#endif
