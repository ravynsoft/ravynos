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
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 26 "yyscript.y"


#include "config.h"
#include "diagnostics.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "script-c.h"

DIAGNOSTIC_IGNORE_UNUSED_BUT_SET_VARIABLE


#line 87 "yyscript.c"

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
#ifndef YY_YY_YYSCRIPT_H_INCLUDED
# define YY_YY_YYSCRIPT_H_INCLUDED
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
    PLUSEQ = 258,                  /* PLUSEQ  */
    MINUSEQ = 259,                 /* MINUSEQ  */
    MULTEQ = 260,                  /* MULTEQ  */
    DIVEQ = 261,                   /* DIVEQ  */
    LSHIFTEQ = 262,                /* LSHIFTEQ  */
    RSHIFTEQ = 263,                /* RSHIFTEQ  */
    ANDEQ = 264,                   /* ANDEQ  */
    OREQ = 265,                    /* OREQ  */
    OROR = 266,                    /* OROR  */
    ANDAND = 267,                  /* ANDAND  */
    EQ = 268,                      /* EQ  */
    NE = 269,                      /* NE  */
    LE = 270,                      /* LE  */
    GE = 271,                      /* GE  */
    LSHIFT = 272,                  /* LSHIFT  */
    RSHIFT = 273,                  /* RSHIFT  */
    UNARY = 274,                   /* UNARY  */
    STRING = 275,                  /* STRING  */
    QUOTED_STRING = 276,           /* QUOTED_STRING  */
    INTEGER = 277,                 /* INTEGER  */
    ABSOLUTE = 278,                /* ABSOLUTE  */
    ADDR = 279,                    /* ADDR  */
    ALIGN_K = 280,                 /* ALIGN_K  */
    ALIGNOF = 281,                 /* ALIGNOF  */
    ASSERT_K = 282,                /* ASSERT_K  */
    AS_NEEDED = 283,               /* AS_NEEDED  */
    AT = 284,                      /* AT  */
    BIND = 285,                    /* BIND  */
    BLOCK = 286,                   /* BLOCK  */
    BYTE = 287,                    /* BYTE  */
    CONSTANT = 288,                /* CONSTANT  */
    CONSTRUCTORS = 289,            /* CONSTRUCTORS  */
    COPY = 290,                    /* COPY  */
    CREATE_OBJECT_SYMBOLS = 291,   /* CREATE_OBJECT_SYMBOLS  */
    DATA_SEGMENT_ALIGN = 292,      /* DATA_SEGMENT_ALIGN  */
    DATA_SEGMENT_END = 293,        /* DATA_SEGMENT_END  */
    DATA_SEGMENT_RELRO_END = 294,  /* DATA_SEGMENT_RELRO_END  */
    DEFINED = 295,                 /* DEFINED  */
    DSECT = 296,                   /* DSECT  */
    ENTRY = 297,                   /* ENTRY  */
    EXCLUDE_FILE = 298,            /* EXCLUDE_FILE  */
    EXTERN = 299,                  /* EXTERN  */
    FILL = 300,                    /* FILL  */
    FLOAT = 301,                   /* FLOAT  */
    FORCE_COMMON_ALLOCATION = 302, /* FORCE_COMMON_ALLOCATION  */
    GLOBAL = 303,                  /* GLOBAL  */
    GROUP = 304,                   /* GROUP  */
    HIDDEN = 305,                  /* HIDDEN  */
    HLL = 306,                     /* HLL  */
    INCLUDE = 307,                 /* INCLUDE  */
    INHIBIT_COMMON_ALLOCATION = 308, /* INHIBIT_COMMON_ALLOCATION  */
    INFO = 309,                    /* INFO  */
    INPUT = 310,                   /* INPUT  */
    KEEP = 311,                    /* KEEP  */
    LEN = 312,                     /* LEN  */
    LENGTH = 313,                  /* LENGTH  */
    LOADADDR = 314,                /* LOADADDR  */
    LOCAL = 315,                   /* LOCAL  */
    LONG = 316,                    /* LONG  */
    MAP = 317,                     /* MAP  */
    MAX_K = 318,                   /* MAX_K  */
    MEMORY = 319,                  /* MEMORY  */
    MIN_K = 320,                   /* MIN_K  */
    NEXT = 321,                    /* NEXT  */
    NOCROSSREFS = 322,             /* NOCROSSREFS  */
    NOFLOAT = 323,                 /* NOFLOAT  */
    NOLOAD = 324,                  /* NOLOAD  */
    ONLY_IF_RO = 325,              /* ONLY_IF_RO  */
    ONLY_IF_RW = 326,              /* ONLY_IF_RW  */
    ORG = 327,                     /* ORG  */
    ORIGIN = 328,                  /* ORIGIN  */
    OUTPUT = 329,                  /* OUTPUT  */
    OUTPUT_ARCH = 330,             /* OUTPUT_ARCH  */
    OUTPUT_FORMAT = 331,           /* OUTPUT_FORMAT  */
    OVERLAY = 332,                 /* OVERLAY  */
    PHDRS = 333,                   /* PHDRS  */
    PROVIDE = 334,                 /* PROVIDE  */
    PROVIDE_HIDDEN = 335,          /* PROVIDE_HIDDEN  */
    QUAD = 336,                    /* QUAD  */
    SEARCH_DIR = 337,              /* SEARCH_DIR  */
    SECTIONS = 338,                /* SECTIONS  */
    SEGMENT_START = 339,           /* SEGMENT_START  */
    SHORT = 340,                   /* SHORT  */
    SIZEOF = 341,                  /* SIZEOF  */
    SIZEOF_HEADERS = 342,          /* SIZEOF_HEADERS  */
    SORT_BY_ALIGNMENT = 343,       /* SORT_BY_ALIGNMENT  */
    SORT_BY_INIT_PRIORITY = 344,   /* SORT_BY_INIT_PRIORITY  */
    SORT_BY_NAME = 345,            /* SORT_BY_NAME  */
    SPECIAL = 346,                 /* SPECIAL  */
    SQUAD = 347,                   /* SQUAD  */
    STARTUP = 348,                 /* STARTUP  */
    SUBALIGN = 349,                /* SUBALIGN  */
    SYSLIB = 350,                  /* SYSLIB  */
    TARGET_K = 351,                /* TARGET_K  */
    TRUNCATE = 352,                /* TRUNCATE  */
    VERSIONK = 353,                /* VERSIONK  */
    OPTION = 354,                  /* OPTION  */
    PARSING_LINKER_SCRIPT = 355,   /* PARSING_LINKER_SCRIPT  */
    PARSING_VERSION_SCRIPT = 356,  /* PARSING_VERSION_SCRIPT  */
    PARSING_DEFSYM = 357,          /* PARSING_DEFSYM  */
    PARSING_DYNAMIC_LIST = 358,    /* PARSING_DYNAMIC_LIST  */
    PARSING_SECTIONS_BLOCK = 359,  /* PARSING_SECTIONS_BLOCK  */
    PARSING_SECTION_COMMANDS = 360, /* PARSING_SECTION_COMMANDS  */
    PARSING_MEMORY_DEF = 361       /* PARSING_MEMORY_DEF  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define PLUSEQ 258
#define MINUSEQ 259
#define MULTEQ 260
#define DIVEQ 261
#define LSHIFTEQ 262
#define RSHIFTEQ 263
#define ANDEQ 264
#define OREQ 265
#define OROR 266
#define ANDAND 267
#define EQ 268
#define NE 269
#define LE 270
#define GE 271
#define LSHIFT 272
#define RSHIFT 273
#define UNARY 274
#define STRING 275
#define QUOTED_STRING 276
#define INTEGER 277
#define ABSOLUTE 278
#define ADDR 279
#define ALIGN_K 280
#define ALIGNOF 281
#define ASSERT_K 282
#define AS_NEEDED 283
#define AT 284
#define BIND 285
#define BLOCK 286
#define BYTE 287
#define CONSTANT 288
#define CONSTRUCTORS 289
#define COPY 290
#define CREATE_OBJECT_SYMBOLS 291
#define DATA_SEGMENT_ALIGN 292
#define DATA_SEGMENT_END 293
#define DATA_SEGMENT_RELRO_END 294
#define DEFINED 295
#define DSECT 296
#define ENTRY 297
#define EXCLUDE_FILE 298
#define EXTERN 299
#define FILL 300
#define FLOAT 301
#define FORCE_COMMON_ALLOCATION 302
#define GLOBAL 303
#define GROUP 304
#define HIDDEN 305
#define HLL 306
#define INCLUDE 307
#define INHIBIT_COMMON_ALLOCATION 308
#define INFO 309
#define INPUT 310
#define KEEP 311
#define LEN 312
#define LENGTH 313
#define LOADADDR 314
#define LOCAL 315
#define LONG 316
#define MAP 317
#define MAX_K 318
#define MEMORY 319
#define MIN_K 320
#define NEXT 321
#define NOCROSSREFS 322
#define NOFLOAT 323
#define NOLOAD 324
#define ONLY_IF_RO 325
#define ONLY_IF_RW 326
#define ORG 327
#define ORIGIN 328
#define OUTPUT 329
#define OUTPUT_ARCH 330
#define OUTPUT_FORMAT 331
#define OVERLAY 332
#define PHDRS 333
#define PROVIDE 334
#define PROVIDE_HIDDEN 335
#define QUAD 336
#define SEARCH_DIR 337
#define SECTIONS 338
#define SEGMENT_START 339
#define SHORT 340
#define SIZEOF 341
#define SIZEOF_HEADERS 342
#define SORT_BY_ALIGNMENT 343
#define SORT_BY_INIT_PRIORITY 344
#define SORT_BY_NAME 345
#define SPECIAL 346
#define SQUAD 347
#define STARTUP 348
#define SUBALIGN 349
#define SYSLIB 350
#define TARGET_K 351
#define TRUNCATE 352
#define VERSIONK 353
#define OPTION 354
#define PARSING_LINKER_SCRIPT 355
#define PARSING_VERSION_SCRIPT 356
#define PARSING_DEFSYM 357
#define PARSING_DYNAMIC_LIST 358
#define PARSING_SECTIONS_BLOCK 359
#define PARSING_SECTION_COMMANDS 360
#define PARSING_MEMORY_DEF 361

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 56 "yyscript.y"

  /* A string.  */
  struct Parser_string string;
  /* A number.  */
  uint64_t integer;
  /* An expression.  */
  Expression_ptr expr;
  /* An output section header.  */
  struct Parser_output_section_header output_section_header;
  /* An output section trailer.  */
  struct Parser_output_section_trailer output_section_trailer;
  /* A section constraint.  */
  enum Section_constraint constraint;
  /* A complete input section specification.  */
  struct Input_section_spec input_section_spec;
  /* A list of wildcard specifications, with exclusions.  */
  struct Wildcard_sections wildcard_sections;
  /* A single wildcard specification.  */
  struct Wildcard_section wildcard_section;
  /* A list of strings.  */
  String_list_ptr string_list;
  /* Information for a program header.  */
  struct Phdr_info phdr_info;
  /* Used for version scripts and within VERSION {}.  */
  struct Version_dependency_list* deplist;
  struct Version_expression_list* versyms;
  struct Version_tree* versnode;
  enum Script_section_type section_type;

#line 382 "yyscript.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int yyparse (void* closure);


