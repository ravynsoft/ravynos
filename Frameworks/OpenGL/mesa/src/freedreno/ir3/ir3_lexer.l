/*
 * Copyright (c) 2013 Rob Clark <robclark@freedesktop.org>
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
#include "util/ralloc.h"
#include "ir3/ir3.h"
#include "ir3_parser.h"

#define YY_NO_INPUT
#define YY_NO_UNPUT
#define TOKEN(t) (ir3_yylval.tok = t)
extern YYSTYPE ir3_yylval;
extern void *ir3_parser_dead_ctx;

void ir3_yyset_input(FILE *f);

void ir3_yyset_input(FILE *f)
{
	YY_FLUSH_BUFFER;
	ir3_yyin = f;
}

static int parse_wrmask(const char *src)
{
	int i, num = 0;
	for (i = 0; i < 4; i++) {
		if ("xyzw"[i] == src[1]) {
			num |= (1 << i);
			src++;
		}
	}
	return num;
}

static int parse_reg(const char *str)
{
	int num = 0;
	if (str[0] == 'h') {
		str++;
		num++;
	}
	str++;
	num += strtol(str, (char **)&str, 10) << 3;
	switch (str[1]) {
	case 'x': num += 0; break;
	case 'y': num += 2; break;
	case 'z': num += 4; break;
	case 'w': num += 6; break;
	default: assert(0); break;
	}
	return num;
}

%}

%option noyywrap
%option prefix="ir3_yy"

%%
"\n"                              yylineno++;
[ \t]                             ; /* ignore whitespace */
";"[^\n]*"\n"                     yylineno++; /* ignore comments */
"(0.0)"                           ir3_yylval.num = 0;  return T_FLUT_0_0;
"(0.5)"                           ir3_yylval.num = 1;  return T_FLUT_0_5;
"(1.0)"                           ir3_yylval.num = 2;  return T_FLUT_1_0;
"(2.0)"                           ir3_yylval.num = 3;  return T_FLUT_2_0;
"(e)"                             ir3_yylval.num = 4;  return T_FLUT_E;
"(pi)"                            ir3_yylval.num = 5;  return T_FLUT_PI;
"(1/pi)"                          ir3_yylval.num = 6;  return T_FLUT_INV_PI;
"(1/log2(e))"                     ir3_yylval.num = 7;  return T_FLUT_INV_LOG2_E;
"(log2(e))"                       ir3_yylval.num = 8;  return T_FLUT_LOG2_E;
"(1/log2(10))"                    ir3_yylval.num = 9;  return T_FLUT_INV_LOG2_10;
"(log2(10))"                      ir3_yylval.num = 10; return T_FLUT_LOG2_10;
"(4.0)"                           ir3_yylval.num = 11; return T_FLUT_4_0;
[0-9]+"."[0-9]+                   ir3_yylval.flt = strtod(yytext, NULL);       return T_FLOAT;
[0-9]*                            ir3_yylval.num = strtoul(yytext, NULL, 0);    return T_INT;
"0x"[0-9a-fA-F]*                  ir3_yylval.num = strtoul(yytext, NULL, 0);    return T_HEX;
"raw 0x"[0-9a-fA-F]*              ir3_yylval.u64 = strtoull(yytext + 4, NULL, 0); return T_RAW;
"@localsize"                      return TOKEN(T_A_LOCALSIZE);
"@const"                          return TOKEN(T_A_CONST);
"@buf"                            return TOKEN(T_A_BUF);
"@invocationid"                   return TOKEN(T_A_INVOCATIONID);
"@wgid"                           return TOKEN(T_A_WGID);
"@numwg"                          return TOKEN(T_A_NUMWG);
"@branchstack"                    return TOKEN(T_A_BRANCHSTACK);
"@in"                             return TOKEN(T_A_IN);
"@out"                            return TOKEN(T_A_OUT);
"@tex"                            return TOKEN(T_A_TEX);
"@pvtmem"                         return TOKEN(T_A_PVTMEM);
"@earlypreamble"                  return TOKEN(T_A_EARLYPREAMBLE);
"(sy)"                            return TOKEN(T_SY);
"(ss)"                            return TOKEN(T_SS);
"(absneg)"                        return TOKEN(T_ABSNEG);
"(neg)"                           return TOKEN(T_NEG);
"(abs)"                           return TOKEN(T_ABS);
"(r)"                             return TOKEN(T_R);
"(last)"                          return TOKEN(T_LAST);
"(ul)"                            return TOKEN(T_UL);
"(even)"                          return TOKEN(T_EVEN);
"(pos_infinity)"                  return TOKEN(T_POS_INFINITY);
"(neg_infinity)"                  return TOKEN(T_NEG_INFINITY);
"(ei)"                            return TOKEN(T_EI);
"(jp)"                            return TOKEN(T_JP);
"(eq)"                            return TOKEN(T_EQ_FLAG);
"(sat)"                           return TOKEN(T_SAT);
"(rpt"[0-7]")"                    ir3_yylval.num = strtol(yytext+4, NULL, 10); return T_RPT;
"(nop"[0-7]")"                    ir3_yylval.num = strtol(yytext+4, NULL, 10); return T_NOP;
"("[x]?[y]?[z]?[w]?")"            ir3_yylval.num = parse_wrmask(yytext); return T_WRMASK;

