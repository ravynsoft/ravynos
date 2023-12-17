/* Unicode CLDR plural rule parser and converter
   Copyright (C) 2015, 2020 Free Software Foundation, Inc.

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

%{
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "unistr.h"
#include "xalloc.h"

#include "cldr-plural-exp.h"
#include "cldr-plural.h"

/* Prototypes for local functions.  */
static int yylex (YYSTYPE *lval, struct cldr_plural_parse_args *arg);
static void yyerror (struct cldr_plural_parse_args *arg, const char *str);

/* Allocation of expressions.  */

static struct cldr_plural_rule_ty *
new_rule (char *name, struct cldr_plural_condition_ty *condition)
{
  struct cldr_plural_rule_ty *result =
    XMALLOC (struct cldr_plural_rule_ty);
  result->name = name;
  result->condition = condition;
  return result;
}

static struct cldr_plural_condition_ty *
new_leaf_condition (struct cldr_plural_relation_ty *relation)
{
  struct cldr_plural_condition_ty *result =
    XMALLOC (struct cldr_plural_condition_ty);
  result->type = CLDR_PLURAL_CONDITION_RELATION;
  result->value.relation = relation;
  return result;
}

static struct cldr_plural_condition_ty *
new_branch_condition (enum cldr_plural_condition type,
                      struct cldr_plural_condition_ty *condition0,
                      struct cldr_plural_condition_ty *condition1)
{
  struct cldr_plural_condition_ty *result =
    XMALLOC (struct cldr_plural_condition_ty);
  result->type = type;
  result->value.conditions[0] = condition0;
  result->value.conditions[1] = condition1;
  return result;
}

static struct cldr_plural_relation_ty *
new_relation (struct cldr_plural_expression_ty *expression,
              enum cldr_plural_relation type,
              struct cldr_plural_range_list_ty *ranges)
{
  struct cldr_plural_relation_ty *result =
    XMALLOC (struct cldr_plural_relation_ty);
  result->expression = expression;
  result->type = type;
  result->ranges = ranges;
  return result;
}

static struct cldr_plural_expression_ty *
new_expression (int operand, int mod)
{
  struct cldr_plural_expression_ty *result =
    XMALLOC (struct cldr_plural_expression_ty);
  result->operand = operand;
  result->mod = mod;
  return result;
}

static struct cldr_plural_range_list_ty *
add_range (struct cldr_plural_range_list_ty *ranges,
           struct cldr_plural_range_ty *range)
{
  if (ranges->nitems == ranges->nitems_max)
    {
      ranges->nitems_max = ranges->nitems_max * 2 + 1;
      ranges->items = xrealloc (ranges->items,
                                sizeof (struct cldr_plural_range_ty *)
                                * ranges->nitems_max);
    }
  ranges->items[ranges->nitems++] = range;
  return ranges;
}

static struct cldr_plural_range_ty *
new_range (struct cldr_plural_operand_ty *start,
           struct cldr_plural_operand_ty *end)
{
  struct cldr_plural_range_ty *result =
    XMALLOC (struct cldr_plural_range_ty);
  result->start = start;
  result->end = end;
  return result;
}
%}

%require "3.0"

%parse-param {struct cldr_plural_parse_args *arg}
%lex-param {struct cldr_plural_parse_args *arg}
%define api.pure full

%union {
  char *sval;
  struct cldr_plural_condition_ty *cval;
  struct cldr_plural_relation_ty *lval;
  struct cldr_plural_expression_ty *eval;
  struct cldr_plural_range_ty *gval;
  struct cldr_plural_operand_ty *oval;
  struct cldr_plural_range_list_ty *rval;
  int ival;
}

%destructor { free ($$); } <sval>
%destructor { cldr_plural_condition_free ($$); } <cval>
%destructor { cldr_plural_relation_free ($$); } <lval>
%destructor { free ($$); } <eval>
%destructor { cldr_plural_range_free ($$); } <gval>
%destructor { free ($$); } <oval>
%destructor { cldr_plural_range_list_free ($$); } <rval>
%destructor { } <ival>

