/* Unicode CLDR plural rule parser and converter
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

#ifndef _CLDR_PLURAL_EXP_H
#define _CLDR_PLURAL_EXP_H 1

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

enum cldr_plural_operand
  {
    CLDR_PLURAL_OPERAND_INTEGER,
    CLDR_PLURAL_OPERAND_DECIMAL
  };

struct cldr_plural_operand_ty
{
  enum cldr_plural_operand type;
  union
  {
    int ival;
    struct
    {
      double d;
      int nfractions;
    } dval;
  } value;
};

enum cldr_plural_relation
  {
    CLDR_PLURAL_RELATION_EQUAL,
    CLDR_PLURAL_RELATION_NOT_EQUAL
  };

struct cldr_plural_range_ty
{
  struct cldr_plural_operand_ty *start;
  struct cldr_plural_operand_ty *end;
};

struct cldr_plural_range_list_ty
{
  struct cldr_plural_range_ty **items;
  size_t nitems;
  size_t nitems_max;
};

struct cldr_plural_expression_ty
{
  /* 'n', 'i', 'f', 't', 'v', 'w' */
  int operand;

  /* 0 if not given */
  int mod;
};

struct cldr_plural_relation_ty
{
  struct cldr_plural_expression_ty *expression;
  enum cldr_plural_relation type;
  struct cldr_plural_range_list_ty *ranges;
};

enum cldr_plural_condition
  {
    CLDR_PLURAL_CONDITION_AND,
    CLDR_PLURAL_CONDITION_OR,
    CLDR_PLURAL_CONDITION_RELATION,
    CLDR_PLURAL_CONDITION_TRUE,
    CLDR_PLURAL_CONDITION_FALSE
  };

struct cldr_plural_condition_ty
{
  enum cldr_plural_condition type;
  union
  {
    struct cldr_plural_relation_ty *relation;
    struct cldr_plural_condition_ty *conditions[2];
  } value;
};

struct cldr_plural_rule_ty
{
  char *name;
  struct cldr_plural_condition_ty *condition;
};

struct cldr_plural_rule_list_ty
{
  struct cldr_plural_rule_ty **items;
  size_t nitems;
  size_t nitems_max;
};

struct cldr_plural_parse_args
{
  const char *cp;
  const char *cp_end;
  struct cldr_plural_rule_list_ty *result;
};

extern void
cldr_plural_range_free (struct cldr_plural_range_ty *range);
extern void
cldr_plural_range_list_free (struct cldr_plural_range_list_ty *ranges);
extern void
cldr_plural_condition_free (struct cldr_plural_condition_ty *condition);
extern void
cldr_plural_relation_free (struct cldr_plural_relation_ty *relation);

extern struct cldr_plural_rule_list_ty *
cldr_plural_parse (const char *input);
extern void
cldr_plural_rule_list_free (struct cldr_plural_rule_list_ty *rules);
extern void
cldr_plural_rule_list_print (struct cldr_plural_rule_list_ty *rules, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif  /* _CLDR_PLURAL_EXP_H */