[h]?"r"[0-9]+"."[xyzw]            ir3_yylval.num = parse_reg(yytext); return T_REGISTER;
[h]?"c"[0-9]+"."[xyzw]            ir3_yylval.num = parse_reg(yytext); return T_CONSTANT;
"a0.x"                            return T_A0;
"a1.x"                            return T_A1;
"p0."[xyzw]                       ir3_yylval.num = parse_reg(yytext); return T_P0;
"w"[0-9]+                         ir3_yylval.num = strtol(yytext+1, NULL, 10); return T_W;
"s#"[0-9]+                        ir3_yylval.num = strtol(yytext+2, NULL, 10); return T_SAMP;
"t#"[0-9]+                        ir3_yylval.num = strtol(yytext+2, NULL, 10); return T_TEX;

                                  /* category 0: */
"nop"                             return TOKEN(T_OP_NOP);
"br"                              return TOKEN(T_OP_BR);
"brao"                            return TOKEN(T_OP_BRAO);
"braa"                            return TOKEN(T_OP_BRAA);
"brac"                            return TOKEN(T_OP_BRAC);
"bany"                            return TOKEN(T_OP_BANY);
"ball"                            return TOKEN(T_OP_BALL);
"brax"                            return TOKEN(T_OP_BRAX);
"jump"                            return TOKEN(T_OP_JUMP);
"call"                            return TOKEN(T_OP_CALL);
"ret"                             return TOKEN(T_OP_RET);
"kill"                            return TOKEN(T_OP_KILL);
"end"                             return TOKEN(T_OP_END);
"emit"                            return TOKEN(T_OP_EMIT);
"cut"                             return TOKEN(T_OP_CUT);
"chmask"                          return TOKEN(T_OP_CHMASK);
"chsh"                            return TOKEN(T_OP_CHSH);
"flow_rev"                        return TOKEN(T_OP_FLOW_REV);
"bkt"                             return TOKEN(T_OP_BKT);
"stks"                            return TOKEN(T_OP_STKS);
"stkr"                            return TOKEN(T_OP_STKR);
"xset"                            return TOKEN(T_OP_XSET);
"xclr"                            return TOKEN(T_OP_XCLR);
"getlast"                         return TOKEN(T_OP_GETLAST);
"getone"                          return TOKEN(T_OP_GETONE);
"dbg"                             return TOKEN(T_OP_DBG);
"shps"                            return TOKEN(T_OP_SHPS);
"shpe"                            return TOKEN(T_OP_SHPE);
"predt"                           return TOKEN(T_OP_PREDT);
"predf"                           return TOKEN(T_OP_PREDF);
"prede"                           return TOKEN(T_OP_PREDE);

                                  /* category 1: */
"movmsk"                          return TOKEN(T_OP_MOVMSK);
"mova1"                           return TOKEN(T_OP_MOVA1);
"mova"                            return TOKEN(T_OP_MOVA);
"mov"                             return TOKEN(T_OP_MOV);
"cov"                             return TOKEN(T_OP_COV);
"swz"                             return TOKEN(T_OP_SWZ);
"gat"                             return TOKEN(T_OP_GAT);
"sct"                             return TOKEN(T_OP_SCT);

("f16"|"f32"|"u16"|"u32"|"s16"|"s32"|"u8"|"s8"){2} ir3_yylval.str = yytext; return T_CAT1_TYPE_TYPE;

                                  /* category 2: */
