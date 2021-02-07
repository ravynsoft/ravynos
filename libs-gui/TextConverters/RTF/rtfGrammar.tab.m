/* A Bison parser, made by GNU Bison 3.6.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.6.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         GSRTFparse
#define yylex           GSRTFlex
#define yyerror         GSRTFerror
#define yydebug         GSRTFdebug
#define yynerrs         GSRTFnerrs

/* First part of user prologue.  */
#line 36 "rtfGrammar.y"


/*
  The overall plan is to make this grammer universal in usage.
  Intrested buddies can implement plain C functions to consume what
  the grammer is producing. this way the rtf-grammer-tree can be
  converted to what is needed: GNUstep attributed strings, tex files,
  ...
  
  The plan is laid out by defining a set of C functions which cover
  all what is needed to mangle rtf information (it is NeXT centric
  however and may even lack some features).  Be aware that some
  functions are called at specific times when some information may or
  may not be available. The first argument of all functions is a
  context, which is asked to be maintained by the consumer at
  whichever purpose seems appropriate.  This context must be passed to
  the parser by issuing 'value = GSRTFparse(ctxt, lctxt);' in the
  first place.
*/

#import <AppKit/AppKit.h>
#include <stdlib.h>
#include <string.h>
#include "rtfScanner.h"

/*	this context is passed to the interface functions	*/
typedef void	*GSRTFctxt;
/*#undef YYLSP_NEEDED*/
#define CTXT            ctxt

#define	YYERROR_VERBOSE
#define YYDEBUG 1

#include "RTFConsumerFunctions.h"
/*int GSRTFlex (YYSTYPE *lvalp, RTFscannerCtxt *lctxt); */
int GSRTFlex(void *lvalp, void *lctxt);

/* */
int fieldStart = 0;


