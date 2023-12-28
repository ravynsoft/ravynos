/*
   Copyright (C) 2021-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3.  If not,
   see <http://www.gnu.org/licenses/>.  */
%{
#include "as.h"
#include "loongarch-lex.h"
#include "loongarch-parse.h"
static void yyerror (const char *s ATTRIBUTE_UNUSED)
{
};
int yylex (void);


static struct reloc_info *top, *end;

static expressionS const_0 =
{
  .X_op = O_constant,
  .X_add_number = 0
};

static int
is_const (struct reloc_info *info)
{
  return (info->type == BFD_RELOC_LARCH_SOP_PUSH_ABSOLUTE
	  && info->value.X_op == O_constant);
}

int
loongarch_parse_expr (const char *expr,
		      struct reloc_info *reloc_stack_top,
		      size_t max_reloc_num,
		      size_t *reloc_num,
		      offsetT *imm)
{
  int ret;
  struct yy_buffer_state *buffstate;
  top = reloc_stack_top;
  end = top + max_reloc_num;
  buffstate = yy_scan_string (expr);
  ret = yyparse ();

  if (ret == 0)
    {
      if (is_const (top - 1))
	*imm = (--top)->value.X_add_number;
      else
	*imm = 0;
      *reloc_num = top - reloc_stack_top;
    }
  yy_delete_buffer (buffstate);

  return ret;
}

static void
emit_const (offsetT imm)
{
  if (end <= top)
    as_fatal (_("expr too huge"));
  top->type = BFD_RELOC_LARCH_SOP_PUSH_ABSOLUTE;
  top->value.X_op = O_constant;
  top->value.X_add_number = imm;
  top++;
}

static const char *
my_getExpression (expressionS *ep, const char *str)
{
  char *save_in, *ret;

  if (*str == ':')
    {
      unsigned long j;
      char *str_1 = (char *) str;
      j = strtol (str_1, &str_1, 10);
      get_internal_label (ep, j, *str_1 == 'f');
      return NULL;
    }
  save_in = input_line_pointer;
  input_line_pointer = (char *)str;
  expression (ep);
  ret = input_line_pointer;
  input_line_pointer = save_in;
  return ret;
}

static void
emit_const_var (const char *op)
{
  expressionS ep;

  if (end <= top)
    as_fatal (_("expr too huge"));

  my_getExpression (&ep, op);

  if (ep.X_op != O_constant)
    as_bad ("illegal operand: %s", op);

  top->value.X_op = O_constant;
  top->value.X_add_number = ep.X_add_number;
  top->type = BFD_RELOC_LARCH_SOP_PUSH_ABSOLUTE;
  top++;
}

static void
reloc (const char *op_c_str, const char *id_c_str, offsetT addend)
{
  expressionS id_sym_expr;
  bfd_reloc_code_real_type btype;

  if (end <= top)
    as_fatal (_("expr too huge"));

  /* For compatible old asm code.  */
  if (0 == strcmp (op_c_str, "plt"))
    btype = BFD_RELOC_LARCH_B26;
  else
    btype = loongarch_larch_reloc_name_lookup (NULL, op_c_str);

  if (id_c_str)
  {
    my_getExpression (&id_sym_expr, id_c_str);
    id_sym_expr.X_add_number += addend;
  }
  else
  {
    id_sym_expr.X_op = O_constant;
    id_sym_expr.X_add_number = addend;
  }

  top->value = id_sym_expr;
  top->type = btype;
  top++;
}

static void
emit_unary (char op)
{
  struct reloc_info *s_top = top - 1;
  if (is_const (s_top))
    {
      offsetT opr = s_top->value.X_add_number;
      switch (op)
	{
	case '+':
	  break;
	case '-':
	  opr = -opr;
	  break;
	case '~':
	  opr = ~opr;
	  break;
	case '!':
	  opr = !opr;
	  break;
	default:
	  abort ();
	}
      s_top->value.X_add_number = opr;
    }
  else
    {
      if (end <= top)
	as_fatal (_("expr too huge"));
      switch (op)
	{
	case '!':
	  top->type = BFD_RELOC_LARCH_SOP_NOT;
	  break;
	default:
	  abort ();
	}
      top->value = const_0;
      top++;
    }
}

static void
emit_bin (int op)
{
  struct reloc_info *last_1st = top - 1, *last_2nd = top - 2;
  if (is_const (last_1st) && is_const (last_2nd))
    {
      offsetT opr1 = last_2nd->value.X_add_number;
      offsetT opr2 = last_1st->value.X_add_number;
      switch (op)
	{
	case '*':
	  opr1 = opr1 * opr2;
	  break;
	case '/':
	  opr1 = opr1 / opr2;
	  break;
	case '%':
	  opr1 = opr1 % opr2;
	  break;
	case '+':
	  opr1 = opr1 + opr2;
	  break;
	case '-':
	  opr1 = opr1 - opr2;
	  break;
	case LEFT_OP:
	  opr1 = opr1 << opr2;
	  break;
	case RIGHT_OP:
	  /* Algorithm right shift.  */
	  opr1 = (offsetT)opr1 >> (offsetT)opr2;
	  break;
	case '<':
	  opr1 = opr1 < opr2;
	  break;
	case '>':
	  opr1 = opr1 > opr2;
	  break;
	case LE_OP:
	  opr1 = opr1 <= opr2;
	  break;
	case GE_OP:
	  opr1 = opr1 >= opr2;
	  break;
	case EQ_OP:
	  opr1 = opr1 == opr2;
	  break;
	case NE_OP:
	  opr1 = opr1 != opr2;
	  break;
	case '&':
	  opr1 = opr1 & opr2;
	  break;
	case '^':
	  opr1 = opr1 ^ opr2;
	  break;
	case '|':
	  opr1 = opr1 | opr2;
	  break;
	case AND_OP:
	  opr1 = opr1 && opr2;
	  break;
	case OR_OP:
	  opr1 = opr1 || opr2;
	  break;
	default:
	  abort ();
	}
      last_2nd->value.X_add_number = opr1;
      last_1st->type = 0;
      top--;
    }
  else
    {
      if (end <= top)
	as_fatal (_("expr too huge"));
      switch (op)
	{
	case '+':
	  top->type = BFD_RELOC_LARCH_SOP_ADD;
	  break;
	case '-':
	  top->type = BFD_RELOC_LARCH_SOP_SUB;
	  break;
	case LEFT_OP:
	  top->type = BFD_RELOC_LARCH_SOP_SL;
	  break;
	case RIGHT_OP:
	  top->type = BFD_RELOC_LARCH_SOP_SR;
	  break;
	case '&':
	  top->type = BFD_RELOC_LARCH_SOP_AND;
	  break;
	default:
	  abort ();
	}
      top->value = const_0;
      top++;
    }
}

static void
emit_if_else (void)
{
  struct reloc_info *last_1st = top - 1;
  struct reloc_info *last_2nd = top - 2;
  struct reloc_info *last_3rd = top - 3;
  if (is_const (last_1st) && is_const (last_2nd) && is_const (last_3rd))
    {
      offsetT opr1 = last_3rd->value.X_add_number;
      offsetT opr2 = last_2nd->value.X_add_number;
      offsetT opr3 = last_1st->value.X_add_number;
      opr1 = opr1 ? opr2 : opr3;
      last_3rd->value.X_add_number = opr1;
      last_2nd->type = 0;
      last_1st->type = 0;
      top -= 2;
    }
  else
    {
      if (end <= top)
	as_fatal (_("expr too huge"));
      top->type = BFD_RELOC_LARCH_SOP_IF_ELSE;
      top->value = const_0;
      top++;
    }
}

%}