"add.f"                           return TOKEN(T_OP_ADD_F);
"min.f"                           return TOKEN(T_OP_MIN_F);
"max.f"                           return TOKEN(T_OP_MAX_F);
"mul.f"                           return TOKEN(T_OP_MUL_F);
"sign.f"                          return TOKEN(T_OP_SIGN_F);
"cmps.f"                          return TOKEN(T_OP_CMPS_F);
"absneg.f"                        return TOKEN(T_OP_ABSNEG_F);
"cmpv.f"                          return TOKEN(T_OP_CMPV_F);
"floor.f"                         return TOKEN(T_OP_FLOOR_F);
"ceil.f"                          return TOKEN(T_OP_CEIL_F);
"rndne.f"                         return TOKEN(T_OP_RNDNE_F);
"rndaz.f"                         return TOKEN(T_OP_RNDAZ_F);
"trunc.f"                         return TOKEN(T_OP_TRUNC_F);
"add.u"                           return TOKEN(T_OP_ADD_U);
"add.s"                           return TOKEN(T_OP_ADD_S);
"sub.u"                           return TOKEN(T_OP_SUB_U);
"sub.s"                           return TOKEN(T_OP_SUB_S);
"cmps.u"                          return TOKEN(T_OP_CMPS_U);
"cmps.s"                          return TOKEN(T_OP_CMPS_S);
"min.u"                           return TOKEN(T_OP_MIN_U);
"min.s"                           return TOKEN(T_OP_MIN_S);
"max.u"                           return TOKEN(T_OP_MAX_U);
"max.s"                           return TOKEN(T_OP_MAX_S);
"absneg.s"                        return TOKEN(T_OP_ABSNEG_S);
"and.b"                           return TOKEN(T_OP_AND_B);
"or.b"                            return TOKEN(T_OP_OR_B);
"not.b"                           return TOKEN(T_OP_NOT_B);
"xor.b"                           return TOKEN(T_OP_XOR_B);
"cmpv.u"                          return TOKEN(T_OP_CMPV_U);
"cmpv.s"                          return TOKEN(T_OP_CMPV_S);
"mul.u24"                         return TOKEN(T_OP_MUL_U24);
"mul.s24"                         return TOKEN(T_OP_MUL_S24);
"mull.u"                          return TOKEN(T_OP_MULL_U);
"bfrev.b"                         return TOKEN(T_OP_BFREV_B);
"clz.s"                           return TOKEN(T_OP_CLZ_S);
"clz.b"                           return TOKEN(T_OP_CLZ_B);
"shl.b"                           return TOKEN(T_OP_SHL_B);
"shr.b"                           return TOKEN(T_OP_SHR_B);
"ashr.b"                          return TOKEN(T_OP_ASHR_B);
"bary.f"                          return TOKEN(T_OP_BARY_F);
"flat.b"                          return TOKEN(T_OP_FLAT_B);
"mgen.b"                          return TOKEN(T_OP_MGEN_B);
"getbit.b"                        return TOKEN(T_OP_GETBIT_B);
"setrm"                           return TOKEN(T_OP_SETRM);
"cbits.b"                         return TOKEN(T_OP_CBITS_B);
"shb"                             return TOKEN(T_OP_SHB);
"msad"                            return TOKEN(T_OP_MSAD);

                                  /* category 3: */
"mad.u16"                         return TOKEN(T_OP_MAD_U16);
"madsh.u16"                       return TOKEN(T_OP_MADSH_U16);
"mad.s16"                         return TOKEN(T_OP_MAD_S16);
"madsh.m16"                       return TOKEN(T_OP_MADSH_M16);
"mad.u24"                         return TOKEN(T_OP_MAD_U24);
"mad.s24"                         return TOKEN(T_OP_MAD_S24);
"mad.f16"                         return TOKEN(T_OP_MAD_F16);
"mad.f32"                         return TOKEN(T_OP_MAD_F32);
"sel.b16"                         return TOKEN(T_OP_SEL_B16);
"sel.b32"                         return TOKEN(T_OP_SEL_B32);
"sel.s16"                         return TOKEN(T_OP_SEL_S16);
"sel.s32"                         return TOKEN(T_OP_SEL_S32);
"sel.f16"                         return TOKEN(T_OP_SEL_F16);
"sel.f32"                         return TOKEN(T_OP_SEL_F32);
"sad.s16"                         return TOKEN(T_OP_SAD_S16);
"sad.s32"                         return TOKEN(T_OP_SAD_S32);
"shrm"                            return TOKEN(T_OP_SHRM);
"shlm"                            return TOKEN(T_OP_SHLM);
"shrg"                            return TOKEN(T_OP_SHRG);
"shlg"                            return TOKEN(T_OP_SHLG);
"andg"                            return TOKEN(T_OP_ANDG);
"dp2acc"                          return TOKEN(T_OP_DP2ACC);
"dp4acc"                          return TOKEN(T_OP_DP4ACC);
"wmm"                             return TOKEN(T_OP_WMM);
"wmm.accu"                        return TOKEN(T_OP_WMM_ACCU);

                                  /* category 4: */