#line 118 "rtfGrammar.tab.m"

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
#ifndef YY_GSRTF_RTFGRAMMAR_TAB_H_INCLUDED
# define YY_GSRTF_RTFGRAMMAR_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int GSRTFdebug;
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
    RTFtext = 258,                 /* RTFtext  */
    RTFstart = 259,                /* RTFstart  */
    RTFansi = 260,                 /* RTFansi  */
    RTFmac = 261,                  /* RTFmac  */
    RTFpc = 262,                   /* RTFpc  */
    RTFpca = 263,                  /* RTFpca  */
    RTFignore = 264,               /* RTFignore  */
    RTFinfo = 265,                 /* RTFinfo  */
    RTFstylesheet = 266,           /* RTFstylesheet  */
    RTFfootnote = 267,             /* RTFfootnote  */
    RTFheader = 268,               /* RTFheader  */
    RTFfooter = 269,               /* RTFfooter  */
    RTFpict = 270,                 /* RTFpict  */
    RTFplain = 271,                /* RTFplain  */
    RTFparagraph = 272,            /* RTFparagraph  */
    RTFdefaultParagraph = 273,     /* RTFdefaultParagraph  */
    RTFrow = 274,                  /* RTFrow  */
    RTFcell = 275,                 /* RTFcell  */
    RTFtabulator = 276,            /* RTFtabulator  */
    RTFemdash = 277,               /* RTFemdash  */
    RTFendash = 278,               /* RTFendash  */
    RTFemspace = 279,              /* RTFemspace  */
    RTFenspace = 280,              /* RTFenspace  */
    RTFbullet = 281,               /* RTFbullet  */
    RTFfield = 282,                /* RTFfield  */
    RTFfldinst = 283,              /* RTFfldinst  */
    RTFfldalt = 284,               /* RTFfldalt  */
    RTFfldrslt = 285,              /* RTFfldrslt  */
    RTFflddirty = 286,             /* RTFflddirty  */
    RTFfldedit = 287,              /* RTFfldedit  */
    RTFfldlock = 288,              /* RTFfldlock  */
    RTFfldpriv = 289,              /* RTFfldpriv  */
    RTFfttruetype = 290,           /* RTFfttruetype  */
    RTFlquote = 291,               /* RTFlquote  */
    RTFrquote = 292,               /* RTFrquote  */
    RTFldblquote = 293,            /* RTFldblquote  */
    RTFrdblquote = 294,            /* RTFrdblquote  */
    RTFred = 295,                  /* RTFred  */
    RTFgreen = 296,                /* RTFgreen  */
    RTFblue = 297,                 /* RTFblue  */
    RTFcolorbg = 298,              /* RTFcolorbg  */
    RTFcolorfg = 299,              /* RTFcolorfg  */
    RTFunderlinecolor = 300,       /* RTFunderlinecolor  */
    RTFcolortable = 301,           /* RTFcolortable  */
    RTFfont = 302,                 /* RTFfont  */
    RTFfontSize = 303,             /* RTFfontSize  */
    RTFNeXTGraphic = 304,          /* RTFNeXTGraphic  */
    RTFNeXTGraphicWidth = 305,     /* RTFNeXTGraphicWidth  */
    RTFNeXTGraphicHeight = 306,    /* RTFNeXTGraphicHeight  */
    RTFNeXTHelpLink = 307,         /* RTFNeXTHelpLink  */
    RTFNeXTHelpMarker = 308,       /* RTFNeXTHelpMarker  */
    RTFNeXTfilename = 309,         /* RTFNeXTfilename  */
    RTFNeXTmarkername = 310,       /* RTFNeXTmarkername  */
    RTFNeXTlinkFilename = 311,     /* RTFNeXTlinkFilename  */
    RTFNeXTlinkMarkername = 312,   /* RTFNeXTlinkMarkername  */
    RTFpaperWidth = 313,           /* RTFpaperWidth  */
    RTFpaperHeight = 314,          /* RTFpaperHeight  */
    RTFmarginLeft = 315,           /* RTFmarginLeft  */
    RTFmarginRight = 316,          /* RTFmarginRight  */
    RTFmarginTop = 317,            /* RTFmarginTop  */
    RTFmarginButtom = 318,         /* RTFmarginButtom  */
    RTFfirstLineIndent = 319,      /* RTFfirstLineIndent  */
    RTFleftIndent = 320,           /* RTFleftIndent  */
    RTFrightIndent = 321,          /* RTFrightIndent  */
    RTFalignCenter = 322,          /* RTFalignCenter  */
    RTFalignJustified = 323,       /* RTFalignJustified  */
    RTFalignLeft = 324,            /* RTFalignLeft  */
    RTFalignRight = 325,           /* RTFalignRight  */
    RTFlineSpace = 326,            /* RTFlineSpace  */
    RTFspaceAbove = 327,           /* RTFspaceAbove  */
    RTFstyle = 328,                /* RTFstyle  */
    RTFbold = 329,                 /* RTFbold  */
    RTFitalic = 330,               /* RTFitalic  */
    RTFunderline = 331,            /* RTFunderline  */
    RTFunderlineDot = 332,         /* RTFunderlineDot  */
    RTFunderlineDash = 333,        /* RTFunderlineDash  */
    RTFunderlineDashDot = 334,     /* RTFunderlineDashDot  */
    RTFunderlineDashDotDot = 335,  /* RTFunderlineDashDotDot  */
    RTFunderlineDouble = 336,      /* RTFunderlineDouble  */
    RTFunderlineStop = 337,        /* RTFunderlineStop  */
    RTFunderlineThick = 338,       /* RTFunderlineThick  */
    RTFunderlineThickDot = 339,    /* RTFunderlineThickDot  */
    RTFunderlineThickDash = 340,   /* RTFunderlineThickDash  */
    RTFunderlineThickDashDot = 341, /* RTFunderlineThickDashDot  */
    RTFunderlineThickDashDotDot = 342, /* RTFunderlineThickDashDotDot  */
    RTFunderlineWord = 343,        /* RTFunderlineWord  */
    RTFstrikethrough = 344,        /* RTFstrikethrough  */
    RTFstrikethroughDouble = 345,  /* RTFstrikethroughDouble  */
    RTFunichar = 346,              /* RTFunichar  */
    RTFsubscript = 347,            /* RTFsubscript  */
    RTFsuperscript = 348,          /* RTFsuperscript  */
    RTFtabstop = 349,              /* RTFtabstop  */
    RTFfcharset = 350,             /* RTFfcharset  */
    RTFfprq = 351,                 /* RTFfprq  */
    RTFcpg = 352,                  /* RTFcpg  */
    RTFansicpg = 353,              /* RTFansicpg  */
    RTFOtherStatement = 354,       /* RTFOtherStatement  */
    RTFfontListStart = 355,        /* RTFfontListStart  */
    RTFfamilyNil = 356,            /* RTFfamilyNil  */
    RTFfamilyRoman = 357,          /* RTFfamilyRoman  */
    RTFfamilySwiss = 358,          /* RTFfamilySwiss  */
    RTFfamilyModern = 359,         /* RTFfamilyModern  */
    RTFfamilyScript = 360,         /* RTFfamilyScript  */
    RTFfamilyDecor = 361,          /* RTFfamilyDecor  */
    RTFfamilyTech = 362,           /* RTFfamilyTech  */
    RTFfamilyBiDi = 363            /* RTFfamilyBiDi  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 82 "rtfGrammar.y"

	int		number;
	const char	*text;
	RTFcmd		cmd;

#line 282 "rtfGrammar.tab.m"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int GSRTFparse (void *ctxt, void *lctxt);

#endif /* !YY_GSRTF_RTFGRAMMAR_TAB_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_RTFtext = 3,                    /* RTFtext  */
  YYSYMBOL_RTFstart = 4,                   /* RTFstart  */
  YYSYMBOL_RTFansi = 5,                    /* RTFansi  */
  YYSYMBOL_RTFmac = 6,                     /* RTFmac  */
  YYSYMBOL_RTFpc = 7,                      /* RTFpc  */
  YYSYMBOL_RTFpca = 8,                     /* RTFpca  */
  YYSYMBOL_RTFignore = 9,                  /* RTFignore  */
  YYSYMBOL_RTFinfo = 10,                   /* RTFinfo  */
  YYSYMBOL_RTFstylesheet = 11,             /* RTFstylesheet  */
  YYSYMBOL_RTFfootnote = 12,               /* RTFfootnote  */
  YYSYMBOL_RTFheader = 13,                 /* RTFheader  */
  YYSYMBOL_RTFfooter = 14,                 /* RTFfooter  */
  YYSYMBOL_RTFpict = 15,                   /* RTFpict  */
  YYSYMBOL_RTFplain = 16,                  /* RTFplain  */
  YYSYMBOL_RTFparagraph = 17,              /* RTFparagraph  */
  YYSYMBOL_RTFdefaultParagraph = 18,       /* RTFdefaultParagraph  */
  YYSYMBOL_RTFrow = 19,                    /* RTFrow  */
  YYSYMBOL_RTFcell = 20,                   /* RTFcell  */
  YYSYMBOL_RTFtabulator = 21,              /* RTFtabulator  */
  YYSYMBOL_RTFemdash = 22,                 /* RTFemdash  */
  YYSYMBOL_RTFendash = 23,                 /* RTFendash  */
  YYSYMBOL_RTFemspace = 24,                /* RTFemspace  */
  YYSYMBOL_RTFenspace = 25,                /* RTFenspace  */
  YYSYMBOL_RTFbullet = 26,                 /* RTFbullet  */
  YYSYMBOL_RTFfield = 27,                  /* RTFfield  */
  YYSYMBOL_RTFfldinst = 28,                /* RTFfldinst  */
  YYSYMBOL_RTFfldalt = 29,                 /* RTFfldalt  */
  YYSYMBOL_RTFfldrslt = 30,                /* RTFfldrslt  */
  YYSYMBOL_RTFflddirty = 31,               /* RTFflddirty  */
  YYSYMBOL_RTFfldedit = 32,                /* RTFfldedit  */
  YYSYMBOL_RTFfldlock = 33,                /* RTFfldlock  */
  YYSYMBOL_RTFfldpriv = 34,                /* RTFfldpriv  */
  YYSYMBOL_RTFfttruetype = 35,             /* RTFfttruetype  */
  YYSYMBOL_RTFlquote = 36,                 /* RTFlquote  */
  YYSYMBOL_RTFrquote = 37,                 /* RTFrquote  */
  YYSYMBOL_RTFldblquote = 38,              /* RTFldblquote  */
  YYSYMBOL_RTFrdblquote = 39,              /* RTFrdblquote  */
  YYSYMBOL_RTFred = 40,                    /* RTFred  */
  YYSYMBOL_RTFgreen = 41,                  /* RTFgreen  */
  YYSYMBOL_RTFblue = 42,                   /* RTFblue  */
  YYSYMBOL_RTFcolorbg = 43,                /* RTFcolorbg  */
  YYSYMBOL_RTFcolorfg = 44,                /* RTFcolorfg  */
  YYSYMBOL_RTFunderlinecolor = 45,         /* RTFunderlinecolor  */
  YYSYMBOL_RTFcolortable = 46,             /* RTFcolortable  */
  YYSYMBOL_RTFfont = 47,                   /* RTFfont  */
  YYSYMBOL_RTFfontSize = 48,               /* RTFfontSize  */
  YYSYMBOL_RTFNeXTGraphic = 49,            /* RTFNeXTGraphic  */
  YYSYMBOL_RTFNeXTGraphicWidth = 50,       /* RTFNeXTGraphicWidth  */
  YYSYMBOL_RTFNeXTGraphicHeight = 51,      /* RTFNeXTGraphicHeight  */
  YYSYMBOL_RTFNeXTHelpLink = 52,           /* RTFNeXTHelpLink  */
  YYSYMBOL_RTFNeXTHelpMarker = 53,         /* RTFNeXTHelpMarker  */
  YYSYMBOL_RTFNeXTfilename = 54,           /* RTFNeXTfilename  */
  YYSYMBOL_RTFNeXTmarkername = 55,         /* RTFNeXTmarkername  */
  YYSYMBOL_RTFNeXTlinkFilename = 56,       /* RTFNeXTlinkFilename  */
  YYSYMBOL_RTFNeXTlinkMarkername = 57,     /* RTFNeXTlinkMarkername  */
  YYSYMBOL_RTFpaperWidth = 58,             /* RTFpaperWidth  */
  YYSYMBOL_RTFpaperHeight = 59,            /* RTFpaperHeight  */
  YYSYMBOL_RTFmarginLeft = 60,             /* RTFmarginLeft  */
  YYSYMBOL_RTFmarginRight = 61,            /* RTFmarginRight  */
  YYSYMBOL_RTFmarginTop = 62,              /* RTFmarginTop  */
  YYSYMBOL_RTFmarginButtom = 63,           /* RTFmarginButtom  */
  YYSYMBOL_RTFfirstLineIndent = 64,        /* RTFfirstLineIndent  */
  YYSYMBOL_RTFleftIndent = 65,             /* RTFleftIndent  */
  YYSYMBOL_RTFrightIndent = 66,            /* RTFrightIndent  */
  YYSYMBOL_RTFalignCenter = 67,            /* RTFalignCenter  */
  YYSYMBOL_RTFalignJustified = 68,         /* RTFalignJustified  */
  YYSYMBOL_RTFalignLeft = 69,              /* RTFalignLeft  */
  YYSYMBOL_RTFalignRight = 70,             /* RTFalignRight  */
  YYSYMBOL_RTFlineSpace = 71,              /* RTFlineSpace  */
  YYSYMBOL_RTFspaceAbove = 72,             /* RTFspaceAbove  */
  YYSYMBOL_RTFstyle = 73,                  /* RTFstyle  */
  YYSYMBOL_RTFbold = 74,                   /* RTFbold  */
  YYSYMBOL_RTFitalic = 75,                 /* RTFitalic  */
  YYSYMBOL_RTFunderline = 76,              /* RTFunderline  */
  YYSYMBOL_RTFunderlineDot = 77,           /* RTFunderlineDot  */
  YYSYMBOL_RTFunderlineDash = 78,          /* RTFunderlineDash  */
  YYSYMBOL_RTFunderlineDashDot = 79,       /* RTFunderlineDashDot  */
  YYSYMBOL_RTFunderlineDashDotDot = 80,    /* RTFunderlineDashDotDot  */
  YYSYMBOL_RTFunderlineDouble = 81,        /* RTFunderlineDouble  */
  YYSYMBOL_RTFunderlineStop = 82,          /* RTFunderlineStop  */
  YYSYMBOL_RTFunderlineThick = 83,         /* RTFunderlineThick  */
  YYSYMBOL_RTFunderlineThickDot = 84,      /* RTFunderlineThickDot  */
  YYSYMBOL_RTFunderlineThickDash = 85,     /* RTFunderlineThickDash  */
  YYSYMBOL_RTFunderlineThickDashDot = 86,  /* RTFunderlineThickDashDot  */
  YYSYMBOL_RTFunderlineThickDashDotDot = 87, /* RTFunderlineThickDashDotDot  */
  YYSYMBOL_RTFunderlineWord = 88,          /* RTFunderlineWord  */
  YYSYMBOL_RTFstrikethrough = 89,          /* RTFstrikethrough  */
  YYSYMBOL_RTFstrikethroughDouble = 90,    /* RTFstrikethroughDouble  */
  YYSYMBOL_RTFunichar = 91,                /* RTFunichar  */
  YYSYMBOL_RTFsubscript = 92,              /* RTFsubscript  */
  YYSYMBOL_RTFsuperscript = 93,            /* RTFsuperscript  */
  YYSYMBOL_RTFtabstop = 94,                /* RTFtabstop  */
  YYSYMBOL_RTFfcharset = 95,               /* RTFfcharset  */
  YYSYMBOL_RTFfprq = 96,                   /* RTFfprq  */
  YYSYMBOL_RTFcpg = 97,                    /* RTFcpg  */
  YYSYMBOL_RTFansicpg = 98,                /* RTFansicpg  */
  YYSYMBOL_RTFOtherStatement = 99,         /* RTFOtherStatement  */
  YYSYMBOL_RTFfontListStart = 100,         /* RTFfontListStart  */
  YYSYMBOL_RTFfamilyNil = 101,             /* RTFfamilyNil  */
  YYSYMBOL_RTFfamilyRoman = 102,           /* RTFfamilyRoman  */
  YYSYMBOL_RTFfamilySwiss = 103,           /* RTFfamilySwiss  */
  YYSYMBOL_RTFfamilyModern = 104,          /* RTFfamilyModern  */
  YYSYMBOL_RTFfamilyScript = 105,          /* RTFfamilyScript  */
  YYSYMBOL_RTFfamilyDecor = 106,           /* RTFfamilyDecor  */
  YYSYMBOL_RTFfamilyTech = 107,            /* RTFfamilyTech  */
  YYSYMBOL_RTFfamilyBiDi = 108,            /* RTFfamilyBiDi  */
  YYSYMBOL_109_ = 109,                     /* '{'  */
  YYSYMBOL_110_ = 110,                     /* '}'  */
  YYSYMBOL_YYACCEPT = 111,                 /* $accept  */
  YYSYMBOL_rtfFile = 112,                  /* rtfFile  */
  YYSYMBOL_113_1 = 113,                    /* $@1  */
  YYSYMBOL_114_2 = 114,                    /* $@2  */
  YYSYMBOL_rtfCharset = 115,               /* rtfCharset  */
  YYSYMBOL_rtfIngredients = 116,           /* rtfIngredients  */
  YYSYMBOL_rtfBlock = 117,                 /* rtfBlock  */
  YYSYMBOL_118_3 = 118,                    /* $@3  */
  YYSYMBOL_119_4 = 119,                    /* $@4  */
  YYSYMBOL_120_5 = 120,                    /* $@5  */
  YYSYMBOL_121_6 = 121,                    /* $@6  */
  YYSYMBOL_122_7 = 122,                    /* $@7  */
  YYSYMBOL_123_8 = 123,                    /* $@8  */
  YYSYMBOL_124_9 = 124,                    /* $@9  */
  YYSYMBOL_125_10 = 125,                   /* $@10  */
  YYSYMBOL_126_11 = 126,                   /* $@11  */
  YYSYMBOL_rtfField = 127,                 /* rtfField  */
  YYSYMBOL_128_12 = 128,                   /* $@12  */
  YYSYMBOL_rtfFieldMod = 129,              /* rtfFieldMod  */
  YYSYMBOL_rtfIgnore = 130,                /* rtfIgnore  */
  YYSYMBOL_rtfFieldinst = 131,             /* rtfFieldinst  */
  YYSYMBOL_132_13 = 132,                   /* $@13  */
  YYSYMBOL_133_14 = 133,                   /* $@14  */
  YYSYMBOL_rtfFieldalt = 134,              /* rtfFieldalt  */
  YYSYMBOL_rtfFieldrslt = 135,             /* rtfFieldrslt  */
  YYSYMBOL_rtfStatementList = 136,         /* rtfStatementList  */
  YYSYMBOL_rtfStatement = 137,             /* rtfStatement  */
  YYSYMBOL_rtfNeXTstuff = 138,             /* rtfNeXTstuff  */
  YYSYMBOL_rtfNeXTGraphic = 139,           /* rtfNeXTGraphic  */
  YYSYMBOL_140_15 = 140,                   /* $@15  */
  YYSYMBOL_141_16 = 141,                   /* $@16  */
  YYSYMBOL_rtfNeXTHelpLink = 142,          /* rtfNeXTHelpLink  */
  YYSYMBOL_143_17 = 143,                   /* $@17  */
  YYSYMBOL_144_18 = 144,                   /* $@18  */
  YYSYMBOL_rtfNeXTHelpMarker = 145,        /* rtfNeXTHelpMarker  */
  YYSYMBOL_146_19 = 146,                   /* $@19  */
  YYSYMBOL_147_20 = 147,                   /* $@20  */
  YYSYMBOL_rtfFontList = 148,              /* rtfFontList  */
  YYSYMBOL_rtfFonts = 149,                 /* rtfFonts  */
  YYSYMBOL_rtfFontStatement = 150,         /* rtfFontStatement  */
  YYSYMBOL_rtfFontAttrs = 151,             /* rtfFontAttrs  */
  YYSYMBOL_rtfFontFamily = 152,            /* rtfFontFamily  */
  YYSYMBOL_rtfColorDef = 153,              /* rtfColorDef  */
  YYSYMBOL_rtfColors = 154,                /* rtfColors  */
  YYSYMBOL_rtfColorStatement = 155         /* rtfColorStatement  */
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
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
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
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1750

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  111
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  45
/* YYNRULES -- Number of rules.  */
#define YYNRULES  144
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  219