#endif /* !YY_YY_YYSCRIPT_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_PLUSEQ = 3,                     /* PLUSEQ  */
  YYSYMBOL_MINUSEQ = 4,                    /* MINUSEQ  */
  YYSYMBOL_MULTEQ = 5,                     /* MULTEQ  */
  YYSYMBOL_DIVEQ = 6,                      /* DIVEQ  */
  YYSYMBOL_7_ = 7,                         /* '='  */
  YYSYMBOL_LSHIFTEQ = 8,                   /* LSHIFTEQ  */
  YYSYMBOL_RSHIFTEQ = 9,                   /* RSHIFTEQ  */
  YYSYMBOL_ANDEQ = 10,                     /* ANDEQ  */
  YYSYMBOL_OREQ = 11,                      /* OREQ  */
  YYSYMBOL_12_ = 12,                       /* '?'  */
  YYSYMBOL_13_ = 13,                       /* ':'  */
  YYSYMBOL_OROR = 14,                      /* OROR  */
  YYSYMBOL_ANDAND = 15,                    /* ANDAND  */
  YYSYMBOL_16_ = 16,                       /* '|'  */
  YYSYMBOL_17_ = 17,                       /* '^'  */
  YYSYMBOL_18_ = 18,                       /* '&'  */
  YYSYMBOL_EQ = 19,                        /* EQ  */
  YYSYMBOL_NE = 20,                        /* NE  */
  YYSYMBOL_21_ = 21,                       /* '<'  */
  YYSYMBOL_22_ = 22,                       /* '>'  */
  YYSYMBOL_LE = 23,                        /* LE  */
  YYSYMBOL_GE = 24,                        /* GE  */
  YYSYMBOL_LSHIFT = 25,                    /* LSHIFT  */
  YYSYMBOL_RSHIFT = 26,                    /* RSHIFT  */
  YYSYMBOL_27_ = 27,                       /* '+'  */
  YYSYMBOL_28_ = 28,                       /* '-'  */
  YYSYMBOL_29_ = 29,                       /* '*'  */
  YYSYMBOL_30_ = 30,                       /* '/'  */
  YYSYMBOL_31_ = 31,                       /* '%'  */
  YYSYMBOL_UNARY = 32,                     /* UNARY  */
  YYSYMBOL_STRING = 33,                    /* STRING  */
  YYSYMBOL_QUOTED_STRING = 34,             /* QUOTED_STRING  */
  YYSYMBOL_INTEGER = 35,                   /* INTEGER  */
  YYSYMBOL_ABSOLUTE = 36,                  /* ABSOLUTE  */
  YYSYMBOL_ADDR = 37,                      /* ADDR  */
  YYSYMBOL_ALIGN_K = 38,                   /* ALIGN_K  */
  YYSYMBOL_ALIGNOF = 39,                   /* ALIGNOF  */
  YYSYMBOL_ASSERT_K = 40,                  /* ASSERT_K  */
  YYSYMBOL_AS_NEEDED = 41,                 /* AS_NEEDED  */
  YYSYMBOL_AT = 42,                        /* AT  */
  YYSYMBOL_BIND = 43,                      /* BIND  */
  YYSYMBOL_BLOCK = 44,                     /* BLOCK  */
  YYSYMBOL_BYTE = 45,                      /* BYTE  */
  YYSYMBOL_CONSTANT = 46,                  /* CONSTANT  */
  YYSYMBOL_CONSTRUCTORS = 47,              /* CONSTRUCTORS  */
  YYSYMBOL_COPY = 48,                      /* COPY  */
  YYSYMBOL_CREATE_OBJECT_SYMBOLS = 49,     /* CREATE_OBJECT_SYMBOLS  */
  YYSYMBOL_DATA_SEGMENT_ALIGN = 50,        /* DATA_SEGMENT_ALIGN  */
  YYSYMBOL_DATA_SEGMENT_END = 51,          /* DATA_SEGMENT_END  */
  YYSYMBOL_DATA_SEGMENT_RELRO_END = 52,    /* DATA_SEGMENT_RELRO_END  */
  YYSYMBOL_DEFINED = 53,                   /* DEFINED  */
  YYSYMBOL_DSECT = 54,                     /* DSECT  */
  YYSYMBOL_ENTRY = 55,                     /* ENTRY  */
  YYSYMBOL_EXCLUDE_FILE = 56,              /* EXCLUDE_FILE  */
  YYSYMBOL_EXTERN = 57,                    /* EXTERN  */
  YYSYMBOL_FILL = 58,                      /* FILL  */
  YYSYMBOL_FLOAT = 59,                     /* FLOAT  */
  YYSYMBOL_FORCE_COMMON_ALLOCATION = 60,   /* FORCE_COMMON_ALLOCATION  */
  YYSYMBOL_GLOBAL = 61,                    /* GLOBAL  */
  YYSYMBOL_GROUP = 62,                     /* GROUP  */
  YYSYMBOL_HIDDEN = 63,                    /* HIDDEN  */
  YYSYMBOL_HLL = 64,                       /* HLL  */
  YYSYMBOL_INCLUDE = 65,                   /* INCLUDE  */
  YYSYMBOL_INHIBIT_COMMON_ALLOCATION = 66, /* INHIBIT_COMMON_ALLOCATION  */
  YYSYMBOL_INFO = 67,                      /* INFO  */
  YYSYMBOL_INPUT = 68,                     /* INPUT  */
  YYSYMBOL_KEEP = 69,                      /* KEEP  */
  YYSYMBOL_LEN = 70,                       /* LEN  */
  YYSYMBOL_LENGTH = 71,                    /* LENGTH  */
  YYSYMBOL_LOADADDR = 72,                  /* LOADADDR  */
  YYSYMBOL_LOCAL = 73,                     /* LOCAL  */
  YYSYMBOL_LONG = 74,                      /* LONG  */
  YYSYMBOL_MAP = 75,                       /* MAP  */
  YYSYMBOL_MAX_K = 76,                     /* MAX_K  */
  YYSYMBOL_MEMORY = 77,                    /* MEMORY  */
  YYSYMBOL_MIN_K = 78,                     /* MIN_K  */
  YYSYMBOL_NEXT = 79,                      /* NEXT  */
  YYSYMBOL_NOCROSSREFS = 80,               /* NOCROSSREFS  */
  YYSYMBOL_NOFLOAT = 81,                   /* NOFLOAT  */
  YYSYMBOL_NOLOAD = 82,                    /* NOLOAD  */
  YYSYMBOL_ONLY_IF_RO = 83,                /* ONLY_IF_RO  */
  YYSYMBOL_ONLY_IF_RW = 84,                /* ONLY_IF_RW  */
  YYSYMBOL_ORG = 85,                       /* ORG  */
  YYSYMBOL_ORIGIN = 86,                    /* ORIGIN  */
  YYSYMBOL_OUTPUT = 87,                    /* OUTPUT  */
  YYSYMBOL_OUTPUT_ARCH = 88,               /* OUTPUT_ARCH  */
  YYSYMBOL_OUTPUT_FORMAT = 89,             /* OUTPUT_FORMAT  */
  YYSYMBOL_OVERLAY = 90,                   /* OVERLAY  */
  YYSYMBOL_PHDRS = 91,                     /* PHDRS  */
  YYSYMBOL_PROVIDE = 92,                   /* PROVIDE  */
  YYSYMBOL_PROVIDE_HIDDEN = 93,            /* PROVIDE_HIDDEN  */
  YYSYMBOL_QUAD = 94,                      /* QUAD  */
  YYSYMBOL_SEARCH_DIR = 95,                /* SEARCH_DIR  */
  YYSYMBOL_SECTIONS = 96,                  /* SECTIONS  */
  YYSYMBOL_SEGMENT_START = 97,             /* SEGMENT_START  */
  YYSYMBOL_SHORT = 98,                     /* SHORT  */
  YYSYMBOL_SIZEOF = 99,                    /* SIZEOF  */
  YYSYMBOL_SIZEOF_HEADERS = 100,           /* SIZEOF_HEADERS  */
  YYSYMBOL_SORT_BY_ALIGNMENT = 101,        /* SORT_BY_ALIGNMENT  */
  YYSYMBOL_SORT_BY_INIT_PRIORITY = 102,    /* SORT_BY_INIT_PRIORITY  */
  YYSYMBOL_SORT_BY_NAME = 103,             /* SORT_BY_NAME  */
  YYSYMBOL_SPECIAL = 104,                  /* SPECIAL  */
  YYSYMBOL_SQUAD = 105,                    /* SQUAD  */
  YYSYMBOL_STARTUP = 106,                  /* STARTUP  */
  YYSYMBOL_SUBALIGN = 107,                 /* SUBALIGN  */
  YYSYMBOL_SYSLIB = 108,                   /* SYSLIB  */
  YYSYMBOL_TARGET_K = 109,                 /* TARGET_K  */
  YYSYMBOL_TRUNCATE = 110,                 /* TRUNCATE  */
  YYSYMBOL_VERSIONK = 111,                 /* VERSIONK  */
  YYSYMBOL_OPTION = 112,                   /* OPTION  */
  YYSYMBOL_PARSING_LINKER_SCRIPT = 113,    /* PARSING_LINKER_SCRIPT  */
  YYSYMBOL_PARSING_VERSION_SCRIPT = 114,   /* PARSING_VERSION_SCRIPT  */
  YYSYMBOL_PARSING_DEFSYM = 115,           /* PARSING_DEFSYM  */
  YYSYMBOL_PARSING_DYNAMIC_LIST = 116,     /* PARSING_DYNAMIC_LIST  */
  YYSYMBOL_PARSING_SECTIONS_BLOCK = 117,   /* PARSING_SECTIONS_BLOCK  */
  YYSYMBOL_PARSING_SECTION_COMMANDS = 118, /* PARSING_SECTION_COMMANDS  */
  YYSYMBOL_PARSING_MEMORY_DEF = 119,       /* PARSING_MEMORY_DEF  */
  YYSYMBOL_120_ = 120,                     /* '('  */
  YYSYMBOL_121_ = 121,                     /* ')'  */
  YYSYMBOL_122_ = 122,                     /* '{'  */
  YYSYMBOL_123_ = 123,                     /* '}'  */
  YYSYMBOL_124_ = 124,                     /* ','  */
  YYSYMBOL_125_ = 125,                     /* ';'  */
  YYSYMBOL_126_ = 126,                     /* '!'  */
  YYSYMBOL_127_o_ = 127,                   /* 'o'  */
  YYSYMBOL_128_l_ = 128,                   /* 'l'  */
  YYSYMBOL_129_ = 129,                     /* '~'  */
  YYSYMBOL_YYACCEPT = 130,                 /* $accept  */
  YYSYMBOL_top = 131,                      /* top  */
  YYSYMBOL_linker_script = 132,            /* linker_script  */
  YYSYMBOL_file_cmd = 133,                 /* file_cmd  */
  YYSYMBOL_134_1 = 134,                    /* $@1  */
  YYSYMBOL_135_2 = 135,                    /* $@2  */
  YYSYMBOL_136_3 = 136,                    /* $@3  */
  YYSYMBOL_ignore_cmd = 137,               /* ignore_cmd  */
  YYSYMBOL_extern_name_list = 138,         /* extern_name_list  */
  YYSYMBOL_139_4 = 139,                    /* $@4  */
  YYSYMBOL_extern_name_list_body = 140,    /* extern_name_list_body  */
  YYSYMBOL_input_list = 141,               /* input_list  */
  YYSYMBOL_input_list_element = 142,       /* input_list_element  */
  YYSYMBOL_143_5 = 143,                    /* $@5  */
  YYSYMBOL_sections_block = 144,           /* sections_block  */
  YYSYMBOL_section_block_cmd = 145,        /* section_block_cmd  */
  YYSYMBOL_146_6 = 146,                    /* $@6  */
  YYSYMBOL_section_header = 147,           /* section_header  */
  YYSYMBOL_148_7 = 148,                    /* $@7  */
  YYSYMBOL_149_8 = 149,                    /* $@8  */
  YYSYMBOL_opt_address_and_section_type = 150, /* opt_address_and_section_type  */
  YYSYMBOL_section_type = 151,             /* section_type  */
  YYSYMBOL_opt_at = 152,                   /* opt_at  */
  YYSYMBOL_opt_align = 153,                /* opt_align  */
  YYSYMBOL_opt_subalign = 154,             /* opt_subalign  */
  YYSYMBOL_opt_constraint = 155,           /* opt_constraint  */
  YYSYMBOL_section_trailer = 156,          /* section_trailer  */
  YYSYMBOL_opt_memspec = 157,              /* opt_memspec  */
  YYSYMBOL_opt_at_memspec = 158,           /* opt_at_memspec  */
  YYSYMBOL_opt_phdr = 159,                 /* opt_phdr  */
  YYSYMBOL_opt_fill = 160,                 /* opt_fill  */
  YYSYMBOL_section_cmds = 161,             /* section_cmds  */
  YYSYMBOL_section_cmd = 162,              /* section_cmd  */
  YYSYMBOL_data_length = 163,              /* data_length  */
  YYSYMBOL_input_section_spec = 164,       /* input_section_spec  */
  YYSYMBOL_input_section_no_keep = 165,    /* input_section_no_keep  */
  YYSYMBOL_wildcard_file = 166,            /* wildcard_file  */
  YYSYMBOL_wildcard_sections = 167,        /* wildcard_sections  */
  YYSYMBOL_wildcard_section = 168,         /* wildcard_section  */
  YYSYMBOL_exclude_names = 169,            /* exclude_names  */
  YYSYMBOL_wildcard_name = 170,            /* wildcard_name  */
  YYSYMBOL_memory_defs = 171,              /* memory_defs  */
  YYSYMBOL_memory_def = 172,               /* memory_def  */
  YYSYMBOL_memory_attr = 173,              /* memory_attr  */
  YYSYMBOL_memory_origin = 174,            /* memory_origin  */
  YYSYMBOL_memory_length = 175,            /* memory_length  */
  YYSYMBOL_phdrs_defs = 176,               /* phdrs_defs  */
  YYSYMBOL_phdr_def = 177,                 /* phdr_def  */
  YYSYMBOL_phdr_type = 178,                /* phdr_type  */
  YYSYMBOL_phdr_info = 179,                /* phdr_info  */
  YYSYMBOL_assignment = 180,               /* assignment  */
  YYSYMBOL_parse_exp = 181,                /* parse_exp  */
  YYSYMBOL_182_9 = 182,                    /* $@9  */
  YYSYMBOL_exp = 183,                      /* exp  */
  YYSYMBOL_defsym_expr = 184,              /* defsym_expr  */
  YYSYMBOL_dynamic_list_expr = 185,        /* dynamic_list_expr  */
  YYSYMBOL_dynamic_list_nodes = 186,       /* dynamic_list_nodes  */
  YYSYMBOL_dynamic_list_node = 187,        /* dynamic_list_node  */
  YYSYMBOL_version_script = 188,           /* version_script  */
  YYSYMBOL_vers_nodes = 189,               /* vers_nodes  */
  YYSYMBOL_vers_node = 190,                /* vers_node  */
  YYSYMBOL_verdep = 191,                   /* verdep  */
  YYSYMBOL_vers_tag = 192,                 /* vers_tag  */
  YYSYMBOL_vers_defns = 193,               /* vers_defns  */
  YYSYMBOL_194_10 = 194,                   /* $@10  */
  YYSYMBOL_195_11 = 195,                   /* $@11  */
  YYSYMBOL_string = 196,                   /* string  */
  YYSYMBOL_end = 197,                      /* end  */
  YYSYMBOL_opt_semicolon = 198,            /* opt_semicolon  */
  YYSYMBOL_opt_comma = 199                 /* opt_comma  */
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
typedef yytype_int16 yy_state_t;

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

