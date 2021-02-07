/* A Bison parser, made by GNU Bison 3.6.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

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

#line 178 "rtfGrammar.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int GSRTFparse (void *ctxt, void *lctxt);

#endif /* !YY_GSRTF_RTFGRAMMAR_TAB_H_INCLUDED  */