#define YYMAXUTOK   363


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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   109,     2,   110,     2,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108
};

#if YYDEBUG
  /* YYRLINEYYN -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   207,   207,   207,   207,   210,   211,   212,   213,   214,
     217,   218,   219,   220,   221,   222,   223,   224,   227,   227,
     228,   228,   229,   229,   230,   230,   231,   231,   232,   232,
     233,   233,   234,   234,   235,   235,   236,   240,   240,   241,
     244,   245,   246,   247,   248,   251,   252,   255,   256,   256,
     256,   257,   260,   261,   264,   265,   268,   269,   270,   277,
     284,   291,   298,   305,   312,   319,   326,   333,   340,   347,
     354,   361,   362,   363,   364,   365,   372,   373,   374,   375,
     382,   389,   396,   403,   410,   417,   424,   431,   438,   445,
     452,   459,   466,   467,   474,   481,   488,   495,   502,   509,
     515,   516,   517,   518,   519,   520,   524,   525,   526,   527,
     539,   539,   539,   554,   554,   554,   568,   568,   568,   577,
     580,   581,   582,   583,   589,   592,   595,   599,   600,   601,
     602,   603,   604,   609,   610,   611,   612,   613,   614,   615,
     623,   626,   627,   631,   636
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
  "\"end of file\"", "error", "\"invalid token\"", "RTFtext", "RTFstart",
  "RTFansi", "RTFmac", "RTFpc", "RTFpca", "RTFignore", "RTFinfo",
  "RTFstylesheet", "RTFfootnote", "RTFheader", "RTFfooter", "RTFpict",
  "RTFplain", "RTFparagraph", "RTFdefaultParagraph", "RTFrow", "RTFcell",
  "RTFtabulator", "RTFemdash", "RTFendash", "RTFemspace", "RTFenspace",
  "RTFbullet", "RTFfield", "RTFfldinst", "RTFfldalt", "RTFfldrslt",
  "RTFflddirty", "RTFfldedit", "RTFfldlock", "RTFfldpriv", "RTFfttruetype",
  "RTFlquote", "RTFrquote", "RTFldblquote", "RTFrdblquote", "RTFred",
  "RTFgreen", "RTFblue", "RTFcolorbg", "RTFcolorfg", "RTFunderlinecolor",
  "RTFcolortable", "RTFfont", "RTFfontSize", "RTFNeXTGraphic",
  "RTFNeXTGraphicWidth", "RTFNeXTGraphicHeight", "RTFNeXTHelpLink",
  "RTFNeXTHelpMarker", "RTFNeXTfilename", "RTFNeXTmarkername",
  "RTFNeXTlinkFilename", "RTFNeXTlinkMarkername", "RTFpaperWidth",
  "RTFpaperHeight", "RTFmarginLeft", "RTFmarginRight", "RTFmarginTop",
  "RTFmarginButtom", "RTFfirstLineIndent", "RTFleftIndent",
  "RTFrightIndent", "RTFalignCenter", "RTFalignJustified", "RTFalignLeft",
  "RTFalignRight", "RTFlineSpace", "RTFspaceAbove", "RTFstyle", "RTFbold",
  "RTFitalic", "RTFunderline", "RTFunderlineDot", "RTFunderlineDash",
  "RTFunderlineDashDot", "RTFunderlineDashDotDot", "RTFunderlineDouble",
  "RTFunderlineStop", "RTFunderlineThick", "RTFunderlineThickDot",
  "RTFunderlineThickDash", "RTFunderlineThickDashDot",
  "RTFunderlineThickDashDotDot", "RTFunderlineWord", "RTFstrikethrough",
  "RTFstrikethroughDouble", "RTFunichar", "RTFsubscript", "RTFsuperscript",
  "RTFtabstop", "RTFfcharset", "RTFfprq", "RTFcpg", "RTFansicpg",
  "RTFOtherStatement", "RTFfontListStart", "RTFfamilyNil",
  "RTFfamilyRoman", "RTFfamilySwiss", "RTFfamilyModern", "RTFfamilyScript",
  "RTFfamilyDecor", "RTFfamilyTech", "RTFfamilyBiDi", "'{'", "'}'",
  "$accept", "rtfFile", "$@1", "$@2", "rtfCharset", "rtfIngredients",
  "rtfBlock", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10",
  "$@11", "rtfField", "$@12", "rtfFieldMod", "rtfIgnore", "rtfFieldinst",
  "$@13", "$@14", "rtfFieldalt", "rtfFieldrslt", "rtfStatementList",
  "rtfStatement", "rtfNeXTstuff", "rtfNeXTGraphic", "$@15", "$@16",
  "rtfNeXTHelpLink", "$@17", "$@18", "rtfNeXTHelpMarker", "$@19", "$@20",
  "rtfFontList", "rtfFonts", "rtfFontStatement", "rtfFontAttrs",
  "rtfFontFamily", "rtfColorDef", "rtfColors", "rtfColorStatement", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   123,
     125
};
#endif

#define YYPACT_NINF (-123)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-118)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACTSTATE-NUM -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -107,  -123,     6,     3,  -123,  -123,   343,  -123,  -123,  -123,
    -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,
    -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,
    -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,
    -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,
    -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,
     138,  -102,   -87,  -123,  -123,  -123,  -123,   -98,  -123,  -123,
    -123,     4,    31,    32,    30,    42,    43,    41,    33,  -123,
    -123,  -123,    22,   -38,   438,  -123,  -123,  -123,  -123,  -123,
    -123,  -123,    13,  -123,    17,  -123,  -123,    68,    12,   -46,
    -123,  -123,    21,   -47,  -123,  -123,  -123,   533,   628,   723,
     818,   913,  1008,  1103,  -123,   -35,  -123,    34,  -123,  -123,
    -123,  -123,  -123,  -123,  -123,    87,  -123,   -30,  -105,   114,
      63,    70,  -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,
    -123,    18,   123,  -123,   268,    87,   248,  -123,   125,    83,
     131,   132,  -123,  -123,  -123,  -123,   159,    27,  -123,   363,
    -123,  -123,  -123,  -123,  -123,  -123,  -123,    28,    89,   103,
      51,    52,  -123,   135,   128,  -123,  -123,   458,  -123,    54,
     163,  -123,  -123,     7,    57,   147,  -123,  -123,   121,  -123,
     150,  -123,  -123,  -123,  -123,   177,  1198,  -123,    85,  -123,
    1293,  1388,   124,  -123,  -123,  1578,  -123,  -123,  -123,   150,
    -123,  -123,  -123,   126,  1483,  -123,  -123,   129,  -123
};

  /* YYDEFACTSTATE-NUM -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     2,     0,     0,     1,    10,     0,    17,    15,     5,
       6,     7,     8,   102,   103,    77,   104,    79,    80,    81,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    71,    72,    73,    74,    76,    75,    78,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,    82,    83,    70,   105,
       0,     0,    11,    16,    14,    12,    13,     0,   141,   120,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     4,
       9,    36,     0,     0,     0,    10,    10,    10,    10,    10,
      10,    10,     0,   144,     0,   140,   142,     0,     0,     0,
     119,   121,     0,     0,   107,   108,   109,     0,     0,     0,
       0,     0,     0,     0,    39,     0,    40,     0,   133,   134,
     135,   136,   137,   138,   139,     0,   127,     0,     0,     0,
       0,     0,    19,    21,    23,    25,    27,    29,    31,    33,
      35,     0,     0,   127,     0,     0,     0,   122,     0,     0,
       0,     0,    41,    42,    43,    44,     0,     0,   143,     0,
     124,   131,   128,   129,   130,   132,   127,     0,     0,     0,
       0,     0,    46,     0,     0,    38,   125,     0,   123,     0,
       0,   116,    51,     0,     0,     0,   126,   110,     0,    10,
      52,    48,    55,    10,    10,     0,     0,    53,     0,    56,
       0,     0,     0,   118,    47,     0,    54,   112,   113,    52,
      58,    57,    10,     0,     0,    49,   115,     0,    50
};

  /* YYPGOTONTERM-NUM.  */
