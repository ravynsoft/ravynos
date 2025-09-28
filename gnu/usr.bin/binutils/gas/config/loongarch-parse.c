/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 19 "./config/loongarch-parse.y"

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


#line 377 "config/loongarch-parse.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_CONFIG_LOONGARCH_PARSE_H_INCLUDED
# define YY_YY_CONFIG_LOONGARCH_PARSE_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    INTEGER = 258,                 /* INTEGER  */
    IDENTIFIER = 259,              /* IDENTIFIER  */
    LEFT_OP = 260,                 /* LEFT_OP  */
    RIGHT_OP = 261,                /* RIGHT_OP  */
    LE_OP = 262,                   /* LE_OP  */
    GE_OP = 263,                   /* GE_OP  */
    EQ_OP = 264,                   /* EQ_OP  */
    NE_OP = 265,                   /* NE_OP  */
    AND_OP = 266,                  /* AND_OP  */
    OR_OP = 267                    /* OR_OP  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define INTEGER 258
#define IDENTIFIER 259
#define LEFT_OP 260
#define RIGHT_OP 261
#define LE_OP 262
#define GE_OP 263
#define EQ_OP 264
#define NE_OP 265
#define AND_OP 266
#define OR_OP 267

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 325 "./config/loongarch-parse.y"

char *c_str;
offsetT imm;

#line 459 "config/loongarch-parse.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_CONFIG_LOONGARCH_PARSE_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_INTEGER = 3,                    /* INTEGER  */
  YYSYMBOL_IDENTIFIER = 4,                 /* IDENTIFIER  */
  YYSYMBOL_LEFT_OP = 5,                    /* LEFT_OP  */
  YYSYMBOL_RIGHT_OP = 6,                   /* RIGHT_OP  */
  YYSYMBOL_LE_OP = 7,                      /* LE_OP  */
  YYSYMBOL_GE_OP = 8,                      /* GE_OP  */
  YYSYMBOL_EQ_OP = 9,                      /* EQ_OP  */
  YYSYMBOL_NE_OP = 10,                     /* NE_OP  */
  YYSYMBOL_AND_OP = 11,                    /* AND_OP  */
  YYSYMBOL_OR_OP = 12,                     /* OR_OP  */
  YYSYMBOL_13_ = 13,                       /* '('  */
  YYSYMBOL_14_ = 14,                       /* ')'  */
  YYSYMBOL_15_ = 15,                       /* '%'  */
  YYSYMBOL_16_ = 16,                       /* '-'  */
  YYSYMBOL_17_ = 17,                       /* '+'  */
  YYSYMBOL_18_ = 18,                       /* '~'  */
  YYSYMBOL_19_ = 19,                       /* '!'  */
  YYSYMBOL_20_ = 20,                       /* '*'  */
  YYSYMBOL_21_ = 21,                       /* '/'  */
  YYSYMBOL_22_ = 22,                       /* '<'  */
  YYSYMBOL_23_ = 23,                       /* '>'  */
  YYSYMBOL_24_ = 24,                       /* '&'  */
  YYSYMBOL_25_ = 25,                       /* '^'  */
  YYSYMBOL_26_ = 26,                       /* '|'  */
  YYSYMBOL_27_ = 27,                       /* '?'  */
  YYSYMBOL_28_ = 28,                       /* ':'  */
  YYSYMBOL_YYACCEPT = 29,                  /* $accept  */
  YYSYMBOL_primary_expression = 30,        /* primary_expression  */
  YYSYMBOL_addend = 31,                    /* addend  */
  YYSYMBOL_unary_expression = 32,          /* unary_expression  */
  YYSYMBOL_multiplicative_expression = 33, /* multiplicative_expression  */
  YYSYMBOL_additive_expression = 34,       /* additive_expression  */
  YYSYMBOL_shift_expression = 35,          /* shift_expression  */
  YYSYMBOL_relational_expression = 36,     /* relational_expression  */
  YYSYMBOL_equality_expression = 37,       /* equality_expression  */
  YYSYMBOL_and_expression = 38,            /* and_expression  */
  YYSYMBOL_exclusive_or_expression = 39,   /* exclusive_or_expression  */
  YYSYMBOL_inclusive_or_expression = 40,   /* inclusive_or_expression  */
  YYSYMBOL_logical_and_expression = 41,    /* logical_and_expression  */
  YYSYMBOL_logical_or_expression = 42,     /* logical_or_expression  */
  YYSYMBOL_conditional_expression = 43,    /* conditional_expression  */
  YYSYMBOL_expression = 44                 /* expression  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  48
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   74

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  29
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  16
/* YYNRULES -- Number of rules.  */
#define YYNRULES  45
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  82

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   267


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    19,     2,     2,     2,    15,    24,     2,
      13,    14,    20,    17,     2,    16,     2,    21,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    28,     2,
      22,     2,    23,    27,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    25,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    26,     2,    18,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   339,   339,   340,   341,   342,   343,   347,   348,   349,
     353,   354,   355,   356,   357,   361,   362,   363,   364,   368,
     369,   370,   374,   375,   376,   380,   381,   382,   383,   384,
     388,   389,   390,   394,   395,   399,   400,   404,   405,   409,
     410,   414,   415,   419,   420,   424
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "INTEGER",
  "IDENTIFIER", "LEFT_OP", "RIGHT_OP", "LE_OP", "GE_OP", "EQ_OP", "NE_OP",
  "AND_OP", "OR_OP", "'('", "')'", "'%'", "'-'", "'+'", "'~'", "'!'",
  "'*'", "'/'", "'<'", "'>'", "'&'", "'^'", "'|'", "'?'", "':'", "$accept",
  "primary_expression", "addend", "unary_expression",
  "multiplicative_expression", "additive_expression", "shift_expression",
  "relational_expression", "equality_expression", "and_expression",
  "exclusive_or_expression", "inclusive_or_expression",
  "logical_and_expression", "logical_or_expression",
  "conditional_expression", "expression", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-28)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
       2,   -28,   -28,     2,    10,     2,     2,     2,     2,   -28,
     -28,     9,    23,    36,     0,    37,    -8,     7,    26,    25,
       1,   -28,    43,    31,    44,   -28,   -28,   -28,   -28,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   -28,   -28,
      45,   -28,   -28,   -28,     9,     9,    23,    23,    36,    36,
      36,    36,     0,     0,    37,    -8,     7,    26,    25,    30,
     -28,   -28,     2,    17,    21,   -28,   -28,    56,    57,   -28,
     -28,   -28
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     2,     3,     0,     0,     0,     0,     0,     0,    10,
      15,    19,    22,    25,    30,    33,    35,    37,    39,    41,
      43,    45,     0,     0,     0,    12,    11,    13,    14,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     1,     4,
       0,    18,    16,    17,    21,    20,    23,    24,    28,    29,
      26,    27,    31,    32,    34,    36,    38,    40,    42,     0,
       9,     9,     0,     0,     0,    44,     6,     0,     0,     5,
       7,     8
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -28,   -28,   -10,    -4,    18,    19,   -27,    15,    20,    22,
      24,    27,    28,   -28,    -9,    -3
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     9,    73,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      23,    25,    26,    27,    28,     1,     2,    36,    37,    58,
      59,    60,    61,    46,    24,     3,    42,     4,     5,     6,
       7,     8,    38,    39,    29,    51,    52,    53,    47,    30,
      31,    76,    43,    77,    78,    79,    45,    77,    78,    32,
      33,    34,    35,    48,    69,    49,    40,    41,    70,    71,
      54,    55,    44,    56,    57,    62,    63,    50,    72,    80,
      81,    74,    64,    75,     0,    65,     0,     0,    66,     0,
       0,     0,    67,     0,    68
};

