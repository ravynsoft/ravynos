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
#line 27 "./config/m68k-parse.y"


#include "as.h"
#include "tc-m68k.h"
#include "m68k-parse.h"
#include "safe-ctype.h"

/* Remap normal yacc parser interface names (yyparse, yylex, yyerror,
   etc), as well as gratuitously global symbol names If other parser
   generators (bison, byacc, etc) produce additional global names that
   conflict at link time, then those parser generators need to be
   fixed instead of adding those names to this list.  */

#define	yymaxdepth m68k_maxdepth
#define	yyparse	m68k_parse
#define	yylex	m68k_lex
#define	yyerror	m68k_error
#define	yylval	m68k_lval
#define	yychar	m68k_char
#define	yydebug	m68k_debug
#define	yypact	m68k_pact
#define	yyr1	m68k_r1
#define	yyr2	m68k_r2
#define	yydef	m68k_def
#define	yychk	m68k_chk
#define	yypgo	m68k_pgo
#define	yyact	m68k_act
#define	yyexca	m68k_exca
#define yyerrflag m68k_errflag
#define yynerrs	m68k_nerrs
#define	yyps	m68k_ps
#define	yypv	m68k_pv
#define	yys	m68k_s
#define	yy_yys	m68k_yys
#define	yystate	m68k_state
#define	yytmp	m68k_tmp
#define	yyv	m68k_v
#define	yy_yyv	m68k_yyv
#define	yyval	m68k_val
#define	yylloc	m68k_lloc
#define yyreds	m68k_reds		/* With YYDEBUG defined */
#define yytoks	m68k_toks		/* With YYDEBUG defined */
#define yylhs	m68k_yylhs
#define yylen	m68k_yylen
#define yydefred m68k_yydefred
#define yydgoto	m68k_yydgoto
#define yysindex m68k_yysindex
#define yyrindex m68k_yyrindex
#define yygindex m68k_yygindex
#define yytable	 m68k_yytable
#define yycheck	 m68k_yycheck

#ifndef YYDEBUG
#define YYDEBUG 1
#endif

/* Internal functions.  */

static enum m68k_register m68k_reg_parse (char **);
static int yylex (void);
static void yyerror (const char *);

/* The parser sets fields pointed to by this global variable.  */
static struct m68k_op *op;