static const yytype_int16 yypgoto[] =
{
    -123,  -123,  -123,  -123,  -123,   -70,  1545,  -123,  -123,  -123,
    -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,    59,
    -123,  -123,  -123,    26,  -123,  -123,    35,  -123,  -123,  -123,
    -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,   142,
     -89,  -122,  -123,  -123,  -123
};

  /* YYDEFGOTONTERM-NUM.  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,    61,    62,     6,    63,    70,    71,    72,
      73,    74,    75,    76,    77,    78,   115,   116,   141,   173,
     157,   199,   217,   198,   175,   205,    64,   103,   104,   194,
     207,   105,   212,   216,   106,   189,   203,    65,    83,   101,
     144,   126,    66,    82,    96
};

  /* YYTABLEYYPACT[STATE-NUM] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      84,    97,     1,   143,   146,   147,     4,     5,    79,    97,
     190,    80,    81,    85,   114,   107,   108,   109,   110,   111,
     112,   113,    67,   166,   -18,    93,   -18,   -18,   -18,   -18,
     -20,   -22,   -24,   -26,   -28,   -30,   -32,   -18,   -18,   -18,
     -18,    86,    88,    87,   -37,   -37,   -37,   -37,   -34,   152,
     153,   154,   155,    98,   159,    89,    91,    90,   117,   127,
      92,    98,    94,   132,   -18,   -18,   -18,    68,   -18,   -18,
     129,    99,   100,   130,   131,   140,   142,   177,   145,   -18,
     -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,
     -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,
     -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,
     -18,   -18,   -18,   -18,   -18,   -18,   191,   149,   150,   196,
     -18,    69,   -37,   200,   201,   151,   158,   156,   167,   184,
     -18,   -18,    95,   168,   169,   170,   174,   172,   178,    67,
     179,   -18,   214,   -18,   -18,   -18,   -18,   -20,   -22,   -24,
     -26,   -28,   -30,   -32,   -18,   -18,   -18,   -18,   -45,   180,
     171,   181,   182,   183,   187,   -34,   188,   192,   172,   118,
     119,   120,   121,   122,   123,   124,   125,   193,   195,   197,
     202,   -18,   -18,   -18,    68,   -18,   -18,   -45,   118,   119,
     120,   121,   122,   123,   124,   204,   -18,   -18,   -18,   -18,
     -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,
     -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,
     -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,
     -18,   -18,   -18,   185,   208,   213,   215,   -18,    69,   218,
     211,   128,     0,     0,     0,     0,     0,   -18,   -18,    67,
       0,   -18,     0,   -18,   -18,   -18,   -18,   -20,   -22,   -24,
     -26,   -28,   -30,   -32,   -18,   -18,   -18,   -18,     0,     0,
       0,   160,     0,     0,     0,   -34,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   -18,   -18,   -18,     0,   -18,   -18,     0,     0,     0,
       0,     0,     0,   161,     0,     0,   -18,   -18,   -18,   -18,
     -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,
     -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,
     -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,
     -18,   -18,   -18,     0,     7,     0,     8,   -18,     9,    10,
      11,    12,     0,     0,     0,     0,     0,   -18,   -18,    13,
      14,    15,    16,   162,   163,   164,   176,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   146,     0,     0,
       0,     0,     0,     0,     0,     0,    17,    18,    19,     0,
      20,    21,     0,     0,     0,     0,     0,     0,   161,     0,
       0,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,     0,     7,
       0,     8,    59,     9,    10,    11,    12,     0,     0,     0,
       0,     0,    60,    -3,    13,    14,    15,    16,   162,   163,
     164,   186,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   146,     0,     0,     0,     0,     0,     0,     0,
       0,    17,    18,    19,     0,    20,    21,     0,     0,     0,
       0,     0,     0,   161,     0,     0,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,     0,     7,     0,     8,    59,     9,    10,
      11,    12,     0,     0,     0,     0,     0,   102,  -106,    13,
      14,    15,    16,   162,   163,   164,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   146,     0,     0,
       0,     0,     0,     0,     0,     0,    17,    18,    19,     0,
      20,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,     0,     7,
       0,     8,    59,     9,    10,    11,    12,     0,     0,     0,
       0,     0,    60,   133,    13,    14,    15,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    17,    18,    19,     0,    20,    21,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,     0,     7,     0,     8,    59,     9,    10,
      11,    12,     0,     0,     0,     0,     0,    60,   134,    13,
      14,    15,    16,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    17,    18,    19,     0,
      20,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,     0,     7,
       0,     8,    59,     9,    10,    11,    12,     0,     0,     0,
       0,     0,    60,   135,    13,    14,    15,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    17,    18,    19,     0,    20,    21,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,     0,     7,     0,     8,    59,     9,    10,
      11,    12,     0,     0,     0,     0,     0,    60,   136,    13,
      14,    15,    16,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    17,    18,    19,     0,
      20,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,     0,     7,
       0,     8,    59,     9,    10,    11,    12,     0,     0,     0,
       0,     0,    60,   137,    13,    14,    15,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    17,    18,    19,     0,    20,    21,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,     0,     7,     0,     8,    59,     9,    10,
      11,    12,     0,     0,     0,     0,     0,    60,   138,    13,
      14,    15,    16,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    17,    18,    19,     0,
      20,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,     0,     7,
       0,     8,    59,     9,    10,    11,    12,     0,     0,     0,
       0,     0,    60,   139,    13,    14,    15,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    17,    18,    19,     0,    20,    21,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,     0,     7,     0,     8,    59,     9,    10,
      11,    12,     0,     0,     0,     0,     0,    60,  -117,    13,
      14,    15,    16,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    17,    18,    19,     0,
      20,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,     0,     7,
       0,     8,    59,     9,    10,    11,    12,     0,     0,     0,
       0,     0,    60,   206,    13,    14,    15,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    17,    18,    19,     0,    20,    21,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,     0,     7,     0,     8,    59,     9,    10,
      11,    12,     0,     0,     0,     0,     0,    60,  -111,    13,
      14,    15,    16,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    17,    18,    19,     0,
      20,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,     0,     0,
       0,   209,    59,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    60,  -114,    13,    14,    15,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    17,    18,    19,     0,    20,    21,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,   148,     0,     0,     0,    59,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   146,     0,   165,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   165,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   165,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     210
};

static const yytype_int16 yycheck[] =
{
      70,    47,   109,   125,   109,   110,     0,     4,   110,    47,
       3,    98,   110,     9,     1,    85,    86,    87,    88,    89,
      90,    91,     1,   145,     3,     3,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    10,    12,    11,    31,    32,    33,    34,    27,    31,
      32,    33,    34,    99,   143,    13,    15,    14,    41,    47,
      27,    99,    40,   110,    43,    44,    45,    46,    47,    48,
      49,   109,   110,    52,    53,   110,    42,   166,   108,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,   109,     3,    55,   189,
      99,   100,   109,   193,   194,    55,     3,   109,     3,     1,
     109,   110,   110,    50,     3,     3,   109,     9,   110,     1,
      51,     3,   212,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    30,    56,
       1,   110,   110,    28,   110,    27,     3,   110,     9,   101,
     102,   103,   104,   105,   106,   107,   108,    30,    57,    29,
       3,    43,    44,    45,    46,    47,    48,    28,   101,   102,
     103,   104,   105,   106,   107,   110,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,   174,   110,   209,   110,    99,   100,   110,
     205,    99,    -1,    -1,    -1,    -1,    -1,   109,   110,     1,
      -1,     3,    -1,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    -1,    -1,
      -1,     3,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    45,    -1,    47,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,     1,    -1,     3,    99,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,   109,   110,    16,
      17,    18,    19,    95,    96,    97,     3,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,
      47,    48,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    -1,     1,
      -1,     3,    99,     5,     6,     7,     8,    -1,    -1,    -1,
      -1,    -1,   109,   110,    16,    17,    18,    19,    95,    96,
      97,     3,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   109,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    45,    -1,    47,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,     1,    -1,     3,    99,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,   109,   110,    16,
      17,    18,    19,    95,    96,    97,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,
      47,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    -1,     1,
      -1,     3,    99,     5,     6,     7,     8,    -1,    -1,    -1,
      -1,    -1,   109,   110,    16,    17,    18,    19,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    45,    -1,    47,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,     1,    -1,     3,    99,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,   109,   110,    16,
      17,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,
      47,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    -1,     1,
      -1,     3,    99,     5,     6,     7,     8,    -1,    -1,    -1,
      -1,    -1,   109,   110,    16,    17,    18,    19,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    45,    -1,    47,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,     1,    -1,     3,    99,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,   109,   110,    16,
      17,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,
      47,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    -1,     1,
      -1,     3,    99,     5,     6,     7,     8,    -1,    -1,    -1,
      -1,    -1,   109,   110,    16,    17,    18,    19,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    45,    -1,    47,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,     1,    -1,     3,    99,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,   109,   110,    16,
      17,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,
      47,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    -1,     1,
      -1,     3,    99,     5,     6,     7,     8,    -1,    -1,    -1,
      -1,    -1,   109,   110,    16,    17,    18,    19,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    45,    -1,    47,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,     1,    -1,     3,    99,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,   109,   110,    16,
      17,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,
      47,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    -1,     1,
      -1,     3,    99,     5,     6,     7,     8,    -1,    -1,    -1,
      -1,    -1,   109,   110,    16,    17,    18,    19,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    45,    -1,    47,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    -1,     1,    -1,     3,    99,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,   109,   110,    16,
      17,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,
      47,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    -1,    -1,
      -1,     3,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   109,   110,    16,    17,    18,    19,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    45,    -1,    47,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,   128,    -1,    -1,    -1,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   109,    -1,   144,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     205
};

  /* YYSTOSSTATE-NUM -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   109,   112,   113,     0,     4,   116,     1,     3,     5,
       6,     7,     8,    16,    17,    18,    19,    43,    44,    45,
      47,    48,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    99,
     109,   114,   115,   117,   137,   148,   153,     1,    46,   100,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   110,
      98,   110,   154,   149,   116,     9,    10,    11,    12,    13,
      14,    15,    27,     3,    40,   110,   155,    47,    99,   109,
     110,   150,   109,   138,   139,   142,   145,   116,   116,   116,
     116,   116,   116,   116,     1,   127,   128,    41,   101,   102,
     103,   104,   105,   106,   107,   108,   152,    47,   150,    49,
      52,    53,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   129,    42,   152,   151,   108,   109,   110,   117,     3,
      55,    55,    31,    32,    33,    34,   109,   131,     3,   151,
       3,    35,    95,    96,    97,   117,   152,     3,    50,     3,
       3,     1,     9,   130,   109,   135,     3,   151,   110,    51,
      56,   110,   110,    28,     1,   130,     3,   110,     3,   146,
       3,   109,   110,    30,   140,    57,   116,    29,   134,   132,
     116,   116,     3,   147,   110,   136,   110,   141,   110,     3,
     117,   137,   143,   134,   116,   110,   144,   133,   110
};

  /* YYR1YYN -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   111,   113,   114,   112,   115,   115,   115,   115,   115,
     116,   116,   116,   116,   116,   116,   116,   116,   118,   117,
     119,   117,   120,   117,   121,   117,   122,   117,   123,   117,
     124,   117,   125,   117,   126,   117,   117,   128,   127,   127,
     129,   129,   129,   129,   129,   130,   130,   131,   132,   133,
     131,   131,   134,   134,   135,   135,   136,   136,   136,   137,
     137,   137,   137,   137,   137,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   138,   138,   138,   138,
     140,   141,   139,   143,   144,   142,   146,   147,   145,   148,
     149,   149,   149,   149,   150,   150,   150,   151,   151,   151,
     151,   151,   151,   152,   152,   152,   152,   152,   152,   152,
     153,   154,   154,   155,   155
};

  /* YYR2YYN -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     0,     6,     1,     1,     1,     1,     2,
       0,     2,     2,     2,     2,     2,     2,     2,     0,     5,
       0,     5,     0,     5,     0,     5,     0,     5,     0,     5,
       0,     5,     0,     5,     0,     5,     3,     0,     4,     1,
       0,     2,     2,     2,     2,     0,     1,     6,     0,     0,
      11,     3,     0,     1,     5,     3,     0,     2,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     1,     1,     1,
       0,     0,     9,     0,     0,    12,     0,     0,     8,     4,
       0,     2,     4,     6,     4,     5,     6,     0,     2,     2,
       2,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       4,     0,     2,     4,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


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
        yyerror (ctxt, lctxt, YY_("syntax error: cannot back up")); \
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

/* This macro is provided for backward compatibility. */
# ifndef YY_LOCATION_PRINT
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, ctxt, lctxt); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *ctxt, void *lctxt)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  YYUSE (ctxt);
  YYUSE (lctxt);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *ctxt, void *lctxt)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, ctxt, lctxt);
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
                 int yyrule, void *ctxt, void *lctxt)
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
                       &yyvsp[(yyi + 1) - (yynrhs)], ctxt, lctxt);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, ctxt, lctxt); \
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
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, void *ctxt, void *lctxt)
{
  YYUSE (yyvaluep);
  YYUSE (ctxt);
  YYUSE (lctxt);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *ctxt, void *lctxt)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize;

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yynerrs = 0;
  yystate = 0;
  yyerrstatus = 0;

  yystacksize = YYINITDEPTH;
  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;


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
    goto yyexhaustedlab;
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
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
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
      yychar = yylex (&yylval, lctxt);
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
  case 2:
#line 207 "rtfGrammar.y"
                    { GSRTFstart(CTXT); }
#line 1934 "rtfGrammar.tab.m"
    break;

  case 3:
#line 207 "rtfGrammar.y"
                                                                  { GSRTFstop(CTXT); }
#line 1940 "rtfGrammar.tab.m"
    break;

  case 5:
#line 210 "rtfGrammar.y"
                    { GSRTFencoding(CTXT, 1); }
#line 1946 "rtfGrammar.tab.m"
    break;

  case 6:
#line 211 "rtfGrammar.y"
                               { GSRTFencoding(CTXT, 2); }
#line 1952 "rtfGrammar.tab.m"
    break;

  case 7:
#line 212 "rtfGrammar.y"
                               { GSRTFencoding(CTXT, 3); }
#line 1958 "rtfGrammar.tab.m"
    break;

  case 8:
#line 213 "rtfGrammar.y"
                               { GSRTFencoding(CTXT, 4); }
#line 1964 "rtfGrammar.tab.m"
    break;

  case 9:
#line 214 "rtfGrammar.y"
                                              { GSRTFencoding(CTXT, (yyvsp[0].cmd).parameter); }
#line 1970 "rtfGrammar.tab.m"
    break;

  case 15:
#line 222 "rtfGrammar.y"
                                                        { GSRTFmangleText(CTXT, (yyvsp[0].text)); free((void *)(yyvsp[0].text)); }