%token AND OR RANGE ELLIPSIS OTHER AT_INTEGER AT_DECIMAL
%token<sval> KEYWORD
%token<oval> INTEGER DECIMAL
%token<ival> OPERAND
%type<cval> condition and_condition
%type<lval> relation
%type<eval> expression
%type<gval> range range_or_integer
%type<rval> range_list

%%

rules: rule
        | rules ';' rule
        ;

rule:   KEYWORD ':' condition samples
        {
          struct cldr_plural_rule_ty *rule = new_rule ($1, $3);
          struct cldr_plural_rule_list_ty *result = arg->result;
          if (result->nitems == result->nitems_max)
            {
              result->nitems_max = result->nitems_max * 2 + 1;
              result->items = xrealloc (result->items,
                                        sizeof (struct cldr_plural_rule_ty *)
                                        * result->nitems_max);
            }
          result->items[result->nitems++] = rule;
        }
        | OTHER ':' samples
        ;

condition: and_condition
        {
          $$ = $1;
        }
        | condition OR and_condition
        {
          $$ = new_branch_condition (CLDR_PLURAL_CONDITION_OR, $1, $3);
        }
        ;

and_condition: relation
        {
          $$ = new_leaf_condition ($1);
        }
        | and_condition AND relation
        {
          $$ = new_branch_condition (CLDR_PLURAL_CONDITION_AND,
                                     $1,
                                     new_leaf_condition ($3));
        }
        ;

relation: expression '=' range_list
        {
          $$ = new_relation ($1, CLDR_PLURAL_RELATION_EQUAL, $3);
        }
        | expression '!' range_list
        {
          $$ = new_relation ($1, CLDR_PLURAL_RELATION_NOT_EQUAL, $3);
        }
        ;

expression: OPERAND
        {
          $$ = new_expression ($1, 0);
        }
        | OPERAND '%' INTEGER
        {
          $$ = new_expression ($1, $3->value.ival);
        }
        ;

range_list: range_or_integer
        {
          struct cldr_plural_range_list_ty *ranges =
            XMALLOC (struct cldr_plural_range_list_ty);
          memset (ranges, 0, sizeof (struct cldr_plural_range_list_ty));
          $$ = add_range (ranges, $1);
        }
        | range_list ',' range_or_integer
        {
          $$ = add_range ($1, $3);
        }
        ;

range_or_integer: range
        {
          $$ = $1;
        }
        | INTEGER
        {
          $$ = new_range ($1, $1);
        }
        ;

range: INTEGER RANGE INTEGER
        {
          $$ = new_range ($1, $3);
        }
        ;

/* FIXME: collect samples */
samples: at_integer at_decimal
        ;

at_integer: %empty
        | AT_INTEGER sample_list
        ;

at_decimal: %empty
        | AT_DECIMAL sample_list
        ;

sample_list: sample_list1 sample_ellipsis
        ;
sample_list1: sample_range
        | sample_list1 ',' sample_range
        ;
sample_ellipsis: %empty
        | ',' ELLIPSIS
        ;

sample_range: DECIMAL
	{ free ($1); }
        | DECIMAL '~' DECIMAL
        { free ($1); free ($3); }
        | INTEGER
        { free ($1); }
        | INTEGER '~' INTEGER
	{ free ($1); free ($3); }
        ;

%%