#if 1

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
#endif /* 1 */

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
#define YYFINAL  26
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1465

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  130
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  70
/* YYNRULES -- Number of rules.  */
#define YYNRULES  241
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  555

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   361


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   126,     2,     2,     2,    31,    18,     2,
     120,   121,    29,    27,   124,    28,     2,    30,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    13,   125,
      21,     7,    22,    12,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    17,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   128,     2,
       2,   127,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   122,    16,   123,   129,     2,     2,     2,
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
       5,     6,     8,     9,    10,    11,    14,    15,    19,    20,
      23,    24,    25,    26,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   239,   239,   240,   241,   242,   243,   244,   245,   250,
     251,   256,   257,   260,   259,   263,   265,   266,   267,   269,
     275,   282,   283,   286,   285,   289,   292,   291,   295,   297,
     298,   300,   303,   304,   312,   320,   320,   326,   328,   330,
     336,   337,   342,   344,   347,   346,   354,   355,   360,   362,
     363,   365,   369,   368,   377,   379,   377,   396,   401,   406,
     411,   416,   421,   430,   432,   437,   442,   447,   457,   458,
     465,   466,   473,   474,   481,   482,   484,   486,   492,   501,
     503,   508,   510,   515,   518,   524,   527,   532,   534,   540,
     541,   542,   544,   546,   548,   555,   556,   559,   565,   567,
     569,   571,   573,   580,   582,   588,   595,   604,   609,   618,
     623,   628,   633,   642,   647,   666,   685,   694,   696,   703,
     705,   710,   719,   720,   725,   728,   731,   736,   739,   742,
     746,   748,   750,   754,   756,   758,   763,   764,   769,   778,
     780,   787,   788,   796,   801,   812,   821,   823,   829,   835,
     841,   847,   853,   859,   865,   871,   873,   875,   881,   881,
     891,   893,   895,   897,   899,   901,   903,   905,   907,   909,
     911,   913,   915,   917,   919,   921,   923,   925,   927,   929,
     931,   933,   935,   937,   939,   941,   943,   945,   947,   949,
     951,   953,   955,   957,   959,   961,   963,   965,   967,   969,
     971,   973,   978,   983,   985,   993,   999,  1009,  1012,  1013,
    1017,  1023,  1027,  1028,  1032,  1036,  1041,  1048,  1052,  1060,
    1061,  1063,  1065,  1067,  1076,  1081,  1086,  1091,  1098,  1097,
    1108,  1107,  1114,  1119,  1129,  1131,  1138,  1139,  1144,  1145,
    1150,  1151
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "PLUSEQ", "MINUSEQ",
  "MULTEQ", "DIVEQ", "'='", "LSHIFTEQ", "RSHIFTEQ", "ANDEQ", "OREQ", "'?'",
  "':'", "OROR", "ANDAND", "'|'", "'^'", "'&'", "EQ", "NE", "'<'", "'>'",
  "LE", "GE", "LSHIFT", "RSHIFT", "'+'", "'-'", "'*'", "'/'", "'%'",
  "UNARY", "STRING", "QUOTED_STRING", "INTEGER", "ABSOLUTE", "ADDR",
  "ALIGN_K", "ALIGNOF", "ASSERT_K", "AS_NEEDED", "AT", "BIND", "BLOCK",
  "BYTE", "CONSTANT", "CONSTRUCTORS", "COPY", "CREATE_OBJECT_SYMBOLS",
  "DATA_SEGMENT_ALIGN", "DATA_SEGMENT_END", "DATA_SEGMENT_RELRO_END",
  "DEFINED", "DSECT", "ENTRY", "EXCLUDE_FILE", "EXTERN", "FILL", "FLOAT",
  "FORCE_COMMON_ALLOCATION", "GLOBAL", "GROUP", "HIDDEN", "HLL", "INCLUDE",
  "INHIBIT_COMMON_ALLOCATION", "INFO", "INPUT", "KEEP", "LEN", "LENGTH",
  "LOADADDR", "LOCAL", "LONG", "MAP", "MAX_K", "MEMORY", "MIN_K", "NEXT",
  "NOCROSSREFS", "NOFLOAT", "NOLOAD", "ONLY_IF_RO", "ONLY_IF_RW", "ORG",
  "ORIGIN", "OUTPUT", "OUTPUT_ARCH", "OUTPUT_FORMAT", "OVERLAY", "PHDRS",
  "PROVIDE", "PROVIDE_HIDDEN", "QUAD", "SEARCH_DIR", "SECTIONS",
  "SEGMENT_START", "SHORT", "SIZEOF", "SIZEOF_HEADERS",
  "SORT_BY_ALIGNMENT", "SORT_BY_INIT_PRIORITY", "SORT_BY_NAME", "SPECIAL",
  "SQUAD", "STARTUP", "SUBALIGN", "SYSLIB", "TARGET_K", "TRUNCATE",
  "VERSIONK", "OPTION", "PARSING_LINKER_SCRIPT", "PARSING_VERSION_SCRIPT",
  "PARSING_DEFSYM", "PARSING_DYNAMIC_LIST", "PARSING_SECTIONS_BLOCK",
  "PARSING_SECTION_COMMANDS", "PARSING_MEMORY_DEF", "'('", "')'", "'{'",
  "'}'", "','", "';'", "'!'", "'o'", "'l'", "'~'", "$accept", "top",
  "linker_script", "file_cmd", "$@1", "$@2", "$@3", "ignore_cmd",
  "extern_name_list", "$@4", "extern_name_list_body", "input_list",
  "input_list_element", "$@5", "sections_block", "section_block_cmd",
  "$@6", "section_header", "$@7", "$@8", "opt_address_and_section_type",
  "section_type", "opt_at", "opt_align", "opt_subalign", "opt_constraint",
  "section_trailer", "opt_memspec", "opt_at_memspec", "opt_phdr",
  "opt_fill", "section_cmds", "section_cmd", "data_length",
  "input_section_spec", "input_section_no_keep", "wildcard_file",
  "wildcard_sections", "wildcard_section", "exclude_names",
  "wildcard_name", "memory_defs", "memory_def", "memory_attr",
  "memory_origin", "memory_length", "phdrs_defs", "phdr_def", "phdr_type",
  "phdr_info", "assignment", "parse_exp", "$@9", "exp", "defsym_expr",
  "dynamic_list_expr", "dynamic_list_nodes", "dynamic_list_node",
  "version_script", "vers_nodes", "vers_node", "verdep", "vers_tag",
  "vers_defns", "$@10", "$@11", "string", "end", "opt_semicolon",
  "opt_comma", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-423)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-120)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     365,  -423,    81,   214,  -112,  -423,  -423,  -423,    14,  1300,
    -423,  -423,    33,  -423,    81,  -423,   -93,  -423,    52,   103,
    -423,  -112,  -423,   177,   568,     9,  -423,   -37,   -28,    -1,
    -423,  -423,    15,   214,  -423,    19,   -69,    60,    62,     1,
      64,    80,    82,    73,    84,    90,    86,  -423,  -423,  -423,
     136,   318,  -423,  -423,   214,   208,   217,   104,   108,  -423,
      33,  -423,   113,  -423,   132,   155,   214,  -423,   136,   318,
    -423,  -423,   157,  -423,  -423,   160,   214,   162,  -423,  -423,
    -423,   168,  -423,  -423,  -423,   169,  -423,  -423,   174,  -423,
     136,    38,  -423,    92,  -423,   214,  -423,   195,   214,  -423,
     231,  -423,   214,   214,  -423,   214,   214,   214,  -423,   214,
    -423,   214,  -423,  -423,  -423,  -423,  -423,  -423,  -423,  -423,
    -423,  -423,  -423,  -423,   151,   103,   103,   192,   159,   156,
    -423,  1253,    21,  -423,   214,  -423,  -423,  -423,   137,  -423,
    -423,  -423,    39,   422,  -423,    46,  -423,   214,  -423,   199,
     207,   215,   220,   214,   231,   341,   316,  -423,   -40,  -423,
    -423,   144,   229,     3,    36,   345,   346,   264,  -423,   266,
      81,   267,  -423,  -423,  -423,  -423,  -423,  -423,  -423,  -423,
    -423,  -423,   269,   270,  -423,  -423,  -423,   214,   -17,  1253,
    1253,  -423,   271,   272,   276,   279,   280,   305,   306,   309,
     310,   312,   322,   323,   324,   330,   332,   333,   337,   339,
    -423,  1253,  1253,  1253,  1434,  -423,   335,   265,   340,   343,
    -423,  1156,   420,  1151,   350,   349,   355,   364,   356,   374,
     375,  -423,   376,   397,   398,   399,   400,     7,  -423,  -423,
    -423,   -10,   508,   214,  -423,  -423,    -6,  -423,    70,  -423,
    -423,   402,  -423,   231,  -423,  -423,  -423,   214,  -423,  -423,
     300,  -423,  -423,  -423,   191,  -423,   401,  -423,   103,    96,
     159,   403,  -423,    -8,  -423,  -423,  -423,  1253,   214,  1253,
     214,  1253,  1253,   214,  1253,  1253,  1253,   214,   214,   214,
    1253,  1253,   214,   214,   214,   281,  -423,  -423,  1253,  1253,
    1253,  1253,  1253,  1253,  1253,  1253,  1253,  1253,  1253,  1253,
    1253,  1253,  1253,  1253,  1253,  1253,  1253,  -423,   214,  -423,
    -423,  -423,  -423,  -423,  -423,  -423,   510,   406,   408,   491,
    -423,   153,   214,  -423,   216,  -423,  -423,  -423,  -423,   216,
      53,   216,    53,  -423,   412,   214,   410,   -67,   413,   214,
    -423,  -423,   416,   231,  -423,   415,  -423,    29,  -423,   421,
     426,  -423,  -423,   405,   535,  -423,  -423,  -423,   825,   430,
     342,   431,   392,   862,   432,   714,   882,   751,   433,   434,
     435,   771,   791,   455,   454,   458,  -423,  1414,   731,   841,
     948,   481,   562,   353,   353,   463,   463,   463,   463,   409,
     409,   315,   315,  -423,  -423,  -423,   474,   565,  -423,   583,
    1253,   480,   496,   591,   485,   486,    75,  -423,   488,   490,
     493,   497,  -423,   495,  -423,  -423,  -423,  -423,   611,  -423,
    -423,  -423,    98,   214,   499,    29,   500,    43,  -423,  -423,
     159,   498,   103,   103,  -423,  -423,  -423,  1253,  -423,   214,
    -423,  -423,  1253,  -423,  1253,  -423,  -423,  -423,  1253,  1253,
    -423,  1253,  -423,  1253,  -423,   598,  -423,   902,  1253,   502,
    -423,  -423,   614,  -423,  -423,   216,  -423,  -423,  -423,   216,
    -423,  -423,  -423,   503,  -423,  -423,  -423,   594,  -423,  -423,
     507,   405,   933,   514,   970,   990,  1010,  1041,  1078,  1434,
     214,  -423,   596,  -423,  1098,  1253,   114,  -423,  -423,   105,
     512,  -423,   519,   520,   159,   521,  -423,  -423,  -423,  -423,
    -423,  -423,  -423,  -423,   621,  -423,  -423,  1118,  -423,  -423,
    -423,  -423,  -423,    18,    29,    29,  -423,   214,   154,  -423,
    -423,  -423,  -423,   638,  -423,  -423,  -423,  -423,   214,   512,
    -423,  -423,  -423,  -423,  -423
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    10,     0,     0,     0,    47,    87,   123,     0,     2,
     234,   235,   219,     3,   211,   212,     0,     4,     0,     0,
       5,   207,   208,     6,     7,   241,     1,     0,     0,     0,
      12,    13,     0,     0,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    33,     9,    32,
       0,     0,   224,   225,   232,     0,     0,     0,     0,   213,
     219,   158,     0,   209,     0,     0,     0,    46,     0,    54,
     121,   120,     0,   102,    94,     0,     0,     0,   100,    98,
     101,     0,    99,    97,    88,     0,    90,   103,     0,   107,
       0,   105,   240,   126,   158,     0,    35,     0,     0,    31,
       0,   123,     0,     0,   137,     0,     0,     0,    23,     0,
      26,     0,   237,   236,    29,   158,   158,   158,   158,   158,
     158,   158,   158,   158,     0,     0,     0,     0,   220,     0,
     206,     0,     0,   158,     0,    51,    49,    52,     0,   158,
     158,    96,     0,     0,   158,     0,    89,     0,   122,   129,
       0,     0,     0,     0,     0,     0,     0,    44,   241,    40,
      42,   241,     0,     0,     0,     0,     0,     0,    47,     0,
       0,     0,   147,   148,   149,   150,   146,   151,   152,   153,
     154,   228,     0,     0,   214,   226,   227,   233,     0,     0,
       0,   184,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     189,     0,     0,     0,   159,   185,     0,     0,     0,     0,
      57,     0,    68,     0,     0,     0,     0,     0,   105,     0,
       0,   119,     0,     0,     0,     0,     0,   241,   110,   113,
     125,     0,     0,     0,    28,    11,    36,    37,   241,   158,
      43,     0,    16,     0,    17,    34,    19,     0,    21,   136,
       0,   158,   158,    22,     0,    25,     0,    18,     0,   221,
     222,     0,   215,     0,   217,   164,   161,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   162,   163,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   210,     0,    48,
      87,    65,    64,    66,    63,    67,     0,     0,     0,    70,
      59,     0,     0,    93,     0,   104,    95,   108,    91,     0,
       0,     0,     0,   106,     0,     0,     0,     0,     0,     0,
      38,    14,     0,     0,    41,     0,   140,   141,   139,     0,
       0,    24,    27,   239,     0,   230,   216,   218,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   160,     0,   182,   181,
     180,   179,   178,   172,   173,   176,   177,   174,   175,   170,
     171,   168,   169,   165,   166,   167,     0,     0,    58,     0,
       0,     0,    72,     0,     0,     0,   241,   118,     0,     0,
       0,     0,   109,     0,   127,   131,   130,   132,     0,    30,
      39,   155,   241,     0,     0,   141,     0,   141,   156,   157,
     238,     0,     0,     0,   197,   192,   198,     0,   190,     0,
     200,   196,     0,   203,     0,   188,   195,   193,     0,     0,
     194,     0,   191,     0,    50,    80,    61,     0,     0,     0,
      55,    60,     0,    92,   112,     0,   115,   116,   114,     0,
     128,   158,    45,     0,   158,   143,   138,     0,   142,   229,
       0,   239,     0,     0,     0,     0,     0,     0,     0,   183,
       0,    53,    82,    69,     0,     0,    74,    62,   117,   241,
     241,    20,     0,     0,   223,     0,   199,   205,   201,   202,
     186,   187,   204,    79,     0,    84,    71,     0,    75,    76,
      77,    56,   111,     0,   141,   141,   231,     0,    86,    73,
     134,   133,   135,     0,   145,   144,    81,   158,     0,   241,
     158,    85,    83,    78,   124
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -423,  -423,  -423,  -423,  -423,  -423,  -423,  -423,  -423,  -423,
    -423,  -148,   393,  -423,   479,  -423,  -423,  -423,  -423,  -423,
    -423,   317,  -423,  -423,  -423,  -423,  -423,  -423,  -423,  -423,
    -423,   329,  -423,  -423,  -423,   509,  -423,  -423,  -245,   171,
     -21,   551,  -423,  -423,  -423,  -423,  -423,  -423,  -423,  -422,
      -4,   -83,  -423,   259,  -423,  -423,  -423,   632,   484,  -423,
     641,  -423,   604,   -15,  -423,  -423,    -2,   -60,   165,   -23
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     8,     9,    48,    97,   168,   170,    49,   152,   153,
     246,   158,   159,   251,    23,    67,   219,   137,   138,   506,
     222,   327,   329,   412,   470,   531,   501,   502,   525,   538,
     549,    24,    84,    85,    86,    87,    88,   237,   238,   416,
     239,    25,   148,   242,   428,   543,   164,   259,   357,   436,
      68,   130,   131,   295,    17,    20,    21,    22,    13,    14,
      15,   273,    57,    58,   268,   443,   215,   114,   441,   253
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      16,    18,    93,    89,    62,    50,   248,    51,   136,    -8,
      19,   150,    16,   485,    26,   488,    10,    11,   425,   426,
      90,    69,    91,    10,    11,    10,    11,    10,    11,    60,
     146,    99,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     217,    70,   124,   101,   185,   186,   224,   225,    70,    61,
     427,   232,    10,    11,   135,    70,    52,    53,    71,    10,
      11,   434,    10,    11,   141,    71,    10,    11,   187,    10,
      11,   252,    71,    94,    92,   434,    10,    11,   540,   541,
      54,   149,    95,   151,    55,   418,   155,   420,   160,   422,
     162,   163,   233,   165,   166,   167,    56,   169,   272,   171,
     182,   183,   544,   545,    10,    11,   345,   366,   349,    96,
     435,    89,   230,   104,   256,    10,    11,   257,   343,   185,
     186,    92,   218,    92,   435,    98,    52,    53,    93,   100,
     228,   231,   226,   231,   216,   240,   542,   234,   235,   236,
     220,   247,   160,   187,   234,   235,   236,   147,  -119,   258,
      54,   547,   260,   487,   189,   190,   352,   548,    16,   364,
      10,    11,   191,   192,   193,   194,   195,   196,   359,   360,
     102,   197,   103,   198,   105,   271,   274,   199,   200,   201,
     202,   351,   185,   186,    92,   108,   474,   528,   529,    92,
     106,   321,   107,    12,   109,   432,   111,   322,   203,   204,
      10,    11,   110,   205,   344,   206,   187,    64,   530,   482,
     323,   125,    92,   207,    10,    11,   532,   127,    70,    92,
     126,    64,    65,   128,   208,   324,   209,   210,   132,   346,
      32,   348,    66,   325,   350,    71,    65,    10,    11,    10,
      11,   160,   133,   363,    32,   355,    66,   221,   358,   156,
     112,   113,    69,   212,    10,    11,   213,   254,    92,    40,
      41,   367,   157,   181,   413,   134,   369,   139,   371,   188,
     140,   374,   142,    40,    41,   378,   379,   380,   143,   144,
     383,   384,   385,   298,   145,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   230,   361,   154,   406,   184,   417,   241,
     419,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     415,   243,   231,    10,    11,   356,   244,   231,   231,   231,
     231,   245,   231,   423,   314,   315,   316,   430,   249,   250,
     255,   160,   261,   262,   298,   437,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   263,    89,   265,   267,   318,
     214,   277,   278,   475,   269,   270,   279,   223,   510,   280,
     281,   512,   386,    90,   298,    91,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,    70,   282,   283,   490,   491,   284,
     285,   483,   286,   437,    70,   437,   312,   313,   314,   315,
     316,    71,   287,   288,   289,    10,    11,   493,   275,   276,
     290,    71,   291,   292,   508,    10,    11,   293,   417,   294,
     317,   319,   328,   446,   551,   320,   447,   554,   421,   229,
     333,   296,   297,   231,   332,   334,  -119,   231,     1,     2,
       3,     4,     5,     6,     7,   335,   475,   533,   310,   311,
     312,   313,   314,   315,   316,   336,   337,   338,   523,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   234,   235,   236,   449,   339,   340,   341,
     342,   347,   353,   408,   362,   365,   553,   409,   410,   411,
     440,   424,   437,   437,   429,   546,   368,   431,   370,   433,
     372,   373,   438,   375,   376,   377,   552,   439,   442,   381,
     382,   445,   448,   451,   455,   456,   457,   387,   388,   389,
     390,   391,   392,   393,   394,   395,   396,   397,   398,   399,
     400,   401,   402,   403,   404,   405,   460,    70,   461,   462,
      70,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,    71,   464,   466,    71,    10,    11,
     468,    10,    11,   469,   471,    72,   472,   473,    72,   476,
      73,   477,    74,    73,   478,    74,   480,   479,   481,   484,
     500,   489,   505,    75,   511,   486,    75,   507,    32,   513,
      76,    32,   514,    76,    77,   517,    92,    77,   524,    78,
     534,   535,    78,   537,   536,   550,   354,   264,   414,   407,
     509,   227,   161,    63,   266,    59,   515,    40,    41,    79,
      40,    41,    79,    80,   129,     0,    80,     0,    81,   467,
      82,    81,     0,    82,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   465,     0,
      83,     0,     0,    83,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   492,     0,     0,     0,
       0,   494,     0,   495,     0,     0,     0,   496,   497,     0,
     498,     0,   499,     0,     0,     0,   298,   504,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   298,   527,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   298,     0,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   298,     0,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   298,   452,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,     0,   298,   454,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   298,   458,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   298,   459,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   298,   444,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
       0,     0,   298,   450,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   298,   453,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   298,   503,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   298,   516,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     298,   518,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     298,   519,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     298,   520,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   521,   298,   330,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   189,   190,     0,     0,     0,     0,    10,
      11,   191,   192,   193,   194,   195,   196,     0,     0,   522,
     197,     0,   198,     0,   321,     0,   199,   200,   201,   202,
     322,     0,     0,     0,     0,     0,     0,     0,     0,   526,
       0,     0,     0,   323,     0,     0,     0,   203,   204,     0,
       0,     0,   205,     0,   206,     0,     0,     0,   324,   539,
       0,     0,   207,     0,     0,     0,   325,     0,     0,     0,
       0,     0,     0,   208,     0,   209,   210,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   331,     0,     0,     0,     0,   211,   326,     0,     0,
     189,   190,   212,     0,     0,   213,    10,    11,   191,   192,
     193,   194,   195,   196,     0,     0,     0,   197,     0,   198,
       0,     0,     0,   199,   200,   201,   202,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   203,   204,     0,     0,     0,   205,
       0,   206,     0,    10,    11,     0,     0,     0,     0,   207,
      27,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     208,     0,   209,   210,     0,    28,     0,    29,     0,     0,
      30,     0,    31,    32,     0,    33,    34,     0,    35,     0,
       0,     0,     0,   211,     0,     0,     0,    36,     0,   212,
       0,     0,   213,     0,     0,     0,     0,     0,    37,    38,
       0,    39,    40,    41,     0,    42,    43,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    44,
       0,    45,    46,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    47,   298,   463,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   298,     0,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316
};