#line 1976 "rtfGrammar.tab.m"
    break;

  case 18:
#line 227 "rtfGrammar.y"
                    { GSRTFopenBlock(CTXT, NO); }
#line 1982 "rtfGrammar.tab.m"
    break;

  case 19:
#line 227 "rtfGrammar.y"
                                                                                  { GSRTFcloseBlock(CTXT, NO); }
#line 1988 "rtfGrammar.tab.m"
    break;

  case 20:
#line 228 "rtfGrammar.y"
                            { GSRTFopenBlock(CTXT, YES); }
#line 1994 "rtfGrammar.tab.m"
    break;

  case 21:
#line 228 "rtfGrammar.y"
                                                                                        { GSRTFcloseBlock(CTXT, YES); }
#line 2000 "rtfGrammar.tab.m"
    break;

  case 22:
#line 229 "rtfGrammar.y"
                            { GSRTFopenBlock(CTXT, YES); }
#line 2006 "rtfGrammar.tab.m"
    break;

  case 23:
#line 229 "rtfGrammar.y"
                                                                                      { GSRTFcloseBlock(CTXT, YES); }
#line 2012 "rtfGrammar.tab.m"
    break;

  case 24:
#line 230 "rtfGrammar.y"
                            { GSRTFopenBlock(CTXT, YES); }
#line 2018 "rtfGrammar.tab.m"
    break;

  case 25:
#line 230 "rtfGrammar.y"
                                                                                            { GSRTFcloseBlock(CTXT, YES); }
#line 2024 "rtfGrammar.tab.m"
    break;

  case 26:
#line 231 "rtfGrammar.y"
                            { GSRTFopenBlock(CTXT, YES); }
#line 2030 "rtfGrammar.tab.m"
    break;

  case 27:
#line 231 "rtfGrammar.y"
                                                                                          { GSRTFcloseBlock(CTXT, YES); }
#line 2036 "rtfGrammar.tab.m"
    break;

  case 28:
#line 232 "rtfGrammar.y"
                            { GSRTFopenBlock(CTXT, YES); }
#line 2042 "rtfGrammar.tab.m"
    break;

  case 29:
#line 232 "rtfGrammar.y"
                                                                                        { GSRTFcloseBlock(CTXT, YES); }
#line 2048 "rtfGrammar.tab.m"
    break;

  case 30:
#line 233 "rtfGrammar.y"
                            { GSRTFopenBlock(CTXT, YES); }
#line 2054 "rtfGrammar.tab.m"
    break;

  case 31:
#line 233 "rtfGrammar.y"
                                                                                        { GSRTFcloseBlock(CTXT, YES); }
#line 2060 "rtfGrammar.tab.m"
    break;

  case 32:
#line 234 "rtfGrammar.y"
                            { GSRTFopenBlock(CTXT, YES); }
#line 2066 "rtfGrammar.tab.m"
    break;

  case 33:
#line 234 "rtfGrammar.y"
                                                                                      { GSRTFcloseBlock(CTXT, YES); }
#line 2072 "rtfGrammar.tab.m"
    break;

  case 34:
#line 235 "rtfGrammar.y"
                            { GSRTFopenBlock(CTXT, NO); }
#line 2078 "rtfGrammar.tab.m"
    break;

  case 35:
#line 235 "rtfGrammar.y"
                                                                                { GSRTFcloseBlock(CTXT, NO); }
#line 2084 "rtfGrammar.tab.m"
    break;

  case 37:
#line 240 "rtfGrammar.y"
          { fieldStart = GSRTFgetPosition(CTXT);}
#line 2090 "rtfGrammar.tab.m"
    break;

  case 38:
#line 240 "rtfGrammar.y"
                                                                                        { GSRTFaddField(CTXT, fieldStart, (yyvsp[-1].text)); free((void *)(yyvsp[-1].text)); }
#line 2096 "rtfGrammar.tab.m"
    break;

  case 47:
#line 255 "rtfGrammar.y"
                                                               { (yyval.text) = (yyvsp[-2].text);}
#line 2102 "rtfGrammar.tab.m"
    break;

  case 48:
#line 256 "rtfGrammar.y"
                                               { GSRTFopenBlock(CTXT, YES); }
#line 2108 "rtfGrammar.tab.m"
    break;

  case 49:
#line 256 "rtfGrammar.y"
                                                                                                                       { GSRTFcloseBlock(CTXT, YES); }
#line 2114 "rtfGrammar.tab.m"
    break;

  case 50:
#line 256 "rtfGrammar.y"
                                                                                                                                                           { (yyval.text) = (yyvsp[-4].text);}
#line 2120 "rtfGrammar.tab.m"
    break;

  case 51:
#line 257 "rtfGrammar.y"
                                { (yyval.text) = NULL;}
#line 2126 "rtfGrammar.tab.m"
    break;

  case 59:
#line 277 "rtfGrammar.y"
                                                { int font;
		    
						  if ((yyvsp[0].cmd).isEmpty)
						      font = 0;
						  else
						      font = (yyvsp[0].cmd).parameter;
						  GSRTFfontNumber(CTXT, font); }
#line 2138 "rtfGrammar.tab.m"
    break;

  case 60:
#line 284 "rtfGrammar.y"
                                                { int size;

						  if ((yyvsp[0].cmd).isEmpty)
						      size = 24;
						  else
						      size = (yyvsp[0].cmd).parameter;
						  GSRTFfontSize(CTXT, size); }
#line 2150 "rtfGrammar.tab.m"
    break;

  case 61:
#line 291 "rtfGrammar.y"
                                                { int width; 
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      width = 12240;
						  else
						      width = (yyvsp[0].cmd).parameter;
						  GSRTFpaperWidth(CTXT, width);}
#line 2162 "rtfGrammar.tab.m"
    break;

  case 62:
#line 298 "rtfGrammar.y"
                                                { int height; 
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      height = 15840;
						  else
						      height = (yyvsp[0].cmd).parameter;
						  GSRTFpaperHeight(CTXT, height);}