"rcp"                             return TOKEN(T_OP_RCP);
"rsq"                             return TOKEN(T_OP_RSQ);
"log2"                            return TOKEN(T_OP_LOG2);
"exp2"                            return TOKEN(T_OP_EXP2);
"sin"                             return TOKEN(T_OP_SIN);
"cos"                             return TOKEN(T_OP_COS);
"sqrt"                            return TOKEN(T_OP_SQRT);
"hrsq"                            return TOKEN(T_OP_HRSQ);
"hlog2"                           return TOKEN(T_OP_HLOG2);
"hexp2"                           return TOKEN(T_OP_HEXP2);

                                  /* category 5: */
"isam"                            return TOKEN(T_OP_ISAM);
"isaml"                           return TOKEN(T_OP_ISAML);
"isamm"                           return TOKEN(T_OP_ISAMM);
"sam"                             return TOKEN(T_OP_SAM);
"samb"                            return TOKEN(T_OP_SAMB);
"saml"                            return TOKEN(T_OP_SAML);
"samgq"                           return TOKEN(T_OP_SAMGQ);
"getlod"                          return TOKEN(T_OP_GETLOD);
"conv"                            return TOKEN(T_OP_CONV);
"convm"                           return TOKEN(T_OP_CONVM);
"getsize"                         return TOKEN(T_OP_GETSIZE);
"getbuf"                          return TOKEN(T_OP_GETBUF);
"getpos"                          return TOKEN(T_OP_GETPOS);
"getinfo"                         return TOKEN(T_OP_GETINFO);
"dsx"                             return TOKEN(T_OP_DSX);
"dsy"                             return TOKEN(T_OP_DSY);
"gather4r"                        return TOKEN(T_OP_GATHER4R);
"gather4g"                        return TOKEN(T_OP_GATHER4G);
"gather4b"                        return TOKEN(T_OP_GATHER4B);
"gather4a"                        return TOKEN(T_OP_GATHER4A);
"samgp0"                          return TOKEN(T_OP_SAMGP0);
"samgp1"                          return TOKEN(T_OP_SAMGP1);
"samgp2"                          return TOKEN(T_OP_SAMGP2);
"samgp3"                          return TOKEN(T_OP_SAMGP3);
"dsxpp.1"                         return TOKEN(T_OP_DSXPP_1);
"dsypp.1"                         return TOKEN(T_OP_DSYPP_1);
"rgetpos"                         return TOKEN(T_OP_RGETPOS);
"rgetinfo"                        return TOKEN(T_OP_RGETINFO);
"brcst.active"                    return TOKEN(T_OP_BRCST_A);
"quad_shuffle.brcst"              return TOKEN(T_OP_QSHUFFLE_BRCST);
"quad_shuffle.horiz"              return TOKEN(T_OP_QSHUFFLE_H);
"quad_shuffle.vert"               return TOKEN(T_OP_QSHUFFLE_V);
"quad_shuffle.diag"               return TOKEN(T_OP_QSHUFFLE_DIAG);
"tcinv"                           return TOKEN(T_OP_TCINV);

                                  /* category 6: */
