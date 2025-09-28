/* XML resource locating rules
   Copyright (C) 2015, 2018 Free Software Foundation, Inc.

   This file was written by Daiki Ueno <ueno@gnu.org>, 2015.

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

#ifndef _LOCATING_RULE_H
#define _LOCATING_RULE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct locating_rule_list_ty locating_rule_list_ty;

/* Creates a fresh locating_rule_list_ty.  */
extern struct locating_rule_list_ty *locating_rule_list_alloc (void);

extern bool
       locating_rule_list_add_from_directory (locating_rule_list_ty *rules,
                                              const char *directory);

/* Determines the location of resource associated with FILENAME,
   accoding to the loaded locating rules.  */
extern const char *locating_rule_list_locate (locating_rule_list_ty *rules,
                                              const char *filename,
                                              const char *name);

/* Releases memory allocated for RULES.  */
extern void locating_rule_list_free (locating_rule_list_ty *rules);

#ifdef __cplusplus
}
#endif

#endif  /* _LOCATING_RULE_H */