static const yytype_int16 yycheck[] =
{
       2,     3,    25,    24,    19,     9,   154,     9,    68,     0,
     122,    94,    14,   435,     0,   437,    33,    34,    85,    86,
      24,    23,    24,    33,    34,    33,    34,    33,    34,   122,
      90,    33,   115,   116,   117,   118,   119,   120,   121,   122,
     123,     3,     4,     5,     6,     7,     8,     9,    10,    11,
     133,    12,    54,   122,    33,    34,   139,   140,    12,     7,
     127,   144,    33,    34,    66,    12,    33,    34,    29,    33,
      34,    42,    33,    34,    76,    29,    33,    34,    57,    33,
      34,   121,    29,   120,   124,    42,    33,    34,    70,    71,
      57,    93,   120,    95,    61,   340,    98,   342,   100,   344,
     102,   103,    56,   105,   106,   107,    73,   109,   125,   111,
     125,   126,   534,   535,    33,    34,   126,   125,   124,   120,
      91,   142,   143,   122,   121,    33,    34,   124,   121,    33,
      34,   124,   134,   124,    91,   120,    33,    34,   161,   120,
     142,   143,   103,   145,   123,   147,   128,   101,   102,   103,
      13,   153,   154,    57,   101,   102,   103,    65,   120,   123,
      57,     7,   164,   120,    27,    28,   249,    13,   170,    73,
      33,    34,    35,    36,    37,    38,    39,    40,   261,   262,
     120,    44,   120,    46,   120,   187,   188,    50,    51,    52,
      53,   121,    33,    34,   124,   122,   121,    83,    84,   124,
     120,    48,   120,   122,   120,   353,   120,    54,    71,    72,
      33,    34,   122,    76,   237,    78,    57,    40,   104,   121,
      67,    13,   124,    86,    33,    34,   121,   123,    12,   124,
      13,    40,    55,   125,    97,    82,    99,   100,   125,   241,
      63,   243,    65,    90,   246,    29,    55,    33,    34,    33,
      34,   253,   120,   268,    63,   257,    65,   120,   260,    28,
     124,   125,   264,   126,    33,    34,   129,   123,   124,    92,
      93,   273,    41,   122,   121,   120,   278,   120,   280,   123,
     120,   283,   120,    92,    93,   287,   288,   289,   120,   120,
     292,   293,   294,    12,   120,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,   334,   123,   120,   318,   125,   339,   120,
     341,     3,     4,     5,     6,     7,     8,     9,    10,    11,
     332,   124,   334,    33,    34,    35,   121,   339,   340,   341,
     342,   121,   344,   345,    29,    30,    31,   349,     7,    33,
     121,   353,     7,     7,    12,   357,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,   121,   407,   121,   121,   124,
     131,   120,   120,   416,   125,   125,   120,   138,   481,   120,
     120,   484,   121,   407,    12,   407,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    12,   120,   120,   442,   443,   120,
     120,   433,   120,   435,    12,   437,    27,    28,    29,    30,
      31,    29,   120,   120,   120,    33,    34,   449,   189,   190,
     120,    29,   120,   120,   475,    33,    34,   120,   479,   120,
     125,   121,    42,   121,   547,   122,   124,   550,    56,    47,
     121,   212,   213,   475,   124,   120,   120,   479,   113,   114,
     115,   116,   117,   118,   119,   121,   509,   510,    25,    26,
      27,    28,    29,    30,    31,   121,   121,   121,   500,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,   101,   102,   103,   124,   120,   120,   120,
     120,    13,   120,    13,   123,   122,   549,   121,   120,    38,
     125,   121,   534,   535,   121,   537,   277,   121,   279,   124,
     281,   282,   121,   284,   285,   286,   548,   121,    13,   290,
     291,   121,   121,   121,   121,   121,   121,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   121,    12,   124,   121,
      12,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    29,   121,    13,    29,    33,    34,
     120,    33,    34,   107,    13,    40,   121,   121,    40,   121,
      45,   121,    47,    45,   121,    47,   121,   120,     7,   120,
      22,   123,   120,    58,   121,   125,    58,    13,    63,    35,
      65,    63,   125,    65,    69,   121,   124,    69,    42,    74,
     121,   121,    74,    22,   123,     7,   253,   168,   331,   320,
     479,   142,   101,    21,   170,    14,   491,    92,    93,    94,
      92,    93,    94,    98,    60,    -1,    98,    -1,   103,   410,
     105,   103,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,    -1,
     125,    -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   447,    -1,    -1,    -1,
      -1,   452,    -1,   454,    -1,    -1,    -1,   458,   459,    -1,
     461,    -1,   463,    -1,    -1,    -1,    12,   468,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    12,   505,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    12,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    12,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    12,   124,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    12,   124,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    12,   124,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    12,   124,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    12,   121,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    -1,    12,   121,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    12,   121,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    12,   121,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    12,   121,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      12,   121,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      12,   121,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      12,   121,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   121,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    27,    28,    -1,    -1,    -1,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    -1,    -1,   121,
      44,    -1,    46,    -1,    48,    -1,    50,    51,    52,    53,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    67,    -1,    -1,    -1,    71,    72,    -1,
      -1,    -1,    76,    -1,    78,    -1,    -1,    -1,    82,   121,
      -1,    -1,    86,    -1,    -1,    -1,    90,    -1,    -1,    -1,
      -1,    -1,    -1,    97,    -1,    99,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   120,    -1,    -1,    -1,    -1,   120,   121,    -1,    -1,
      27,    28,   126,    -1,    -1,   129,    33,    34,    35,    36,
      37,    38,    39,    40,    -1,    -1,    -1,    44,    -1,    46,
      -1,    -1,    -1,    50,    51,    52,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    72,    -1,    -1,    -1,    76,
      -1,    78,    -1,    33,    34,    -1,    -1,    -1,    -1,    86,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      97,    -1,    99,   100,    -1,    55,    -1,    57,    -1,    -1,
      60,    -1,    62,    63,    -1,    65,    66,    -1,    68,    -1,
      -1,    -1,    -1,   120,    -1,    -1,    -1,    77,    -1,   126,
      -1,    -1,   129,    -1,    -1,    -1,    -1,    -1,    88,    89,
      -1,    91,    92,    93,    -1,    95,    96,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,
      -1,   111,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    12,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   113,   114,   115,   116,   117,   118,   119,   131,   132,
      33,    34,   122,   188,   189,   190,   196,   184,   196,   122,
     185,   186,   187,   144,   161,   171,     0,    40,    55,    57,
      60,    62,    63,    65,    66,    68,    77,    88,    89,    91,
      92,    93,    95,    96,   109,   111,   112,   125,   133,   137,
     180,   196,    33,    34,    57,    61,    73,   192,   193,   190,
     122,     7,   193,   187,    40,    55,    65,   145,   180,   196,
      12,    29,    40,    45,    47,    58,    65,    69,    74,    94,
      98,   103,   105,   125,   162,   163,   164,   165,   166,   170,
     180,   196,   124,   199,   120,   120,   120,   134,   120,   196,
     120,   122,   120,   120,   122,   120,   120,   120,   122,   120,
     122,   120,   124,   125,   197,     3,     4,     5,     6,     7,
       8,     9,    10,    11,   196,    13,    13,   123,   125,   192,
     181,   182,   125,   120,   120,   196,   197,   147,   148,   120,
     120,   196,   120,   120,   120,   120,   197,    65,   172,   196,
     181,   196,   138,   139,   120,   196,    28,    41,   141,   142,
     196,   171,   196,   196,   176,   196,   196,   196,   135,   196,
     136,   196,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   122,   193,   193,   125,    33,    34,    57,   123,    27,
      28,    35,    36,    37,    38,    39,    40,    44,    46,    50,
      51,    52,    53,    71,    72,    76,    78,    86,    97,    99,
     100,   120,   126,   129,   183,   196,   123,   181,   196,   146,
      13,   120,   150,   183,   181,   181,   103,   165,   196,    47,
     170,   196,   181,    56,   101,   102,   103,   167,   168,   170,
     196,   120,   173,   124,   121,   121,   140,   196,   141,     7,
      33,   143,   121,   199,   123,   121,   121,   124,   123,   177,
     196,     7,     7,   121,   144,   121,   188,   121,   194,   125,
     125,   196,   125,   191,   196,   183,   183,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   183,   183,   183,    12,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,   125,   124,   121,
     122,    48,    54,    67,    82,    90,   121,   151,    42,   152,
      13,   120,   124,   121,   120,   121,   121,   121,   121,   120,
     120,   120,   120,   121,   199,   126,   196,    13,   196,   124,
     196,   121,   181,   120,   142,   196,    35,   178,   196,   181,
     181,   123,   123,   193,    73,   122,   125,   196,   183,   196,
     183,   196,   183,   183,   196,   183,   183,   183,   196,   196,
     196,   183,   183,   196,   196,   196,   121,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   196,   161,    13,   121,
     120,    38,   153,   121,   151,   196,   169,   170,   168,   170,
     168,    56,   168,   196,   121,    85,    86,   127,   174,   121,
     196,   121,   141,   124,    42,    91,   179,   196,   121,   121,
     125,   198,    13,   195,   121,   121,   121,   124,   121,   124,
     121,   121,   124,   121,   124,   121,   121,   121,   124,   124,
     121,   124,   121,    13,   121,   123,    13,   183,   120,   107,
     154,    13,   121,   121,   121,   199,   121,   121,   121,   120,
     121,     7,   121,   196,   120,   179,   125,   120,   179,   123,
     193,   193,   183,   196,   183,   183,   183,   183,   183,   183,
      22,   156,   157,   121,   183,   120,   149,    13,   170,   169,
     181,   121,   181,    35,   125,   198,   121,   121,   121,   121,
     121,   121,   121,   196,    42,   158,   121,   183,    83,    84,
     104,   155,   121,   199,   121,   121,   123,    22,   159,   121,
      70,    71,   128,   175,   179,   179,   196,     7,    13,   160,
       7,   181,   196,   199,   181
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   130,   131,   131,   131,   131,   131,   131,   131,   132,
     132,   133,   133,   134,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   135,   133,   133,   136,   133,   133,   133,
     133,   133,   133,   133,   137,   139,   138,   140,   140,   140,
     141,   141,   142,   142,   143,   142,   144,   144,   145,   145,
     145,   145,   146,   145,   148,   149,   147,   150,   150,   150,
     150,   150,   150,   151,   151,   151,   151,   151,   152,   152,
     153,   153,   154,   154,   155,   155,   155,   155,   156,   157,
     157,   158,   158,   159,   159,   160,   160,   161,   161,   162,
     162,   162,   162,   162,   162,   162,   162,   162,   163,   163,
     163,   163,   163,   164,   164,   165,   165,   166,   166,   167,
     167,   167,   167,   168,   168,   168,   168,   169,   169,   170,
     170,   170,   171,   171,   172,   172,   172,   173,   173,   173,
     174,   174,   174,   175,   175,   175,   176,   176,   177,   178,
     178,   179,   179,   179,   179,   179,   180,   180,   180,   180,
     180,   180,   180,   180,   180,   180,   180,   180,   182,   181,
     183,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   183,   184,   185,   186,   186,
     187,   188,   189,   189,   190,   190,   190,   191,   191,   192,
     192,   192,   192,   192,   193,   193,   193,   193,   194,   193,
     195,   193,   193,   193,   196,   196,   197,   197,   198,   198,
     199,   199
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       0,     4,     1,     0,     5,     1,     4,     4,     4,     4,
       8,     4,     4,     0,     5,     4,     0,     5,     4,     2,
       6,     2,     1,     1,     4,     0,     2,     1,     2,     3,
       1,     3,     1,     2,     0,     5,     2,     0,     4,     2,
       6,     2,     0,     7,     0,     0,     7,     1,     3,     2,
       4,     4,     5,     1,     1,     1,     1,     1,     0,     4,
       0,     4,     0,     4,     0,     1,     1,     1,     5,     2,
       0,     3,     0,     3,     0,     2,     0,     0,     2,     2,
       1,     4,     6,     4,     1,     4,     2,     1,     1,     1,
       1,     1,     1,     1,     4,     1,     4,     1,     4,     3,
       1,     6,     4,     1,     4,     4,     4,     3,     1,     1,
       1,     1,     3,     0,    10,     2,     0,     3,     4,     0,
       1,     1,     1,     1,     1,     1,     2,     0,     4,     1,
       1,     0,     2,     2,     5,     5,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     6,     6,     6,     0,     2,
       3,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     1,     1,     6,     6,     4,     1,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     6,
       4,     6,     6,     4,     6,     6,     3,     1,     1,     2,
       5,     1,     1,     2,     4,     5,     6,     1,     2,     0,
       2,     4,     4,     8,     1,     1,     3,     3,     0,     7,
       0,     9,     1,     3,     1,     1,     1,     1,     1,     0,
       1,     0
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
        yyerror (closure, YY_("syntax error: cannot back up")); \
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
                  Kind, Value, closure); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void* closure)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (closure);
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
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void* closure)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, closure);
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
                 int yyrule, void* closure)
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
                       &yyvsp[(yyi + 1) - (yynrhs)], closure);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, closure); \
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


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, void* closure)
{
  YY_USE (yyvaluep);
  YY_USE (closure);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void* closure)
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

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

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
      yychar = yylex (&yylval, closure);
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
  case 12: /* file_cmd: FORCE_COMMON_ALLOCATION  */
