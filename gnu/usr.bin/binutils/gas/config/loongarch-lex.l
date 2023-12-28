%option noyywrap
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
#include "loongarch-parse.h"

/* Flex generates static functions "input" & "unput" which are not used.  */
#define YY_NO_INPUT
#define YY_NO_UNPUT
%}

D	[0-9]
/* We consider anything greater than \x7f to be a "letter" for UTF-8
   support.  See the lex_type array in ../read.c.  */
L	[a-zA-Z_\.\$\x80-\xff]
H	[0-9A-Fa-f]

hex	0[xX]{H}+
oct	0[0-7]+
bin	0[bB][01]+
dec	([1-9]{D}*)|0
id	({D}+[fb])|({L}({D}|{L})*)|(:{dec}[bf])
ws	[ \t\v\f]+

%%

{dec}	{ yylval.imm = strtoull (yytext, 0, 0); return INTEGER; }
{hex}	{ yylval.imm = strtoull (yytext + 2, 0, 16); return INTEGER; }
{bin}	{ yylval.imm = strtoull (yytext + 2, 0, 2); return INTEGER; }
{oct}	{ yylval.imm = strtoull (yytext + 1, 0, 8); return INTEGER; }
{id}	{ yylval.c_str = strdup (yytext);return IDENTIFIER; }
{ws}	{ }

">>"	{ return RIGHT_OP; }
"<<"	{ return LEFT_OP; }
"&&"	{ return AND_OP; }
"||"	{ return OR_OP; }
"<="	{ return LE_OP; }
">="	{ return GE_OP; }
"=="	{ return EQ_OP; }
"!="	{ return NE_OP; }
.	{ return yytext[0];}

%%
