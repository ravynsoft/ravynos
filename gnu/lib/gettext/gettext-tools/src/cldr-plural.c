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
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 19 "cldr-plural.y"

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

#line 175 "cldr-plural.c"

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

#include "cldr-plural.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_AND = 3,                        /* AND  */
  YYSYMBOL_OR = 4,                         /* OR  */
  YYSYMBOL_RANGE = 5,                      /* RANGE  */
  YYSYMBOL_ELLIPSIS = 6,                   /* ELLIPSIS  */
  YYSYMBOL_OTHER = 7,                      /* OTHER  */
  YYSYMBOL_AT_INTEGER = 8,                 /* AT_INTEGER  */
  YYSYMBOL_AT_DECIMAL = 9,                 /* AT_DECIMAL  */
  YYSYMBOL_KEYWORD = 10,                   /* KEYWORD  */
  YYSYMBOL_INTEGER = 11,                   /* INTEGER  */
  YYSYMBOL_DECIMAL = 12,                   /* DECIMAL  */
  YYSYMBOL_OPERAND = 13,                   /* OPERAND  */
  YYSYMBOL_14_ = 14,                       /* ';'  */
  YYSYMBOL_15_ = 15,                       /* ':'  */
  YYSYMBOL_16_ = 16,                       /* '='  */
  YYSYMBOL_17_ = 17,                       /* '!'  */
  YYSYMBOL_18_ = 18,                       /* '%'  */
  YYSYMBOL_19_ = 19,                       /* ','  */
  YYSYMBOL_20_ = 20,                       /* '~'  */
  YYSYMBOL_YYACCEPT = 21,                  /* $accept  */
  YYSYMBOL_rules = 22,                     /* rules  */
  YYSYMBOL_rule = 23,                      /* rule  */
  YYSYMBOL_condition = 24,                 /* condition  */
  YYSYMBOL_and_condition = 25,             /* and_condition  */
  YYSYMBOL_relation = 26,                  /* relation  */
  YYSYMBOL_expression = 27,                /* expression  */
  YYSYMBOL_range_list = 28,                /* range_list  */
  YYSYMBOL_range_or_integer = 29,          /* range_or_integer  */
  YYSYMBOL_range = 30,                     /* range  */
  YYSYMBOL_samples = 31,                   /* samples  */
  YYSYMBOL_at_integer = 32,                /* at_integer  */
  YYSYMBOL_at_decimal = 33,                /* at_decimal  */
  YYSYMBOL_sample_list = 34,               /* sample_list  */
  YYSYMBOL_sample_list1 = 35,              /* sample_list1  */
  YYSYMBOL_sample_ellipsis = 36,           /* sample_ellipsis  */
  YYSYMBOL_sample_range = 37               /* sample_range  */
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
#define YYFINAL  7
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   55

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  21
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  17
/* YYNRULES -- Number of rules.  */
#define YYNRULES  32
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  52

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   268


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
       2,     2,     2,    17,     2,     2,     2,    18,     2,     2,
       2,     2,     2,     2,    19,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    15,    14,
       2,    16,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    20,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10,    11,    12,    13
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   161,   161,   162,   165,   178,   181,   185,   191,   195,
     203,   207,   213,   217,   223,   230,   236,   240,   246,   253,
     256,   257,   260,   261,   264,   266,   267,   269,   270,   273,
     275,   277,   279
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
  "\"end of file\"", "error", "\"invalid token\"", "AND", "OR", "RANGE",
  "ELLIPSIS", "OTHER", "AT_INTEGER", "AT_DECIMAL", "KEYWORD", "INTEGER",
  "DECIMAL", "OPERAND", "';'", "':'", "'='", "'!'", "'%'", "','", "'~'",
  "$accept", "rules", "rule", "condition", "and_condition", "relation",
  "expression", "range_list", "range_or_integer", "range", "samples",
  "at_integer", "at_decimal", "sample_list", "sample_list1",
  "sample_ellipsis", "sample_range", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-20)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
       3,   -11,    -7,     0,   -20,     4,    -2,   -20,     3,    -9,
     -20,     8,     2,     1,    15,   -20,    -1,   -20,     5,     6,
     -20,     9,   -20,    -9,   -20,    10,    -2,   -20,    -2,    11,
      11,    12,     7,    -5,   -20,   -20,   -20,    15,   -20,    19,
      13,   -20,   -20,    13,   -20,   -20,   -20,   -20,    16,    11,
     -20,   -20
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     2,    20,     0,     1,     0,     0,
       5,    22,    12,    20,     6,     8,     0,     3,    31,    29,
      21,    27,    25,     0,    19,     0,     0,     4,     0,     0,
       0,     0,     0,     0,    24,    23,    13,     7,     9,    17,
      10,    14,    16,    11,    32,    30,    28,    26,     0,     0,
      18,    15
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -20,   -20,    21,   -20,    14,    17,   -20,    18,   -19,   -20,
      20,   -20,   -20,    23,   -20,   -20,    22
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     3,     4,    13,    14,    15,    16,    40,    41,    42,
      10,    11,    24,    20,    21,    34,    22
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
       7,    46,    18,    19,     5,    26,    18,    19,     6,     9,
       1,    12,     9,     2,     8,    29,    30,    23,    28,    45,
      25,    36,    39,    44,    48,    31,    32,    50,    33,    17,
      51,     0,    49,    27,     0,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,    38,    35,     0,    43,     0,
       0,     0,     0,     0,     0,    47
};