#line 2174 "rtfGrammar.tab.m"
    break;

  case 63:
#line 305 "rtfGrammar.y"
                                                { int margin; 
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      margin = 1800;
						  else
						      margin = (yyvsp[0].cmd).parameter;
						  GSRTFmarginLeft(CTXT, margin);}
#line 2186 "rtfGrammar.tab.m"
    break;

  case 64:
#line 312 "rtfGrammar.y"
                                                { int margin; 
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      margin = 1800;
						  else
						      margin = (yyvsp[0].cmd).parameter;
						  GSRTFmarginRight(CTXT, margin); }
#line 2198 "rtfGrammar.tab.m"
    break;

  case 65:
#line 319 "rtfGrammar.y"
                                                { int margin; 
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      margin = 1440;
						  else
						      margin = (yyvsp[0].cmd).parameter;
						  GSRTFmarginTop(CTXT, margin); }
#line 2210 "rtfGrammar.tab.m"
    break;

  case 66:
#line 326 "rtfGrammar.y"
                                                { int margin; 
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      margin = 1440;
						  else
						      margin = (yyvsp[0].cmd).parameter;
						  GSRTFmarginButtom(CTXT, margin); }
#line 2222 "rtfGrammar.tab.m"
    break;

  case 67:
#line 333 "rtfGrammar.y"
                                                { int indent; 
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      indent = 0;
						  else
						      indent = (yyvsp[0].cmd).parameter;
						  GSRTFfirstLineIndent(CTXT, indent); }
#line 2234 "rtfGrammar.tab.m"
    break;

  case 68:
#line 340 "rtfGrammar.y"
                                                { int indent; 
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      indent = 0;
						  else
						      indent = (yyvsp[0].cmd).parameter;
						  GSRTFleftIndent(CTXT, indent);}
#line 2246 "rtfGrammar.tab.m"
    break;

  case 69:
#line 347 "rtfGrammar.y"
                                                { int indent; 
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      indent = 0;
						  else
						      indent = (yyvsp[0].cmd).parameter;
						  GSRTFrightIndent(CTXT, indent);}
#line 2258 "rtfGrammar.tab.m"
    break;

  case 70:
#line 354 "rtfGrammar.y"
                                                { int location; 
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      location = 0;
						  else
						      location = (yyvsp[0].cmd).parameter;
						  GSRTFtabstop(CTXT, location);}
#line 2270 "rtfGrammar.tab.m"
    break;

  case 71:
#line 361 "rtfGrammar.y"
                                                { GSRTFalignCenter(CTXT); }
#line 2276 "rtfGrammar.tab.m"
    break;

  case 72:
#line 362 "rtfGrammar.y"
                                                { GSRTFalignJustified(CTXT); }
#line 2282 "rtfGrammar.tab.m"
    break;

  case 73:
#line 363 "rtfGrammar.y"
                                                { GSRTFalignLeft(CTXT); }
#line 2288 "rtfGrammar.tab.m"
    break;

  case 74:
#line 364 "rtfGrammar.y"
                                                { GSRTFalignRight(CTXT); }
#line 2294 "rtfGrammar.tab.m"
    break;

  case 75:
#line 365 "rtfGrammar.y"
                                                { int space; 
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      space = 0;
						  else
						      space = (yyvsp[0].cmd).parameter;
						  GSRTFspaceAbove(CTXT, space); }
#line 2306 "rtfGrammar.tab.m"
    break;

  case 76:
#line 372 "rtfGrammar.y"
                                                { GSRTFlineSpace(CTXT, (yyvsp[0].cmd).parameter); }
#line 2312 "rtfGrammar.tab.m"
    break;

  case 77:
#line 373 "rtfGrammar.y"
                                                { GSRTFdefaultParagraph(CTXT); }
#line 2318 "rtfGrammar.tab.m"
    break;

  case 78:
#line 374 "rtfGrammar.y"
                                                { GSRTFstyle(CTXT, (yyvsp[0].cmd).parameter); }
#line 2324 "rtfGrammar.tab.m"
    break;

  case 79:
#line 375 "rtfGrammar.y"
                                                { int color; 
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      color = 0;
						  else
						      color = (yyvsp[0].cmd).parameter;
						  GSRTFcolorbg(CTXT, color); }
#line 2336 "rtfGrammar.tab.m"
    break;

  case 80:
#line 382 "rtfGrammar.y"
                                                { int color; 
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      color = 0;
						  else
						      color = (yyvsp[0].cmd).parameter;
						  GSRTFcolorfg(CTXT, color); }
#line 2348 "rtfGrammar.tab.m"
    break;

  case 81:
#line 389 "rtfGrammar.y"
                                                { int color; 
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      color = 0;
						  else
						      color = (yyvsp[0].cmd).parameter;
						  GSRTFunderlinecolor(CTXT, color); }
#line 2360 "rtfGrammar.tab.m"
    break;

  case 82:
#line 396 "rtfGrammar.y"
                                                { int script;
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      script = 6;
						  else
						      script = (yyvsp[0].cmd).parameter;
						  GSRTFsubscript(CTXT, script); }
#line 2372 "rtfGrammar.tab.m"
    break;

  case 83:
#line 403 "rtfGrammar.y"
                                                { int script;
		
		                                  if ((yyvsp[0].cmd).isEmpty)
						      script = 6;
						  else
						      script = (yyvsp[0].cmd).parameter;
						  GSRTFsuperscript(CTXT, script); }
#line 2384 "rtfGrammar.tab.m"
    break;

  case 84:
#line 410 "rtfGrammar.y"
                                                { BOOL on;

		                                  if ((yyvsp[0].cmd).isEmpty || (yyvsp[0].cmd).parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFbold(CTXT, on); }
#line 2396 "rtfGrammar.tab.m"
    break;

  case 85:
#line 417 "rtfGrammar.y"
                                                { BOOL on;

		                                  if ((yyvsp[0].cmd).isEmpty || (yyvsp[0].cmd).parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFitalic(CTXT, on); }
#line 2408 "rtfGrammar.tab.m"
    break;

  case 86:
#line 424 "rtfGrammar.y"
                                                { BOOL on;

		                                  if ((yyvsp[0].cmd).isEmpty || (yyvsp[0].cmd).parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleSingle | NSUnderlinePatternSolid); }
#line 2420 "rtfGrammar.tab.m"
    break;

  case 87:
#line 431 "rtfGrammar.y"
                                                { BOOL on;

		                                  if ((yyvsp[0].cmd).isEmpty || (yyvsp[0].cmd).parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleSingle | NSUnderlinePatternDot); }
#line 2432 "rtfGrammar.tab.m"
    break;

  case 88:
#line 438 "rtfGrammar.y"
                                                { BOOL on;

		                                  if ((yyvsp[0].cmd).isEmpty || (yyvsp[0].cmd).parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleSingle | NSUnderlinePatternDash); }
#line 2444 "rtfGrammar.tab.m"
    break;

  case 89:
#line 445 "rtfGrammar.y"
                                                { BOOL on;

		                                  if ((yyvsp[0].cmd).isEmpty || (yyvsp[0].cmd).parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleSingle | NSUnderlinePatternDashDot); }
#line 2456 "rtfGrammar.tab.m"
    break;

  case 90:
#line 452 "rtfGrammar.y"
                                                { BOOL on;

		                                  if ((yyvsp[0].cmd).isEmpty || (yyvsp[0].cmd).parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleSingle | NSUnderlinePatternDashDotDot); }
#line 2468 "rtfGrammar.tab.m"
    break;

  case 91:
#line 459 "rtfGrammar.y"
                                                { BOOL on;

		                                  if ((yyvsp[0].cmd).isEmpty || (yyvsp[0].cmd).parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleDouble | NSUnderlinePatternSolid); }
#line 2480 "rtfGrammar.tab.m"
    break;

  case 92:
#line 466 "rtfGrammar.y"
                                { GSRTFunderline(CTXT, NO, NSUnderlineStyleNone); }
#line 2486 "rtfGrammar.tab.m"
    break;

  case 93:
#line 467 "rtfGrammar.y"
                                                { BOOL on;

		                                  if ((yyvsp[0].cmd).isEmpty || (yyvsp[0].cmd).parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleThick | NSUnderlinePatternSolid); }
#line 2498 "rtfGrammar.tab.m"
    break;

  case 94:
#line 474 "rtfGrammar.y"
                                                { BOOL on;

		                                  if ((yyvsp[0].cmd).isEmpty || (yyvsp[0].cmd).parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleThick | NSUnderlinePatternDot); }
#line 2510 "rtfGrammar.tab.m"
    break;

  case 95:
#line 481 "rtfGrammar.y"
                                                { BOOL on;

		                                  if ((yyvsp[0].cmd).isEmpty || (yyvsp[0].cmd).parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleThick | NSUnderlinePatternDash); }
#line 2522 "rtfGrammar.tab.m"
    break;

  case 96:
#line 488 "rtfGrammar.y"
                                                        { BOOL on;

		                                  if ((yyvsp[0].cmd).isEmpty || (yyvsp[0].cmd).parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleThick | NSUnderlinePatternDashDot); }
#line 2534 "rtfGrammar.tab.m"
    break;

  case 97:
#line 495 "rtfGrammar.y"
                                                    { BOOL on;

		                                  if ((yyvsp[0].cmd).isEmpty || (yyvsp[0].cmd).parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleThick | NSUnderlinePatternDashDotDot); }
#line 2546 "rtfGrammar.tab.m"
    break;

  case 98:
#line 502 "rtfGrammar.y"
                                                { BOOL on;

		                                  if ((yyvsp[0].cmd).isEmpty || (yyvsp[0].cmd).parameter)
						      on = YES;
						  else
						      on = NO;
						  GSRTFunderline(CTXT, on, NSUnderlineStyleSingle | NSUnderlinePatternSolid | NSUnderlineByWordMask); }
#line 2558 "rtfGrammar.tab.m"
    break;

  case 99:
#line 509 "rtfGrammar.y"
                                {   NSInteger style;
   if ((yyvsp[0].cmd).isEmpty || (yyvsp[0].cmd).parameter)
     style = NSUnderlineStyleSingle | NSUnderlinePatternSolid;
   else
     style = NSUnderlineStyleNone;
   GSRTFstrikethrough(CTXT, style); }
#line 2569 "rtfGrammar.tab.m"
    break;

  case 100:
#line 515 "rtfGrammar.y"
                                { GSRTFstrikethrough(CTXT, NSUnderlineStyleDouble | NSUnderlinePatternSolid); }
#line 2575 "rtfGrammar.tab.m"
    break;

  case 101:
#line 516 "rtfGrammar.y"
                                                { GSRTFunicode(CTXT, (yyvsp[0].cmd).parameter); }
#line 2581 "rtfGrammar.tab.m"
    break;

  case 102:
#line 517 "rtfGrammar.y"
                                                { GSRTFdefaultCharacterStyle(CTXT); }
#line 2587 "rtfGrammar.tab.m"
    break;

  case 103:
#line 518 "rtfGrammar.y"
                                                { GSRTFparagraph(CTXT); }
#line 2593 "rtfGrammar.tab.m"
    break;

  case 104:
#line 519 "rtfGrammar.y"
                                                { GSRTFparagraph(CTXT); }
#line 2599 "rtfGrammar.tab.m"
    break;

  case 105:
#line 520 "rtfGrammar.y"
                                                { GSRTFgenericRTFcommand(CTXT, (yyvsp[0].cmd)); 
		                                  free((void*)(yyvsp[0].cmd).name); }
#line 2606 "rtfGrammar.tab.m"
    break;

  case 110:
#line 539 "rtfGrammar.y"
                                                                                        { GSRTFopenBlock(CTXT, YES); }
#line 2612 "rtfGrammar.tab.m"
    break;

  case 111:
#line 539 "rtfGrammar.y"
                                                                                                                                      { GSRTFcloseBlock(CTXT, YES); }
#line 2618 "rtfGrammar.tab.m"
    break;

  case 112:
#line 540 "rtfGrammar.y"
                {
			GSRTFNeXTGraphic (CTXT, (yyvsp[-6].text), (yyvsp[-5].cmd).parameter, (yyvsp[-4].cmd).parameter);
		}
#line 2626 "rtfGrammar.tab.m"
    break;

  case 113:
#line 554 "rtfGrammar.y"
                                                                                                                             { GSRTFopenBlock(CTXT, YES); }
#line 2632 "rtfGrammar.tab.m"
    break;

  case 114:
#line 554 "rtfGrammar.y"
                                                                                                                                                                           { GSRTFcloseBlock(CTXT, YES); }
#line 2638 "rtfGrammar.tab.m"
    break;

  case 115:
#line 555 "rtfGrammar.y"
                {
			GSRTFNeXTHelpLink (CTXT, (yyvsp[-10].cmd).parameter, (yyvsp[-8].text), (yyvsp[-6].text), (yyvsp[-4].text));
		}
#line 2646 "rtfGrammar.tab.m"
    break;

  case 116:
#line 568 "rtfGrammar.y"
                                                                       { GSRTFopenBlock(CTXT, YES); }
#line 2652 "rtfGrammar.tab.m"
    break;

  case 117:
#line 568 "rtfGrammar.y"
                                                                                                                     { GSRTFcloseBlock(CTXT, YES); }
#line 2658 "rtfGrammar.tab.m"
    break;

  case 118:
#line 569 "rtfGrammar.y"
                {
			GSRTFNeXTHelpMarker (CTXT, (yyvsp[-6].cmd).parameter, (yyvsp[-4].text));
		}
#line 2666 "rtfGrammar.tab.m"
    break;

  case 123:
#line 584 "rtfGrammar.y"
                    { free((void *)(yyvsp[-1].text));}
#line 2672 "rtfGrammar.tab.m"
    break;

  case 124:
#line 589 "rtfGrammar.y"
                                                                        { GSRTFregisterFont(CTXT, (yyvsp[0].text), (yyvsp[-2].number), (yyvsp[-3].cmd).parameter);
                                                          free((void *)(yyvsp[0].text)); }
#line 2679 "rtfGrammar.tab.m"
    break;

  case 125:
#line 592 "rtfGrammar.y"
                                                                                        { GSRTFregisterFont(CTXT, (yyvsp[0].text), (yyvsp[-2].number), (yyvsp[-4].cmd).parameter);
                                                          free((void *)(yyvsp[0].text)); }
#line 2686 "rtfGrammar.tab.m"
    break;

  case 126:
#line 595 "rtfGrammar.y"
                                                                                                        { GSRTFregisterFont(CTXT, (yyvsp[0].text), (yyvsp[-2].number), (yyvsp[-4].cmd).parameter);
                                                          free((void *)(yyvsp[0].text)); }
#line 2693 "rtfGrammar.tab.m"
    break;

  case 133:
#line 609 "rtfGrammar.y"
                                        { (yyval.number) = RTFfamilyNil - RTFfamilyNil; }
#line 2699 "rtfGrammar.tab.m"
    break;

  case 134:
#line 610 "rtfGrammar.y"
                                        { (yyval.number) = RTFfamilyRoman - RTFfamilyNil; }
#line 2705 "rtfGrammar.tab.m"
    break;

  case 135:
#line 611 "rtfGrammar.y"
                                        { (yyval.number) = RTFfamilySwiss - RTFfamilyNil; }
#line 2711 "rtfGrammar.tab.m"
    break;

  case 136:
#line 612 "rtfGrammar.y"
                                        { (yyval.number) = RTFfamilyModern - RTFfamilyNil; }
#line 2717 "rtfGrammar.tab.m"
    break;

  case 137:
#line 613 "rtfGrammar.y"
                                        { (yyval.number) = RTFfamilyScript - RTFfamilyNil; }
#line 2723 "rtfGrammar.tab.m"
    break;

  case 138:
#line 614 "rtfGrammar.y"
                                        { (yyval.number) = RTFfamilyDecor - RTFfamilyNil; }
#line 2729 "rtfGrammar.tab.m"
    break;

  case 139:
#line 615 "rtfGrammar.y"
                                        { (yyval.number) = RTFfamilyTech - RTFfamilyNil; }
#line 2735 "rtfGrammar.tab.m"
    break;

  case 143:
#line 632 "rtfGrammar.y"
                     { 
		       GSRTFaddColor(CTXT, (yyvsp[-3].cmd).parameter, (yyvsp[-2].cmd).parameter, (yyvsp[-1].cmd).parameter);
		       free((void *)(yyvsp[0].text));
		     }
#line 2744 "rtfGrammar.tab.m"
    break;

  case 144:
#line 637 "rtfGrammar.y"
                     { 
		       GSRTFaddDefaultColor(CTXT);
		       free((void *)(yyvsp[0].text));
		     }
#line 2753 "rtfGrammar.tab.m"
    break;


#line 2757 "rtfGrammar.tab.m"

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
      yyerror (ctxt, lctxt, YY_("syntax error"));
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
                      yytoken, &yylval, ctxt, lctxt);
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, ctxt, lctxt);
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
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (ctxt, lctxt, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, ctxt, lctxt);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, ctxt, lctxt);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 649 "rtfGrammar.y"


/*	some C code here	*/