#line 138 "config/m68k-parse.c"

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
    DR = 258,                      /* DR  */
    AR = 259,                      /* AR  */
    FPR = 260,                     /* FPR  */
    FPCR = 261,                    /* FPCR  */
    LPC = 262,                     /* LPC  */
    ZAR = 263,                     /* ZAR  */
    ZDR = 264,                     /* ZDR  */
    LZPC = 265,                    /* LZPC  */
    CREG = 266,                    /* CREG  */
    INDEXREG = 267,                /* INDEXREG  */
    EXPR = 268                     /* EXPR  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define DR 258
#define AR 259
#define FPR 260
#define FPCR 261
#define LPC 262
#define ZAR 263
#define ZDR 264
#define LZPC 265
#define CREG 266
#define INDEXREG 267
#define EXPR 268

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 95 "./config/m68k-parse.y"

  struct m68k_indexreg indexreg;
  enum m68k_register reg;
  struct m68k_exp exp;
  unsigned long mask;
  int onereg;
  int trailing_ampersand;

#line 223 "config/m68k-parse.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);



/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_DR = 3,                         /* DR  */
  YYSYMBOL_AR = 4,                         /* AR  */
  YYSYMBOL_FPR = 5,                        /* FPR  */
  YYSYMBOL_FPCR = 6,                       /* FPCR  */
  YYSYMBOL_LPC = 7,                        /* LPC  */
  YYSYMBOL_ZAR = 8,                        /* ZAR  */
  YYSYMBOL_ZDR = 9,                        /* ZDR  */
  YYSYMBOL_LZPC = 10,                      /* LZPC  */
  YYSYMBOL_CREG = 11,                      /* CREG  */
  YYSYMBOL_INDEXREG = 12,                  /* INDEXREG  */
  YYSYMBOL_EXPR = 13,                      /* EXPR  */
  YYSYMBOL_14_ = 14,                       /* '&'  */
  YYSYMBOL_15_ = 15,                       /* '<'  */
  YYSYMBOL_16_ = 16,                       /* '>'  */
  YYSYMBOL_17_ = 17,                       /* '#'  */
  YYSYMBOL_18_ = 18,                       /* '('  */
  YYSYMBOL_19_ = 19,                       /* ')'  */
  YYSYMBOL_20_ = 20,                       /* '+'  */
  YYSYMBOL_21_ = 21,                       /* '-'  */
  YYSYMBOL_22_ = 22,                       /* ','  */
  YYSYMBOL_23_ = 23,                       /* '['  */
  YYSYMBOL_24_ = 24,                       /* ']'  */
  YYSYMBOL_25_ = 25,                       /* '@'  */
  YYSYMBOL_26_ = 26,                       /* '/'  */
  YYSYMBOL_YYACCEPT = 27,                  /* $accept  */
  YYSYMBOL_operand = 28,                   /* operand  */
  YYSYMBOL_optional_ampersand = 29,        /* optional_ampersand  */
  YYSYMBOL_generic_operand = 30,           /* generic_operand  */
  YYSYMBOL_motorola_operand = 31,          /* motorola_operand  */
  YYSYMBOL_mit_operand = 32,               /* mit_operand  */
  YYSYMBOL_zireg = 33,                     /* zireg  */
  YYSYMBOL_zdireg = 34,                    /* zdireg  */
  YYSYMBOL_zadr = 35,                      /* zadr  */
  YYSYMBOL_zdr = 36,                       /* zdr  */
  YYSYMBOL_apc = 37,                       /* apc  */
  YYSYMBOL_zapc = 38,                      /* zapc  */
  YYSYMBOL_optzapc = 39,                   /* optzapc  */
  YYSYMBOL_zpc = 40,                       /* zpc  */
  YYSYMBOL_optczapc = 41,                  /* optczapc  */
  YYSYMBOL_optcexpr = 42,                  /* optcexpr  */
  YYSYMBOL_optexprc = 43,                  /* optexprc  */
  YYSYMBOL_reglist = 44,                   /* reglist  */
  YYSYMBOL_ireglist = 45,                  /* ireglist  */
  YYSYMBOL_reglistpair = 46,               /* reglistpair  */
  YYSYMBOL_reglistreg = 47                 /* reglistreg  */
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
typedef yytype_uint8 yy_state_t;

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
#define YYFINAL  44
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   215

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  27
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  21
/* YYNRULES -- Number of rules.  */
#define YYNRULES  89
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  180

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
       2,     2,     2,     2,     2,    17,     2,     2,    14,     2,
      18,    19,     2,    20,    22,    21,     2,    26,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      15,     2,    16,     2,    25,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    23,     2,    24,     2,     2,     2,     2,     2,     2,
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
       0,   120,   120,   121,   125,   134,   135,   142,   147,   152,
     157,   162,   167,   172,   177,   182,   187,   192,   205,   210,
     215,   220,   230,   240,   250,   255,   260,   265,   272,   283,
     290,   296,   303,   309,   320,   330,   337,   343,   351,   358,
     365,   371,   379,   386,   398,   409,   422,   430,   438,   446,
     456,   463,   471,   478,   492,   493,   506,   507,   519,   520,
     521,   527,   528,   534,   535,   542,   543,   544,   551,   554,
     560,   561,   568,   571,   581,   585,   595,   599,   608,   609,
     613,   625,   629,   630,   634,   641,   651,   655,   659,   663
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
  "\"end of file\"", "error", "\"invalid token\"", "DR", "AR", "FPR",
  "FPCR", "LPC", "ZAR", "ZDR", "LZPC", "CREG", "INDEXREG", "EXPR", "'&'",
  "'<'", "'>'", "'#'", "'('", "')'", "'+'", "'-'", "','", "'['", "']'",
  "'@'", "'/'", "$accept", "operand", "optional_ampersand",
  "generic_operand", "motorola_operand", "mit_operand", "zireg", "zdireg",
  "zadr", "zdr", "apc", "zapc", "optzapc", "zpc", "optczapc", "optcexpr",
  "optexprc", "reglist", "ireglist", "reglistpair", "reglistreg", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-98)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-64)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      89,    14,     9,    31,    35,   -98,   -98,   -98,   -98,     0,
      36,    42,    28,    56,    63,    67,    90,   -98,    75,    75,
     -98,   -98,    86,   -98,    96,   -15,   123,   -98,   -98,   -98,
     -98,   -98,    97,   115,   119,   -98,   120,   -98,   122,    16,
     126,   -98,   127,   157,   -98,   -98,   -98,   -98,    19,   154,
     154,   154,   -98,   140,    29,   144,   -98,   -98,   -98,   123,
     141,    99,    18,    70,   147,   105,   148,   152,   -98,   -98,
     -98,   -98,   -98,   -98,   -98,   142,   -13,   -98,   -98,   146,
     150,   -98,   133,   -98,   140,    60,   146,   149,   133,   153,
     140,   151,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   155,
     158,   -98,   -98,   159,   -98,    62,   143,   154,   154,   -98,
     160,   161,   162,   -98,   133,   163,   164,   165,   166,   116,
     168,   167,   -98,   -98,   -98,   -98,   169,   -98,   173,   -98,
     -98,   -98,   -98,   -98,   174,   176,   133,   116,   177,   175,
     175,   -98,   175,   -98,   175,   170,   178,   -98,   -98,   180,
     181,   175,   -98,   171,   179,   182,   183,   187,   186,   189,
     175,   175,   190,   -98,   -98,   -98,   -98,    79,   143,   195,
     191,   192,   -98,   -98,   193,   194,   -98,   -98,   -98,   -98
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
      68,    86,    87,    88,    89,    64,    67,    66,    13,    14,
       0,     0,     0,     0,     0,     0,     0,     2,     5,     5,
      65,    69,     0,    17,    78,     0,     0,    16,     7,     8,
      15,    61,    63,    64,    67,    62,    66,    56,     0,    76,
      72,    57,     0,     0,     1,     6,     3,     4,    46,     0,
       0,     0,    63,    72,     0,    18,    24,    25,    26,     0,
      72,     0,     0,     0,     0,     0,     0,    76,    47,    48,
      86,    87,    88,    89,    79,    82,    81,    85,    80,     0,
       0,    23,     0,    19,    72,     0,    77,     0,     0,    74,
      72,     0,    73,    36,    59,    70,    60,    71,    54,     0,
       0,    55,    58,     0,    20,     0,     0,     0,     0,    35,
       0,     0,     0,    21,     0,    73,    74,     0,     0,     0,
       0,     0,    30,    22,    32,    34,    49,    77,     0,    83,
      84,    31,    33,    29,     0,     0,     0,     0,     0,    74,
      74,    75,    74,    40,    74,     0,    50,    27,    28,     0,
       0,    74,    38,     0,     0,     0,     0,     0,    76,     0,
      74,    74,     0,    42,    44,    39,    45,     0,     0,     0,
       0,     0,    37,    52,     0,     0,    41,    43,    51,    53
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -98,   -98,   196,   -98,   -98,   -98,   -81,     6,   -98,    -9,
     -98,     2,   -98,   -78,   -38,   -97,   -67,   -98,   -48,   172,
      12
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,    16,    46,    17,    18,    19,   100,    40,   101,   102,
      20,    92,    22,   103,    64,   120,    62,    23,    74,    75,
      76
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     106,   110,    21,    78,   111,    41,    50,   117,    50,   -10,
     118,    51,    25,   108,    -9,    80,    42,    41,    26,   138,
      52,    31,    87,     5,     6,   128,     7,    35,    54,    60,
      37,   -11,    53,   134,   -63,   -12,   135,    67,   142,    68,
      69,    61,   154,   155,    29,   156,   112,   157,    81,    27,
      41,    82,   121,    41,   162,   149,   151,    28,   150,   129,
     130,    85,    77,   170,   171,    84,    31,    32,    90,    30,
      33,    34,    35,    36,    52,    37,    38,     5,     6,   113,
       7,   126,   114,    91,   127,    43,    39,   174,   115,    45,
      44,   168,     1,     2,     3,     4,     5,     6,   173,     7,
       8,   127,     9,    10,    11,    12,    13,    14,    31,    94,
      15,    48,    95,    96,    35,    97,    55,    98,    99,    31,
      94,    88,    49,    89,    96,    35,    31,    52,    98,   141,
       5,     6,    35,     7,    56,    37,    31,    94,    57,    58,
      95,    96,    35,    97,    59,    98,    31,    94,    63,    65,
      52,    96,    35,     5,     6,    98,     7,    70,    71,    72,
      73,    66,    79,    86,    83,   105,    93,   104,   107,   109,
     122,     0,    24,   116,   123,   119,     0,   124,   125,   131,
     132,   133,     0,     0,   141,   136,   137,   143,   158,   139,
     140,   144,   146,   147,   145,   148,   152,   153,   163,   167,
       0,   164,   165,   159,   160,   161,   166,   169,   175,   172,
     176,   177,   178,   179,     0,    47
};