"ldg"                             return TOKEN(T_OP_LDG);
"ldg.a"                           return TOKEN(T_OP_LDG_A);
"ldl"                             return TOKEN(T_OP_LDL);
"ldp"                             return TOKEN(T_OP_LDP);
"stg"                             return TOKEN(T_OP_STG);
"stg.a"                           return TOKEN(T_OP_STG_A);
"stl"                             return TOKEN(T_OP_STL);
"stp"                             return TOKEN(T_OP_STP);
"ldib"                            return TOKEN(T_OP_LDIB);
"g2l"                             return TOKEN(T_OP_G2L);
"l2g"                             return TOKEN(T_OP_L2G);
"prefetch"                        return TOKEN(T_OP_PREFETCH);
"ldlw"                            return TOKEN(T_OP_LDLW);
"stlw"                            return TOKEN(T_OP_STLW);
"resfmt"                          return TOKEN(T_OP_RESFMT);
"resinfo"                         return TOKEN(T_OP_RESINFO);
"atomic.add"                      return TOKEN(T_OP_ATOMIC_ADD);
"atomic.sub"                      return TOKEN(T_OP_ATOMIC_SUB);
"atomic.xchg"                     return TOKEN(T_OP_ATOMIC_XCHG);
"atomic.inc"                      return TOKEN(T_OP_ATOMIC_INC);
"atomic.dec"                      return TOKEN(T_OP_ATOMIC_DEC);
"atomic.cmpxchg"                  return TOKEN(T_OP_ATOMIC_CMPXCHG);
"atomic.min"                      return TOKEN(T_OP_ATOMIC_MIN);
"atomic.max"                      return TOKEN(T_OP_ATOMIC_MAX);
"atomic.and"                      return TOKEN(T_OP_ATOMIC_AND);
"atomic.or"                       return TOKEN(T_OP_ATOMIC_OR);
"atomic.xor"                      return TOKEN(T_OP_ATOMIC_XOR);
"resinfo.b"                       return TOKEN(T_OP_RESINFO_B);
"ldib.b"                          return TOKEN(T_OP_LDIB_B);
"stib.b"                          return TOKEN(T_OP_STIB_B);
"atomic.b.add"                    return TOKEN(T_OP_ATOMIC_B_ADD);
"atomic.b.sub"                    return TOKEN(T_OP_ATOMIC_B_SUB);
"atomic.b.xchg"                   return TOKEN(T_OP_ATOMIC_B_XCHG);
"atomic.b.inc"                    return TOKEN(T_OP_ATOMIC_B_INC);
"atomic.b.dec"                    return TOKEN(T_OP_ATOMIC_B_DEC);
"atomic.b.cmpxchg"                return TOKEN(T_OP_ATOMIC_B_CMPXCHG);
"atomic.b.min"                    return TOKEN(T_OP_ATOMIC_B_MIN);
"atomic.b.max"                    return TOKEN(T_OP_ATOMIC_B_MAX);
"atomic.b.and"                    return TOKEN(T_OP_ATOMIC_B_AND);
"atomic.b.or"                     return TOKEN(T_OP_ATOMIC_B_OR);
"atomic.b.xor"                    return TOKEN(T_OP_ATOMIC_B_XOR);
"atomic.s.add"                    return TOKEN(T_OP_ATOMIC_S_ADD);
"atomic.s.sub"                    return TOKEN(T_OP_ATOMIC_S_SUB);
"atomic.s.xchg"                   return TOKEN(T_OP_ATOMIC_S_XCHG);
"atomic.s.inc"                    return TOKEN(T_OP_ATOMIC_S_INC);
"atomic.s.dec"                    return TOKEN(T_OP_ATOMIC_S_DEC);
"atomic.s.cmpxchg"                return TOKEN(T_OP_ATOMIC_S_CMPXCHG);
"atomic.s.min"                    return TOKEN(T_OP_ATOMIC_S_MIN);
"atomic.s.max"                    return TOKEN(T_OP_ATOMIC_S_MAX);
"atomic.s.and"                    return TOKEN(T_OP_ATOMIC_S_AND);
"atomic.s.or"                     return TOKEN(T_OP_ATOMIC_S_OR);
"atomic.s.xor"                    return TOKEN(T_OP_ATOMIC_S_XOR);
"atomic.g.add"                    return TOKEN(T_OP_ATOMIC_G_ADD);
"atomic.g.sub"                    return TOKEN(T_OP_ATOMIC_G_SUB);
"atomic.g.xchg"                   return TOKEN(T_OP_ATOMIC_G_XCHG);
"atomic.g.inc"                    return TOKEN(T_OP_ATOMIC_G_INC);
"atomic.g.dec"                    return TOKEN(T_OP_ATOMIC_G_DEC);
"atomic.g.cmpxchg"                return TOKEN(T_OP_ATOMIC_G_CMPXCHG);
"atomic.g.min"                    return TOKEN(T_OP_ATOMIC_G_MIN);
"atomic.g.max"                    return TOKEN(T_OP_ATOMIC_G_MAX);
"atomic.g.and"                    return TOKEN(T_OP_ATOMIC_G_AND);
"atomic.g.or"                     return TOKEN(T_OP_ATOMIC_G_OR);
"atomic.g.xor"                    return TOKEN(T_OP_ATOMIC_G_XOR);