static int
yylex (YYSTYPE *lval, struct cldr_plural_parse_args *arg)
{
  const char *exp = arg->cp;
  ucs4_t uc;
  int length;
  int result;
  static char *buffer;
  static size_t bufmax;
  size_t bufpos;

  while (1)
    {
      if (exp[0] == '\0')
        {
          arg->cp = exp;
          return YYEOF;
        }

      if (exp[0] != ' ' && exp[0] != '\t')
        break;

      ++exp;
    }

  length = u8_mbtouc (&uc, (const uint8_t *) exp, arg->cp_end - exp);
  if (uc == 0x2026)
    {
      arg->cp = exp + length;
      return ELLIPSIS;
    }
  else if (strncmp ("...", exp, 3) == 0)
    {
      arg->cp = exp + 3;
      return ELLIPSIS;
    }
  else if (strncmp ("..", exp, 2) == 0)
    {
      arg->cp = exp + 2;
      return RANGE;
    }
  else if (strncmp ("other", exp, 5) == 0)
    {
      arg->cp = exp + 5;
      return OTHER;
    }
  else if (strncmp ("@integer", exp, 8) == 0)
    {
      arg->cp = exp + 8;
      return AT_INTEGER;
    }
  else if (strncmp ("@decimal", exp, 8) == 0)
    {
      arg->cp = exp + 8;
      return AT_DECIMAL;
    }

  result = *exp++;
  switch (result)
    {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      {
        unsigned long int ival = result - '0';

        while (exp[0] >= '0' && exp[0] <= '9')
          {
            ival *= 10;
            ival += exp[0] - '0';
            ++exp;
          }

        lval->oval = XMALLOC (struct cldr_plural_operand_ty);
        if (exp[0] == '.' && exp[1] >= '0' && exp[1] <= '9')
          {
            double dval = ival;
            int denominator = 10, nfractions = 0;
            ++exp;
            while (exp[0] >= '0' && exp[0] <= '9')
              {
                dval += (exp[0] - '0') / (double) denominator;
                denominator *= 10;
                ++nfractions;
                ++exp;
              }
            lval->oval->type = CLDR_PLURAL_OPERAND_DECIMAL;
            lval->oval->value.dval.d = dval;
            lval->oval->value.dval.nfractions = nfractions;
            result = DECIMAL;
          }
        else
          {
            lval->oval->type = CLDR_PLURAL_OPERAND_INTEGER;
            lval->oval->value.ival = ival;
            result = INTEGER;
          }
      }
      break;
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z':
      bufpos = 0;
      for (;;)
        {
          if (bufpos >= bufmax)
            {
              bufmax = 2 * bufmax + 10;
              buffer = xrealloc (buffer, bufmax);
            }
          buffer[bufpos++] = result;
          result = *exp;
          switch (result)
            {
            case 'a': case 'b': case 'c': case 'd': case 'e':
            case 'f': case 'g': case 'h': case 'i': case 'j':
            case 'k': case 'l': case 'm': case 'n': case 'o':
            case 'p': case 'q': case 'r': case 's': case 't':
            case 'u': case 'v': case 'w': case 'x': case 'y':
            case 'z':
              ++exp;
              continue;
            default:
              break;
            }
          break;
        }

      if (bufpos >= bufmax)
        {
          bufmax = 2 * bufmax + 10;
          buffer = xrealloc (buffer, bufmax);
        }
      buffer[bufpos] = '\0';

      /* Operands.  */
      if (bufpos == 1)
        {
          switch (buffer[0])
            {
            case 'n': case 'i': case 'f': case 't': case 'v': case 'w':
              arg->cp = exp;
              lval->ival = buffer[0];
              return OPERAND;
            default:
              break;
            }
        }

      /* Keywords.  */
      if (strcmp (buffer, "and") == 0)
        {
          arg->cp = exp;
          return AND;
        }
      else if (strcmp (buffer, "or") == 0)
        {
          arg->cp = exp;
          return OR;
        }

      lval->sval = xstrdup (buffer);
      result = KEYWORD;
      break;
    case '!':
      if (exp[0] == '=')
        {
          ++exp;
          result = '!';
        }
      else
        result = YYERRCODE;
      break;
    default:
      break;
    }

  arg->cp = exp;

  return result;
}

static void
yyerror (struct cldr_plural_parse_args *arg, char const *s)
{
  fprintf (stderr, "%s\n", s);
}
