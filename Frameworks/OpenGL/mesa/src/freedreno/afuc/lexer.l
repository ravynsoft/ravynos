/*
 * Copyright (c) 2017 Rob Clark <robdclark@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

%{
#include <stdlib.h>
#include "parser.h"
#include "asm.h"

#define YY_NO_INPUT
#define YY_NO_UNPUT

#define TOKEN(t) (yylval.tok = t)
extern YYSTYPE yylval;

%}

%option noyywrap

%%
"\n"                              yylineno++;
[ \t]                             ; /* ignore whitespace */
";"[^\n]*"\n"                     yylineno++; /* ignore comments */
0|[1-9][0-9]*                     yylval.num = strtoul(yytext, NULL, 0);    return T_INT;
"0x"[0-9a-fA-F]*                  yylval.num = strtoul(yytext, NULL, 0);    return T_HEX;

"$"[0-9a-fA-F][0-9a-fA-F]         yylval.num = parse_reg(yytext); return T_REGISTER;
"$"[a-zA-Z][a-zA-Z0-9]*           yylval.num = parse_reg(yytext); return T_REGISTER;
"b"[0-9][0-9]*                    yylval.num = parse_bit(yytext); return T_BIT;
"@"[a-zA-Z_][a-zA-Z0-9_]*         yylval.num = parse_control_reg(yytext); return T_CONTROL_REG;
"%"[a-zA-Z_][a-zA-Z0-9_]*         yylval.num = parse_sqe_reg(yytext); return T_SQE_REG;
"#"[a-zA-Z_][a-zA-Z0-9_]*         yylval.str = strdup(yytext+1);  return T_LABEL_REF; /* label reference */
[a-zA-Z_][a-zA-Z0-9_]*":"         yylval.str = parse_label_decl(yytext); return T_LABEL_DECL; /* label declaration */
"["[0-9a-fA-F][0-9a-fA-F]*"]"     yylval.num = parse_literal(yytext); return T_LITERAL;

                                  /* instructions: */
"nop"                             return TOKEN(T_OP_NOP);
"add"                             return TOKEN(T_OP_ADD);
"addhi"                           return TOKEN(T_OP_ADDHI);
"sub"                             return TOKEN(T_OP_SUB);
"subhi"                           return TOKEN(T_OP_SUBHI);
"and"                             return TOKEN(T_OP_AND);
"or"                              return TOKEN(T_OP_OR);
"xor"                             return TOKEN(T_OP_XOR);
"not"                             return TOKEN(T_OP_NOT);
"shl"                             return TOKEN(T_OP_SHL);
"ushr"                            return TOKEN(T_OP_USHR);
"ishr"                            return TOKEN(T_OP_ISHR);
"rot"                             return TOKEN(T_OP_ROT);
"mul8"                            return TOKEN(T_OP_MUL8);
"min"                             return TOKEN(T_OP_MIN);
"max"                             return TOKEN(T_OP_MAX);
"cmp"                             return TOKEN(T_OP_CMP);
"bic"                             return TOKEN(T_OP_BIC);
"msb"                             return TOKEN(T_OP_MSB);
"setbit"                          return TOKEN(T_OP_SETBIT);
"clrbit"                          return TOKEN(T_OP_CLRBIT);
"ubfx"                            return TOKEN(T_OP_UBFX);
"bfi"                             return TOKEN(T_OP_BFI);
"mov"                             return TOKEN(T_OP_MOV);
"cwrite"                          return TOKEN(T_OP_CWRITE);
"cread"                           return TOKEN(T_OP_CREAD);
"swrite"                          return TOKEN(T_OP_SWRITE);
"sread"                           return TOKEN(T_OP_SREAD);
"store"                           return TOKEN(T_OP_STORE);
"load"                            return TOKEN(T_OP_LOAD);
"brne"                            return TOKEN(T_OP_BRNE);
"breq"                            return TOKEN(T_OP_BREQ);
"ret"                             return TOKEN(T_OP_RET);
"iret"                            return TOKEN(T_OP_IRET);
"call"                            return TOKEN(T_OP_CALL);
"jump"                            return TOKEN(T_OP_JUMP);
"waitin"                          return TOKEN(T_OP_WAITIN);
"preemptleave"			  return TOKEN(T_OP_PREEMPTLEAVE);
"setsecure"			  return TOKEN(T_OP_SETSECURE);
"<<"                              return TOKEN(T_LSHIFT);
"(rep)"                           return TOKEN(T_REP);
"(xmov"[1-3]")"	                  yylval.num = yytext[5] - '0'; return T_XMOV;
"(sds"[1-3]")"                    yylval.num = yytext[4] - '0'; return T_SDS;

","                               return ',';
"["                               return '[';
"]"                               return ']';
"+"                               return '+';
"!"                               return '!';

.                                 fprintf(stderr, "error at line %d: Unknown token: %s\n", yyget_lineno(), yytext); yyterminate();

%%