"ldgb"                            return TOKEN(T_OP_LDGB);
"stgb"                            return TOKEN(T_OP_STGB);
"stib"                            return TOKEN(T_OP_STIB);
"ldc"                             return TOKEN(T_OP_LDC);
"ldlv"                            return TOKEN(T_OP_LDLV);
"getspid"                         return TOKEN(T_OP_GETSPID);
"getwid"                          return TOKEN(T_OP_GETWID);
"getfiberid"                      return TOKEN(T_OP_GETFIBERID);
"stc"                             return TOKEN(T_OP_STC);
"stsc"                            return TOKEN(T_OP_STSC);

("b16"|"b32"){1} ir3_yylval.str = yytext; return T_INSTR_TYPE;

                                  /* category 7: */
"bar"                             return TOKEN(T_OP_BAR);
"fence"                           return TOKEN(T_OP_FENCE);
"sleep.l"                         return TOKEN(T_OP_SLEEP);
"icinv"                           return TOKEN(T_OP_ICINV);
"dccln.all"                       return TOKEN(T_OP_DCCLN);
"dcinv.all"                       return TOKEN(T_OP_DCINV);
"dcflu.all"                       return TOKEN(T_OP_DCFLU);
"ccinv"                           return TOKEN(T_OP_CCINV);
"lock"                            return TOKEN(T_OP_LOCK);
"unlock"                          return TOKEN(T_OP_UNLOCK);
"alias"                           return TOKEN(T_OP_ALIAS);

"print"                           return TOKEN(T_OP_PRINT);

"f16"                             return TOKEN(T_TYPE_F16);
"f32"                             return TOKEN(T_TYPE_F32);
"u16"                             return TOKEN(T_TYPE_U16);
"u32"                             return TOKEN(T_TYPE_U32);
"s16"                             return TOKEN(T_TYPE_S16);
"s32"                             return TOKEN(T_TYPE_S32);
"u8"                              return TOKEN(T_TYPE_U8);
"s8"                              return TOKEN(T_TYPE_S8);

"untyped"                         return TOKEN(T_UNTYPED);
"typed"                           return TOKEN(T_TYPED);

"unsigned"                        return TOKEN(T_UNSIGNED);
"mixed"                           return TOKEN(T_MIXED);
"low"                             return TOKEN(T_LOW);
"high"                            return TOKEN(T_HIGH);

"1d"                              return TOKEN(T_1D);
"2d"                              return TOKEN(T_2D);
"3d"                              return TOKEN(T_3D);
"4d"                              return TOKEN(T_4D);

"lt"                              return TOKEN(T_LT);
"le"                              return TOKEN(T_LE);
"gt"                              return TOKEN(T_GT);
"ge"                              return TOKEN(T_GE);
"eq"                              return TOKEN(T_EQ);
"ne"                              return TOKEN(T_NE);

"a"                               return 'a';
"o"                               return 'o';
"p"                               return 'p';
"s2en"                            return TOKEN(T_S2EN);
"s"                               return 's';
"k"                               return 'k';
"base"[0-9]+                      ir3_yylval.num = strtol(yytext+4, NULL, 10); return T_BASE;
"offset"[0-9]+                    ir3_yylval.num = strtol(yytext+6, NULL, 10); return T_OFFSET;
"uniform"                         return T_UNIFORM;
"nonuniform"                      return T_NONUNIFORM;
"imm"                             return T_IMM;

"tex"                             return T_MOD_TEX;
"mem"                             return T_MOD_MEM;
"rt"                              return T_MOD_RT;

"h"                               return 'h';
"="                               return '=';
"("                               return '(';
")"                               return ')';
"["                               return '[';
"]"                               return ']';
","                               return ',';
"."                               return '.';
"-"                               return '-';
"+"                               return '+';
"|"                               return '|';
"c"                               return 'c';
"r"                               return 'r';
"hc"                              return TOKEN(T_HC);
"hr"                              return TOKEN(T_HR);
"g"                               return 'g';
"w"                               return 'w';
"l"                               return 'l';
"<"                               return '<';
">"                               return '>';
"!"                               return '!';
"#"                               return '#';
":"                               return ':';

"nan"                             return TOKEN(T_NAN);
"inf"                             return TOKEN(T_INF);

[a-zA-Z_][a-zA-Z_0-9]*            ir3_yylval.str = ralloc_strdup(ir3_parser_dead_ctx, yytext); return T_IDENTIFIER;
.                                 fprintf(stderr, "error at line %d: Unknown token: %s\n", ir3_yyget_lineno(), yytext); yyterminate();
%%