%union {
char *c_str;
offsetT imm;
}

%token <imm> INTEGER
%token <c_str> IDENTIFIER
%type <imm> addend

%token LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP AND_OP OR_OP
%start expression
%%

primary_expression
	: INTEGER {emit_const ($1);}
	| IDENTIFIER {emit_const_var ($1);}
	| '(' expression ')'
	| '%' IDENTIFIER '(' IDENTIFIER addend ')' {reloc ($2, $4, $5); free ($2); free ($4);}
	| '%' IDENTIFIER '(' INTEGER addend ')' {reloc ($2, NULL, $4 + $5); free ($2);}
	;

addend
	: addend '-' INTEGER {$$ -= $3;}
	| addend '+' INTEGER {$$ += $3;}
	| {$$ = 0;}
	;

unary_expression
	: primary_expression
	| '+' unary_expression {emit_unary ('+');}
	| '-' unary_expression {emit_unary ('-');}
	| '~' unary_expression {emit_unary ('~');}
	| '!' unary_expression {emit_unary ('!');}
	;

multiplicative_expression
	: unary_expression
	| multiplicative_expression '*' unary_expression {emit_bin ('*');}
	| multiplicative_expression '/' unary_expression {emit_bin ('/');}
	| multiplicative_expression '%' unary_expression {emit_bin ('%');}
	;

additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression {emit_bin ('+');}
	| additive_expression '-' multiplicative_expression {emit_bin ('-');}
	;

shift_expression
	: additive_expression
	| shift_expression LEFT_OP additive_expression {emit_bin (LEFT_OP);}
	| shift_expression RIGHT_OP additive_expression {emit_bin (RIGHT_OP);}
	;

relational_expression
	: shift_expression
	| relational_expression '<' shift_expression {emit_bin ('<');}
	| relational_expression '>' shift_expression {emit_bin ('>');}
	| relational_expression LE_OP shift_expression {emit_bin (LE_OP);}
	| relational_expression GE_OP shift_expression {emit_bin (GE_OP);}
	;

equality_expression
	: relational_expression
	| equality_expression EQ_OP relational_expression {emit_bin (EQ_OP);}
	| equality_expression NE_OP relational_expression {emit_bin (NE_OP);}
	;

and_expression
	: equality_expression
	| and_expression '&' equality_expression {emit_bin ('&');}
	;

exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression {emit_bin ('^');}
	;

inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression {emit_bin ('|');}
	;

logical_and_expression
	: inclusive_or_expression
	| logical_and_expression AND_OP inclusive_or_expression {emit_bin (AND_OP);}
	;

logical_or_expression
	: logical_and_expression
	| logical_or_expression OR_OP logical_and_expression {emit_bin (OR_OP);}
	;

conditional_expression
	: logical_or_expression
	| logical_or_expression '?' expression ':' conditional_expression {emit_if_else ();}
	;

expression
	: conditional_expression
	;
%%