static const yytype_int8 yycheck[] =
{
       0,     6,    11,    12,    15,     4,    11,    12,    15,     8,
       7,    13,     8,    10,    14,    16,    17,     9,     3,    12,
      18,    11,    11,    11,     5,    20,    20,    11,    19,     8,
      49,    -1,    19,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    -1,    -1,    -1,    28,    23,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    33
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     7,    10,    22,    23,    15,    15,     0,    14,     8,
      31,    32,    13,    24,    25,    26,    27,    23,    11,    12,
      34,    35,    37,     9,    33,    18,     4,    31,     3,    16,
      17,    20,    20,    19,    36,    34,    11,    25,    26,    11,
      28,    29,    30,    28,    11,    12,     6,    37,     5,    19,
      11,    29
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    21,    22,    22,    23,    23,    24,    24,    25,    25,
      26,    26,    27,    27,    28,    28,    29,    29,    30,    31,
      32,    32,    33,    33,    34,    35,    35,    36,    36,    37,
      37,    37,    37
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     3,     4,     3,     1,     3,     1,     3,
       3,     3,     1,     3,     1,     3,     1,     1,     3,     2,
       0,     2,     0,     2,     2,     1,     3,     0,     2,     1,
       3,     1,     3
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
        yyerror (arg, YY_("syntax error: cannot back up")); \
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
                  Kind, Value, arg); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, struct cldr_plural_parse_args *arg)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (arg);
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
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, struct cldr_plural_parse_args *arg)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, arg);
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
                 int yyrule, struct cldr_plural_parse_args *arg)
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
                       &yyvsp[(yyi + 1) - (yynrhs)], arg);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, arg); \
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
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, struct cldr_plural_parse_args *arg)
{
  YY_USE (yyvaluep);
  YY_USE (arg);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_KEYWORD: /* KEYWORD  */
#line 140 "cldr-plural.y"
            { free (((*yyvaluep).sval)); }
#line 969 "cldr-plural.c"
        break;

    case YYSYMBOL_INTEGER: /* INTEGER  */
#line 145 "cldr-plural.y"
            { free (((*yyvaluep).oval)); }
#line 975 "cldr-plural.c"
        break;

    case YYSYMBOL_DECIMAL: /* DECIMAL  */
#line 145 "cldr-plural.y"
            { free (((*yyvaluep).oval)); }
#line 981 "cldr-plural.c"
        break;

    case YYSYMBOL_OPERAND: /* OPERAND  */
#line 147 "cldr-plural.y"
            { }
#line 987 "cldr-plural.c"
        break;

    case YYSYMBOL_condition: /* condition  */
#line 141 "cldr-plural.y"
            { cldr_plural_condition_free (((*yyvaluep).cval)); }
#line 993 "cldr-plural.c"
        break;

    case YYSYMBOL_and_condition: /* and_condition  */
#line 141 "cldr-plural.y"
            { cldr_plural_condition_free (((*yyvaluep).cval)); }
#line 999 "cldr-plural.c"
        break;

    case YYSYMBOL_relation: /* relation  */
#line 142 "cldr-plural.y"
            { cldr_plural_relation_free (((*yyvaluep).lval)); }
#line 1005 "cldr-plural.c"
        break;

    case YYSYMBOL_expression: /* expression  */
#line 143 "cldr-plural.y"
            { free (((*yyvaluep).eval)); }
#line 1011 "cldr-plural.c"
        break;

    case YYSYMBOL_range_list: /* range_list  */
#line 146 "cldr-plural.y"
            { cldr_plural_range_list_free (((*yyvaluep).rval)); }
#line 1017 "cldr-plural.c"
        break;

    case YYSYMBOL_range_or_integer: /* range_or_integer  */
#line 144 "cldr-plural.y"
            { cldr_plural_range_free (((*yyvaluep).gval)); }
#line 1023 "cldr-plural.c"
        break;

    case YYSYMBOL_range: /* range  */
#line 144 "cldr-plural.y"
            { cldr_plural_range_free (((*yyvaluep).gval)); }
#line 1029 "cldr-plural.c"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (struct cldr_plural_parse_args *arg)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

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
      yychar = yylex (&yylval, arg);
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
  case 4: /* rule: KEYWORD ':' condition samples  */
#line 166 "cldr-plural.y"
        {
          struct cldr_plural_rule_ty *rule = new_rule ((yyvsp[-3].sval), (yyvsp[-1].cval));
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
#line 1316 "cldr-plural.c"
    break;

  case 6: /* condition: and_condition  */
#line 182 "cldr-plural.y"
        {
          (yyval.cval) = (yyvsp[0].cval);
        }
#line 1324 "cldr-plural.c"
    break;

  case 7: /* condition: condition OR and_condition  */
#line 186 "cldr-plural.y"
        {
          (yyval.cval) = new_branch_condition (CLDR_PLURAL_CONDITION_OR, (yyvsp[-2].cval), (yyvsp[0].cval));
        }
#line 1332 "cldr-plural.c"
    break;

  case 8: /* and_condition: relation  */
#line 192 "cldr-plural.y"
        {
          (yyval.cval) = new_leaf_condition ((yyvsp[0].lval));
        }
#line 1340 "cldr-plural.c"
    break;

  case 9: /* and_condition: and_condition AND relation  */
#line 196 "cldr-plural.y"
        {
          (yyval.cval) = new_branch_condition (CLDR_PLURAL_CONDITION_AND,
                                     (yyvsp[-2].cval),
                                     new_leaf_condition ((yyvsp[0].lval)));
        }
#line 1350 "cldr-plural.c"
    break;

  case 10: /* relation: expression '=' range_list  */
#line 204 "cldr-plural.y"
        {
          (yyval.lval) = new_relation ((yyvsp[-2].eval), CLDR_PLURAL_RELATION_EQUAL, (yyvsp[0].rval));
        }
#line 1358 "cldr-plural.c"
    break;

  case 11: /* relation: expression '!' range_list  */
#line 208 "cldr-plural.y"
        {
          (yyval.lval) = new_relation ((yyvsp[-2].eval), CLDR_PLURAL_RELATION_NOT_EQUAL, (yyvsp[0].rval));
        }
#line 1366 "cldr-plural.c"
    break;

  case 12: /* expression: OPERAND  */
#line 214 "cldr-plural.y"
        {
          (yyval.eval) = new_expression ((yyvsp[0].ival), 0);
        }
#line 1374 "cldr-plural.c"
    break;

  case 13: /* expression: OPERAND '%' INTEGER  */
#line 218 "cldr-plural.y"
        {
          (yyval.eval) = new_expression ((yyvsp[-2].ival), (yyvsp[0].oval)->value.ival);
        }
#line 1382 "cldr-plural.c"
    break;

  case 14: /* range_list: range_or_integer  */
#line 224 "cldr-plural.y"
        {
          struct cldr_plural_range_list_ty *ranges =
            XMALLOC (struct cldr_plural_range_list_ty);
          memset (ranges, 0, sizeof (struct cldr_plural_range_list_ty));
          (yyval.rval) = add_range (ranges, (yyvsp[0].gval));
        }
#line 1393 "cldr-plural.c"
    break;

  case 15: /* range_list: range_list ',' range_or_integer  */
#line 231 "cldr-plural.y"
        {
          (yyval.rval) = add_range ((yyvsp[-2].rval), (yyvsp[0].gval));
        }
#line 1401 "cldr-plural.c"
    break;

  case 16: /* range_or_integer: range  */
#line 237 "cldr-plural.y"
        {
          (yyval.gval) = (yyvsp[0].gval);
        }
#line 1409 "cldr-plural.c"
    break;

  case 17: /* range_or_integer: INTEGER  */
#line 241 "cldr-plural.y"
        {
          (yyval.gval) = new_range ((yyvsp[0].oval), (yyvsp[0].oval));
        }
#line 1417 "cldr-plural.c"
    break;

  case 18: /* range: INTEGER RANGE INTEGER  */
#line 247 "cldr-plural.y"
        {
          (yyval.gval) = new_range ((yyvsp[-2].oval), (yyvsp[0].oval));
        }
#line 1425 "cldr-plural.c"
    break;

  case 29: /* sample_range: DECIMAL  */
#line 274 "cldr-plural.y"
        { free ((yyvsp[0].oval)); }
#line 1431 "cldr-plural.c"
    break;

  case 30: /* sample_range: DECIMAL '~' DECIMAL  */
#line 276 "cldr-plural.y"
        { free ((yyvsp[-2].oval)); free ((yyvsp[0].oval)); }
#line 1437 "cldr-plural.c"
    break;

  case 31: /* sample_range: INTEGER  */
#line 278 "cldr-plural.y"
        { free ((yyvsp[0].oval)); }
#line 1443 "cldr-plural.c"
    break;

  case 32: /* sample_range: INTEGER '~' INTEGER  */
#line 280 "cldr-plural.y"
        { free ((yyvsp[-2].oval)); free ((yyvsp[0].oval)); }
#line 1449 "cldr-plural.c"
    break;


#line 1453 "cldr-plural.c"

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
      yyerror (arg, YY_("syntax error"));
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
                      yytoken, &yylval, arg);
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, arg);
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
  yyerror (arg, YY_("memory exhausted"));
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
                  yytoken, &yylval, arg);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, arg);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 283 "cldr-plural.y"


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