static const yytype_int16 yycheck[] =
{
      67,    82,     0,    51,    82,    14,    21,    88,    21,     0,
      88,    26,     0,    26,     0,    53,    14,    26,    18,   116,
       4,     3,    60,     7,     8,   106,    10,     9,    26,    13,
      12,     0,    26,   114,    25,     0,   114,    18,   119,    20,
      21,    39,   139,   140,    16,   142,    84,   144,    19,    13,
      59,    22,    90,    62,   151,   136,   137,    15,   136,   107,
     108,    59,    50,   160,   161,    59,     3,     4,    62,    13,
       7,     8,     9,    10,     4,    12,    13,     7,     8,    19,
      10,    19,    22,    13,    22,    18,    23,   168,    86,    14,
       0,   158,     3,     4,     5,     6,     7,     8,    19,    10,
      11,    22,    13,    14,    15,    16,    17,    18,     3,     4,
      21,    25,     7,     8,     9,    10,    19,    12,    13,     3,
       4,    22,    26,    24,     8,     9,     3,     4,    12,    13,
       7,     8,     9,    10,    19,    12,     3,     4,    19,    19,
       7,     8,     9,    10,    22,    12,     3,     4,    22,    22,
       4,     8,     9,     7,     8,    12,    10,     3,     4,     5,
       6,     4,    22,    22,    20,    13,    19,    19,    26,    19,
      19,    -1,     0,    24,    19,    22,    -1,    19,    19,    19,
      19,    19,    -1,    -1,    13,    22,    22,    19,    18,    24,
      24,    24,    19,    19,    25,    19,    19,    22,    19,    13,
      -1,    19,    19,    25,    24,    24,    19,    18,    13,    19,
      19,    19,    19,    19,    -1,    19
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    10,    11,    13,
      14,    15,    16,    17,    18,    21,    28,    30,    31,    32,
      37,    38,    39,    44,    46,    47,    18,    13,    15,    16,
      13,     3,     4,     7,     8,     9,    10,    12,    13,    23,
      34,    36,    38,    18,     0,    14,    29,    29,    25,    26,
      21,    26,     4,    34,    38,    19,    19,    19,    19,    22,
      13,    38,    43,    22,    41,    22,     4,    18,    20,    21,
       3,     4,     5,     6,    45,    46,    47,    47,    45,    22,
      41,    19,    22,    20,    34,    38,    22,    41,    22,    24,
      34,    13,    38,    19,     4,     7,     8,    10,    12,    13,
      33,    35,    36,    40,    19,    13,    43,    26,    26,    19,
      33,    40,    41,    19,    22,    38,    24,    33,    40,    22,
      42,    41,    19,    19,    19,    19,    19,    22,    33,    45,
      45,    19,    19,    19,    33,    40,    22,    22,    42,    24,
      24,    13,    33,    19,    24,    25,    19,    19,    19,    33,
      40,    33,    19,    22,    42,    42,    42,    42,    18,    25,
      24,    24,    42,    19,    19,    19,    19,    13,    43,    18,
      42,    42,    19,    19,    33,    13,    19,    19,    19,    19
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    27,    28,    28,    28,    29,    29,    30,    30,    30,
      30,    30,    30,    30,    30,    30,    30,    30,    31,    31,
      31,    31,    31,    31,    31,    31,    31,    31,    31,    31,
      31,    31,    31,    31,    31,    31,    31,    31,    31,    31,
      31,    31,    31,    31,    31,    31,    32,    32,    32,    32,
      32,    32,    32,    32,    33,    33,    34,    34,    35,    35,
      35,    36,    36,    37,    37,    38,    38,    38,    39,    39,
      40,    40,    41,    41,    42,    42,    43,    43,    44,    44,
      44,    45,    45,    45,    45,    46,    47,    47,    47,    47
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     2,     0,     1,     2,     2,     1,
       1,     1,     1,     1,     1,     2,     2,     1,     3,     4,
       4,     5,     5,     4,     3,     3,     3,     7,     7,     6,
       5,     6,     5,     6,     5,     5,     4,     9,     7,     8,
       6,    10,     8,    10,     8,     8,     2,     3,     3,     5,
       6,    10,     9,    10,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     1,     0,     2,     0,     2,     0,     2,     1,     3,
       3,     1,     1,     3,     3,     3,     1,     1,     1,     1
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
  case 3: /* operand: motorola_operand optional_ampersand  */
#line 122 "./config/m68k-parse.y"
                {
		  op->trailing_ampersand = (yyvsp[0].trailing_ampersand);
		}
#line 1364 "config/m68k-parse.c"
    break;

  case 4: /* operand: mit_operand optional_ampersand  */
#line 126 "./config/m68k-parse.y"
                {
		  op->trailing_ampersand = (yyvsp[0].trailing_ampersand);
		}
#line 1372 "config/m68k-parse.c"
    break;

  case 5: /* optional_ampersand: %empty  */
#line 134 "./config/m68k-parse.y"
                { (yyval.trailing_ampersand) = 0; }
#line 1378 "config/m68k-parse.c"
    break;

  case 6: /* optional_ampersand: '&'  */
#line 136 "./config/m68k-parse.y"
                { (yyval.trailing_ampersand) = 1; }
#line 1384 "config/m68k-parse.c"
    break;

  case 7: /* generic_operand: '<' '<'  */
#line 143 "./config/m68k-parse.y"
                {
		  op->mode = LSH;
		}
#line 1392 "config/m68k-parse.c"
    break;

  case 8: /* generic_operand: '>' '>'  */
#line 148 "./config/m68k-parse.y"
                {
		  op->mode = RSH;
		}
#line 1400 "config/m68k-parse.c"
    break;

  case 9: /* generic_operand: DR  */
#line 153 "./config/m68k-parse.y"
                {
		  op->mode = DREG;
		  op->reg = (yyvsp[0].reg);
		}
#line 1409 "config/m68k-parse.c"
    break;

  case 10: /* generic_operand: AR  */
#line 158 "./config/m68k-parse.y"
                {
		  op->mode = AREG;
		  op->reg = (yyvsp[0].reg);
		}
#line 1418 "config/m68k-parse.c"
    break;

  case 11: /* generic_operand: FPR  */
#line 163 "./config/m68k-parse.y"
                {
		  op->mode = FPREG;
		  op->reg = (yyvsp[0].reg);
		}
#line 1427 "config/m68k-parse.c"
    break;

  case 12: /* generic_operand: FPCR  */
#line 168 "./config/m68k-parse.y"
                {
		  op->mode = CONTROL;
		  op->reg = (yyvsp[0].reg);
		}
#line 1436 "config/m68k-parse.c"
    break;

  case 13: /* generic_operand: CREG  */
#line 173 "./config/m68k-parse.y"
                {
		  op->mode = CONTROL;
		  op->reg = (yyvsp[0].reg);
		}
#line 1445 "config/m68k-parse.c"
    break;

  case 14: /* generic_operand: EXPR  */
#line 178 "./config/m68k-parse.y"
                {
		  op->mode = ABSL;
		  op->disp = (yyvsp[0].exp);
		}
#line 1454 "config/m68k-parse.c"
    break;

  case 15: /* generic_operand: '#' EXPR  */
#line 183 "./config/m68k-parse.y"
                {
		  op->mode = IMMED;
		  op->disp = (yyvsp[0].exp);
		}
#line 1463 "config/m68k-parse.c"
    break;

  case 16: /* generic_operand: '&' EXPR  */
#line 188 "./config/m68k-parse.y"
                {
		  op->mode = IMMED;
		  op->disp = (yyvsp[0].exp);
		}
#line 1472 "config/m68k-parse.c"
    break;

  case 17: /* generic_operand: reglist  */
#line 193 "./config/m68k-parse.y"
                {
		  op->mode = REGLST;
		  op->mask = (yyvsp[0].mask);
		}
#line 1481 "config/m68k-parse.c"
    break;

  case 18: /* motorola_operand: '(' AR ')'  */
#line 206 "./config/m68k-parse.y"
                {
		  op->mode = AINDR;
		  op->reg = (yyvsp[-1].reg);
		}
#line 1490 "config/m68k-parse.c"
    break;

  case 19: /* motorola_operand: '(' AR ')' '+'  */
#line 211 "./config/m68k-parse.y"
                {
		  op->mode = AINC;
		  op->reg = (yyvsp[-2].reg);
		}
#line 1499 "config/m68k-parse.c"
    break;

  case 20: /* motorola_operand: '-' '(' AR ')'  */
#line 216 "./config/m68k-parse.y"
                {
		  op->mode = ADEC;
		  op->reg = (yyvsp[-1].reg);
		}
#line 1508 "config/m68k-parse.c"
    break;

  case 21: /* motorola_operand: '(' EXPR ',' zapc ')'  */
#line 221 "./config/m68k-parse.y"
                {
		  op->reg = (yyvsp[-1].reg);
		  op->disp = (yyvsp[-3].exp);
		  if (((yyvsp[-1].reg) >= ZADDR0 && (yyvsp[-1].reg) <= ZADDR7)
		      || (yyvsp[-1].reg) == ZPC)
		    op->mode = BASE;
		  else
		    op->mode = DISP;
		}
#line 1522 "config/m68k-parse.c"
    break;

  case 22: /* motorola_operand: '(' zapc ',' EXPR ')'  */
#line 231 "./config/m68k-parse.y"
                {
		  op->reg = (yyvsp[-3].reg);
		  op->disp = (yyvsp[-1].exp);
		  if (((yyvsp[-3].reg) >= ZADDR0 && (yyvsp[-3].reg) <= ZADDR7)
		      || (yyvsp[-3].reg) == ZPC)
		    op->mode = BASE;
		  else
		    op->mode = DISP;
		}
#line 1536 "config/m68k-parse.c"
    break;

  case 23: /* motorola_operand: EXPR '(' zapc ')'  */
#line 241 "./config/m68k-parse.y"
                {
		  op->reg = (yyvsp[-1].reg);
		  op->disp = (yyvsp[-3].exp);
		  if (((yyvsp[-1].reg) >= ZADDR0 && (yyvsp[-1].reg) <= ZADDR7)
		      || (yyvsp[-1].reg) == ZPC)
		    op->mode = BASE;
		  else
		    op->mode = DISP;
		}
#line 1550 "config/m68k-parse.c"
    break;

  case 24: /* motorola_operand: '(' LPC ')'  */
#line 251 "./config/m68k-parse.y"
                {
		  op->mode = DISP;
		  op->reg = (yyvsp[-1].reg);
		}
#line 1559 "config/m68k-parse.c"
    break;

  case 25: /* motorola_operand: '(' ZAR ')'  */
#line 256 "./config/m68k-parse.y"
                {
		  op->mode = BASE;
		  op->reg = (yyvsp[-1].reg);
		}
#line 1568 "config/m68k-parse.c"
    break;

  case 26: /* motorola_operand: '(' LZPC ')'  */
#line 261 "./config/m68k-parse.y"
                {
		  op->mode = BASE;
		  op->reg = (yyvsp[-1].reg);
		}
#line 1577 "config/m68k-parse.c"
    break;

  case 27: /* motorola_operand: '(' EXPR ',' zapc ',' zireg ')'  */
#line 266 "./config/m68k-parse.y"
                {
		  op->mode = BASE;
		  op->reg = (yyvsp[-3].reg);
		  op->disp = (yyvsp[-5].exp);
		  op->index = (yyvsp[-1].indexreg);
		}
#line 1588 "config/m68k-parse.c"
    break;

  case 28: /* motorola_operand: '(' EXPR ',' zapc ',' zpc ')'  */
#line 273 "./config/m68k-parse.y"
                {
		  if ((yyvsp[-3].reg) == PC || (yyvsp[-3].reg) == ZPC)
		    yyerror (_("syntax error"));
		  op->mode = BASE;
		  op->reg = (yyvsp[-1].reg);
		  op->disp = (yyvsp[-5].exp);
		  op->index.reg = (yyvsp[-3].reg);
		  op->index.size = SIZE_UNSPEC;
		  op->index.scale = 1;
		}
#line 1603 "config/m68k-parse.c"
    break;

  case 29: /* motorola_operand: '(' EXPR ',' zdireg optczapc ')'  */
#line 284 "./config/m68k-parse.y"
                {
		  op->mode = BASE;
		  op->reg = (yyvsp[-1].reg);
		  op->disp = (yyvsp[-4].exp);
		  op->index = (yyvsp[-2].indexreg);
		}
#line 1614 "config/m68k-parse.c"
    break;

  case 30: /* motorola_operand: '(' zdireg ',' EXPR ')'  */
#line 291 "./config/m68k-parse.y"
                {
		  op->mode = BASE;
		  op->disp = (yyvsp[-1].exp);
		  op->index = (yyvsp[-3].indexreg);
		}
#line 1624 "config/m68k-parse.c"
    break;

  case 31: /* motorola_operand: EXPR '(' zapc ',' zireg ')'  */
#line 297 "./config/m68k-parse.y"
                {
		  op->mode = BASE;
		  op->reg = (yyvsp[-3].reg);
		  op->disp = (yyvsp[-5].exp);
		  op->index = (yyvsp[-1].indexreg);
		}
#line 1635 "config/m68k-parse.c"
    break;

  case 32: /* motorola_operand: '(' zapc ',' zireg ')'  */
#line 304 "./config/m68k-parse.y"
                {
		  op->mode = BASE;
		  op->reg = (yyvsp[-3].reg);
		  op->index = (yyvsp[-1].indexreg);
		}
#line 1645 "config/m68k-parse.c"
    break;

  case 33: /* motorola_operand: EXPR '(' zapc ',' zpc ')'  */
#line 310 "./config/m68k-parse.y"
                {
		  if ((yyvsp[-3].reg) == PC || (yyvsp[-3].reg) == ZPC)
		    yyerror (_("syntax error"));
		  op->mode = BASE;
		  op->reg = (yyvsp[-1].reg);
		  op->disp = (yyvsp[-5].exp);
		  op->index.reg = (yyvsp[-3].reg);
		  op->index.size = SIZE_UNSPEC;
		  op->index.scale = 1;
		}
#line 1660 "config/m68k-parse.c"
    break;

  case 34: /* motorola_operand: '(' zapc ',' zpc ')'  */
#line 321 "./config/m68k-parse.y"
                {
		  if ((yyvsp[-3].reg) == PC || (yyvsp[-3].reg) == ZPC)
		    yyerror (_("syntax error"));
		  op->mode = BASE;
		  op->reg = (yyvsp[-1].reg);
		  op->index.reg = (yyvsp[-3].reg);
		  op->index.size = SIZE_UNSPEC;
		  op->index.scale = 1;
		}
#line 1674 "config/m68k-parse.c"
    break;

  case 35: /* motorola_operand: EXPR '(' zdireg optczapc ')'  */
#line 331 "./config/m68k-parse.y"
                {
		  op->mode = BASE;
		  op->reg = (yyvsp[-1].reg);
		  op->disp = (yyvsp[-4].exp);
		  op->index = (yyvsp[-2].indexreg);
		}
#line 1685 "config/m68k-parse.c"
    break;

  case 36: /* motorola_operand: '(' zdireg optczapc ')'  */
#line 338 "./config/m68k-parse.y"
                {
		  op->mode = BASE;
		  op->reg = (yyvsp[-1].reg);
		  op->index = (yyvsp[-2].indexreg);
		}
#line 1695 "config/m68k-parse.c"
    break;

  case 37: /* motorola_operand: '(' '[' EXPR optczapc ']' ',' zireg optcexpr ')'  */
#line 344 "./config/m68k-parse.y"
                {
		  op->mode = POST;
		  op->reg = (yyvsp[-5].reg);
		  op->disp = (yyvsp[-6].exp);
		  op->index = (yyvsp[-2].indexreg);
		  op->odisp = (yyvsp[-1].exp);
		}
#line 1707 "config/m68k-parse.c"
    break;

  case 38: /* motorola_operand: '(' '[' EXPR optczapc ']' optcexpr ')'  */
#line 352 "./config/m68k-parse.y"
                {
		  op->mode = POST;
		  op->reg = (yyvsp[-3].reg);
		  op->disp = (yyvsp[-4].exp);
		  op->odisp = (yyvsp[-1].exp);
		}
#line 1718 "config/m68k-parse.c"
    break;

  case 39: /* motorola_operand: '(' '[' zapc ']' ',' zireg optcexpr ')'  */
#line 359 "./config/m68k-parse.y"
                {
		  op->mode = POST;
		  op->reg = (yyvsp[-5].reg);
		  op->index = (yyvsp[-2].indexreg);
		  op->odisp = (yyvsp[-1].exp);
		}
#line 1729 "config/m68k-parse.c"
    break;

  case 40: /* motorola_operand: '(' '[' zapc ']' optcexpr ')'  */
#line 366 "./config/m68k-parse.y"
                {
		  op->mode = POST;
		  op->reg = (yyvsp[-3].reg);
		  op->odisp = (yyvsp[-1].exp);
		}
#line 1739 "config/m68k-parse.c"
    break;

  case 41: /* motorola_operand: '(' '[' EXPR ',' zapc ',' zireg ']' optcexpr ')'  */
#line 372 "./config/m68k-parse.y"
                {
		  op->mode = PRE;
		  op->reg = (yyvsp[-5].reg);
		  op->disp = (yyvsp[-7].exp);
		  op->index = (yyvsp[-3].indexreg);
		  op->odisp = (yyvsp[-1].exp);
		}
#line 1751 "config/m68k-parse.c"
    break;

  case 42: /* motorola_operand: '(' '[' zapc ',' zireg ']' optcexpr ')'  */
#line 380 "./config/m68k-parse.y"
                {
		  op->mode = PRE;
		  op->reg = (yyvsp[-5].reg);
		  op->index = (yyvsp[-3].indexreg);
		  op->odisp = (yyvsp[-1].exp);
		}
#line 1762 "config/m68k-parse.c"
    break;

  case 43: /* motorola_operand: '(' '[' EXPR ',' zapc ',' zpc ']' optcexpr ')'  */
#line 387 "./config/m68k-parse.y"
                {
		  if ((yyvsp[-5].reg) == PC || (yyvsp[-5].reg) == ZPC)
		    yyerror (_("syntax error"));
		  op->mode = PRE;
		  op->reg = (yyvsp[-3].reg);
		  op->disp = (yyvsp[-7].exp);
		  op->index.reg = (yyvsp[-5].reg);
		  op->index.size = SIZE_UNSPEC;
		  op->index.scale = 1;
		  op->odisp = (yyvsp[-1].exp);
		}
#line 1778 "config/m68k-parse.c"
    break;

  case 44: /* motorola_operand: '(' '[' zapc ',' zpc ']' optcexpr ')'  */
#line 399 "./config/m68k-parse.y"
                {
		  if ((yyvsp[-5].reg) == PC || (yyvsp[-5].reg) == ZPC)
		    yyerror (_("syntax error"));
		  op->mode = PRE;
		  op->reg = (yyvsp[-3].reg);
		  op->index.reg = (yyvsp[-5].reg);
		  op->index.size = SIZE_UNSPEC;
		  op->index.scale = 1;
		  op->odisp = (yyvsp[-1].exp);
		}
#line 1793 "config/m68k-parse.c"
    break;

  case 45: /* motorola_operand: '(' '[' optexprc zdireg optczapc ']' optcexpr ')'  */
#line 410 "./config/m68k-parse.y"
                {
		  op->mode = PRE;
		  op->reg = (yyvsp[-3].reg);
		  op->disp = (yyvsp[-5].exp);
		  op->index = (yyvsp[-4].indexreg);
		  op->odisp = (yyvsp[-1].exp);
		}
#line 1805 "config/m68k-parse.c"
    break;

  case 46: /* mit_operand: optzapc '@'  */
#line 423 "./config/m68k-parse.y"
                {
		  /* We use optzapc to avoid a shift/reduce conflict.  */
		  if ((yyvsp[-1].reg) < ADDR0 || (yyvsp[-1].reg) > ADDR7)
		    yyerror (_("syntax error"));
		  op->mode = AINDR;
		  op->reg = (yyvsp[-1].reg);
		}
#line 1817 "config/m68k-parse.c"
    break;

  case 47: /* mit_operand: optzapc '@' '+'  */
#line 431 "./config/m68k-parse.y"
                {
		  /* We use optzapc to avoid a shift/reduce conflict.  */
		  if ((yyvsp[-2].reg) < ADDR0 || (yyvsp[-2].reg) > ADDR7)
		    yyerror (_("syntax error"));
		  op->mode = AINC;
		  op->reg = (yyvsp[-2].reg);
		}
#line 1829 "config/m68k-parse.c"
    break;

  case 48: /* mit_operand: optzapc '@' '-'  */
#line 439 "./config/m68k-parse.y"
                {
		  /* We use optzapc to avoid a shift/reduce conflict.  */
		  if ((yyvsp[-2].reg) < ADDR0 || (yyvsp[-2].reg) > ADDR7)
		    yyerror (_("syntax error"));
		  op->mode = ADEC;
		  op->reg = (yyvsp[-2].reg);
		}
#line 1841 "config/m68k-parse.c"
    break;

  case 49: /* mit_operand: optzapc '@' '(' EXPR ')'  */
#line 447 "./config/m68k-parse.y"
                {
		  op->reg = (yyvsp[-4].reg);
		  op->disp = (yyvsp[-1].exp);
		  if (((yyvsp[-4].reg) >= ZADDR0 && (yyvsp[-4].reg) <= ZADDR7)
		      || (yyvsp[-4].reg) == ZPC)
		    op->mode = BASE;
		  else
		    op->mode = DISP;
		}
#line 1855 "config/m68k-parse.c"
    break;

  case 50: /* mit_operand: optzapc '@' '(' optexprc zireg ')'  */
#line 457 "./config/m68k-parse.y"
                {
		  op->mode = BASE;
		  op->reg = (yyvsp[-5].reg);
		  op->disp = (yyvsp[-2].exp);
		  op->index = (yyvsp[-1].indexreg);
		}
#line 1866 "config/m68k-parse.c"
    break;

  case 51: /* mit_operand: optzapc '@' '(' EXPR ')' '@' '(' optexprc zireg ')'  */
#line 464 "./config/m68k-parse.y"
                {
		  op->mode = POST;
		  op->reg = (yyvsp[-9].reg);
		  op->disp = (yyvsp[-6].exp);
		  op->index = (yyvsp[-1].indexreg);
		  op->odisp = (yyvsp[-2].exp);
		}
#line 1878 "config/m68k-parse.c"
    break;

  case 52: /* mit_operand: optzapc '@' '(' EXPR ')' '@' '(' EXPR ')'  */
#line 472 "./config/m68k-parse.y"
                {
		  op->mode = POST;
		  op->reg = (yyvsp[-8].reg);
		  op->disp = (yyvsp[-5].exp);
		  op->odisp = (yyvsp[-1].exp);
		}
#line 1889 "config/m68k-parse.c"
    break;

  case 53: /* mit_operand: optzapc '@' '(' optexprc zireg ')' '@' '(' EXPR ')'  */
#line 479 "./config/m68k-parse.y"
                {
		  op->mode = PRE;
		  op->reg = (yyvsp[-9].reg);
		  op->disp = (yyvsp[-6].exp);
		  op->index = (yyvsp[-5].indexreg);
		  op->odisp = (yyvsp[-1].exp);
		}
#line 1901 "config/m68k-parse.c"
    break;

  case 55: /* zireg: zadr  */
#line 494 "./config/m68k-parse.y"
                {
		  (yyval.indexreg).reg = (yyvsp[0].reg);
		  (yyval.indexreg).size = SIZE_UNSPEC;
		  (yyval.indexreg).scale = 1;
		}
#line 1911 "config/m68k-parse.c"
    break;

  case 57: /* zdireg: zdr  */
#line 508 "./config/m68k-parse.y"
                {
		  (yyval.indexreg).reg = (yyvsp[0].reg);
		  (yyval.indexreg).size = SIZE_UNSPEC;
		  (yyval.indexreg).scale = 1;
		}
#line 1921 "config/m68k-parse.c"
    break;

  case 68: /* optzapc: %empty  */
#line 551 "./config/m68k-parse.y"
                {
		  (yyval.reg) = ZADDR0;
		}
#line 1929 "config/m68k-parse.c"
    break;

  case 72: /* optczapc: %empty  */
#line 568 "./config/m68k-parse.y"
                {
		  (yyval.reg) = ZADDR0;
		}
#line 1937 "config/m68k-parse.c"
    break;

  case 73: /* optczapc: ',' zapc  */
#line 572 "./config/m68k-parse.y"
                {
		  (yyval.reg) = (yyvsp[0].reg);
		}
#line 1945 "config/m68k-parse.c"
    break;

  case 74: /* optcexpr: %empty  */
#line 581 "./config/m68k-parse.y"
                {
		  (yyval.exp).exp.X_op = O_absent;
		  (yyval.exp).size = SIZE_UNSPEC;
		}
#line 1954 "config/m68k-parse.c"
    break;

  case 75: /* optcexpr: ',' EXPR  */
#line 586 "./config/m68k-parse.y"
                {
		  (yyval.exp) = (yyvsp[0].exp);
		}
#line 1962 "config/m68k-parse.c"
    break;

  case 76: /* optexprc: %empty  */
#line 595 "./config/m68k-parse.y"
                {
		  (yyval.exp).exp.X_op = O_absent;
		  (yyval.exp).size = SIZE_UNSPEC;
		}
#line 1971 "config/m68k-parse.c"
    break;

  case 77: /* optexprc: EXPR ','  */
#line 600 "./config/m68k-parse.y"
                {
		  (yyval.exp) = (yyvsp[-1].exp);
		}
#line 1979 "config/m68k-parse.c"
    break;

  case 79: /* reglist: reglistpair '/' ireglist  */
#line 610 "./config/m68k-parse.y"
                {
		  (yyval.mask) = (yyvsp[-2].mask) | (yyvsp[0].mask);
		}
#line 1987 "config/m68k-parse.c"
    break;

  case 80: /* reglist: reglistreg '/' ireglist  */
#line 614 "./config/m68k-parse.y"
                {
		  (yyval.mask) = (1 << (yyvsp[-2].onereg)) | (yyvsp[0].mask);
		}
#line 1995 "config/m68k-parse.c"
    break;

  case 81: /* ireglist: reglistreg  */
#line 626 "./config/m68k-parse.y"
                {
		  (yyval.mask) = 1 << (yyvsp[0].onereg);
		}
#line 2003 "config/m68k-parse.c"
    break;

  case 83: /* ireglist: reglistpair '/' ireglist  */
#line 631 "./config/m68k-parse.y"
                {
		  (yyval.mask) = (yyvsp[-2].mask) | (yyvsp[0].mask);
		}
#line 2011 "config/m68k-parse.c"
    break;

  case 84: /* ireglist: reglistreg '/' ireglist  */
#line 635 "./config/m68k-parse.y"
                {
		  (yyval.mask) = (1 << (yyvsp[-2].onereg)) | (yyvsp[0].mask);
		}
#line 2019 "config/m68k-parse.c"
    break;

  case 85: /* reglistpair: reglistreg '-' reglistreg  */
#line 642 "./config/m68k-parse.y"
                {
		  if ((yyvsp[-2].onereg) <= (yyvsp[0].onereg))
		    (yyval.mask) = (1 << ((yyvsp[0].onereg) + 1)) - 1 - ((1 << (yyvsp[-2].onereg)) - 1);
		  else
		    (yyval.mask) = (1 << ((yyvsp[-2].onereg) + 1)) - 1 - ((1 << (yyvsp[0].onereg)) - 1);
		}
#line 2030 "config/m68k-parse.c"
    break;

  case 86: /* reglistreg: DR  */
#line 652 "./config/m68k-parse.y"
                {
		  (yyval.onereg) = (yyvsp[0].reg) - DATA0;
		}
#line 2038 "config/m68k-parse.c"
    break;

  case 87: /* reglistreg: AR  */
#line 656 "./config/m68k-parse.y"
                {
		  (yyval.onereg) = (yyvsp[0].reg) - ADDR0 + 8;
		}
#line 2046 "config/m68k-parse.c"
    break;

  case 88: /* reglistreg: FPR  */
#line 660 "./config/m68k-parse.y"
                {
		  (yyval.onereg) = (yyvsp[0].reg) - FP0 + 16;
		}
#line 2054 "config/m68k-parse.c"
    break;

  case 89: /* reglistreg: FPCR  */
#line 664 "./config/m68k-parse.y"
                {
		  if ((yyvsp[0].reg) == FPI)
		    (yyval.onereg) = 24;
		  else if ((yyvsp[0].reg) == FPS)
		    (yyval.onereg) = 25;
		  else
		    (yyval.onereg) = 26;
		}
#line 2067 "config/m68k-parse.c"
    break;


#line 2071 "config/m68k-parse.c"

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

#line 674 "./config/m68k-parse.y"


/* The string to parse is stored here, and modified by yylex.  */

static char *str;

/* The original string pointer.  */

static char *strorig;

/* If *CCP could be a register, return the register number and advance
   *CCP.  Otherwise don't change *CCP, and return 0.  */

static enum m68k_register
m68k_reg_parse (char **ccp)
{
  char *start = *ccp;
  char c;
  char *p;
  symbolS *symbolp;

  if (flag_reg_prefix_optional)
    {
      if (*start == REGISTER_PREFIX)
	start++;
      p = start;
    }
  else
    {
      if (*start != REGISTER_PREFIX)
	return 0;
      p = start + 1;
    }

  if (! is_name_beginner (*p))
    return 0;

  p++;
  while (is_part_of_name (*p) && *p != '.' && *p != ':' && *p != '*')
    p++;

  c = *p;
  *p = 0;
  symbolp = symbol_find (start);
  *p = c;

  if (symbolp != NULL && S_GET_SEGMENT (symbolp) == reg_section)
    {
      *ccp = p;
      return S_GET_VALUE (symbolp);
    }

  /* In MRI mode, something like foo.bar can be equated to a register
     name.  */
  while (flag_mri && c == '.')
    {
      ++p;
      while (is_part_of_name (*p) && *p != '.' && *p != ':' && *p != '*')
	p++;
      c = *p;
      *p = '\0';
      symbolp = symbol_find (start);
      *p = c;
      if (symbolp != NULL && S_GET_SEGMENT (symbolp) == reg_section)
	{
	  *ccp = p;
	  return S_GET_VALUE (symbolp);
	}
    }

  return 0;
}

/* The lexer.  */

static int
yylex (void)
{
  enum m68k_register reg;
  char *s;
  int parens;
  int c = 0;
  int tail = 0;

  if (*str == ' ')
    ++str;

  if (*str == '\0')
    return 0;

  /* Various special characters are just returned directly.  */
  switch (*str)
    {
    case '@':
      /* In MRI mode, this can be the start of an octal number.  */
      if (flag_mri)
	{
	  if (ISDIGIT (str[1])
	      || ((str[1] == '+' || str[1] == '-')
		  && ISDIGIT (str[2])))
	    break;
	}
      /* Fall through.  */
    case '#':
    case '&':
    case ',':
    case ')':
    case '/':
    case '[':
    case ']':
    case '<':
    case '>':
      return *str++;
    case '+':
      /* It so happens that a '+' can only appear at the end of an
	 operand, or if it is trailed by an '&'(see mac load insn).
	 If it appears anywhere else, it must be a unary.  */
      if (str[1] == '\0' || (str[1] == '&' && str[2] == '\0'))
	return *str++;
      break;
    case '-':
      /* A '-' can only appear in -(ar), rn-rn, or ar@-.  If it
         appears anywhere else, it must be a unary minus on an
         expression, unless it it trailed by a '&'(see mac load insn).  */
      if (str[1] == '\0' || (str[1] == '&' && str[2] == '\0'))
	return *str++;
      s = str + 1;
      if (*s == '(')
	++s;
      if (m68k_reg_parse (&s) != 0)
	return *str++;
      break;
    case '(':
      /* A '(' can only appear in `(reg)', `(expr,...', `([', `@(', or
         `)('.  If it appears anywhere else, it must be starting an
         expression.  */
      if (str[1] == '['
	  || (str > strorig
	      && (str[-1] == '@'
		  || str[-1] == ')')))
	return *str++;
      s = str + 1;
      if (m68k_reg_parse (&s) != 0)
	return *str++;
      /* Check for the case of '(expr,...' by scanning ahead.  If we
         find a comma outside of balanced parentheses, we return '('.
         If we find an unbalanced right parenthesis, then presumably
         the '(' really starts an expression.  */
      parens = 0;
      for (s = str + 1; *s != '\0'; s++)
	{
	  if (*s == '(')
	    ++parens;
	  else if (*s == ')')
	    {
	      if (parens == 0)
		break;
	      --parens;
	    }
	  else if (*s == ',' && parens == 0)
	    {
	      /* A comma can not normally appear in an expression, so
		 this is a case of '(expr,...'.  */
	      return *str++;
	    }
	}
    }

  /* See if it's a register.  */

  reg = m68k_reg_parse (&str);
  if (reg != 0)
    {
      int ret;

      yylval.reg = reg;

      if (reg >= DATA0 && reg <= DATA7)
	ret = DR;
      else if (reg >= ADDR0 && reg <= ADDR7)
	ret = AR;
      else if (reg >= FP0 && reg <= FP7)
	return FPR;
      else if (reg == FPI
	       || reg == FPS
	       || reg == FPC)
	return FPCR;
      else if (reg == PC)
	return LPC;
      else if (reg >= ZDATA0 && reg <= ZDATA7)
	ret = ZDR;
      else if (reg >= ZADDR0 && reg <= ZADDR7)
	ret = ZAR;
      else if (reg == ZPC)
	return LZPC;
      else
	return CREG;

      /* If we get here, we have a data or address register.  We
	 must check for a size or scale; if we find one, we must
	 return INDEXREG.  */

      s = str;

      if (*s != '.' && *s != ':' && *s != '*')
	return ret;

      yylval.indexreg.reg = reg;

      if (*s != '.' && *s != ':')
	yylval.indexreg.size = SIZE_UNSPEC;
      else
	{
	  ++s;
	  switch (*s)
	    {
	    case 'w':
	    case 'W':
	      yylval.indexreg.size = SIZE_WORD;
	      ++s;
	      break;
	    case 'l':
	    case 'L':
	      yylval.indexreg.size = SIZE_LONG;
	      ++s;
	      break;
	    default:
	      yyerror (_("illegal size specification"));
	      yylval.indexreg.size = SIZE_UNSPEC;
	      break;
	    }
	}

      yylval.indexreg.scale = 1;

      if (*s == '*' || *s == ':')
	{
	  expressionS scale;

	  ++s;

	  temp_ilp (s);
	  expression (&scale);
	  s = input_line_pointer;
	  restore_ilp ();

	  if (scale.X_op != O_constant)
	    yyerror (_("scale specification must resolve to a number"));
	  else
	    {
	      switch (scale.X_add_number)
		{
		case 1:
		case 2:
		case 4:
		case 8:
		  yylval.indexreg.scale = scale.X_add_number;
		  break;
		default:
		  yyerror (_("invalid scale value"));
		  break;
		}
	    }
	}

      str = s;

      return INDEXREG;
    }

  /* It must be an expression.  Before we call expression, we need to
     look ahead to see if there is a size specification.  We must do
     that first, because otherwise foo.l will be treated as the symbol
     foo.l, rather than as the symbol foo with a long size
     specification.  The grammar requires that all expressions end at
     the end of the operand, or with ',', '(', ']', ')'.  */

  parens = 0;
  for (s = str; *s != '\0'; s++)
    {
      if (*s == '(')
	{
	  if (parens == 0
	      && s > str
	      && (s[-1] == ')' || ISALNUM (s[-1])))
	    break;
	  ++parens;
	}
      else if (*s == ')')
	{
	  if (parens == 0)
	    break;
	  --parens;
	}
      else if (parens == 0
	       && (*s == ',' || *s == ']'))
	break;
    }

  yylval.exp.size = SIZE_UNSPEC;
  if (s <= str + 2
      || (s[-2] != '.' && s[-2] != ':'))
    tail = 0;
  else
    {
      switch (s[-1])
	{
	case 's':
	case 'S':
	case 'b':
	case 'B':
	  yylval.exp.size = SIZE_BYTE;
	  break;
	case 'w':
	case 'W':
	  yylval.exp.size = SIZE_WORD;
	  break;
	case 'l':
	case 'L':
	  yylval.exp.size = SIZE_LONG;
	  break;
	default:
	  break;
	}
      if (yylval.exp.size != SIZE_UNSPEC)
	tail = 2;
    }

#ifdef OBJ_ELF
  {
    /* Look for @PLTPC, etc.  */
    char *cp;

    yylval.exp.pic_reloc = pic_none;
    cp = s - tail;
    if (cp - 7 > str && cp[-7] == '@')
      {
	if (startswith (cp - 7, "@TLSLDM"))
	  {
	    yylval.exp.pic_reloc = pic_tls_ldm;
	    tail += 7;
	  }
	else if (startswith (cp - 7, "@TLSLDO"))
	  {
	    yylval.exp.pic_reloc = pic_tls_ldo;
	    tail += 7;
	  }
      }
    else if (cp - 6 > str && cp[-6] == '@')
      {
	if (startswith (cp - 6, "@PLTPC"))
	  {
	    yylval.exp.pic_reloc = pic_plt_pcrel;
	    tail += 6;
	  }
	else if (startswith (cp - 6, "@GOTPC"))
	  {
	    yylval.exp.pic_reloc = pic_got_pcrel;
	    tail += 6;
	  }
	else if (startswith (cp - 6, "@TLSGD"))
	  {
	    yylval.exp.pic_reloc = pic_tls_gd;
	    tail += 6;
	  }
	else if (startswith (cp - 6, "@TLSIE"))
	  {
	    yylval.exp.pic_reloc = pic_tls_ie;
	    tail += 6;
	  }
	else if (startswith (cp - 6, "@TLSLE"))
	  {
	    yylval.exp.pic_reloc = pic_tls_le;
	    tail += 6;
	  }
      }
    else if (cp - 4 > str && cp[-4] == '@')
      {
	if (startswith (cp - 4, "@PLT"))
	  {
	    yylval.exp.pic_reloc = pic_plt_off;
	    tail += 4;
	  }
	else if (startswith (cp - 4, "@GOT"))
	  {
	    yylval.exp.pic_reloc = pic_got_off;
	    tail += 4;
	  }
      }
  }
#endif

  if (tail != 0)
    {
      c = s[-tail];
      s[-tail] = 0;
    }

  temp_ilp (str);
  expression (&yylval.exp.exp);
  str = input_line_pointer;
  restore_ilp ();

  if (tail != 0)
    {
      s[-tail] = c;
      str = s;
    }

  return EXPR;
}

/* Parse an m68k operand.  This is the only function which is called
   from outside this file.  */

int
m68k_ip_op (char *s, struct m68k_op *oparg)
{
  memset (oparg, 0, sizeof *oparg);
  oparg->error = NULL;
  oparg->index.reg = ZDATA0;
  oparg->index.scale = 1;
  oparg->disp.exp.X_op = O_absent;
  oparg->odisp.exp.X_op = O_absent;

  str = strorig = s;
  op = oparg;

  return yyparse ();
}

/* The error handler.  */

static void
yyerror (const char *s)
{
  op->error = s;
}