static const yytype_int8 yycheck[] =
{
       3,     5,     6,     7,     8,     3,     4,     7,     8,    36,
      37,    38,    39,    12,     4,    13,    24,    15,    16,    17,
      18,    19,    22,    23,    15,    29,    30,    31,    27,    20,
      21,    14,    25,    16,    17,    14,    11,    16,    17,    16,
      17,     5,     6,     0,    47,    14,     9,    10,     3,     4,
      32,    33,    26,    34,    35,    40,    41,    13,    28,     3,
       3,    71,    42,    72,    -1,    43,    -1,    -1,    44,    -1,
      -1,    -1,    45,    -1,    46
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,    13,    15,    16,    17,    18,    19,    30,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    44,     4,    32,    32,    32,    32,    15,
      20,    21,    16,    17,     5,     6,     7,     8,    22,    23,
       9,    10,    24,    25,    26,    11,    12,    27,     0,    14,
      13,    32,    32,    32,    33,    33,    34,    34,    35,    35,
      35,    35,    36,    36,    37,    38,    39,    40,    41,    44,
       3,     4,    28,    31,    31,    43,    14,    16,    17,    14,
       3,     3
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    29,    30,    30,    30,    30,    30,    31,    31,    31,
      32,    32,    32,    32,    32,    33,    33,    33,    33,    34,
      34,    34,    35,    35,    35,    36,    36,    36,    36,    36,
      37,    37,    37,    38,    38,    39,    39,    40,    40,    41,
      41,    42,    42,    43,    43,    44
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     3,     6,     6,     3,     3,     0,
       1,     2,     2,     2,     2,     1,     3,     3,     3,     1,
       3,     3,     1,     3,     3,     1,     3,     3,     3,     3,
       1,     3,     3,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     5,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* primary_expression: INTEGER  */
#line 339 "./config/loongarch-parse.y"
                  {emit_const ((yyvsp[0].imm));}
#line 1529 "config/loongarch-parse.c"
    break;

  case 3: /* primary_expression: IDENTIFIER  */
#line 340 "./config/loongarch-parse.y"
                     {emit_const_var ((yyvsp[0].c_str));}
#line 1535 "config/loongarch-parse.c"
    break;

  case 5: /* primary_expression: '%' IDENTIFIER '(' IDENTIFIER addend ')'  */
#line 342 "./config/loongarch-parse.y"
                                                   {reloc ((yyvsp[-4].c_str), (yyvsp[-2].c_str), (yyvsp[-1].imm)); free ((yyvsp[-4].c_str)); free ((yyvsp[-2].c_str));}
#line 1541 "config/loongarch-parse.c"
    break;

  case 6: /* primary_expression: '%' IDENTIFIER '(' INTEGER addend ')'  */
#line 343 "./config/loongarch-parse.y"
                                                {reloc ((yyvsp[-4].c_str), NULL, (yyvsp[-2].imm) + (yyvsp[-1].imm)); free ((yyvsp[-4].c_str));}
#line 1547 "config/loongarch-parse.c"
    break;

  case 7: /* addend: addend '-' INTEGER  */
#line 347 "./config/loongarch-parse.y"
                             {(yyval.imm) -= (yyvsp[0].imm);}
#line 1553 "config/loongarch-parse.c"
    break;

  case 8: /* addend: addend '+' INTEGER  */
#line 348 "./config/loongarch-parse.y"
                             {(yyval.imm) += (yyvsp[0].imm);}
#line 1559 "config/loongarch-parse.c"
    break;

  case 9: /* addend: %empty  */
#line 349 "./config/loongarch-parse.y"
          {(yyval.imm) = 0;}
#line 1565 "config/loongarch-parse.c"
    break;

  case 11: /* unary_expression: '+' unary_expression  */
#line 354 "./config/loongarch-parse.y"
                               {emit_unary ('+');}
#line 1571 "config/loongarch-parse.c"
    break;

  case 12: /* unary_expression: '-' unary_expression  */
#line 355 "./config/loongarch-parse.y"
                               {emit_unary ('-');}
#line 1577 "config/loongarch-parse.c"
    break;

  case 13: /* unary_expression: '~' unary_expression  */
#line 356 "./config/loongarch-parse.y"
                               {emit_unary ('~');}
#line 1583 "config/loongarch-parse.c"
    break;

  case 14: /* unary_expression: '!' unary_expression  */
#line 357 "./config/loongarch-parse.y"
                               {emit_unary ('!');}
#line 1589 "config/loongarch-parse.c"
    break;

  case 16: /* multiplicative_expression: multiplicative_expression '*' unary_expression  */
#line 362 "./config/loongarch-parse.y"
                                                         {emit_bin ('*');}
#line 1595 "config/loongarch-parse.c"
    break;

  case 17: /* multiplicative_expression: multiplicative_expression '/' unary_expression  */
#line 363 "./config/loongarch-parse.y"
                                                         {emit_bin ('/');}
#line 1601 "config/loongarch-parse.c"
    break;

  case 18: /* multiplicative_expression: multiplicative_expression '%' unary_expression  */
#line 364 "./config/loongarch-parse.y"
                                                         {emit_bin ('%');}
#line 1607 "config/loongarch-parse.c"
    break;

  case 20: /* additive_expression: additive_expression '+' multiplicative_expression  */
#line 369 "./config/loongarch-parse.y"
                                                            {emit_bin ('+');}
#line 1613 "config/loongarch-parse.c"
    break;

  case 21: /* additive_expression: additive_expression '-' multiplicative_expression  */
#line 370 "./config/loongarch-parse.y"
                                                            {emit_bin ('-');}
#line 1619 "config/loongarch-parse.c"
    break;

  case 23: /* shift_expression: shift_expression LEFT_OP additive_expression  */
#line 375 "./config/loongarch-parse.y"
                                                       {emit_bin (LEFT_OP);}
#line 1625 "config/loongarch-parse.c"
    break;

  case 24: /* shift_expression: shift_expression RIGHT_OP additive_expression  */
#line 376 "./config/loongarch-parse.y"
                                                        {emit_bin (RIGHT_OP);}
#line 1631 "config/loongarch-parse.c"
    break;

  case 26: /* relational_expression: relational_expression '<' shift_expression  */
#line 381 "./config/loongarch-parse.y"
                                                     {emit_bin ('<');}
#line 1637 "config/loongarch-parse.c"
    break;

  case 27: /* relational_expression: relational_expression '>' shift_expression  */
#line 382 "./config/loongarch-parse.y"
                                                     {emit_bin ('>');}
#line 1643 "config/loongarch-parse.c"
    break;

  case 28: /* relational_expression: relational_expression LE_OP shift_expression  */
#line 383 "./config/loongarch-parse.y"
                                                       {emit_bin (LE_OP);}
#line 1649 "config/loongarch-parse.c"
    break;

  case 29: /* relational_expression: relational_expression GE_OP shift_expression  */
#line 384 "./config/loongarch-parse.y"
                                                       {emit_bin (GE_OP);}
#line 1655 "config/loongarch-parse.c"
    break;

  case 31: /* equality_expression: equality_expression EQ_OP relational_expression  */
#line 389 "./config/loongarch-parse.y"
                                                          {emit_bin (EQ_OP);}
#line 1661 "config/loongarch-parse.c"
    break;

  case 32: /* equality_expression: equality_expression NE_OP relational_expression  */
#line 390 "./config/loongarch-parse.y"
                                                          {emit_bin (NE_OP);}
#line 1667 "config/loongarch-parse.c"
    break;

  case 34: /* and_expression: and_expression '&' equality_expression  */
#line 395 "./config/loongarch-parse.y"
                                                 {emit_bin ('&');}
#line 1673 "config/loongarch-parse.c"
    break;

  case 36: /* exclusive_or_expression: exclusive_or_expression '^' and_expression  */
#line 400 "./config/loongarch-parse.y"
                                                     {emit_bin ('^');}
#line 1679 "config/loongarch-parse.c"
    break;

  case 38: /* inclusive_or_expression: inclusive_or_expression '|' exclusive_or_expression  */
#line 405 "./config/loongarch-parse.y"
                                                              {emit_bin ('|');}
#line 1685 "config/loongarch-parse.c"
    break;

  case 40: /* logical_and_expression: logical_and_expression AND_OP inclusive_or_expression  */
#line 410 "./config/loongarch-parse.y"
                                                                {emit_bin (AND_OP);}
#line 1691 "config/loongarch-parse.c"
    break;

  case 42: /* logical_or_expression: logical_or_expression OR_OP logical_and_expression  */
#line 415 "./config/loongarch-parse.y"
                                                             {emit_bin (OR_OP);}
#line 1697 "config/loongarch-parse.c"
    break;

  case 44: /* conditional_expression: logical_or_expression '?' expression ':' conditional_expression  */
#line 420 "./config/loongarch-parse.y"
                                                                          {emit_if_else ();}
#line 1703 "config/loongarch-parse.c"
    break;


#line 1707 "config/loongarch-parse.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 426 "./config/loongarch-parse.y"