#line 258 "yyscript.y"
            { script_set_common_allocation(closure, 1); }
#line 2411 "yyscript.c"
    break;

  case 13: /* $@1: %empty  */
#line 260 "yyscript.y"
            { script_start_group(closure); }
#line 2417 "yyscript.c"
    break;

  case 14: /* file_cmd: GROUP $@1 '(' input_list ')'  */
#line 262 "yyscript.y"
            { script_end_group(closure); }
#line 2423 "yyscript.c"
    break;

  case 15: /* file_cmd: INHIBIT_COMMON_ALLOCATION  */
#line 264 "yyscript.y"
            { script_set_common_allocation(closure, 0); }
#line 2429 "yyscript.c"
    break;

  case 18: /* file_cmd: OPTION '(' string ')'  */
#line 268 "yyscript.y"
            { script_parse_option(closure, (yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 2435 "yyscript.c"
    break;

  case 19: /* file_cmd: OUTPUT_FORMAT '(' string ')'  */
#line 270 "yyscript.y"
            {
	      if (!script_check_output_format(closure, (yyvsp[-1].string).value, (yyvsp[-1].string).length,
					      NULL, 0, NULL, 0))
		YYABORT;
	    }
#line 2445 "yyscript.c"
    break;

  case 20: /* file_cmd: OUTPUT_FORMAT '(' string ',' string ',' string ')'  */
#line 276 "yyscript.y"
            {
	      if (!script_check_output_format(closure, (yyvsp[-5].string).value, (yyvsp[-5].string).length,
					      (yyvsp[-3].string).value, (yyvsp[-3].string).length,
					      (yyvsp[-1].string).value, (yyvsp[-1].string).length))
		YYABORT;
	    }
#line 2456 "yyscript.c"
    break;

  case 22: /* file_cmd: SEARCH_DIR '(' string ')'  */
#line 284 "yyscript.y"
            { script_add_search_dir(closure, (yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 2462 "yyscript.c"
    break;

  case 23: /* $@2: %empty  */
#line 286 "yyscript.y"
            { script_start_sections(closure); }
#line 2468 "yyscript.c"
    break;

  case 24: /* file_cmd: SECTIONS '{' $@2 sections_block '}'  */
#line 288 "yyscript.y"
            { script_finish_sections(closure); }
#line 2474 "yyscript.c"
    break;

  case 25: /* file_cmd: TARGET_K '(' string ')'  */
#line 290 "yyscript.y"
            { script_set_target(closure, (yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 2480 "yyscript.c"
    break;

  case 26: /* $@3: %empty  */
#line 292 "yyscript.y"
            { script_push_lex_into_version_mode(closure); }
#line 2486 "yyscript.c"
    break;

  case 27: /* file_cmd: VERSIONK '{' $@3 version_script '}'  */
#line 294 "yyscript.y"
            { script_pop_lex_mode(closure); }
#line 2492 "yyscript.c"
    break;

  case 28: /* file_cmd: ENTRY '(' string ')'  */
#line 296 "yyscript.y"
            { script_set_entry(closure, (yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 2498 "yyscript.c"
    break;

  case 30: /* file_cmd: ASSERT_K '(' parse_exp ',' string ')'  */
#line 299 "yyscript.y"
            { script_add_assertion(closure, (yyvsp[-3].expr), (yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 2504 "yyscript.c"
    break;

  case 31: /* file_cmd: INCLUDE string  */
#line 301 "yyscript.y"
            { script_include_directive(PARSING_LINKER_SCRIPT, closure,
				       (yyvsp[0].string).value, (yyvsp[0].string).length); }
#line 2511 "yyscript.c"
    break;

  case 35: /* $@4: %empty  */
#line 320 "yyscript.y"
            { script_push_lex_into_expression_mode(closure); }
#line 2517 "yyscript.c"
    break;

  case 36: /* extern_name_list: $@4 extern_name_list_body  */
#line 322 "yyscript.y"
            { script_pop_lex_mode(closure); }
#line 2523 "yyscript.c"
    break;

  case 37: /* extern_name_list_body: string  */
#line 327 "yyscript.y"
            { script_add_extern(closure, (yyvsp[0].string).value, (yyvsp[0].string).length); }
#line 2529 "yyscript.c"
    break;

  case 38: /* extern_name_list_body: extern_name_list_body string  */
#line 329 "yyscript.y"
            { script_add_extern(closure, (yyvsp[0].string).value, (yyvsp[0].string).length); }
#line 2535 "yyscript.c"
    break;

  case 39: /* extern_name_list_body: extern_name_list_body ',' string  */
#line 331 "yyscript.y"
            { script_add_extern(closure, (yyvsp[0].string).value, (yyvsp[0].string).length); }
#line 2541 "yyscript.c"
    break;

  case 42: /* input_list_element: string  */
#line 343 "yyscript.y"
            { script_add_file(closure, (yyvsp[0].string).value, (yyvsp[0].string).length); }
#line 2547 "yyscript.c"
    break;

  case 43: /* input_list_element: '-' STRING  */
#line 345 "yyscript.y"
            { script_add_library(closure, (yyvsp[0].string).value, (yyvsp[0].string).length); }
#line 2553 "yyscript.c"
    break;

  case 44: /* $@5: %empty  */
#line 347 "yyscript.y"
            { script_start_as_needed(closure); }
#line 2559 "yyscript.c"
    break;

  case 45: /* input_list_element: AS_NEEDED $@5 '(' input_list ')'  */
#line 349 "yyscript.y"
            { script_end_as_needed(closure); }
#line 2565 "yyscript.c"
    break;

  case 48: /* section_block_cmd: ENTRY '(' string ')'  */
#line 361 "yyscript.y"
            { script_set_entry(closure, (yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 2571 "yyscript.c"
    break;

  case 50: /* section_block_cmd: ASSERT_K '(' parse_exp ',' string ')'  */
#line 364 "yyscript.y"
            { script_add_assertion(closure, (yyvsp[-3].expr), (yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 2577 "yyscript.c"
    break;

  case 51: /* section_block_cmd: INCLUDE string  */
#line 366 "yyscript.y"
            { script_include_directive(PARSING_SECTIONS_BLOCK, closure,
				       (yyvsp[0].string).value, (yyvsp[0].string).length); }
#line 2584 "yyscript.c"
    break;

  case 52: /* $@6: %empty  */
#line 369 "yyscript.y"
            { script_start_output_section(closure, (yyvsp[-1].string).value, (yyvsp[-1].string).length, &(yyvsp[0].output_section_header)); }
#line 2590 "yyscript.c"
    break;

  case 53: /* section_block_cmd: string section_header $@6 '{' section_cmds '}' section_trailer  */
#line 371 "yyscript.y"
            { script_finish_output_section(closure, &(yyvsp[0].output_section_trailer)); }
#line 2596 "yyscript.c"
    break;

  case 54: /* $@7: %empty  */
#line 377 "yyscript.y"
            { script_push_lex_into_expression_mode(closure); }
#line 2602 "yyscript.c"
    break;

  case 55: /* $@8: %empty  */
#line 379 "yyscript.y"
            { script_pop_lex_mode(closure); }
#line 2608 "yyscript.c"
    break;

  case 56: /* section_header: $@7 opt_address_and_section_type opt_at opt_align opt_subalign $@8 opt_constraint  */
#line 381 "yyscript.y"
            {
	      (yyval.output_section_header).address = (yyvsp[-5].output_section_header).address;
	      (yyval.output_section_header).section_type = (yyvsp[-5].output_section_header).section_type;
	      (yyval.output_section_header).load_address = (yyvsp[-4].expr);
	      (yyval.output_section_header).align = (yyvsp[-3].expr);
	      (yyval.output_section_header).subalign = (yyvsp[-2].expr);
	      (yyval.output_section_header).constraint = (yyvsp[0].constraint);
	    }
#line 2621 "yyscript.c"
    break;

  case 57: /* opt_address_and_section_type: ':'  */
#line 397 "yyscript.y"
            {
	      (yyval.output_section_header).address = NULL;
	      (yyval.output_section_header).section_type = SCRIPT_SECTION_TYPE_NONE;
	    }
#line 2630 "yyscript.c"
    break;

  case 58: /* opt_address_and_section_type: '(' ')' ':'  */
#line 402 "yyscript.y"
            {
	      (yyval.output_section_header).address = NULL;
	      (yyval.output_section_header).section_type = SCRIPT_SECTION_TYPE_NONE;
	    }
#line 2639 "yyscript.c"
    break;

  case 59: /* opt_address_and_section_type: exp ':'  */
#line 407 "yyscript.y"
            {
	      (yyval.output_section_header).address = (yyvsp[-1].expr);
	      (yyval.output_section_header).section_type = SCRIPT_SECTION_TYPE_NONE;
	    }
#line 2648 "yyscript.c"
    break;

  case 60: /* opt_address_and_section_type: exp '(' ')' ':'  */
#line 412 "yyscript.y"
            {
	      (yyval.output_section_header).address = (yyvsp[-3].expr);
	      (yyval.output_section_header).section_type = SCRIPT_SECTION_TYPE_NONE;
	    }
#line 2657 "yyscript.c"
    break;

  case 61: /* opt_address_and_section_type: '(' section_type ')' ':'  */
#line 417 "yyscript.y"
            {
	      (yyval.output_section_header).address = NULL;
	      (yyval.output_section_header).section_type = (yyvsp[-2].section_type);
	    }
#line 2666 "yyscript.c"
    break;

  case 62: /* opt_address_and_section_type: exp '(' section_type ')' ':'  */
#line 422 "yyscript.y"
            {
	      (yyval.output_section_header).address = (yyvsp[-4].expr);
	      (yyval.output_section_header).section_type = (yyvsp[-2].section_type);
	    }
#line 2675 "yyscript.c"
    break;

  case 63: /* section_type: NOLOAD  */
#line 431 "yyscript.y"
            { (yyval.section_type) = SCRIPT_SECTION_TYPE_NOLOAD; }
#line 2681 "yyscript.c"
    break;

  case 64: /* section_type: DSECT  */
#line 433 "yyscript.y"
            {
	      yyerror(closure, "DSECT section type is unsupported");
	      (yyval.section_type) = SCRIPT_SECTION_TYPE_DSECT;
	    }
#line 2690 "yyscript.c"
    break;

  case 65: /* section_type: COPY  */
#line 438 "yyscript.y"
            {
	      yyerror(closure, "COPY section type is unsupported");
	      (yyval.section_type) = SCRIPT_SECTION_TYPE_COPY;
	    }
#line 2699 "yyscript.c"
    break;

  case 66: /* section_type: INFO  */
#line 443 "yyscript.y"
            {
	      yyerror(closure, "INFO section type is unsupported");
	      (yyval.section_type) = SCRIPT_SECTION_TYPE_INFO;
	    }
#line 2708 "yyscript.c"
    break;

  case 67: /* section_type: OVERLAY  */
#line 448 "yyscript.y"
            {
	      yyerror(closure, "OVERLAY section type is unsupported");
	      (yyval.section_type) = SCRIPT_SECTION_TYPE_OVERLAY;
	    }
#line 2717 "yyscript.c"
    break;

  case 68: /* opt_at: %empty  */
#line 457 "yyscript.y"
            { (yyval.expr) = NULL; }
#line 2723 "yyscript.c"
    break;

  case 69: /* opt_at: AT '(' exp ')'  */
#line 459 "yyscript.y"
            { (yyval.expr) = (yyvsp[-1].expr); }
#line 2729 "yyscript.c"
    break;

  case 70: /* opt_align: %empty  */
#line 465 "yyscript.y"
            { (yyval.expr) = NULL; }
#line 2735 "yyscript.c"
    break;

  case 71: /* opt_align: ALIGN_K '(' exp ')'  */
#line 467 "yyscript.y"
            { (yyval.expr) = (yyvsp[-1].expr); }
#line 2741 "yyscript.c"
    break;

  case 72: /* opt_subalign: %empty  */
#line 473 "yyscript.y"
            { (yyval.expr) = NULL; }
#line 2747 "yyscript.c"
    break;

  case 73: /* opt_subalign: SUBALIGN '(' exp ')'  */
#line 475 "yyscript.y"
            { (yyval.expr) = (yyvsp[-1].expr); }
#line 2753 "yyscript.c"
    break;

  case 74: /* opt_constraint: %empty  */
#line 481 "yyscript.y"
            { (yyval.constraint) = CONSTRAINT_NONE; }
#line 2759 "yyscript.c"
    break;

  case 75: /* opt_constraint: ONLY_IF_RO  */
#line 483 "yyscript.y"
            { (yyval.constraint) = CONSTRAINT_ONLY_IF_RO; }
#line 2765 "yyscript.c"
    break;

  case 76: /* opt_constraint: ONLY_IF_RW  */
#line 485 "yyscript.y"
            { (yyval.constraint) = CONSTRAINT_ONLY_IF_RW; }
#line 2771 "yyscript.c"
    break;

  case 77: /* opt_constraint: SPECIAL  */
#line 487 "yyscript.y"
            { (yyval.constraint) = CONSTRAINT_SPECIAL; }
#line 2777 "yyscript.c"
    break;

  case 78: /* section_trailer: opt_memspec opt_at_memspec opt_phdr opt_fill opt_comma  */
#line 493 "yyscript.y"
            {
	      (yyval.output_section_trailer).fill = (yyvsp[-1].expr);
	      (yyval.output_section_trailer).phdrs = (yyvsp[-2].string_list);
	    }
#line 2786 "yyscript.c"
    break;

  case 79: /* opt_memspec: '>' string  */
#line 502 "yyscript.y"
            { script_set_section_region(closure, (yyvsp[0].string).value, (yyvsp[0].string).length, 1); }
#line 2792 "yyscript.c"
    break;

  case 81: /* opt_at_memspec: AT '>' string  */
#line 509 "yyscript.y"
            { script_set_section_region(closure, (yyvsp[0].string).value, (yyvsp[0].string).length, 0); }
#line 2798 "yyscript.c"
    break;

  case 83: /* opt_phdr: opt_phdr ':' string  */
#line 516 "yyscript.y"
            { (yyval.string_list) = script_string_list_push_back((yyvsp[-2].string_list), (yyvsp[0].string).value, (yyvsp[0].string).length); }
#line 2804 "yyscript.c"
    break;

  case 84: /* opt_phdr: %empty  */
#line 518 "yyscript.y"
            { (yyval.string_list) = NULL; }
#line 2810 "yyscript.c"
    break;

  case 85: /* opt_fill: '=' parse_exp  */
#line 525 "yyscript.y"
            { (yyval.expr) = (yyvsp[0].expr); }
#line 2816 "yyscript.c"
    break;

  case 86: /* opt_fill: %empty  */
#line 527 "yyscript.y"
            { (yyval.expr) = NULL; }
#line 2822 "yyscript.c"
    break;

  case 91: /* section_cmd: data_length '(' parse_exp ')'  */
#line 543 "yyscript.y"
            { script_add_data(closure, (yyvsp[-3].integer), (yyvsp[-1].expr)); }
#line 2828 "yyscript.c"
    break;

  case 92: /* section_cmd: ASSERT_K '(' parse_exp ',' string ')'  */
#line 545 "yyscript.y"
            { script_add_assertion(closure, (yyvsp[-3].expr), (yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 2834 "yyscript.c"
    break;

  case 93: /* section_cmd: FILL '(' parse_exp ')'  */
#line 547 "yyscript.y"
            { script_add_fill(closure, (yyvsp[-1].expr)); }
#line 2840 "yyscript.c"
    break;

  case 94: /* section_cmd: CONSTRUCTORS  */
#line 549 "yyscript.y"
            {
	      /* The GNU linker uses CONSTRUCTORS for the a.out object
		 file format.  It does nothing when using ELF.  Since
		 some ELF linker scripts use it although it does
		 nothing, we accept it and ignore it.  */
	    }
#line 2851 "yyscript.c"
    break;

  case 96: /* section_cmd: INCLUDE string  */
#line 557 "yyscript.y"
            { script_include_directive(PARSING_SECTION_COMMANDS, closure,
				       (yyvsp[0].string).value, (yyvsp[0].string).length); }
#line 2858 "yyscript.c"
    break;

  case 98: /* data_length: QUAD  */
#line 566 "yyscript.y"
            { (yyval.integer) = QUAD; }
#line 2864 "yyscript.c"
    break;

  case 99: /* data_length: SQUAD  */
#line 568 "yyscript.y"
            { (yyval.integer) = SQUAD; }
#line 2870 "yyscript.c"
    break;

  case 100: /* data_length: LONG  */
#line 570 "yyscript.y"
            { (yyval.integer) = LONG; }
#line 2876 "yyscript.c"
    break;

  case 101: /* data_length: SHORT  */
#line 572 "yyscript.y"
            { (yyval.integer) = SHORT; }
#line 2882 "yyscript.c"
    break;

  case 102: /* data_length: BYTE  */
#line 574 "yyscript.y"
            { (yyval.integer) = BYTE; }
#line 2888 "yyscript.c"
    break;

  case 103: /* input_section_spec: input_section_no_keep  */
#line 581 "yyscript.y"
            { script_add_input_section(closure, &(yyvsp[0].input_section_spec), 0); }
#line 2894 "yyscript.c"
    break;

  case 104: /* input_section_spec: KEEP '(' input_section_no_keep ')'  */
#line 583 "yyscript.y"
            { script_add_input_section(closure, &(yyvsp[-1].input_section_spec), 1); }
#line 2900 "yyscript.c"
    break;

  case 105: /* input_section_no_keep: string  */
#line 589 "yyscript.y"
            {
	      (yyval.input_section_spec).file.name = (yyvsp[0].string);
	      (yyval.input_section_spec).file.sort = SORT_WILDCARD_NONE;
	      (yyval.input_section_spec).input_sections.sections = NULL;
	      (yyval.input_section_spec).input_sections.exclude = NULL;
	    }
#line 2911 "yyscript.c"
    break;

  case 106: /* input_section_no_keep: wildcard_file '(' wildcard_sections ')'  */
#line 596 "yyscript.y"
            {
	      (yyval.input_section_spec).file = (yyvsp[-3].wildcard_section);
	      (yyval.input_section_spec).input_sections = (yyvsp[-1].wildcard_sections);
	    }
#line 2920 "yyscript.c"
    break;

  case 107: /* wildcard_file: wildcard_name  */
#line 605 "yyscript.y"
            {
	      (yyval.wildcard_section).name = (yyvsp[0].string);
	      (yyval.wildcard_section).sort = SORT_WILDCARD_NONE;
	    }
#line 2929 "yyscript.c"
    break;

  case 108: /* wildcard_file: SORT_BY_NAME '(' wildcard_name ')'  */
#line 610 "yyscript.y"
            {
	      (yyval.wildcard_section).name = (yyvsp[-1].string);
	      (yyval.wildcard_section).sort = SORT_WILDCARD_BY_NAME;
	    }
#line 2938 "yyscript.c"
    break;

  case 109: /* wildcard_sections: wildcard_sections opt_comma wildcard_section  */
#line 619 "yyscript.y"
            {
	      (yyval.wildcard_sections).sections = script_string_sort_list_add((yyvsp[-2].wildcard_sections).sections, &(yyvsp[0].wildcard_section));
	      (yyval.wildcard_sections).exclude = (yyvsp[-2].wildcard_sections).exclude;
	    }
#line 2947 "yyscript.c"
    break;

  case 110: /* wildcard_sections: wildcard_section  */
#line 624 "yyscript.y"
            {
	      (yyval.wildcard_sections).sections = script_new_string_sort_list(&(yyvsp[0].wildcard_section));
	      (yyval.wildcard_sections).exclude = NULL;
	    }
#line 2956 "yyscript.c"
    break;

  case 111: /* wildcard_sections: wildcard_sections opt_comma EXCLUDE_FILE '(' exclude_names ')'  */
#line 629 "yyscript.y"
            {
	      (yyval.wildcard_sections).sections = (yyvsp[-5].wildcard_sections).sections;
	      (yyval.wildcard_sections).exclude = script_string_list_append((yyvsp[-5].wildcard_sections).exclude, (yyvsp[-1].string_list));
	    }
#line 2965 "yyscript.c"
    break;

  case 112: /* wildcard_sections: EXCLUDE_FILE '(' exclude_names ')'  */
#line 634 "yyscript.y"
            {
	      (yyval.wildcard_sections).sections = NULL;
	      (yyval.wildcard_sections).exclude = (yyvsp[-1].string_list);
	    }
#line 2974 "yyscript.c"
    break;

  case 113: /* wildcard_section: wildcard_name  */
#line 643 "yyscript.y"
            {
	      (yyval.wildcard_section).name = (yyvsp[0].string);
	      (yyval.wildcard_section).sort = SORT_WILDCARD_NONE;
	    }
#line 2983 "yyscript.c"
    break;

  case 114: /* wildcard_section: SORT_BY_NAME '(' wildcard_section ')'  */
#line 648 "yyscript.y"
            {
	      (yyval.wildcard_section).name = (yyvsp[-1].wildcard_section).name;
	      switch ((yyvsp[-1].wildcard_section).sort)
		{
		case SORT_WILDCARD_NONE:
		  (yyval.wildcard_section).sort = SORT_WILDCARD_BY_NAME;
		  break;
		case SORT_WILDCARD_BY_NAME:
		case SORT_WILDCARD_BY_NAME_BY_ALIGNMENT:
		  break;
		case SORT_WILDCARD_BY_ALIGNMENT:
		case SORT_WILDCARD_BY_ALIGNMENT_BY_NAME:
		  (yyval.wildcard_section).sort = SORT_WILDCARD_BY_NAME_BY_ALIGNMENT;
		  break;
		default:
		  abort();
		}
	    }
#line 3006 "yyscript.c"
    break;

  case 115: /* wildcard_section: SORT_BY_ALIGNMENT '(' wildcard_section ')'  */
#line 667 "yyscript.y"
            {
	      (yyval.wildcard_section).name = (yyvsp[-1].wildcard_section).name;
	      switch ((yyvsp[-1].wildcard_section).sort)
		{
		case SORT_WILDCARD_NONE:
		  (yyval.wildcard_section).sort = SORT_WILDCARD_BY_ALIGNMENT;
		  break;
		case SORT_WILDCARD_BY_ALIGNMENT:
		case SORT_WILDCARD_BY_ALIGNMENT_BY_NAME:
		  break;
		case SORT_WILDCARD_BY_NAME:
		case SORT_WILDCARD_BY_NAME_BY_ALIGNMENT:
		  (yyval.wildcard_section).sort = SORT_WILDCARD_BY_ALIGNMENT_BY_NAME;
		  break;
		default:
		  abort();
		}
	    }
#line 3029 "yyscript.c"
    break;

  case 116: /* wildcard_section: SORT_BY_INIT_PRIORITY '(' wildcard_name ')'  */
#line 686 "yyscript.y"
            {
	      (yyval.wildcard_section).name = (yyvsp[-1].string);
	      (yyval.wildcard_section).sort = SORT_WILDCARD_BY_INIT_PRIORITY;
	    }
#line 3038 "yyscript.c"
    break;

  case 117: /* exclude_names: exclude_names opt_comma wildcard_name  */
#line 695 "yyscript.y"
            { (yyval.string_list) = script_string_list_push_back((yyvsp[-2].string_list), (yyvsp[0].string).value, (yyvsp[0].string).length); }
#line 3044 "yyscript.c"
    break;

  case 118: /* exclude_names: wildcard_name  */
#line 697 "yyscript.y"
            { (yyval.string_list) = script_new_string_list((yyvsp[0].string).value, (yyvsp[0].string).length); }
#line 3050 "yyscript.c"
    break;

  case 119: /* wildcard_name: string  */
#line 704 "yyscript.y"
            { (yyval.string) = (yyvsp[0].string); }
#line 3056 "yyscript.c"
    break;

  case 120: /* wildcard_name: '*'  */
#line 706 "yyscript.y"
            {
	      (yyval.string).value = "*";
	      (yyval.string).length = 1;
	    }
#line 3065 "yyscript.c"
    break;

  case 121: /* wildcard_name: '?'  */
#line 711 "yyscript.y"
            {
	      (yyval.string).value = "?";
	      (yyval.string).length = 1;
	    }
#line 3074 "yyscript.c"
    break;

  case 124: /* memory_def: string memory_attr ':' memory_origin '=' parse_exp opt_comma memory_length '=' parse_exp  */
#line 726 "yyscript.y"
          { script_add_memory(closure, (yyvsp[-9].string).value, (yyvsp[-9].string).length, (yyvsp[-8].integer), (yyvsp[-4].expr), (yyvsp[0].expr)); }
#line 3080 "yyscript.c"
    break;

  case 125: /* memory_def: INCLUDE string  */
#line 729 "yyscript.y"
          { script_include_directive(PARSING_MEMORY_DEF, closure,
				     (yyvsp[0].string).value, (yyvsp[0].string).length); }
#line 3087 "yyscript.c"
    break;

  case 127: /* memory_attr: '(' string ')'  */
#line 737 "yyscript.y"
          { (yyval.integer) = script_parse_memory_attr(closure, (yyvsp[-1].string).value, (yyvsp[-1].string).length, 0); }
#line 3093 "yyscript.c"
    break;

  case 128: /* memory_attr: '(' '!' string ')'  */
#line 740 "yyscript.y"
          { (yyval.integer) = script_parse_memory_attr(closure, (yyvsp[-1].string).value, (yyvsp[-1].string).length, 1); }
#line 3099 "yyscript.c"
    break;

  case 129: /* memory_attr: %empty  */
#line 742 "yyscript.y"
            { (yyval.integer) = 0; }
#line 3105 "yyscript.c"
    break;

  case 138: /* phdr_def: string phdr_type phdr_info ';'  */
#line 770 "yyscript.y"
            { script_add_phdr(closure, (yyvsp[-3].string).value, (yyvsp[-3].string).length, (yyvsp[-2].integer), &(yyvsp[-1].phdr_info)); }
#line 3111 "yyscript.c"
    break;

  case 139: /* phdr_type: string  */
#line 779 "yyscript.y"
            { (yyval.integer) = script_phdr_string_to_type(closure, (yyvsp[0].string).value, (yyvsp[0].string).length); }
#line 3117 "yyscript.c"
    break;

  case 140: /* phdr_type: INTEGER  */
#line 781 "yyscript.y"
            { (yyval.integer) = (yyvsp[0].integer); }
#line 3123 "yyscript.c"
    break;

  case 141: /* phdr_info: %empty  */
#line 787 "yyscript.y"
            { memset(&(yyval.phdr_info), 0, sizeof(struct Phdr_info)); }
#line 3129 "yyscript.c"
    break;

  case 142: /* phdr_info: string phdr_info  */
#line 789 "yyscript.y"
            {
	      (yyval.phdr_info) = (yyvsp[0].phdr_info);
	      if ((yyvsp[-1].string).length == 7 && strncmp((yyvsp[-1].string).value, "FILEHDR", 7) == 0)
		(yyval.phdr_info).includes_filehdr = 1;
	      else
		yyerror(closure, "PHDRS syntax error");
	    }
#line 3141 "yyscript.c"
    break;

  case 143: /* phdr_info: PHDRS phdr_info  */
#line 797 "yyscript.y"
            {
	      (yyval.phdr_info) = (yyvsp[0].phdr_info);
	      (yyval.phdr_info).includes_phdrs = 1;
	    }
#line 3150 "yyscript.c"
    break;

  case 144: /* phdr_info: string '(' INTEGER ')' phdr_info  */
#line 802 "yyscript.y"
            {
	      (yyval.phdr_info) = (yyvsp[0].phdr_info);
	      if ((yyvsp[-4].string).length == 5 && strncmp((yyvsp[-4].string).value, "FLAGS", 5) == 0)
		{
		  (yyval.phdr_info).is_flags_valid = 1;
		  (yyval.phdr_info).flags = (yyvsp[-2].integer);
		}
	      else
		yyerror(closure, "PHDRS syntax error");
	    }
#line 3165 "yyscript.c"
    break;

  case 145: /* phdr_info: AT '(' parse_exp ')' phdr_info  */
#line 813 "yyscript.y"
            {
	      (yyval.phdr_info) = (yyvsp[0].phdr_info);
	      (yyval.phdr_info).load_address = (yyvsp[-2].expr);
	    }
#line 3174 "yyscript.c"
    break;

  case 146: /* assignment: string '=' parse_exp  */
#line 822 "yyscript.y"
            { script_set_symbol(closure, (yyvsp[-2].string).value, (yyvsp[-2].string).length, (yyvsp[0].expr), 0, 0); }
#line 3180 "yyscript.c"
    break;

  case 147: /* assignment: string PLUSEQ parse_exp  */
#line 824 "yyscript.y"
            {
	      Expression_ptr s = script_exp_string((yyvsp[-2].string).value, (yyvsp[-2].string).length);
	      Expression_ptr e = script_exp_binary_add(s, (yyvsp[0].expr));
	      script_set_symbol(closure, (yyvsp[-2].string).value, (yyvsp[-2].string).length, e, 0, 0);
	    }
#line 3190 "yyscript.c"
    break;

  case 148: /* assignment: string MINUSEQ parse_exp  */
#line 830 "yyscript.y"
            {
	      Expression_ptr s = script_exp_string((yyvsp[-2].string).value, (yyvsp[-2].string).length);
	      Expression_ptr e = script_exp_binary_sub(s, (yyvsp[0].expr));
	      script_set_symbol(closure, (yyvsp[-2].string).value, (yyvsp[-2].string).length, e, 0, 0);
	    }
#line 3200 "yyscript.c"
    break;

  case 149: /* assignment: string MULTEQ parse_exp  */
#line 836 "yyscript.y"
            {
	      Expression_ptr s = script_exp_string((yyvsp[-2].string).value, (yyvsp[-2].string).length);
	      Expression_ptr e = script_exp_binary_mult(s, (yyvsp[0].expr));
	      script_set_symbol(closure, (yyvsp[-2].string).value, (yyvsp[-2].string).length, e, 0, 0);
	    }
#line 3210 "yyscript.c"
    break;

  case 150: /* assignment: string DIVEQ parse_exp  */
#line 842 "yyscript.y"
            {
	      Expression_ptr s = script_exp_string((yyvsp[-2].string).value, (yyvsp[-2].string).length);
	      Expression_ptr e = script_exp_binary_div(s, (yyvsp[0].expr));
	      script_set_symbol(closure, (yyvsp[-2].string).value, (yyvsp[-2].string).length, e, 0, 0);
	    }
#line 3220 "yyscript.c"
    break;

  case 151: /* assignment: string LSHIFTEQ parse_exp  */
#line 848 "yyscript.y"
            {
	      Expression_ptr s = script_exp_string((yyvsp[-2].string).value, (yyvsp[-2].string).length);
	      Expression_ptr e = script_exp_binary_lshift(s, (yyvsp[0].expr));
	      script_set_symbol(closure, (yyvsp[-2].string).value, (yyvsp[-2].string).length, e, 0, 0);
	    }
#line 3230 "yyscript.c"
    break;

  case 152: /* assignment: string RSHIFTEQ parse_exp  */
#line 854 "yyscript.y"
            {
	      Expression_ptr s = script_exp_string((yyvsp[-2].string).value, (yyvsp[-2].string).length);
	      Expression_ptr e = script_exp_binary_rshift(s, (yyvsp[0].expr));
	      script_set_symbol(closure, (yyvsp[-2].string).value, (yyvsp[-2].string).length, e, 0, 0);
	    }
#line 3240 "yyscript.c"
    break;

  case 153: /* assignment: string ANDEQ parse_exp  */
#line 860 "yyscript.y"
            {
	      Expression_ptr s = script_exp_string((yyvsp[-2].string).value, (yyvsp[-2].string).length);
	      Expression_ptr e = script_exp_binary_bitwise_and(s, (yyvsp[0].expr));
	      script_set_symbol(closure, (yyvsp[-2].string).value, (yyvsp[-2].string).length, e, 0, 0);
	    }
#line 3250 "yyscript.c"
    break;

  case 154: /* assignment: string OREQ parse_exp  */
#line 866 "yyscript.y"
            {
	      Expression_ptr s = script_exp_string((yyvsp[-2].string).value, (yyvsp[-2].string).length);
	      Expression_ptr e = script_exp_binary_bitwise_or(s, (yyvsp[0].expr));
	      script_set_symbol(closure, (yyvsp[-2].string).value, (yyvsp[-2].string).length, e, 0, 0);
	    }
#line 3260 "yyscript.c"
    break;

  case 155: /* assignment: HIDDEN '(' string '=' parse_exp ')'  */
#line 872 "yyscript.y"
            { script_set_symbol(closure, (yyvsp[-3].string).value, (yyvsp[-3].string).length, (yyvsp[-1].expr), 0, 1); }
#line 3266 "yyscript.c"
    break;

  case 156: /* assignment: PROVIDE '(' string '=' parse_exp ')'  */
#line 874 "yyscript.y"
            { script_set_symbol(closure, (yyvsp[-3].string).value, (yyvsp[-3].string).length, (yyvsp[-1].expr), 1, 0); }
#line 3272 "yyscript.c"
    break;

  case 157: /* assignment: PROVIDE_HIDDEN '(' string '=' parse_exp ')'  */
#line 876 "yyscript.y"
            { script_set_symbol(closure, (yyvsp[-3].string).value, (yyvsp[-3].string).length, (yyvsp[-1].expr), 1, 1); }
#line 3278 "yyscript.c"
    break;

  case 158: /* $@9: %empty  */
#line 881 "yyscript.y"
            { script_push_lex_into_expression_mode(closure); }
#line 3284 "yyscript.c"
    break;

  case 159: /* parse_exp: $@9 exp  */
#line 883 "yyscript.y"
            {
	      script_pop_lex_mode(closure);
	      (yyval.expr) = (yyvsp[0].expr);
	    }
#line 3293 "yyscript.c"
    break;

  case 160: /* exp: '(' exp ')'  */
#line 892 "yyscript.y"
            { (yyval.expr) = (yyvsp[-1].expr); }
#line 3299 "yyscript.c"
    break;

  case 161: /* exp: '-' exp  */
#line 894 "yyscript.y"
            { (yyval.expr) = script_exp_unary_minus((yyvsp[0].expr)); }
#line 3305 "yyscript.c"
    break;

  case 162: /* exp: '!' exp  */
#line 896 "yyscript.y"
            { (yyval.expr) = script_exp_unary_logical_not((yyvsp[0].expr)); }
#line 3311 "yyscript.c"
    break;

  case 163: /* exp: '~' exp  */
#line 898 "yyscript.y"
            { (yyval.expr) = script_exp_unary_bitwise_not((yyvsp[0].expr)); }
#line 3317 "yyscript.c"
    break;

  case 164: /* exp: '+' exp  */
#line 900 "yyscript.y"
            { (yyval.expr) = (yyvsp[0].expr); }
#line 3323 "yyscript.c"
    break;

  case 165: /* exp: exp '*' exp  */
#line 902 "yyscript.y"
            { (yyval.expr) = script_exp_binary_mult((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3329 "yyscript.c"
    break;

  case 166: /* exp: exp '/' exp  */
#line 904 "yyscript.y"
            { (yyval.expr) = script_exp_binary_div((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3335 "yyscript.c"
    break;

  case 167: /* exp: exp '%' exp  */
#line 906 "yyscript.y"
            { (yyval.expr) = script_exp_binary_mod((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3341 "yyscript.c"
    break;

  case 168: /* exp: exp '+' exp  */
#line 908 "yyscript.y"
            { (yyval.expr) = script_exp_binary_add((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3347 "yyscript.c"
    break;

  case 169: /* exp: exp '-' exp  */
#line 910 "yyscript.y"
            { (yyval.expr) = script_exp_binary_sub((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3353 "yyscript.c"
    break;

  case 170: /* exp: exp LSHIFT exp  */
#line 912 "yyscript.y"
            { (yyval.expr) = script_exp_binary_lshift((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3359 "yyscript.c"
    break;

  case 171: /* exp: exp RSHIFT exp  */
#line 914 "yyscript.y"
            { (yyval.expr) = script_exp_binary_rshift((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3365 "yyscript.c"
    break;

  case 172: /* exp: exp EQ exp  */
#line 916 "yyscript.y"
            { (yyval.expr) = script_exp_binary_eq((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3371 "yyscript.c"
    break;

  case 173: /* exp: exp NE exp  */
#line 918 "yyscript.y"
            { (yyval.expr) = script_exp_binary_ne((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3377 "yyscript.c"
    break;

  case 174: /* exp: exp LE exp  */
#line 920 "yyscript.y"
            { (yyval.expr) = script_exp_binary_le((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3383 "yyscript.c"
    break;

  case 175: /* exp: exp GE exp  */
#line 922 "yyscript.y"
            { (yyval.expr) = script_exp_binary_ge((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3389 "yyscript.c"
    break;

  case 176: /* exp: exp '<' exp  */
#line 924 "yyscript.y"
            { (yyval.expr) = script_exp_binary_lt((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3395 "yyscript.c"
    break;

  case 177: /* exp: exp '>' exp  */
#line 926 "yyscript.y"
            { (yyval.expr) = script_exp_binary_gt((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3401 "yyscript.c"
    break;

  case 178: /* exp: exp '&' exp  */
#line 928 "yyscript.y"
            { (yyval.expr) = script_exp_binary_bitwise_and((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3407 "yyscript.c"
    break;

  case 179: /* exp: exp '^' exp  */
#line 930 "yyscript.y"
            { (yyval.expr) = script_exp_binary_bitwise_xor((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3413 "yyscript.c"
    break;

  case 180: /* exp: exp '|' exp  */
#line 932 "yyscript.y"
            { (yyval.expr) = script_exp_binary_bitwise_or((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3419 "yyscript.c"
    break;

  case 181: /* exp: exp ANDAND exp  */
#line 934 "yyscript.y"
            { (yyval.expr) = script_exp_binary_logical_and((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3425 "yyscript.c"
    break;

  case 182: /* exp: exp OROR exp  */
#line 936 "yyscript.y"
            { (yyval.expr) = script_exp_binary_logical_or((yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3431 "yyscript.c"
    break;

  case 183: /* exp: exp '?' exp ':' exp  */
#line 938 "yyscript.y"
            { (yyval.expr) = script_exp_trinary_cond((yyvsp[-4].expr), (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 3437 "yyscript.c"
    break;

  case 184: /* exp: INTEGER  */
#line 940 "yyscript.y"
            { (yyval.expr) = script_exp_integer((yyvsp[0].integer)); }
#line 3443 "yyscript.c"
    break;

  case 185: /* exp: string  */
#line 942 "yyscript.y"
            { (yyval.expr) = script_symbol(closure, (yyvsp[0].string).value, (yyvsp[0].string).length); }
#line 3449 "yyscript.c"
    break;

  case 186: /* exp: MAX_K '(' exp ',' exp ')'  */
#line 944 "yyscript.y"
            { (yyval.expr) = script_exp_function_max((yyvsp[-3].expr), (yyvsp[-1].expr)); }
#line 3455 "yyscript.c"
    break;

  case 187: /* exp: MIN_K '(' exp ',' exp ')'  */
#line 946 "yyscript.y"
            { (yyval.expr) = script_exp_function_min((yyvsp[-3].expr), (yyvsp[-1].expr)); }
#line 3461 "yyscript.c"
    break;

  case 188: /* exp: DEFINED '(' string ')'  */
#line 948 "yyscript.y"
            { (yyval.expr) = script_exp_function_defined((yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 3467 "yyscript.c"
    break;

  case 189: /* exp: SIZEOF_HEADERS  */
#line 950 "yyscript.y"
            { (yyval.expr) = script_exp_function_sizeof_headers(); }
#line 3473 "yyscript.c"
    break;

  case 190: /* exp: ALIGNOF '(' string ')'  */
#line 952 "yyscript.y"
            { (yyval.expr) = script_exp_function_alignof((yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 3479 "yyscript.c"
    break;

  case 191: /* exp: SIZEOF '(' string ')'  */
#line 954 "yyscript.y"
            { (yyval.expr) = script_exp_function_sizeof((yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 3485 "yyscript.c"
    break;

  case 192: /* exp: ADDR '(' string ')'  */
#line 956 "yyscript.y"
            { (yyval.expr) = script_exp_function_addr((yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 3491 "yyscript.c"
    break;

  case 193: /* exp: LOADADDR '(' string ')'  */
#line 958 "yyscript.y"
            { (yyval.expr) = script_exp_function_loadaddr((yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 3497 "yyscript.c"
    break;

  case 194: /* exp: ORIGIN '(' string ')'  */
#line 960 "yyscript.y"
            { (yyval.expr) = script_exp_function_origin(closure, (yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 3503 "yyscript.c"
    break;

  case 195: /* exp: LENGTH '(' string ')'  */
#line 962 "yyscript.y"
            { (yyval.expr) = script_exp_function_length(closure, (yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 3509 "yyscript.c"
    break;

  case 196: /* exp: CONSTANT '(' string ')'  */
#line 964 "yyscript.y"
            { (yyval.expr) = script_exp_function_constant((yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 3515 "yyscript.c"
    break;

  case 197: /* exp: ABSOLUTE '(' exp ')'  */
#line 966 "yyscript.y"
            { (yyval.expr) = script_exp_function_absolute((yyvsp[-1].expr)); }
#line 3521 "yyscript.c"
    break;

  case 198: /* exp: ALIGN_K '(' exp ')'  */
#line 968 "yyscript.y"
            { (yyval.expr) = script_exp_function_align(script_exp_string(".", 1), (yyvsp[-1].expr)); }
#line 3527 "yyscript.c"
    break;

  case 199: /* exp: ALIGN_K '(' exp ',' exp ')'  */
#line 970 "yyscript.y"
            { (yyval.expr) = script_exp_function_align((yyvsp[-3].expr), (yyvsp[-1].expr)); }
#line 3533 "yyscript.c"
    break;

  case 200: /* exp: BLOCK '(' exp ')'  */
#line 972 "yyscript.y"
            { (yyval.expr) = script_exp_function_align(script_exp_string(".", 1), (yyvsp[-1].expr)); }
#line 3539 "yyscript.c"
    break;

  case 201: /* exp: DATA_SEGMENT_ALIGN '(' exp ',' exp ')'  */
#line 974 "yyscript.y"
            {
	      script_data_segment_align(closure);
	      (yyval.expr) = script_exp_function_data_segment_align((yyvsp[-3].expr), (yyvsp[-1].expr));
	    }
#line 3548 "yyscript.c"
    break;

  case 202: /* exp: DATA_SEGMENT_RELRO_END '(' exp ',' exp ')'  */
#line 979 "yyscript.y"
            {
	      script_data_segment_relro_end(closure);
	      (yyval.expr) = script_exp_function_data_segment_relro_end((yyvsp[-3].expr), (yyvsp[-1].expr));
	    }
#line 3557 "yyscript.c"
    break;

  case 203: /* exp: DATA_SEGMENT_END '(' exp ')'  */
#line 984 "yyscript.y"
            { (yyval.expr) = script_exp_function_data_segment_end((yyvsp[-1].expr)); }
#line 3563 "yyscript.c"
    break;

  case 204: /* exp: SEGMENT_START '(' string ',' exp ')'  */
#line 986 "yyscript.y"
            {
	      (yyval.expr) = script_exp_function_segment_start((yyvsp[-3].string).value, (yyvsp[-3].string).length, (yyvsp[-1].expr));
	      /* We need to take note of any SEGMENT_START expressions
		 because they change the behaviour of -Ttext, -Tdata and
		 -Tbss options.  */
	      script_saw_segment_start_expression(closure);
	    }
#line 3575 "yyscript.c"
    break;

  case 205: /* exp: ASSERT_K '(' exp ',' string ')'  */
#line 994 "yyscript.y"
            { (yyval.expr) = script_exp_function_assert((yyvsp[-3].expr), (yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 3581 "yyscript.c"
    break;

  case 206: /* defsym_expr: string '=' parse_exp  */
#line 1000 "yyscript.y"
            { script_set_symbol(closure, (yyvsp[-2].string).value, (yyvsp[-2].string).length, (yyvsp[0].expr), 0, 0); }
#line 3587 "yyscript.c"
    break;

  case 210: /* dynamic_list_node: '{' vers_defns ';' '}' ';'  */
#line 1018 "yyscript.y"
            { script_new_vers_node (closure, NULL, (yyvsp[-3].versyms)); }
#line 3593 "yyscript.c"
    break;

  case 214: /* vers_node: '{' vers_tag '}' ';'  */
#line 1033 "yyscript.y"
            {
	      script_register_vers_node (closure, NULL, 0, (yyvsp[-2].versnode), NULL);
	    }
#line 3601 "yyscript.c"
    break;

  case 215: /* vers_node: string '{' vers_tag '}' ';'  */
#line 1037 "yyscript.y"
            {
	      script_register_vers_node (closure, (yyvsp[-4].string).value, (yyvsp[-4].string).length, (yyvsp[-2].versnode),
					 NULL);
	    }
#line 3610 "yyscript.c"
    break;

  case 216: /* vers_node: string '{' vers_tag '}' verdep ';'  */
#line 1042 "yyscript.y"
            {
	      script_register_vers_node (closure, (yyvsp[-5].string).value, (yyvsp[-5].string).length, (yyvsp[-3].versnode), (yyvsp[-1].deplist));
	    }
#line 3618 "yyscript.c"
    break;

  case 217: /* verdep: string  */
#line 1049 "yyscript.y"
            {
	      (yyval.deplist) = script_add_vers_depend (closure, NULL, (yyvsp[0].string).value, (yyvsp[0].string).length);
	    }
#line 3626 "yyscript.c"
    break;

  case 218: /* verdep: verdep string  */
#line 1053 "yyscript.y"
            {
	      (yyval.deplist) = script_add_vers_depend (closure, (yyvsp[-1].deplist), (yyvsp[0].string).value, (yyvsp[0].string).length);
	    }
#line 3634 "yyscript.c"
    break;

  case 219: /* vers_tag: %empty  */
#line 1060 "yyscript.y"
            { (yyval.versnode) = script_new_vers_node (closure, NULL, NULL); }
#line 3640 "yyscript.c"
    break;

  case 220: /* vers_tag: vers_defns ';'  */
#line 1062 "yyscript.y"
            { (yyval.versnode) = script_new_vers_node (closure, (yyvsp[-1].versyms), NULL); }
#line 3646 "yyscript.c"
    break;

  case 221: /* vers_tag: GLOBAL ':' vers_defns ';'  */
#line 1064 "yyscript.y"
            { (yyval.versnode) = script_new_vers_node (closure, (yyvsp[-1].versyms), NULL); }
#line 3652 "yyscript.c"
    break;

  case 222: /* vers_tag: LOCAL ':' vers_defns ';'  */
#line 1066 "yyscript.y"
            { (yyval.versnode) = script_new_vers_node (closure, NULL, (yyvsp[-1].versyms)); }
#line 3658 "yyscript.c"
    break;

  case 223: /* vers_tag: GLOBAL ':' vers_defns ';' LOCAL ':' vers_defns ';'  */
#line 1068 "yyscript.y"
            { (yyval.versnode) = script_new_vers_node (closure, (yyvsp[-5].versyms), (yyvsp[-1].versyms)); }
#line 3664 "yyscript.c"
    break;

  case 224: /* vers_defns: STRING  */
#line 1077 "yyscript.y"
            {
	      (yyval.versyms) = script_new_vers_pattern (closure, NULL, (yyvsp[0].string).value,
					    (yyvsp[0].string).length, 0);
	    }
#line 3673 "yyscript.c"
    break;

  case 225: /* vers_defns: QUOTED_STRING  */
#line 1082 "yyscript.y"
            {
	      (yyval.versyms) = script_new_vers_pattern (closure, NULL, (yyvsp[0].string).value,
					    (yyvsp[0].string).length, 1);
	    }
#line 3682 "yyscript.c"
    break;

  case 226: /* vers_defns: vers_defns ';' STRING  */
#line 1087 "yyscript.y"
            {
	      (yyval.versyms) = script_new_vers_pattern (closure, (yyvsp[-2].versyms), (yyvsp[0].string).value,
                                            (yyvsp[0].string).length, 0);
	    }
#line 3691 "yyscript.c"
    break;

  case 227: /* vers_defns: vers_defns ';' QUOTED_STRING  */
#line 1092 "yyscript.y"
            {
	      (yyval.versyms) = script_new_vers_pattern (closure, (yyvsp[-2].versyms), (yyvsp[0].string).value,
                                            (yyvsp[0].string).length, 1);
	    }
#line 3700 "yyscript.c"
    break;

  case 228: /* $@10: %empty  */
#line 1098 "yyscript.y"
            { version_script_push_lang (closure, (yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 3706 "yyscript.c"
    break;

  case 229: /* vers_defns: EXTERN string '{' $@10 vers_defns opt_semicolon '}'  */
#line 1100 "yyscript.y"
            {
	      (yyval.versyms) = (yyvsp[-2].versyms);
	      version_script_pop_lang(closure);
	    }
#line 3715 "yyscript.c"
    break;

  case 230: /* $@11: %empty  */
#line 1108 "yyscript.y"
            { version_script_push_lang (closure, (yyvsp[-1].string).value, (yyvsp[-1].string).length); }
#line 3721 "yyscript.c"
    break;

  case 231: /* vers_defns: vers_defns ';' EXTERN string '{' $@11 vers_defns opt_semicolon '}'  */
#line 1110 "yyscript.y"
            {
	      (yyval.versyms) = script_merge_expressions ((yyvsp[-8].versyms), (yyvsp[-2].versyms));
	      version_script_pop_lang(closure);
	    }
#line 3730 "yyscript.c"
    break;

  case 232: /* vers_defns: EXTERN  */
#line 1115 "yyscript.y"
            {
	      (yyval.versyms) = script_new_vers_pattern (closure, NULL, "extern",
					    sizeof("extern") - 1, 1);
	    }
#line 3739 "yyscript.c"
    break;

  case 233: /* vers_defns: vers_defns ';' EXTERN  */
#line 1120 "yyscript.y"
            {
	      (yyval.versyms) = script_new_vers_pattern (closure, (yyvsp[-2].versyms), "extern",
					    sizeof("extern") - 1, 1);
	    }
#line 3748 "yyscript.c"
    break;

  case 234: /* string: STRING  */
#line 1130 "yyscript.y"
            { (yyval.string) = (yyvsp[0].string); }
#line 3754 "yyscript.c"
    break;

  case 235: /* string: QUOTED_STRING  */
#line 1132 "yyscript.y"
            { (yyval.string) = (yyvsp[0].string); }
#line 3760 "yyscript.c"
    break;


#line 3764 "yyscript.c"

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
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (closure, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
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
                      yytoken, &yylval, closure);
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, closure);
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
  yyerror (closure, YY_("memory exhausted"));
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
                  yytoken, &yylval, closure);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, closure);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 1154 "yyscript.y"

