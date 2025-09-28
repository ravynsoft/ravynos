/* aarch64-tbl.h -- AArch64 opcode description table and instruction
   operand description table.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "aarch64-opc.h"

#ifndef VERIFIER
#error  VERIFIER must be defined.
#endif

/* Operand type.  */

#define OPND(x)	AARCH64_OPND_##x
#define OP0() {}
#define OP1(a) {OPND(a)}
#define OP2(a,b) {OPND(a), OPND(b)}
#define OP3(a,b,c) {OPND(a), OPND(b), OPND(c)}
#define OP4(a,b,c,d) {OPND(a), OPND(b), OPND(c), OPND(d)}
#define OP5(a,b,c,d,e) {OPND(a), OPND(b), OPND(c), OPND(d), OPND(e)}

#define QLF(x) AARCH64_OPND_QLF_##x
#define QLF1(a) {QLF(a)}
#define QLF2(a,b) {QLF(a), QLF(b)}
#define QLF3(a,b,c) {QLF(a), QLF(b), QLF(c)}
#define QLF4(a,b,c,d) {QLF(a), QLF(b), QLF(c), QLF(d)}
#define QLF5(a,b,c,d,e) {QLF(a), QLF(b), QLF(c), QLF(d), QLF(e)}

/* Qualifiers list.  */

/* e.g. MSR <systemreg>, <Xt>.  */
#define QL_SRC_X		\
{				\
  QLF2(NIL,X),			\
}

/* e.g. MRS <Xt>, <systemreg>.  */
#define QL_DST_X		\
{				\
  QLF2(X,NIL),			\
}

/* e.g. SYS #<op1>, <Cn>, <Cm>, #<op2>{, <Xt>}.  */
#define QL_SYS			\
{				\
  QLF5(NIL,CR,CR,NIL,X),	\
}

/* e.g. SYSL <Xt>, #<op1>, <Cn>, <Cm>, #<op2>.  */
#define QL_SYSL			\
{				\
  QLF5(X,NIL,CR,CR,NIL),	\
}

/* e.g. ADRP <Xd>, <label>.  */
#define QL_ADRP			\
{				\
  QLF2(X,NIL),			\
}

/* e.g. TCANCEL #<imm>.  */
#define QL_IMM_NIL		\
{				\
  QLF1(NIL),			\
}

/* e.g. B.<cond> <label>.  */
#define QL_PCREL_NIL		\
{				\
  QLF1(NIL),			\
}

/* e.g. TBZ <Xt>, #<imm>, <label>.  */
#define QL_PCREL_14		\
{				\
  QLF3(X,imm_0_63,NIL),		\
}

/* e.g. BL <label>.  */
#define QL_PCREL_26		\
{				\
  QLF1(NIL),			\
}

/* e.g. LDRSW <Xt>, <label>.  */
#define QL_X_PCREL		\
{				\
  QLF2(X,NIL),			\
}

/* e.g. LDR <Wt>, <label>.  */
#define QL_R_PCREL		\
{				\
  QLF2(W,NIL),			\
  QLF2(X,NIL),			\
}

/* e.g. LDR <Dt>, <label>.  */
#define QL_FP_PCREL		\
{				\
  QLF2(S_S,NIL),		\
  QLF2(S_D,NIL),		\
  QLF2(S_Q,NIL),		\
}

/* e.g. PRFM <prfop>, <label>.  */
#define QL_PRFM_PCREL		\
{				\
  QLF2(NIL,NIL),		\
}

/* e.g. BR <Xn>.  */
#define QL_I1X			\
{				\
  QLF1(X),			\
}

/* e.g. STG <Xt|SP>, [<Xn|SP>, #<imm9>].  */
#define QL_LDST_AT		\
{				\
  QLF2(X, imm_tag),		\
  QLF2(SP, imm_tag),		\
}

/* e.g. RBIT <Wd>, <Wn>.  */
#define QL_I2SAME		\
{				\
  QLF2(W,W),			\
  QLF2(X,X),			\
}

/* e.g. CMN <Wn|WSP>, <Wm>{, <extend> {#<amount>}}.  */
#define QL_I2_EXT		\
{				\
  QLF2(W,W),			\
  QLF2(X,W),			\
  QLF2(X,X),			\
}

/* e.g. MOV <Wd|WSP>, <Wn|WSP>, at least one SP.  */
#define QL_I2SP			\
{				\
  QLF2(WSP,W),			\
  QLF2(W,WSP),			\
  QLF2(SP,X),			\
  QLF2(X,SP),			\
}

/* e.g. REV <Wd>, <Wn>.  */
#define QL_I2SAMEW		\
{				\
  QLF2(W,W),			\
}

/* e.g. REV32 <Xd>, <Xn>.  */
#define QL_I2SAMEX		\
{				\
  QLF2(X,X),			\
}

#define QL_I2SAMER		\
{				\
  QLF2(W,W),			\
  QLF2(X,X),			\
}

/* e.g. CRC32B <Wd>, <Wn>, <Wm>.  */
#define QL_I3SAMEW		\
{				\
  QLF3(W,W,W),			\
}

/* e.g. SMULH <Xd>, <Xn>, <Xm>.  */
#define QL_I3SAMEX		\
{				\
  QLF3(X,X,X),			\
}

/* e.g. CRC32X <Wd>, <Wn>, <Xm>.  */
#define QL_I3WWX		\
{				\
  QLF3(W,W,X),			\
}

/* e.g. UDIV <Xd>, <Xn>, <Xm>.  */
#define QL_I3SAMER		\
{				\
  QLF3(W,W,W),			\
  QLF3(X,X,X),			\
}

/* e.g. ADDS <Xd>, <Xn|SP>, <R><m>{, <extend> {#<amount>}}.  */
#define QL_I3_EXT		\
{				\
  QLF3(W,W,W),			\
  QLF3(X,X,W),			\
  QLF3(X,X,X),			\
}

/* e.g. MADD <Xd>, <Xn>, <Xm>, <Xa>.  */
#define QL_I4SAMER		\
{				\
  QLF4(W,W,W,W),		\
  QLF4(X,X,X,X),		\
}

/* e.g. SMADDL <Xd>, <Wn>, <Wm>, <Xa>.  */
#define QL_I3SAMEL		\
{				\
  QLF3(X,W,W),			\
}

/* e.g. SMADDL <Xd>, <Wn>, <Wm>, <Xa>.  */
#define QL_I4SAMEL		\
{				\
  QLF4(X,W,W,X),		\
}

/* e.g. CSINC <Xd>, <Xn>, <Xm>, <cond>.  */
#define QL_CSEL			\
{				\
  QLF4(W, W, W, NIL),		\
  QLF4(X, X, X, NIL),		\
}

/* e.g. CSET <Wd>, <cond>.  */
#define QL_DST_R			\
{				\
  QLF2(W, NIL),			\
  QLF2(X, NIL),			\
}

/* e.g. BFM <Wd>, <Wn>, #<immr>, #<imms>.  */
#define QL_BF			\
{				\
  QLF4(W,W,imm_0_31,imm_0_31),	\
  QLF4(X,X,imm_0_63,imm_0_63),	\
}

/* e.g. ADDG <Xd>, <Xn>, #<uimm10>, #<uimm4>.  */
#define QL_ADDG			\
{				\
  QLF4(X,X,NIL,imm_0_15),	\
}				\

/* e.g. BFC <Wd>, #<immr>, #<imms>.  */
#define QL_BF1					\
{						\
  QLF3 (W, imm_0_31, imm_1_32),			\
  QLF3 (X, imm_0_63, imm_1_64),			\
}

/* e.g. UBFIZ <Wd>, <Wn>, #<lsb>, #<width>.  */
#define QL_BF2			\
{				\
  QLF4(W,W,imm_0_31,imm_1_32),	\
  QLF4(X,X,imm_0_63,imm_1_64),	\
}

/* e.g. SCVTF <Sd>, <Xn>, #<fbits>.  */
#define QL_FIX2FP		\
{				\
  QLF3(S_D,W,imm_1_32),		\
  QLF3(S_S,W,imm_1_32),		\
  QLF3(S_D,X,imm_1_64),		\
  QLF3(S_S,X,imm_1_64),		\
}

/* e.g. SCVTF <Hd>, <Xn>, #<fbits>.  */
#define QL_FIX2FP_H			\
{					\
  QLF3 (S_H, W, imm_1_32),		\
  QLF3 (S_H, X, imm_1_64),		\
}

/* e.g. FCVTZS <Wd>, <Dn>, #<fbits>.  */
#define QL_FP2FIX		\
{				\
  QLF3(W,S_D,imm_1_32),		\
  QLF3(W,S_S,imm_1_32),		\
  QLF3(X,S_D,imm_1_64),		\
  QLF3(X,S_S,imm_1_64),		\
}

/* e.g. FCVTZS <Wd>, <Hn>, #<fbits>.  */
#define QL_FP2FIX_H			\
{					\
  QLF3 (W, S_H, imm_1_32),		\
  QLF3 (X, S_H, imm_1_64),		\
}

/* e.g. SCVTF <Dd>, <Wn>.  */
#define QL_INT2FP		\
{				\
  QLF2(S_D,W),			\
  QLF2(S_S,W),			\
  QLF2(S_D,X),			\
  QLF2(S_S,X),			\
}

/* e.g. FMOV <Dd>, <Xn>.  */
#define QL_INT2FP_FMOV		\
{				\
  QLF2(S_S,W),			\
  QLF2(S_D,X),			\
}

/* e.g. SCVTF <Hd>, <Wn>.  */
#define QL_INT2FP_H			\
{					\
  QLF2 (S_H, W),			\
  QLF2 (S_H, X),			\
}

/* e.g. FCVTNS <Xd>, <Dn>.  */
#define QL_FP2INT		\
{				\
  QLF2(W,S_D),			\
  QLF2(W,S_S),			\
  QLF2(X,S_D),			\
  QLF2(X,S_S),			\
}

/* e.g. FMOV <Xd>, <Dn>.  */
#define QL_FP2INT_FMOV		\
{				\
  QLF2(W,S_S),			\
  QLF2(X,S_D),			\
}

/* e.g. FCVTNS <Hd>, <Wn>.  */
#define QL_FP2INT_H			\
{					\
  QLF2 (W, S_H),			\
  QLF2 (X, S_H),			\
}

/* e.g. FJCVTZS <Wd>, <Dn>.  */
#define QL_FP2INT_W_D			\
{					\
  QLF2 (W, S_D),			\
}

/* e.g. FMOV <Xd>, <Vn>.D[1].  */
#define QL_XVD1			\
{				\
  QLF2(X,S_D),			\
}

/* e.g. FMOV <Vd>.D[1], <Xn>.  */
#define QL_VD1X			\
{				\
  QLF2(S_D,X),			\
}

/* e.g. EXTR <Xd>, <Xn>, <Xm>, #<lsb>.  */
#define QL_EXTR			\
{				\
  QLF4(W,W,W,imm_0_31),		\
  QLF4(X,X,X,imm_0_63),		\
}

/* e.g. LSL <Wd>, <Wn>, #<uimm>.  */
#define QL_SHIFT		\
{				\
  QLF3(W,W,imm_0_31),		\
  QLF3(X,X,imm_0_63),		\
}

/* e.g. UXTH <Xd>, <Wn>.  */
#define QL_EXT			\
{				\
  QLF2(W,W),			\
  QLF2(X,W),			\
}

/* e.g. UXTW <Xd>, <Wn>.  */
#define QL_EXT_W		\
{				\
  QLF2(X,W),			\
}

/* e.g. SQSHL <V><d>, <V><n>, #<shift>.  */
#define QL_SSHIFT		\
{				\
  QLF3(S_B , S_B , S_B ),	\
  QLF3(S_H , S_H , S_H ),	\
  QLF3(S_S , S_S , S_S ),	\
  QLF3(S_D , S_D , S_D )	\
}

/* e.g. SSHR <V><d>, <V><n>, #<shift>.  */
#define QL_SSHIFT_D		\
{				\
  QLF3(S_D , S_D , S_D )	\
}

/* e.g. UCVTF <Vd>.<T>, <Vn>.<T>, #<fbits>.  */
#define QL_SSHIFT_SD		\
{				\
  QLF3(S_S , S_S , S_S ),	\
  QLF3(S_D , S_D , S_D )	\
}

/* e.g. UCVTF <Vd>.<T>, <Vn>.<T>, #<fbits>.  */
#define QL_SSHIFT_H		\
{				\
  QLF3 (S_H, S_H, S_H)		\
}

/* e.g. SQSHRUN <Vb><d>, <Va><n>, #<shift>.  */
#define QL_SSHIFTN		\
{				\
  QLF3(S_B , S_H , S_B ),	\
  QLF3(S_H , S_S , S_H ),	\
  QLF3(S_S , S_D , S_S ),	\
}

/* e.g. SSHR <Vd>.<T>, <Vn>.<T>, #<shift>.
   The register operand variant qualifiers are deliberately used for the
   immediate operand to ease the operand encoding/decoding and qualifier
   sequence matching.  */
#define QL_VSHIFT		\
{				\
  QLF3(V_8B , V_8B , V_8B ),	\
  QLF3(V_16B, V_16B, V_16B),	\
  QLF3(V_4H , V_4H , V_4H ),	\
  QLF3(V_8H , V_8H , V_8H ),	\
  QLF3(V_2S , V_2S , V_2S ),	\
  QLF3(V_4S , V_4S , V_4S ),	\
  QLF3(V_2D , V_2D , V_2D )	\
}

/* e.g. SCVTF <Vd>.<T>, <Vn>.<T>, #<fbits>.  */
#define QL_VSHIFT_SD		\
{				\
  QLF3(V_2S , V_2S , V_2S ),	\
  QLF3(V_4S , V_4S , V_4S ),	\
  QLF3(V_2D , V_2D , V_2D )	\
}

/* e.g. SCVTF <Vd>.<T>, <Vn>.<T>, #<fbits>.  */
#define QL_VSHIFT_H		\
{				\
  QLF3 (V_4H, V_4H, V_4H),	\
  QLF3 (V_8H, V_8H, V_8H)	\
}

/* e.g. SHRN<Q> <Vd>.<Tb>, <Vn>.<Ta>, #<shift>.  */
#define QL_VSHIFTN		\
{				\
  QLF3(V_8B , V_8H , V_8B ),	\
  QLF3(V_4H , V_4S , V_4H ),	\
  QLF3(V_2S , V_2D , V_2S ),	\
}

/* e.g. SHRN<Q> <Vd>.<Tb>, <Vn>.<Ta>, #<shift>.  */
#define QL_VSHIFTN2		\
{				\
  QLF3(V_16B, V_8H, V_16B),	\
  QLF3(V_8H , V_4S , V_8H ),	\
  QLF3(V_4S , V_2D , V_4S ),	\
}

/* e.g. SSHLL<Q> <Vd>.<Ta>, <Vn>.<Tb>, #<shift>.
   the 3rd qualifier is used to help the encoding.  */
#define QL_VSHIFTL		\
{				\
  QLF3(V_8H , V_8B , V_8B ),	\
  QLF3(V_4S , V_4H , V_4H ),	\
  QLF3(V_2D , V_2S , V_2S ),	\
}

/* e.g. SSHLL<Q> <Vd>.<Ta>, <Vn>.<Tb>, #<shift>.  */
#define QL_VSHIFTL2		\
{				\
  QLF3(V_8H , V_16B, V_16B),	\
  QLF3(V_4S , V_8H , V_8H ),	\
  QLF3(V_2D , V_4S , V_4S ),	\
}

/* e.g. TBL.  */
#define QL_TABLE		\
{				\
  QLF3(V_8B , V_16B, V_8B ),	\
  QLF3(V_16B, V_16B, V_16B),	\
}

/* e.g. SHA1H.  */
#define QL_2SAMES		\
{				\
  QLF2(S_S, S_S),		\
}

/* e.g. ABS <V><d>, <V><n>.  */
#define QL_2SAMED		\
{				\
  QLF2(S_D, S_D),		\
}

/* e.g. CMGT <V><d>, <V><n>, #0.  */
#define QL_SISD_CMP_0		\
{				\
  QLF3(S_D, S_D, NIL),		\
}

/* e.g. FCMEQ <V><d>, <V><n>, #0.  */
#define QL_SISD_FCMP_0		\
{				\
  QLF3(S_S, S_S, NIL),		\
  QLF3(S_D, S_D, NIL),		\
}

/* e.g. FCMEQ <V><d>, <V><n>, #0.  */
#define QL_SISD_FCMP_H_0	\
{				\
  QLF3 (S_H, S_H, NIL),		\
}

/* e.g. FMAXNMP <V><d>, <Vn>.<T>.  */
#define QL_SISD_PAIR		\
{				\
  QLF2(S_S, V_2S),		\
  QLF2(S_D, V_2D),		\
}

/* e.g. FMAXNMP <V><d>, <Vn>.<T>.  */
#define QL_SISD_PAIR_H		\
{				\
  QLF2 (S_H, V_2H),		\
}

/* e.g. ADDP <V><d>, <Vn>.<T>.  */
#define QL_SISD_PAIR_D		\
{				\
  QLF2(S_D, V_2D),		\
}

/* e.g. DUP <V><d>, <Vn>.<T>[<index>].  */
#define QL_S_2SAME		\
{				\
  QLF2(S_B, S_B),		\
  QLF2(S_H, S_H),		\
  QLF2(S_S, S_S),		\
  QLF2(S_D, S_D),		\
}

/* e.g. FCVTNS <V><d>, <V><n>.  */
#define QL_S_2SAMESD		\
{				\
  QLF2(S_S, S_S),		\
  QLF2(S_D, S_D),		\
}

/* e.g. FCVTNS <V><d>, <V><n>.  */
#define QL_S_2SAMEH		\
{				\
  QLF2 (S_H, S_H),		\
}

/* e.g. SQXTN <Vb><d>, <Va><n>.  */
#define QL_SISD_NARROW		\
{				\
  QLF2(S_B, S_H),		\
  QLF2(S_H, S_S),		\
  QLF2(S_S, S_D),		\
}

/* e.g. FCVTXN <Vb><d>, <Va><n>.  */
#define QL_SISD_NARROW_S	\
{				\
  QLF2(S_S, S_D),		\
}

/* e.g. FCVT.  */
#define QL_FCVT			\
{				\
  QLF2(S_S, S_H),		\
  QLF2(S_S, S_D),		\
  QLF2(S_D, S_H),		\
  QLF2(S_D, S_S),		\
  QLF2(S_H, S_S),		\
  QLF2(S_H, S_D),		\
}

/* FMOV <Dd>, <Dn>.  */
#define QL_FP2			\
{				\
  QLF2(S_S, S_S),		\
  QLF2(S_D, S_D),		\
}

/* FMOV <Hd>, <Hn>.  */
#define QL_FP2_H		\
{				\
  QLF2 (S_H, S_H),		\
}

/* e.g. SQADD <V><d>, <V><n>, <V><m>.  */
#define QL_S_3SAME		\
{				\
  QLF3(S_B, S_B, S_B),		\
  QLF3(S_H, S_H, S_H),		\
  QLF3(S_S, S_S, S_S),		\
  QLF3(S_D, S_D, S_D),		\
}

/* e.g. CMGE <V><d>, <V><n>, <V><m>.  */
#define QL_S_3SAMED		\
{				\
  QLF3(S_D, S_D, S_D),		\
}

/* e.g. SQDMULH <V><d>, <V><n>, <V><m>.  */
#define QL_SISD_HS		\
{				\
  QLF3(S_H, S_H, S_H),		\
  QLF3(S_S, S_S, S_S),		\
}

/* e.g. SQDMLAL <Va><d>, <Vb><n>, <Vb><m>.  */
#define QL_SISDL_HS		\
{				\
  QLF3(S_S, S_H, S_H),		\
  QLF3(S_D, S_S, S_S),		\
}

/* FMUL <Sd>, <Sn>, <Sm>.  */
#define QL_FP3			\
{				\
  QLF3(S_S, S_S, S_S),		\
  QLF3(S_D, S_D, S_D),		\
}

/* FMUL <Hd>, <Hn>, <Hm>.  */
#define QL_FP3_H		\
{				\
  QLF3 (S_H, S_H, S_H),		\
}

/* FMADD <Dd>, <Dn>, <Dm>, <Da>.  */
#define QL_FP4			\
{				\
  QLF4(S_S, S_S, S_S, S_S),	\
  QLF4(S_D, S_D, S_D, S_D),	\
}

/* FMADD <Hd>, <Hn>, <Hm>, <Ha>.  */
#define QL_FP4_H		\
{				\
  QLF4 (S_H, S_H, S_H, S_H),	\
}

/* e.g. FCMP <Dn>, #0.0.  */
#define QL_DST_SD			\
{				\
  QLF2(S_S, NIL),		\
  QLF2(S_D, NIL),		\
}

/* e.g. FCMP <Hn>, #0.0.  */
#define QL_DST_H		\
{				\
  QLF2 (S_H, NIL),		\
}

/* FCSEL <Sd>, <Sn>, <Sm>, <cond>.  */
#define QL_FP_COND		\
{				\
  QLF4(S_S, S_S, S_S, NIL),	\
  QLF4(S_D, S_D, S_D, NIL),	\
}

/* FCSEL <Hd>, <Hn>, <Hm>, <cond>.  */
#define QL_FP_COND_H		\
{				\
  QLF4 (S_H, S_H, S_H, NIL),	\
}

/* e.g. CCMN <Xn>, <Xm>, #<nzcv>, <cond>.  */
#define QL_CCMP			\
{				\
  QLF4(W, W, NIL, NIL),		\
  QLF4(X, X, NIL, NIL),		\
}

/* e.g. CCMN <Xn>, #<imm>, #<nzcv>, <cond>,  */
#define QL_CCMP_IMM		\
{				\
  QLF4(W, NIL, NIL, NIL),	\
  QLF4(X, NIL, NIL, NIL),	\
}

/* e.g. FCCMP <Sn>, <Sm>, #<nzcv>, <cond>.  */
#define QL_FCCMP		\
{				\
  QLF4(S_S, S_S, NIL, NIL),	\
  QLF4(S_D, S_D, NIL, NIL),	\
}

/* e.g. FCCMP <Sn>, <Sm>, #<nzcv>, <cond>.  */
#define QL_FCCMP_H		\
{				\
  QLF4 (S_H, S_H, NIL, NIL),	\
}

/* e.g. DUP <Vd>.<T>, <Vn>.<Ts>[<index>].  */
#define QL_DUP_VX		\
{				\
  QLF2(V_8B , S_B ),		\
  QLF2(V_16B, S_B ),		\
  QLF2(V_4H , S_H ),		\
  QLF2(V_8H , S_H ),		\
  QLF2(V_2S , S_S ),		\
  QLF2(V_4S , S_S ),		\
  QLF2(V_2D , S_D ),		\
}

/* e.g. DUP <Vd>.<T>, <Wn>.  */
#define QL_DUP_VR		\
{				\
  QLF2(V_8B , W ),		\
  QLF2(V_16B, W ),		\
  QLF2(V_4H , W ),		\
  QLF2(V_8H , W ),		\
  QLF2(V_2S , W ),		\
  QLF2(V_4S , W ),		\
  QLF2(V_2D , X ),		\
}

/* e.g. INS <Vd>.<Ts>[<index>], <Wn>.  */
#define QL_INS_XR		\
{				\
  QLF2(S_H , W ),		\
  QLF2(S_S , W ),		\
  QLF2(S_D , X ),		\
  QLF2(S_B , W ),		\
}

/* e.g. SMOV <Wd>, <Vn>.<Ts>[<index>].  */
#define QL_SMOV			\
{				\
  QLF2(W , S_H),		\
  QLF2(X , S_H),		\
  QLF2(X , S_S),		\
  QLF2(W , S_B),		\
  QLF2(X , S_B),		\
}

/* e.g. UMOV <Wd>, <Vn>.<Ts>[<index>].  */
#define QL_UMOV			\
{				\
  QLF2(W , S_H),		\
  QLF2(W , S_S),		\
  QLF2(X , S_D),		\
  QLF2(W , S_B),		\
}

/* e.g. MOV <Wd>, <Vn>.<Ts>[<index>].  */
#define QL_MOV			\
{				\
  QLF2(W , S_S),		\
  QLF2(X , S_D),		\
}

/* e.g. SUQADD <Vd>.<T>, <Vn>.<T>.  */
#define QL_V2SAME		\
{				\
  QLF2(V_8B , V_8B ),		\
  QLF2(V_16B, V_16B),		\
  QLF2(V_4H , V_4H ),		\
  QLF2(V_8H , V_8H ),		\
  QLF2(V_2S , V_2S ),		\
  QLF2(V_4S , V_4S ),		\
  QLF2(V_2D , V_2D ),		\
}

/* e.g. URSQRTE <Vd>.<T>, <Vn>.<T>.  */
#define QL_V2SAMES		\
{				\
  QLF2(V_2S , V_2S ),		\
  QLF2(V_4S , V_4S ),		\
}

/* e.g. REV32 <Vd>.<T>, <Vn>.<T>.  */
#define QL_V2SAMEBH		\
{				\
  QLF2(V_8B , V_8B ),		\
  QLF2(V_16B, V_16B),		\
  QLF2(V_4H , V_4H ),		\
  QLF2(V_8H , V_8H ),		\
}

/* e.g. FRINTN <Vd>.<T>, <Vn>.<T>.  */
#define QL_V2SAMESD		\
{				\
  QLF2(V_2S , V_2S ),		\
  QLF2(V_4S , V_4S ),		\
  QLF2(V_2D , V_2D ),		\
}

/* e.g. REV64 <Vd>.<T>, <Vn>.<T>.  */
#define QL_V2SAMEBHS		\
{				\
  QLF2(V_8B , V_8B ),		\
  QLF2(V_16B, V_16B),		\
  QLF2(V_4H , V_4H ),		\
  QLF2(V_8H , V_8H ),		\
  QLF2(V_2S , V_2S ),		\
  QLF2(V_4S , V_4S ),		\
}

/* e.g. FCMGT <Vd>.<T>, <Vd>.<T>>, #0.0.  */
#define QL_V2SAMEH		\
{				\
  QLF2 (V_4H, V_4H),		\
  QLF2 (V_8H, V_8H),		\
}

/* e.g. REV16 <Vd>.<T>, <Vn>.<T>.  */
#define QL_V2SAMEB		\
{				\
  QLF2(V_8B , V_8B ),		\
  QLF2(V_16B, V_16B),		\
}

/* e.g. SADDLP <Vd>.<Ta>, <Vn>.<Tb>.  */
#define QL_V2PAIRWISELONGBHS		\
{				\
  QLF2(V_4H , V_8B ),		\
  QLF2(V_8H , V_16B),		\
  QLF2(V_2S , V_4H ),		\
  QLF2(V_4S , V_8H ),		\
  QLF2(V_1D , V_2S ),		\
  QLF2(V_2D , V_4S ),		\
}

/* e.g. SHLL<Q> <Vd>.<Ta>, <Vn>.<Tb>, #<shift>.  */
#define QL_V2LONGBHS		\
{				\
  QLF2(V_8H , V_8B ),		\
  QLF2(V_4S , V_4H ),		\
  QLF2(V_2D , V_2S ),		\
}

/* e.g. SHLL<Q> <Vd>.<Ta>, <Vn>.<Tb>, #<shift>.  */
#define QL_V2LONGBHS2		\
{				\
  QLF2(V_8H , V_16B),		\
  QLF2(V_4S , V_8H ),		\
  QLF2(V_2D , V_4S ),		\
}

/* */
#define QL_V3SAME		\
{				\
  QLF3(V_8B , V_8B , V_8B ),	\
  QLF3(V_16B, V_16B, V_16B),	\
  QLF3(V_4H , V_4H , V_4H ),	\
  QLF3(V_8H , V_8H , V_8H ),	\
  QLF3(V_2S , V_2S , V_2S ),	\
  QLF3(V_4S , V_4S , V_4S ),	\
  QLF3(V_2D , V_2D , V_2D )	\
}

/* e.g. SHADD.  */
#define QL_V3SAMEBHS		\
{				\
  QLF3(V_8B , V_8B , V_8B ),	\
  QLF3(V_16B, V_16B, V_16B),	\
  QLF3(V_4H , V_4H , V_4H ),	\
  QLF3(V_8H , V_8H , V_8H ),	\
  QLF3(V_2S , V_2S , V_2S ),	\
  QLF3(V_4S , V_4S , V_4S ),	\
}

/* e.g. FCVTXN<Q> <Vd>.<Tb>, <Vn>.<Ta>.  */
#define QL_V2NARRS		\
{				\
  QLF2(V_2S , V_2D ),		\
}

/* e.g. FCVTXN<Q> <Vd>.<Tb>, <Vn>.<Ta>.  */
#define QL_V2NARRS2		\
{				\
  QLF2(V_4S , V_2D ),		\
}

/* e.g. FCVTN<Q> <Vd>.<Tb>, <Vn>.<Ta>.  */
#define QL_V2NARRHS		\
{				\
  QLF2(V_4H , V_4S ),		\
  QLF2(V_2S , V_2D ),		\
}

/* e.g. FCVTN<Q> <Vd>.<Tb>, <Vn>.<Ta>.  */
#define QL_V2NARRHS2		\
{				\
  QLF2(V_8H , V_4S ),		\
  QLF2(V_4S , V_2D ),		\
}

/* e.g. FCVTL<Q> <Vd>.<Ta>, <Vn>.<Tb>.  */
#define QL_V2LONGHS		\
{				\
  QLF2(V_4S , V_4H ),		\
  QLF2(V_2D , V_2S ),		\
}

/* e.g. FCVTL<Q> <Vd>.<Ta>, <Vn>.<Tb>.  */
#define QL_V2LONGHS2		\
{				\
  QLF2(V_4S , V_8H ),		\
  QLF2(V_2D , V_4S ),		\
}

/* e.g. XTN<Q> <Vd>.<Tb>, <Vn>.<Ta>.  */
#define QL_V2NARRBHS		\
{				\
  QLF2(V_8B , V_8H ),		\
  QLF2(V_4H , V_4S ),		\
  QLF2(V_2S , V_2D ),		\
}

/* e.g. XTN<Q> <Vd>.<Tb>, <Vn>.<Ta>.  */
#define QL_V2NARRBHS2		\
{				\
  QLF2(V_16B, V_8H ),		\
  QLF2(V_8H , V_4S ),		\
  QLF2(V_4S , V_2D ),		\
}

/* e.g. ORR.  */
#define QL_V2SAMEB		\
{				\
  QLF2(V_8B , V_8B ),		\
  QLF2(V_16B, V_16B),		\
}

/* e.g. AESE.  */
#define QL_V2SAME16B		\
{				\
  QLF2(V_16B, V_16B),		\
}

/* e.g. SHA1SU1.  */
#define QL_V2SAME4S		\
{				\
  QLF2(V_4S, V_4S),		\
}

/* e.g. SHA1SU0.  */
#define QL_V3SAME4S		\
{				\
  QLF3(V_4S, V_4S, V_4S),	\
}

/* e.g. SHADD.  */
#define QL_V3SAMEB		\
{				\
  QLF3(V_8B , V_8B , V_8B ),	\
  QLF3(V_16B, V_16B, V_16B),	\
}

/* e.g. EXT <Vd>.<T>, <Vn>.<T>, <Vm>.<T>, #<index>.  */
#define QL_VEXT			\
{					\
  QLF4(V_8B , V_8B , V_8B , imm_0_7),	\
  QLF4(V_16B, V_16B, V_16B, imm_0_15),	\
}

/* e.g. .  */
#define QL_V3SAMEHS		\
{				\
  QLF3(V_4H , V_4H , V_4H ),	\
  QLF3(V_8H , V_8H , V_8H ),	\
  QLF3(V_2S , V_2S , V_2S ),	\
  QLF3(V_4S , V_4S , V_4S ),	\
}

/* */
#define QL_V3SAMESD		\
{				\
  QLF3(V_2S , V_2S , V_2S ),	\
  QLF3(V_4S , V_4S , V_4S ),	\
  QLF3(V_2D , V_2D , V_2D )	\
}

/* e.g. FCMLA <Vd>.<T>, <Vn>.<T>, <Vm>.<T>, #<rotate>.  */
#define QL_V3SAMEHSD_ROT	\
{				\
  QLF4 (V_4H, V_4H, V_4H, NIL),	\
  QLF4 (V_8H, V_8H, V_8H, NIL),	\
  QLF4 (V_2S, V_2S, V_2S, NIL),	\
  QLF4 (V_4S, V_4S, V_4S, NIL),	\
  QLF4 (V_2D, V_2D, V_2D, NIL),	\
}

/* e.g. FMAXNM <Vd>.<T>, <Vn>.<T>, <Vm>.<T>.  */
#define QL_V3SAMEH		\
{				\
  QLF3 (V_4H , V_4H , V_4H ),	\
  QLF3 (V_8H , V_8H , V_8H ),	\
}

/* e.g. SQDMLAL<Q> <Vd>.<Ta>, <Vn>.<Tb>, <Vm>.<Tb>.  */
#define QL_V3LONGHS		\
{				\
  QLF3(V_4S , V_4H , V_4H ),	\
  QLF3(V_2D , V_2S , V_2S ),	\
}

/* e.g. SQDMLAL<Q> <Vd>.<Ta>, <Vn>.<Tb>, <Vm>.<Tb>.  */
#define QL_V3LONGHS2		\
{				\
  QLF3(V_4S , V_8H , V_8H ),	\
  QLF3(V_2D , V_4S , V_4S ),	\
}

/* e.g. SADDL<Q> <Vd>.<Ta>, <Vn>.<Tb>, <Vm>.<Tb>.  */
#define QL_V3LONGBHS		\
{				\
  QLF3(V_8H , V_8B , V_8B ),	\
  QLF3(V_4S , V_4H , V_4H ),	\
  QLF3(V_2D , V_2S , V_2S ),	\
}

/* e.g. SADDL<Q> <Vd>.<Ta>, <Vn>.<Tb>, <Vm>.<Tb>.  */
#define QL_V3LONGBHS2		\
{				\
  QLF3(V_8H , V_16B , V_16B ),	\
  QLF3(V_4S , V_8H , V_8H ),	\
  QLF3(V_2D , V_4S , V_4S ),	\
}

/* e.g. SADDW<Q> <Vd>.<Ta>, <Vn>.<Ta>, <Vm>.<Tb>.  */
#define QL_V3WIDEBHS		\
{				\
  QLF3(V_8H , V_8H , V_8B ),	\
  QLF3(V_4S , V_4S , V_4H ),	\
  QLF3(V_2D , V_2D , V_2S ),	\
}

/* e.g. SADDW<Q> <Vd>.<Ta>, <Vn>.<Ta>, <Vm>.<Tb>.  */
#define QL_V3WIDEBHS2		\
{				\
  QLF3(V_8H , V_8H , V_16B ),	\
  QLF3(V_4S , V_4S , V_8H ),	\
  QLF3(V_2D , V_2D , V_4S ),	\
}

/* e.g. ADDHN<Q> <Vd>.<Tb>, <Vn>.<Ta>, <Vm>.<Ta>.  */
#define QL_V3NARRBHS		\
{				\
  QLF3(V_8B , V_8H , V_8H ),	\
  QLF3(V_4H , V_4S , V_4S ),	\
  QLF3(V_2S , V_2D , V_2D ),	\
}

/* e.g. ADDHN<Q> <Vd>.<Tb>, <Vn>.<Ta>, <Vm>.<Ta>.  */
#define QL_V3NARRBHS2		\
{				\
  QLF3(V_16B , V_8H , V_8H ),	\
  QLF3(V_8H , V_4S , V_4S ),	\
  QLF3(V_4S , V_2D , V_2D ),	\
}

/* e.g. PMULL.  */
#define QL_V3LONGB		\
{				\
  QLF3(V_8H , V_8B , V_8B ),	\
}

/* e.g. PMULL crypto.  */
#define QL_V3LONGD		\
{				\
  QLF3(V_1Q , V_1D , V_1D ),	\
}

/* e.g. PMULL2.  */
#define QL_V3LONGB2		\
{				\
  QLF3(V_8H , V_16B, V_16B),	\
}

/* e.g. PMULL2 crypto.  */
#define QL_V3LONGD2		\
{				\
  QLF3(V_1Q , V_2D , V_2D ),	\
}

/* e.g. SHA1C.  */
#define QL_SHAUPT		\
{				\
  QLF3(S_Q, S_S, V_4S),		\
}

/* e.g. SHA256H2.  */
#define QL_SHA256UPT		\
{				\
  QLF3(S_Q, S_Q, V_4S),		\
}

/* e.g. LDXRB <Wt>, [<Xn|SP>{,#0}].  */
#define QL_W1_LDST_EXC		\
{				\
  QLF2(W, NIL),			\
}

/* e.g. LDXR <Xt>, [<Xn|SP>{,#0}].  */
#define QL_R1NIL		\
{				\
  QLF2(W, NIL),			\
  QLF2(X, NIL),			\
}

/* e.g. STXRB <Ws>, <Wt>, [<Xn|SP>{,#0}].  */
#define QL_W2_LDST_EXC		\
{				\
  QLF3(W, W, NIL),		\
}

/* e.g. STXR <Ws>, <Xt>, [<Xn|SP>{,#0}].  */
#define QL_R2_LDST_EXC		\
{				\
  QLF3(W, W, NIL),		\
  QLF3(W, X, NIL),		\
}

/* e.g. ST64B <Xs>, <Xt>, [<Xn|SP>].  */
#define QL_X2NIL		\
{				\
  QLF3(X, X, NIL),			\
}

/* e.g. LDRAA <Xt>, [<Xn|SP>{,#imm}].  */
#define QL_X1NIL		\
{				\
  QLF2(X, NIL),			\
}

/* e.g. LDXP <Xt1>, <Xt2>, [<Xn|SP>{,#0}].  */
#define QL_R2NIL		\
{				\
  QLF3(W, W, NIL),		\
  QLF3(X, X, NIL),		\
}

/* e.g. CASP <Xt1>, <Xt1+1>, <Xt2>, <Xt2+1>, [<Xn|SP>{,#0}].  */
#define QL_R4NIL		\
{				\
  QLF5(W, W, W, W, NIL),	\
  QLF5(X, X, X, X, NIL),	\
}

/* e.g. STXP <Ws>, <Xt1>, <Xt2>, [<Xn|SP>{,#0}].  */
#define QL_R3_LDST_EXC		\
{				\
  QLF4(W, W, W, NIL),		\
  QLF4(W, X, X, NIL),		\
}

/* e.g. STR <Qt>, [<Xn|SP>, <R><m>{, <extend> {<amount>}}].  */
#define QL_LDST_FP		\
{				\
  QLF2(S_B, S_B),		\
  QLF2(S_H, S_H),		\
  QLF2(S_S, S_S),		\
  QLF2(S_D, S_D),		\
  QLF2(S_Q, S_Q),		\
}

/* e.g. STR <Xt>, [<Xn|SP>, <R><m>{, <extend> {<amount>}}].  */
#define QL_LDST_R		\
{				\
  QLF2(W, S_S),			\
  QLF2(X, S_D),			\
}

/* e.g. STRB <Wt>, [<Xn|SP>, <R><m>{, <extend> {<amount>}}].  */
#define QL_LDST_W8		\
{				\
  QLF2(W, S_B),			\
}

/* e.g. LDRSB <Wt>, [<Xn|SP>, <R><m>{, <extend> {<amount>}}].  */
#define QL_LDST_R8		\
{				\
  QLF2(W, S_B),			\
  QLF2(X, S_B),			\
}

/* e.g. STRH <Wt>, [<Xn|SP>, <R><m>{, <extend> {<amount>}}].  */
#define QL_LDST_W16		\
{				\
  QLF2(W, S_H),			\
}

/* e.g. LDRSW <Xt>, [<Xn|SP>, <R><m>{, <extend> {<amount>}}].  */
#define QL_LDST_X32		\
{				\
  QLF2(X, S_S),			\
}

/* e.g. LDRSH <Wt>, [<Xn|SP>, <R><m>{, <extend> {<amount>}}].  */
#define QL_LDST_R16		\
{				\
  QLF2(W, S_H),			\
  QLF2(X, S_H),			\
}

/* e.g. PRFM <prfop>, [<Xn|SP>, <R><m>{, <extend> {<amount>}}].  */
#define QL_LDST_PRFM		\
{				\
  QLF2(NIL, S_D),		\
}

/* e.g. LDG <Xt>, [<Xn|SP>{, #<simm>}].  */
#define QL_LDG			\
{				\
  QLF2(X, imm_tag),		\
}

/* e.g. LDPSW <Xt1>, <Xt2>, [<Xn|SP>{, #<imm>}].  */
#define QL_LDST_PAIR_X32	\
{				\
  QLF3(X, X, S_S),		\
}

/* e.g. STGP <Xt1>, <Xt2>, [<Xn|SP>{, #<imm>}].  */
#define QL_STGP			\
{				\
  QLF3(X, X, imm_tag),		\
}

/* e.g. STP <Wt1>, <Wt2>, [<Xn|SP>, #<imm>]!.  */
#define QL_LDST_PAIR_R		\
{				\
  QLF3(W, W, S_S),		\
  QLF3(X, X, S_D),		\
}

/* e.g. STNP <Qt1>, <Qt2>, [<Xn|SP>{, #<imm>}].  */
#define QL_LDST_PAIR_FP		\
{				\
  QLF3(S_S, S_S, S_S),		\
  QLF3(S_D, S_D, S_D),		\
  QLF3(S_Q, S_Q, S_Q),		\
}

/* e.g. LD3 {<Vt>.<T>, <Vt2>.<T>, <Vt3>.<T>}, [<Xn|SP>].  */
#define QL_SIMD_LDST		\
{				\
  QLF2(V_8B, NIL),		\
  QLF2(V_16B, NIL),		\
  QLF2(V_4H, NIL),		\
  QLF2(V_8H, NIL),		\
  QLF2(V_2S, NIL),		\
  QLF2(V_4S, NIL),		\
  QLF2(V_2D, NIL),		\
}

/* e.g. LD1 {<Vt>.<T>, <Vt2>.<T>, <Vt3>.<T>}, [<Xn|SP>].  */
#define QL_SIMD_LDST_ANY	\
{				\
  QLF2(V_8B, NIL),		\
  QLF2(V_16B, NIL),		\
  QLF2(V_4H, NIL),		\
  QLF2(V_8H, NIL),		\
  QLF2(V_2S, NIL),		\
  QLF2(V_4S, NIL),		\
  QLF2(V_1D, NIL),		\
  QLF2(V_2D, NIL),		\
}

/* e.g. LD4 {<Vt>.<T>, <Vt2a>.<T>, <Vt3a>.<T>, <Vt4a>.<T>}[<index>], [<Xn|SP>].  */
#define QL_SIMD_LDSTONE		\
{				\
  QLF2(S_B, NIL),		\
  QLF2(S_H, NIL),		\
  QLF2(S_S, NIL),		\
  QLF2(S_D, NIL),		\
}

/* e.g. ADDV <V><d>, <Vn>.<T>.  */
#define QL_XLANES		\
{				\
  QLF2(S_B, V_8B),		\
  QLF2(S_B, V_16B),		\
  QLF2(S_H, V_4H),		\
  QLF2(S_H, V_8H),		\
  QLF2(S_S, V_4S),		\
}

/* e.g. FMINV <V><d>, <Vn>.<T>.  */
#define QL_XLANES_FP		\
{				\
  QLF2(S_S, V_4S),		\
}

/* e.g. FMINV <V><d>, <Vn>.<T>.  */
#define QL_XLANES_FP_H		\
{				\
  QLF2 (S_H, V_4H),		\
  QLF2 (S_H, V_8H),		\
}

/* e.g. SADDLV <V><d>, <Vn>.<T>.  */
#define QL_XLANES_L		\
{				\
  QLF2(S_H, V_8B),		\
  QLF2(S_H, V_16B),		\
  QLF2(S_S, V_4H),		\
  QLF2(S_S, V_8H),		\
  QLF2(S_D, V_4S),		\
}

/* e.g. MUL <Vd>.<T>, <Vn>.<T>, <Vm>.<Ts>[<index>].  */
#define QL_ELEMENT		\
{				\
  QLF3(V_4H, V_4H, S_H),	\
  QLF3(V_8H, V_8H, S_H),	\
  QLF3(V_2S, V_2S, S_S),	\
  QLF3(V_4S, V_4S, S_S),	\
}

/* e.g. SMLAL <Vd>.<Ta>, <Vn>.<Tb>, <Vm>.<Ts>[<index>].  */
#define QL_ELEMENT_L		\
{				\
  QLF3(V_4S, V_4H, S_H),	\
  QLF3(V_2D, V_2S, S_S),	\
}

/* e.g. SMLAL2 <Vd>.<Ta>, <Vn>.<Tb>, <Vm>.<Ts>[<index>].  */
#define QL_ELEMENT_L2		\
{				\
  QLF3(V_4S, V_8H, S_H),	\
  QLF3(V_2D, V_4S, S_S),	\
}

/* e.g. FMLA <V><d>, <V><n>, <Vm>.<Ts>[<index>].  */
#define QL_ELEMENT_FP		\
{				\
  QLF3(V_2S, V_2S, S_S),	\
  QLF3(V_4S, V_4S, S_S),	\
  QLF3(V_2D, V_2D, S_D),	\
}

/* e.g. FMLA <V><d>, <V><n>, <Vm>.<Ts>[<index>].  */
#define QL_ELEMENT_FP_H		\
{				\
  QLF3 (V_4H, V_4H, S_H),	\
  QLF3 (V_8H, V_8H, S_H),	\
}

/* e.g. FCMLA <Vd>.<T>, <Vn>.<T>, <Vm>.<Ts>[<index>], #<rotate>.  */
#define QL_ELEMENT_ROT		\
{				\
  QLF4 (V_4H, V_4H, S_H, NIL),	\
  QLF4 (V_8H, V_8H, S_H, NIL),	\
  QLF4 (V_4S, V_4S, S_S, NIL),	\
}

/* e.g. MOVI <Vd>.4S, #<imm8> {, LSL #<amount>}.  */
#define QL_SIMD_IMM_S0W		\
{				\
  QLF2(V_2S, LSL),		\
  QLF2(V_4S, LSL),		\
}

/* e.g. MOVI <Vd>.4S, #<imm8>, MSL #<amount>.  */
#define QL_SIMD_IMM_S1W		\
{				\
  QLF2(V_2S, MSL),		\
  QLF2(V_4S, MSL),		\
}

/* e.g. MOVI <Vd>.4H, #<imm8> {, LSL #<amount>}.  */
#define QL_SIMD_IMM_S0H		\
{				\
  QLF2(V_4H, LSL),		\
  QLF2(V_8H, LSL),		\
}

/* e.g. FMOV <Vd>.<T>, #<imm>.  */
#define QL_SIMD_IMM_S		\
{				\
  QLF2(V_2S, NIL),		\
  QLF2(V_4S, NIL),		\
}

/* e.g. MOVI <Vd>.8B, #<imm8> {, LSL #<amount>}.  */
#define QL_SIMD_IMM_B		\
{				\
  QLF2(V_8B, LSL),		\
  QLF2(V_16B, LSL),		\
}
/* e.g. MOVI <Dd>, #<imm>.  */
#define QL_SIMD_IMM_D		\
{				\
  QLF2(S_D, NIL),		\
}

/* e.g. FMOV <Vd>.<T>, #<imm>.  */
#define QL_SIMD_IMM_H		\
{				\
  QLF2 (V_4H, NIL),		\
  QLF2 (V_8H, NIL),		\
}

/* e.g. MOVI <Vd>.2D, #<imm>.  */
#define QL_SIMD_IMM_V2D		\
{				\
  QLF2(V_2D, NIL),		\
}

/* The naming convention for SVE macros is:

   OP_SVE_<operands>[_<sizes>]*

   <operands> contains one character per operand, using the following scheme:

   - U: the operand is unqualified (NIL).

   - [BHSDQ]: the operand has a S_[BHSDQ] qualifier and the choice of
     qualifier is the same for all variants.  This is used for:

     - .[BHSDQ] suffixes on an SVE vector or predicate
     - .[BHSDQ] suffixes on an SME ZA operand
     - vector registers and scalar FPRs of the form [BHSDQ]<number>

   - [WX]: the operand has a [WX] qualifier and the choice of qualifier
     is the same for all variants.

   - [ZM]: the operand has a /[ZM] suffix and the choice of suffix
     is the same for all variants.

   - V: the operand has a S_[BHSD] qualifier and the choice of qualifier
     is not the same for all variants.  This is used for the same kinds
     of operand as [BHSDQ] above.

   - R: the operand has a [WX] qualifier and the choice of qualifier is
     not the same for all variants.

   - P: the operand has a /[ZM] suffix and the choice of suffix is not
     the same for all variants.

   The _<sizes>, if present, give the subset of [BHSD] that are accepted
   by the V entries in <operands>.  */
#define OP_SVE_B                                        \
{                                                       \
  QLF1(S_B),                                            \
}
#define OP_SVE_BB                                       \
{                                                       \
  QLF2(S_B,S_B),                                        \
}
#define OP_SVE_BBU                                      \
{                                                       \
  QLF3(S_B,S_B,NIL),                                \
}
#define OP_SVE_BBB                                      \
{                                                       \
  QLF3(S_B,S_B,S_B),                                \
}
#define OP_SVE_BBBU                                     \
{                                                       \
  QLF4(S_B,S_B,S_B,NIL),                                \
}
#define OP_SVE_BMB                                      \
{                                                       \
  QLF3(S_B,P_M,S_B),                                    \
}
#define OP_SVE_BPB                                      \
{                                                       \
  QLF3(S_B,P_Z,S_B),                                    \
  QLF3(S_B,P_M,S_B),                                    \
}
#define OP_SVE_BUB                                      \
{                                                       \
  QLF3(S_B,NIL,S_B),                                    \
}
#define OP_SVE_BUBB                                     \
{                                                       \
  QLF4(S_B,NIL,S_B,S_B),                                \
}
#define OP_SVE_BUU                                      \
{                                                       \
  QLF3(S_B,NIL,NIL),                                    \
}
#define OP_SVE_BZ                                       \
{                                                       \
  QLF2(S_B,P_Z),                                        \
}
#define OP_SVE_BZB                                      \
{                                                       \
  QLF3(S_B,P_Z,S_B),                                    \
}
#define OP_SVE_NN_BHSD                                  \
{                                                       \
  QLF3(NIL,NIL,S_B),                                    \
  QLF3(NIL,NIL,S_H),                                    \
  QLF3(NIL,NIL,S_S),                                    \
  QLF3(NIL,NIL,S_D)                                     \
}
#define OP_SVE_BZBB                                     \
{                                                       \
  QLF4(S_B,P_Z,S_B,S_B),                                \
}
#define OP_SVE_BZU                                      \
{                                                       \
  QLF3(S_B,P_Z,NIL),                                    \
}
#define OP_SVE_DD                                       \
{                                                       \
  QLF2(S_D,S_D),                                        \
}
#define OP_SVE_DDD                                      \
{                                                       \
  QLF3(S_D,S_D,S_D),                                    \
}
#define OP_SVE_DHH                                      \
{                                                       \
  QLF3(S_D,S_H,S_H),                                    \
}
#define OP_SVE_DMMD                                     \
{                                                       \
  QLF4(S_D,P_M,P_M,S_D),                                \
}
#define OP_SVE_DMMDD                                    \
{                                                       \
  QLF5(S_D,P_M,P_M,S_D,S_D)                             \
}
#define OP_SVE_DMMHH                                    \
{                                                       \
  QLF5(S_D,P_M,P_M,S_H,S_H)                             \
}
#define OP_SVE_DDDD                                     \
{                                                       \
  QLF4(S_D,S_D,S_D,S_D),                                \
}
#define OP_SVE_DMD                                      \
{                                                       \
  QLF3(S_D,P_M,S_D),                                    \
}
#define OP_SVE_DMH                                      \
{                                                       \
  QLF3(S_D,P_M,S_H),                                    \
}
#define OP_SVE_DMS                                      \
{                                                       \
  QLF3(S_D,P_M,S_S),                                    \
}
#define OP_SVE_DU                                       \
{                                                       \
  QLF2(S_D,NIL),                                        \
}
#define OP_SVE_DUD                                      \
{                                                       \
  QLF3(S_D,NIL,S_D),                                    \
}
#define OP_SVE_DUU                                      \
{                                                       \
  QLF3(S_D,NIL,NIL),                                    \
}
#define OP_SVE_DUV_BHS                                  \
{                                                       \
  QLF3(S_D,NIL,S_B),                                    \
  QLF3(S_D,NIL,S_H),                                    \
  QLF3(S_D,NIL,S_S),                                    \
}
#define OP_SVE_DUV_BHSD                                 \
{                                                       \
  QLF3(S_D,NIL,S_B),                                    \
  QLF3(S_D,NIL,S_H),                                    \
  QLF3(S_D,NIL,S_S),                                    \
  QLF3(S_D,NIL,S_D),                                    \
}
#define OP_SVE_DZD                                      \
{                                                       \
  QLF3(S_D,P_Z,S_D),                                    \
}
#define OP_SVE_DZU                                      \
{                                                       \
  QLF3(S_D,P_Z,NIL),                                    \
}
#define OP_SVE_HB                                       \
{                                                       \
  QLF2(S_H,S_B),                                        \
}
#define OP_SVE_HHH                                      \
{                                                       \
  QLF3(S_H,S_H,S_H),                                    \
}
#define OP_SVE_HHHU                                     \
{                                                       \
  QLF4(S_H,S_H,S_H,NIL),                                \
}
#define OP_SVE_HMH                                      \
{                                                       \
  QLF3(S_H,P_M,S_H),                                    \
}
#define OP_SVE_HMD                                      \
{                                                       \
  QLF3(S_H,P_M,S_D),                                    \
}
#define OP_SVE_HMS                                      \
{                                                       \
  QLF3(S_H,P_M,S_S),                                    \
}
#define OP_SVE_HS                                       \
{                                                       \
  QLF2(S_H,S_S),                                        \
}
#define OP_SVE_HSU                                      \
{                                                       \
  QLF3(S_H,S_S,NIL),                                    \
}
#define OP_SVE_HU                                       \
{                                                       \
  QLF2(S_H,NIL),                                        \
}
#define OP_SVE_HUU                                      \
{                                                       \
  QLF3(S_H,NIL,NIL),                                    \
}
#define OP_SVE_HZU                                      \
{                                                       \
  QLF3(S_H,P_Z,NIL),                                    \
}
#define OP_SVE_QMQ                                      \
{                                                       \
  QLF3(S_Q,P_M,S_Q),                                    \
}
#define OP_SVE_QQ                                       \
{                                                       \
  QLF2(S_Q,S_Q),                                        \
}
#define OP_SVE_QQQ                                      \
{                                                       \
  QLF3(S_Q,S_Q,S_Q),                                    \
}
#define OP_SVE_QUU                                      \
{                                                       \
  QLF3(S_Q,NIL,NIL),                                    \
}
#define OP_SVE_QZU                                      \
{                                                       \
  QLF3(S_Q,P_Z,NIL),                                    \
}
#define OP_SVE_RR                                       \
{                                                       \
  QLF2(W,W),                                            \
  QLF2(X,X),                                            \
}
#define OP_SVE_RURV_BHSD                                \
{                                                       \
  QLF4(W,NIL,W,S_B),                                    \
  QLF4(W,NIL,W,S_H),                                    \
  QLF4(W,NIL,W,S_S),                                    \
  QLF4(X,NIL,X,S_D),                                    \
}
#define OP_SVE_RUV_BHSD                                 \
{                                                       \
  QLF3(W,NIL,S_B),                                      \
  QLF3(W,NIL,S_H),                                      \
  QLF3(W,NIL,S_S),                                      \
  QLF3(X,NIL,S_D),                                      \
}
#define OP_SVE_SMD                                      \
{                                                       \
  QLF3(S_S,P_M,S_D),                                    \
}
#define OP_SVE_SMMBB                                    \
{                                                       \
  QLF5(S_S,P_M,P_M,S_B,S_B)                             \
}
#define OP_SVE_SMMHH                                    \
{                                                       \
  QLF5(S_S,P_M,P_M,S_H,S_H),                            \
}
#define OP_SVE_SMMS                                     \
{                                                       \
  QLF4(S_S,P_M,P_M,S_S),                                \
}
#define OP_SVE_SMMSS                                    \
{                                                       \
  QLF5(S_S,P_M,P_M,S_S,S_S)                             \
}
#define OP_SVE_SSS                                      \
{                                                       \
  QLF3(S_S,S_S,S_S),                                    \
}
#define OP_SVE_SSSU                                     \
{                                                       \
  QLF4(S_S,S_S,S_S,NIL),                                \
}
#define OP_SVE_SMH                                      \
{                                                       \
  QLF3(S_S,P_M,S_H),                                    \
}
#define OP_SVE_SHH                                      \
{                                                       \
  QLF3(S_S,S_H,S_H),                                    \
}
#define OP_SVE_SMS                                      \
{                                                       \
  QLF3(S_S,P_M,S_S),                                    \
}
#define OP_SVE_SS                                       \
{                                                       \
  QLF2(S_S,S_S),                                        \
}
#define OP_SVE_SU                                       \
{                                                       \
  QLF2(S_S,NIL),                                        \
}
#define OP_SVE_SUS                                      \
{                                                       \
  QLF3(S_S,NIL,S_S),                                    \
}
#define OP_SVE_SUU                                      \
{                                                       \
  QLF3(S_S,NIL,NIL),                                    \
}
#define OP_SVE_SZS                                      \
{                                                       \
  QLF3(S_S,P_Z,S_S),                                    \
}
#define OP_SVE_SBB                                      \
{                                                       \
  QLF3(S_S,S_B,S_B),                                    \
}
#define OP_SVE_SBBU                                     \
{                                                       \
  QLF4(S_S,S_B,S_B,NIL),                                \
}
#define OP_SVE_DSS                                      \
{                                                       \
  QLF3(S_D,S_S,S_S),                                    \
}
#define OP_SVE_DHHU                                     \
{                                                       \
  QLF4(S_D,S_H,S_H,NIL),                                \
}
#define OP_SVE_SZU                                      \
{                                                       \
  QLF3(S_S,P_Z,NIL),                                    \
}
#define OP_SVE_UB                                       \
{                                                       \
  QLF2(NIL,S_B),                                        \
}
#define OP_SVE_UUD                                      \
{                                                       \
  QLF3(NIL,NIL,S_D),                                    \
}
#define OP_SVE_UUS                                      \
{                                                       \
  QLF3(NIL,NIL,S_S),                                    \
}
#define OP_SVE_UX                                       \
{                                                       \
  QLF2(NIL,X),                                          \
}
#define OP_SVE_UXU                                      \
{                                                       \
  QLF3(NIL,X,NIL),                                      \
}
#define OP_SVE_VMR_BHSD                                 \
{                                                       \
  QLF3(S_B,P_M,W),                                      \
  QLF3(S_H,P_M,W),                                      \
  QLF3(S_S,P_M,W),                                      \
  QLF3(S_D,P_M,X),                                      \
}
#define OP_SVE_VMU_HSD                                  \
{                                                       \
  QLF3(S_H,P_M,NIL),                                    \
  QLF3(S_S,P_M,NIL),                                    \
  QLF3(S_D,P_M,NIL),                                    \
}
#define OP_SVE_VMVD_BHS                                 \
{                                                       \
  QLF4(S_B,P_M,S_B,S_D),                                \
  QLF4(S_H,P_M,S_H,S_D),                                \
  QLF4(S_S,P_M,S_S,S_D),                                \
}
#define OP_SVE_VMVU_BHSD                                \
{                                                       \
  QLF4(S_B,P_M,S_B,NIL),                                \
  QLF4(S_H,P_M,S_H,NIL),                                \
  QLF4(S_S,P_M,S_S,NIL),                                \
  QLF4(S_D,P_M,S_D,NIL),                                \
}
#define OP_SVE_VMVU_HSD                                 \
{                                                       \
  QLF4(S_H,P_M,S_H,NIL),                                \
  QLF4(S_S,P_M,S_S,NIL),                                \
  QLF4(S_D,P_M,S_D,NIL),                                \
}
#define OP_SVE_VMVV_BHSD                                \
{                                                       \
  QLF4(S_B,P_M,S_B,S_B),                                \
  QLF4(S_H,P_M,S_H,S_H),                                \
  QLF4(S_S,P_M,S_S,S_S),                                \
  QLF4(S_D,P_M,S_D,S_D),                                \
}
#define OP_SVE_VMVV_HSD                                 \
{                                                       \
  QLF4(S_H,P_M,S_H,S_H),                                \
  QLF4(S_S,P_M,S_S,S_S),                                \
  QLF4(S_D,P_M,S_D,S_D),                                \
}
#define OP_SVE_VMVV_SD                                  \
{                                                       \
  QLF4(S_S,P_M,S_S,S_S),                                \
  QLF4(S_D,P_M,S_D,S_D),                                \
}
#define OP_SVE_VMVVU_HSD                                \
{                                                       \
  QLF5(S_H,P_M,S_H,S_H,NIL),                            \
  QLF5(S_S,P_M,S_S,S_S,NIL),                            \
  QLF5(S_D,P_M,S_D,S_D,NIL),                            \
}
#define OP_SVE_VMV_BHSD                                 \
{                                                       \
  QLF3(S_B,P_M,S_B),                                    \
  QLF3(S_H,P_M,S_H),                                    \
  QLF3(S_S,P_M,S_S),                                    \
  QLF3(S_D,P_M,S_D),                                    \
}
#define OP_SVE_VMV_BHSDQ                                \
{                                                       \
  QLF3(S_B,P_M,S_B),                                    \
  QLF3(S_H,P_M,S_H),                                    \
  QLF3(S_S,P_M,S_S),                                    \
  QLF3(S_D,P_M,S_D),                                    \
  QLF3(S_Q,P_M,S_Q)                                     \
}
#define OP_SVE_VMV_HSD                                  \
{                                                       \
  QLF3(S_H,P_M,S_H),                                    \
  QLF3(S_S,P_M,S_S),                                    \
  QLF3(S_D,P_M,S_D),                                    \
}
#define OP_SVE_VMV_HSD_BHS                              \
{                                                       \
  QLF3(S_H,P_M,S_B),                                    \
  QLF3(S_S,P_M,S_H),                                    \
  QLF3(S_D,P_M,S_S),                                    \
}
#define OP_SVE_VVU_BH_SD                                \
{                                                       \
  QLF3(S_B,S_S,NIL),                                    \
  QLF3(S_H,S_D,NIL),                                    \
}
#define OP_SVE_VVU_HSD_BHS                              \
{                                                       \
  QLF3(S_H,S_B,NIL),                                    \
  QLF3(S_S,S_H,NIL),                                    \
  QLF3(S_D,S_S,NIL),                                    \
}
#define OP_SVE_VMV_SD                                   \
{                                                       \
  QLF3(S_S,P_M,S_S),                                    \
  QLF3(S_D,P_M,S_D),                                    \
}
#define OP_SVE_VM_HSD                                   \
{                                                       \
  QLF2(S_H,P_M),                                        \
  QLF2(S_S,P_M),                                        \
  QLF2(S_D,P_M),                                        \
}
#define OP_SVE_VPU_BHSD                                 \
{                                                       \
  QLF3(S_B,P_Z,NIL),                                    \
  QLF3(S_B,P_M,NIL),                                    \
  QLF3(S_H,P_Z,NIL),                                    \
  QLF3(S_H,P_M,NIL),                                    \
  QLF3(S_S,P_Z,NIL),                                    \
  QLF3(S_S,P_M,NIL),                                    \
  QLF3(S_D,P_Z,NIL),                                    \
  QLF3(S_D,P_M,NIL),                                    \
}
#define OP_SVE_VPV_BHSD                                 \
{                                                       \
  QLF3(S_B,P_Z,S_B),                                    \
  QLF3(S_B,P_M,S_B),                                    \
  QLF3(S_H,P_Z,S_H),                                    \
  QLF3(S_H,P_M,S_H),                                    \
  QLF3(S_S,P_Z,S_S),                                    \
  QLF3(S_S,P_M,S_S),                                    \
  QLF3(S_D,P_Z,S_D),                                    \
  QLF3(S_D,P_M,S_D),                                    \
}
#define OP_SVE_VRR_BHSD                                 \
{                                                       \
  QLF3(S_B,W,W),                                        \
  QLF3(S_H,W,W),                                        \
  QLF3(S_S,W,W),                                        \
  QLF3(S_D,X,X),                                        \
}
#define OP_SVE_VRU_BHSD                                 \
{                                                       \
  QLF3(S_B,W,NIL),                                      \
  QLF3(S_H,W,NIL),                                      \
  QLF3(S_S,W,NIL),                                      \
  QLF3(S_D,X,NIL),                                      \
}
#define OP_SVE_VR_BHSD                                  \
{                                                       \
  QLF2(S_B,W),                                          \
  QLF2(S_H,W),                                          \
  QLF2(S_S,W),                                          \
  QLF2(S_D,X),                                          \
}
#define OP_SVE_VUR_BHSD                                 \
{                                                       \
  QLF3(S_B,NIL,W),                                      \
  QLF3(S_H,NIL,W),                                      \
  QLF3(S_S,NIL,W),                                      \
  QLF3(S_D,NIL,X),                                      \
}
#define OP_SVE_VUU_BHS                                  \
{                                                       \
  QLF3(S_B,NIL,NIL),                                    \
  QLF3(S_H,NIL,NIL),                                    \
  QLF3(S_S,NIL,NIL),                                    \
}
#define OP_SVE_VUU_BHSD                                 \
{                                                       \
  QLF3(S_B,NIL,NIL),                                    \
  QLF3(S_H,NIL,NIL),                                    \
  QLF3(S_S,NIL,NIL),                                    \
  QLF3(S_D,NIL,NIL),                                    \
}
#define OP_SVE_VUVV_BHSD                                \
{                                                       \
  QLF4(S_B,NIL,S_B,S_B),                                \
  QLF4(S_H,NIL,S_H,S_H),                                \
  QLF4(S_S,NIL,S_S,S_S),                                \
  QLF4(S_D,NIL,S_D,S_D),                                \
}
#define OP_SVE_VUU_HS                                   \
{                                                       \
  QLF3(S_H,NIL,NIL),                                    \
  QLF3(S_S,NIL,NIL),                                    \
}
#define OP_SVE_VUVV_HSD                                 \
{                                                       \
  QLF4(S_H,NIL,S_H,S_H),                                \
  QLF4(S_S,NIL,S_S,S_S),                                \
  QLF4(S_D,NIL,S_D,S_D),                                \
}
#define OP_SVE_VUV_BHSD                                 \
{                                                       \
  QLF3(S_B,NIL,S_B),                                    \
  QLF3(S_H,NIL,S_H),                                    \
  QLF3(S_S,NIL,S_S),                                    \
  QLF3(S_D,NIL,S_D),                                    \
}
#define OP_SVE_VUV_HSD                                  \
{                                                       \
  QLF3(S_H,NIL,S_H),                                    \
  QLF3(S_S,NIL,S_S),                                    \
  QLF3(S_D,NIL,S_D),                                    \
}
#define OP_SVE_VUV_SD                                   \
{                                                       \
  QLF3(S_S,NIL,S_S),                                    \
  QLF3(S_D,NIL,S_D),                                    \
}
#define OP_SVE_VU_BHSD                                  \
{                                                       \
  QLF2(S_B,NIL),                                        \
  QLF2(S_H,NIL),                                        \
  QLF2(S_S,NIL),                                        \
  QLF2(S_D,NIL),                                        \
}
#define OP_SVE_VU_HSD                                   \
{                                                       \
  QLF2(S_H,NIL),                                        \
  QLF2(S_S,NIL),                                        \
  QLF2(S_D,NIL),                                        \
}
#define OP_SVE_VU_HSD                                   \
{                                                       \
  QLF2(S_H,NIL),                                        \
  QLF2(S_S,NIL),                                        \
  QLF2(S_D,NIL),                                        \
}
#define OP_SVE_Vv_HSD                                   \
{                                                       \
  QLF2(S_H,S_H),                                        \
  QLF2(S_S,S_S),                                        \
  QLF2(S_D,S_D),                                        \
  QLF2(S_H,NIL),                                        \
  QLF2(S_S,NIL),                                        \
  QLF2(S_D,NIL),                                        \
}
#define OP_SVE_VVD_BHS                                  \
{                                                       \
  QLF3(S_B,S_B,S_D),                                    \
  QLF3(S_H,S_H,S_D),                                    \
  QLF3(S_S,S_S,S_D),                                    \
}
#define OP_SVE_VVU_BHSD                                 \
{                                                       \
  QLF3(S_B,S_B,NIL),                                    \
  QLF3(S_H,S_H,NIL),                                    \
  QLF3(S_S,S_S,NIL),                                    \
  QLF3(S_D,S_D,NIL),                                    \
}
#define OP_SVE_VVVU_H                                   \
{                                                       \
  QLF4(S_H,S_H,S_H,NIL),                                \
}
#define OP_SVE_VVVU_S                                   \
{                                                       \
  QLF4(S_S,S_S,S_S,NIL),                                \
}
#define OP_SVE_VVVU_SD_BH                               \
{                                                       \
  QLF4(S_S,S_B,S_B,NIL),                                \
  QLF4(S_D,S_H,S_H,NIL),                                \
}
#define OP_SVE_VVVU_HSD                                 \
{                                                       \
  QLF4(S_H,S_H,S_H,NIL),                                \
  QLF4(S_S,S_S,S_S,NIL),                                \
  QLF4(S_D,S_D,S_D,NIL),                                \
}
#define OP_SVE_VVVU_BHSD                                \
{                                                       \
  QLF4(S_B,S_B,S_B,NIL),                                \
  QLF4(S_H,S_H,S_H,NIL),                                \
  QLF4(S_S,S_S,S_S,NIL),                                \
  QLF4(S_D,S_D,S_D,NIL),                                \
}
#define OP_SVE_VVV_BHSD                                 \
{                                                       \
  QLF3(S_B,S_B,S_B),                                    \
  QLF3(S_H,S_H,S_H),                                    \
  QLF3(S_S,S_S,S_S),                                    \
  QLF3(S_D,S_D,S_D),                                    \
}
#define OP_SVE_VVV_D                                    \
{                                                       \
  QLF3(S_D,S_D,S_D),                                    \
}
#define OP_SVE_VVV_D_H                                  \
{                                                       \
  QLF3(S_D,S_H,S_H),                                    \
}
#define OP_SVE_VVV_H                                    \
{                                                       \
  QLF3(S_H,S_H,S_H),                                    \
}
#define OP_SVE_VVV_HSD                                  \
{                                                       \
  QLF3(S_H,S_H,S_H),                                    \
  QLF3(S_S,S_S,S_S),                                    \
  QLF3(S_D,S_D,S_D),                                    \
}
#define OP_SVE_VVV_S                                    \
{                                                       \
  QLF3(S_S,S_S,S_S),                                    \
}
#define OP_SVE_VVV_HD_BS				\
{                                                       \
  QLF3(S_H,S_B,S_B),                                    \
  QLF3(S_D,S_S,S_S),                                    \
}
#define OP_SVE_VVV_S_B                                  \
{                                                       \
  QLF3(S_S,S_B,S_B),                                    \
}
#define OP_SVE_VVV_Q_D					\
{                                                       \
  QLF3(S_Q,S_D,S_D),                                    \
}
#define OP_SVE_VVV_HSD_BHS                              \
{                                                       \
  QLF3(S_H,S_B,S_B),                                    \
  QLF3(S_S,S_H,S_H),                                    \
  QLF3(S_D,S_S,S_S),                                    \
}
#define OP_SVE_VVV_HSD_BHS2                             \
{                                                       \
  QLF3(S_H,S_H,S_B),                                    \
  QLF3(S_S,S_S,S_H),                                    \
  QLF3(S_D,S_D,S_S),                                    \
}
#define OP_SVE_VVV_BHS_HSD                              \
{                                                       \
  QLF3(S_B,S_H,S_H),                                    \
  QLF3(S_H,S_S,S_S),                                    \
  QLF3(S_S,S_D,S_D),                                    \
}
#define OP_SVE_VV_BHS_HSD                               \
{                                                       \
  QLF2(S_B,S_H),                                        \
  QLF2(S_H,S_S),                                        \
  QLF2(S_S,S_D),                                        \
}
#define OP_SVE_VVV_SD_BH                                \
{                                                       \
  QLF3(S_S,S_B,S_B),                                    \
  QLF3(S_D,S_H,S_H),                                    \
}
#define OP_SVE_VVV_SD                                   \
{                                                       \
  QLF3(S_S,S_S,S_S),                                    \
  QLF3(S_D,S_D,S_D),                                    \
}
#define OP_SVE_VV_BHSD                                  \
{                                                       \
  QLF2(S_B,S_B),                                        \
  QLF2(S_H,S_H),                                        \
  QLF2(S_S,S_S),                                        \
  QLF2(S_D,S_D),                                        \
}
#define OP_SVE_VV_BHSDQ                                 \
{                                                       \
  QLF2(S_B,S_B),                                        \
  QLF2(S_H,S_H),                                        \
  QLF2(S_S,S_S),                                        \
  QLF2(S_D,S_D),                                        \
  QLF2(S_Q,S_Q),                                        \
}
#define OP_SVE_VV_BH_SD                                 \
{                                                       \
  QLF2(S_B,S_S),                                        \
  QLF2(S_H,S_D),                                        \
}
#define OP_SVE_VV_HSD                                   \
{                                                       \
  QLF2(S_H,S_H),                                        \
  QLF2(S_S,S_S),                                        \
  QLF2(S_D,S_D),                                        \
}
#define OP_SVE_VVU_BHS_HSD                               \
{                                                       \
  QLF3(S_B,S_H,NIL),                                        \
  QLF3(S_H,S_S,NIL),                                        \
  QLF3(S_S,S_D,NIL),                                        \
}
#define OP_SVE_VV_HSD_BHS                               \
{                                                       \
  QLF2(S_H,S_B),                                        \
  QLF2(S_S,S_H),                                        \
  QLF2(S_D,S_S),                                        \
}
#define OP_SVE_VV_SD                                    \
{                                                       \
  QLF2(S_S,S_S),                                        \
  QLF2(S_D,S_D),                                        \
}
#define OP_SVE_VWW_BHSD                                 \
{                                                       \
  QLF3(S_B,W,W),                                        \
  QLF3(S_H,W,W),                                        \
  QLF3(S_S,W,W),                                        \
  QLF3(S_D,W,W),                                        \
}
#define OP_SVE_VXX_BHSD                                 \
{                                                       \
  QLF3(S_B,X,X),                                        \
  QLF3(S_H,X,X),                                        \
  QLF3(S_S,X,X),                                        \
  QLF3(S_D,X,X),                                        \
}
#define OP_SVE_VXXU_BHSD                                \
{                                                       \
  QLF4(S_B,X,X,NIL),                                    \
  QLF4(S_H,X,X,NIL),                                    \
  QLF4(S_S,X,X,NIL),                                    \
  QLF4(S_D,X,X,NIL),                                    \
}
#define OP_SVE_VZVD_BHS                                 \
{                                                       \
  QLF4(S_B,P_Z,S_B,S_D),                                \
  QLF4(S_H,P_Z,S_H,S_D),                                \
  QLF4(S_S,P_Z,S_S,S_D),                                \
}
#define OP_SVE_VZVU_BHSD                                \
{                                                       \
  QLF4(S_B,P_Z,S_B,NIL),                                \
  QLF4(S_H,P_Z,S_H,NIL),                                \
  QLF4(S_S,P_Z,S_S,NIL),                                \
  QLF4(S_D,P_Z,S_D,NIL),                                \
}
#define OP_SVE_VZVV_BHSD                                \
{                                                       \
  QLF4(S_B,P_Z,S_B,S_B),                                \
  QLF4(S_H,P_Z,S_H,S_H),                                \
  QLF4(S_S,P_Z,S_S,S_S),                                \
  QLF4(S_D,P_Z,S_D,S_D),                                \
}
#define OP_SVE_VZVV_HSD                                 \
{                                                       \
  QLF4(S_H,P_Z,S_H,S_H),                                \
  QLF4(S_S,P_Z,S_S,S_S),                                \
  QLF4(S_D,P_Z,S_D,S_D),                                \
}
#define OP_SVE_VZVV_SD                                 \
{                                                       \
  QLF4(S_S,P_Z,S_S,S_S),                                \
  QLF4(S_D,P_Z,S_D,S_D),                                \
}
#define OP_SVE_VZVV_BH                                 \
{                                                       \
  QLF4(S_B,P_Z,S_B,S_B),                                \
  QLF4(S_H,P_Z,S_H,S_H),                                \
}
#define OP_SVE_VZV_SD                                   \
{                                                       \
  QLF3(S_S,P_Z,S_S),                                    \
  QLF3(S_D,P_Z,S_D),                                    \
}
#define OP_SVE_VZV_HSD                                  \
{                                                       \
  QLF3(S_H,P_Z,S_H),                                    \
  QLF3(S_S,P_Z,S_S),                                    \
  QLF3(S_D,P_Z,S_D),                                    \
}
#define OP_SVE_V_BHSD                                   \
{                                                       \
  QLF1(S_B),                                            \
  QLF1(S_H),                                            \
  QLF1(S_S),                                            \
  QLF1(S_D),                                            \
}
#define OP_SVE_V_HSD                                    \
{                                                       \
  QLF1(S_H),                                            \
  QLF1(S_S),                                            \
  QLF1(S_D),                                            \
}
#define OP_SVE_WU                                       \
{                                                       \
  QLF2(W,NIL),                                          \
}
#define OP_SVE_WV_BHSD                                  \
{                                                       \
  QLF2(W,S_B),                                          \
  QLF2(W,S_H),                                          \
  QLF2(W,S_S),                                          \
  QLF2(W,S_D),                                          \
}
#define OP_SVE_XU                                       \
{                                                       \
  QLF2(X,NIL),                                          \
}
#define OP_SVE_XUV_BHSD                                 \
{                                                       \
  QLF3(X,NIL,S_B),                                      \
  QLF3(X,NIL,S_H),                                      \
  QLF3(X,NIL,S_S),                                      \
  QLF3(X,NIL,S_D),                                      \
}
#define OP_SVE_XVW_BHSD                                 \
{                                                       \
  QLF3(X,S_B,W),                                        \
  QLF3(X,S_H,W),                                        \
  QLF3(X,S_S,W),                                        \
  QLF3(X,S_D,W),                                        \
}
#define OP_SVE_XV_BHSD                                  \
{                                                       \
  QLF2(X,S_B),                                          \
  QLF2(X,S_H),                                          \
  QLF2(X,S_S),                                          \
  QLF2(X,S_D),                                          \
}
#define OP_SVE_XWU                                      \
{                                                       \
  QLF3(X,W,NIL),                                        \
}
#define OP_SVE_XXU                                      \
{                                                       \
  QLF3(X,X,NIL),                                        \
}

/* e.g. UDOT <Vd>.2S, <Vn>.8B, <Vm>.8B.  */
#define QL_V3DOT	   \
{			   \
  QLF3(V_2S, V_8B,  V_8B), \
  QLF3(V_4S, V_16B, V_16B),\
}

/* e.g. UDOT <Vd>.2S, <Vn>.8B, <Vm>.4B[<index>].  */
#define QL_V2DOT	 \
{			 \
  QLF3(V_2S, V_8B,  S_4B),\
  QLF3(V_4S, V_16B, S_4B),\
}

/* e.g. SHA512H <Qd>, <Qn>, <Vm>.2D.  */
#define QL_SHA512UPT			\
{				\
  QLF3(S_Q, S_Q, V_2D),		\
}

/* e.g. SHA512SU0 <Vd.2D>, <Vn>.2D.  */
#define QL_V2SAME2D		\
{				\
  QLF2(V_2D, V_2D),		\
}

/* e.g. SHA512SU1 <Vd>.2D, <Vn>.2D, <Vm>.2D>.  */
#define QL_V3SAME2D		\
{				\
  QLF3(V_2D, V_2D, V_2D),	\
}

/* e.g. EOR3 <Vd>.16B, <Vn>.16B, <Vm>.16B, <Va>.16B.  */
#define QL_V4SAME16B			\
{					\
  QLF4(V_16B, V_16B, V_16B, V_16B),	\
}

/* e.g. SM3SS1 <Vd>.4S, <Vn>.4S, <Vm>.4S, <Va>.4S.  */
#define QL_V4SAME4S			\
{					\
  QLF4(V_4S, V_4S, V_4S, V_4S),		\
}

/* e.g. XAR <Vd>.2D, <Vn>.2D, <Vm>.2D, #<imm6>.  */
#define QL_XAR			\
{				\
  QLF4(V_2D, V_2D, V_2D, imm_0_63),	\
}

/* e.g. SM3TT1A <Vd>.4S, <Vn>.4S, <Vm>.S[<imm2>].  */
#define QL_SM3TT	\
{			\
  QLF3(V_4S, V_4S, S_S),\
}

/* e.g. FMLAL  <Vd>.2S, <Vn>.2H, <Vm>.2H.  */
#define QL_V3FML2S \
{		   \
  QLF3(V_2S, V_2H, V_2H),\
}

/* e.g. FMLAL  <Vd>.4S, <Vn>.4H, <Vm>.4H.  */
#define QL_V3FML4S \
{		   \
  QLF3(V_4S, V_4H, V_4H),\
}

/* e.g. FMLAL  <Vd>.2S, <Vn>.2H, <Vm>.H[<index>].  */
#define QL_V2FML2S \
{		   \
  QLF3(V_2S, V_2H, S_H),\
}

/* e.g. FMLAL  <Vd>.4S, <Vn>.4H, <Vm>.H[<index>].  */
#define QL_V2FML4S \
{		   \
  QLF3(V_4S, V_4H, S_H),\
}

/* e.g. RMIF <Xn>, #<shift>, #<mask>.  */
#define QL_RMIF  \
{		  \
  QLF3(X, imm_0_63, imm_0_15),\
}

/* e.g. SETF8 <Wn>.  */
#define QL_SETF \
{		\
  QLF1(W),	\
}

/* e.g. STLURB <Wt>, [<Xn|SP>{,#<simm>}].  */
#define QL_STLW \
{		\
  QLF2(W, NIL),	\
}

/* e.g. STLURB <Xt>, [<Xn|SP>{,#<simm>}].  */
#define QL_STLX \
{		\
  QLF2(X, NIL),	\
}

/* e.g. BFDOT <Vd>.<Ta>, <Vn>.<Tb>, <Vm>.<Tb> */
#define QL_BFDOT64    \
{		      \
  QLF3(V_2S, V_4H, V_4H),\
  QLF3(V_4S, V_8H, V_8H),\
}

/* e.g. BFDOT <Vd>.<Ta>, <Vn>.<Tb>, <Vm>.2H[<index>] */
#define QL_BFDOT64I    \
{		      \
  QLF3(V_2S, V_4H, S_2H),\
  QLF3(V_4S, V_8H, S_2H),\
}

/* e.g. SMMLA <Vd>.4S, <Vn>.16B, <Vm>.16B */
#define QL_MMLA64      \
{		      \
  QLF3(V_4S, V_16B, V_16B),\
}

/* e.g. BFMMLA <Vd>.4s, <Vn>.8h, <Vm>.8h */
#define QL_BFMMLA    \
{		      \
  QLF3(V_4S, V_8H, V_8H),\
}

/* e.g. BFCVT <Hd>, <Sn> */
#define QL_BFCVT64      \
{		      \
  QLF2(S_H,S_S),  \
}

/* e.g. BFCVT <Hd>, <Sn> */
#define QL_BFCVTN64      \
{		      \
  QLF2(V_4H,V_4S),  \
}

/* e.g. BFCVT <Hd>, <Sn> */
#define QL_BFCVTN2_64      \
{		      \
  QLF2(V_8H,V_4S),  \
}

/* e.g. BFMLAL2 <Vd>.4s, <Vn>.8h, <Vm>.H[<index>] */
#define QL_V3BFML4S      \
{		      \
  QLF3(V_4S, V_8H, S_H),	\
}

/* Opcode table.

  Any SVE or SVE2 feature must include AARCH64_FEATURE_{SVE|SVE2} in its
  bitmask, even if this is implied by other selected feature bits.  This
  allows verify_constraints to identify SVE instructions when selecting an
  error message for MOVPRFX constraint violations.  */

static const aarch64_feature_set aarch64_feature_v8 =
  AARCH64_FEATURE (AARCH64_FEATURE_V8, 0);
static const aarch64_feature_set aarch64_feature_fp =
  AARCH64_FEATURE (AARCH64_FEATURE_FP, 0);
static const aarch64_feature_set aarch64_feature_simd =
  AARCH64_FEATURE (AARCH64_FEATURE_SIMD, 0);
static const aarch64_feature_set aarch64_feature_crc =
  AARCH64_FEATURE (AARCH64_FEATURE_CRC, 0);
static const aarch64_feature_set aarch64_feature_lse =
  AARCH64_FEATURE (AARCH64_FEATURE_LSE, 0);
static const aarch64_feature_set aarch64_feature_lor =
  AARCH64_FEATURE (AARCH64_FEATURE_LOR, 0);
static const aarch64_feature_set aarch64_feature_rdma =
  AARCH64_FEATURE (AARCH64_FEATURE_RDMA, 0);
static const aarch64_feature_set aarch64_feature_v8_2 =
  AARCH64_FEATURE (AARCH64_FEATURE_V8_2, 0);
static const aarch64_feature_set aarch64_feature_fp_f16 =
  AARCH64_FEATURE (AARCH64_FEATURE_F16 | AARCH64_FEATURE_FP, 0);
static const aarch64_feature_set aarch64_feature_simd_f16 =
  AARCH64_FEATURE (AARCH64_FEATURE_F16 | AARCH64_FEATURE_SIMD, 0);
static const aarch64_feature_set aarch64_feature_sve =
  AARCH64_FEATURE (AARCH64_FEATURE_SVE, 0);
static const aarch64_feature_set aarch64_feature_v8_3 =
  AARCH64_FEATURE (AARCH64_FEATURE_V8_3, 0);
static const aarch64_feature_set aarch64_feature_fp_v8_3 =
  AARCH64_FEATURE (AARCH64_FEATURE_V8_3 | AARCH64_FEATURE_FP, 0);
static const aarch64_feature_set aarch64_feature_pac =
  AARCH64_FEATURE (AARCH64_FEATURE_PAC, 0);
static const aarch64_feature_set aarch64_feature_compnum =
  AARCH64_FEATURE (AARCH64_FEATURE_COMPNUM, 0);
static const aarch64_feature_set aarch64_feature_rcpc =
  AARCH64_FEATURE (AARCH64_FEATURE_RCPC, 0);
static const aarch64_feature_set aarch64_feature_dotprod =
  AARCH64_FEATURE (AARCH64_FEATURE_DOTPROD, 0);
static const aarch64_feature_set aarch64_feature_sha2 =
  AARCH64_FEATURE (AARCH64_FEATURE_V8 | AARCH64_FEATURE_SHA2, 0);
static const aarch64_feature_set aarch64_feature_aes =
  AARCH64_FEATURE (AARCH64_FEATURE_V8 | AARCH64_FEATURE_AES, 0);
static const aarch64_feature_set aarch64_feature_v8_4 =
  AARCH64_FEATURE (AARCH64_FEATURE_V8_4, 0);
static const aarch64_feature_set aarch64_feature_sm4 =
  AARCH64_FEATURE (AARCH64_FEATURE_SM4 | AARCH64_FEATURE_SIMD
		   | AARCH64_FEATURE_FP, 0);
static const aarch64_feature_set aarch64_feature_sha3 =
  AARCH64_FEATURE (AARCH64_FEATURE_SHA2 | AARCH64_FEATURE_SHA3
		   | AARCH64_FEATURE_SIMD | AARCH64_FEATURE_FP, 0);
static const aarch64_feature_set aarch64_feature_fp_16_v8_2 =
  AARCH64_FEATURE (AARCH64_FEATURE_F16_FML | AARCH64_FEATURE_F16
		   | AARCH64_FEATURE_FP, 0);
static const aarch64_feature_set aarch64_feature_v8_5 =
  AARCH64_FEATURE (AARCH64_FEATURE_V8_5, 0);
static const aarch64_feature_set aarch64_feature_flagmanip =
  AARCH64_FEATURE (AARCH64_FEATURE_FLAGMANIP, 0);
static const aarch64_feature_set aarch64_feature_frintts =
  AARCH64_FEATURE (AARCH64_FEATURE_FRINTTS, 0);
static const aarch64_feature_set aarch64_feature_sb =
  AARCH64_FEATURE (AARCH64_FEATURE_SB, 0);
static const aarch64_feature_set aarch64_feature_predres =
  AARCH64_FEATURE (AARCH64_FEATURE_PREDRES, 0);
static const aarch64_feature_set aarch64_feature_memtag =
  AARCH64_FEATURE (AARCH64_FEATURE_MEMTAG, 0);
static const aarch64_feature_set aarch64_feature_bfloat16 =
  AARCH64_FEATURE (AARCH64_FEATURE_BFLOAT16, 0);
static const aarch64_feature_set aarch64_feature_bfloat16_sve =
  AARCH64_FEATURE (AARCH64_FEATURE_BFLOAT16 | AARCH64_FEATURE_SVE, 0);
static const aarch64_feature_set aarch64_feature_tme =
  AARCH64_FEATURE (AARCH64_FEATURE_TME, 0);
static const aarch64_feature_set aarch64_feature_sve2 =
  AARCH64_FEATURE (AARCH64_FEATURE_SVE2, 0);
static const aarch64_feature_set aarch64_feature_sve2aes =
  AARCH64_FEATURE (AARCH64_FEATURE_SVE2 | AARCH64_FEATURE_SVE2_AES, 0);
static const aarch64_feature_set aarch64_feature_sve2sha3 =
  AARCH64_FEATURE (AARCH64_FEATURE_SVE2 | AARCH64_FEATURE_SVE2_SHA3, 0);
static const aarch64_feature_set aarch64_feature_sve2sm4 =
  AARCH64_FEATURE (AARCH64_FEATURE_SVE2 | AARCH64_FEATURE_SVE2_SM4, 0);
static const aarch64_feature_set aarch64_feature_sve2bitperm =
  AARCH64_FEATURE (AARCH64_FEATURE_SVE2 | AARCH64_FEATURE_SVE2_BITPERM, 0);
static const aarch64_feature_set aarch64_feature_sme =
  AARCH64_FEATURE (AARCH64_FEATURE_SVE2 | AARCH64_FEATURE_SME, 0);
static const aarch64_feature_set aarch64_feature_sme_f64f64 =
  AARCH64_FEATURE (AARCH64_FEATURE_SVE2 | AARCH64_FEATURE_SME
       | AARCH64_FEATURE_SME_F64F64, 0);
static const aarch64_feature_set aarch64_feature_sme_i16i64 =
  AARCH64_FEATURE (AARCH64_FEATURE_SVE2 | AARCH64_FEATURE_SME
       | AARCH64_FEATURE_SME_I16I64, 0);
static const aarch64_feature_set aarch64_feature_sme2 =
  AARCH64_FEATURE (AARCH64_FEATURE_SVE2 | AARCH64_FEATURE_SME
		   | AARCH64_FEATURE_SME2, 0);
static const aarch64_feature_set aarch64_feature_sme2_i16i64 =
  AARCH64_FEATURE (AARCH64_FEATURE_SME2 | AARCH64_FEATURE_SME_I16I64, 0);
static const aarch64_feature_set aarch64_feature_sme2_f64f64 =
  AARCH64_FEATURE (AARCH64_FEATURE_SME2 | AARCH64_FEATURE_SME_F64F64, 0);
static const aarch64_feature_set aarch64_feature_v8_6 =
  AARCH64_FEATURE (AARCH64_FEATURE_V8_6, 0);
static const aarch64_feature_set aarch64_feature_v8_7 =
  AARCH64_FEATURE (AARCH64_FEATURE_V8_7, 0);
static const aarch64_feature_set aarch64_feature_i8mm =
  AARCH64_FEATURE (AARCH64_FEATURE_I8MM, 0);
static const aarch64_feature_set aarch64_feature_i8mm_sve =
  AARCH64_FEATURE (AARCH64_FEATURE_I8MM | AARCH64_FEATURE_SVE, 0);
static const aarch64_feature_set aarch64_feature_f32mm_sve =
  AARCH64_FEATURE (AARCH64_FEATURE_F32MM | AARCH64_FEATURE_SVE, 0);
static const aarch64_feature_set aarch64_feature_f64mm_sve =
  AARCH64_FEATURE (AARCH64_FEATURE_F64MM | AARCH64_FEATURE_SVE, 0);
static const aarch64_feature_set aarch64_feature_v8_r =
  AARCH64_FEATURE (AARCH64_FEATURE_V8_R, 0);
static const aarch64_feature_set aarch64_feature_ls64 =
  AARCH64_FEATURE (AARCH64_FEATURE_LS64, 0);
static const aarch64_feature_set aarch64_feature_flagm =
  AARCH64_FEATURE (AARCH64_FEATURE_FLAGM, 0);
static const aarch64_feature_set aarch64_feature_mops =
  AARCH64_FEATURE (AARCH64_FEATURE_MOPS, 0);
static const aarch64_feature_set aarch64_feature_mops_memtag =
  AARCH64_FEATURE (AARCH64_FEATURE_MOPS | AARCH64_FEATURE_MEMTAG, 0);
static const aarch64_feature_set aarch64_feature_hbc =
  AARCH64_FEATURE (AARCH64_FEATURE_HBC, 0);
static const aarch64_feature_set aarch64_feature_cssc =
  AARCH64_FEATURE (AARCH64_FEATURE_CSSC, 0);

#define CORE		&aarch64_feature_v8
#define FP		&aarch64_feature_fp
#define SIMD		&aarch64_feature_simd
#define CRC		&aarch64_feature_crc
#define LSE		&aarch64_feature_lse
#define LOR		&aarch64_feature_lor
#define RDMA		&aarch64_feature_rdma
#define FP_F16		&aarch64_feature_fp_f16
#define SIMD_F16	&aarch64_feature_simd_f16
#define ARMV8_2		&aarch64_feature_v8_2
#define SVE		&aarch64_feature_sve
#define ARMV8_3		&aarch64_feature_v8_3
#define FP_V8_3		&aarch64_feature_fp_v8_3
#define PAC		&aarch64_feature_pac
#define COMPNUM		&aarch64_feature_compnum
#define RCPC		&aarch64_feature_rcpc
#define SHA2		&aarch64_feature_sha2
#define AES		&aarch64_feature_aes
#define ARMV8_4		&aarch64_feature_v8_4
#define SHA3		&aarch64_feature_sha3
#define SM4		&aarch64_feature_sm4
#define FP_F16_V8_2	&aarch64_feature_fp_16_v8_2
#define DOTPROD		&aarch64_feature_dotprod
#define ARMV8_5		&aarch64_feature_v8_5
#define FLAGMANIP	&aarch64_feature_flagmanip
#define FRINTTS		&aarch64_feature_frintts
#define SB		&aarch64_feature_sb
#define PREDRES		&aarch64_feature_predres
#define MEMTAG		&aarch64_feature_memtag
#define TME		&aarch64_feature_tme
#define SVE2		&aarch64_feature_sve2
#define SVE2_AES		&aarch64_feature_sve2aes
#define SVE2_SHA3	&aarch64_feature_sve2sha3
#define SVE2_SM4		&aarch64_feature_sve2sm4
#define SVE2_BITPERM	&aarch64_feature_sve2bitperm
#define SME		&aarch64_feature_sme
#define SME_F64F64	&aarch64_feature_sme_f64f64
#define SME_I16I64	&aarch64_feature_sme_i16i64
#define SME2		&aarch64_feature_sme2
#define SME2_I16I64	&aarch64_feature_sme2_i16i64
#define SME2_F64F64	&aarch64_feature_sme2_f64f64
#define ARMV8_6		&aarch64_feature_v8_6
#define ARMV8_6_SVE		&aarch64_feature_v8_6
#define BFLOAT16_SVE	&aarch64_feature_bfloat16_sve
#define BFLOAT16	&aarch64_feature_bfloat16
#define I8MM_SVE      &aarch64_feature_i8mm_sve
#define F32MM_SVE     &aarch64_feature_f32mm_sve
#define F64MM_SVE     &aarch64_feature_f64mm_sve
#define I8MM      &aarch64_feature_i8mm
#define ARMV8_R	  &aarch64_feature_v8_r
#define ARMV8_7	  &aarch64_feature_v8_7
#define LS64	  &aarch64_feature_ls64
#define FLAGM	  &aarch64_feature_flagm
#define MOPS	  &aarch64_feature_mops
#define MOPS_MEMTAG &aarch64_feature_mops_memtag
#define HBC	  &aarch64_feature_hbc
#define CSSC	  &aarch64_feature_cssc

#define CORE_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, OP, CORE, OPS, QUALS, FLAGS, 0, 0, NULL }
#define __FP_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, OP, FP, OPS, QUALS, FLAGS, 0, 0, NULL }
#define SIMD_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, OP, SIMD, OPS, QUALS, FLAGS, 0, 0, NULL }
#define _SIMD_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,VERIFIER) \
  { NAME, OPCODE, MASK, CLASS, OP, SIMD, OPS, QUALS, FLAGS, 0, 0, VERIFIER }
#define _CRC_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, CRC, OPS, QUALS, FLAGS, 0, 0, NULL }
#define _LSE_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, LSE, OPS, QUALS, FLAGS, 0, 0, NULL }
#define _LOR_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, LOR, OPS, QUALS, FLAGS, 0, 0, NULL }
#define RDMA_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, RDMA, OPS, QUALS, FLAGS, 0, 0, NULL }
#define FF16_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, FP_F16, OPS, QUALS, FLAGS, 0, 0, NULL }
#define SF16_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS)		\
  { NAME, OPCODE, MASK, CLASS, 0, SIMD_F16, OPS, QUALS, FLAGS, 0, 0, NULL }
#define V8_2_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, OP, ARMV8_2, OPS, QUALS, FLAGS, 0, 0, NULL }
#define _SVE_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SVE, OPS, QUALS, \
    FLAGS | F_STRICT, 0, TIED, NULL }
#define _SVE_INSNC(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,CONSTRAINTS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SVE, OPS, QUALS, \
    FLAGS | F_STRICT, CONSTRAINTS, TIED, NULL }
#define V8_3_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, ARMV8_3, OPS, QUALS, FLAGS, 0, 0, NULL }
#define PAC_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, PAC, OPS, QUALS, FLAGS, 0, 0, NULL }
#define CNUM_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, OP, COMPNUM, OPS, QUALS, FLAGS, 0, 0, NULL }
#define RCPC_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, RCPC, OPS, QUALS, FLAGS, 0, 0, NULL }
#define SHA2_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, SHA2, OPS, QUALS, FLAGS, 0, 0, NULL }
#define AES_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, AES, OPS, QUALS, FLAGS, 0, 0, NULL }
#define V8_4_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, ARMV8_4, OPS, QUALS, FLAGS, 0, 0, NULL }
#define SHA3_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, SHA3, OPS, QUALS, FLAGS, 0, 0, NULL }
#define SM4_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, SM4, OPS, QUALS, FLAGS, 0, 0, NULL }
#define FP16_V8_2_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, FP_F16_V8_2, OPS, QUALS, FLAGS, 0, 0, NULL }
#define DOT_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, DOTPROD, OPS, QUALS, FLAGS, 0, 0, NULL }
#define V8_5_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, ARMV8_5, OPS, QUALS, FLAGS, 0, 0, NULL }
#define FLAGMANIP_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, FLAGMANIP, OPS, QUALS, FLAGS, 0, 0, NULL }
#define FRINTTS_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, FRINTTS, OPS, QUALS, FLAGS, 0, 0, NULL }
#define SB_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, SB, OPS, QUALS, FLAGS, 0, 0, NULL }
#define PREDRES_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, PREDRES, OPS, QUALS, FLAGS, 0, 0, NULL }
#define MEMTAG_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, MEMTAG, OPS, QUALS, FLAGS, 0, 0, NULL }
#define _TME_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, OP, TME, OPS, QUALS, FLAGS, 0, 0, NULL }
#define SVE2_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SVE2, OPS, QUALS, \
    FLAGS | F_STRICT, 0, TIED, NULL }
#define SVE2_INSNC(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,CONSTRAINTS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SVE2, OPS, QUALS, \
    FLAGS | F_STRICT, CONSTRAINTS, TIED, NULL }
#define SVE2AES_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SVE2_AES, OPS, QUALS, \
    FLAGS | F_STRICT, 0, TIED, NULL }
#define SVE2SHA3_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SVE2_SHA3, OPS, QUALS, \
    FLAGS | F_STRICT, 0, TIED, NULL }
#define SVE2SM4_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SVE2_SM4, OPS, QUALS, \
    FLAGS | F_STRICT, 0, TIED, NULL }
#define SVE2SM4_INSNC(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,CONSTRAINTS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SVE2_SM4, OPS, QUALS, \
    FLAGS | F_STRICT, CONSTRAINTS, TIED, NULL }
#define SME_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SME, OPS, QUALS, \
    F_STRICT | FLAGS, 0, TIED, NULL }
#define SME_F64F64_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SME_F64F64, OPS, QUALS, \
    F_STRICT | FLAGS, 0, TIED, NULL }
#define SME_I16I64_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SME_I16I64, OPS, QUALS, \
    F_STRICT | FLAGS, 0, TIED, NULL }
#define SME_INSNC(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,CONSTRAINTS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SME, OPS, QUALS, \
    F_STRICT | FLAGS, CONSTRAINTS, TIED, NULL }
#define SME2_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SME2, OPS, QUALS, \
    F_STRICT | FLAGS, 0, TIED, NULL }
#define SME2_INSNC(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,CONSTRAINTS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SME2, OPS, QUALS, \
    FLAGS | F_STRICT, CONSTRAINTS, TIED, NULL }
#define SME2_I16I64_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SME2_I16I64, OPS, QUALS, \
    F_STRICT | FLAGS, 0, TIED, NULL }
#define SME2_F64F64_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SME2_F64F64, OPS, QUALS, \
    F_STRICT | FLAGS, 0, TIED, NULL }
#define SVE2BITPERM_INSN(NAME,OPCODE,MASK,CLASS,OP,OPS,QUALS,FLAGS,TIED) \
  { NAME, OPCODE, MASK, CLASS, OP, SVE2_BITPERM, OPS, QUALS, \
    FLAGS | F_STRICT, 0, TIED, NULL }
#define V8_6_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, ARMV8_6, OPS, QUALS, FLAGS, 0, 0, NULL }
#define BFLOAT16_SVE_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, BFLOAT16_SVE, OPS, QUALS, FLAGS, 0, 0, NULL }
#define BFLOAT16_SVE_INSNC(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS, CONSTRAINTS, TIED) \
  { NAME, OPCODE, MASK, CLASS, 0, BFLOAT16_SVE, OPS, QUALS, FLAGS | F_STRICT, \
    CONSTRAINTS, TIED, NULL }
#define BFLOAT16_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, BFLOAT16, OPS, QUALS, FLAGS, 0, 0, NULL }
#define INT8MATMUL_SVE_INSNC(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS, CONSTRAINTS, TIED) \
  { NAME, OPCODE, MASK, CLASS, 0, I8MM_SVE, OPS, QUALS, FLAGS, CONSTRAINTS, TIED, NULL }
#define INT8MATMUL_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, I8MM, OPS, QUALS, FLAGS, 0, 0, NULL }
#define F64MATMUL_SVE_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS,TIED) \
  { NAME, OPCODE, MASK, CLASS, 0, F64MM_SVE, OPS, QUALS, FLAGS, 0, TIED, NULL }
#define F64MATMUL_SVE_INSNC(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS, CONSTRAINTS, TIED) \
  { NAME, OPCODE, MASK, CLASS, 0, F64MM_SVE, OPS, QUALS, FLAGS, CONSTRAINTS, TIED, NULL }
#define F32MATMUL_SVE_INSNC(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS, CONSTRAINTS, TIED) \
  { NAME, OPCODE, MASK, CLASS, 0, F32MM_SVE, OPS, QUALS, FLAGS, CONSTRAINTS, TIED, NULL }
#define V8_R_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, ARMV8_R, OPS, QUALS, FLAGS, 0, 0, NULL }
#define V8_7_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, ARMV8_7, OPS, QUALS, FLAGS, 0, 0, NULL }
#define _LS64_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, LS64, OPS, QUALS, FLAGS, 0, 0, NULL }
#define FLAGM_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, FLAGM, OPS, QUALS, FLAGS, 0, 0, NULL }
#define MOPS_INSN(NAME, OPCODE, MASK, CLASS, OPS, QUALS, FLAGS, CONSTRAINTS, VERIFIER) \
  { NAME, OPCODE, MASK, CLASS, 0, MOPS, OPS, QUALS, FLAGS, CONSTRAINTS, \
    0, VERIFIER }
#define MOPS_MEMTAG_INSN(NAME, OPCODE, MASK, CLASS, OPS, QUALS, FLAGS, CONSTRAINTS, VERIFIER) \
  { NAME, OPCODE, MASK, CLASS, 0, MOPS_MEMTAG, OPS, QUALS, FLAGS, \
    CONSTRAINTS, 0, VERIFIER }
#define HBC_INSN(NAME,OPCODE,MASK,CLASS,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, CLASS, 0, HBC, OPS, QUALS, FLAGS, 0, 0, NULL }
#define CSSC_INSN(NAME,OPCODE,MASK,OPS,QUALS,FLAGS) \
  { NAME, OPCODE, MASK, cssc, 0, CSSC, OPS, QUALS, FLAGS, 0, 0, NULL }

#define MOPS_CPY_OP1_OP2_PME_INSN(NAME, OPCODE, MASK, FLAGS, CONSTRAINTS) \
  MOPS_INSN (NAME, OPCODE, MASK, 0, \
	     OP3 (MOPS_ADDR_Rd, MOPS_ADDR_Rs, MOPS_WB_Rn), QL_I3SAMEX, \
	     FLAGS, CONSTRAINTS, VERIFIER (three_different_regs))

/* These instructions must remain consecutive, since we rely on the order
   when detecting invalid sequences.  */
#define MOPS_CPY_OP1_OP2_INSN(NAME, SUFFIX, OPCODE, MASK) \
  MOPS_CPY_OP1_OP2_PME_INSN (NAME "p" SUFFIX, OPCODE, MASK, F_SCAN, \
			     C_SCAN_MOPS_P), \
  MOPS_CPY_OP1_OP2_PME_INSN (NAME "m" SUFFIX, OPCODE | 0x400000, MASK, \
			     0, C_SCAN_MOPS_M), \
  MOPS_CPY_OP1_OP2_PME_INSN (NAME "e" SUFFIX, OPCODE | 0x800000, MASK, \
			     0, C_SCAN_MOPS_E)

#define MOPS_CPY_OP1_INSN(NAME, SUFFIX, OPCODE, MASK) \
  MOPS_CPY_OP1_OP2_INSN (NAME, SUFFIX, OPCODE, MASK), \
  MOPS_CPY_OP1_OP2_INSN (NAME, SUFFIX "wn", OPCODE | 0x4000, MASK), \
  MOPS_CPY_OP1_OP2_INSN (NAME, SUFFIX "rn", OPCODE | 0x8000, MASK), \
  MOPS_CPY_OP1_OP2_INSN (NAME, SUFFIX "n", OPCODE | 0xc000, MASK)

#define MOPS_CPY_INSN(NAME, OPCODE, MASK) \
  MOPS_CPY_OP1_INSN (NAME, "", OPCODE, MASK), \
  MOPS_CPY_OP1_INSN (NAME, "wt", OPCODE | 0x1000, MASK), \
  MOPS_CPY_OP1_INSN (NAME, "rt", OPCODE | 0x2000, MASK), \
  MOPS_CPY_OP1_INSN (NAME, "t", OPCODE | 0x3000, MASK)

#define MOPS_SET_OP1_OP2_PME_INSN(NAME, OPCODE, MASK, FLAGS, CONSTRAINTS, ISA) \
  ISA (NAME, OPCODE, MASK, 0, \
       OP3 (MOPS_ADDR_Rd, MOPS_WB_Rn, Rm), QL_I3SAMEX, FLAGS, \
       CONSTRAINTS, VERIFIER (three_different_regs))

/* These instructions must remain consecutive, since we rely on the order
   when detecting invalid sequences.  */
#define MOPS_SET_OP1_OP2_INSN(NAME, SUFFIX, OPCODE, MASK, ISA) \
  MOPS_SET_OP1_OP2_PME_INSN (NAME "p" SUFFIX, OPCODE, MASK, \
			     F_SCAN, C_SCAN_MOPS_P, ISA), \
  MOPS_SET_OP1_OP2_PME_INSN (NAME "m" SUFFIX, OPCODE | 0x4000, MASK, \
			     0, C_SCAN_MOPS_M, ISA), \
  MOPS_SET_OP1_OP2_PME_INSN (NAME "e" SUFFIX, OPCODE | 0x8000, MASK, \
			     0, C_SCAN_MOPS_E, ISA)

#define MOPS_SET_INSN(NAME, OPCODE, MASK, ISA) \
  MOPS_SET_OP1_OP2_INSN (NAME, "", OPCODE, MASK, ISA), \
  MOPS_SET_OP1_OP2_INSN (NAME, "t", OPCODE | 0x1000, MASK, ISA), \
  MOPS_SET_OP1_OP2_INSN (NAME, "n", OPCODE | 0x2000, MASK, ISA), \
  MOPS_SET_OP1_OP2_INSN (NAME, "tn", OPCODE | 0x3000, MASK, ISA)

const struct aarch64_opcode aarch64_opcode_table[] =
{
  /* Add/subtract (with carry).  */
  CORE_INSN ("adc",  0x1a000000, 0x7fe0fc00, addsub_carry, 0, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF),
  CORE_INSN ("adcs", 0x3a000000, 0x7fe0fc00, addsub_carry, 0, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF),
  CORE_INSN ("sbc",  0x5a000000, 0x7fe0fc00, addsub_carry, 0, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_HAS_ALIAS | F_SF),
  CORE_INSN ("ngc",  0x5a0003e0, 0x7fe0ffe0, addsub_carry, 0, OP2 (Rd, Rm),     QL_I2SAME,  F_ALIAS | F_SF),
  CORE_INSN ("sbcs", 0x7a000000, 0x7fe0fc00, addsub_carry, 0, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_HAS_ALIAS | F_SF),
  CORE_INSN ("ngcs", 0x7a0003e0, 0x7fe0ffe0, addsub_carry, 0, OP2 (Rd, Rm),     QL_I2SAME,  F_ALIAS | F_SF),
  /* Add/subtract (extended register).  */
  CORE_INSN ("add",  0x0b200000, 0x7fe00000, addsub_ext, 0, OP3 (Rd_SP, Rn_SP, Rm_EXT), QL_I3_EXT, F_SF),
  CORE_INSN ("adds", 0x2b200000, 0x7fe00000, addsub_ext, 0, OP3 (Rd, Rn_SP, Rm_EXT),    QL_I3_EXT, F_HAS_ALIAS | F_SF),
  CORE_INSN ("cmn",  0x2b20001f, 0x7fe0001f, addsub_ext, 0, OP2 (Rn_SP, Rm_EXT),        QL_I2_EXT, F_ALIAS | F_SF),
  CORE_INSN ("sub",  0x4b200000, 0x7fe00000, addsub_ext, 0, OP3 (Rd_SP, Rn_SP, Rm_EXT), QL_I3_EXT, F_SF),
  CORE_INSN ("subs", 0x6b200000, 0x7fe00000, addsub_ext, 0, OP3 (Rd, Rn_SP, Rm_EXT),    QL_I3_EXT, F_HAS_ALIAS | F_SF),
  CORE_INSN ("cmp",  0x6b20001f, 0x7fe0001f, addsub_ext, 0, OP2 (Rn_SP, Rm_EXT),        QL_I2_EXT, F_ALIAS | F_SF),
  /* Add/subtract (immediate).  */
  CORE_INSN ("add",  0x11000000, 0x7f000000, addsub_imm, OP_ADD, OP3 (Rd_SP, Rn_SP, AIMM), QL_R2NIL, F_HAS_ALIAS | F_SF),
  CORE_INSN ("mov",  0x11000000, 0x7ffffc00, addsub_imm, 0, OP2 (Rd_SP, Rn_SP),       QL_I2SP, F_ALIAS | F_SF),
  CORE_INSN ("adds", 0x31000000, 0x7f000000, addsub_imm, 0, OP3 (Rd, Rn_SP, AIMM),    QL_R2NIL, F_HAS_ALIAS | F_SF),
  CORE_INSN ("cmn",  0x3100001f, 0x7f00001f, addsub_imm, 0, OP2 (Rn_SP, AIMM),        QL_R1NIL, F_ALIAS | F_SF),
  CORE_INSN ("sub",  0x51000000, 0x7f000000, addsub_imm, 0, OP3 (Rd_SP, Rn_SP, AIMM), QL_R2NIL, F_SF),
  CORE_INSN ("subs", 0x71000000, 0x7f000000, addsub_imm, 0, OP3 (Rd, Rn_SP, AIMM),    QL_R2NIL, F_HAS_ALIAS | F_SF),
  CORE_INSN ("cmp",  0x7100001f, 0x7f00001f, addsub_imm, 0, OP2 (Rn_SP, AIMM),        QL_R1NIL, F_ALIAS | F_SF),
  MEMTAG_INSN ("addg",  0x91800000, 0xffc0c000, addsub_imm, OP4 (Rd_SP, Rn_SP, UIMM10, UIMM4_ADDG), QL_ADDG, 0),
  MEMTAG_INSN ("subg",  0xd1800000, 0xffc0c000, addsub_imm, OP4 (Rd_SP, Rn_SP, UIMM10, UIMM4_ADDG), QL_ADDG, 0),
  /* Add/subtract (shifted register).  */
  CORE_INSN ("add",  0x0b000000, 0x7f200000, addsub_shift, 0, OP3 (Rd, Rn, Rm_SFT), QL_I3SAMER, F_SF),
  CORE_INSN ("adds", 0x2b000000, 0x7f200000, addsub_shift, 0, OP3 (Rd, Rn, Rm_SFT), QL_I3SAMER, F_HAS_ALIAS | F_SF),
  CORE_INSN ("cmn",  0x2b00001f, 0x7f20001f, addsub_shift, 0, OP2 (Rn, Rm_SFT),     QL_I2SAME,  F_ALIAS | F_SF),
  CORE_INSN ("sub",  0x4b000000, 0x7f200000, addsub_shift, 0, OP3 (Rd, Rn, Rm_SFT), QL_I3SAMER, F_HAS_ALIAS | F_SF),
  CORE_INSN ("neg",  0x4b0003e0, 0x7f2003e0, addsub_shift, 0, OP2 (Rd, Rm_SFT),     QL_I2SAME,  F_ALIAS | F_SF),
  CORE_INSN ("subs", 0x6b000000, 0x7f200000, addsub_shift, 0, OP3 (Rd, Rn, Rm_SFT), QL_I3SAMER, F_HAS_ALIAS | F_SF),
  CORE_INSN ("cmp",  0x6b00001f, 0x7f20001f, addsub_shift, 0, OP2 (Rn, Rm_SFT),     QL_I2SAME,  F_ALIAS | F_SF | F_P1),
  CORE_INSN ("negs", 0x6b0003e0, 0x7f2003e0, addsub_shift, 0, OP2 (Rd, Rm_SFT),     QL_I2SAME,  F_ALIAS | F_SF),
  /* AdvSIMD across lanes.  */
  SIMD_INSN ("saddlv", 0x0e303800, 0xbf3ffc00, asimdall, 0, OP2 (Fd, Vn), QL_XLANES_L,    F_SIZEQ),
  SIMD_INSN ("smaxv",  0x0e30a800, 0xbf3ffc00, asimdall, 0, OP2 (Fd, Vn), QL_XLANES,      F_SIZEQ),
  SIMD_INSN ("sminv",  0x0e31a800, 0xbf3ffc00, asimdall, 0, OP2 (Fd, Vn), QL_XLANES,      F_SIZEQ),
  SIMD_INSN ("addv",   0x0e31b800, 0xbf3ffc00, asimdall, 0, OP2 (Fd, Vn), QL_XLANES,      F_SIZEQ),
  SIMD_INSN ("uaddlv", 0x2e303800, 0xbf3ffc00, asimdall, 0, OP2 (Fd, Vn), QL_XLANES_L,    F_SIZEQ),
  SIMD_INSN ("umaxv",  0x2e30a800, 0xbf3ffc00, asimdall, 0, OP2 (Fd, Vn), QL_XLANES,      F_SIZEQ),
  SIMD_INSN ("uminv",  0x2e31a800, 0xbf3ffc00, asimdall, 0, OP2 (Fd, Vn), QL_XLANES,      F_SIZEQ),
  SIMD_INSN ("fmaxnmv",0x2e30c800, 0xbfbffc00, asimdall, 0, OP2 (Fd, Vn), QL_XLANES_FP,   F_SIZEQ),
  SF16_INSN ("fmaxnmv",0x0e30c800, 0xbffffc00, asimdall, OP2 (Fd, Vn), QL_XLANES_FP_H, F_SIZEQ),
  SIMD_INSN ("fmaxv",  0x2e30f800, 0xbfbffc00, asimdall, 0, OP2 (Fd, Vn), QL_XLANES_FP,   F_SIZEQ),
  SF16_INSN ("fmaxv",  0x0e30f800, 0xbffffc00, asimdall, OP2 (Fd, Vn), QL_XLANES_FP_H, F_SIZEQ),
  SIMD_INSN ("fminnmv",0x2eb0c800, 0xbfbffc00, asimdall, 0, OP2 (Fd, Vn), QL_XLANES_FP,   F_SIZEQ),
  SF16_INSN ("fminnmv",0x0eb0c800, 0xbffffc00, asimdall, OP2 (Fd, Vn), QL_XLANES_FP_H, F_SIZEQ),
  SIMD_INSN ("fminv",  0x2eb0f800, 0xbfbffc00, asimdall, 0, OP2 (Fd, Vn), QL_XLANES_FP,   F_SIZEQ),
  SF16_INSN ("fminv",  0x0eb0f800, 0xbffffc00, asimdall, OP2 (Fd, Vn), QL_XLANES_FP_H, F_SIZEQ),
  /* AdvSIMD three different.  */
  SIMD_INSN ("saddl",   0x0e200000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS,  F_SIZEQ),
  SIMD_INSN ("saddl2",  0x4e200000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS2, F_SIZEQ),
  SIMD_INSN ("saddw",   0x0e201000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3WIDEBHS,  F_SIZEQ),
  SIMD_INSN ("saddw2",  0x4e201000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3WIDEBHS2, F_SIZEQ),
  SIMD_INSN ("ssubl",   0x0e202000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS,  F_SIZEQ),
  SIMD_INSN ("ssubl2",  0x4e202000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS2, F_SIZEQ),
  SIMD_INSN ("ssubw",   0x0e203000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3WIDEBHS,  F_SIZEQ),
  SIMD_INSN ("ssubw2",  0x4e203000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3WIDEBHS2, F_SIZEQ),
  SIMD_INSN ("addhn",   0x0e204000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3NARRBHS,  F_SIZEQ),
  SIMD_INSN ("addhn2",  0x4e204000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3NARRBHS2, F_SIZEQ),
  SIMD_INSN ("sabal",   0x0e205000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS,  F_SIZEQ),
  SIMD_INSN ("sabal2",  0x4e205000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS2, F_SIZEQ),
  SIMD_INSN ("subhn",   0x0e206000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3NARRBHS,  F_SIZEQ),
  SIMD_INSN ("subhn2",  0x4e206000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3NARRBHS2, F_SIZEQ),
  SIMD_INSN ("sabdl",   0x0e207000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS,  F_SIZEQ),
  SIMD_INSN ("sabdl2",  0x4e207000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS2, F_SIZEQ),
  SIMD_INSN ("smlal",   0x0e208000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS,  F_SIZEQ),
  SIMD_INSN ("smlal2",  0x4e208000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS2, F_SIZEQ),
  SIMD_INSN ("sqdmlal", 0x0e209000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGHS,   F_SIZEQ),
  SIMD_INSN ("sqdmlal2",0x4e209000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGHS2,  F_SIZEQ),
  SIMD_INSN ("smlsl",   0x0e20a000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS,  F_SIZEQ),
  SIMD_INSN ("smlsl2",  0x4e20a000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS2, F_SIZEQ),
  SIMD_INSN ("sqdmlsl", 0x0e20b000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGHS,   F_SIZEQ),
  SIMD_INSN ("sqdmlsl2",0x4e20b000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGHS2,  F_SIZEQ),
  SIMD_INSN ("smull",   0x0e20c000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS,  F_SIZEQ),
  SIMD_INSN ("smull2",  0x4e20c000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS2, F_SIZEQ),
  SIMD_INSN ("sqdmull", 0x0e20d000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGHS,   F_SIZEQ),
  SIMD_INSN ("sqdmull2",0x4e20d000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGHS2,  F_SIZEQ),
  SIMD_INSN ("pmull",   0x0e20e000, 0xffe0fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGB,    0),
  AES_INSN ("pmull",   0x0ee0e000, 0xffe0fc00, asimddiff, OP3 (Vd, Vn, Vm), QL_V3LONGD,    0),
  SIMD_INSN ("pmull2",  0x4e20e000, 0xffe0fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGB2,   0),
  AES_INSN ("pmull2",  0x4ee0e000, 0xffe0fc00, asimddiff, OP3 (Vd, Vn, Vm), QL_V3LONGD2,   0),
  SIMD_INSN ("uaddl",   0x2e200000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS,  F_SIZEQ),
  SIMD_INSN ("uaddl2",  0x6e200000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS2, F_SIZEQ),
  SIMD_INSN ("uaddw",   0x2e201000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3WIDEBHS,  F_SIZEQ),
  SIMD_INSN ("uaddw2",  0x6e201000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3WIDEBHS2, F_SIZEQ),
  SIMD_INSN ("usubl",   0x2e202000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS,  F_SIZEQ),
  SIMD_INSN ("usubl2",  0x6e202000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS2, F_SIZEQ),
  SIMD_INSN ("usubw",   0x2e203000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3WIDEBHS,  F_SIZEQ),
  SIMD_INSN ("usubw2",  0x6e203000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3WIDEBHS2, F_SIZEQ),
  SIMD_INSN ("raddhn",  0x2e204000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3NARRBHS,  F_SIZEQ),
  SIMD_INSN ("raddhn2", 0x6e204000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3NARRBHS2, F_SIZEQ),
  SIMD_INSN ("uabal",   0x2e205000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS,  F_SIZEQ),
  SIMD_INSN ("uabal2",  0x6e205000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS2, F_SIZEQ),
  SIMD_INSN ("rsubhn",  0x2e206000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3NARRBHS,  F_SIZEQ),
  SIMD_INSN ("rsubhn2", 0x6e206000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3NARRBHS2, F_SIZEQ),
  SIMD_INSN ("uabdl",   0x2e207000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS,  F_SIZEQ),
  SIMD_INSN ("uabdl2",  0x6e207000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS2, F_SIZEQ),
  SIMD_INSN ("umlal",   0x2e208000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS,  F_SIZEQ),
  SIMD_INSN ("umlal2",  0x6e208000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS2, F_SIZEQ),
  SIMD_INSN ("umlsl",   0x2e20a000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS,  F_SIZEQ),
  SIMD_INSN ("umlsl2",  0x6e20a000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS2, F_SIZEQ),
  SIMD_INSN ("umull",   0x2e20c000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS,  F_SIZEQ),
  SIMD_INSN ("umull2",  0x6e20c000, 0xff20fc00, asimddiff, 0, OP3 (Vd, Vn, Vm), QL_V3LONGBHS2, F_SIZEQ),
  /* AdvSIMD vector x indexed element.  */
  SIMD_INSN ("smlal",   0x0f002000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L,    F_SIZEQ),
  SIMD_INSN ("smlal2",  0x4f002000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L2,   F_SIZEQ),
  SIMD_INSN ("sqdmlal", 0x0f003000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L,    F_SIZEQ),
  SIMD_INSN ("sqdmlal2",0x4f003000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L2,   F_SIZEQ),
  SIMD_INSN ("smlsl",   0x0f006000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L,    F_SIZEQ),
  SIMD_INSN ("smlsl2",  0x4f006000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L2,   F_SIZEQ),
  SIMD_INSN ("sqdmlsl", 0x0f007000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L,    F_SIZEQ),
  SIMD_INSN ("sqdmlsl2",0x4f007000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L2,   F_SIZEQ),
  SIMD_INSN ("mul",     0x0f008000, 0xbf00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT,      F_SIZEQ),
  SIMD_INSN ("smull",   0x0f00a000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L,    F_SIZEQ),
  SIMD_INSN ("smull2",  0x4f00a000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L2,   F_SIZEQ),
  SIMD_INSN ("sqdmull", 0x0f00b000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L,    F_SIZEQ),
  SIMD_INSN ("sqdmull2",0x4f00b000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L2,   F_SIZEQ),
  SIMD_INSN ("sqdmulh", 0x0f00c000, 0xbf00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT,      F_SIZEQ),
  SIMD_INSN ("sqrdmulh",0x0f00d000, 0xbf00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT,      F_SIZEQ),
  _SIMD_INSN ("fmla",    0x0f801000, 0xbf80f400, asimdelem, 0, OP3 (Vd, Vn, Em), QL_ELEMENT_FP,  F_SIZEQ, VERIFIER (elem_sd)),
  SF16_INSN ("fmla",    0x0f001000, 0xbfc0f400, asimdelem, OP3 (Vd, Vn, Em16), QL_ELEMENT_FP_H, F_SIZEQ),
  _SIMD_INSN ("fmls",    0x0f805000, 0xbf80f400, asimdelem, 0, OP3 (Vd, Vn, Em), QL_ELEMENT_FP,   F_SIZEQ, VERIFIER (elem_sd)),
  SF16_INSN ("fmls",    0x0f005000, 0xbfc0f400, asimdelem, OP3 (Vd, Vn, Em16), QL_ELEMENT_FP_H, F_SIZEQ),
  _SIMD_INSN ("fmul",    0x0f809000, 0xbf80f400, asimdelem, 0, OP3 (Vd, Vn, Em), QL_ELEMENT_FP,   F_SIZEQ, VERIFIER (elem_sd)),
  SF16_INSN ("fmul",    0x0f009000, 0xbfc0f400, asimdelem, OP3 (Vd, Vn, Em16), QL_ELEMENT_FP_H, F_SIZEQ),
  SIMD_INSN ("mla",     0x2f000000, 0xbf00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT,      F_SIZEQ),
  SIMD_INSN ("umlal",   0x2f002000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L,    F_SIZEQ),
  SIMD_INSN ("umlal2",  0x6f002000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L2,   F_SIZEQ),
  SIMD_INSN ("mls",     0x2f004000, 0xbf00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT,      F_SIZEQ),
  SIMD_INSN ("umlsl",   0x2f006000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L,    F_SIZEQ),
  SIMD_INSN ("umlsl2",  0x6f006000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L2,   F_SIZEQ),
  SIMD_INSN ("umull",   0x2f00a000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L,    F_SIZEQ),
  SIMD_INSN ("umull2",  0x6f00a000, 0xff00f400, asimdelem, 0, OP3 (Vd, Vn, Em16), QL_ELEMENT_L2,   F_SIZEQ),
  _SIMD_INSN ("fmulx",   0x2f809000, 0xbf80f400, asimdelem, 0, OP3 (Vd, Vn, Em), QL_ELEMENT_FP,   F_SIZEQ, VERIFIER (elem_sd)),
  SF16_INSN ("fmulx",   0x2f009000, 0xbfc0f400, asimdelem, OP3 (Vd, Vn, Em16), QL_ELEMENT_FP_H, F_SIZEQ),
  RDMA_INSN ("sqrdmlah",0x2f00d000, 0xbf00f400, asimdelem, OP3 (Vd, Vn, Em16), QL_ELEMENT,      F_SIZEQ),
  RDMA_INSN ("sqrdmlsh",0x2f00f000, 0xbf00f400, asimdelem, OP3 (Vd, Vn, Em16), QL_ELEMENT,      F_SIZEQ),
  CNUM_INSN ("fcmla", 0x2f001000, 0xbf009400, asimdelem, OP_FCMLA_ELEM, OP4 (Vd, Vn, Em, IMM_ROT2), QL_ELEMENT_ROT, F_SIZEQ),
  /* AdvSIMD EXT.  */
  SIMD_INSN ("ext",  0x2e000000, 0xbfe08400, asimdext, 0, OP4 (Vd, Vn, Vm, IDX), QL_VEXT, F_SIZEQ),
  /* AdvSIMD modified immediate.  */
  SIMD_INSN ("movi", 0x0f000400, 0xbff89c00, asimdimm, 0, OP2 (Vd, SIMD_IMM_SFT), QL_SIMD_IMM_S0W, F_SIZEQ),
  SIMD_INSN ("orr",  0x0f001400, 0xbff89c00, asimdimm, 0, OP2 (Vd, SIMD_IMM_SFT), QL_SIMD_IMM_S0W, F_SIZEQ),
  SIMD_INSN ("movi", 0x0f008400, 0xbff8dc00, asimdimm, 0, OP2 (Vd, SIMD_IMM_SFT), QL_SIMD_IMM_S0H, F_SIZEQ),
  SIMD_INSN ("orr",  0x0f009400, 0xbff8dc00, asimdimm, 0, OP2 (Vd, SIMD_IMM_SFT), QL_SIMD_IMM_S0H, F_SIZEQ),
  SIMD_INSN ("movi", 0x0f00c400, 0xbff8ec00, asimdimm, 0, OP2 (Vd, SIMD_IMM_SFT), QL_SIMD_IMM_S1W, F_SIZEQ),
  SIMD_INSN ("movi", 0x0f00e400, 0xbff8fc00, asimdimm, 0, OP2 (Vd, SIMD_IMM_SFT), QL_SIMD_IMM_B, F_SIZEQ),
  SIMD_INSN ("fmov", 0x0f00f400, 0xbff8fc00, asimdimm, 0, OP2 (Vd, SIMD_FPIMM), QL_SIMD_IMM_S, F_SIZEQ),
  SF16_INSN ("fmov", 0x0f00fc00, 0xbff8fc00, asimdimm, OP2 (Vd, SIMD_FPIMM), QL_SIMD_IMM_H, F_SIZEQ),
  SIMD_INSN ("mvni", 0x2f000400, 0xbff89c00, asimdimm, 0, OP2 (Vd, SIMD_IMM_SFT), QL_SIMD_IMM_S0W, F_SIZEQ),
  SIMD_INSN ("bic",  0x2f001400, 0xbff89c00, asimdimm, 0, OP2 (Vd, SIMD_IMM_SFT), QL_SIMD_IMM_S0W, F_SIZEQ),
  SIMD_INSN ("mvni", 0x2f008400, 0xbff8dc00, asimdimm, 0, OP2 (Vd, SIMD_IMM_SFT), QL_SIMD_IMM_S0H, F_SIZEQ),
  SIMD_INSN ("bic",  0x2f009400, 0xbff8dc00, asimdimm, 0, OP2 (Vd, SIMD_IMM_SFT), QL_SIMD_IMM_S0H, F_SIZEQ),
  SIMD_INSN ("mvni", 0x2f00c400, 0xbff8ec00, asimdimm, 0, OP2 (Vd, SIMD_IMM_SFT), QL_SIMD_IMM_S1W, F_SIZEQ),
  SIMD_INSN ("movi", 0x2f00e400, 0xfff8fc00, asimdimm, 0, OP2 (Sd, SIMD_IMM), QL_SIMD_IMM_D, F_SIZEQ),
  SIMD_INSN ("movi", 0x6f00e400, 0xfff8fc00, asimdimm, 0, OP2 (Vd, SIMD_IMM), QL_SIMD_IMM_V2D, F_SIZEQ),
  SIMD_INSN ("fmov", 0x6f00f400, 0xfff8fc00, asimdimm, 0, OP2 (Vd, SIMD_FPIMM), QL_SIMD_IMM_V2D, F_SIZEQ),
  /* AdvSIMD copy.  */
  SIMD_INSN ("dup", 0x0e000400, 0xbfe0fc00, asimdins, 0, OP2 (Vd, En), QL_DUP_VX, F_T),
  SIMD_INSN ("dup", 0x0e000c00, 0xbfe0fc00, asimdins, 0, OP2 (Vd, Rn), QL_DUP_VR, F_T),
  SIMD_INSN ("smov",0x0e002c00, 0xbfe0fc00, asimdins, 0, OP2 (Rd, En), QL_SMOV, F_GPRSIZE_IN_Q),
  SIMD_INSN ("umov",0x0e003c00, 0xbfe0fc00, asimdins, 0, OP2 (Rd, En), QL_UMOV, F_HAS_ALIAS | F_GPRSIZE_IN_Q),
  SIMD_INSN ("mov", 0x0e003c00, 0xbfe0fc00, asimdins, 0, OP2 (Rd, En), QL_MOV, F_ALIAS | F_GPRSIZE_IN_Q),
  SIMD_INSN ("ins", 0x4e001c00, 0xffe0fc00, asimdins, 0, OP2 (Ed, Rn), QL_INS_XR, F_HAS_ALIAS),
  SIMD_INSN ("mov", 0x4e001c00, 0xffe0fc00, asimdins, 0, OP2 (Ed, Rn), QL_INS_XR, F_ALIAS),
  SIMD_INSN ("ins", 0x6e000400, 0xffe08400, asimdins, 0, OP2 (Ed, En), QL_S_2SAME, F_HAS_ALIAS),
  SIMD_INSN ("mov", 0x6e000400, 0xffe08400, asimdins, 0, OP2 (Ed, En), QL_S_2SAME, F_ALIAS),
  /* AdvSIMD two-reg misc.  */
  FRINTTS_INSN ("frint32z", 0x0e21e800, 0xbfbffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  FRINTTS_INSN ("frint32x", 0x2e21e800, 0xbfbffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  FRINTTS_INSN ("frint64z", 0x0e21f800, 0xbfbffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  FRINTTS_INSN ("frint64x", 0x2e21f800, 0xbfbffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SIMD_INSN ("rev64", 0x0e200800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMEBHS, F_SIZEQ),
  SIMD_INSN ("rev16", 0x0e201800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMEB, F_SIZEQ),
  SIMD_INSN ("saddlp",0x0e202800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2PAIRWISELONGBHS, F_SIZEQ),
  SIMD_INSN ("suqadd",0x0e203800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAME, F_SIZEQ),
  SIMD_INSN ("cls",   0x0e204800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMEBHS, F_SIZEQ),
  SIMD_INSN ("cnt",   0x0e205800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMEB, F_SIZEQ),
  SIMD_INSN ("sadalp",0x0e206800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2PAIRWISELONGBHS, F_SIZEQ),
  SIMD_INSN ("sqabs", 0x0e207800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAME, F_SIZEQ),
  SIMD_INSN ("cmgt",  0x0e208800, 0xbf3ffc00, asimdmisc, 0, OP3 (Vd, Vn, IMM0), QL_V2SAME, F_SIZEQ),
  SIMD_INSN ("cmeq",  0x0e209800, 0xbf3ffc00, asimdmisc, 0, OP3 (Vd, Vn, IMM0), QL_V2SAME, F_SIZEQ),
  SIMD_INSN ("cmlt",  0x0e20a800, 0xbf3ffc00, asimdmisc, 0, OP3 (Vd, Vn, IMM0), QL_V2SAME, F_SIZEQ),
  SIMD_INSN ("abs",   0x0e20b800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAME, F_SIZEQ),
  SIMD_INSN ("xtn",   0x0e212800, 0xff3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2NARRBHS, F_SIZEQ),
  SIMD_INSN ("xtn2",  0x4e212800, 0xff3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2NARRBHS2, F_SIZEQ),
  SIMD_INSN ("sqxtn", 0xe214800, 0xff3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2NARRBHS, F_SIZEQ),
  SIMD_INSN ("sqxtn2",0x4e214800, 0xff3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2NARRBHS2, F_SIZEQ),
  SIMD_INSN ("fcvtn",  0x0e216800, 0xffbffc00, asimdmisc, OP_FCVTN,  OP2 (Vd, Vn), QL_V2NARRHS,  F_MISC),
  SIMD_INSN ("fcvtn2", 0x4e216800, 0xffbffc00, asimdmisc, OP_FCVTN2, OP2 (Vd, Vn), QL_V2NARRHS2, F_MISC),
  SIMD_INSN ("fcvtl",  0x0e217800, 0xffbffc00, asimdmisc, OP_FCVTL,  OP2 (Vd, Vn), QL_V2LONGHS,  F_MISC),
  SIMD_INSN ("fcvtl2", 0x4e217800, 0xffbffc00, asimdmisc, OP_FCVTL2, OP2 (Vd, Vn), QL_V2LONGHS2, F_MISC),
  SIMD_INSN ("frintn", 0x0e218800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("frintn", 0x0e798800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("frintm", 0x0e219800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("frintm", 0x0e799800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fcvtns", 0x0e21a800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fcvtns", 0x0e79a800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fcvtms", 0x0e21b800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fcvtms", 0x0e79b800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fcvtas", 0x0e21c800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fcvtas", 0x0e79c800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("scvtf",  0x0e21d800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("scvtf",  0x0e79d800, 0xbfbffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fcmgt",  0x0ea0c800, 0xbfbffc00, asimdmisc, 0, OP3 (Vd, Vn, FPIMM0), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fcmgt",  0x0ef8c800, 0xbffffc00, asimdmisc, OP3 (Vd, Vn, FPIMM0), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fcmeq",  0x0ea0d800, 0xbfbffc00, asimdmisc, 0, OP3 (Vd, Vn, FPIMM0), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fcmeq",  0x0ef8d800, 0xbffffc00, asimdmisc, OP3 (Vd, Vn, FPIMM0), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fcmlt",  0x0ea0e800, 0xbfbffc00, asimdmisc, 0, OP3 (Vd, Vn, FPIMM0), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fcmlt",  0x0ef8e800, 0xbffffc00, asimdmisc, OP3 (Vd, Vn, FPIMM0), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fabs",   0x0ea0f800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fabs",   0x0ef8f800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("frintp", 0x0ea18800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("frintp", 0x0ef98800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("frintz", 0x0ea19800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("frintz", 0x0ef99800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fcvtps", 0x0ea1a800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fcvtps", 0x0ef9a800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fcvtzs", 0x0ea1b800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fcvtzs", 0x0ef9b800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("urecpe", 0x0ea1c800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMES, F_SIZEQ),
  SIMD_INSN ("frecpe", 0x0ea1d800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("frecpe", 0x0ef9d800, 0xbfbffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("rev32",  0x2e200800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMEBH, F_SIZEQ),
  SIMD_INSN ("uaddlp", 0x2e202800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2PAIRWISELONGBHS, F_SIZEQ),
  SIMD_INSN ("usqadd", 0x2e203800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAME, F_SIZEQ),
  SIMD_INSN ("clz",    0x2e204800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMEBHS, F_SIZEQ),
  SIMD_INSN ("uadalp", 0x2e206800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2PAIRWISELONGBHS, F_SIZEQ),
  SIMD_INSN ("sqneg",  0x2e207800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAME, F_SIZEQ),
  SIMD_INSN ("cmge",   0x2e208800, 0xbf3ffc00, asimdmisc, 0, OP3 (Vd, Vn, IMM0), QL_V2SAME, F_SIZEQ),
  SIMD_INSN ("cmle",   0x2e209800, 0xbf3ffc00, asimdmisc, 0, OP3 (Vd, Vn, IMM0), QL_V2SAME, F_SIZEQ),
  SIMD_INSN ("neg",    0x2e20b800, 0xbf3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAME, F_SIZEQ),
  SIMD_INSN ("sqxtun", 0x2e212800, 0xff3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2NARRBHS, F_SIZEQ),
  SIMD_INSN ("sqxtun2",0x6e212800, 0xff3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2NARRBHS2, F_SIZEQ),
  SIMD_INSN ("shll",   0x2e213800, 0xff3ffc00, asimdmisc, 0, OP3 (Vd, Vn, SHLL_IMM), QL_V2LONGBHS, F_SIZEQ),
  SIMD_INSN ("shll2",  0x6e213800, 0xff3ffc00, asimdmisc, 0, OP3 (Vd, Vn, SHLL_IMM), QL_V2LONGBHS2, F_SIZEQ),
  SIMD_INSN ("uqxtn",  0x2e214800, 0xff3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2NARRBHS, F_SIZEQ),
  SIMD_INSN ("uqxtn2", 0x6e214800, 0xff3ffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2NARRBHS2, F_SIZEQ),
  SIMD_INSN ("fcvtxn", 0x2e616800, 0xfffffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2NARRS, 0),
  SIMD_INSN ("fcvtxn2",0x6e616800, 0xfffffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2NARRS2, 0),
  SIMD_INSN ("frinta", 0x2e218800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("frinta", 0x2e798800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("frintx", 0x2e219800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("frintx", 0x2e799800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fcvtnu", 0x2e21a800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fcvtnu", 0x2e79a800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fcvtmu", 0x2e21b800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fcvtmu", 0x2e79b800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fcvtau", 0x2e21c800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fcvtau", 0x2e79c800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("ucvtf",  0x2e21d800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("ucvtf",  0x2e79d800, 0xbfbffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("not",    0x2e205800, 0xbffffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMEB, F_SIZEQ | F_HAS_ALIAS),
  SIMD_INSN ("mvn",    0x2e205800, 0xbffffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMEB, F_SIZEQ | F_ALIAS),
  SIMD_INSN ("rbit",   0x2e605800, 0xbffffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMEB, F_SIZEQ),
  SIMD_INSN ("fcmge",  0x2ea0c800, 0xbfbffc00, asimdmisc, 0, OP3 (Vd, Vn, FPIMM0), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fcmge",  0x2ef8c800, 0xbffffc00, asimdmisc, OP3 (Vd, Vn, FPIMM0), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fcmle",  0x2ea0d800, 0xbfbffc00, asimdmisc, 0, OP3 (Vd, Vn, FPIMM0), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fcmle",  0x2ef8d800, 0xbffffc00, asimdmisc, OP3 (Vd, Vn, FPIMM0), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fneg",   0x2ea0f800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fneg",   0x2ef8f800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("frinti", 0x2ea19800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("frinti", 0x2ef99800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fcvtpu", 0x2ea1a800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fcvtpu", 0x2ef9a800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fcvtzu", 0x2ea1b800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fcvtzu", 0x2ef9b800, 0xbffffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("ursqrte",0x2ea1c800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMES, F_SIZEQ),
  SIMD_INSN ("frsqrte",0x2ea1d800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("frsqrte",0x2ef9d800, 0xbfbffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  SIMD_INSN ("fsqrt",  0x2ea1f800, 0xbfbffc00, asimdmisc, 0, OP2 (Vd, Vn), QL_V2SAMESD, F_SIZEQ),
  SF16_INSN ("fsqrt",  0x2ef9f800, 0xbfbffc00, asimdmisc, OP2 (Vd, Vn), QL_V2SAMEH, F_SIZEQ),
  /* AdvSIMD ZIP/UZP/TRN.  */
  SIMD_INSN ("uzp1", 0xe001800, 0xbf20fc00, asimdperm, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("trn1", 0xe002800, 0xbf20fc00, asimdperm, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("zip1", 0xe003800, 0xbf20fc00, asimdperm, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("uzp2", 0xe005800, 0xbf20fc00, asimdperm, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("trn2", 0xe006800, 0xbf20fc00, asimdperm, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("zip2", 0xe007800, 0xbf20fc00, asimdperm, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  /* AdvSIMD three same.  */
  SIMD_INSN ("shadd", 0xe200400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("sqadd", 0xe200c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("srhadd", 0xe201400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("shsub", 0xe202400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("sqsub", 0xe202c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("cmgt", 0xe203400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("cmge", 0xe203c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("sshl", 0xe204400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("sqshl", 0xe204c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("srshl", 0xe205400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("sqrshl", 0xe205c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("smax", 0xe206400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("smin", 0xe206c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("sabd", 0xe207400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("saba", 0xe207c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("add", 0xe208400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("cmtst", 0xe208c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("mla", 0xe209400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("mul", 0xe209c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("smaxp", 0xe20a400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("sminp", 0xe20ac00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("sqdmulh", 0xe20b400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEHS, F_SIZEQ),
  SIMD_INSN ("addp", 0xe20bc00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("fmaxnm", 0xe20c400, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fmaxnm", 0xe400400, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("fmla", 0xe20cc00, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fmla", 0xe400c00, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("fadd", 0xe20d400, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fadd", 0xe401400, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("fmulx", 0xe20dc00, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fmulx", 0xe401c00, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("fcmeq", 0xe20e400, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fcmeq", 0xe402400, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("fmax", 0xe20f400, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fmax", 0xe403400, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("frecps", 0xe20fc00, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("frecps", 0xe403c00, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("and", 0xe201c00, 0xbfe0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEB, F_SIZEQ),
  SIMD_INSN ("bic", 0xe601c00, 0xbfe0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEB, F_SIZEQ),
  SIMD_INSN ("fminnm", 0xea0c400, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fminnm", 0xec00400, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("fmls", 0xea0cc00, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fmls", 0xec00c00, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("fsub", 0xea0d400, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fsub", 0xec01400, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("fmin", 0xea0f400, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fmin", 0xec03400, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("frsqrts", 0xea0fc00, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("frsqrts", 0xec03c00, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("orr", 0xea01c00, 0xbfe0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEB, F_HAS_ALIAS | F_SIZEQ),
  SIMD_INSN ("mov", 0xea01c00, 0xbfe0fc00, asimdsame, OP_MOV_V, OP2 (Vd, Vn), QL_V2SAMEB, F_ALIAS | F_CONV),
  SIMD_INSN ("orn", 0xee01c00, 0xbfe0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEB, F_SIZEQ),
  SIMD_INSN ("uhadd", 0x2e200400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("uqadd", 0x2e200c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("urhadd", 0x2e201400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("uhsub", 0x2e202400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("uqsub", 0x2e202c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("cmhi", 0x2e203400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("cmhs", 0x2e203c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("ushl", 0x2e204400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("uqshl", 0x2e204c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("urshl", 0x2e205400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("uqrshl", 0x2e205c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("umax", 0x2e206400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("umin", 0x2e206c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("uabd", 0x2e207400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("uaba", 0x2e207c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("sub", 0x2e208400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("cmeq", 0x2e208c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAME, F_SIZEQ),
  SIMD_INSN ("mls", 0x2e209400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("pmul", 0x2e209c00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEB, F_SIZEQ),
  SIMD_INSN ("umaxp", 0x2e20a400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("uminp", 0x2e20ac00, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEBHS, F_SIZEQ),
  SIMD_INSN ("sqrdmulh", 0x2e20b400, 0xbf20fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEHS, F_SIZEQ),
  SIMD_INSN ("fmaxnmp", 0x2e20c400, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fmaxnmp", 0x2e400400, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("faddp", 0x2e20d400, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("faddp", 0x2e401400, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("fmul", 0x2e20dc00, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fmul", 0x2e401c00, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("fcmge", 0x2e20e400, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fcmge", 0x2e402400, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("facge", 0x2e20ec00, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("facge", 0x2e402c00, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("fmaxp", 0x2e20f400, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fmaxp", 0x2e403400, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("fdiv", 0x2e20fc00, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fdiv", 0x2e403c00, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("eor", 0x2e201c00, 0xbfe0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEB, F_SIZEQ),
  SIMD_INSN ("bsl", 0x2e601c00, 0xbfe0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEB, F_SIZEQ),
  SIMD_INSN ("fminnmp", 0x2ea0c400, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fminnmp", 0x2ec00400, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("fabd", 0x2ea0d400, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fabd", 0x2ec01400, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("fcmgt", 0x2ea0e400, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fcmgt", 0x2ec02400, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("facgt", 0x2ea0ec00, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("facgt", 0x2ec02c00, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("fminp", 0x2ea0f400, 0xbfa0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMESD, F_SIZEQ),
  SF16_INSN ("fminp", 0x2ec03400, 0xbfe0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEH, F_SIZEQ),
  SIMD_INSN ("bit", 0x2ea01c00, 0xbfe0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEB, F_SIZEQ),
  SIMD_INSN ("bif", 0x2ee01c00, 0xbfe0fc00, asimdsame, 0, OP3 (Vd, Vn, Vm), QL_V3SAMEB, F_SIZEQ),
  /* AdvSIMD three same extension.  */
  RDMA_INSN ("sqrdmlah",0x2e008400, 0xbf20fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEHS, F_SIZEQ),
  RDMA_INSN ("sqrdmlsh",0x2e008c00, 0xbf20fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3SAMEHS, F_SIZEQ),
  CNUM_INSN ("fcmla", 0x2e00c400, 0xbf20e400, asimdsame, 0, OP4 (Vd, Vn, Vm, IMM_ROT1), QL_V3SAMEHSD_ROT, F_SIZEQ),
  CNUM_INSN ("fcadd", 0x2e00e400, 0xbf20ec00, asimdsame, 0, OP4 (Vd, Vn, Vm, IMM_ROT3), QL_V3SAMEHSD_ROT, F_SIZEQ),
  /* AdvSIMD shift by immediate.  */
  SIMD_INSN ("sshr", 0xf000400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT, 0),
  SIMD_INSN ("ssra", 0xf001400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT, 0),
  SIMD_INSN ("srshr", 0xf002400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT, 0),
  SIMD_INSN ("srsra", 0xf003400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT, 0),
  SIMD_INSN ("shl", 0xf005400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSL), QL_VSHIFT, 0),
  SIMD_INSN ("sqshl", 0xf007400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSL), QL_VSHIFT, 0),
  SIMD_INSN ("shrn", 0xf008400, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN, 0),
  SIMD_INSN ("shrn2", 0x4f008400, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN2, 0),
  SIMD_INSN ("rshrn", 0xf008c00, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN, 0),
  SIMD_INSN ("rshrn2", 0x4f008c00, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN2, 0),
  SIMD_INSN ("sqshrn", 0xf009400, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN, 0),
  SIMD_INSN ("sqshrn2", 0x4f009400, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN2, 0),
  SIMD_INSN ("sqrshrn", 0xf009c00, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN, 0),
  SIMD_INSN ("sqrshrn2", 0x4f009c00, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN2, 0),
  SIMD_INSN ("sshll", 0xf00a400, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSL), QL_VSHIFTL, F_HAS_ALIAS),
  SIMD_INSN ("sxtl", 0xf00a400, 0xff87fc00, asimdshf, OP_SXTL, OP2 (Vd, Vn), QL_V2LONGBHS, F_ALIAS | F_CONV),
  SIMD_INSN ("sshll2", 0x4f00a400, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSL), QL_VSHIFTL2, F_HAS_ALIAS),
  SIMD_INSN ("sxtl2", 0x4f00a400, 0xff87fc00, asimdshf, OP_SXTL2, OP2 (Vd, Vn), QL_V2LONGBHS2, F_ALIAS | F_CONV),
  SIMD_INSN ("scvtf", 0xf00e400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT_SD, 0),
  SF16_INSN ("scvtf", 0xf10e400, 0xbf80fc00, asimdshf, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT_H, 0),
  SIMD_INSN ("fcvtzs", 0xf00fc00, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT_SD, 0),
  SF16_INSN ("fcvtzs", 0xf10fc00, 0xbf80fc00, asimdshf, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT_H, 0),
  SIMD_INSN ("ushr", 0x2f000400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT, 0),
  SIMD_INSN ("usra", 0x2f001400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT, 0),
  SIMD_INSN ("urshr", 0x2f002400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT, 0),
  SIMD_INSN ("ursra", 0x2f003400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT, 0),
  SIMD_INSN ("sri", 0x2f004400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT, 0),
  SIMD_INSN ("sli", 0x2f005400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSL), QL_VSHIFT, 0),
  SIMD_INSN ("sqshlu", 0x2f006400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSL), QL_VSHIFT, 0),
  SIMD_INSN ("uqshl", 0x2f007400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSL), QL_VSHIFT, 0),
  SIMD_INSN ("sqshrun", 0x2f008400, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN, 0),
  SIMD_INSN ("sqshrun2", 0x6f008400, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN2, 0),
  SIMD_INSN ("sqrshrun", 0x2f008c00, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN, 0),
  SIMD_INSN ("sqrshrun2", 0x6f008c00, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN2, 0),
  SIMD_INSN ("uqshrn", 0x2f009400, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN, 0),
  SIMD_INSN ("uqshrn2", 0x6f009400, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN2, 0),
  SIMD_INSN ("uqrshrn", 0x2f009c00, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN, 0),
  SIMD_INSN ("uqrshrn2", 0x6f009c00, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFTN2, 0),
  SIMD_INSN ("ushll", 0x2f00a400, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSL), QL_VSHIFTL, F_HAS_ALIAS),
  SIMD_INSN ("uxtl", 0x2f00a400, 0xff87fc00, asimdshf, OP_UXTL, OP2 (Vd, Vn), QL_V2LONGBHS, F_ALIAS | F_CONV),
  SIMD_INSN ("ushll2", 0x6f00a400, 0xff80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSL), QL_VSHIFTL2, F_HAS_ALIAS),
  SIMD_INSN ("uxtl2", 0x6f00a400, 0xff87fc00, asimdshf, OP_UXTL2, OP2 (Vd, Vn), QL_V2LONGBHS2, F_ALIAS | F_CONV),
  SIMD_INSN ("ucvtf", 0x2f00e400, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT_SD, 0),
  SF16_INSN ("ucvtf", 0x2f10e400, 0xbf80fc00, asimdshf, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT_H, 0),
  SIMD_INSN ("fcvtzu", 0x2f00fc00, 0xbf80fc00, asimdshf, 0, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT_SD, 0),
  SF16_INSN ("fcvtzu", 0x2f10fc00, 0xbf80fc00, asimdshf, OP3 (Vd, Vn, IMM_VLSR), QL_VSHIFT_H, 0),
  /* AdvSIMD TBL/TBX.  */
  SIMD_INSN ("tbl", 0xe000000, 0xbfe09c00, asimdtbl, 0, OP3 (Vd, LVn, Vm), QL_TABLE, F_SIZEQ),
  SIMD_INSN ("tbx", 0xe001000, 0xbfe09c00, asimdtbl, 0, OP3 (Vd, LVn, Vm), QL_TABLE, F_SIZEQ),
  /* AdvSIMD scalar three different.  */
  SIMD_INSN ("sqdmlal", 0x5e209000, 0xff20fc00, asisddiff, 0, OP3 (Sd, Sn, Sm), QL_SISDL_HS, F_SSIZE),
  SIMD_INSN ("sqdmlsl", 0x5e20b000, 0xff20fc00, asisddiff, 0, OP3 (Sd, Sn, Sm), QL_SISDL_HS, F_SSIZE),
  SIMD_INSN ("sqdmull", 0x5e20d000, 0xff20fc00, asisddiff, 0, OP3 (Sd, Sn, Sm), QL_SISDL_HS, F_SSIZE),
  /* AdvSIMD scalar x indexed element.  */
  SIMD_INSN ("sqdmlal", 0x5f003000, 0xff00f400, asisdelem, 0, OP3 (Sd, Sn, Em16), QL_SISDL_HS, F_SSIZE),
  SIMD_INSN ("sqdmlsl", 0x5f007000, 0xff00f400, asisdelem, 0, OP3 (Sd, Sn, Em16), QL_SISDL_HS, F_SSIZE),
  SIMD_INSN ("sqdmull", 0x5f00b000, 0xff00f400, asisdelem, 0, OP3 (Sd, Sn, Em16), QL_SISDL_HS, F_SSIZE),
  SIMD_INSN ("sqdmulh", 0x5f00c000, 0xff00f400, asisdelem, 0, OP3 (Sd, Sn, Em16), QL_SISD_HS, F_SSIZE),
  SIMD_INSN ("sqrdmulh", 0x5f00d000, 0xff00f400, asisdelem, 0, OP3 (Sd, Sn, Em16), QL_SISD_HS, F_SSIZE),
  _SIMD_INSN ("fmla", 0x5f801000, 0xff80f400, asisdelem, 0, OP3 (Sd, Sn, Em), QL_FP3, F_SSIZE, VERIFIER (elem_sd)),
  SF16_INSN ("fmla", 0x5f001000, 0xffc0f400, asisdelem, OP3 (Sd, Sn, Em16), QL_FP3_H, F_SSIZE),
  _SIMD_INSN ("fmls", 0x5f805000, 0xff80f400, asisdelem, 0, OP3 (Sd, Sn, Em), QL_FP3, F_SSIZE, VERIFIER (elem_sd)),
  SF16_INSN ("fmls", 0x5f005000, 0xffc0f400, asisdelem, OP3 (Sd, Sn, Em16), QL_FP3_H, F_SSIZE),
  _SIMD_INSN ("fmul", 0x5f809000, 0xff80f400, asisdelem, 0, OP3 (Sd, Sn, Em), QL_FP3, F_SSIZE, VERIFIER (elem_sd)),
  SF16_INSN ("fmul", 0x5f009000, 0xffc0f400, asisdelem, OP3 (Sd, Sn, Em16), QL_FP3_H, F_SSIZE),
  _SIMD_INSN ("fmulx", 0x7f809000, 0xff80f400, asisdelem, 0, OP3 (Sd, Sn, Em), QL_FP3, F_SSIZE, VERIFIER (elem_sd)),
  SF16_INSN ("fmulx", 0x7f009000, 0xffc0f400, asisdelem, OP3 (Sd, Sn, Em16), QL_FP3_H, F_SSIZE),
  RDMA_INSN ("sqrdmlah", 0x7f00d000, 0xff00f400, asisdelem, OP3 (Sd, Sn, Em16), QL_SISD_HS, F_SSIZE),
  RDMA_INSN ("sqrdmlsh", 0x7f00f000, 0xff00f400, asisdelem, OP3 (Sd, Sn, Em16), QL_SISD_HS, F_SSIZE),
  /* AdvSIMD load/store multiple structures.  */
  SIMD_INSN ("st4", 0xc000000, 0xbfff0000, asisdlse, 0, OP2 (LVt, SIMD_ADDR_SIMPLE), QL_SIMD_LDST, F_SIZEQ | F_OD(4)),
  SIMD_INSN ("st1", 0xc000000, 0xbfff0000, asisdlse, 0, OP2 (LVt, SIMD_ADDR_SIMPLE), QL_SIMD_LDST_ANY, F_SIZEQ | F_OD(1)),
  SIMD_INSN ("st2", 0xc000000, 0xbfff0000, asisdlse, 0, OP2 (LVt, SIMD_ADDR_SIMPLE), QL_SIMD_LDST, F_SIZEQ | F_OD(2)),
  SIMD_INSN ("st3", 0xc000000, 0xbfff0000, asisdlse, 0, OP2 (LVt, SIMD_ADDR_SIMPLE), QL_SIMD_LDST, F_SIZEQ | F_OD(3)),
  SIMD_INSN ("ld4", 0xc400000, 0xbfff0000, asisdlse, 0, OP2 (LVt, SIMD_ADDR_SIMPLE), QL_SIMD_LDST, F_SIZEQ | F_OD(4)),
  SIMD_INSN ("ld1", 0xc400000, 0xbfff0000, asisdlse, 0, OP2 (LVt, SIMD_ADDR_SIMPLE), QL_SIMD_LDST_ANY, F_SIZEQ | F_OD(1)),
  SIMD_INSN ("ld2", 0xc400000, 0xbfff0000, asisdlse, 0, OP2 (LVt, SIMD_ADDR_SIMPLE), QL_SIMD_LDST, F_SIZEQ | F_OD(2)),
  SIMD_INSN ("ld3", 0xc400000, 0xbfff0000, asisdlse, 0, OP2 (LVt, SIMD_ADDR_SIMPLE), QL_SIMD_LDST, F_SIZEQ | F_OD(3)),
  /* AdvSIMD load/store multiple structures (post-indexed).  */
  SIMD_INSN ("st4", 0xc800000, 0xbfe00000, asisdlsep, 0, OP2 (LVt, SIMD_ADDR_POST), QL_SIMD_LDST, F_SIZEQ | F_OD(4)),
  SIMD_INSN ("st1", 0xc800000, 0xbfe00000, asisdlsep, 0, OP2 (LVt, SIMD_ADDR_POST), QL_SIMD_LDST_ANY, F_SIZEQ | F_OD(1)),
  SIMD_INSN ("st2", 0xc800000, 0xbfe00000, asisdlsep, 0, OP2 (LVt, SIMD_ADDR_POST), QL_SIMD_LDST, F_SIZEQ | F_OD(2)),
  SIMD_INSN ("st3", 0xc800000, 0xbfe00000, asisdlsep, 0, OP2 (LVt, SIMD_ADDR_POST), QL_SIMD_LDST, F_SIZEQ | F_OD(3)),
  SIMD_INSN ("ld4", 0xcc00000, 0xbfe00000, asisdlsep, 0, OP2 (LVt, SIMD_ADDR_POST), QL_SIMD_LDST, F_SIZEQ | F_OD(4)),
  SIMD_INSN ("ld1", 0xcc00000, 0xbfe00000, asisdlsep, 0, OP2 (LVt, SIMD_ADDR_POST), QL_SIMD_LDST_ANY, F_SIZEQ | F_OD(1)),
  SIMD_INSN ("ld2", 0xcc00000, 0xbfe00000, asisdlsep, 0, OP2 (LVt, SIMD_ADDR_POST), QL_SIMD_LDST, F_SIZEQ | F_OD(2)),
  SIMD_INSN ("ld3", 0xcc00000, 0xbfe00000, asisdlsep, 0, OP2 (LVt, SIMD_ADDR_POST), QL_SIMD_LDST, F_SIZEQ | F_OD(3)),
  /* AdvSIMD load/store single structure.  */
  SIMD_INSN ("st1", 0xd000000, 0xbfff2000, asisdlso, 0, OP2 (LEt, SIMD_ADDR_SIMPLE), QL_SIMD_LDSTONE, F_OD(1)),
  SIMD_INSN ("st3", 0xd002000, 0xbfff2000, asisdlso, 0, OP2 (LEt, SIMD_ADDR_SIMPLE), QL_SIMD_LDSTONE, F_OD(3)),
  SIMD_INSN ("st2", 0xd200000, 0xbfff2000, asisdlso, 0, OP2 (LEt, SIMD_ADDR_SIMPLE), QL_SIMD_LDSTONE, F_OD(2)),
  SIMD_INSN ("st4", 0xd202000, 0xbfff2000, asisdlso, 0, OP2 (LEt, SIMD_ADDR_SIMPLE), QL_SIMD_LDSTONE, F_OD(4)),
  SIMD_INSN ("ld1", 0xd400000, 0xbfff2000, asisdlso, 0, OP2 (LEt, SIMD_ADDR_SIMPLE), QL_SIMD_LDSTONE, F_OD(1)),
  SIMD_INSN ("ld3", 0xd402000, 0xbfff2000, asisdlso, 0, OP2 (LEt, SIMD_ADDR_SIMPLE), QL_SIMD_LDSTONE, F_OD(3)),
  SIMD_INSN ("ld1r", 0xd40c000, 0xbffff000, asisdlso, 0, OP2 (LVt_AL, SIMD_ADDR_SIMPLE), QL_SIMD_LDST_ANY, F_SIZEQ | F_OD(1)),
  SIMD_INSN ("ld3r", 0xd40e000, 0xbffff000, asisdlso, 0, OP2 (LVt_AL, SIMD_ADDR_SIMPLE), QL_SIMD_LDST_ANY, F_SIZEQ | F_OD(3)),
  SIMD_INSN ("ld2", 0xd600000, 0xbfff2000, asisdlso, 0, OP2 (LEt, SIMD_ADDR_SIMPLE), QL_SIMD_LDSTONE, F_OD(2)),
  SIMD_INSN ("ld4", 0xd602000, 0xbfff2000, asisdlso, 0, OP2 (LEt, SIMD_ADDR_SIMPLE), QL_SIMD_LDSTONE, F_OD(4)),
  SIMD_INSN ("ld2r", 0xd60c000, 0xbffff000, asisdlso, 0, OP2 (LVt_AL, SIMD_ADDR_SIMPLE), QL_SIMD_LDST_ANY, F_SIZEQ | F_OD(2)),
  SIMD_INSN ("ld4r", 0xd60e000, 0xbffff000, asisdlso, 0, OP2 (LVt_AL, SIMD_ADDR_SIMPLE), QL_SIMD_LDST_ANY, F_SIZEQ | F_OD(4)),
  /* AdvSIMD load/store single structure (post-indexed).  */
  SIMD_INSN ("st1", 0xd800000, 0xbfe02000, asisdlsop, 0, OP2 (LEt, SIMD_ADDR_POST), QL_SIMD_LDSTONE, F_OD(1)),
  SIMD_INSN ("st3", 0xd802000, 0xbfe02000, asisdlsop, 0, OP2 (LEt, SIMD_ADDR_POST), QL_SIMD_LDSTONE, F_OD(3)),
  SIMD_INSN ("st2", 0xda00000, 0xbfe02000, asisdlsop, 0, OP2 (LEt, SIMD_ADDR_POST), QL_SIMD_LDSTONE, F_OD(2)),
  SIMD_INSN ("st4", 0xda02000, 0xbfe02000, asisdlsop, 0, OP2 (LEt, SIMD_ADDR_POST), QL_SIMD_LDSTONE, F_OD(4)),
  SIMD_INSN ("ld1", 0xdc00000, 0xbfe02000, asisdlsop, 0, OP2 (LEt, SIMD_ADDR_POST), QL_SIMD_LDSTONE, F_OD(1)),
  SIMD_INSN ("ld3", 0xdc02000, 0xbfe02000, asisdlsop, 0, OP2 (LEt, SIMD_ADDR_POST), QL_SIMD_LDSTONE, F_OD(3)),
  SIMD_INSN ("ld1r", 0xdc0c000, 0xbfe0f000, asisdlsop, 0, OP2 (LVt_AL, SIMD_ADDR_POST), QL_SIMD_LDST_ANY, F_SIZEQ | F_OD(1)),
  SIMD_INSN ("ld3r", 0xdc0e000, 0xbfe0f000, asisdlsop, 0, OP2 (LVt_AL, SIMD_ADDR_POST), QL_SIMD_LDST_ANY, F_SIZEQ | F_OD(3)),
  SIMD_INSN ("ld2", 0xde00000, 0xbfe02000, asisdlsop, 0, OP2 (LEt, SIMD_ADDR_POST), QL_SIMD_LDSTONE, F_OD(2)),
  SIMD_INSN ("ld4", 0xde02000, 0xbfe02000, asisdlsop, 0, OP2 (LEt, SIMD_ADDR_POST), QL_SIMD_LDSTONE, F_OD(4)),
  SIMD_INSN ("ld2r", 0xde0c000, 0xbfe0f000, asisdlsop, 0, OP2 (LVt_AL, SIMD_ADDR_POST), QL_SIMD_LDST_ANY, F_SIZEQ | F_OD(2)),
  SIMD_INSN ("ld4r", 0xde0e000, 0xbfe0f000, asisdlsop, 0, OP2 (LVt_AL, SIMD_ADDR_POST), QL_SIMD_LDST_ANY, F_SIZEQ | F_OD(4)),
  /* AdvSIMD scalar two-reg misc.  */
  SIMD_INSN ("suqadd", 0x5e203800, 0xff3ffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAME, F_SSIZE),
  SIMD_INSN ("sqabs", 0x5e207800, 0xff3ffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAME, F_SSIZE),
  SIMD_INSN ("cmgt", 0x5e208800, 0xff3ffc00, asisdmisc, 0, OP3 (Sd, Sn, IMM0), QL_SISD_CMP_0, F_SSIZE),
  SIMD_INSN ("cmeq", 0x5e209800, 0xff3ffc00, asisdmisc, 0, OP3 (Sd, Sn, IMM0), QL_SISD_CMP_0, F_SSIZE),
  SIMD_INSN ("cmlt", 0x5e20a800, 0xff3ffc00, asisdmisc, 0, OP3 (Sd, Sn, IMM0), QL_SISD_CMP_0, F_SSIZE),
  SIMD_INSN ("abs", 0x5e20b800, 0xff3ffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_2SAMED, F_SSIZE),
  SIMD_INSN ("sqxtn", 0x5e214800, 0xff3ffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_SISD_NARROW, F_SSIZE),
  SIMD_INSN ("fcvtns", 0x5e21a800, 0xffbffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAMESD, F_SSIZE),
  SF16_INSN ("fcvtns", 0x5e79a800, 0xfffffc00, asisdmisc, OP2 (Sd, Sn), QL_S_2SAMEH, F_SSIZE),
  SIMD_INSN ("fcvtms", 0x5e21b800, 0xffbffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAMESD, F_SSIZE),
  SF16_INSN ("fcvtms", 0x5e79b800, 0xfffffc00, asisdmisc, OP2 (Sd, Sn), QL_S_2SAMEH, F_SSIZE),
  SIMD_INSN ("fcvtas", 0x5e21c800, 0xffbffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAMESD, F_SSIZE),
  SF16_INSN ("fcvtas", 0x5e79c800, 0xfffffc00, asisdmisc, OP2 (Sd, Sn), QL_S_2SAMEH, F_SSIZE),
  SIMD_INSN ("scvtf", 0x5e21d800, 0xffbffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAMESD, F_SSIZE),
  SF16_INSN ("scvtf", 0x5e79d800, 0xfffffc00, asisdmisc, OP2 (Sd, Sn), QL_S_2SAMEH, F_SSIZE),
  SIMD_INSN ("fcmgt", 0x5ea0c800, 0xffbffc00, asisdmisc, 0, OP3 (Sd, Sn, FPIMM0), QL_SISD_FCMP_0, F_SSIZE),
  SF16_INSN ("fcmgt", 0x5ef8c800, 0xfffffc00, asisdmisc, OP3 (Sd, Sn, FPIMM0), QL_SISD_FCMP_H_0, F_SSIZE),
  SIMD_INSN ("fcmeq", 0x5ea0d800, 0xffbffc00, asisdmisc, 0, OP3 (Sd, Sn, FPIMM0), QL_SISD_FCMP_0, F_SSIZE),
  SF16_INSN ("fcmeq", 0x5ef8d800, 0xfffffc00, asisdmisc, OP3 (Sd, Sn, FPIMM0), QL_SISD_FCMP_H_0, F_SSIZE),
  SIMD_INSN ("fcmlt", 0x5ea0e800, 0xffbffc00, asisdmisc, 0, OP3 (Sd, Sn, FPIMM0), QL_SISD_FCMP_0, F_SSIZE),
  SF16_INSN ("fcmlt", 0x5ef8e800, 0xfffffc00, asisdmisc, OP3 (Sd, Sn, FPIMM0), QL_SISD_FCMP_H_0, F_SSIZE),
  SIMD_INSN ("fcvtps", 0x5ea1a800, 0xffbffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAMESD, F_SSIZE),
  SF16_INSN ("fcvtps", 0x5ef9a800, 0xfffffc00, asisdmisc, OP2 (Sd, Sn), QL_S_2SAMEH, F_SSIZE),
  SIMD_INSN ("fcvtzs", 0x5ea1b800, 0xffbffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAMESD, F_SSIZE),
  SF16_INSN ("fcvtzs", 0x5ef9b800, 0xfffffc00, asisdmisc, OP2 (Sd, Sn), QL_S_2SAMEH, F_SSIZE),
  SIMD_INSN ("frecpe", 0x5ea1d800, 0xffbffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAMESD, F_SSIZE),
  SF16_INSN ("frecpe", 0x5ef9d800, 0xfffffc00, asisdmisc, OP2 (Sd, Sn), QL_S_2SAMEH, F_SSIZE),
  SIMD_INSN ("frecpx", 0x5ea1f800, 0xffbffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAMESD, F_SSIZE),
  SF16_INSN ("frecpx", 0x5ef9f800, 0xfffffc00, asisdmisc, OP2 (Sd, Sn), QL_S_2SAMEH, F_SSIZE),
  SIMD_INSN ("usqadd", 0x7e203800, 0xff3ffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAME, F_SSIZE),
  SIMD_INSN ("sqneg", 0x7e207800, 0xff3ffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAME, F_SSIZE),
  SIMD_INSN ("cmge", 0x7e208800, 0xff3ffc00, asisdmisc, 0, OP3 (Sd, Sn, IMM0), QL_SISD_CMP_0, F_SSIZE),
  SIMD_INSN ("cmle", 0x7e209800, 0xff3ffc00, asisdmisc, 0, OP3 (Sd, Sn, IMM0), QL_SISD_CMP_0, F_SSIZE),
  SIMD_INSN ("neg", 0x7e20b800, 0xff3ffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_2SAMED, F_SSIZE),
  SIMD_INSN ("sqxtun", 0x7e212800, 0xff3ffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_SISD_NARROW, F_SSIZE),
  SIMD_INSN ("uqxtn", 0x7e214800, 0xff3ffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_SISD_NARROW, F_SSIZE),
  SIMD_INSN ("fcvtxn", 0x7e216800, 0xffbffc00, asisdmisc, OP_FCVTXN_S, OP2 (Sd, Sn), QL_SISD_NARROW_S, F_MISC),
  SIMD_INSN ("fcvtnu", 0x7e21a800, 0xffbffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAMESD, F_SSIZE),
  SF16_INSN ("fcvtnu", 0x7e79a800, 0xfffffc00, asisdmisc, OP2 (Sd, Sn), QL_S_2SAMEH, F_SSIZE),
  SIMD_INSN ("fcvtmu", 0x7e21b800, 0xffbffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAMESD, F_SSIZE),
  SF16_INSN ("fcvtmu", 0x7e79b800, 0xfffffc00, asisdmisc, OP2 (Sd, Sn), QL_S_2SAMEH, F_SSIZE),
  SIMD_INSN ("fcvtau", 0x7e21c800, 0xffbffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAMESD, F_SSIZE),
  SF16_INSN ("fcvtau", 0x7e79c800, 0xfffffc00, asisdmisc, OP2 (Sd, Sn), QL_S_2SAMEH, F_SSIZE),
  SIMD_INSN ("ucvtf", 0x7e21d800, 0xffbffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAMESD, F_SSIZE),
  SF16_INSN ("ucvtf", 0x7e79d800, 0xfffffc00, asisdmisc, OP2 (Sd, Sn), QL_S_2SAMEH, F_SSIZE),
  SIMD_INSN ("fcmge", 0x7ea0c800, 0xffbffc00, asisdmisc, 0, OP3 (Sd, Sn, FPIMM0), QL_SISD_FCMP_0, F_SSIZE),
  SF16_INSN ("fcmge", 0x7ef8c800, 0xfffffc00, asisdmisc, OP3 (Sd, Sn, FPIMM0), QL_SISD_FCMP_H_0, F_SSIZE),
  SIMD_INSN ("fcmle", 0x7ea0d800, 0xffbffc00, asisdmisc, 0, OP3 (Sd, Sn, FPIMM0), QL_SISD_FCMP_0, F_SSIZE),
  SF16_INSN ("fcmle", 0x7ef8d800, 0xfffffc00, asisdmisc, OP3 (Sd, Sn, FPIMM0), QL_SISD_FCMP_H_0, F_SSIZE),
  SIMD_INSN ("fcvtpu", 0x7ea1a800, 0xffbffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAMESD, F_SSIZE),
  SF16_INSN ("fcvtpu", 0x7ef9a800, 0xfffffc00, asisdmisc, OP2 (Sd, Sn), QL_SISD_FCMP_H_0, F_SSIZE),
  SIMD_INSN ("fcvtzu", 0x7ea1b800, 0xffbffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAMESD, F_SSIZE),
  SF16_INSN ("fcvtzu", 0x7ef9b800, 0xfffffc00, asisdmisc, OP2 (Sd, Sn), QL_S_2SAMEH, F_SSIZE),
  SIMD_INSN ("frsqrte", 0x7ea1d800, 0xffbffc00, asisdmisc, 0, OP2 (Sd, Sn), QL_S_2SAMESD, F_SSIZE),
  SF16_INSN ("frsqrte", 0x7ef9d800, 0xfffffc00, asisdmisc, OP2 (Sd, Sn), QL_S_2SAMEH, F_SSIZE),
  /* AdvSIMD scalar copy.  */
  SIMD_INSN ("dup", 0x5e000400, 0xffe0fc00, asisdone, 0, OP2 (Sd, En), QL_S_2SAME, F_HAS_ALIAS),
  SIMD_INSN ("mov", 0x5e000400, 0xffe0fc00, asisdone, 0, OP2 (Sd, En), QL_S_2SAME, F_ALIAS),
  /* AdvSIMD scalar pairwise.  */
  SIMD_INSN ("addp", 0x5e31b800, 0xff3ffc00, asisdpair, 0, OP2 (Sd, Vn), QL_SISD_PAIR_D, F_SIZEQ),
  SIMD_INSN ("fmaxnmp", 0x7e30c800, 0xffbffc00, asisdpair, 0, OP2 (Sd, Vn), QL_SISD_PAIR, F_SIZEQ),
  SF16_INSN ("fmaxnmp", 0x5e30c800, 0xfffffc00, asisdpair, OP2 (Sd, Vn), QL_SISD_PAIR_H, F_SIZEQ),
  SIMD_INSN ("faddp", 0x7e30d800, 0xffbffc00, asisdpair, 0, OP2 (Sd, Vn), QL_SISD_PAIR, F_SIZEQ),
  SF16_INSN ("faddp", 0x5e30d800, 0xfffffc00, asisdpair, OP2 (Sd, Vn), QL_SISD_PAIR_H, F_SIZEQ),
  SIMD_INSN ("fmaxp", 0x7e30f800, 0xffbffc00, asisdpair, 0, OP2 (Sd, Vn), QL_SISD_PAIR, F_SIZEQ),
  SF16_INSN ("fmaxp", 0x5e30f800, 0xfffffc00, asisdpair, OP2 (Sd, Vn), QL_SISD_PAIR_H, F_SIZEQ),
  SIMD_INSN ("fminnmp", 0x7eb0c800, 0xffbffc00, asisdpair, 0, OP2 (Sd, Vn), QL_SISD_PAIR, F_SIZEQ),
  SF16_INSN ("fminnmp", 0x5eb0c800, 0xfffffc00, asisdpair, OP2 (Sd, Vn), QL_SISD_PAIR_H, F_SIZEQ),
  SIMD_INSN ("fminp", 0x7eb0f800, 0xffbffc00, asisdpair, 0, OP2 (Sd, Vn), QL_SISD_PAIR, F_SIZEQ),
  SF16_INSN ("fminp", 0x5eb0f800, 0xfffffc00, asisdpair, OP2 (Sd, Vn), QL_SISD_PAIR_H, F_SIZEQ),
  /* AdvSIMD scalar three same.  */
  SIMD_INSN ("sqadd", 0x5e200c00, 0xff20fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAME, F_SSIZE),
  SIMD_INSN ("sqsub", 0x5e202c00, 0xff20fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAME, F_SSIZE),
  SIMD_INSN ("sqshl", 0x5e204c00, 0xff20fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAME, F_SSIZE),
  SIMD_INSN ("sqrshl", 0x5e205c00, 0xff20fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAME, F_SSIZE),
  SIMD_INSN ("sqdmulh", 0x5e20b400, 0xff20fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_SISD_HS, F_SSIZE),
  SIMD_INSN ("fmulx", 0x5e20dc00, 0xffa0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_FP3, F_SSIZE),
  SF16_INSN ("fmulx", 0x5e401c00, 0xffe0fc00, asisdsame, OP3 (Sd, Sn, Sm), QL_FP3_H, F_SSIZE),
  SIMD_INSN ("fcmeq", 0x5e20e400, 0xffa0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_FP3, F_SSIZE),
  SF16_INSN ("fcmeq", 0x5e402400, 0xffe0fc00, asisdsame, OP3 (Sd, Sn, Sm), QL_FP3_H, F_SSIZE),
  SIMD_INSN ("frecps", 0x5e20fc00, 0xffa0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_FP3, F_SSIZE),
  SF16_INSN ("frecps", 0x5e403c00, 0xffe0fc00, asisdsame, OP3 (Sd, Sn, Sm), QL_FP3_H, F_SSIZE),
  SIMD_INSN ("frsqrts", 0x5ea0fc00, 0xffa0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_FP3, F_SSIZE),
  SF16_INSN ("frsqrts", 0x5ec03c00, 0xffe0fc00, asisdsame, OP3 (Sd, Sn, Sm), QL_FP3_H, F_SSIZE),
  SIMD_INSN ("cmgt", 0x5ee03400, 0xffe0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAMED, F_SSIZE),
  SIMD_INSN ("cmge", 0x5ee03c00, 0xffe0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAMED, F_SSIZE),
  SIMD_INSN ("sshl", 0x5ee04400, 0xffe0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAMED, F_SSIZE),
  SIMD_INSN ("srshl", 0x5ee05400, 0xffe0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAMED, F_SSIZE),
  SIMD_INSN ("add", 0x5ee08400, 0xffe0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAMED, F_SSIZE),
  SIMD_INSN ("cmtst", 0x5ee08c00, 0xffe0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAMED, F_SSIZE),
  SIMD_INSN ("uqadd", 0x7e200c00, 0xff20fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAME, F_SSIZE),
  SIMD_INSN ("uqsub", 0x7e202c00, 0xff20fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAME, F_SSIZE),
  SIMD_INSN ("uqshl", 0x7e204c00, 0xff20fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAME, F_SSIZE),
  SIMD_INSN ("uqrshl", 0x7e205c00, 0xff20fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAME, F_SSIZE),
  SIMD_INSN ("sqrdmulh", 0x7e20b400, 0xff20fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_SISD_HS, F_SSIZE),
  SIMD_INSN ("fcmge", 0x7e20e400, 0xffa0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_FP3, F_SSIZE),
  SF16_INSN ("fcmge", 0x7e402400, 0xffe0fc00, asisdsame, OP3 (Sd, Sn, Sm), QL_FP3_H, F_SSIZE),
  SIMD_INSN ("facge", 0x7e20ec00, 0xffa0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_FP3, F_SSIZE),
  SF16_INSN ("facge", 0x7e402c00, 0xffe0fc00, asisdsame, OP3 (Sd, Sn, Sm), QL_FP3_H, F_SSIZE),
  SIMD_INSN ("fabd", 0x7ea0d400, 0xffa0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_FP3, F_SSIZE),
  SF16_INSN ("fabd", 0x7ec01400, 0xffe0fc00, asisdsame, OP3 (Sd, Sn, Sm), QL_FP3_H, F_SSIZE),
  SIMD_INSN ("fcmgt", 0x7ea0e400, 0xffa0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_FP3, F_SSIZE),
  SF16_INSN ("fcmgt", 0x7ec02400, 0xffe0fc00, asisdsame, OP3 (Sd, Sn, Sm), QL_FP3_H, F_SSIZE),
  SIMD_INSN ("facgt", 0x7ea0ec00, 0xffa0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_FP3, F_SSIZE),
  SF16_INSN ("facgt", 0x7ec02c00, 0xffe0fc00, asisdsame, OP3 (Sd, Sn, Sm), QL_FP3_H, F_SSIZE),
  SIMD_INSN ("cmhi", 0x7ee03400, 0xffe0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAMED, F_SSIZE),
  SIMD_INSN ("cmhs", 0x7ee03c00, 0xffe0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAMED, F_SSIZE),
  SIMD_INSN ("ushl", 0x7ee04400, 0xffe0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAMED, F_SSIZE),
  SIMD_INSN ("urshl", 0x7ee05400, 0xffe0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAMED, F_SSIZE),
  SIMD_INSN ("sub", 0x7ee08400, 0xffe0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAMED, F_SSIZE),
  SIMD_INSN ("cmeq", 0x7ee08c00, 0xffe0fc00, asisdsame, 0, OP3 (Sd, Sn, Sm), QL_S_3SAMED, F_SSIZE),
  /* AdvSIMDs scalar three same extension.  */
  RDMA_INSN ("sqrdmlah", 0x7e008400, 0xff20fc00, asimdsame, OP3 (Sd, Sn, Sm), QL_SISD_HS, F_SSIZE),
  RDMA_INSN ("sqrdmlsh", 0x7e008c00, 0xff20fc00, asimdsame, OP3 (Sd, Sn, Sm), QL_SISD_HS, F_SSIZE),
  /* AdvSIMD scalar shift by immediate.  */
  SIMD_INSN ("sshr", 0x5f000400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_D, 0),
  SIMD_INSN ("ssra", 0x5f001400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_D, 0),
  SIMD_INSN ("srshr", 0x5f002400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_D, 0),
  SIMD_INSN ("srsra", 0x5f003400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_D, 0),
  SIMD_INSN ("shl", 0x5f005400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSL), QL_SSHIFT_D, 0),
  SIMD_INSN ("sqshl", 0x5f007400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSL), QL_SSHIFT, 0),
  SIMD_INSN ("sqshrn", 0x5f009400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFTN, 0),
  SIMD_INSN ("sqrshrn", 0x5f009c00, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFTN, 0),
  SIMD_INSN ("scvtf", 0x5f00e400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_SD, 0),
  SF16_INSN ("scvtf", 0x5f10e400, 0xff80fc00, asisdshf, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_H, 0),
  SIMD_INSN ("fcvtzs", 0x5f00fc00, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_SD, 0),
  SF16_INSN ("fcvtzs", 0x5f10fc00, 0xff80fc00, asisdshf, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_H, 0),
  SIMD_INSN ("ushr", 0x7f000400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_D, 0),
  SIMD_INSN ("usra", 0x7f001400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_D, 0),
  SIMD_INSN ("urshr", 0x7f002400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_D, 0),
  SIMD_INSN ("ursra", 0x7f003400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_D, 0),
  SIMD_INSN ("sri", 0x7f004400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_D, 0),
  SIMD_INSN ("sli", 0x7f005400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSL), QL_SSHIFT_D, 0),
  SIMD_INSN ("sqshlu", 0x7f006400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSL), QL_SSHIFT, 0),
  SIMD_INSN ("uqshl", 0x7f007400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSL), QL_SSHIFT, 0),
  SIMD_INSN ("sqshrun", 0x7f008400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFTN, 0),
  SIMD_INSN ("sqrshrun", 0x7f008c00, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFTN, 0),
  SIMD_INSN ("uqshrn", 0x7f009400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFTN, 0),
  SIMD_INSN ("uqrshrn", 0x7f009c00, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFTN, 0),
  SIMD_INSN ("ucvtf", 0x7f00e400, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_SD, 0),
  SF16_INSN ("ucvtf", 0x7f10e400, 0xff80fc00, asisdshf, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_H, 0),
  SIMD_INSN ("fcvtzu", 0x7f00fc00, 0xff80fc00, asisdshf, 0, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_SD, 0),
  SF16_INSN ("fcvtzu", 0x7f10fc00, 0xff80fc00, asisdshf, OP3 (Sd, Sn, IMM_VLSR), QL_SSHIFT_H, 0),
  /* Bitfield.  */
  CORE_INSN ("sbfm", 0x13000000, 0x7f800000, bitfield, 0, OP4 (Rd, Rn, IMMR, IMMS), QL_BF, F_HAS_ALIAS | F_SF | F_N),
  CORE_INSN ("sbfiz", 0x13000000, 0x7f800000, bitfield, OP_SBFIZ, OP4 (Rd, Rn, IMM, WIDTH), QL_BF2, F_ALIAS | F_P1 | F_CONV),
  CORE_INSN ("sbfx",  0x13000000, 0x7f800000, bitfield, OP_SBFX,  OP4 (Rd, Rn, IMM, WIDTH), QL_BF2, F_ALIAS | F_P1 | F_CONV),
  CORE_INSN ("sxtb", 0x13001c00, 0x7fbffc00, bitfield, 0, OP2 (Rd, Rn), QL_EXT, F_ALIAS | F_P3 | F_SF | F_N),
  CORE_INSN ("sxth", 0x13003c00, 0x7fbffc00, bitfield, 0, OP2 (Rd, Rn), QL_EXT, F_ALIAS | F_P3 | F_SF | F_N),
  CORE_INSN ("sxtw", 0x93407c00, 0xfffffc00, bitfield, 0, OP2 (Rd, Rn), QL_EXT_W, F_ALIAS | F_P3),
  CORE_INSN ("asr", 0x13000000, 0x7f800000, bitfield, OP_ASR_IMM, OP3 (Rd, Rn, IMM), QL_SHIFT, F_ALIAS | F_P2 | F_CONV),
  CORE_INSN ("bfm", 0x33000000, 0x7f800000, bitfield, 0, OP4 (Rd, Rn, IMMR, IMMS), QL_BF, F_HAS_ALIAS | F_SF | F_N),
  CORE_INSN ("bfi", 0x33000000, 0x7f800000, bitfield, OP_BFI, OP4 (Rd, Rn, IMM, WIDTH), QL_BF2, F_ALIAS | F_P1 | F_CONV),
  V8_2_INSN ("bfc", 0x330003e0, 0x7f8003e0, bitfield, OP_BFC, OP3 (Rd, IMM, WIDTH), QL_BF1, F_ALIAS | F_P2 | F_CONV),
  CORE_INSN ("bfxil", 0x33000000, 0x7f800000, bitfield, OP_BFXIL, OP4 (Rd, Rn, IMM, WIDTH), QL_BF2, F_ALIAS | F_P1 | F_CONV),
  CORE_INSN ("ubfm", 0x53000000, 0x7f800000, bitfield, 0, OP4 (Rd, Rn, IMMR, IMMS), QL_BF, F_HAS_ALIAS | F_SF | F_N),
  CORE_INSN ("ubfiz", 0x53000000, 0x7f800000, bitfield, OP_UBFIZ, OP4 (Rd, Rn, IMM, WIDTH), QL_BF2, F_ALIAS | F_P1 | F_CONV),
  CORE_INSN ("ubfx", 0x53000000, 0x7f800000, bitfield, OP_UBFX, OP4 (Rd, Rn, IMM, WIDTH), QL_BF2, F_ALIAS | F_P1 | F_CONV),
  CORE_INSN ("uxtb", 0x53001c00, 0xfffffc00, bitfield, OP_UXTB, OP2 (Rd, Rn), QL_I2SAMEW, F_ALIAS | F_P3),
  CORE_INSN ("uxth", 0x53003c00, 0xfffffc00, bitfield, OP_UXTH, OP2 (Rd, Rn), QL_I2SAMEW, F_ALIAS | F_P3),
  CORE_INSN ("lsl", 0x53000000, 0x7f800000, bitfield, OP_LSL_IMM, OP3 (Rd, Rn, IMM), QL_SHIFT, F_ALIAS | F_P2 | F_CONV),
  CORE_INSN ("lsr", 0x53000000, 0x7f800000, bitfield, OP_LSR_IMM, OP3 (Rd, Rn, IMM), QL_SHIFT, F_ALIAS | F_P2 | F_CONV),
  /* Unconditional branch (immediate).  */
  CORE_INSN ("b", 0x14000000, 0xfc000000, branch_imm, OP_B, OP1 (ADDR_PCREL26), QL_PCREL_26, 0),
  CORE_INSN ("bl", 0x94000000, 0xfc000000, branch_imm, OP_BL, OP1 (ADDR_PCREL26), QL_PCREL_26, 0),
  /* Unconditional branch (register).  */
  CORE_INSN ("br", 0xd61f0000, 0xfffffc1f, branch_reg, 0, OP1 (Rn), QL_I1X, 0),
  CORE_INSN ("blr", 0xd63f0000, 0xfffffc1f, branch_reg, 0, OP1 (Rn), QL_I1X, 0),
  CORE_INSN ("ret", 0xd65f0000, 0xfffffc1f, branch_reg, 0, OP1 (Rn), QL_I1X, F_OPD0_OPT | F_DEFAULT (30)),
  CORE_INSN ("eret", 0xd69f03e0, 0xffffffff, branch_reg, 0, OP0 (), {}, 0),
  CORE_INSN ("drps", 0xd6bf03e0, 0xffffffff, branch_reg, 0, OP0 (), {}, 0),
  PAC_INSN ("braa", 0xd71f0800, 0xfffffc00, branch_reg, OP2 (Rn, Rd_SP), QL_I2SAMEX, 0),
  PAC_INSN ("brab", 0xd71f0c00, 0xfffffc00, branch_reg, OP2 (Rn, Rd_SP), QL_I2SAMEX, 0),
  PAC_INSN ("blraa", 0xd73f0800, 0xfffffc00, branch_reg, OP2 (Rn, Rd_SP), QL_I2SAMEX, 0),
  PAC_INSN ("blrab", 0xd73f0c00, 0xfffffc00, branch_reg, OP2 (Rn, Rd_SP), QL_I2SAMEX, 0),
  PAC_INSN ("braaz", 0xd61f081f, 0xfffffc1f, branch_reg, OP1 (Rn), QL_I1X, 0),
  PAC_INSN ("brabz", 0xd61f0c1f, 0xfffffc1f, branch_reg, OP1 (Rn), QL_I1X, 0),
  PAC_INSN ("blraaz", 0xd63f081f, 0xfffffc1f, branch_reg, OP1 (Rn), QL_I1X, 0),
  PAC_INSN ("blrabz", 0xd63f0c1f, 0xfffffc1f, branch_reg, OP1 (Rn), QL_I1X, 0),
  PAC_INSN ("retaa", 0xd65f0bff, 0xffffffff, branch_reg, OP0 (), {}, 0),
  PAC_INSN ("retab", 0xd65f0fff, 0xffffffff, branch_reg, OP0 (), {}, 0),
  PAC_INSN ("eretaa", 0xd69f0bff, 0xffffffff, branch_reg, OP0 (), {}, 0),
  PAC_INSN ("eretab", 0xd69f0fff, 0xffffffff, branch_reg, OP0 (), {}, 0),
  /* Compare & branch (immediate).  */
  CORE_INSN ("cbz", 0x34000000, 0x7f000000, compbranch, 0, OP2 (Rt, ADDR_PCREL19), QL_R_PCREL, F_SF),
  CORE_INSN ("cbnz", 0x35000000, 0x7f000000, compbranch, 0, OP2 (Rt, ADDR_PCREL19), QL_R_PCREL, F_SF),
  /* Conditional branch (immediate).  */
  CORE_INSN ("b.c", 0x54000000, 0xff000010, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_COND),
  /* Conditional compare (immediate).  */
  CORE_INSN ("ccmn", 0x3a400800, 0x7fe00c10, condcmp_imm, 0, OP4 (Rn, CCMP_IMM, NZCV, COND), QL_CCMP_IMM, F_SF),
  CORE_INSN ("ccmp", 0x7a400800, 0x7fe00c10, condcmp_imm, 0, OP4 (Rn, CCMP_IMM, NZCV, COND), QL_CCMP_IMM, F_SF),
  /* Conditional compare (register).  */
  CORE_INSN ("ccmn", 0x3a400000, 0x7fe00c10, condcmp_reg, 0, OP4 (Rn, Rm, NZCV, COND), QL_CCMP, F_SF),
  CORE_INSN ("ccmp", 0x7a400000, 0x7fe00c10, condcmp_reg, 0, OP4 (Rn, Rm, NZCV, COND), QL_CCMP, F_SF),
  /* Conditional select.  */
  CORE_INSN ("csel", 0x1a800000, 0x7fe00c00, condsel, 0, OP4 (Rd, Rn, Rm, COND), QL_CSEL, F_SF),
  CORE_INSN ("csinc", 0x1a800400, 0x7fe00c00, condsel, 0, OP4 (Rd, Rn, Rm, COND), QL_CSEL, F_HAS_ALIAS | F_SF),
  CORE_INSN ("cinc", 0x1a800400, 0x7fe00c00, condsel, OP_CINC, OP3 (Rd, Rn, COND1), QL_CSEL, F_ALIAS | F_SF | F_CONV),
  CORE_INSN ("cset", 0x1a9f07e0, 0x7fff0fe0, condsel, OP_CSET, OP2 (Rd, COND1), QL_DST_R, F_ALIAS | F_P1 | F_SF | F_CONV),
  CORE_INSN ("csinv", 0x5a800000, 0x7fe00c00, condsel, 0, OP4 (Rd, Rn, Rm, COND), QL_CSEL, F_HAS_ALIAS | F_SF),
  CORE_INSN ("cinv", 0x5a800000, 0x7fe00c00, condsel, OP_CINV, OP3 (Rd, Rn, COND1), QL_CSEL, F_ALIAS | F_SF | F_CONV),
  CORE_INSN ("csetm", 0x5a9f03e0, 0x7fff0fe0, condsel, OP_CSETM, OP2 (Rd, COND1), QL_DST_R, F_ALIAS | F_P1 | F_SF | F_CONV),
  CORE_INSN ("csneg", 0x5a800400, 0x7fe00c00, condsel, 0, OP4 (Rd, Rn, Rm, COND), QL_CSEL, F_HAS_ALIAS | F_SF),
  CORE_INSN ("cneg", 0x5a800400, 0x7fe00c00, condsel, OP_CNEG, OP3 (Rd, Rn, COND1), QL_CSEL, F_ALIAS | F_SF | F_CONV),
  /* Crypto AES.  */
  AES_INSN ("aese",     0x4e284800, 0xfffffc00, cryptoaes, OP2 (Vd, Vn), QL_V2SAME16B, 0),
  AES_INSN ("aesd",     0x4e285800, 0xfffffc00, cryptoaes, OP2 (Vd, Vn), QL_V2SAME16B, 0),
  AES_INSN ("aesmc",    0x4e286800, 0xfffffc00, cryptoaes, OP2 (Vd, Vn), QL_V2SAME16B, 0),
  AES_INSN ("aesimc",   0x4e287800, 0xfffffc00, cryptoaes, OP2 (Vd, Vn), QL_V2SAME16B, 0),
  /* Crypto two-reg SHA.  */
  SHA2_INSN ("sha1h",    0x5e280800, 0xfffffc00, cryptosha2, OP2 (Fd, Fn), QL_2SAMES, 0),
  SHA2_INSN ("sha1su1",  0x5e281800, 0xfffffc00, cryptosha2, OP2 (Vd, Vn), QL_V2SAME4S, 0),
  SHA2_INSN ("sha256su0",0x5e282800, 0xfffffc00, cryptosha2, OP2 (Vd, Vn), QL_V2SAME4S, 0),
  /* Crypto three-reg SHA.  */
  SHA2_INSN ("sha1c",    0x5e000000, 0xffe0fc00, cryptosha3, OP3 (Fd, Fn, Vm), QL_SHAUPT, 0),
  SHA2_INSN ("sha1p",    0x5e001000, 0xffe0fc00, cryptosha3, OP3 (Fd, Fn, Vm), QL_SHAUPT, 0),
  SHA2_INSN ("sha1m",    0x5e002000, 0xffe0fc00, cryptosha3, OP3 (Fd, Fn, Vm), QL_SHAUPT, 0),
  SHA2_INSN ("sha1su0",  0x5e003000, 0xffe0fc00, cryptosha3, OP3 (Vd, Vn, Vm), QL_V3SAME4S, 0),
  SHA2_INSN ("sha256h",  0x5e004000, 0xffe0fc00, cryptosha3, OP3 (Fd, Fn, Vm), QL_SHA256UPT, 0),
  SHA2_INSN ("sha256h2", 0x5e005000, 0xffe0fc00, cryptosha3, OP3 (Fd, Fn, Vm), QL_SHA256UPT, 0),
  SHA2_INSN ("sha256su1",0x5e006000, 0xffe0fc00, cryptosha3, OP3 (Vd, Vn, Vm), QL_V3SAME4S, 0),
  /* Data-processing (1 source).  */
  CORE_INSN ("rbit",  0x5ac00000, 0x7ffffc00, dp_1src, 0, OP2 (Rd, Rn), QL_I2SAME, F_SF),
  CORE_INSN ("rev16", 0x5ac00400, 0x7ffffc00, dp_1src, 0, OP2 (Rd, Rn), QL_I2SAME, F_SF),
  CORE_INSN ("rev",   0x5ac00800, 0xfffffc00, dp_1src, 0, OP2 (Rd, Rn), QL_I2SAMEW, 0),
  CORE_INSN ("rev",   0xdac00c00, 0xfffffc00, dp_1src, 0, OP2 (Rd, Rn), QL_I2SAMEX, F_SF | F_HAS_ALIAS | F_P1),
  V8_2_INSN ("rev64", 0xdac00c00, 0xfffffc00, dp_1src, 0, OP2 (Rd, Rn), QL_I2SAMEX, F_SF | F_ALIAS),
  CORE_INSN ("clz",   0x5ac01000, 0x7ffffc00, dp_1src, 0, OP2 (Rd, Rn), QL_I2SAME, F_SF),
  CORE_INSN ("cls",   0x5ac01400, 0x7ffffc00, dp_1src, 0, OP2 (Rd, Rn), QL_I2SAME, F_SF),
  CORE_INSN ("rev32", 0xdac00800, 0xfffffc00, dp_1src, 0, OP2 (Rd, Rn), QL_I2SAMEX, 0),
  PAC_INSN ("pacia", 0xdac10000, 0xfffffc00, dp_1src, OP2 (Rd, Rn_SP), QL_I2SAMEX, 0),
  PAC_INSN ("pacib", 0xdac10400, 0xfffffc00, dp_1src, OP2 (Rd, Rn_SP), QL_I2SAMEX, 0),
  PAC_INSN ("pacda", 0xdac10800, 0xfffffc00, dp_1src, OP2 (Rd, Rn_SP), QL_I2SAMEX, 0),
  PAC_INSN ("pacdb", 0xdac10c00, 0xfffffc00, dp_1src, OP2 (Rd, Rn_SP), QL_I2SAMEX, 0),
  PAC_INSN ("autia", 0xdac11000, 0xfffffc00, dp_1src, OP2 (Rd, Rn_SP), QL_I2SAMEX, 0),
  PAC_INSN ("autib", 0xdac11400, 0xfffffc00, dp_1src, OP2 (Rd, Rn_SP), QL_I2SAMEX, 0),
  PAC_INSN ("autda", 0xdac11800, 0xfffffc00, dp_1src, OP2 (Rd, Rn_SP), QL_I2SAMEX, 0),
  PAC_INSN ("autdb", 0xdac11c00, 0xfffffc00, dp_1src, OP2 (Rd, Rn_SP), QL_I2SAMEX, 0),
  PAC_INSN ("paciza", 0xdac123e0, 0xffffffe0, dp_1src, OP1 (Rd), QL_I1X, 0),
  PAC_INSN ("pacizb", 0xdac127e0, 0xffffffe0, dp_1src, OP1 (Rd), QL_I1X, 0),
  PAC_INSN ("pacdza", 0xdac12be0, 0xffffffe0, dp_1src, OP1 (Rd), QL_I1X, 0),
  PAC_INSN ("pacdzb", 0xdac12fe0, 0xffffffe0, dp_1src, OP1 (Rd), QL_I1X, 0),
  PAC_INSN ("autiza", 0xdac133e0, 0xffffffe0, dp_1src, OP1 (Rd), QL_I1X, 0),
  PAC_INSN ("autizb", 0xdac137e0, 0xffffffe0, dp_1src, OP1 (Rd), QL_I1X, 0),
  PAC_INSN ("autdza", 0xdac13be0, 0xffffffe0, dp_1src, OP1 (Rd), QL_I1X, 0),
  PAC_INSN ("autdzb", 0xdac13fe0, 0xffffffe0, dp_1src, OP1 (Rd), QL_I1X, 0),
  PAC_INSN ("xpaci", 0xdac143e0, 0xffffffe0, dp_1src, OP1 (Rd), QL_I1X, 0),
  PAC_INSN ("xpacd", 0xdac147e0, 0xffffffe0, dp_1src, OP1 (Rd), QL_I1X, 0),
  /* Data-processing (2 source).  */
  CORE_INSN ("udiv",  0x1ac00800, 0x7fe0fc00, dp_2src, 0, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF),
  CORE_INSN ("sdiv",  0x1ac00c00, 0x7fe0fc00, dp_2src, 0, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF),
  CORE_INSN ("lslv",  0x1ac02000, 0x7fe0fc00, dp_2src, 0, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF | F_HAS_ALIAS),
  CORE_INSN ("lsl",   0x1ac02000, 0x7fe0fc00, dp_2src, 0, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF | F_ALIAS),
  CORE_INSN ("lsrv",  0x1ac02400, 0x7fe0fc00, dp_2src, 0, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF | F_HAS_ALIAS),
  CORE_INSN ("lsr",   0x1ac02400, 0x7fe0fc00, dp_2src, 0, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF | F_ALIAS),
  CORE_INSN ("asrv",  0x1ac02800, 0x7fe0fc00, dp_2src, 0, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF | F_HAS_ALIAS),
  CORE_INSN ("asr",   0x1ac02800, 0x7fe0fc00, dp_2src, 0, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF | F_ALIAS),
  CORE_INSN ("rorv",  0x1ac02c00, 0x7fe0fc00, dp_2src, 0, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF | F_HAS_ALIAS),
  CORE_INSN ("ror",   0x1ac02c00, 0x7fe0fc00, dp_2src, 0, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF | F_ALIAS),
  MEMTAG_INSN ("subp",   0x9ac00000, 0xffe0fc00, dp_2src, OP3 (Rd, Rn_SP, Rm_SP), QL_I3SAMEX, 0),
  MEMTAG_INSN ("subps",  0xbac00000, 0xffe0fc00, dp_2src, OP3 (Rd, Rn_SP, Rm_SP), QL_I3SAMEX, F_HAS_ALIAS),
  MEMTAG_INSN ("cmpp",   0xbac0001f, 0xffe0fc1f, dp_2src, OP2 (Rn_SP, Rm_SP), QL_I2SAMEX, F_ALIAS),
  MEMTAG_INSN ("irg",    0x9ac01000, 0xffe0fc00, dp_2src, OP3 (Rd_SP, Rn_SP, Rm), QL_I3SAMEX, F_OPD2_OPT | F_DEFAULT (0x1f)),
  MEMTAG_INSN ("gmi",    0x9ac01400, 0xffe0fc00, dp_2src, OP3 (Rd, Rn_SP, Rm), QL_I3SAMEX, 0),
  PAC_INSN ("pacga", 0x9ac03000, 0xffe0fc00, dp_2src, OP3 (Rd, Rn, Rm_SP), QL_I3SAMEX, 0),
  /* CRC instructions.  */
  _CRC_INSN ("crc32b", 0x1ac04000, 0xffe0fc00, dp_2src, OP3 (Rd, Rn, Rm), QL_I3SAMEW, 0),
  _CRC_INSN ("crc32h", 0x1ac04400, 0xffe0fc00, dp_2src, OP3 (Rd, Rn, Rm), QL_I3SAMEW, 0),
  _CRC_INSN ("crc32w", 0x1ac04800, 0xffe0fc00, dp_2src, OP3 (Rd, Rn, Rm), QL_I3SAMEW, 0),
  _CRC_INSN ("crc32x", 0x9ac04c00, 0xffe0fc00, dp_2src, OP3 (Rd, Rn, Rm), QL_I3WWX,   0),
  _CRC_INSN ("crc32cb",0x1ac05000, 0xffe0fc00, dp_2src, OP3 (Rd, Rn, Rm), QL_I3SAMEW, 0),
  _CRC_INSN ("crc32ch",0x1ac05400, 0xffe0fc00, dp_2src, OP3 (Rd, Rn, Rm), QL_I3SAMEW, 0),
  _CRC_INSN ("crc32cw",0x1ac05800, 0xffe0fc00, dp_2src, OP3 (Rd, Rn, Rm), QL_I3SAMEW, 0),
  _CRC_INSN ("crc32cx",0x9ac05c00, 0xffe0fc00, dp_2src, OP3 (Rd, Rn, Rm), QL_I3WWX,   0),
  /* Data-processing (3 source).  */
  CORE_INSN ("madd",  0x1b000000, 0x7fe08000, dp_3src, 0, OP4 (Rd, Rn, Rm, Ra), QL_I4SAMER, F_HAS_ALIAS | F_SF),
  CORE_INSN ("mul",   0x1b007c00, 0x7fe0fc00, dp_3src, 0, OP3 (Rd, Rn, Rm),     QL_I3SAMER, F_ALIAS | F_SF),
  CORE_INSN ("msub",  0x1b008000, 0x7fe08000, dp_3src, 0, OP4 (Rd, Rn, Rm, Ra), QL_I4SAMER, F_HAS_ALIAS | F_SF),
  CORE_INSN ("mneg",  0x1b00fc00, 0x7fe0fc00, dp_3src, 0, OP3 (Rd, Rn, Rm),     QL_I3SAMER, F_ALIAS | F_SF),
  CORE_INSN ("smaddl",0x9b200000, 0xffe08000, dp_3src, 0, OP4 (Rd, Rn, Rm, Ra), QL_I4SAMEL, F_HAS_ALIAS),
  CORE_INSN ("smull", 0x9b207c00, 0xffe0fc00, dp_3src, 0, OP3 (Rd, Rn, Rm),     QL_I3SAMEL, F_ALIAS),
  CORE_INSN ("smsubl",0x9b208000, 0xffe08000, dp_3src, 0, OP4 (Rd, Rn, Rm, Ra), QL_I4SAMEL, F_HAS_ALIAS),
  CORE_INSN ("smnegl",0x9b20fc00, 0xffe0fc00, dp_3src, 0, OP3 (Rd, Rn, Rm),     QL_I3SAMEL, F_ALIAS),
  CORE_INSN ("smulh", 0x9b407c00, 0xffe08000, dp_3src, 0, OP3 (Rd, Rn, Rm),     QL_I3SAMEX, 0),
  CORE_INSN ("umaddl",0x9ba00000, 0xffe08000, dp_3src, 0, OP4 (Rd, Rn, Rm, Ra), QL_I4SAMEL, F_HAS_ALIAS),
  CORE_INSN ("umull", 0x9ba07c00, 0xffe0fc00, dp_3src, 0, OP3 (Rd, Rn, Rm),     QL_I3SAMEL, F_ALIAS),
  CORE_INSN ("umsubl",0x9ba08000, 0xffe08000, dp_3src, 0, OP4 (Rd, Rn, Rm, Ra), QL_I4SAMEL, F_HAS_ALIAS),
  CORE_INSN ("umnegl",0x9ba0fc00, 0xffe0fc00, dp_3src, 0, OP3 (Rd, Rn, Rm),     QL_I3SAMEL, F_ALIAS),
  CORE_INSN ("umulh", 0x9bc07c00, 0xffe08000, dp_3src, 0, OP3 (Rd, Rn, Rm),     QL_I3SAMEX, 0),
  /* Excep'n generation.  */
  CORE_INSN ("svc",   0xd4000001, 0xffe0001f, exception, 0, OP1 (EXCEPTION), {}, 0),
  CORE_INSN ("hvc",   0xd4000002, 0xffe0001f, exception, 0, OP1 (EXCEPTION), {}, 0),
  CORE_INSN ("smc",   0xd4000003, 0xffe0001f, exception, 0, OP1 (EXCEPTION), {}, 0),
  CORE_INSN ("brk",   0xd4200000, 0xffe0001f, exception, 0, OP1 (EXCEPTION), {}, 0),
  CORE_INSN ("hlt",   0xd4400000, 0xffe0001f, exception, 0, OP1 (EXCEPTION), {}, 0),
  CORE_INSN ("udf",   0x00000000, 0xffff0000, exception, 0, OP1 (UNDEFINED), {}, 0),
  CORE_INSN ("dcps1", 0xd4a00001, 0xffe0001f, exception, 0, OP1 (EXCEPTION), {}, F_OPD0_OPT | F_DEFAULT (0)),
  CORE_INSN ("dcps2", 0xd4a00002, 0xffe0001f, exception, 0, OP1 (EXCEPTION), {}, F_OPD0_OPT | F_DEFAULT (0)),
  CORE_INSN ("dcps3", 0xd4a00003, 0xffe0001f, exception, 0, OP1 (EXCEPTION), {}, F_OPD0_OPT | F_DEFAULT (0)),
  /* Extract.  */
  CORE_INSN ("extr",  0x13800000, 0x7fa00000, extract, 0, OP4 (Rd, Rn, Rm, IMMS), QL_EXTR, F_HAS_ALIAS | F_SF | F_N),
  CORE_INSN ("ror", 0x13800000, 0x7fa00000, extract, OP_ROR_IMM, OP3 (Rd, Rm, IMMS), QL_SHIFT, F_ALIAS | F_CONV),
  /* Floating-point<->fixed-point conversions.  */
  __FP_INSN ("scvtf", 0x1e020000, 0x7f3f0000, float2fix, 0, OP3 (Fd, Rn, FBITS), QL_FIX2FP, F_FPTYPE | F_SF),
  FF16_INSN ("scvtf", 0x1ec20000, 0x7f3f0000, float2fix, OP3 (Fd, Rn, FBITS), QL_FIX2FP_H, F_FPTYPE | F_SF),
  __FP_INSN ("ucvtf", 0x1e030000, 0x7f3f0000, float2fix, 0, OP3 (Fd, Rn, FBITS), QL_FIX2FP, F_FPTYPE | F_SF),
  FF16_INSN ("ucvtf", 0x1ec30000, 0x7f3f0000, float2fix, OP3 (Fd, Rn, FBITS), QL_FIX2FP_H, F_FPTYPE | F_SF),
  __FP_INSN ("fcvtzs",0x1e180000, 0x7f3f0000, float2fix, 0, OP3 (Rd, Fn, FBITS), QL_FP2FIX, F_FPTYPE | F_SF),
  FF16_INSN ("fcvtzs",0x1ed80000, 0x7f3f0000, float2fix, OP3 (Rd, Fn, FBITS), QL_FP2FIX_H, F_FPTYPE | F_SF),
  __FP_INSN ("fcvtzu",0x1e190000, 0x7f3f0000, float2fix, 0, OP3 (Rd, Fn, FBITS), QL_FP2FIX, F_FPTYPE | F_SF),
  FF16_INSN ("fcvtzu",0x1ed90000, 0x7f3f0000, float2fix, OP3 (Rd, Fn, FBITS), QL_FP2FIX_H, F_FPTYPE | F_SF),
  /* Floating-point<->integer conversions.  */
  __FP_INSN ("fcvtns",0x1e200000, 0x7f3ffc00, float2int, 0, OP2 (Rd, Fn), QL_FP2INT, F_FPTYPE | F_SF),
  FF16_INSN ("fcvtns",0x1ee00000, 0x7f3ffc00, float2int, OP2 (Rd, Fn), QL_FP2INT_H, F_FPTYPE | F_SF),
  __FP_INSN ("fcvtnu",0x1e210000, 0x7f3ffc00, float2int, 0, OP2 (Rd, Fn), QL_FP2INT, F_FPTYPE | F_SF),
  FF16_INSN ("fcvtnu",0x1ee10000, 0x7f3ffc00, float2int, OP2 (Rd, Fn), QL_FP2INT_H, F_FPTYPE | F_SF),
  __FP_INSN ("scvtf", 0x1e220000, 0x7f3ffc00, float2int, 0, OP2 (Fd, Rn), QL_INT2FP, F_FPTYPE | F_SF),
  FF16_INSN ("scvtf", 0x1ee20000, 0x7f3ffc00, float2int, OP2 (Fd, Rn), QL_INT2FP_H, F_FPTYPE | F_SF),
  __FP_INSN ("ucvtf", 0x1e230000, 0x7f3ffc00, float2int, 0, OP2 (Fd, Rn), QL_INT2FP, F_FPTYPE | F_SF),
  FF16_INSN ("ucvtf", 0x1ee30000, 0x7f3ffc00, float2int, OP2 (Fd, Rn), QL_INT2FP_H, F_FPTYPE | F_SF),
  __FP_INSN ("fcvtas",0x1e240000, 0x7f3ffc00, float2int, 0, OP2 (Rd, Fn), QL_FP2INT, F_FPTYPE | F_SF),
  FF16_INSN ("fcvtas",0x1ee40000, 0x7f3ffc00, float2int, OP2 (Rd, Fn), QL_FP2INT_H, F_FPTYPE | F_SF),
  __FP_INSN ("fcvtau",0x1e250000, 0x7f3ffc00, float2int, 0, OP2 (Rd, Fn), QL_FP2INT, F_FPTYPE | F_SF),
  FF16_INSN ("fcvtau",0x1ee50000, 0x7f3ffc00, float2int, OP2 (Rd, Fn), QL_FP2INT_H, F_FPTYPE | F_SF),
  __FP_INSN ("fmov",  0x1e260000, 0x7f3ffc00, float2int, 0, OP2 (Rd, Fn), QL_FP2INT_FMOV, F_FPTYPE | F_SF),
  FF16_INSN ("fmov",  0x1ee60000, 0x7f3ffc00, float2int, OP2 (Rd, Fn), QL_FP2INT_H, F_FPTYPE | F_SF),
  __FP_INSN ("fmov",  0x1e270000, 0x7f3ffc00, float2int, 0, OP2 (Fd, Rn), QL_INT2FP_FMOV, F_FPTYPE | F_SF),
  FF16_INSN ("fmov",  0x1ee70000, 0x7f3ffc00, float2int, OP2 (Fd, Rn), QL_INT2FP_H, F_FPTYPE | F_SF),
  __FP_INSN ("fcvtps",0x1e280000, 0x7f3ffc00, float2int, 0, OP2 (Rd, Fn), QL_FP2INT, F_FPTYPE | F_SF),
  FF16_INSN ("fcvtps",0x1ee80000, 0x7f3ffc00, float2int, OP2 (Rd, Fn), QL_FP2INT_H, F_FPTYPE | F_SF),
  __FP_INSN ("fcvtpu",0x1e290000, 0x7f3ffc00, float2int, 0, OP2 (Rd, Fn), QL_FP2INT, F_FPTYPE | F_SF),
  FF16_INSN ("fcvtpu",0x1ee90000, 0x7f3ffc00, float2int, OP2 (Rd, Fn), QL_FP2INT_H, F_FPTYPE | F_SF),
  __FP_INSN ("fcvtms",0x1e300000, 0x7f3ffc00, float2int, 0, OP2 (Rd, Fn), QL_FP2INT, F_FPTYPE | F_SF),
  FF16_INSN ("fcvtms",0x1ef00000, 0x7f3ffc00, float2int, OP2 (Rd, Fn), QL_FP2INT_H, F_FPTYPE | F_SF),
  __FP_INSN ("fcvtmu",0x1e310000, 0x7f3ffc00, float2int, 0, OP2 (Rd, Fn), QL_FP2INT, F_FPTYPE | F_SF),
  FF16_INSN ("fcvtmu",0x1ef10000, 0x7f3ffc00, float2int, OP2 (Rd, Fn), QL_FP2INT_H, F_FPTYPE | F_SF),
  __FP_INSN ("fcvtzs",0x1e380000, 0x7f3ffc00, float2int, 0, OP2 (Rd, Fn), QL_FP2INT, F_FPTYPE | F_SF),
  FF16_INSN ("fcvtzs",0x1ef80000, 0x7f3ffc00, float2int, OP2 (Rd, Fn), QL_FP2INT_H, F_FPTYPE | F_SF),
  __FP_INSN ("fcvtzu",0x1e390000, 0x7f3ffc00, float2int, 0, OP2 (Rd, Fn), QL_FP2INT, F_FPTYPE | F_SF),
  FF16_INSN ("fcvtzu",0x1ef90000, 0x7f3ffc00, float2int, OP2 (Rd, Fn), QL_FP2INT_H, F_FPTYPE | F_SF),
  __FP_INSN ("fmov",  0x9eae0000, 0xfffffc00, float2int, 0, OP2 (Rd, VnD1), QL_XVD1, 0),
  __FP_INSN ("fmov",  0x9eaf0000, 0xfffffc00, float2int, 0, OP2 (VdD1, Rn), QL_VD1X, 0),
  {"fjcvtzs", 0x1e7e0000, 0xfffffc00, float2int, 0, FP_V8_3, OP2 (Rd, Fn), QL_FP2INT_W_D, 0, 0, 0, NULL },
  /* Floating-point conditional compare.  */
  __FP_INSN ("fccmp", 0x1e200400, 0xff200c10, floatccmp, 0, OP4 (Fn, Fm, NZCV, COND), QL_FCCMP, F_FPTYPE),
  FF16_INSN ("fccmp", 0x1ee00400, 0xff200c10, floatccmp, OP4 (Fn, Fm, NZCV, COND), QL_FCCMP_H, F_FPTYPE),
  __FP_INSN ("fccmpe",0x1e200410, 0xff200c10, floatccmp, 0, OP4 (Fn, Fm, NZCV, COND), QL_FCCMP, F_FPTYPE),
  FF16_INSN ("fccmpe",0x1ee00410, 0xff200c10, floatccmp, OP4 (Fn, Fm, NZCV, COND), QL_FCCMP_H, F_FPTYPE),
  /* Floating-point compare.  */
  __FP_INSN ("fcmp",  0x1e202000, 0xff20fc1f, floatcmp, 0, OP2 (Fn, Fm),     QL_FP2,   F_FPTYPE),
  FF16_INSN ("fcmp",  0x1ee02000, 0xff20fc1f, floatcmp, OP2 (Fn, Fm),     QL_FP2_H, F_FPTYPE),
  __FP_INSN ("fcmpe", 0x1e202010, 0xff20fc1f, floatcmp, 0, OP2 (Fn, Fm),     QL_FP2,   F_FPTYPE),
  FF16_INSN ("fcmpe", 0x1ee02010, 0xff20fc1f, floatcmp, OP2 (Fn, Fm),     QL_FP2_H, F_FPTYPE),
  __FP_INSN ("fcmp",  0x1e202008, 0xff20fc1f, floatcmp, 0, OP2 (Fn, FPIMM0), QL_DST_SD,F_FPTYPE),
  FF16_INSN ("fcmp",  0x1ee02008, 0xff20fc1f, floatcmp, OP2 (Fn, FPIMM0), QL_FP2_H, F_FPTYPE),
  __FP_INSN ("fcmpe", 0x1e202018, 0xff20fc1f, floatcmp, 0, OP2 (Fn, FPIMM0), QL_DST_SD,F_FPTYPE),
  FF16_INSN ("fcmpe", 0x1ee02018, 0xff20fc1f, floatcmp, OP2 (Fn, FPIMM0), QL_FP2_H, F_FPTYPE),
  /* Data processing instructions ARMv8.5-A.  */
  FLAGMANIP_INSN ("xaflag",   0xd500403f, 0xffffffff, 0, OP0 (), {}, 0),
  FLAGMANIP_INSN ("axflag",   0xd500405f, 0xffffffff, 0, OP0 (), {}, 0),
  FRINTTS_INSN ("frint32z", 0x1e284000, 0xffbffc00, floatdp1, OP2 (Fd, Fn), QL_FP2, F_FPTYPE),
  FRINTTS_INSN ("frint32x", 0x1e28c000, 0xffbffc00, floatdp1, OP2 (Fd, Fn), QL_FP2, F_FPTYPE),
  FRINTTS_INSN ("frint64z", 0x1e294000, 0xffbffc00, floatdp1, OP2 (Fd, Fn), QL_FP2, F_FPTYPE),
  FRINTTS_INSN ("frint64x", 0x1e29c000, 0xffbffc00, floatdp1, OP2 (Fd, Fn), QL_FP2, F_FPTYPE),
  /* Floating-point data-processing (1 source).  */
  __FP_INSN ("fmov",  0x1e204000, 0xff3ffc00, floatdp1, 0, OP2 (Fd, Fn), QL_FP2,   F_FPTYPE),
  FF16_INSN ("fmov",  0x1ee04000, 0xff3ffc00, floatdp1, OP2 (Fd, Fn), QL_FP2_H, F_FPTYPE),
  __FP_INSN ("fabs",  0x1e20c000, 0xff3ffc00, floatdp1, 0, OP2 (Fd, Fn), QL_FP2,   F_FPTYPE),
  FF16_INSN ("fabs",  0x1ee0c000, 0xff3ffc00, floatdp1, OP2 (Fd, Fn), QL_FP2_H, F_FPTYPE),
  __FP_INSN ("fneg",  0x1e214000, 0xff3ffc00, floatdp1, 0, OP2 (Fd, Fn), QL_FP2,   F_FPTYPE),
  FF16_INSN ("fneg",  0x1ee14000, 0xff3ffc00, floatdp1, OP2 (Fd, Fn), QL_FP2_H, F_FPTYPE),
  __FP_INSN ("fsqrt", 0x1e21c000, 0xff3ffc00, floatdp1, 0, OP2 (Fd, Fn), QL_FP2,   F_FPTYPE),
  FF16_INSN ("fsqrt", 0x1ee1c000, 0xff3ffc00, floatdp1, OP2 (Fd, Fn), QL_FP2_H, F_FPTYPE),
  __FP_INSN ("fcvt", 0x1e224000, 0xff3e7c00, floatdp1, OP_FCVT, OP2 (Fd, Fn), QL_FCVT, F_FPTYPE | F_MISC),
  __FP_INSN ("frintn",0x1e244000, 0xff3ffc00, floatdp1, 0, OP2 (Fd, Fn), QL_FP2,   F_FPTYPE),
  FF16_INSN ("frintn",0x1ee44000, 0xff3ffc00, floatdp1, OP2 (Fd, Fn), QL_FP2_H, F_FPTYPE),
  __FP_INSN ("frintp",0x1e24c000, 0xff3ffc00, floatdp1, 0, OP2 (Fd, Fn), QL_FP2,   F_FPTYPE),
  FF16_INSN ("frintp",0x1ee4c000, 0xff3ffc00, floatdp1, OP2 (Fd, Fn), QL_FP2_H, F_FPTYPE),
  __FP_INSN ("frintm",0x1e254000, 0xff3ffc00, floatdp1, 0, OP2 (Fd, Fn), QL_FP2,   F_FPTYPE),
  FF16_INSN ("frintm",0x1ee54000, 0xff3ffc00, floatdp1, OP2 (Fd, Fn), QL_FP2_H, F_FPTYPE),
  __FP_INSN ("frintz",0x1e25c000, 0xff3ffc00, floatdp1, 0, OP2 (Fd, Fn), QL_FP2,   F_FPTYPE),
  FF16_INSN ("frintz",0x1ee5c000, 0xff3ffc00, floatdp1, OP2 (Fd, Fn), QL_FP2_H, F_FPTYPE),
  __FP_INSN ("frinta",0x1e264000, 0xff3ffc00, floatdp1, 0, OP2 (Fd, Fn), QL_FP2,   F_FPTYPE),
  FF16_INSN ("frinta",0x1ee64000, 0xff3ffc00, floatdp1, OP2 (Fd, Fn), QL_FP2_H, F_FPTYPE),
  __FP_INSN ("frintx",0x1e274000, 0xff3ffc00, floatdp1, 0, OP2 (Fd, Fn), QL_FP2,   F_FPTYPE),
  FF16_INSN ("frintx",0x1ee74000, 0xff3ffc00, floatdp1, OP2 (Fd, Fn), QL_FP2_H, F_FPTYPE),
  __FP_INSN ("frinti",0x1e27c000, 0xff3ffc00, floatdp1, 0, OP2 (Fd, Fn), QL_FP2,   F_FPTYPE),
  FF16_INSN ("frinti",0x1ee7c000, 0xff3ffc00, floatdp1, OP2 (Fd, Fn), QL_FP2_H, F_FPTYPE),
  /* Floating-point data-processing (2 source).  */
  __FP_INSN ("fmul",  0x1e200800, 0xff20fc00, floatdp2, 0, OP3 (Fd, Fn, Fm), QL_FP3,   F_FPTYPE),
  FF16_INSN ("fmul",  0x1ee00800, 0xff20fc00, floatdp2, OP3 (Fd, Fn, Fm), QL_FP3_H, F_FPTYPE),
  __FP_INSN ("fdiv",  0x1e201800, 0xff20fc00, floatdp2, 0, OP3 (Fd, Fn, Fm), QL_FP3,   F_FPTYPE),
  FF16_INSN ("fdiv",  0x1ee01800, 0xff20fc00, floatdp2, OP3 (Fd, Fn, Fm), QL_FP3_H, F_FPTYPE),
  __FP_INSN ("fadd",  0x1e202800, 0xff20fc00, floatdp2, 0, OP3 (Fd, Fn, Fm), QL_FP3,   F_FPTYPE),
  FF16_INSN ("fadd",  0x1ee02800, 0xff20fc00, floatdp2, OP3 (Fd, Fn, Fm), QL_FP3_H, F_FPTYPE),
  __FP_INSN ("fsub",  0x1e203800, 0xff20fc00, floatdp2, 0, OP3 (Fd, Fn, Fm), QL_FP3,   F_FPTYPE),
  FF16_INSN ("fsub",  0x1ee03800, 0xff20fc00, floatdp2, OP3 (Fd, Fn, Fm), QL_FP3_H, F_FPTYPE),
  __FP_INSN ("fmax",  0x1e204800, 0xff20fc00, floatdp2, 0, OP3 (Fd, Fn, Fm), QL_FP3,   F_FPTYPE),
  FF16_INSN ("fmax",  0x1ee04800, 0xff20fc00, floatdp2, OP3 (Fd, Fn, Fm), QL_FP3_H, F_FPTYPE),
  __FP_INSN ("fmin",  0x1e205800, 0xff20fc00, floatdp2, 0, OP3 (Fd, Fn, Fm), QL_FP3,   F_FPTYPE),
  FF16_INSN ("fmin",  0x1ee05800, 0xff20fc00, floatdp2, OP3 (Fd, Fn, Fm), QL_FP3_H, F_FPTYPE),
  __FP_INSN ("fmaxnm",0x1e206800, 0xff20fc00, floatdp2, 0, OP3 (Fd, Fn, Fm), QL_FP3,   F_FPTYPE),
  FF16_INSN ("fmaxnm",0x1ee06800, 0xff20fc00, floatdp2, OP3 (Fd, Fn, Fm), QL_FP3_H, F_FPTYPE),
  __FP_INSN ("fminnm",0x1e207800, 0xff20fc00, floatdp2, 0, OP3 (Fd, Fn, Fm), QL_FP3,   F_FPTYPE),
  FF16_INSN ("fminnm",0x1ee07800, 0xff20fc00, floatdp2, OP3 (Fd, Fn, Fm), QL_FP3_H, F_FPTYPE),
  __FP_INSN ("fnmul", 0x1e208800, 0xff20fc00, floatdp2, 0, OP3 (Fd, Fn, Fm), QL_FP3,   F_FPTYPE),
  FF16_INSN ("fnmul", 0x1ee08800, 0xff20fc00, floatdp2, OP3 (Fd, Fn, Fm), QL_FP3_H, F_FPTYPE),
  /* Floating-point data-processing (3 source).  */
  __FP_INSN ("fmadd", 0x1f000000, 0xff208000, floatdp3, 0, OP4 (Fd, Fn, Fm, Fa), QL_FP4,   F_FPTYPE),
  FF16_INSN ("fmadd", 0x1fc00000, 0xff208000, floatdp3, OP4 (Fd, Fn, Fm, Fa), QL_FP4_H, F_FPTYPE),
  __FP_INSN ("fmsub", 0x1f008000, 0xff208000, floatdp3, 0, OP4 (Fd, Fn, Fm, Fa), QL_FP4,   F_FPTYPE),
  FF16_INSN ("fmsub", 0x1fc08000, 0xff208000, floatdp3, OP4 (Fd, Fn, Fm, Fa), QL_FP4_H, F_FPTYPE),
  __FP_INSN ("fnmadd",0x1f200000, 0xff208000, floatdp3, 0, OP4 (Fd, Fn, Fm, Fa), QL_FP4,   F_FPTYPE),
  FF16_INSN ("fnmadd",0x1fe00000, 0xff208000, floatdp3, OP4 (Fd, Fn, Fm, Fa), QL_FP4_H, F_FPTYPE),
  __FP_INSN ("fnmsub",0x1f208000, 0xff208000, floatdp3, 0, OP4 (Fd, Fn, Fm, Fa), QL_FP4,   F_FPTYPE),
  FF16_INSN ("fnmsub",0x1fe08000, 0xff208000, floatdp3, OP4 (Fd, Fn, Fm, Fa), QL_FP4_H, F_FPTYPE),
  /* Floating-point immediate.  */
  __FP_INSN ("fmov", 0x1e201000, 0xff201fe0, floatimm, 0, OP2 (Fd, FPIMM), QL_DST_SD, F_FPTYPE),
  FF16_INSN ("fmov", 0x1ee01000, 0xff201fe0, floatimm, OP2 (Fd, FPIMM), QL_DST_H, F_FPTYPE),
  /* Floating-point conditional select.  */
  __FP_INSN ("fcsel", 0x1e200c00, 0xff200c00, floatsel, 0, OP4 (Fd, Fn, Fm, COND), QL_FP_COND, F_FPTYPE),
  FF16_INSN ("fcsel", 0x1ee00c00, 0xff200c00, floatsel, OP4 (Fd, Fn, Fm, COND), QL_FP_COND_H, F_FPTYPE),
  /* Load/store register (immediate indexed).  */
  CORE_INSN ("strb", 0x38000400, 0xffe00400, ldst_imm9, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_W8, 0),
  CORE_INSN ("ldrb", 0x38400400, 0xffe00400, ldst_imm9, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_W8, 0),
  CORE_INSN ("ldrsb", 0x38800400, 0xffa00400, ldst_imm9, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_R8, F_LDS_SIZE),
  CORE_INSN ("str", 0x3c000400, 0x3f600400, ldst_imm9, 0, OP2 (Ft, ADDR_SIMM9), QL_LDST_FP, 0),
  CORE_INSN ("ldr", 0x3c400400, 0x3f600400, ldst_imm9, 0, OP2 (Ft, ADDR_SIMM9), QL_LDST_FP, 0),
  CORE_INSN ("strh", 0x78000400, 0xffe00400, ldst_imm9, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_W16, 0),
  CORE_INSN ("ldrh", 0x78400400, 0xffe00400, ldst_imm9, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_W16, 0),
  CORE_INSN ("ldrsh", 0x78800400, 0xffa00400, ldst_imm9, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_R16, F_LDS_SIZE),
  CORE_INSN ("str", 0xb8000400, 0xbfe00400, ldst_imm9, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_R, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldr", 0xb8400400, 0xbfe00400, ldst_imm9, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_R, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldrsw", 0xb8800400, 0xffe00400, ldst_imm9, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_X32, 0),
  /* Load/store Allocation Tag instructions.  */
  MEMTAG_INSN ("stg",  0xd9200800, 0xffe00c00, ldst_unscaled, OP2 (Rt_SP, ADDR_SIMM13), QL_LDST_AT, 0),
  MEMTAG_INSN ("stzg", 0xd9600800, 0xffe00c00, ldst_unscaled, OP2 (Rt_SP, ADDR_SIMM13), QL_LDST_AT, 0),
  MEMTAG_INSN ("st2g", 0xd9a00800, 0xffe00c00, ldst_unscaled, OP2 (Rt_SP, ADDR_SIMM13), QL_LDST_AT, 0),
  MEMTAG_INSN ("stz2g",0xd9e00800, 0xffe00c00, ldst_unscaled, OP2 (Rt_SP, ADDR_SIMM13), QL_LDST_AT, 0),
  MEMTAG_INSN ("stg",  0xd9200400, 0xffe00400, ldst_imm9, OP2 (Rt_SP, ADDR_SIMM13), QL_LDST_AT, 0),
  MEMTAG_INSN ("stzg", 0xd9600400, 0xffe00400, ldst_imm9, OP2 (Rt_SP, ADDR_SIMM13), QL_LDST_AT, 0),
  MEMTAG_INSN ("st2g", 0xd9a00400, 0xffe00400, ldst_imm9, OP2 (Rt_SP, ADDR_SIMM13), QL_LDST_AT, 0),
  MEMTAG_INSN ("stz2g",0xd9e00400, 0xffe00400, ldst_imm9, OP2 (Rt_SP, ADDR_SIMM13), QL_LDST_AT, 0),
  /* Load/store register (unsigned immediate).  */
  CORE_INSN ("strb", 0x39000000, 0xffc00000, ldst_pos, OP_STRB_POS, OP2 (Rt, ADDR_UIMM12), QL_LDST_W8, 0),
  CORE_INSN ("ldrb", 0x39400000, 0xffc00000, ldst_pos, OP_LDRB_POS, OP2 (Rt, ADDR_UIMM12), QL_LDST_W8, 0),
  CORE_INSN ("ldrsb", 0x39800000, 0xff800000, ldst_pos, OP_LDRSB_POS, OP2 (Rt, ADDR_UIMM12), QL_LDST_R8, F_LDS_SIZE),
  CORE_INSN ("str", 0x3d000000, 0x3f400000, ldst_pos, OP_STRF_POS, OP2 (Ft, ADDR_UIMM12), QL_LDST_FP, 0),
  CORE_INSN ("ldr", 0x3d400000, 0x3f400000, ldst_pos, OP_LDRF_POS, OP2 (Ft, ADDR_UIMM12), QL_LDST_FP, 0),
  CORE_INSN ("strh", 0x79000000, 0xffc00000, ldst_pos, OP_STRH_POS, OP2 (Rt, ADDR_UIMM12), QL_LDST_W16, 0),
  CORE_INSN ("ldrh", 0x79400000, 0xffc00000, ldst_pos, OP_LDRH_POS, OP2 (Rt, ADDR_UIMM12), QL_LDST_W16, 0),
  CORE_INSN ("ldrsh", 0x79800000, 0xff800000, ldst_pos, OP_LDRSH_POS, OP2 (Rt, ADDR_UIMM12), QL_LDST_R16, F_LDS_SIZE),
  CORE_INSN ("str", 0xb9000000, 0xbfc00000, ldst_pos, OP_STR_POS, OP2 (Rt, ADDR_UIMM12), QL_LDST_R, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldr", 0xb9400000, 0xbfc00000, ldst_pos, OP_LDR_POS, OP2 (Rt, ADDR_UIMM12), QL_LDST_R, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldrsw", 0xb9800000, 0xffc00000, ldst_pos, OP_LDRSW_POS, OP2 (Rt, ADDR_UIMM12), QL_LDST_X32, 0),
  CORE_INSN ("prfm", 0xf9800000, 0xffc00000, ldst_pos, OP_PRFM_POS, OP2 (PRFOP, ADDR_UIMM12), QL_LDST_PRFM, 0),
  /* Load/store register (register offset).  */
  CORE_INSN ("strb", 0x38200800, 0xffe00c00, ldst_regoff, 0, OP2 (Rt, ADDR_REGOFF), QL_LDST_W8, 0),
  CORE_INSN ("ldrb", 0x38600800, 0xffe00c00, ldst_regoff, 0, OP2 (Rt, ADDR_REGOFF), QL_LDST_W8, 0),
  CORE_INSN ("ldrsb", 0x38a00800, 0xffa00c00, ldst_regoff, 0, OP2 (Rt, ADDR_REGOFF), QL_LDST_R8, F_LDS_SIZE),
  CORE_INSN ("str", 0x3c200800, 0x3f600c00, ldst_regoff, 0, OP2 (Ft, ADDR_REGOFF), QL_LDST_FP, 0),
  CORE_INSN ("ldr", 0x3c600800, 0x3f600c00, ldst_regoff, 0, OP2 (Ft, ADDR_REGOFF), QL_LDST_FP, 0),
  CORE_INSN ("strh", 0x78200800, 0xffe00c00, ldst_regoff, 0, OP2 (Rt, ADDR_REGOFF), QL_LDST_W16, 0),
  CORE_INSN ("ldrh", 0x78600800, 0xffe00c00, ldst_regoff, 0, OP2 (Rt, ADDR_REGOFF), QL_LDST_W16, 0),
  CORE_INSN ("ldrsh", 0x78a00800, 0xffa00c00, ldst_regoff, 0, OP2 (Rt, ADDR_REGOFF), QL_LDST_R16, F_LDS_SIZE),
  CORE_INSN ("str", 0xb8200800, 0xbfe00c00, ldst_regoff, 0, OP2 (Rt, ADDR_REGOFF), QL_LDST_R, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldr", 0xb8600800, 0xbfe00c00, ldst_regoff, 0, OP2 (Rt, ADDR_REGOFF), QL_LDST_R, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldrsw", 0xb8a00800, 0xffe00c00, ldst_regoff, 0, OP2 (Rt, ADDR_REGOFF), QL_LDST_X32, 0),
  CORE_INSN ("prfm", 0xf8a00800, 0xffe00c00, ldst_regoff, 0, OP2 (PRFOP, ADDR_REGOFF), QL_LDST_PRFM, 0),
  /* Load/store register (unprivileged).  */
  CORE_INSN ("sttrb", 0x38000800, 0xffe00c00, ldst_unpriv, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_W8, 0),
  CORE_INSN ("ldtrb", 0x38400800, 0xffe00c00, ldst_unpriv, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_W8, 0),
  CORE_INSN ("ldtrsb", 0x38800800, 0xffa00c00, ldst_unpriv, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_R8, F_LDS_SIZE),
  CORE_INSN ("sttrh", 0x78000800, 0xffe00c00, ldst_unpriv, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_W16, 0),
  CORE_INSN ("ldtrh", 0x78400800, 0xffe00c00, ldst_unpriv, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_W16, 0),
  CORE_INSN ("ldtrsh", 0x78800800, 0xffa00c00, ldst_unpriv, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_R16, F_LDS_SIZE),
  CORE_INSN ("sttr", 0xb8000800, 0xbfe00c00, ldst_unpriv, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_R, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldtr", 0xb8400800, 0xbfe00c00, ldst_unpriv, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_R, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldtrsw", 0xb8800800, 0xffe00c00, ldst_unpriv, 0, OP2 (Rt, ADDR_SIMM9), QL_LDST_X32, 0),
  /* Load/store register (unscaled immediate).  */
  CORE_INSN ("sturb", 0x38000000, 0xffe00c00, ldst_unscaled, OP_STURB, OP2 (Rt, ADDR_SIMM9), QL_LDST_W8, 0),
  CORE_INSN ("ldurb", 0x38400000, 0xffe00c00, ldst_unscaled, OP_LDURB, OP2 (Rt, ADDR_SIMM9), QL_LDST_W8, 0),
  CORE_INSN ("ldursb", 0x38800000, 0xffa00c00, ldst_unscaled, OP_LDURSB, OP2 (Rt, ADDR_SIMM9), QL_LDST_R8, F_LDS_SIZE),
  CORE_INSN ("stur", 0x3c000000, 0x3f600c00, ldst_unscaled, OP_STURV, OP2 (Ft, ADDR_SIMM9), QL_LDST_FP, 0),
  CORE_INSN ("ldur", 0x3c400000, 0x3f600c00, ldst_unscaled, OP_LDURV, OP2 (Ft, ADDR_SIMM9), QL_LDST_FP, 0),
  CORE_INSN ("sturh", 0x78000000, 0xffe00c00, ldst_unscaled, OP_STURH, OP2 (Rt, ADDR_SIMM9), QL_LDST_W16, 0),
  CORE_INSN ("ldurh", 0x78400000, 0xffe00c00, ldst_unscaled, OP_LDURH, OP2 (Rt, ADDR_SIMM9), QL_LDST_W16, 0),
  CORE_INSN ("ldursh", 0x78800000, 0xffa00c00, ldst_unscaled, OP_LDURSH, OP2 (Rt, ADDR_SIMM9), QL_LDST_R16, F_LDS_SIZE),
  CORE_INSN ("stur", 0xb8000000, 0xbfe00c00, ldst_unscaled, OP_STUR, OP2 (Rt, ADDR_SIMM9), QL_LDST_R, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldur", 0xb8400000, 0xbfe00c00, ldst_unscaled, OP_LDUR, OP2 (Rt, ADDR_SIMM9), QL_LDST_R, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldursw", 0xb8800000, 0xffe00c00, ldst_unscaled, OP_LDURSW, OP2 (Rt, ADDR_SIMM9), QL_LDST_X32, 0),
  CORE_INSN ("prfum", 0xf8800000, 0xffe00c00, ldst_unscaled, OP_PRFUM, OP2 (PRFOP, ADDR_SIMM9), QL_LDST_PRFM, 0),
  MEMTAG_INSN ("ldg",  0xd9600000, 0xffe00c00, ldst_unscaled, OP2 (Rt, ADDR_SIMM13), QL_LDG, 0),
  /* Load/store register (scaled signed immediate).  */
  PAC_INSN ("ldraa", 0xf8200400, 0xffa00400, ldst_imm10, OP2 (Rt, ADDR_SIMM10), QL_X1NIL, 0),
  PAC_INSN ("ldrab", 0xf8a00400, 0xffa00400, ldst_imm10, OP2 (Rt, ADDR_SIMM10), QL_X1NIL, 0),
  /* Load/store exclusive.  */
  CORE_INSN ("stxrb", 0x8007c00, 0xffe08000, ldstexcl, 0, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  CORE_INSN ("stlxrb", 0x800fc00, 0xffe08000, ldstexcl, 0, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  CORE_INSN ("ldxrb", 0x85f7c00, 0xffe08000, ldstexcl, 0, OP2 (Rt, ADDR_SIMPLE), QL_W1_LDST_EXC, 0),
  CORE_INSN ("ldaxrb", 0x85ffc00, 0xffe08000, ldstexcl, 0, OP2 (Rt, ADDR_SIMPLE), QL_W1_LDST_EXC, 0),
  CORE_INSN ("stlrb", 0x89ffc00, 0xffe08000, ldstexcl, 0, OP2 (Rt, ADDR_SIMPLE), QL_W1_LDST_EXC, 0),
  CORE_INSN ("ldarb", 0x8dffc00, 0xffeffc00, ldstexcl, 0, OP2 (Rt, ADDR_SIMPLE), QL_W1_LDST_EXC, 0),
  CORE_INSN ("stxrh", 0x48007c00, 0xffe08000, ldstexcl, 0, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  CORE_INSN ("stlxrh", 0x4800fc00, 0xffe08000, ldstexcl, 0, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  CORE_INSN ("ldxrh", 0x485f7c00, 0xffe08000, ldstexcl, 0, OP2 (Rt, ADDR_SIMPLE), QL_W1_LDST_EXC, 0),
  CORE_INSN ("ldaxrh", 0x485ffc00, 0xffe08000, ldstexcl, 0, OP2 (Rt, ADDR_SIMPLE), QL_W1_LDST_EXC, 0),
  CORE_INSN ("stlrh", 0x489ffc00, 0xffe08000, ldstexcl, 0, OP2 (Rt, ADDR_SIMPLE), QL_W1_LDST_EXC, 0),
  CORE_INSN ("ldarh", 0x48dffc00, 0xfffffc00, ldstexcl, 0, OP2 (Rt, ADDR_SIMPLE), QL_W1_LDST_EXC, 0),
  CORE_INSN ("stxr", 0x88007c00, 0xbfe08000, ldstexcl, 0, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2_LDST_EXC, F_GPRSIZE_IN_Q),
  CORE_INSN ("stlxr", 0x8800fc00, 0xbfe08000, ldstexcl, 0, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2_LDST_EXC, F_GPRSIZE_IN_Q),
  CORE_INSN ("stxp", 0x88200000, 0xbfe08000, ldstexcl, 0, OP4 (Rs, Rt, Rt2, ADDR_SIMPLE), QL_R3_LDST_EXC, F_GPRSIZE_IN_Q),
  CORE_INSN ("stlxp", 0x88208000, 0xbfe08000, ldstexcl, 0, OP4 (Rs, Rt, Rt2, ADDR_SIMPLE), QL_R3_LDST_EXC, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldxr", 0x885f7c00, 0xbfe08000, ldstexcl, 0, OP2 (Rt, ADDR_SIMPLE), QL_R1NIL, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldaxr", 0x885ffc00, 0xbfe08000, ldstexcl, 0, OP2 (Rt, ADDR_SIMPLE), QL_R1NIL, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldxp", 0x887f0000, 0xbfe08000, ldstexcl, 0, OP3 (Rt, Rt2, ADDR_SIMPLE), QL_R2NIL, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldaxp", 0x887f8000, 0xbfe08000, ldstexcl, 0, OP3 (Rt, Rt2, ADDR_SIMPLE), QL_R2NIL, F_GPRSIZE_IN_Q),
  CORE_INSN ("stlr", 0x889ffc00, 0xbfe08000, ldstexcl, 0, OP2 (Rt, ADDR_SIMPLE), QL_R1NIL, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldar", 0x88dffc00, 0xbfeffc00, ldstexcl, 0, OP2 (Rt, ADDR_SIMPLE), QL_R1NIL, F_GPRSIZE_IN_Q),
  RCPC_INSN ("ldaprb", 0x38bfc000, 0xfffffc00, ldstexcl, OP2 (Rt, ADDR_SIMPLE), QL_W1_LDST_EXC, 0),
  RCPC_INSN ("ldaprh", 0x78bfc000, 0xfffffc00, ldstexcl, OP2 (Rt, ADDR_SIMPLE), QL_W1_LDST_EXC, 0),
  RCPC_INSN ("ldapr", 0xb8bfc000, 0xbffffc00, ldstexcl, OP2 (Rt, ADDR_SIMPLE), QL_R1NIL, F_GPRSIZE_IN_Q),
  MEMTAG_INSN ("ldgm", 0xd9e00000, 0xfffffc00, ldstexcl, OP2 (Rt, ADDR_SIMPLE), QL_X1NIL, 0),
  MEMTAG_INSN ("stgm", 0xd9a00000, 0xfffffc00, ldstexcl, OP2 (Rt, ADDR_SIMPLE), QL_X1NIL, 0),
  MEMTAG_INSN ("stzgm", 0xd9200000, 0xfffffc00, ldstexcl, OP2 (Rt, ADDR_SIMPLE), QL_X1NIL, 0),
  /* Limited Ordering Regions load/store instructions.  */
  _LOR_INSN ("ldlar",  0x88df7c00, 0xbfe08000, ldstexcl, OP2 (Rt, ADDR_SIMPLE), QL_R1NIL,       F_GPRSIZE_IN_Q),
  _LOR_INSN ("ldlarb", 0x08df7c00, 0xffe08000, ldstexcl, OP2 (Rt, ADDR_SIMPLE), QL_W1_LDST_EXC, 0),
  _LOR_INSN ("ldlarh", 0x48df7c00, 0xffe08000, ldstexcl, OP2 (Rt, ADDR_SIMPLE), QL_W1_LDST_EXC, 0),
  _LOR_INSN ("stllr",  0x889f7c00, 0xbfe08000, ldstexcl, OP2 (Rt, ADDR_SIMPLE), QL_R1NIL,       F_GPRSIZE_IN_Q),
  _LOR_INSN ("stllrb", 0x089f7c00, 0xffe08000, ldstexcl, OP2 (Rt, ADDR_SIMPLE), QL_W1_LDST_EXC, 0),
  _LOR_INSN ("stllrh", 0x489f7c00, 0xbfe08000, ldstexcl, OP2 (Rt, ADDR_SIMPLE), QL_W1_LDST_EXC, 0),
  /* Load/store no-allocate pair (offset).  */
  CORE_INSN ("stnp", 0x28000000, 0x7fc00000, ldstnapair_offs, 0, OP3 (Rt, Rt2, ADDR_SIMM7), QL_LDST_PAIR_R, F_SF),
  CORE_INSN ("ldnp", 0x28400000, 0x7fc00000, ldstnapair_offs, 0, OP3 (Rt, Rt2, ADDR_SIMM7), QL_LDST_PAIR_R, F_SF),
  CORE_INSN ("stnp", 0x2c000000, 0x3fc00000, ldstnapair_offs, 0, OP3 (Ft, Ft2, ADDR_SIMM7), QL_LDST_PAIR_FP, 0),
  CORE_INSN ("ldnp", 0x2c400000, 0x3fc00000, ldstnapair_offs, 0, OP3 (Ft, Ft2, ADDR_SIMM7), QL_LDST_PAIR_FP, 0),
  /* Load/store register pair (offset).  */
  CORE_INSN ("stp", 0x29000000, 0x7ec00000, ldstpair_off, 0, OP3 (Rt, Rt2, ADDR_SIMM7), QL_LDST_PAIR_R, F_SF),
  CORE_INSN ("ldp", 0x29400000, 0x7ec00000, ldstpair_off, 0, OP3 (Rt, Rt2, ADDR_SIMM7), QL_LDST_PAIR_R, F_SF),
  CORE_INSN ("stp", 0x2d000000, 0x3fc00000, ldstpair_off, 0, OP3 (Ft, Ft2, ADDR_SIMM7), QL_LDST_PAIR_FP, 0),
  CORE_INSN ("ldp", 0x2d400000, 0x3fc00000, ldstpair_off, 0, OP3 (Ft, Ft2, ADDR_SIMM7), QL_LDST_PAIR_FP, 0),
  {"ldpsw", 0x69400000, 0xffc00000, ldstpair_off, 0, CORE, OP3 (Rt, Rt2, ADDR_SIMM7), QL_LDST_PAIR_X32, 0, 0, 0, VERIFIER (ldpsw)},
  MEMTAG_INSN ("stgp", 0x69000000, 0xffc00000, ldstpair_off, OP3 (Rt, Rt2, ADDR_SIMM11), QL_STGP, 0),
  /* Load/store register pair (indexed).  */
  CORE_INSN ("stp", 0x28800000, 0x7ec00000, ldstpair_indexed, 0, OP3 (Rt, Rt2, ADDR_SIMM7), QL_LDST_PAIR_R, F_SF),
  CORE_INSN ("ldp", 0x28c00000, 0x7ec00000, ldstpair_indexed, 0, OP3 (Rt, Rt2, ADDR_SIMM7), QL_LDST_PAIR_R, F_SF),
  CORE_INSN ("stp", 0x2c800000, 0x3ec00000, ldstpair_indexed, 0, OP3 (Ft, Ft2, ADDR_SIMM7), QL_LDST_PAIR_FP, 0),
  CORE_INSN ("ldp", 0x2cc00000, 0x3ec00000, ldstpair_indexed, 0, OP3 (Ft, Ft2, ADDR_SIMM7), QL_LDST_PAIR_FP, 0),
  {"ldpsw", 0x68c00000, 0xfec00000, ldstpair_indexed, 0, CORE, OP3 (Rt, Rt2, ADDR_SIMM7), QL_LDST_PAIR_X32, 0, 0, 0, VERIFIER (ldpsw)},
  MEMTAG_INSN ("stgp", 0x68800000, 0xfec00000, ldstpair_indexed, OP3 (Rt, Rt2, ADDR_SIMM11), QL_STGP, 0),
  /* Load register (literal).  */
  CORE_INSN ("ldr",   0x18000000, 0xbf000000, loadlit, OP_LDR_LIT,   OP2 (Rt, ADDR_PCREL19),    QL_R_PCREL, F_GPRSIZE_IN_Q),
  CORE_INSN ("ldr",   0x1c000000, 0x3f000000, loadlit, OP_LDRV_LIT,  OP2 (Ft, ADDR_PCREL19),    QL_FP_PCREL, 0),
  CORE_INSN ("ldrsw", 0x98000000, 0xff000000, loadlit, OP_LDRSW_LIT, OP2 (Rt, ADDR_PCREL19),    QL_X_PCREL, 0),
  CORE_INSN ("prfm",  0xd8000000, 0xff000000, loadlit, OP_PRFM_LIT,  OP2 (PRFOP, ADDR_PCREL19), QL_PRFM_PCREL, 0),
  /* Atomic 64-byte load/store in Armv8.7.  */
  _LS64_INSN ("ld64b",   0xf83fd000, 0xfffffc00, lse_atomic, OP2 (Rt_LS64, ADDR_SIMPLE), QL_X1NIL, 0),
  _LS64_INSN ("st64b",   0xf83f9000, 0xfffffc00, lse_atomic, OP2 (Rt_LS64, ADDR_SIMPLE), QL_X1NIL, 0),
  _LS64_INSN ("st64bv",  0xf820b000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt_LS64, ADDR_SIMPLE), QL_X2NIL, 0),
  _LS64_INSN ("st64bv0", 0xf820a000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt_LS64, ADDR_SIMPLE), QL_X2NIL, 0),
  /* Logical (immediate).  */
  CORE_INSN ("and", 0x12000000, 0x7f800000, log_imm, 0, OP3 (Rd_SP, Rn, LIMM), QL_R2NIL, F_HAS_ALIAS | F_SF),
  CORE_INSN ("bic", 0x12000000, 0x7f800000, log_imm, OP_BIC, OP3 (Rd_SP, Rn, LIMM), QL_R2NIL, F_ALIAS | F_PSEUDO | F_SF),
  CORE_INSN ("orr", 0x32000000, 0x7f800000, log_imm, 0, OP3 (Rd_SP, Rn, LIMM), QL_R2NIL, F_HAS_ALIAS | F_SF),
  CORE_INSN ("mov", 0x320003e0, 0x7f8003e0, log_imm, OP_MOV_IMM_LOG, OP2 (Rd_SP, IMM_MOV), QL_R1NIL, F_ALIAS | F_P1 | F_SF | F_CONV),
  CORE_INSN ("eor", 0x52000000, 0x7f800000, log_imm, 0, OP3 (Rd_SP, Rn, LIMM), QL_R2NIL, F_SF),
  CORE_INSN ("ands", 0x72000000, 0x7f800000, log_imm, 0, OP3 (Rd, Rn, LIMM), QL_R2NIL, F_HAS_ALIAS | F_SF),
  CORE_INSN ("tst", 0x7200001f, 0x7f80001f, log_imm, 0, OP2 (Rn, LIMM), QL_R1NIL, F_ALIAS | F_SF),
  /* Logical (shifted register).  */
  CORE_INSN ("and", 0xa000000, 0x7f200000, log_shift, 0, OP3 (Rd, Rn, Rm_SFT), QL_I3SAMER, F_SF),
  CORE_INSN ("bic", 0xa200000, 0x7f200000, log_shift, 0, OP3 (Rd, Rn, Rm_SFT), QL_I3SAMER, F_SF),
  CORE_INSN ("orr", 0x2a000000, 0x7f200000, log_shift, 0, OP3 (Rd, Rn, Rm_SFT), QL_I3SAMER, F_HAS_ALIAS | F_SF),
  CORE_INSN ("mov", 0x2a0003e0, 0x7fe0ffe0, log_shift, 0, OP2 (Rd, Rm_SFT), QL_I2SAMER, F_ALIAS | F_SF),
  CORE_INSN ("uxtw", 0x2a0003e0, 0x7f2003e0, log_shift, OP_UXTW, OP2 (Rd, Rm), QL_I2SAMEW, F_ALIAS | F_PSEUDO),
  CORE_INSN ("orn", 0x2a200000, 0x7f200000, log_shift, 0, OP3 (Rd, Rn, Rm_SFT), QL_I3SAMER, F_HAS_ALIAS | F_SF),
  CORE_INSN ("mvn", 0x2a2003e0, 0x7f2003e0, log_shift, 0, OP2 (Rd, Rm_SFT), QL_I2SAMER, F_ALIAS | F_SF),
  CORE_INSN ("eor", 0x4a000000, 0x7f200000, log_shift, 0, OP3 (Rd, Rn, Rm_SFT), QL_I3SAMER, F_SF),
  CORE_INSN ("eon", 0x4a200000, 0x7f200000, log_shift, 0, OP3 (Rd, Rn, Rm_SFT), QL_I3SAMER, F_SF),
  CORE_INSN ("ands", 0x6a000000, 0x7f200000, log_shift, 0, OP3 (Rd, Rn, Rm_SFT), QL_I3SAMER, F_HAS_ALIAS | F_SF),
  CORE_INSN ("tst", 0x6a00001f, 0x7f20001f, log_shift, 0, OP2 (Rn, Rm_SFT), QL_I2SAMER, F_ALIAS | F_SF),
  CORE_INSN ("bics", 0x6a200000, 0x7f200000, log_shift, 0, OP3 (Rd, Rn, Rm_SFT), QL_I3SAMER, F_SF),
  /* LSE extension (atomic).  */
  _LSE_INSN ("casb", 0x8a07c00, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("cash", 0x48a07c00, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("cas", 0x88a07c00, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("casab", 0x8e07c00, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("caslb", 0x8a0fc00, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("casalb", 0x8e0fc00, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("casah", 0x48e07c00, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("caslh", 0x48a0fc00, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("casalh", 0x48e0fc00, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("casa", 0x88e07c00, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("casl", 0x88a0fc00, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("casal", 0x88e0fc00, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("casp", 0x8207c00, 0xbfe0fc00, lse_atomic, OP5 (Rs, PAIRREG, Rt, PAIRREG, ADDR_SIMPLE), QL_R4NIL, F_LSE_SZ),
  _LSE_INSN ("caspa", 0x8607c00, 0xbfe0fc00, lse_atomic, OP5 (Rs, PAIRREG, Rt, PAIRREG, ADDR_SIMPLE), QL_R4NIL, F_LSE_SZ),
  _LSE_INSN ("caspl", 0x820fc00, 0xbfe0fc00, lse_atomic, OP5 (Rs, PAIRREG, Rt, PAIRREG, ADDR_SIMPLE), QL_R4NIL, F_LSE_SZ),
  _LSE_INSN ("caspal", 0x860fc00, 0xbfe0fc00, lse_atomic, OP5 (Rs, PAIRREG, Rt, PAIRREG, ADDR_SIMPLE), QL_R4NIL, F_LSE_SZ),
  _LSE_INSN ("swpb", 0x38208000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("swph", 0x78208000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("swp", 0xb8208000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("swpab", 0x38a08000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("swplb", 0x38608000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("swpalb", 0x38e08000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("swpah", 0x78a08000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("swplh", 0x78608000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("swpalh", 0x78e08000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("swpa", 0xb8a08000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("swpl", 0xb8608000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("swpal", 0xb8e08000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("ldaddb", 0x38200000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldaddh", 0x78200000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldadd", 0xb8200000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("ldaddab", 0x38a00000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldaddlb", 0x38600000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldaddalb", 0x38e00000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldaddah", 0x78a00000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldaddlh", 0x78600000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldaddalh", 0x78e00000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldadda", 0xb8a00000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("ldaddl", 0xb8600000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("ldaddal", 0xb8e00000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("ldclrb", 0x38201000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldclrh", 0x78201000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldclr", 0xb8201000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("ldclrab", 0x38a01000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldclrlb", 0x38601000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldclralb", 0x38e01000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldclrah", 0x78a01000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldclrlh", 0x78601000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldclralh", 0x78e01000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldclra", 0xb8a01000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("ldclrl", 0xb8601000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("ldclral", 0xb8e01000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("ldeorb", 0x38202000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldeorh", 0x78202000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldeor", 0xb8202000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("ldeorab", 0x38a02000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldeorlb", 0x38602000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldeoralb", 0x38e02000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldeorah", 0x78a02000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldeorlh", 0x78602000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldeoralh", 0x78e02000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldeora", 0xb8a02000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("ldeorl", 0xb8602000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("ldeoral", 0xb8e02000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("ldsetb", 0x38203000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldseth", 0x78203000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldset", 0xb8203000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("ldsetab", 0x38a03000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldsetlb", 0x38603000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldsetalb", 0x38e03000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldsetah", 0x78a03000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldsetlh", 0x78603000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldsetalh", 0x78e03000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldseta", 0xb8a03000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("ldsetl", 0xb8603000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("ldsetal", 0xb8e03000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("ldsmaxb", 0x38204000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldsmaxh", 0x78204000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldsmax", 0xb8204000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("ldsmaxab", 0x38a04000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldsmaxlb", 0x38604000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldsmaxalb", 0x38e04000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldsmaxah", 0x78a04000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldsmaxlh", 0x78604000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldsmaxalh", 0x78e04000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldsmaxa", 0xb8a04000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("ldsmaxl", 0xb8604000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("ldsmaxal", 0xb8e04000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("ldsminb", 0x38205000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldsminh", 0x78205000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldsmin", 0xb8205000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("ldsminab", 0x38a05000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldsminlb", 0x38605000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldsminalb", 0x38e05000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldsminah", 0x78a05000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldsminlh", 0x78605000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldsminalh", 0x78e05000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldsmina", 0xb8a05000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("ldsminl", 0xb8605000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("ldsminal", 0xb8e05000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("ldumaxb", 0x38206000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldumaxh", 0x78206000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldumax", 0xb8206000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("ldumaxab", 0x38a06000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldumaxlb", 0x38606000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldumaxalb", 0x38e06000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldumaxah", 0x78a06000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldumaxlh", 0x78606000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldumaxalh", 0x78e06000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldumaxa", 0xb8a06000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("ldumaxl", 0xb8606000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("ldumaxal", 0xb8e06000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("lduminb", 0x38207000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("lduminh", 0x78207000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("ldumin", 0xb8207000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("lduminab", 0x38a07000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("lduminlb", 0x38607000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("lduminalb", 0x38e07000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("lduminah", 0x78a07000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("lduminlh", 0x78607000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, F_HAS_ALIAS),
  _LSE_INSN ("lduminalh", 0x78e07000, 0xffe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_W2_LDST_EXC, 0),
  _LSE_INSN ("ldumina", 0xb8a07000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("lduminl", 0xb8607000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ | F_HAS_ALIAS),
  _LSE_INSN ("lduminal", 0xb8e07000, 0xbfe0fc00, lse_atomic, OP3 (Rs, Rt, ADDR_SIMPLE), QL_R2NIL, F_LSE_SZ),
  _LSE_INSN ("staddb", 0x3820001f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("staddh", 0x7820001f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stadd", 0xb820001f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  _LSE_INSN ("staddlb", 0x3860001f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("staddlh", 0x7860001f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("staddl", 0xb860001f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  _LSE_INSN ("stclrb", 0x3820101f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stclrh", 0x7820101f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stclr", 0xb820101f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  _LSE_INSN ("stclrlb", 0x3860101f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stclrlh", 0x7860101f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stclrl", 0xb860101f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  _LSE_INSN ("steorb", 0x3820201f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("steorh", 0x7820201f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("steor", 0xb820201f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  _LSE_INSN ("steorlb", 0x3860201f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("steorlh", 0x7860201f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("steorl", 0xb860201f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  _LSE_INSN ("stsetb", 0x3820301f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stseth", 0x7820301f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stset", 0xb820301f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  _LSE_INSN ("stsetlb", 0x3860301f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stsetlh", 0x7860301f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stsetl", 0xb860301f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  _LSE_INSN ("stsmaxb", 0x3820401f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stsmaxh", 0x7820401f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stsmax", 0xb820401f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  _LSE_INSN ("stsmaxlb", 0x3860401f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stsmaxlh", 0x7860401f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stsmaxl", 0xb860401f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  _LSE_INSN ("stsminb", 0x3820501f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stsminh", 0x7820501f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stsmin", 0xb820501f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  _LSE_INSN ("stsminlb", 0x3860501f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stsminlh", 0x7860501f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stsminl", 0xb860501f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  _LSE_INSN ("stumaxb", 0x3820601f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stumaxh", 0x7820601f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stumax", 0xb820601f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  _LSE_INSN ("stumaxlb", 0x3860601f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stumaxlh", 0x7860601f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stumaxl", 0xb860601f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  _LSE_INSN ("stuminb", 0x3820701f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stuminh", 0x7820701f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stumin", 0xb820701f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  _LSE_INSN ("stuminlb", 0x3860701f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stuminlh", 0x7860701f, 0xffe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_W1_LDST_EXC, F_ALIAS),
  _LSE_INSN ("stuminl", 0xb860701f, 0xbfe0fc1f, lse_atomic, OP2 (Rs, ADDR_SIMPLE), QL_R1NIL, F_LSE_SZ | F_ALIAS),
  /* Move wide (immediate).  */
  CORE_INSN ("movn", 0x12800000, 0x7f800000, movewide, OP_MOVN, OP2 (Rd, HALF), QL_DST_R, F_SF | F_HAS_ALIAS),
  CORE_INSN ("mov",  0x12800000, 0x7f800000, movewide, OP_MOV_IMM_WIDEN, OP2 (Rd, IMM_MOV), QL_DST_R, F_SF | F_ALIAS | F_CONV),
  CORE_INSN ("movz", 0x52800000, 0x7f800000, movewide, OP_MOVZ, OP2 (Rd, HALF), QL_DST_R, F_SF | F_HAS_ALIAS),
  CORE_INSN ("mov",  0x52800000, 0x7f800000, movewide, OP_MOV_IMM_WIDE, OP2 (Rd, IMM_MOV), QL_DST_R, F_SF | F_ALIAS | F_CONV),
  CORE_INSN ("movk", 0x72800000, 0x7f800000, movewide, OP_MOVK, OP2 (Rd, HALF), QL_DST_R, F_SF),
  /* PC-rel. addressing.  */
  CORE_INSN ("adr",  0x10000000, 0x9f000000, pcreladdr, 0, OP2 (Rd, ADDR_PCREL21), QL_ADRP, 0),
  CORE_INSN ("adrp", 0x90000000, 0x9f000000, pcreladdr, 0, OP2 (Rd, ADDR_ADRP), QL_ADRP, 0),
  /* TME Instructions.  */
  _TME_INSN ("tstart", 0xd5233060, 0xffffffe0, 0, 0, OP1 (Rd), QL_I1X, 0),
  _TME_INSN ("tcommit", 0xd503307f, 0xffffffff, 0, 0, OP0 (), {}, 0),
  _TME_INSN ("ttest", 0xd5233160, 0xffffffe0, 0, 0, OP1 (Rd), QL_I1X, 0),
  _TME_INSN ("tcancel", 0xd4600000, 0xffe0001f, 0, 0, OP1 (TME_UIMM16), QL_IMM_NIL, 0),
  /* SME instructions (aliases for MSR <sysreg> operations.  */
  SME_INSN ("smstart", 0xd503477f, 0xffffffff, sme_start, 0, OP0 (), {}, F_SYS_WRITE, 0),
  SME_INSN ("smstop",  0xd503467f, 0xffffffff, sme_stop,  0, OP0 (), {}, F_SYS_WRITE, 0),
  SME_INSN ("smstart", 0xd503417f, 0xfffff1ff, sme_start, 0, OP1 (SME_SM_ZA), {}, F_SYS_WRITE, 0),
  SME_INSN ("smstop",  0xd503407f, 0xfffff1ff, sme_stop,  0, OP1 (SME_SM_ZA), {}, F_SYS_WRITE, 0),
  /* System.  */
  CORE_INSN ("msr", 0xd500401f, 0xfff8f01f, ic_system, 0, OP2 (PSTATEFIELD, UIMM4), {}, F_SYS_WRITE),
  CORE_INSN ("hint",0xd503201f, 0xfffff01f, ic_system, 0, OP1 (UIMM7), {}, F_HAS_ALIAS),
  CORE_INSN ("nop", 0xd503201f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("csdb",0xd503229f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("bti",0xd503241f, 0xffffff3f, ic_system, 0, OP1 (BTI_TARGET), {}, F_ALIAS | F_OPD0_OPT | F_DEFAULT (0x0)),
  CORE_INSN ("yield", 0xd503203f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("wfe", 0xd503205f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("wfi", 0xd503207f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("sev", 0xd503209f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("sevl",0xd50320bf, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("dgh", 0xd50320df, 0xffffffff, ic_system, 0, OP0 (), {}, 0),
  CORE_INSN ("xpaclri", 0xd50320ff, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("pacia1716", 0xd503211f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("pacib1716", 0xd503215f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("autia1716", 0xd503219f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("autib1716", 0xd50321df, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("esb", 0xd503221f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("psb", 0xd503223f, 0xffffffff, ic_system, 0, OP1 (BARRIER_PSB), {}, F_ALIAS),
  CORE_INSN ("tsb", 0xd503225f, 0xffffffff, ic_system, 0, OP1 (BARRIER_PSB), {}, F_ALIAS),
  CORE_INSN ("clearbhb", 0xd50322df, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("clrex", 0xd503305f, 0xfffff0ff, ic_system, 0, OP1 (UIMM4), {}, F_OPD0_OPT | F_DEFAULT (0xF)),
  CORE_INSN ("dsb", 0xd503309f, 0xfffff0ff, ic_system, 0, OP1 (BARRIER), {}, F_HAS_ALIAS),
  V8_7_INSN ("dsb", 0xd503323f, 0xfffff3ff, ic_system, OP1 (BARRIER_DSB_NXS), {}, F_HAS_ALIAS),
  V8_R_INSN ("dfb", 0xd5033c9f, 0xffffffff, ic_system, OP0 (), {}, F_ALIAS),
  CORE_INSN ("ssbb", 0xd503309f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("pssbb", 0xd503349f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("dmb", 0xd50330bf, 0xfffff0ff, ic_system, 0, OP1 (BARRIER), {}, 0),
  CORE_INSN ("isb", 0xd50330df, 0xfffff0ff, ic_system, 0, OP1 (BARRIER_ISB), {}, F_OPD0_OPT | F_DEFAULT (0xF)),
  SB_INSN ("sb", 0xd50330ff, 0xffffffff, ic_system, OP0 (), {}, 0),
  CORE_INSN ("sys", 0xd5080000, 0xfff80000, ic_system, 0, OP5 (UIMM3_OP1, CRn, CRm, UIMM3_OP2, Rt), QL_SYS, F_HAS_ALIAS | F_OPD4_OPT | F_DEFAULT (0x1F)),
  CORE_INSN ("at",  0xd5080000, 0xfff80000, ic_system, 0, OP2 (SYSREG_AT, Rt), QL_SRC_X, F_ALIAS),
  CORE_INSN ("dc",  0xd5080000, 0xfff80000, ic_system, 0, OP2 (SYSREG_DC, Rt), QL_SRC_X, F_ALIAS),
  CORE_INSN ("ic",  0xd5080000, 0xfff80000, ic_system, 0, OP2 (SYSREG_IC, Rt_SYS), QL_SRC_X, F_ALIAS | F_OPD1_OPT | F_DEFAULT (0x1F)),
  CORE_INSN ("tlbi",0xd5080000, 0xfff80000, ic_system, 0, OP2 (SYSREG_TLBI, Rt_SYS), QL_SRC_X, F_ALIAS | F_OPD1_OPT | F_DEFAULT (0x1F)),
  V8_7_INSN ("wfet", 0xd5031000, 0xffffffe0, ic_system, OP1 (Rd), QL_I1X, F_HAS_ALIAS),
  V8_7_INSN ("wfit", 0xd5031020, 0xffffffe0, ic_system, OP1 (Rd), QL_I1X, F_HAS_ALIAS),
  PREDRES_INSN ("cfp", 0xd50b7380, 0xffffffe0, ic_system, OP2 (SYSREG_SR, Rt), QL_SRC_X, F_ALIAS),
  PREDRES_INSN ("dvp", 0xd50b73a0, 0xffffffe0, ic_system, OP2 (SYSREG_SR, Rt), QL_SRC_X, F_ALIAS),
  PREDRES_INSN ("cpp", 0xd50b73e0, 0xffffffe0, ic_system, OP2 (SYSREG_SR, Rt), QL_SRC_X, F_ALIAS),
  /* Armv8.4-a flag setting instruction, However this encoding has an encoding clash with the msr
     below it.  Usually we can resolve this by setting an alias condition on the flags, however that
     depends on the disassembly masks to be able to quickly find the alias.  The problem is the
     cfinv instruction has no arguments, so all bits are set in the mask.  Which means it will
     potentially alias with too many instructions and so the tree can't be constructed.   As a work
     around we just place cfinv before msr.  This means the order between these two shouldn't be
     changed.  */
  FLAGM_INSN ("cfinv",  0xd500401f, 0xffffffff, ic_system, OP0 (), {}, 0),
  CORE_INSN ("msr", 0xd5000000, 0xffe00000, ic_system, 0, OP2 (SYSREG, Rt), QL_SRC_X, F_SYS_WRITE),
  CORE_INSN ("sysl",0xd5280000, 0xfff80000, ic_system, 0, OP5 (Rt, UIMM3_OP1, CRn, CRm, UIMM3_OP2), QL_SYSL, 0),
  CORE_INSN ("mrs", 0xd5200000, 0xffe00000, ic_system, 0, OP2 (Rt, SYSREG), QL_DST_X, F_SYS_READ),
  CORE_INSN ("paciaz",  0xd503231f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("paciasp", 0xd503233f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("pacibz",  0xd503235f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("pacibsp", 0xd503237f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("autiaz",  0xd503239f, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("autiasp", 0xd50323bf, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("autibz",  0xd50323df, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  CORE_INSN ("autibsp", 0xd50323ff, 0xffffffff, ic_system, 0, OP0 (), {}, F_ALIAS),
  /* Test & branch (immediate).  */
  CORE_INSN ("tbz", 0x36000000, 0x7f000000, testbranch, 0, OP3 (Rt, BIT_NUM, ADDR_PCREL14), QL_PCREL_14, 0),
  CORE_INSN ("tbnz",0x37000000, 0x7f000000, testbranch, 0, OP3 (Rt, BIT_NUM, ADDR_PCREL14), QL_PCREL_14, 0),
  /* The old UAL conditional branch mnemonics (to aid portability).  */
  CORE_INSN ("beq", 0x54000000, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  CORE_INSN ("bne", 0x54000001, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  CORE_INSN ("bcs", 0x54000002, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  CORE_INSN ("bhs", 0x54000002, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  CORE_INSN ("bcc", 0x54000003, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  CORE_INSN ("blo", 0x54000003, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  CORE_INSN ("bmi", 0x54000004, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  CORE_INSN ("bpl", 0x54000005, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  CORE_INSN ("bvs", 0x54000006, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  CORE_INSN ("bvc", 0x54000007, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  CORE_INSN ("bhi", 0x54000008, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  CORE_INSN ("bls", 0x54000009, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  CORE_INSN ("bge", 0x5400000a, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  CORE_INSN ("blt", 0x5400000b, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  CORE_INSN ("bgt", 0x5400000c, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  CORE_INSN ("ble", 0x5400000d, 0xff00001f, condbranch, 0, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_ALIAS | F_PSEUDO),
  /* SVE instructions.  */
  _SVE_INSN ("fmov", 0x2539c000, 0xff3fe000, sve_size_hsd, 0, OP2 (SVE_Zd, SVE_FPIMM8), OP_SVE_VU_HSD, F_ALIAS, 0),
  _SVE_INSNC ("fmov", 0x0510c000, 0xff30e000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg4_16, SVE_FPIMM8), OP_SVE_VMU_HSD, F_ALIAS, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("mov", 0x04603000, 0xffe0fc00, sve_misc, OP_MOV_Z_Z, OP2 (SVE_Zd, SVE_Zn), OP_SVE_DD, F_ALIAS | F_MISC, 0),
  _SVE_INSN ("mov", 0x05202000, 0xff20fc00, sve_index, OP_MOV_Z_V, OP2 (SVE_Zd, SVE_VZn), OP_SVE_VV_BHSDQ, F_ALIAS | F_MISC, 0),
  _SVE_INSN ("mov", 0x05203800, 0xff3ffc00, sve_size_bhsd, 0, OP2 (SVE_Zd, Rn_SP), OP_SVE_VR_BHSD, F_ALIAS, 0),
  _SVE_INSN ("mov", 0x25804000, 0xfff0c210, sve_misc, OP_MOV_P_P, OP2 (SVE_Pd, SVE_Pn), OP_SVE_BB, F_ALIAS | F_MISC, 0),
  _SVE_INSN ("mov", 0x25804000, 0xfff0c210, sve_misc, OP_MOV_PN_PN, OP2 (SVE_PNd, SVE_PNn), OP_SVE_BB, F_ALIAS | F_MISC, 0),
  _SVE_INSN ("mov", 0x05202000, 0xff20fc00, sve_index, OP_MOV_Z_Zi, OP2 (SVE_Zd, SVE_Zn_INDEX), OP_SVE_VV_BHSDQ, F_ALIAS | F_MISC, 0),
  _SVE_INSN ("mov", 0x05c00000, 0xfffc0000, sve_limm, 0, OP2 (SVE_Zd, SVE_LIMM_MOV), OP_SVE_VU_BHSD, F_ALIAS, 0),
  _SVE_INSN ("mov", 0x2538c000, 0xff3fc000, sve_size_bhsd, 0, OP2 (SVE_Zd, SVE_ASIMM), OP_SVE_VU_BHSD, F_ALIAS, 0),
  _SVE_INSNC ("mov", 0x05208000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Vn), OP_SVE_VMV_BHSD, F_ALIAS, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("mov", 0x0520c000, 0xff20c000, sve_size_bhsd, OP_MOV_Z_P_Z, OP3 (SVE_Zd, SVE_Pg4_10, SVE_Zn), OP_SVE_VMV_BHSD, F_ALIAS | F_MISC, 0),
  _SVE_INSNC ("mov", 0x0528a000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Pg3, Rn_SP), OP_SVE_VMR_BHSD, F_ALIAS, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("mov", 0x25004000, 0xfff0c210, sve_misc, OP_MOVZ_P_P_P, OP3 (SVE_Pd, SVE_Pg4_10, SVE_Pn), OP_SVE_BZB, F_ALIAS | F_MISC, 0),
  _SVE_INSN ("mov", 0x25004210, 0xfff0c210, sve_misc, OP_MOVM_P_P_P, OP3 (SVE_Pd, SVE_Pg4_10, SVE_Pn), OP_SVE_BMB, F_ALIAS | F_MISC, 0),
  _SVE_INSNC ("mov", 0x05100000, 0xff308000, sve_cpy, 0, OP3 (SVE_Zd, SVE_Pg4_16, SVE_ASIMM), OP_SVE_VPU_BHSD, F_ALIAS, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("movs", 0x25c04000, 0xfff0c210, sve_misc, OP_MOVS_P_P, OP2 (SVE_Pd, SVE_Pn), OP_SVE_BB, F_ALIAS | F_MISC, 0),
  _SVE_INSN ("movs", 0x25404000, 0xfff0c210, sve_misc, OP_MOVZS_P_P_P, OP3 (SVE_Pd, SVE_Pg4_10, SVE_Pn), OP_SVE_BZB, F_ALIAS | F_MISC, 0),
  _SVE_INSN ("not", 0x25004200, 0xfff0c210, sve_misc, OP_NOT_P_P_P_Z, OP3 (SVE_Pd, SVE_Pg4_10, SVE_Pn), OP_SVE_BZB, F_ALIAS | F_MISC, 0),
  _SVE_INSN ("nots", 0x25404200, 0xfff0c210, sve_misc, OP_NOTS_P_P_P_Z, OP3 (SVE_Pd, SVE_Pg4_10, SVE_Pn), OP_SVE_BZB, F_ALIAS | F_MISC, 0),
  _SVE_INSNC ("abs", 0x0416a000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("add", 0x04200000, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSNC ("add", 0x2520c000, 0xff3fc000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zd, SVE_AIMM), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 1),
  _SVE_INSNC ("add", 0x04000000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("addpl", 0x04605000, 0xffe0f800, sve_misc, 0, OP3 (Rd_SP, SVE_Rn_SP, SVE_SIMM6), OP_SVE_XXU, 0, 0),
  _SVE_INSN ("addvl", 0x04205000, 0xffe0f800, sve_misc, 0, OP3 (Rd_SP, SVE_Rn_SP, SVE_SIMM6), OP_SVE_XXU, 0, 0),
  _SVE_INSN ("adr", 0x0420a000, 0xffe0f000, sve_misc, 0, OP2 (SVE_Zd, SVE_ADDR_ZZ_SXTW), OP_SVE_DD, 0, 0),
  _SVE_INSN ("adr", 0x0460a000, 0xffe0f000, sve_misc, 0, OP2 (SVE_Zd, SVE_ADDR_ZZ_UXTW), OP_SVE_DD, 0, 0),
  _SVE_INSN ("adr", 0x04a0a000, 0xffa0f000, sve_size_sd, 0, OP2 (SVE_Zd, SVE_ADDR_ZZ_LSL), OP_SVE_VV_SD, 0, 0),
  _SVE_INSN ("and", 0x04203000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_DDD, 0, 0),
  _SVE_INSNC ("and", 0x05800000, 0xfffc0000, sve_limm, 0, OP3 (SVE_Zd, SVE_Zd, SVE_LIMM), OP_SVE_VVU_BHSD, F_HAS_ALIAS, C_SCAN_MOVPRFX, 1),
  _SVE_INSNC ("and", 0x041a0000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("and", 0x25004000, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, F_HAS_ALIAS, 0),
  _SVE_INSN ("ands", 0x25404000, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, F_HAS_ALIAS, 0),
  _SVE_INSN ("andv", 0x041a2000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_VUV_BHSD, 0, 0),
  _SVE_INSN ("asr", 0x04208000, 0xff20fc00, sve_size_bhs, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVD_BHS, 0, 0),
  _SVE_INSN ("asr", 0x04209000, 0xff20fc00, sve_shift_unpred, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED), OP_SVE_VVU_BHSD, 0, 0),
  _SVE_INSNC ("asr", 0x04108000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("asr", 0x04188000, 0xff3fe000, sve_size_bhs, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVD_BHS, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("asr", 0x04008000, 0xff3fe000, sve_shift_pred, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_SHRIMM_PRED), OP_SVE_VMVU_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("asrd", 0x04048000, 0xff3fe000, sve_shift_pred, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_SHRIMM_PRED), OP_SVE_VMVU_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("asrr", 0x04148000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("bic", 0x04e03000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_DDD, 0, 0),
  _SVE_INSNC ("bic", 0x041b0000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("bic", 0x25004010, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, 0, 0),
  _SVE_INSN ("bics", 0x25404010, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, 0, 0),
  _SVE_INSN ("brka", 0x25104000, 0xffffc200, sve_pred_zm, 0, OP3 (SVE_Pd, SVE_Pg4_10, SVE_Pn), OP_SVE_BPB, 0, 0),
  _SVE_INSN ("brkas", 0x25504000, 0xffffc210, sve_misc, 0, OP3 (SVE_Pd, SVE_Pg4_10, SVE_Pn), OP_SVE_BZB, 0, 0),
  _SVE_INSN ("brkb", 0x25904000, 0xffffc200, sve_pred_zm, 0, OP3 (SVE_Pd, SVE_Pg4_10, SVE_Pn), OP_SVE_BPB, 0, 0),
  _SVE_INSN ("brkbs", 0x25d04000, 0xffffc210, sve_misc, 0, OP3 (SVE_Pd, SVE_Pg4_10, SVE_Pn), OP_SVE_BZB, 0, 0),
  _SVE_INSN ("brkn", 0x25184000, 0xffffc210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pd), OP_SVE_BZBB, 0, 3),
  _SVE_INSN ("brkns", 0x25584000, 0xffffc210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pd), OP_SVE_BZBB, 0, 3),
  _SVE_INSN ("brkpa", 0x2500c000, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, 0, 0),
  _SVE_INSN ("brkpas", 0x2540c000, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, 0, 0),
  _SVE_INSN ("brkpb", 0x2500c010, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, 0, 0),
  _SVE_INSN ("brkpbs", 0x2540c010, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, 0, 0),
  _SVE_INSNC ("clasta", 0x05288000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VUVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("clasta", 0x052a8000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Vd, SVE_Pg3, SVE_Vd, SVE_Zm_5), OP_SVE_VUVV_BHSD, 0, 2),
  _SVE_INSN ("clasta", 0x0530a000, 0xff3fe000, sve_size_bhsd, 0, OP4 (Rd, SVE_Pg3, Rd, SVE_Zm_5), OP_SVE_RURV_BHSD, 0, 2),
  _SVE_INSNC ("clastb", 0x05298000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VUVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("clastb", 0x052b8000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Vd, SVE_Pg3, SVE_Vd, SVE_Zm_5), OP_SVE_VUVV_BHSD, 0, 2),
  _SVE_INSN ("clastb", 0x0531a000, 0xff3fe000, sve_size_bhsd, 0, OP4 (Rd, SVE_Pg3, Rd, SVE_Zm_5), OP_SVE_RURV_BHSD, 0, 2),
  _SVE_INSNC ("cls", 0x0418a000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("clz", 0x0419a000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("cmpeq", 0x24002000, 0xff20e010, sve_size_bhs, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVD_BHS, 0, 0),
  _SVE_INSN ("cmpeq", 0x2400a000, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_BHSD, 0, 0),
  _SVE_INSN ("cmpeq", 0x25008000, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SIMM5), OP_SVE_VZVU_BHSD, 0, 0),
  _SVE_INSN ("cmpge", 0x24004000, 0xff20e010, sve_size_bhs, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVD_BHS, 0, 0),
  _SVE_INSN ("cmpge", 0x24008000, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_BHSD, F_HAS_ALIAS, 0),
  _SVE_INSN ("cmpge", 0x25000000, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SIMM5), OP_SVE_VZVU_BHSD, 0, 0),
  _SVE_INSN ("cmpgt", 0x24004010, 0xff20e010, sve_size_bhs, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVD_BHS, 0, 0),
  _SVE_INSN ("cmpgt", 0x24008010, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_BHSD, F_HAS_ALIAS, 0),
  _SVE_INSN ("cmpgt", 0x25000010, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SIMM5), OP_SVE_VZVU_BHSD, 0, 0),
  _SVE_INSN ("cmphi", 0x24000010, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_BHSD, F_HAS_ALIAS, 0),
  _SVE_INSN ("cmphi", 0x2400c010, 0xff20e010, sve_size_bhs, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVD_BHS, 0, 0),
  _SVE_INSN ("cmphi", 0x24200010, 0xff202010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_UIMM7), OP_SVE_VZVU_BHSD, 0, 0),
  _SVE_INSN ("cmphs", 0x24000000, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_BHSD, F_HAS_ALIAS, 0),
  _SVE_INSN ("cmphs", 0x2400c000, 0xff20e010, sve_size_bhs, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVD_BHS, 0, 0),
  _SVE_INSN ("cmphs", 0x24200000, 0xff202010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_UIMM7), OP_SVE_VZVU_BHSD, 0, 0),
  _SVE_INSN ("cmple", 0x24006010, 0xff20e010, sve_size_bhs, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVD_BHS, 0, 0),
  _SVE_INSN ("cmple", 0x25002010, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SIMM5), OP_SVE_VZVU_BHSD, 0, 0),
  _SVE_INSN ("cmplo", 0x2400e000, 0xff20e010, sve_size_bhs, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVD_BHS, 0, 0),
  _SVE_INSN ("cmplo", 0x24202000, 0xff202010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_UIMM7), OP_SVE_VZVU_BHSD, 0, 0),
  _SVE_INSN ("cmpls", 0x2400e010, 0xff20e010, sve_size_bhs, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVD_BHS, 0, 0),
  _SVE_INSN ("cmpls", 0x24202010, 0xff202010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_UIMM7), OP_SVE_VZVU_BHSD, 0, 0),
  _SVE_INSN ("cmplt", 0x24006000, 0xff20e010, sve_size_bhs, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVD_BHS, 0, 0),
  _SVE_INSN ("cmplt", 0x25002000, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SIMM5), OP_SVE_VZVU_BHSD, 0, 0),
  _SVE_INSN ("cmpne", 0x24002010, 0xff20e010, sve_size_bhs, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVD_BHS, 0, 0),
  _SVE_INSN ("cmpne", 0x2400a010, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_BHSD, 0, 0),
  _SVE_INSN ("cmpne", 0x25008010, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SIMM5), OP_SVE_VZVU_BHSD, 0, 0),
  _SVE_INSNC ("cnot", 0x041ba000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("cnt", 0x041aa000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("cntb", 0x0420e000, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("cntd", 0x04e0e000, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("cnth", 0x0460e000, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("cntp", 0x25208000, 0xff3fc200, sve_size_bhsd, 0, OP3 (Rd, SVE_Pg4_10, SVE_Pn), OP_SVE_XUV_BHSD, 0, 0),
  _SVE_INSN ("cntw", 0x04a0e000, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("compact", 0x05a18000, 0xffbfe000, sve_size_sd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VUV_SD, 0, 0),
  _SVE_INSNC ("cpy", 0x05208000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Vn), OP_SVE_VMV_BHSD, F_HAS_ALIAS, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("cpy", 0x0528a000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Pg3, Rn_SP), OP_SVE_VMR_BHSD, F_HAS_ALIAS, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("cpy", 0x05100000, 0xff308000, sve_cpy, 0, OP3 (SVE_Zd, SVE_Pg4_16, SVE_ASIMM), OP_SVE_VPU_BHSD, F_HAS_ALIAS, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("ctermeq", 0x25a02000, 0xffa0fc1f, sve_size_sd, 0, OP2 (Rn, Rm), OP_SVE_RR, 0, 0),
  _SVE_INSN ("ctermne", 0x25a02010, 0xffa0fc1f, sve_size_sd, 0, OP2 (Rn, Rm), OP_SVE_RR, 0, 0),
  _SVE_INSN ("decb", 0x0430e400, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSNC ("decd", 0x04f0c400, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_DU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("decd", 0x04f0e400, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSNC ("dech", 0x0470c400, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_HU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("dech", 0x0470e400, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSNC ("decp", 0x252d8000, 0xff3ffe00, sve_size_hsd, 0, OP2 (SVE_Zd, SVE_Pg4_5), OP_SVE_Vv_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("decp", 0x252d8800, 0xff3ffe00, sve_size_bhsd, 0, OP2 (Rd, SVE_Pg4_5), OP_SVE_XV_BHSD, 0, 0),
  _SVE_INSNC ("decw", 0x04b0c400, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_SU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("decw", 0x04b0e400, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("dup", 0x05203800, 0xff3ffc00, sve_size_bhsd, 0, OP2 (SVE_Zd, Rn_SP), OP_SVE_VR_BHSD, F_HAS_ALIAS, 0),
  _SVE_INSN ("dup", 0x05202000, 0xff20fc00, sve_index, 0, OP2 (SVE_Zd, SVE_Zn_INDEX), OP_SVE_VV_BHSDQ, F_HAS_ALIAS, 0),
  _SVE_INSN ("dup", 0x2538c000, 0xff3fc000, sve_size_bhsd, 0, OP2 (SVE_Zd, SVE_ASIMM), OP_SVE_VU_BHSD, F_HAS_ALIAS, 0),
  _SVE_INSN ("dupm", 0x05c00000, 0xfffc0000, sve_limm, 0, OP2 (SVE_Zd, SVE_LIMM), OP_SVE_VU_BHSD, F_HAS_ALIAS, 0),
  _SVE_INSN ("eor", 0x04a03000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_DDD, 0, 0),
  _SVE_INSNC ("eor", 0x05400000, 0xfffc0000, sve_limm, 0, OP3 (SVE_Zd, SVE_Zd, SVE_LIMM), OP_SVE_VVU_BHSD, F_HAS_ALIAS, C_SCAN_MOVPRFX, 1),
  _SVE_INSNC ("eor", 0x04190000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("eor", 0x25004200, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, F_HAS_ALIAS, 0),
  _SVE_INSN ("eors", 0x25404200, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, F_HAS_ALIAS, 0),
  _SVE_INSN ("eorv", 0x04192000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_VUV_BHSD, 0, 0),
  _SVE_INSNC ("ext", 0x05200000, 0xffe0e000, sve_misc, 0, OP4 (SVE_Zd, SVE_Zd, SVE_Zm_5, SVE_UIMM8_53), OP_SVE_BBBU, 0, C_SCAN_MOVPRFX, 1),
  _SVE_INSNC ("fabd", 0x65088000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fabs", 0x041ca000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("facge", 0x6500c010, 0xff20e010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_HSD, F_HAS_ALIAS, 0),
  _SVE_INSN ("facgt", 0x6500e010, 0xff20e010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_HSD, F_HAS_ALIAS, 0),
  _SVE_INSN ("fadd", 0x65000000, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD, 0, 0),
  _SVE_INSNC ("fadd", 0x65008000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fadd", 0x65188000, 0xff3fe3c0, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_I1_HALF_ONE), OP_SVE_VMVU_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("fadda", 0x65182000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Vd, SVE_Pg3, SVE_Vd, SVE_Zm_5), OP_SVE_VUVV_HSD, 0, 2),
  _SVE_INSN ("faddv", 0x65002000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_VUV_HSD, 0, 0),
  _SVE_INSNC ("fcadd", 0x64008000, 0xff3ee000, sve_size_hsd, 0, OP5 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5, SVE_IMM_ROT1), OP_SVE_VMVVU_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fcmla", 0x64000000, 0xff208000, sve_size_hsd, 0, OP5 (SVE_Zd, SVE_Pg3, SVE_Zn, SVE_Zm_16, IMM_ROT2), OP_SVE_VMVVU_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fcmla", 0x64a01000, 0xffe0f000, sve_misc, 0, OP4 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX, SVE_IMM_ROT2), OP_SVE_VVVU_H, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fcmla", 0x64e01000, 0xffe0f000, sve_misc, 0, OP4 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX, SVE_IMM_ROT2), OP_SVE_VVVU_S, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("fcmeq", 0x65122000, 0xff3fe010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, FPIMM0), OP_SVE_VZV_HSD, 0, 0),
  _SVE_INSN ("fcmeq", 0x65006000, 0xff20e010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_HSD, 0, 0),
  _SVE_INSN ("fcmge", 0x65102000, 0xff3fe010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, FPIMM0), OP_SVE_VZV_HSD, 0, 0),
  _SVE_INSN ("fcmge", 0x65004000, 0xff20e010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_HSD, F_HAS_ALIAS, 0),
  _SVE_INSN ("fcmgt", 0x65102010, 0xff3fe010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, FPIMM0), OP_SVE_VZV_HSD, 0, 0),
  _SVE_INSN ("fcmgt", 0x65004010, 0xff20e010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_HSD, F_HAS_ALIAS, 0),
  _SVE_INSN ("fcmle", 0x65112010, 0xff3fe010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, FPIMM0), OP_SVE_VZV_HSD, 0, 0),
  _SVE_INSN ("fcmlt", 0x65112000, 0xff3fe010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, FPIMM0), OP_SVE_VZV_HSD, 0, 0),
  _SVE_INSN ("fcmne", 0x65132000, 0xff3fe010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, FPIMM0), OP_SVE_VZV_HSD, 0, 0),
  _SVE_INSN ("fcmne", 0x65006010, 0xff20e010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_HSD, 0, 0),
  _SVE_INSN ("fcmuo", 0x6500c000, 0xff20e010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_HSD, 0, 0),
  _SVE_INSNC ("fcpy", 0x0510c000, 0xff30e000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg4_16, SVE_FPIMM8), OP_SVE_VMU_HSD, F_HAS_ALIAS, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fcvt", 0x6588a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_HMS, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvt", 0x6589a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMH, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvt", 0x65c8a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_HMD, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvt", 0x65c9a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMH, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvt", 0x65caa000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMD, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvt", 0x65cba000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMS, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvtzs", 0x655aa000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_HMH, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvtzs", 0x655ca000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMH, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvtzs", 0x655ea000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMH, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvtzs", 0x659ca000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMS, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvtzs", 0x65d8a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMD, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvtzs", 0x65dca000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMS, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvtzs", 0x65dea000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMD, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvtzu", 0x655ba000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_HMH, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvtzu", 0x655da000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMH, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvtzu", 0x655fa000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMH, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvtzu", 0x659da000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMS, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvtzu", 0x65d9a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMD, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvtzu", 0x65dda000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMS, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fcvtzu", 0x65dfa000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMD, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("fdiv", 0x650d8000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fdivr", 0x650c8000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("fdup", 0x2539c000, 0xff3fe000, sve_size_hsd, 0, OP2 (SVE_Zd, SVE_FPIMM8), OP_SVE_VU_HSD, F_HAS_ALIAS, 0),
  _SVE_INSN ("fexpa", 0x0420b800, 0xff3ffc00, sve_size_hsd, 0, OP2 (SVE_Zd, SVE_Zn), OP_SVE_VV_HSD, 0, 0),
  _SVE_INSNC ("fmad", 0x65208000, 0xff20e000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zm_5, SVE_Za_16), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fmax", 0x65068000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fmax", 0x651e8000, 0xff3fe3c0, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_I1_ZERO_ONE), OP_SVE_VMVU_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fmaxnm", 0x65048000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fmaxnm", 0x651c8000, 0xff3fe3c0, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_I1_ZERO_ONE), OP_SVE_VMVU_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("fmaxnmv", 0x65042000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_VUV_HSD, 0, 0),
  _SVE_INSN ("fmaxv", 0x65062000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_VUV_HSD, 0, 0),
  _SVE_INSNC ("fmin", 0x65078000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fmin", 0x651f8000, 0xff3fe3c0, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_I1_ZERO_ONE), OP_SVE_VMVU_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fminnm", 0x65058000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fminnm", 0x651d8000, 0xff3fe3c0, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_I1_ZERO_ONE), OP_SVE_VMVU_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("fminnmv", 0x65052000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_VUV_HSD, 0, 0),
  _SVE_INSN ("fminv", 0x65072000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_VUV_HSD, 0, 0),
  _SVE_INSNC ("fmla", 0x65200000, 0xff20e000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fmla", 0x64200000, 0xffa0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_22_INDEX), OP_SVE_VVV_H, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fmla", 0x64a00000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX), OP_SVE_VVV_S, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fmla", 0x64e00000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX), OP_SVE_VVV_D, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fmls", 0x65202000, 0xff20e000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fmls", 0x64200400, 0xffa0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_22_INDEX), OP_SVE_VVV_H, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fmls", 0x64a00400, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX), OP_SVE_VVV_S, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fmls", 0x64e00400, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX), OP_SVE_VVV_D, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fmsb", 0x6520a000, 0xff20e000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zm_5, SVE_Za_16), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("fmul", 0x65000800, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD, 0, 0),
  _SVE_INSNC ("fmul", 0x65028000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fmul", 0x651a8000, 0xff3fe3c0, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_I1_HALF_TWO), OP_SVE_VMVU_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("fmul", 0x64202000, 0xffa0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_22_INDEX), OP_SVE_VVV_H, 0, 0),
  _SVE_INSN ("fmul", 0x64a02000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX), OP_SVE_VVV_S, 0, 0),
  _SVE_INSN ("fmul", 0x64e02000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX), OP_SVE_VVV_D, 0, 0),
  _SVE_INSNC ("fmulx", 0x650a8000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fneg", 0x041da000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fnmad", 0x6520c000, 0xff20e000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zm_5, SVE_Za_16), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fnmla", 0x65204000, 0xff20e000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fnmls", 0x65206000, 0xff20e000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("fnmsb", 0x6520e000, 0xff20e000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zm_5, SVE_Za_16), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("frecpe", 0x650e3000, 0xff3ffc00, sve_size_hsd, 0, OP2 (SVE_Zd, SVE_Zn), OP_SVE_VV_HSD, 0, 0),
  _SVE_INSN ("frecps", 0x65001800, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD, 0, 0),
  _SVE_INSNC ("frecpx", 0x650ca000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("frinta", 0x6504a000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("frinti", 0x6507a000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("frintm", 0x6502a000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("frintn", 0x6500a000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("frintp", 0x6501a000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("frintx", 0x6506a000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("frintz", 0x6503a000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("frsqrte", 0x650f3000, 0xff3ffc00, sve_size_hsd, 0, OP2 (SVE_Zd, SVE_Zn), OP_SVE_VV_HSD, 0, 0),
  _SVE_INSN ("frsqrts", 0x65001c00, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD, 0, 0),
  _SVE_INSNC ("fscale", 0x65098000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fsqrt", 0x650da000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("fsub", 0x65000400, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD, 0, 0),
  _SVE_INSNC ("fsub", 0x65018000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fsub", 0x65198000, 0xff3fe3c0, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_I1_HALF_ONE), OP_SVE_VMVU_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fsubr", 0x65038000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("fsubr", 0x651b8000, 0xff3fe3c0, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_I1_HALF_ONE), OP_SVE_VMVU_HSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("ftmad", 0x65108000, 0xff38fc00, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Zd, SVE_Zm_5, SVE_UIMM3), OP_SVE_VVVU_HSD, 0, C_SCAN_MOVPRFX, 1),
  _SVE_INSN ("ftsmul", 0x65000c00, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD, 0, 0),
  _SVE_INSN ("ftssel", 0x0420b000, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD, 0, 0),
  _SVE_INSN ("incb", 0x0430e000, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSNC ("incd", 0x04f0c000, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_DU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("incd", 0x04f0e000, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSNC ("inch", 0x0470c000, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_HU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("inch", 0x0470e000, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSNC ("incp", 0x252c8000, 0xff3ffe00, sve_size_hsd, 0, OP2 (SVE_Zd, SVE_Pg4_5), OP_SVE_Vv_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("incp", 0x252c8800, 0xff3ffe00, sve_size_bhsd, 0, OP2 (Rd, SVE_Pg4_5), OP_SVE_XV_BHSD, 0, 0),
  _SVE_INSNC ("incw", 0x04b0c000, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_SU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("incw", 0x04b0e000, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("index", 0x04204c00, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, Rn, Rm), OP_SVE_VRR_BHSD, 0, 0),
  _SVE_INSN ("index", 0x04204000, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_SIMM5, SVE_SIMM5B), OP_SVE_VUU_BHSD, 0, 0),
  _SVE_INSN ("index", 0x04204400, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, Rn, SIMM5), OP_SVE_VRU_BHSD, 0, 0),
  _SVE_INSN ("index", 0x04204800, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_SIMM5, Rm), OP_SVE_VUR_BHSD, 0, 0),
  _SVE_INSNC ("insr", 0x05243800, 0xff3ffc00, sve_size_bhsd, 0, OP2 (SVE_Zd, SVE_Rm), OP_SVE_VR_BHSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("insr", 0x05343800, 0xff3ffc00, sve_size_bhsd, 0, OP2 (SVE_Zd, SVE_Vm), OP_SVE_VV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("lasta", 0x0520a000, 0xff3fe000, sve_size_bhsd, 0, OP3 (Rd, SVE_Pg3, SVE_Zn), OP_SVE_RUV_BHSD, 0, 0),
  _SVE_INSN ("lasta", 0x05228000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_VUV_BHSD, 0, 0),
  _SVE_INSN ("lastb", 0x0521a000, 0xff3fe000, sve_size_bhsd, 0, OP3 (Rd, SVE_Pg3, SVE_Zn), OP_SVE_RUV_BHSD, 0, 0),
  _SVE_INSN ("lastb", 0x05238000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_VUV_BHSD, 0, 0),
  _SVE_INSN ("ld1b", 0x84004000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ld1b", 0xa4004000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_BZU, F_OD(1), 0),
  _SVE_INSN ("ld1b", 0xa4204000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ld1b", 0xa4404000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1b", 0xa4604000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1b", 0xc4004000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1b", 0xc440c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1b", 0x8420c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ld1b", 0xa400a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_BZU, F_OD(1), 0),
  _SVE_INSN ("ld1b", 0xa420a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ld1b", 0xa440a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1b", 0xa460a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1b", 0xc420c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1d", 0xa5e04000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL3), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1d", 0xc5804000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1d", 0xc5a04000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW3_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1d", 0xc5c0c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1d", 0xc5e0c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_LSL3), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1d", 0xa5e0a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1d", 0xc5a0c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x8), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1h", 0x84804000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ld1h", 0x84a04000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW1_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ld1h", 0xa4a04000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ld1h", 0xa4c04000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1h", 0xa4e04000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1h", 0xc4804000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1h", 0xc4a04000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW1_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1h", 0xc4c0c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1h", 0xc4e0c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_LSL1), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1h", 0x84a0c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x2), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ld1h", 0xa4a0a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ld1h", 0xa4c0a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1h", 0xa4e0a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1h", 0xc4a0c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x2), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1rb", 0x84408000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6), OP_SVE_BZU, F_OD(1), 0),
  _SVE_INSN ("ld1rb", 0x8440a000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ld1rb", 0x8440c000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1rb", 0x8440e000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1rd", 0x85c0e000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6x8), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1rh", 0x84c0a000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6x2), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ld1rh", 0x84c0c000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6x2), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1rh", 0x84c0e000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6x2), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1rqb", 0xa4002000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x16), OP_SVE_BZU, F_OD(1), 0),
  _SVE_INSN ("ld1rqb", 0xa4000000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_BZU, F_OD(1), 0),
  _SVE_INSN ("ld1rqd", 0xa5802000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x16), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1rqd", 0xa5800000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL3), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1rqh", 0xa4802000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x16), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ld1rqh", 0xa4800000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ld1rqw", 0xa5002000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x16), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1rqw", 0xa5000000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL2), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1rsb", 0x85c08000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1rsb", 0x85c0a000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1rsb", 0x85c0c000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ld1rsh", 0x85408000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6x2), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1rsh", 0x8540a000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6x2), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1rsw", 0x84c08000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6x4), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1rw", 0x8540c000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6x4), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1rw", 0x8540e000, 0xffc0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_U6x4), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1sb", 0x84000000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ld1sb", 0xa5804000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1sb", 0xa5a04000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1sb", 0xa5c04000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ld1sb", 0xc4000000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1sb", 0xc4408000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1sb", 0x84208000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ld1sb", 0xa580a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1sb", 0xa5a0a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1sb", 0xa5c0a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ld1sb", 0xc4208000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1sh", 0x84800000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ld1sh", 0x84a00000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW1_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ld1sh", 0xa5004000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1sh", 0xa5204000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1sh", 0xc4800000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1sh", 0xc4a00000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW1_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1sh", 0xc4c08000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1sh", 0xc4e08000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_LSL1), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1sh", 0x84a08000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x2), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ld1sh", 0xa500a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1sh", 0xa520a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1sh", 0xc4a08000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x2), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1sw", 0xa4804000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL2), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1sw", 0xc5000000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1sw", 0xc5200000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW2_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1sw", 0xc5408000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1sw", 0xc5608000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_LSL2), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1sw", 0xa480a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1sw", 0xc5208000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x4), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1w", 0x85004000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ld1w", 0x85204000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW2_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ld1w", 0xa5404000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL2), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1w", 0xa5604000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL2), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1w", 0xc5004000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1w", 0xc5204000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW2_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1w", 0xc540c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1w", 0xc560c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_LSL2), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld1w", 0x8520c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x4), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ld1w", 0xa540a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ld1w", 0xa560a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ld1w", 0xc520c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x4), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ld2b", 0xa420c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_BZU, F_OD(2), 0),
  _SVE_INSN ("ld2b", 0xa420e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_BZU, F_OD(2), 0),
  _SVE_INSN ("ld2d", 0xa5a0c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL3), OP_SVE_DZU, F_OD(2), 0),
  _SVE_INSN ("ld2d", 0xa5a0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_DZU, F_OD(2), 0),
  _SVE_INSN ("ld2h", 0xa4a0c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_HZU, F_OD(2), 0),
  _SVE_INSN ("ld2h", 0xa4a0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_HZU, F_OD(2), 0),
  _SVE_INSN ("ld2w", 0xa520c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL2), OP_SVE_SZU, F_OD(2), 0),
  _SVE_INSN ("ld2w", 0xa520e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_SZU, F_OD(2), 0),
  _SVE_INSN ("ld3b", 0xa440c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_BZU, F_OD(3), 0),
  _SVE_INSN ("ld3b", 0xa440e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x3xVL), OP_SVE_BZU, F_OD(3), 0),
  _SVE_INSN ("ld3d", 0xa5c0c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL3), OP_SVE_DZU, F_OD(3), 0),
  _SVE_INSN ("ld3d", 0xa5c0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x3xVL), OP_SVE_DZU, F_OD(3), 0),
  _SVE_INSN ("ld3h", 0xa4c0c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_HZU, F_OD(3), 0),
  _SVE_INSN ("ld3h", 0xa4c0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x3xVL), OP_SVE_HZU, F_OD(3), 0),
  _SVE_INSN ("ld3w", 0xa540c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL2), OP_SVE_SZU, F_OD(3), 0),
  _SVE_INSN ("ld3w", 0xa540e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x3xVL), OP_SVE_SZU, F_OD(3), 0),
  _SVE_INSN ("ld4b", 0xa460c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_BZU, F_OD(4), 0),
  _SVE_INSN ("ld4b", 0xa460e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_BZU, F_OD(4), 0),
  _SVE_INSN ("ld4d", 0xa5e0c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL3), OP_SVE_DZU, F_OD(4), 0),
  _SVE_INSN ("ld4d", 0xa5e0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_DZU, F_OD(4), 0),
  _SVE_INSN ("ld4h", 0xa4e0c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_HZU, F_OD(4), 0),
  _SVE_INSN ("ld4h", 0xa4e0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_HZU, F_OD(4), 0),
  _SVE_INSN ("ld4w", 0xa560c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL2), OP_SVE_SZU, F_OD(4), 0),
  _SVE_INSN ("ld4w", 0xa560e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_SZU, F_OD(4), 0),

  _SVE_INSN ("ldff1b", 0x84006000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ldff1b", 0xa4006000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR), OP_SVE_BZU, F_OD(1), 0),
  _SVE_INSN ("ldff1b", 0xa4006000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_BZU, F_OD(1), 0),
  _SVE_INSN ("ldff1b", 0xa4206000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ldff1b", 0xa4206000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ldff1b", 0xa4406000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldff1b", 0xa4406000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldff1b", 0xa4606000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldff1b", 0xa4606000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldff1b", 0xc4006000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1b", 0xc440e000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1b", 0x8420e000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ldff1b", 0xc420e000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5), OP_SVE_DZD, F_OD(1), 0),

  _SVE_INSN ("ldff1d", 0xa5e06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR_LSL3), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldff1d", 0xa5e06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldff1d", 0xc5806000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1d", 0xc5a06000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW3_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1d", 0xc5c0e000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1d", 0xc5e0e000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_LSL3), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1d", 0xc5a0e000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x8), OP_SVE_DZD, F_OD(1), 0),

  _SVE_INSN ("ldff1h", 0x84806000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ldff1h", 0x84a06000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW1_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ldff1h", 0xa4a06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR_LSL1), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ldff1h", 0xa4a06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ldff1h", 0xa4c06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR_LSL1), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldff1h", 0xa4c06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldff1h", 0xa4e06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR_LSL1), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldff1h", 0xa4e06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldff1h", 0xc4806000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1h", 0xc4a06000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW1_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1h", 0xc4c0e000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1h", 0xc4e0e000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_LSL1), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1h", 0x84a0e000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x2), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ldff1h", 0xc4a0e000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x2), OP_SVE_DZD, F_OD(1), 0),

  _SVE_INSN ("ldff1sb", 0x84002000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ldff1sb", 0xa5806000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldff1sb", 0xa5806000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldff1sb", 0xa5a06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldff1sb", 0xa5a06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldff1sb", 0xa5c06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ldff1sb", 0xa5c06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ldff1sb", 0xc4002000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1sb", 0xc440a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1sb", 0x8420a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ldff1sb", 0xc420a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5), OP_SVE_DZD, F_OD(1), 0),

  _SVE_INSN ("ldff1sh", 0x84802000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ldff1sh", 0x84a02000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW1_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ldff1sh", 0xa5006000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR_LSL1), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldff1sh", 0xa5006000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldff1sh", 0xa5206000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR_LSL1), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldff1sh", 0xa5206000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldff1sh", 0xc4802000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1sh", 0xc4a02000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW1_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1sh", 0xc4c0a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1sh", 0xc4e0a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_LSL1), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1sh", 0x84a0a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x2), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ldff1sh", 0xc4a0a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x2), OP_SVE_DZD, F_OD(1), 0),

  _SVE_INSN ("ldff1sw", 0xa4806000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR_LSL2), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldff1sw", 0xa4806000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldff1sw", 0xc5002000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1sw", 0xc5202000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW2_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1sw", 0xc540a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1sw", 0xc560a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_LSL2), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1sw", 0xc520a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x4), OP_SVE_DZD, F_OD(1), 0),
  
  _SVE_INSN ("ldff1w", 0x85006000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ldff1w", 0x85206000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW2_22), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ldff1w", 0xa5406000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR_LSL2), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldff1w", 0xa5406000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldff1w", 0xa5606000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RR_LSL2), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldff1w", 0xa5606000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_R), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldff1w", 0xc5006000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1w", 0xc5206000, 0xffa0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW2_22), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1w", 0xc540e000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1w", 0xc560e000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_LSL2), OP_SVE_DZD, F_OD(1), 0),
  _SVE_INSN ("ldff1w", 0x8520e000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x4), OP_SVE_SZS, F_OD(1), 0),
  _SVE_INSN ("ldff1w", 0xc520e000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x4), OP_SVE_DZD, F_OD(1), 0),

  _SVE_INSN ("ldnf1b", 0xa410a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_BZU, F_OD(1), 0),
  _SVE_INSN ("ldnf1b", 0xa430a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ldnf1b", 0xa450a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldnf1b", 0xa470a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldnf1d", 0xa5f0a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldnf1h", 0xa4b0a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ldnf1h", 0xa4d0a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldnf1h", 0xa4f0a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldnf1sb", 0xa590a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldnf1sb", 0xa5b0a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldnf1sb", 0xa5d0a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ldnf1sh", 0xa510a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldnf1sh", 0xa530a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldnf1sw", 0xa490a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldnf1w", 0xa550a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldnf1w", 0xa570a000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldnt1b", 0xa400c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_BZU, F_OD(1), 0),
  _SVE_INSN ("ldnt1b", 0xa400e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_BZU, F_OD(1), 0),
  _SVE_INSN ("ldnt1d", 0xa580c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL3), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldnt1d", 0xa580e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DZU, F_OD(1), 0),
  _SVE_INSN ("ldnt1h", 0xa480c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ldnt1h", 0xa480e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_HZU, F_OD(1), 0),
  _SVE_INSN ("ldnt1w", 0xa500c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL2), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldnt1w", 0xa500e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_SZU, F_OD(1), 0),
  _SVE_INSN ("ldr", 0x85800000, 0xffc0e010, sve_misc, 0, OP2 (SVE_Pt, SVE_ADDR_RI_S9xVL), {}, 0, 0),
  _SVE_INSN ("ldr", 0x85800000, 0xffc0e010, sve_misc, 0, OP2 (SVE_PNt, SVE_ADDR_RI_S9xVL), {}, 0, 0),
  _SVE_INSN ("ldr", 0x85804000, 0xffc0e000, sve_misc, 0, OP2 (SVE_Zt, SVE_ADDR_RI_S9xVL), {}, 0, 0),
  _SVE_INSN ("lsl", 0x04208c00, 0xff20fc00, sve_size_bhs, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVD_BHS, 0, 0),
  _SVE_INSN ("lsl", 0x04209c00, 0xff20fc00, sve_shift_unpred, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHLIMM_UNPRED), OP_SVE_VVU_BHSD, 0, 0),
  _SVE_INSNC ("lsl", 0x04138000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("lsl", 0x041b8000, 0xff3fe000, sve_size_bhs, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVD_BHS, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("lsl", 0x04038000, 0xff3fe000, sve_shift_pred, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_SHLIMM_PRED), OP_SVE_VMVU_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("lslr", 0x04178000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0,  C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("lsr", 0x04208400, 0xff20fc00, sve_size_bhs, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVD_BHS, 0, 0),
  _SVE_INSN ("lsr", 0x04209400, 0xff20fc00, sve_shift_unpred, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED), OP_SVE_VVU_BHSD, 0, 0),
  _SVE_INSNC ("lsr", 0x04118000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("lsr", 0x04198000, 0xff3fe000, sve_size_bhs, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVD_BHS, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("lsr", 0x04018000, 0xff3fe000, sve_shift_pred, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_SHRIMM_PRED), OP_SVE_VMVU_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("lsrr", 0x04158000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("mad", 0x0400c000, 0xff20e000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zm_16, SVE_Za_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("mla", 0x04004000, 0xff20e000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("mls", 0x04006000, 0xff20e000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("movprfx", 0x0420bc00, 0xfffffc00, sve_misc, 0, OP2 (SVE_Zd, SVE_Zn), {}, F_SCAN, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("movprfx", 0x04102000, 0xff3ee000, sve_movprfx, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VPV_BHSD, F_SCAN, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("msb", 0x0400e000, 0xff20e000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zm_16, SVE_Za_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("mul", 0x2530c000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zd, SVE_SIMM8), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 1),
  _SVE_INSNC ("mul", 0x04100000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("nand", 0x25804210, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, 0, 0),
  _SVE_INSN ("nands", 0x25c04210, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, 0, 0),
  _SVE_INSNC ("neg", 0x0417a000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("nor", 0x25804200, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, 0, 0),
  _SVE_INSN ("nors", 0x25c04200, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, 0, 0),
  _SVE_INSNC ("not", 0x041ea000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("orn", 0x25804010, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, 0, 0),
  _SVE_INSN ("orns", 0x25c04010, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, 0, 0),
  _SVE_INSN ("orr", 0x04603000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_DDD, F_HAS_ALIAS, 0),
  _SVE_INSNC ("orr", 0x05000000, 0xfffc0000, sve_limm, 0, OP3 (SVE_Zd, SVE_Zd, SVE_LIMM), OP_SVE_VVU_BHSD, F_HAS_ALIAS, C_SCAN_MOVPRFX, 1),
  _SVE_INSNC ("orr", 0x04180000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("orr", 0x25804000, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, F_HAS_ALIAS, 0),
  _SVE_INSN ("orrs", 0x25c04000, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BZBB, F_HAS_ALIAS, 0),
  _SVE_INSN ("orv", 0x04182000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_VUV_BHSD, 0, 0),
  _SVE_INSN ("pfalse", 0x2518e400, 0xfffffff0, sve_misc, 0, OP1 (SVE_Pd), OP_SVE_B, 0, 0),
  _SVE_INSN ("pfalse", 0x2518e400, 0xfffffff0, sve_misc, 0, OP1 (SVE_PNd), OP_SVE_B, 0, 0),
  _SVE_INSN ("pfirst", 0x2558c000, 0xfffffe10, sve_misc, 0, OP3 (SVE_Pd, SVE_Pg4_5, SVE_Pd), OP_SVE_BUB, 0, 2),
  _SVE_INSN ("pnext", 0x2519c400, 0xff3ffe10, sve_size_bhsd, 0, OP3 (SVE_Pd, SVE_Pg4_5, SVE_Pd), OP_SVE_VUV_BHSD, 0, 2),
  _SVE_INSN ("prfb", 0x8400c000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RX), {}, 0, 0),
  _SVE_INSN ("prfb", 0x84200000, 0xffa0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_UUS, 0, 0),
  _SVE_INSN ("prfb", 0xc4200000, 0xffa0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RZ_XTW_22), OP_SVE_UUD, 0, 0),
  _SVE_INSN ("prfb", 0xc4608000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_UUD, 0, 0),
  _SVE_INSN ("prfb", 0x8400e000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_ZI_U5), OP_SVE_UUS, 0, 0),
  _SVE_INSN ("prfb", 0x85c00000, 0xffc0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RI_S6xVL), {}, 0, 0),
  _SVE_INSN ("prfb", 0xc400e000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_ZI_U5), OP_SVE_UUD, 0, 0),
  _SVE_INSN ("prfd", 0x84206000, 0xffa0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RZ_XTW3_22), OP_SVE_UUS, 0, 0),
  _SVE_INSN ("prfd", 0x8580c000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RX_LSL3), {}, 0, 0),
  _SVE_INSN ("prfd", 0xc4206000, 0xffa0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RZ_XTW3_22), OP_SVE_UUD, 0, 0),
  _SVE_INSN ("prfd", 0xc460e000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RZ_LSL3), OP_SVE_UUD, 0, 0),
  _SVE_INSN ("prfd", 0x8580e000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_ZI_U5x8), OP_SVE_UUS, 0, 0),
  _SVE_INSN ("prfd", 0x85c06000, 0xffc0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RI_S6xVL), {}, 0, 0),
  _SVE_INSN ("prfd", 0xc580e000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_ZI_U5x8), OP_SVE_UUD, 0, 0),
  _SVE_INSN ("prfh", 0x84202000, 0xffa0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RZ_XTW1_22), OP_SVE_UUS, 0, 0),
  _SVE_INSN ("prfh", 0x8480c000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RX_LSL1), {}, 0, 0),
  _SVE_INSN ("prfh", 0xc4202000, 0xffa0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RZ_XTW1_22), OP_SVE_UUD, 0, 0),
  _SVE_INSN ("prfh", 0xc460a000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RZ_LSL1), OP_SVE_UUD, 0, 0),
  _SVE_INSN ("prfh", 0x8480e000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_ZI_U5x2), OP_SVE_UUS, 0, 0),
  _SVE_INSN ("prfh", 0x85c02000, 0xffc0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RI_S6xVL), {}, 0, 0),
  _SVE_INSN ("prfh", 0xc480e000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_ZI_U5x2), OP_SVE_UUD, 0, 0),
  _SVE_INSN ("prfw", 0x84204000, 0xffa0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RZ_XTW2_22), OP_SVE_UUS, 0, 0),
  _SVE_INSN ("prfw", 0x8500c000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RX_LSL2), {}, 0, 0),
  _SVE_INSN ("prfw", 0xc4204000, 0xffa0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RZ_XTW2_22), OP_SVE_UUD, 0, 0),
  _SVE_INSN ("prfw", 0xc460c000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RZ_LSL2), OP_SVE_UUD, 0, 0),
  _SVE_INSN ("prfw", 0x8500e000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_ZI_U5x4), OP_SVE_UUS, 0, 0),
  _SVE_INSN ("prfw", 0x85c04000, 0xffc0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_RI_S6xVL), {}, 0, 0),
  _SVE_INSN ("prfw", 0xc500e000, 0xffe0e010, sve_misc, 0, OP3 (SVE_PRFOP, SVE_Pg3, SVE_ADDR_ZI_U5x4), OP_SVE_UUD, 0, 0),
  _SVE_INSN ("ptest", 0x2550c000, 0xffffc21f, sve_misc, 0, OP2 (SVE_Pg4_10, SVE_Pn), OP_SVE_UB, 0, 0),
  _SVE_INSN ("ptrue", 0x2518e000, 0xff3ffc10, sve_size_bhsd, 0, OP2 (SVE_Pd, SVE_PATTERN), OP_SVE_VU_BHSD, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("ptrues", 0x2519e000, 0xff3ffc10, sve_size_bhsd, 0, OP2 (SVE_Pd, SVE_PATTERN), OP_SVE_VU_BHSD, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("punpkhi", 0x05314000, 0xfffffe10, sve_misc, 0, OP2 (SVE_Pd, SVE_Pn), OP_SVE_HB, 0, 0),
  _SVE_INSN ("punpklo", 0x05304000, 0xfffffe10, sve_misc, 0, OP2 (SVE_Pd, SVE_Pn), OP_SVE_HB, 0, 0),
  _SVE_INSNC ("rbit", 0x05278000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("rdffr", 0x2519f000, 0xfffffff0, sve_misc, 0, OP1 (SVE_Pd), OP_SVE_B, 0, 0),
  _SVE_INSN ("rdffr", 0x2518f000, 0xfffffe10, sve_misc, 0, OP2 (SVE_Pd, SVE_Pg4_5), OP_SVE_BZ, 0, 0),
  _SVE_INSN ("rdffrs", 0x2558f000, 0xfffffe10, sve_misc, 0, OP2 (SVE_Pd, SVE_Pg4_5), OP_SVE_BZ, 0, 0),
  _SVE_INSN ("rdvl", 0x04bf5000, 0xfffff800, sve_misc, 0, OP2 (Rd, SVE_SIMM6), OP_SVE_XU, 0, 0),
  _SVE_INSN ("rev", 0x05344000, 0xff3ffe10, sve_size_bhsd, 0, OP2 (SVE_Pd, SVE_Pn), OP_SVE_VV_BHSD, 0, 0),
  _SVE_INSN ("rev", 0x05383800, 0xff3ffc00, sve_size_bhsd, 0, OP2 (SVE_Zd, SVE_Zn), OP_SVE_VV_BHSD, 0, 0),
  _SVE_INSNC ("revb", 0x05248000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("revh", 0x05a58000, 0xffbfe000, sve_size_sd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_SD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("revw", 0x05e68000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("sabd", 0x040c0000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("saddv", 0x04002000, 0xff3fe000, sve_size_bhs, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_DUV_BHS, 0, 0),
  _SVE_INSNC ("scvtf", 0x6552a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_HMH, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("scvtf", 0x6554a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_HMS, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("scvtf", 0x6594a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMS, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("scvtf", 0x65d0a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMS, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("scvtf", 0x6556a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_HMD, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("scvtf", 0x65d4a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMD, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("scvtf", 0x65d6a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMD, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("sdiv", 0x04940000, 0xffbfe000, sve_size_sd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_SD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("sdivr", 0x04960000, 0xffbfe000, sve_size_sd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_SD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("sdot", 0x44800000, 0xffa0fc00, sve_size_sd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_SD_BH, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("sdot", 0x44a00000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX), OP_SVE_VVV_S_B, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("sdot", 0x44e00000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX), OP_SVE_VVV_D_H, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("sel", 0x0520c000, 0xff20c000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg4_10, SVE_Zn, SVE_Zm_16), OP_SVE_VUVV_BHSD, F_HAS_ALIAS, 0),
  _SVE_INSN ("sel", 0x25004210, 0xfff0c210, sve_misc, 0, OP4 (SVE_Pd, SVE_Pg4_10, SVE_Pn, SVE_Pm), OP_SVE_BUBB, F_HAS_ALIAS, 0),
  _SVE_INSN ("setffr", 0x252c9000, 0xffffffff, sve_misc, 0, OP0 (), {}, 0, 0),
  _SVE_INSNC ("smax", 0x2528c000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zd, SVE_SIMM8), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 1),
  _SVE_INSNC ("smax", 0x04080000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("smaxv", 0x04082000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_VUV_BHSD, 0, 0),
  _SVE_INSNC ("smin", 0x252ac000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zd, SVE_SIMM8), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 1),
  _SVE_INSNC ("smin", 0x040a0000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("sminv", 0x040a2000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_VUV_BHSD, 0, 0),
  _SVE_INSNC ("smulh", 0x04120000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("splice", 0x052c8000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VUVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("sqadd", 0x04201000, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSNC ("sqadd", 0x2524c000, 0xff3fc000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zd, SVE_AIMM), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 1),
  _SVE_INSN ("sqdecb", 0x0430f800, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("sqdecb", 0x0420f800, 0xfff0fc00, sve_misc, 0, OP3 (Rd, Rd, SVE_PATTERN_SCALED), OP_SVE_XWU, F_OPD2_OPT | F_DEFAULT(31), 1),
  _SVE_INSNC ("sqdecd", 0x04e0c800, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_DU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("sqdecd", 0x04f0f800, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("sqdecd", 0x04e0f800, 0xfff0fc00, sve_misc, 0, OP3 (Rd, Rd, SVE_PATTERN_SCALED), OP_SVE_XWU, F_OPD2_OPT | F_DEFAULT(31), 1),
  _SVE_INSNC ("sqdech", 0x0460c800, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_HU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("sqdech", 0x0470f800, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("sqdech", 0x0460f800, 0xfff0fc00, sve_misc, 0, OP3 (Rd, Rd, SVE_PATTERN_SCALED), OP_SVE_XWU, F_OPD2_OPT | F_DEFAULT(31), 1),
  _SVE_INSNC ("sqdecp", 0x252a8000, 0xff3ffe00, sve_size_hsd, 0, OP2 (SVE_Zd, SVE_Pg4_5), OP_SVE_Vv_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("sqdecp", 0x252a8c00, 0xff3ffe00, sve_size_bhsd, 0, OP2 (Rd, SVE_Pg4_5), OP_SVE_XV_BHSD, 0, 0),
  _SVE_INSN ("sqdecp", 0x252a8800, 0xff3ffe00, sve_size_bhsd, 0, OP3 (Rd, SVE_Pg4_5, Rd), OP_SVE_XVW_BHSD, 0, 2),
  _SVE_INSNC ("sqdecw", 0x04a0c800, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_SU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("sqdecw", 0x04b0f800, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("sqdecw", 0x04a0f800, 0xfff0fc00, sve_misc, 0, OP3 (Rd, Rd, SVE_PATTERN_SCALED), OP_SVE_XWU, F_OPD2_OPT | F_DEFAULT(31), 1),
  _SVE_INSN ("sqincb", 0x0430f000, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("sqincb", 0x0420f000, 0xfff0fc00, sve_misc, 0, OP3 (Rd, Rd, SVE_PATTERN_SCALED), OP_SVE_XWU, F_OPD2_OPT | F_DEFAULT(31), 1),
  _SVE_INSNC ("sqincd", 0x04e0c000, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_DU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("sqincd", 0x04f0f000, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("sqincd", 0x04e0f000, 0xfff0fc00, sve_misc, 0, OP3 (Rd, Rd, SVE_PATTERN_SCALED), OP_SVE_XWU, F_OPD2_OPT | F_DEFAULT(31), 1),
  _SVE_INSNC ("sqinch", 0x0460c000, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_HU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("sqinch", 0x0470f000, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("sqinch", 0x0460f000, 0xfff0fc00, sve_misc, 0, OP3 (Rd, Rd, SVE_PATTERN_SCALED), OP_SVE_XWU, F_OPD2_OPT | F_DEFAULT(31), 1),
  _SVE_INSNC ("sqincp", 0x25288000, 0xff3ffe00, sve_size_hsd, 0, OP2 (SVE_Zd, SVE_Pg4_5), OP_SVE_Vv_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("sqincp", 0x25288c00, 0xff3ffe00, sve_size_bhsd, 0, OP2 (Rd, SVE_Pg4_5), OP_SVE_XV_BHSD, 0, 0),
  _SVE_INSN ("sqincp", 0x25288800, 0xff3ffe00, sve_size_bhsd, 0, OP3 (Rd, SVE_Pg4_5, Rd), OP_SVE_XVW_BHSD, 0, 2),
  _SVE_INSNC ("sqincw", 0x04a0c000, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_SU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("sqincw", 0x04b0f000, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("sqincw", 0x04a0f000, 0xfff0fc00, sve_misc, 0, OP3 (Rd, Rd, SVE_PATTERN_SCALED), OP_SVE_XWU, F_OPD2_OPT | F_DEFAULT(31), 1),
  _SVE_INSN ("sqsub", 0x04201800, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSNC ("sqsub", 0x2526c000, 0xff3fc000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zd, SVE_AIMM), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 1),
  _SVE_INSN ("st1b", 0xe4004000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_BUU, F_OD(1), 0),
  _SVE_INSN ("st1b", 0xe4008000, 0xffe0a000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_14), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1b", 0xe400a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1b", 0xe4204000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_HUU, F_OD(1), 0),
  _SVE_INSN ("st1b", 0xe4404000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_SUU, F_OD(1), 0),
  _SVE_INSN ("st1b", 0xe4408000, 0xffe0a000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_14), OP_SVE_SUS, F_OD(1), 0),
  _SVE_INSN ("st1b", 0xe4604000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_DUU, F_OD(1), 0),
  _SVE_INSN ("st1b", 0xe400e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_BUU, F_OD(1), 0),
  _SVE_INSN ("st1b", 0xe420e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_HUU, F_OD(1), 0),
  _SVE_INSN ("st1b", 0xe440a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1b", 0xe440e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_SUU, F_OD(1), 0),
  _SVE_INSN ("st1b", 0xe460a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5), OP_SVE_SUS, F_OD(1), 0),
  _SVE_INSN ("st1b", 0xe460e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DUU, F_OD(1), 0),
  _SVE_INSN ("st1d", 0xe5808000, 0xffe0a000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_14), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1d", 0xe580a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1d", 0xe5a08000, 0xffe0a000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW3_14), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1d", 0xe5a0a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_LSL3), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1d", 0xe5e04000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL3), OP_SVE_DUU, F_OD(1), 0),
  _SVE_INSN ("st1d", 0xe5c0a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x8), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1d", 0xe5e0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DUU, F_OD(1), 0),
  _SVE_INSN ("st1h", 0xe4808000, 0xffe0a000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_14), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1h", 0xe480a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1h", 0xe4a04000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_HUU, F_OD(1), 0),
  _SVE_INSN ("st1h", 0xe4a08000, 0xffe0a000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW1_14), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1h", 0xe4a0a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_LSL1), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1h", 0xe4c04000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_SUU, F_OD(1), 0),
  _SVE_INSN ("st1h", 0xe4c08000, 0xffe0a000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_14), OP_SVE_SUS, F_OD(1), 0),
  _SVE_INSN ("st1h", 0xe4e04000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_DUU, F_OD(1), 0),
  _SVE_INSN ("st1h", 0xe4e08000, 0xffe0a000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW1_14), OP_SVE_SUS, F_OD(1), 0),
  _SVE_INSN ("st1h", 0xe4a0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_HUU, F_OD(1), 0),
  _SVE_INSN ("st1h", 0xe4c0a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x2), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1h", 0xe4c0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_SUU, F_OD(1), 0),
  _SVE_INSN ("st1h", 0xe4e0a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x2), OP_SVE_SUS, F_OD(1), 0),
  _SVE_INSN ("st1h", 0xe4e0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DUU, F_OD(1), 0),
  _SVE_INSN ("st1w", 0xe5008000, 0xffe0a000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_14), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1w", 0xe500a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1w", 0xe5208000, 0xffe0a000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW2_14), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1w", 0xe520a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_LSL2), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1w", 0xe5404000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL2), OP_SVE_SUU, F_OD(1), 0),
  _SVE_INSN ("st1w", 0xe5408000, 0xffe0a000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW_14), OP_SVE_SUS, F_OD(1), 0),
  _SVE_INSN ("st1w", 0xe5604000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL2), OP_SVE_DUU, F_OD(1), 0),
  _SVE_INSN ("st1w", 0xe5608000, 0xffe0a000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RZ_XTW2_14), OP_SVE_SUS, F_OD(1), 0),
  _SVE_INSN ("st1w", 0xe540a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x4), OP_SVE_DUD, F_OD(1), 0),
  _SVE_INSN ("st1w", 0xe540e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_SUU, F_OD(1), 0),
  _SVE_INSN ("st1w", 0xe560a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZI_U5x4), OP_SVE_SUS, F_OD(1), 0),
  _SVE_INSN ("st1w", 0xe560e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DUU, F_OD(1), 0),
  _SVE_INSN ("st2b", 0xe4206000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_BUU, F_OD(2), 0),
  _SVE_INSN ("st2b", 0xe430e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_BUU, F_OD(2), 0),
  _SVE_INSN ("st2d", 0xe5a06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL3), OP_SVE_DUU, F_OD(2), 0),
  _SVE_INSN ("st2d", 0xe5b0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_DUU, F_OD(2), 0),
  _SVE_INSN ("st2h", 0xe4a06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_HUU, F_OD(2), 0),
  _SVE_INSN ("st2h", 0xe4b0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_HUU, F_OD(2), 0),
  _SVE_INSN ("st2w", 0xe5206000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL2), OP_SVE_SUU, F_OD(2), 0),
  _SVE_INSN ("st2w", 0xe530e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_SUU, F_OD(2), 0),
  _SVE_INSN ("st3b", 0xe4406000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_BUU, F_OD(3), 0),
  _SVE_INSN ("st3b", 0xe450e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x3xVL), OP_SVE_BUU, F_OD(3), 0),
  _SVE_INSN ("st3d", 0xe5c06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL3), OP_SVE_DUU, F_OD(3), 0),
  _SVE_INSN ("st3d", 0xe5d0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x3xVL), OP_SVE_DUU, F_OD(3), 0),
  _SVE_INSN ("st3h", 0xe4c06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_HUU, F_OD(3), 0),
  _SVE_INSN ("st3h", 0xe4d0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x3xVL), OP_SVE_HUU, F_OD(3), 0),
  _SVE_INSN ("st3w", 0xe5406000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL2), OP_SVE_SUU, F_OD(3), 0),
  _SVE_INSN ("st3w", 0xe550e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x3xVL), OP_SVE_SUU, F_OD(3), 0),
  _SVE_INSN ("st4b", 0xe4606000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_BUU, F_OD(4), 0),
  _SVE_INSN ("st4b", 0xe470e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_BUU, F_OD(4), 0),
  _SVE_INSN ("st4d", 0xe5e06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL3), OP_SVE_DUU, F_OD(4), 0),
  _SVE_INSN ("st4d", 0xe5f0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_DUU, F_OD(4), 0),
  _SVE_INSN ("st4h", 0xe4e06000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_HUU, F_OD(4), 0),
  _SVE_INSN ("st4h", 0xe4f0e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_HUU, F_OD(4), 0),
  _SVE_INSN ("st4w", 0xe5606000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL2), OP_SVE_SUU, F_OD(4), 0),
  _SVE_INSN ("st4w", 0xe570e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_SUU, F_OD(4), 0),
  _SVE_INSN ("stnt1b", 0xe4006000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_BUU, F_OD(1), 0),
  _SVE_INSN ("stnt1b", 0xe410e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_BUU, F_OD(1), 0),
  _SVE_INSN ("stnt1d", 0xe5806000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL3), OP_SVE_DUU, F_OD(1), 0),
  _SVE_INSN ("stnt1d", 0xe590e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_DUU, F_OD(1), 0),
  _SVE_INSN ("stnt1h", 0xe4806000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_HUU, F_OD(1), 0),
  _SVE_INSN ("stnt1h", 0xe490e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_HUU, F_OD(1), 0),
  _SVE_INSN ("stnt1w", 0xe5006000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL2), OP_SVE_SUU, F_OD(1), 0),
  _SVE_INSN ("stnt1w", 0xe510e000, 0xfff0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4xVL), OP_SVE_SUU, F_OD(1), 0),
  _SVE_INSN ("str", 0xe5800000, 0xffc0e010, sve_misc, 0, OP2 (SVE_Pt, SVE_ADDR_RI_S9xVL), {}, 0, 0),
  _SVE_INSN ("str", 0xe5800000, 0xffc0e010, sve_misc, 0, OP2 (SVE_PNt, SVE_ADDR_RI_S9xVL), {}, 0, 0),
  _SVE_INSN ("str", 0xe5804000, 0xffc0e000, sve_misc, 0, OP2 (SVE_Zt, SVE_ADDR_RI_S9xVL), {}, 0, 0),
  _SVE_INSN ("sub", 0x04200400, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSNC ("sub", 0x2521c000, 0xff3fc000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zd, SVE_AIMM), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 1),
  _SVE_INSNC ("sub", 0x04010000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("subr", 0x2523c000, 0xff3fc000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zd, SVE_AIMM), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 1),
  _SVE_INSNC ("subr", 0x04030000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("sunpkhi", 0x05313800, 0xff3ffc00, sve_size_hsd, 0, OP2 (SVE_Zd, SVE_Zn), OP_SVE_VV_HSD_BHS, 0, 0),
  _SVE_INSN ("sunpklo", 0x05303800, 0xff3ffc00, sve_size_hsd, 0, OP2 (SVE_Zd, SVE_Zn), OP_SVE_VV_HSD_BHS, 0, 0),
  _SVE_INSNC ("sxtb", 0x0410a000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("sxth", 0x0492a000, 0xffbfe000, sve_size_sd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_SD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("sxtw", 0x04d4a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("tbl", 0x05203000, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_ZnxN, SVE_Zm_16), OP_SVE_VVV_BHSD, F_OD(1), 0),
  _SVE_INSN ("trn1", 0x05205000, 0xff30fe10, sve_size_bhsd, 0, OP3 (SVE_Pd, SVE_Pn, SVE_Pm), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSN ("trn1", 0x05207000, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSN ("trn2", 0x05205400, 0xff30fe10, sve_size_bhsd, 0, OP3 (SVE_Pd, SVE_Pn, SVE_Pm), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSN ("trn2", 0x05207400, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSNC ("uabd", 0x040d0000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("uaddv", 0x04012000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_DUV_BHSD, 0, 0),
  _SVE_INSNC ("ucvtf", 0x6553a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_HMH, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("ucvtf", 0x6555a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_HMS, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("ucvtf", 0x6595a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMS, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("ucvtf", 0x65d1a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMS, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("ucvtf", 0x6557a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_HMD, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("ucvtf", 0x65d5a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMD, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("ucvtf", 0x65d7a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMD, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  _SVE_INSNC ("udiv", 0x04950000, 0xffbfe000, sve_size_sd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_SD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("udivr", 0x04970000, 0xffbfe000, sve_size_sd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_SD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSNC ("udot", 0x44800400, 0xffa0fc00, sve_size_sd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_SD_BH, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("udot", 0x44a00400, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX), OP_SVE_VVV_S_B, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("udot", 0x44e00400, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX), OP_SVE_VVV_D_H, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("umax", 0x2529c000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zd, SVE_UIMM8), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 1),
  _SVE_INSNC ("umax", 0x04090000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("umaxv", 0x04092000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_VUV_BHSD, 0, 0),
  _SVE_INSNC ("umin", 0x252bc000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zd, SVE_UIMM8), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 1),
  _SVE_INSNC ("umin", 0x040b0000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("uminv", 0x040b2000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Vd, SVE_Pg3, SVE_Zn), OP_SVE_VUV_BHSD, 0, 0),
  _SVE_INSNC ("umulh", 0x04130000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zm_5), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  _SVE_INSN ("uqadd", 0x04201400, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSNC ("uqadd", 0x2525c000, 0xff3fc000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zd, SVE_AIMM), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 1),
  _SVE_INSN ("uqdecb", 0x0420fc00, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_WU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("uqdecb", 0x0430fc00, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSNC ("uqdecd", 0x04e0cc00, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_DU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("uqdecd", 0x04e0fc00, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_WU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("uqdecd", 0x04f0fc00, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSNC ("uqdech", 0x0460cc00, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_HU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("uqdech", 0x0460fc00, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_WU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("uqdech", 0x0470fc00, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSNC ("uqdecp", 0x252b8000, 0xff3ffe00, sve_size_hsd, 0, OP2 (SVE_Zd, SVE_Pg4_5), OP_SVE_Vv_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("uqdecp", 0x252b8800, 0xff3ffe00, sve_size_bhsd, 0, OP2 (Rd, SVE_Pg4_5), OP_SVE_WV_BHSD, 0, 0),
  _SVE_INSN ("uqdecp", 0x252b8c00, 0xff3ffe00, sve_size_bhsd, 0, OP2 (Rd, SVE_Pg4_5), OP_SVE_XV_BHSD, 0, 0),
  _SVE_INSNC ("uqdecw", 0x04a0cc00, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_SU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("uqdecw", 0x04a0fc00, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_WU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("uqdecw", 0x04b0fc00, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("uqincb", 0x0420f400, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_WU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("uqincb", 0x0430f400, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSNC ("uqincd", 0x04e0c400, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_DU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("uqincd", 0x04e0f400, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_WU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("uqincd", 0x04f0f400, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSNC ("uqinch", 0x0460c400, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_HU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("uqinch", 0x0460f400, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_WU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("uqinch", 0x0470f400, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSNC ("uqincp", 0x25298000, 0xff3ffe00, sve_size_hsd, 0, OP2 (SVE_Zd, SVE_Pg4_5), OP_SVE_Vv_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("uqincp", 0x25298800, 0xff3ffe00, sve_size_bhsd, 0, OP2 (Rd, SVE_Pg4_5), OP_SVE_WV_BHSD, 0, 0),
  _SVE_INSN ("uqincp", 0x25298c00, 0xff3ffe00, sve_size_bhsd, 0, OP2 (Rd, SVE_Pg4_5), OP_SVE_XV_BHSD, 0, 0),
  _SVE_INSNC ("uqincw", 0x04a0c400, 0xfff0fc00, sve_misc, 0, OP2 (SVE_Zd, SVE_PATTERN_SCALED), OP_SVE_SU, F_OPD1_OPT | F_DEFAULT(31), C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("uqincw", 0x04a0f400, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_WU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("uqincw", 0x04b0f400, 0xfff0fc00, sve_misc, 0, OP2 (Rd, SVE_PATTERN_SCALED), OP_SVE_XU, F_OPD1_OPT | F_DEFAULT(31), 0),
  _SVE_INSN ("uqsub", 0x04201c00, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSNC ("uqsub", 0x2527c000, 0xff3fc000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zd, SVE_AIMM), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 1),
  _SVE_INSN ("uunpkhi", 0x05333800, 0xff3ffc00, sve_size_hsd, 0, OP2 (SVE_Zd, SVE_Zn), OP_SVE_VV_HSD_BHS, 0, 0),
  _SVE_INSN ("uunpklo", 0x05323800, 0xff3ffc00, sve_size_hsd, 0, OP2 (SVE_Zd, SVE_Zn), OP_SVE_VV_HSD_BHS, 0, 0),
  _SVE_INSNC ("uxtb", 0x0411a000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("uxth", 0x0493a000, 0xffbfe000, sve_size_sd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_SD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("uxtw", 0x04d5a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMD, 0, C_SCAN_MOVPRFX, 0),
  _SVE_INSN ("uzp1", 0x05204800, 0xff30fe10, sve_size_bhsd, 0, OP3 (SVE_Pd, SVE_Pn, SVE_Pm), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSN ("uzp1", 0x05206800, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSN ("uzp2", 0x05204c00, 0xff30fe10, sve_size_bhsd, 0, OP3 (SVE_Pd, SVE_Pn, SVE_Pm), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSN ("uzp2", 0x05206c00, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSN ("whilele", 0x25200410, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VWW_BHSD, 0, 0),
  _SVE_INSN ("whilele", 0x25201410, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  _SVE_INSN ("whilelo", 0x25200c00, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VWW_BHSD, 0, 0),
  _SVE_INSN ("whilelo", 0x25201c00, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  _SVE_INSN ("whilels", 0x25200c10, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VWW_BHSD, 0, 0),
  _SVE_INSN ("whilels", 0x25201c10, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  _SVE_INSN ("whilelt", 0x25200400, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VWW_BHSD, 0, 0),
  _SVE_INSN ("whilelt", 0x25201400, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  _SVE_INSN ("wrffr", 0x25289000, 0xfffffe1f, sve_misc, 0, OP1 (SVE_Pn), OP_SVE_B, 0, 0),
  _SVE_INSN ("zip1", 0x05204000, 0xff30fe10, sve_size_bhsd, 0, OP3 (SVE_Pd, SVE_Pn, SVE_Pm), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSN ("zip1", 0x05206000, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSN ("zip2", 0x05204400, 0xff30fe10, sve_size_bhsd, 0, OP3 (SVE_Pd, SVE_Pn, SVE_Pm), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSN ("zip2", 0x05206400, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  _SVE_INSNC ("bic", 0x05800000, 0xfffc0000, sve_limm, 0, OP3 (SVE_Zd, SVE_Zd, SVE_INV_LIMM), OP_SVE_VVU_BHSD, F_ALIAS | F_PSEUDO, C_SCAN_MOVPRFX, 1),
  _SVE_INSN ("cmple", 0x24008000, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zm_16, SVE_Zn), OP_SVE_VZVV_BHSD, F_ALIAS | F_PSEUDO, 0),
  _SVE_INSN ("cmplo", 0x24000010, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zm_16, SVE_Zn), OP_SVE_VZVV_BHSD, F_ALIAS | F_PSEUDO, 0),
  _SVE_INSN ("cmpls", 0x24000000, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zm_16, SVE_Zn), OP_SVE_VZVV_BHSD, F_ALIAS | F_PSEUDO, 0),
  _SVE_INSN ("cmplt", 0x24008010, 0xff20e010, sve_size_bhsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zm_16, SVE_Zn), OP_SVE_VZVV_BHSD, F_ALIAS | F_PSEUDO, 0),
  _SVE_INSNC ("eon", 0x05400000, 0xfffc0000, sve_limm, 0, OP3 (SVE_Zd, SVE_Zd, SVE_INV_LIMM), OP_SVE_VVU_BHSD, F_ALIAS | F_PSEUDO, C_SCAN_MOVPRFX, 1),
  _SVE_INSN ("facle", 0x6500c010, 0xff20e010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zm_16, SVE_Zn), OP_SVE_VZVV_HSD, F_ALIAS | F_PSEUDO, 0),
  _SVE_INSN ("faclt", 0x6500e010, 0xff20e010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zm_16, SVE_Zn), OP_SVE_VZVV_HSD, F_ALIAS | F_PSEUDO, 0),
  _SVE_INSN ("fcmle", 0x65004000, 0xff20e010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zm_16, SVE_Zn), OP_SVE_VZVV_HSD, F_ALIAS | F_PSEUDO, 0),
  _SVE_INSN ("fcmlt", 0x65004010, 0xff20e010, sve_size_hsd, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zm_16, SVE_Zn), OP_SVE_VZVV_HSD, F_ALIAS | F_PSEUDO, 0),
  _SVE_INSN ("fmov", 0x2538c000, 0xff3fffe0, sve_size_hsd, 0, OP2 (SVE_Zd, FPIMM0), OP_SVE_V_HSD, F_ALIAS | F_PSEUDO, 0),
  _SVE_INSNC ("fmov", 0x05104000, 0xff30ffe0, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg4_16, FPIMM0), OP_SVE_VM_HSD, F_ALIAS | F_PSEUDO, C_SCAN_MOVPRFX, 0),
  _SVE_INSNC ("orn", 0x05000000, 0xfffc0000, sve_limm, 0, OP3 (SVE_Zd, SVE_Zd, SVE_INV_LIMM), OP_SVE_VVU_BHSD, F_ALIAS | F_PSEUDO, C_SCAN_MOVPRFX, 1),

  /* SVE2 instructions.  */
  SVE2_INSNC ("adclb", 0x4500d000, 0xffa0fc00, sve_size_sd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_SD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("adclt", 0x4500d400, 0xffa0fc00, sve_size_sd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_SD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("addhnb", 0x45206000, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHS_HSD, 0, 0),
  SVE2_INSN ("addhnt", 0x45206400, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHS_HSD, 0, 0),
  SVE2_INSNC ("addp", 0x4411a000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("bcax", 0x04603800, 0xffe0fc00, sve_misc, 0, OP4 (SVE_Zd, SVE_Zd, SVE_Zm_16, SVE_Zn), OP_SVE_DDDD, 0, C_SCAN_MOVPRFX, 1),
  SVE2_INSNC ("bsl", 0x04203c00, 0xffe0fc00, sve_misc, 0, OP4 (SVE_Zd, SVE_Zd, SVE_Zm_16, SVE_Zn), OP_SVE_DDDD, 0, C_SCAN_MOVPRFX, 1),
  SVE2_INSNC ("bsl1n", 0x04603c00, 0xffe0fc00, sve_misc, 0, OP4 (SVE_Zd, SVE_Zd, SVE_Zm_16, SVE_Zn), OP_SVE_DDDD, 0, C_SCAN_MOVPRFX, 1),
  SVE2_INSNC ("bsl2n", 0x04a03c00, 0xffe0fc00, sve_misc, 0, OP4 (SVE_Zd, SVE_Zd, SVE_Zm_16, SVE_Zn), OP_SVE_DDDD, 0, C_SCAN_MOVPRFX, 1),
  SVE2_INSNC ("cadd", 0x4500d800, 0xff3ff800, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Zd, SVE_Zn, SVE_IMM_ROT3), OP_SVE_VVVU_BHSD, 0, C_SCAN_MOVPRFX, 1),
  SVE2_INSNC ("cdot", 0x44801000, 0xffa0f000, sve_size_sd, 0, OP4 (SVE_Zd, SVE_Zn, SVE_Zm_16, SVE_IMM_ROT2), OP_SVE_VVVU_SD_BH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("cdot", 0x44e04000, 0xffe0f000, sve_misc, 0, OP4 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX, SVE_IMM_ROT2), OP_SVE_DHHU, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("cdot", 0x44a04000, 0xffe0f000, sve_misc, 0, OP4 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX, SVE_IMM_ROT2), OP_SVE_SBBU, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("cmla", 0x44002000, 0xff20f000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Zn, SVE_Zm_16, SVE_IMM_ROT2), OP_SVE_VVVU_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("cmla", 0x44a06000, 0xffe0f000, sve_misc, 0, OP4 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX, SVE_IMM_ROT2), OP_SVE_VVVU_H, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("cmla", 0x44e06000, 0xffe0f000, sve_misc, 0, OP4 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX, SVE_IMM_ROT2), OP_SVE_VVVU_S, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("eor3", 0x04203800, 0xffe0fc00, sve_misc, 0, OP4 (SVE_Zd, SVE_Zd, SVE_Zm_16, SVE_Zn), OP_SVE_DDDD, 0, C_SCAN_MOVPRFX, 1),
  SVE2_INSNC ("eorbt", 0x45009000, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("eortb", 0x45009400, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("ext", 0x05600000, 0xffe0e000, sve_misc, 0, OP3 (SVE_Zd, SVE_ZnxN, SVE_UIMM8_53), OP_SVE_BBU, F_OD(2), 0),
  SVE2_INSNC ("faddp", 0x64108000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSN ("fcvtlt", 0x6489a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMH, 0, 0),
  SVE2_INSN ("fcvtlt", 0x64cba000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_DMS, 0, 0),
  SVE2_INSN ("fcvtnt", 0x6488a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_HMS, 0, 0),
  SVE2_INSN ("fcvtnt", 0x64caa000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMD, 0, 0),
  SVE2_INSNC ("fcvtx", 0x650aa000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMD, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  SVE2_INSN ("fcvtxnt", 0x640aa000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMD, 0, 0),
  SVE2_INSNC ("flogb", 0x6518a000, 0xfff9e000, sve_size_hsd2, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("fmaxnmp", 0x64148000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("fmaxp", 0x64168000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("fminnmp", 0x64158000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("fminp", 0x64178000, 0xff3fe000, sve_size_hsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_HSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("fmlalb", 0x64a04000, 0xffe0f400, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("fmlalb", 0x64a08000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("fmlalt", 0x64a04400, 0xffe0f400, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("fmlalt", 0x64a08400, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("fmlslb", 0x64a06000, 0xffe0f400, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("fmlslb", 0x64a0a000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("fmlslt", 0x64a06400, 0xffe0f400, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("fmlslt", 0x64a0a400, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("histcnt", 0x45a0c000, 0xffa0e000, sve_size_sd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_SD, 0, 0),
  SVE2_INSN ("histseg", 0x4520a000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_BBB, 0, 0),
  SVE2_INSN ("ldnt1b", 0x8400a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_SZS, F_OD(1), 0),
  SVE2_INSN ("ldnt1b", 0xc400c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_DZD, F_OD(1), 0),
  SVE2_INSN ("ldnt1d", 0xc580c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_DZD, F_OD(1), 0),
  SVE2_INSN ("ldnt1h", 0x8480a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_SZS, F_OD(1), 0),
  SVE2_INSN ("ldnt1h", 0xc480c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_DZD, F_OD(1), 0),
  SVE2_INSN ("ldnt1sb", 0x84008000, 0xbfe0e000, sve_size_sd2, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_VZV_SD, F_OD(1), 0),
  SVE2_INSN ("ldnt1sh", 0x84808000, 0xbfe0e000, sve_size_sd2, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_VZV_SD, F_OD(1), 0),
  SVE2_INSN ("ldnt1sw", 0xc5008000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_DZD, F_OD(1), 0),
  SVE2_INSN ("ldnt1w", 0x8500a000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_SZS, F_OD(1), 0),
  SVE2_INSN ("ldnt1w", 0xc500c000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_DZD, F_OD(1), 0),
  SVE2_INSN ("match", 0x45208000, 0xffa0e010, sve_size_bh, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_BH, 0, 0),
  SVE2_INSNC ("mla", 0x44200800, 0xffa0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_22_INDEX), OP_SVE_HHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("mla", 0x44a00800, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX), OP_SVE_SSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("mla", 0x44e00800, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX), OP_SVE_DDD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("mls", 0x44200c00, 0xffa0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_22_INDEX), OP_SVE_HHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("mls", 0x44a00c00, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX), OP_SVE_SSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("mls", 0x44e00c00, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX), OP_SVE_DDD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("mul", 0x4420f800, 0xffa0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_22_INDEX), OP_SVE_HHH, 0, 0),
  SVE2_INSN ("mul", 0x44a0f800, 0xffe0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX), OP_SVE_SSS, 0, 0),
  SVE2_INSN ("mul", 0x44e0f800, 0xffe0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX), OP_SVE_DDD, 0, 0),
  SVE2_INSN ("mul", 0x04206000, 0xff20fc00,  sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  SVE2_INSNC ("nbsl", 0x04e03c00, 0xffe0fc00, sve_misc, 0, OP4 (SVE_Zd, SVE_Zd, SVE_Zm_16, SVE_Zn), OP_SVE_DDDD, 0, C_SCAN_MOVPRFX, 1),
  SVE2_INSN ("nmatch", 0x45208010, 0xffa0e010,  sve_size_bh, 0, OP4 (SVE_Pd, SVE_Pg3, SVE_Zn, SVE_Zm_16), OP_SVE_VZVV_BH, 0, 0),
  SVE2_INSN ("pmul", 0x04206400, 0xffe0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_BBB, 0, 0),
  SVE2_INSN ("pmullb", 0x45406800, 0xff60fc00, sve_size_13, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HD_BS, 0, 0),
  SVE2_INSN ("pmullt", 0x45406c00, 0xff60fc00, sve_size_13, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HD_BS, 0, 0),
  SVE2_INSN ("raddhnb", 0x45206800, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHS_HSD, 0, 0),
  SVE2_INSN ("raddhnt", 0x45206c00, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHS_HSD, 0, 0),
  SVE2_INSN ("rshrnb", 0x45201800, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSN ("rshrnt", 0x45201c00, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSN ("rsubhnb", 0x45207800, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHS_HSD, 0, 0),
  SVE2_INSN ("rsubhnt", 0x45207c00, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHS_HSD, 0, 0),
  SVE2_INSNC ("saba", 0x4500f800, 0xff20fc00,  sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sabalb", 0x4500c000, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sabalt", 0x4500c400, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("sabdlb", 0x45003000, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("sabdlt", 0x45003400, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSNC ("sadalp", 0x4404a000, 0xff3fe000,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("saddlb", 0x45000000, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("saddlbt", 0x45008000, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("saddlt", 0x45000400, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("saddwb", 0x45004000, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS2, 0, 0),
  SVE2_INSN ("saddwt", 0x45004400, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS2, 0, 0),
  SVE2_INSNC ("sbclb", 0x4580d000, 0xffa0fc00,  sve_size_sd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_SD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sbclt", 0x4580d400, 0xffa0fc00,  sve_size_sd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_SD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("shadd", 0x44108000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSN ("shrnb", 0x45201000, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSN ("shrnt", 0x45201400, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSNC ("shsub", 0x44128000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("shsubr", 0x44168000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSN ("sli", 0x4500f400, 0xff20fc00,  sve_shift_tsz_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHLIMM_UNPRED), OP_SVE_VVU_BHSD, 0, 0),
  SVE2_INSNC ("smaxp", 0x4414a000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("sminp", 0x4416a000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("smlalb", 0x44a08000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("smlalb", 0x44e08000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("smlalb", 0x44004000, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("smlalt", 0x44a08400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("smlalt", 0x44e08400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("smlalt", 0x44004400, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("smlslb", 0x44a0a000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("smlslb", 0x44e0a000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("smlslb", 0x44005000, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("smlslt", 0x44a0a400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("smlslt", 0x44e0a400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("smlslt", 0x44005400, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("smulh", 0x04206800, 0xff20fc00,  sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  SVE2_INSN ("smullb", 0x44a0c000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, 0),
  SVE2_INSN ("smullb", 0x44e0c000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, 0),
  SVE2_INSN ("smullb", 0x45007000, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("smullt", 0x44a0c400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, 0),
  SVE2_INSN ("smullt", 0x44e0c400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, 0),
  SVE2_INSN ("smullt", 0x45007400, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("splice", 0x052d8000, 0xff3fe000, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_ZnxN), OP_SVE_VUV_BHSD, F_OD(2), 0),
  SVE2_INSNC ("sqabs", 0x4408a000, 0xff3fe000,  sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqadd", 0x44188000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("sqcadd", 0x4501d800, 0xff3ff800,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Zd, SVE_Zn, SVE_IMM_ROT3), OP_SVE_VVVU_BHSD, 0, C_SCAN_MOVPRFX, 1),
  SVE2_INSNC ("sqdmlalb", 0x44a02000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqdmlalb", 0x44e02000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqdmlalb", 0x44006000, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqdmlalbt", 0x44000800, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqdmlalt", 0x44a02400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqdmlalt", 0x44e02400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqdmlalt", 0x44006400, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqdmlslb", 0x44a03000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqdmlslb", 0x44e03000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqdmlslb", 0x44006800, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqdmlslbt", 0x44000c00, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqdmlslt", 0x44a03400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqdmlslt", 0x44e03400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqdmlslt", 0x44006c00, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("sqdmulh", 0x4420f000, 0xffa0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_22_INDEX), OP_SVE_HHH, 0, 0),
  SVE2_INSN ("sqdmulh", 0x44a0f000, 0xffe0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX), OP_SVE_SSS, 0, 0),
  SVE2_INSN ("sqdmulh", 0x44e0f000, 0xffe0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX), OP_SVE_DDD, 0, 0),
  SVE2_INSN ("sqdmulh", 0x04207000, 0xff20fc00,  sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  SVE2_INSN ("sqdmullb", 0x44a0e000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, 0),
  SVE2_INSN ("sqdmullb", 0x44e0e000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, 0),
  SVE2_INSN ("sqdmullb", 0x45006000, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("sqdmullt", 0x44a0e400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, 0),
  SVE2_INSN ("sqdmullt", 0x44e0e400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, 0),
  SVE2_INSN ("sqdmullt", 0x45006400, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSNC ("sqneg", 0x4409a000, 0xff3fe000,  sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqrdcmlah", 0x44a07000, 0xffe0f000,  sve_misc, 0, OP4 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX, SVE_IMM_ROT2), OP_SVE_HHHU, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqrdcmlah", 0x44e07000, 0xffe0f000,  sve_misc, 0, OP4 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX, SVE_IMM_ROT2), OP_SVE_SSSU, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqrdcmlah", 0x44003000, 0xff20f000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Zn, SVE_Zm_16, SVE_IMM_ROT2), OP_SVE_VVVU_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqrdmlah", 0x44201000, 0xffa0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_22_INDEX), OP_SVE_HHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqrdmlah", 0x44a01000, 0xffe0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX), OP_SVE_SSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqrdmlah", 0x44e01000, 0xffe0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX), OP_SVE_DDD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqrdmlah", 0x44007000, 0xff20fc00,  sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqrdmlsh", 0x44201400, 0xffa0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_22_INDEX), OP_SVE_HHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqrdmlsh", 0x44a01400, 0xffe0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX), OP_SVE_SSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqrdmlsh", 0x44e01400, 0xffe0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX), OP_SVE_DDD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("sqrdmlsh", 0x44007400, 0xff20fc00,  sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("sqrdmulh", 0x4420f400, 0xffa0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_22_INDEX), OP_SVE_HHH, 0, 0),
  SVE2_INSN ("sqrdmulh", 0x44a0f400, 0xffe0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX), OP_SVE_SSS, 0, 0),
  SVE2_INSN ("sqrdmulh", 0x44e0f400, 0xffe0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_INDEX), OP_SVE_DDD, 0, 0),
  SVE2_INSN ("sqrdmulh", 0x04207400, 0xff20fc00,  sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  SVE2_INSNC ("sqrshl", 0x440a8000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("sqrshlr", 0x440e8000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSN ("sqrshrnb", 0x45202800, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSN ("sqrshrnt", 0x45202c00, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSN ("sqrshrunb", 0x45200800, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSN ("sqrshrunt", 0x45200c00, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSNC ("sqshl", 0x04068000, 0xff3fe000,  sve_shift_pred, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_SHLIMM_PRED), OP_SVE_VMVU_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("sqshl", 0x44088000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("sqshlr", 0x440c8000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("sqshlu", 0x040f8000, 0xff3fe000,  sve_shift_pred, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_SHLIMM_PRED), OP_SVE_VMVU_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSN ("sqshrnb", 0x45202000, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSN ("sqshrnt", 0x45202400, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSN ("sqshrunb", 0x45200000, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSN ("sqshrunt", 0x45200400, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSNC ("sqsub", 0x441a8000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("sqsubr", 0x441e8000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSN ("sqxtnb", 0x45204000, 0xffa7fc00,  sve_size_tsz_bhs, 0, OP2 (SVE_Zd, SVE_Zn), OP_SVE_VV_BHS_HSD, 0, 0),
  SVE2_INSN ("sqxtnt", 0x45204400, 0xffa7fc00,  sve_size_tsz_bhs, 0, OP2 (SVE_Zd, SVE_Zn), OP_SVE_VV_BHS_HSD, 0, 0),
  SVE2_INSN ("sqxtunb", 0x45205000, 0xffa7fc00,  sve_size_tsz_bhs, 0, OP2 (SVE_Zd, SVE_Zn), OP_SVE_VV_BHS_HSD, 0, 0),
  SVE2_INSN ("sqxtunt", 0x45205400, 0xffa7fc00,  sve_size_tsz_bhs, 0, OP2 (SVE_Zd, SVE_Zn), OP_SVE_VV_BHS_HSD, 0, 0),
  SVE2_INSNC ("srhadd", 0x44148000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSN ("sri", 0x4500f000, 0xff20fc00,  sve_shift_tsz_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED), OP_SVE_VVU_BHSD, 0, 0),
  SVE2_INSNC ("srshl", 0x44028000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("srshlr", 0x44068000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("srshr", 0x040c8000, 0xff3fe000,  sve_shift_pred, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_SHRIMM_PRED), OP_SVE_VMVU_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("srsra", 0x4500e800, 0xff20fc00,  sve_shift_tsz_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("sshllb", 0x4500a000, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHLIMM_UNPRED_22), OP_SVE_VVU_HSD_BHS, 0, 0),
  SVE2_INSN ("sshllt", 0x4500a400, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHLIMM_UNPRED_22), OP_SVE_VVU_HSD_BHS, 0, 0),
  SVE2_INSNC ("ssra", 0x4500e000, 0xff20fc00,  sve_shift_tsz_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("ssublb", 0x45001000, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("ssublbt", 0x45008800, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("ssublt", 0x45001400, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("ssubltb", 0x45008c00, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("ssubwb", 0x45005000, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS2, 0, 0),
  SVE2_INSN ("ssubwt", 0x45005400, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS2, 0, 0),
  SVE2_INSN ("stnt1b", 0xe4402000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_SUS, F_OD(1), 0),
  SVE2_INSN ("stnt1b", 0xe4002000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_DUD, F_OD(1), 0),
  SVE2_INSN ("stnt1d", 0xe5802000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_DUD, F_OD(1), 0),
  SVE2_INSN ("stnt1h", 0xe4c02000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_SUS, F_OD(1), 0),
  SVE2_INSN ("stnt1h", 0xe4802000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_DUD, F_OD(1), 0),
  SVE2_INSN ("stnt1w", 0xe5402000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_SUS, F_OD(1), 0),
  SVE2_INSN ("stnt1w", 0xe5002000, 0xffe0e000, sve_misc, 0, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_ZX), OP_SVE_DUD, F_OD(1), 0),
  SVE2_INSN ("subhnb", 0x45207000, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHS_HSD, 0, 0),
  SVE2_INSN ("subhnt", 0x45207400, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHS_HSD, 0, 0),
  SVE2_INSNC ("suqadd", 0x441c8000, 0xff3fe000,  sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSN ("tbl", 0x05202800, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_ZnxN, SVE_Zm_16), OP_SVE_VVV_BHSD, F_OD(2), 0),
  SVE2_INSN ("tbx", 0x05202c00, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  SVE2_INSNC ("uaba", 0x4500fc00, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("uabalb", 0x4500c800, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("uabalt", 0x4500cc00, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("uabdlb", 0x45003800, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("uabdlt", 0x45003c00, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSNC ("uadalp", 0x4405a000, 0xff3fe000, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_VMV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("uaddlb", 0x45000800, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("uaddlt", 0x45000c00, 0xff20fc00, sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("uaddwb", 0x45004800, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS2, 0, 0),
  SVE2_INSN ("uaddwt", 0x45004c00, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS2, 0, 0),
  SVE2_INSNC ("uhadd", 0x44118000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("uhsub", 0x44138000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("uhsubr", 0x44178000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("umaxp", 0x4415a000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("uminp", 0x4417a000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("umlalb", 0x44a09000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("umlalb", 0x44e09000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("umlalb", 0x44004800, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("umlalt", 0x44a09400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("umlalt", 0x44e09400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("umlalt", 0x44004c00, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("umlslb", 0x44a0b000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("umlslb", 0x44e0b000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("umlslb", 0x44005800, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("umlslt", 0x44a0b400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("umlslt", 0x44e0b400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("umlslt", 0x44005c00, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("umulh", 0x04206c00, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  SVE2_INSN ("umullb", 0x44a0d000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, 0),
  SVE2_INSN ("umullb", 0x44e0d000, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, 0),
  SVE2_INSN ("umullb", 0x45007800, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("umullt", 0x44a0d400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, 0),
  SVE2_INSN ("umullt", 0x44e0d400, 0xffe0f400,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm4_11_INDEX), OP_SVE_DSS, 0, 0),
  SVE2_INSN ("umullt", 0x45007c00, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSNC ("uqadd", 0x44198000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("uqrshl", 0x440b8000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("uqrshlr", 0x440f8000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSN ("uqrshrnb", 0x45203800, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSN ("uqrshrnt", 0x45203c00, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSNC ("uqshl", 0x04078000, 0xff3fe000,  sve_shift_pred, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_SHLIMM_PRED), OP_SVE_VMVU_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("uqshl", 0x44098000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("uqshlr", 0x440d8000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSN ("uqshrnb", 0x45203000, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSN ("uqshrnt", 0x45203400, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED_22), OP_SVE_VVU_BHS_HSD, 0, 0),
  SVE2_INSNC ("uqsub", 0x441b8000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("uqsubr", 0x441f8000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSN ("uqxtnb", 0x45204800, 0xffa7fc00,  sve_size_tsz_bhs, 0, OP2 (SVE_Zd, SVE_Zn), OP_SVE_VV_BHS_HSD, 0, 0),
  SVE2_INSN ("uqxtnt", 0x45204c00, 0xffa7fc00,  sve_size_tsz_bhs, 0, OP2 (SVE_Zd, SVE_Zn), OP_SVE_VV_BHS_HSD, 0, 0),
  SVE2_INSNC ("urecpe", 0x4480a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("urhadd", 0x44158000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("urshl", 0x44038000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("urshlr", 0x44078000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("urshr", 0x040d8000, 0xff3fe000,  sve_shift_pred, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_SHRIMM_PRED), OP_SVE_VMVU_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("ursqrte", 0x4481a000, 0xffffe000, sve_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_SMS, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSNC ("ursra", 0x4500ec00, 0xff20fc00,  sve_shift_tsz_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("ushllb", 0x4500a800, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHLIMM_UNPRED_22), OP_SVE_VVU_HSD_BHS, 0, 0),
  SVE2_INSN ("ushllt", 0x4500ac00, 0xffa0fc00,  sve_shift_tsz_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHLIMM_UNPRED_22), OP_SVE_VVU_HSD_BHS, 0, 0),
  SVE2_INSNC ("usqadd", 0x441d8000, 0xff3fe000, sve_size_bhsd, 0, OP4 (SVE_Zd, SVE_Pg3, SVE_Zd, SVE_Zn), OP_SVE_VMVV_BHSD, 0, C_SCAN_MOVPRFX, 2),
  SVE2_INSNC ("usra", 0x4500e400, 0xff20fc00,  sve_shift_tsz_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED), OP_SVE_VVU_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SVE2_INSN ("usublb", 0x45001800, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("usublt", 0x45001c00, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS, 0, 0),
  SVE2_INSN ("usubwb", 0x45005800, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS2, 0, 0),
  SVE2_INSN ("usubwt", 0x45005c00, 0xff20fc00,  sve_size_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD_BHS2, 0, 0),
  SVE2_INSN ("whilege", 0x25200000, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VWW_BHSD, 0, 0),
  SVE2_INSN ("whilege", 0x25201000, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  SVE2_INSN ("whilegt", 0x25200010, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VWW_BHSD, 0, 0),
  SVE2_INSN ("whilegt", 0x25201010, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  SVE2_INSN ("whilehi", 0x25200810, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VWW_BHSD, 0, 0),
  SVE2_INSN ("whilehi", 0x25201810, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  SVE2_INSN ("whilehs", 0x25200800, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VWW_BHSD, 0, 0),
  SVE2_INSN ("whilehs", 0x25201800, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  SVE2_INSN ("whilerw", 0x25203010, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  SVE2_INSN ("whilewr", 0x25203000, 0xff20fc10, sve_size_bhsd, 0, OP3 (SVE_Pd, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  SVE2_INSNC ("xar", 0x04203400, 0xff20fc00, sve_shift_tsz_bhsd, 0, OP4 (SVE_Zd, SVE_Zd, SVE_Zn, SVE_SHRIMM_UNPRED), OP_SVE_VVVU_BHSD, 0, C_SCAN_MOVPRFX, 1),
  /* SVE2_SM4 instructions.  */
  SVE2SM4_INSN ("sm4e", 0x4523e000, 0xfffffc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zd, SVE_Zn), OP_SVE_SSS, 0, 1),
  SVE2SM4_INSN ("sm4ekey", 0x4520f000, 0xffe0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SSS, 0, 0),
  /* SVE2_AES instructions.  */
  SVE2AES_INSN ("aesd", 0x4522e400, 0xfffffc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zd, SVE_Zn), OP_SVE_BBB, 0, 1),
  SVE2AES_INSN ("aese", 0x4522e000, 0xfffffc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zd, SVE_Zn), OP_SVE_BBB, 0, 1),
  SVE2AES_INSN ("aesimc", 0x4520e400, 0xffffffe0, sve_misc, 0, OP2 (SVE_Zd, SVE_Zd), OP_SVE_BB, 0, 1),
  SVE2AES_INSN ("aesmc", 0x4520e000, 0xffffffe0, sve_misc, 0, OP2 (SVE_Zd, SVE_Zd), OP_SVE_BB, 0, 1),
  SVE2AES_INSN ("pmullb", 0x45006800, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_Q_D, 0, 0),
  SVE2AES_INSN ("pmullt", 0x45006c00, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_Q_D, 0, 0),
  /* SVE2_SHA3 instructions.  */
  SVE2SHA3_INSN ("rax1", 0x4520f400, 0xffe0fc00,  sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_DDD, 0, 0),
  /* SVE2_BITPERM instructions. */
  SVE2BITPERM_INSN ("bdep", 0x4500b400, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  SVE2BITPERM_INSN ("bext", 0x4500b000, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  SVE2BITPERM_INSN ("bgrp", 0x4500b800, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  /* SME instructions.  */
  SME_INSN ("addha", 0xc0900000, 0xffff001c, sme_misc, 0, OP4 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn), OP_SVE_SMMS, 0, 0),
  SME_I16I64_INSN ("addha", 0xc0d00000, 0xffff0018, sme_misc, 0, OP4 (SME_ZAda_3b, SVE_Pg3, SME_Pm, SVE_Zn), OP_SVE_DMMD, 0, 0),
  SME_INSN ("addspl", 0x04605800, 0xffe0f800, sme_misc, 0, OP3 (Rd_SP, SVE_Rn_SP, SVE_SIMM6), OP_SVE_XXU, 0, 0),
  SME_INSN ("addsvl", 0x04205800, 0xffe0f800, sme_misc, 0, OP3 (Rd_SP, SVE_Rn_SP, SVE_SIMM6), OP_SVE_XXU, 0, 0),
  SME_INSN ("addva", 0xc0910000, 0xffff001c, sme_misc, 0, OP4 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn), OP_SVE_SMMS, 0, 0),
  SME_I16I64_INSN ("addva", 0xc0d10000, 0xffff0018, sme_misc, 0, OP4 (SME_ZAda_3b, SVE_Pg3, SME_Pm, SVE_Zn), OP_SVE_DMMD, 0, 0),
  SME_INSN ("bfmopa", 0x81800000, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMHH, 0, 0),
  SME_INSN ("bfmops", 0x81800010, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMHH, 0, 0),
  SME_INSN ("fmopa", 0x80800000, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMSS, 0, 0),
  SME_F64F64_INSN ("fmopa", 0x80c00000, 0xffe00018, sme_misc, 0, OP5 (SME_ZAda_3b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_DMMDD, 0, 0),
  SME_INSN ("fmopa", 0x81a00000, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMHH, 0, 0),
  SME_INSN ("fmops", 0x80800010, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMSS, 0, 0),
  SME_F64F64_INSN ("fmops", 0x80c00010, 0xffe00018, sme_misc, 0, OP5 (SME_ZAda_3b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_DMMDD, 0, 0),
  SME_INSN ("fmops", 0x81a00010, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMHH, 0, 0),
  SME_INSN ("rdsvl", 0x04bf5800, 0xfffff800, sme_misc, 0, OP2 (Rd, SVE_SIMM6), OP_SVE_XU, 0, 0),
  SME_INSN ("smopa", 0xa0800000, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMBB, 0, 0),
  SME_I16I64_INSN ("smopa", 0xa0c00000, 0xffe00018, sme_misc, 0, OP5 (SME_ZAda_3b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_DMMHH, 0, 0),
  SME_INSN ("smops", 0xa0800010, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMBB, 0, 0),
  SME_I16I64_INSN ("smops", 0xa0c00010, 0xffe00018, sme_misc, 0, OP5 (SME_ZAda_3b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_DMMHH, 0, 0),
  SME_INSN ("sumopa", 0xa0a00000, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMBB, 0, 0),
  SME_I16I64_INSN ("sumopa", 0xa0e00000, 0xffe00018, sme_misc, 0, OP5 (SME_ZAda_3b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_DMMHH, 0, 0),
  SME_INSN ("sumops", 0xa0a00010 ,0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMBB, 0, 0),
  SME_I16I64_INSN ("sumops", 0xa0e00010 ,0xffe00018, sme_misc, 0, OP5 (SME_ZAda_3b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_DMMHH, 0, 0),
  SME_INSN ("umopa", 0xa1a00000, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMBB, 0, 0),
  SME_I16I64_INSN ("umopa", 0xa1e00000, 0xffe00018, sme_misc, 0, OP5 (SME_ZAda_3b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_DMMHH, 0, 0),
  SME_INSN ("umops", 0xa1a00010 ,0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMBB, 0, 0),
  SME_I16I64_INSN ("umops", 0xa1e00010 ,0xffe00018, sme_misc, 0, OP5 (SME_ZAda_3b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_DMMHH, 0, 0),
  SME_INSN ("usmopa", 0xa1800000, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMBB, 0, 0),
  SME_I16I64_INSN ("usmopa", 0xa1c00000, 0xffe00018, sme_misc, 0, OP5 (SME_ZAda_3b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_DMMHH, 0, 0),
  SME_INSN ("usmops", 0xa1800010, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMBB, 0, 0),
  SME_I16I64_INSN ("usmops", 0xa1c00010, 0xffe00018, sme_misc, 0, OP5 (SME_ZAda_3b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_DMMHH, 0, 0),

  SME_INSN ("mov", 0xc0020000, 0xff3e0200, sme_mov, 0, OP3 (SVE_Zd, SVE_Pg3, SME_ZA_HV_idx_src), OP_SVE_VMV_BHSDQ, 0, 0),
  SME_INSN ("mov", 0xc0000000, 0xff3e0010, sme_mov, 0, OP3 (SME_ZA_HV_idx_dest, SVE_Pg3, SVE_Zn), OP_SVE_VMV_BHSDQ, 0, 0),
  SME_INSN ("mova", 0xc0020000, 0xff3e0200, sme_mov, 0, OP3 (SVE_Zd, SVE_Pg3, SME_ZA_HV_idx_src), OP_SVE_VMV_BHSDQ, 0, 0),
  SME_INSN ("mova", 0xc0000000, 0xff3e0010, sme_mov, 0, OP3 (SME_ZA_HV_idx_dest, SVE_Pg3, SVE_Zn), OP_SVE_VMV_BHSDQ, 0, 0),

  SME_INSN ("zero", 0xc0080000, 0xffffff00, sme_misc, 0, OP1 (SME_list_of_64bit_tiles), {}, 0, 0),

  SME_INSN ("ld1b", 0xe0000000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_RR), OP_SVE_BZU, 0, 0),
  SME_INSN ("ld1h", 0xe0400000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_RR_LSL1), OP_SVE_HZU, 0, 0),
  SME_INSN ("ld1w", 0xe0800000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_RR_LSL2), OP_SVE_SZU, 0, 0),
  SME_INSN ("ld1d", 0xe0c00000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_RR_LSL3), OP_SVE_DZU, 0, 0),
  SME_INSN ("ld1q", 0xe1c00000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_RR_LSL4), OP_SVE_QZU, 0, 0),

  SME_INSN ("ld1b", 0xe0000000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_R), OP_SVE_BZU, 0, 0),
  SME_INSN ("ld1h", 0xe0400000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_R), OP_SVE_HZU, 0, 0),
  SME_INSN ("ld1w", 0xe0800000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_R), OP_SVE_SZU, 0, 0),
  SME_INSN ("ld1d", 0xe0c00000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_R), OP_SVE_DZU, 0, 0),
  SME_INSN ("ld1q", 0xe1c00000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_R), OP_SVE_QZU, 0, 0),

  SME_INSN ("st1b", 0xe0200000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_RR), OP_SVE_BUU, 0, 0),
  SME_INSN ("st1h", 0xe0600000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_RR_LSL1), OP_SVE_HUU, 0, 0),
  SME_INSN ("st1w", 0xe0a00000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_RR_LSL2), OP_SVE_SUU, 0, 0),
  SME_INSN ("st1d", 0xe0e00000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_RR_LSL3), OP_SVE_DUU, 0, 0),
  SME_INSN ("st1q", 0xe1e00000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_RR_LSL4), OP_SVE_QUU, 0, 0),

  SME_INSN ("st1b", 0xe0200000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_R), OP_SVE_BUU, 0, 0),
  SME_INSN ("st1h", 0xe0600000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_R), OP_SVE_HUU, 0, 0),
  SME_INSN ("st1w", 0xe0a00000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_R), OP_SVE_SUU, 0, 0),
  SME_INSN ("st1d", 0xe0e00000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_R), OP_SVE_DUU, 0, 0),
  SME_INSN ("st1q", 0xe1e00000, 0xffe00010, sve_misc, 0, OP3 (SME_ZA_HV_idx_ldstr, SVE_Pg3, SVE_ADDR_R), OP_SVE_QUU, 0, 0),

  SME_INSN ("ldr", 0xe1000000, 0xffff9c10, sme_ldr, 0, OP2 (SME_ZA_array_off4, SME_ADDR_RI_U4xVL), {}, 0, 1),
  SME_INSN ("str", 0xe1200000, 0xffff9c10, sme_str, 0, OP2 (SME_ZA_array_off4, SME_ADDR_RI_U4xVL), {}, 0, 1),

  SME_INSNC ("revd", 0x52e8000, 0xffffe000, sme_misc, 0, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_QMQ, 0, C_SCAN_MOVPRFX, 0),
  SME_INSNC ("sclamp", 0x4400c000, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SME_INSNC ("uclamp", 0x4400c400, 0xff20fc00, sve_size_bhsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, C_SCAN_MOVPRFX, 0),
  SME_INSN ("psel", 0x25204000, 0xff20c210, sme_psel, 0, OP3 (SVE_Pd, SVE_Pg4_10, SME_PnT_Wm_imm), OP_SVE_NN_BHSD, 0, 0),
  SME_INSN ("psel", 0x25204000, 0xff20c210, sme_psel, 0, OP3 (SVE_PNd, SVE_PNg4_10, SME_PnT_Wm_imm), OP_SVE_NN_BHSD, 0, 0),

  /* Added in SME2, but part of the prefetch hint space and available
     without special command-line flags.  */
  CORE_INSN ("rprfm", 0xf8a04818, 0xffe04c18, sme_misc, 0, OP3 (RPRFMOP, Rm, SIMD_ADDR_SIMPLE), OP_SVE_UXU, 0),

  /* SME2 extensions to SVE2.  */
  SME2_INSNC ("bfmlslb", 0x64e06000, 0xffe0f400, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SME2_INSNC ("bfmlslb", 0x64e0a000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SME2_INSNC ("bfmlslt", 0x64e06400, 0xffe0f400, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SME2_INSNC ("bfmlslt", 0x64e0a400, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SME2_INSNC ("fdot", 0x64204000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_19_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SME2_INSNC ("fdot", 0x64208000, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SME2_INSNC ("fclamp", 0x64202400, 0xff20fc00, sme_size_22_hsd, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD, 0, C_SCAN_MOVPRFX, 0),
  SME2_INSNC ("sdot", 0x4480c800, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_19_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SME2_INSNC ("sdot", 0x4400c800, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SME2_INSN ("sqcvtn", 0x45314000, 0xfffffc20, sve_misc, 0, OP2 (SVE_Zd, SME_Znx2), OP_SVE_HS, 0, 0),
  SME2_INSN ("sqcvtun", 0x45315000, 0xfffffc20, sve_misc, 0, OP2 (SVE_Zd, SME_Znx2), OP_SVE_HS, 0, 0),
  SME2_INSN ("sqrshrn", 0x45b02800, 0xfff0fc20, sme_misc, 0, OP3 (SVE_Zd, SME_Znx2, SME_SHRIMM4), OP_SVE_HSU, 0, 0),
  SME2_INSN ("sqrshrun", 0x45b00800, 0xfff0fc20, sme_misc, 0, OP3 (SVE_Zd, SME_Znx2, SME_SHRIMM4), OP_SVE_HSU, 0, 0),
  SME2_INSNC ("udot", 0x4480cc00, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_19_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SME2_INSNC ("udot", 0x4400cc00, 0xffe0fc00, sve_misc, 0, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  SME2_INSN ("uqcvtn", 0x45314800, 0xfffffc20, sve_misc, 0, OP2 (SVE_Zd, SME_Znx2), OP_SVE_HS, 0, 0),
  SME2_INSN ("uqrshrn", 0x45b03800, 0xfff0fc20, sme_misc, 0, OP3 (SVE_Zd, SME_Znx2, SME_SHRIMM4), OP_SVE_HSU, 0, 0),
  SME2_INSN ("whilege", 0x25205010, 0xff20fc11, sme_size_22, 0, OP3 (SME_Pdx2, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  SME2_INSN ("whilegt", 0x25205011, 0xff20fc11, sme_size_22, 0, OP3 (SME_Pdx2, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  SME2_INSN ("whilehi", 0x25205811, 0xff20fc11, sme_size_22, 0, OP3 (SME_Pdx2, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  SME2_INSN ("whilehs", 0x25205810, 0xff20fc11, sme_size_22, 0, OP3 (SME_Pdx2, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  SME2_INSN ("whilele", 0x25205411, 0xff20fc11, sme_size_22, 0, OP3 (SME_Pdx2, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  SME2_INSN ("whilelo", 0x25205c10, 0xff20fc11, sme_size_22, 0, OP3 (SME_Pdx2, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  SME2_INSN ("whilels", 0x25205c11, 0xff20fc11, sme_size_22, 0, OP3 (SME_Pdx2, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),
  SME2_INSN ("whilelt", 0x25205410, 0xff20fc11, sme_size_22, 0, OP3 (SME_Pdx2, Rn, Rm), OP_SVE_VXX_BHSD, 0, 0),

  /* SME2 extensions to SME.  */
  SME2_INSN ("add", 0xc1a01c10, 0xffbf9c38, sme_int_sd, 0, OP2 (SME_ZA_array_off3_0, SME_Znx2), OP_SVE_VVV_SD, F_OD (2), 0),
  SME2_INSN ("add", 0xc1a11c10, 0xffbf9c78, sme_int_sd, 0, OP2 (SME_ZA_array_off3_0, SME_Znx4), OP_SVE_VVV_SD, F_OD (4), 0),
  SME2_INSN ("add", 0xc1201810, 0xffb09c18, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD, F_OD (2), 0),
  SME2_INSN ("add", 0xc1301810, 0xffb09c18, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD, F_OD (4), 0),
  SME2_INSN ("add", 0xc1a01810, 0xffa19c38, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zmx2), OP_SVE_VVV_SD, F_OD (2), 0),
  SME2_INSN ("add", 0xc1a11810, 0xffa39c78, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zmx4), OP_SVE_VVV_SD, F_OD (4), 0),
  SME2_INSN ("add", 0xc120a300, 0xff30ffe1, sme_size_22, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("add", 0xc120ab00, 0xff30ffe3, sme_size_22, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("bfcvt", 0xc160e000, 0xfffffc20, sme_misc, 0, OP2 (SVE_Zd, SME_Znx2), OP_SVE_HS, 0, 0),
  SME2_INSN ("bfcvtn", 0xc160e020, 0xfffffc20, sme_misc, 0, OP2 (SVE_Zd, SME_Znx2), OP_SVE_HS, 0, 0),
  SME2_INSN ("bfdot", 0xc1501018, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("bfdot", 0xc1509018, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX2), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("bfdot", 0xc1201010, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("bfdot", 0xc1301010, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("bfdot", 0xc1a01010, 0xffe19c38, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zmx2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("bfdot", 0xc1a11010, 0xffe39c78, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zmx4), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("bfmlal", 0xc1801010, 0xfff01018, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm_INDEX3_10), OP_SVE_SHH, 0, 0),
  SME2_INSN ("bfmlal", 0xc1901010, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("bfmlal", 0xc1909010, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("bfmlal", 0xc1200c10, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm), OP_SVE_SHH, 0, 0),
  SME2_INSN ("bfmlal", 0xc1200810, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("bfmlal", 0xc1300810, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("bfmlal", 0xc1a00810, 0xffe19c3c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zmx2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("bfmlal", 0xc1a10810, 0xffe39c7c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zmx4), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("bfmlsl", 0xc1801018, 0xfff01018, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm_INDEX3_10), OP_SVE_SHH, 0, 0),
  SME2_INSN ("bfmlsl", 0xc1901018, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("bfmlsl", 0xc1909018, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("bfmlsl", 0xc1200c18, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm), OP_SVE_SHH, 0, 0),
  SME2_INSN ("bfmlsl", 0xc1200818, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("bfmlsl", 0xc1300818, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("bfmlsl", 0xc1a00818, 0xffe19c3c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zmx2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("bfmlsl", 0xc1a10818, 0xffe39c7c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zmx4), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("bfvdot", 0xc1500018, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("bmopa", 0x80800008, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMSS, 0, 0),
  SME2_INSN ("bmops", 0x80800018, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMSS, 0, 0),
  SME2_INSN ("cntp", 0x25208200, 0xff3ffa00, sme_size_22, 0, OP3 (Rd, SME_PNn, SME_VLxN_10), OP_SVE_XV_BHSD, 0, 0),
  SME2_INSN ("fadd", 0xc1a01c00, 0xffbf9c38, sme_fp_sd, 0, OP2 (SME_ZA_array_off3_0, SME_Znx2), OP_SVE_VVV_SD, F_OD (2), 0),
  SME2_INSN ("fadd", 0xc1a11c00, 0xffbf9c78, sme_fp_sd, 0, OP2 (SME_ZA_array_off3_0, SME_Znx4), OP_SVE_VVV_SD, F_OD (4), 0),
  SME2_INSN ("fclamp", 0xc120c000, 0xff20fc01, sme_size_22_hsd, 0, OP3 (SME_Zdnx2, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD, 0, 0),
  SME2_INSN ("fclamp", 0xc120c800, 0xff20fc03, sme_size_22_hsd, 0, OP3 (SME_Zdnx4, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_HSD, 0, 0),
  SME2_INSN ("fcvt", 0xc120e000, 0xfffffc20, sve_misc, 0, OP2 (SVE_Zd, SME_Znx2), OP_SVE_HS, 0, 0),
  SME2_INSN ("fcvtn", 0xc120e020, 0xfffffc20, sve_misc, 0, OP2 (SVE_Zd, SME_Znx2), OP_SVE_HS, 0, 0),
  SME2_INSN ("fcvtzs", 0xc121e000, 0xfffffc21, sve_misc, 0, OP2 (SME_Zdnx2, SME_Znx2), OP_SVE_SS, 0, 0),
  SME2_INSN ("fcvtzs", 0xc131e000, 0xfffffc63, sve_misc, 0, OP2 (SME_Zdnx4, SME_Znx4), OP_SVE_SS, 0, 0),
  SME2_INSN ("fcvtzu", 0xc121e020, 0xfffffc21, sve_misc, 0, OP2 (SME_Zdnx2, SME_Znx2), OP_SVE_SS, 0, 0),
  SME2_INSN ("fcvtzu", 0xc131e020, 0xfffffc63, sve_misc, 0, OP2 (SME_Zdnx4, SME_Znx4), OP_SVE_SS, 0, 0),
  SME2_INSN ("fdot", 0xc1501008, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("fdot", 0xc1509008, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX2), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("fdot", 0xc1201000, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("fdot", 0xc1301000, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("fdot", 0xc1a01000, 0xffe19c38, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zmx2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("fdot", 0xc1a11000, 0xffe39c78, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zmx4), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("fmax", 0xc120a100, 0xff30ffe1, sme_size_22_hsd, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zm), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fmax", 0xc120a900, 0xff30ffe3, sme_size_22_hsd, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zm), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fmax", 0xc120b100, 0xff21ffe1, sme_size_22_hsd, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zmx2), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fmax", 0xc120b900, 0xff23ffe3, sme_size_22_hsd, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zmx4), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fmaxnm", 0xc120a120, 0xff30ffe1, sme_size_22_hsd, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zm), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fmaxnm", 0xc120a920, 0xff30ffe3, sme_size_22_hsd, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zm), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fmaxnm", 0xc120b120, 0xff21ffe1, sme_size_22_hsd, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zmx2), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fmaxnm", 0xc120b920, 0xff23ffe3, sme_size_22_hsd, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zmx4), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fmin", 0xc120a101, 0xff30ffe1, sme_size_22_hsd, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zm), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fmin", 0xc120a901, 0xff30ffe3, sme_size_22_hsd, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zm), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fmin", 0xc120b101, 0xff21ffe1, sme_size_22_hsd, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zmx2), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fmin", 0xc120b901, 0xff23ffe3, sme_size_22_hsd, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zmx4), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fminnm", 0xc120a121, 0xff30ffe1, sme_size_22_hsd, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zm), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fminnm", 0xc120a921, 0xff30ffe3, sme_size_22_hsd, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zm), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fminnm", 0xc120b121, 0xff21ffe1, sme_size_22_hsd, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zmx2), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fminnm", 0xc120b921, 0xff23ffe3, sme_size_22_hsd, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zmx4), OP_SVE_VVV_HSD, 0, 1),
  SME2_INSN ("fmla", 0xc1500000, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX2), OP_SVE_SSS, F_OD (2), 0),
  SME2_INSN ("fmla", 0xc1508000, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX2), OP_SVE_SSS, F_OD (4), 0),
  SME2_INSN ("fmla", 0xc1201800, 0xffb09c18, sme_fp_sd, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD, F_OD (2), 0),
  SME2_INSN ("fmla", 0xc1301800, 0xffb09c18, sme_fp_sd, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD, F_OD (4), 0),
  SME2_INSN ("fmla", 0xc1a01800, 0xffa19c38, sme_fp_sd, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zmx2), OP_SVE_VVV_SD, F_OD (2), 0),
  SME2_INSN ("fmla", 0xc1a11800, 0xffa39c78, sme_fp_sd, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zmx4), OP_SVE_VVV_SD, F_OD (4), 0),
  SME2_INSN ("fmlal", 0xc1801000, 0xfff01018, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm_INDEX3_10), OP_SVE_SHH, 0, 0),
  SME2_INSN ("fmlal", 0xc1901000, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("fmlal", 0xc1909000, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("fmlal", 0xc1200c00, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm), OP_SVE_SHH, 0, 0),
  SME2_INSN ("fmlal", 0xc1200800, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("fmlal", 0xc1300800, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("fmlal", 0xc1a00800, 0xffe19c3c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zmx2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("fmlal", 0xc1a10800, 0xffe39c7c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zmx4), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("fmls", 0xc1500010, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX2), OP_SVE_SSS, F_OD (2), 0),
  SME2_INSN ("fmls", 0xc1508010, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX2), OP_SVE_SSS, F_OD (4), 0),
  SME2_INSN ("fmls", 0xc1201808, 0xffb09c18, sme_fp_sd, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD, F_OD (2), 0),
  SME2_INSN ("fmls", 0xc1301808, 0xffb09c18, sme_fp_sd, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD, F_OD (4), 0),
  SME2_INSN ("fmls", 0xc1a01808, 0xffa19c38, sme_fp_sd, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zmx2), OP_SVE_VVV_SD, F_OD (2), 0),
  SME2_INSN ("fmls", 0xc1a11808, 0xffa39c78, sme_fp_sd, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zmx4), OP_SVE_VVV_SD, F_OD (4), 0),
  SME2_INSN ("fmlsl", 0xc1801008, 0xfff01018, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm_INDEX3_10), OP_SVE_SHH, 0, 0),
  SME2_INSN ("fmlsl", 0xc1901008, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("fmlsl", 0xc1909008, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("fmlsl", 0xc1200c08, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm), OP_SVE_SHH, 0, 0),
  SME2_INSN ("fmlsl", 0xc1200808, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("fmlsl", 0xc1300808, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("fmlsl", 0xc1a00808, 0xffe19c3c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zmx2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("fmlsl", 0xc1a10808, 0xffe39c7c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zmx4), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("frinta", 0xc1ace000, 0xfffffc21, sve_misc, 0, OP2 (SME_Zdnx2, SME_Znx2), OP_SVE_SS, 0, 0),
  SME2_INSN ("frinta", 0xc1bce000, 0xfffffc63, sve_misc, 0, OP2 (SME_Zdnx4, SME_Znx4), OP_SVE_SS, 0, 0),
  SME2_INSN ("frintm", 0xc1aae000, 0xfffffc21, sve_misc, 0, OP2 (SME_Zdnx2, SME_Znx2), OP_SVE_SS, 0, 0),
  SME2_INSN ("frintm", 0xc1bae000, 0xfffffc63, sve_misc, 0, OP2 (SME_Zdnx4, SME_Znx4), OP_SVE_SS, 0, 0),
  SME2_INSN ("frintn", 0xc1a8e000, 0xfffffc21, sve_misc, 0, OP2 (SME_Zdnx2, SME_Znx2), OP_SVE_SS, 0, 0),
  SME2_INSN ("frintn", 0xc1b8e000, 0xfffffc63, sve_misc, 0, OP2 (SME_Zdnx4, SME_Znx4), OP_SVE_SS, 0, 0),
  SME2_INSN ("frintp", 0xc1a9e000, 0xfffffc21, sve_misc, 0, OP2 (SME_Zdnx2, SME_Znx2), OP_SVE_SS, 0, 0),
  SME2_INSN ("frintp", 0xc1b9e000, 0xfffffc63, sve_misc, 0, OP2 (SME_Zdnx4, SME_Znx4), OP_SVE_SS, 0, 0),
  SME2_INSN ("fsub", 0xc1a01c08, 0xffbf9c38, sme_fp_sd, 0, OP2 (SME_ZA_array_off3_0, SME_Znx2), OP_SVE_VVV_SD, F_OD (2), 0),
  SME2_INSN ("fsub", 0xc1a11c08, 0xffbf9c78, sme_fp_sd, 0, OP2 (SME_ZA_array_off3_0, SME_Znx4), OP_SVE_VVV_SD, F_OD (4), 0),
  SME2_INSN ("fvdot", 0xc1500008, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("ld1b", 0xa0400000, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ld1b", 0xa0408000, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ld1b", 0xa1400000, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ld1b", 0xa1408000, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ld1b", 0xa0000000, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ld1b", 0xa0008000, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ld1b", 0xa1000000, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ld1b", 0xa1008000, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ld1d", 0xa0406000, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ld1d", 0xa040e000, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ld1d", 0xa1406000, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ld1d", 0xa140e000, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ld1d", 0xa0006000, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ld1d", 0xa000e000, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ld1d", 0xa1006000, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ld1d", 0xa100e000, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ld1h", 0xa0402000, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ld1h", 0xa040a000, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ld1h", 0xa1402000, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ld1h", 0xa140a000, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ld1h", 0xa0002000, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ld1h", 0xa000a000, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ld1h", 0xa1002000, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ld1h", 0xa100a000, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ld1w", 0xa0404000, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ld1w", 0xa040c000, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ld1w", 0xa1404000, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ld1w", 0xa140c000, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ld1w", 0xa0004000, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ld1w", 0xa000c000, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ld1w", 0xa1004000, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ld1w", 0xa100c000, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ldnt1b", 0xa0400001, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ldnt1b", 0xa0408001, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ldnt1b", 0xa1400008, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ldnt1b", 0xa1408008, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ldnt1b", 0xa0000001, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ldnt1b", 0xa0008001, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ldnt1b", 0xa1000008, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ldnt1b", 0xa1008008, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR), OP_SVE_BZU, 0, 0),
  SME2_INSN ("ldnt1d", 0xa0406001, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ldnt1d", 0xa040e001, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ldnt1d", 0xa1406008, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ldnt1d", 0xa140e008, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ldnt1d", 0xa0006001, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ldnt1d", 0xa000e001, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ldnt1d", 0xa1006008, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ldnt1d", 0xa100e008, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DZU, 0, 0),
  SME2_INSN ("ldnt1h", 0xa0402001, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ldnt1h", 0xa040a001, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ldnt1h", 0xa1402008, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ldnt1h", 0xa140a008, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ldnt1h", 0xa0002001, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ldnt1h", 0xa000a001, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ldnt1h", 0xa1002008, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ldnt1h", 0xa100a008, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HZU, 0, 0),
  SME2_INSN ("ldnt1w", 0xa0404001, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ldnt1w", 0xa040c001, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ldnt1w", 0xa1404008, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ldnt1w", 0xa140c008, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ldnt1w", 0xa0004001, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ldnt1w", 0xa000c001, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ldnt1w", 0xa1004008, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ldnt1w", 0xa100c008, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SZU, 0, 0),
  SME2_INSN ("ldr", 0xe11f8000, 0xfffffc1f, sme_misc, 0, OP2 (SME_ZT0, SIMD_ADDR_SIMPLE), {}, 0, 0),
  SME2_INSN ("luti2", 0xc0cc0000, 0xfffc0c00, sme_size_12_bhs, 0, OP3 (SVE_Zd, SME_ZT0, SME_Zn_INDEX4_14), OP_SVE_VUU_BHS, 0, 0),
  SME2_INSN ("luti2", 0xc08c4000, 0xfffc4c01, sme_size_12_bhs, 0, OP3 (SME_Zdnx2, SME_ZT0, SME_Zn_INDEX3_15), OP_SVE_VUU_BHS, 0, 0),
  SME2_INSN ("luti2", 0xc08c8000, 0xfffccc03, sme_size_12_bhs, 0, OP3 (SME_Zdnx4, SME_ZT0, SME_Zn_INDEX2_16), OP_SVE_VUU_BHS, 0, 0),
  SME2_INSN ("luti4", 0xc0ca0000, 0xfffe0c00, sme_size_12_bhs, 0, OP3 (SVE_Zd, SME_ZT0, SME_Zn_INDEX3_14), OP_SVE_VUU_BHS, 0, 0),
  SME2_INSN ("luti4", 0xc08a4000, 0xfffe4c01, sme_size_12_bhs, 0, OP3 (SME_Zdnx2, SME_ZT0, SME_Zn_INDEX2_15), OP_SVE_VUU_BHS, 0, 0),
  SME2_INSN ("luti4", 0xc08a8000, 0xfffecc03, sme_size_12_hs, 0, OP3 (SME_Zdnx4, SME_ZT0, SME_Zn_INDEX1_16), OP_SVE_VUU_HS, 0, 0),
  SME2_INSN ("mov", 0xc0060800, 0xffff9f01, sme2_mov, 0, OP2 (SME_Zdnx2, SME_ZA_array_off3_5), OP_SVE_VV_BHSD, F_OD (2), 0),
  SME2_INSN ("mov", 0xc0060c00, 0xffff9f03, sme2_mov, 0, OP2 (SME_Zdnx4, SME_ZA_array_off3_5), OP_SVE_VV_BHSD, F_OD (4), 0),
  SME2_INSN ("mov", 0xc0060000, 0xff3f1f01, sme_size_22, 0, OP2 (SME_Zdnx2, SME_ZA_HV_idx_srcxN), OP_SVE_VV_BHSDQ, F_OD (2), 0),
  SME2_INSN ("mov", 0xc0060400, 0xff3f1f03, sme_size_22, 0, OP2 (SME_Zdnx4, SME_ZA_HV_idx_srcxN), OP_SVE_VV_BHSDQ, F_OD (4), 0),
  SME2_INSN ("mov", 0xc0040800, 0xffff9c38, sme2_mov, 0, OP2 (SME_ZA_array_off3_0, SME_Znx2), OP_SVE_VV_BHSD, F_OD (2), 0),
  SME2_INSN ("mov", 0xc0040c00, 0xffff9c78, sme2_mov, 0, OP2 (SME_ZA_array_off3_0, SME_Znx4), OP_SVE_VV_BHSD, F_OD (4), 0),
  SME2_INSN ("mov", 0xc0040000, 0xff3f1c38, sme_size_22, 0, OP2 (SME_ZA_HV_idx_destxN, SME_Znx2), OP_SVE_VV_BHSDQ, F_OD (2), 0),
  SME2_INSN ("mov", 0xc0040400, 0xff3f1c78, sme_size_22, 0, OP2 (SME_ZA_HV_idx_destxN, SME_Znx4), OP_SVE_VV_BHSDQ, F_OD (4), 0),
  SME2_INSN ("mova", 0xc0060800, 0xffff9f01, sme2_mov, 0, OP2 (SME_Zdnx2, SME_ZA_array_off3_5), OP_SVE_VV_BHSD, F_OD (2), 0),
  SME2_INSN ("mova", 0xc0060c00, 0xffff9f03, sme2_mov, 0, OP2 (SME_Zdnx4, SME_ZA_array_off3_5), OP_SVE_VV_BHSD, F_OD (4), 0),
  SME2_INSN ("mova", 0xc0060000, 0xff3f1f01, sme_size_22, 0, OP2 (SME_Zdnx2, SME_ZA_HV_idx_srcxN), OP_SVE_VV_BHSDQ, F_OD (2), 0),
  SME2_INSN ("mova", 0xc0060400, 0xff3f1f03, sme_size_22, 0, OP2 (SME_Zdnx4, SME_ZA_HV_idx_srcxN), OP_SVE_VV_BHSDQ, F_OD (4), 0),
  SME2_INSN ("mova", 0xc0040800, 0xffff9c38, sme2_mov, 0, OP2 (SME_ZA_array_off3_0, SME_Znx2), OP_SVE_VV_BHSD, F_OD (2), 0),
  SME2_INSN ("mova", 0xc0040c00, 0xffff9c78, sme2_mov, 0, OP2 (SME_ZA_array_off3_0, SME_Znx4), OP_SVE_VV_BHSD, F_OD (4), 0),
  SME2_INSN ("mova", 0xc0040000, 0xff3f1c38, sme_size_22, 0, OP2 (SME_ZA_HV_idx_destxN, SME_Znx2), OP_SVE_VV_BHSDQ, F_OD (2), 0),
  SME2_INSN ("mova", 0xc0040400, 0xff3f1c78, sme_size_22, 0, OP2 (SME_ZA_HV_idx_destxN, SME_Znx4), OP_SVE_VV_BHSDQ, F_OD (4), 0),
  SME2_INSN ("movt", 0xc04e03e0, 0xffff8fe0, sme_misc, 0, OP2 (SME_ZT0_INDEX, Rt), OP_SVE_UX, 0, 0),
  SME2_INSN ("movt", 0xc04c03e0, 0xffff8fe0, sme_misc, 0, OP2 (Rt, SME_ZT0_INDEX), OP_SVE_XU, 0, 0),
  SME2_INSN ("pext", 0x25207010, 0xff3ffc10, sme_size_22, 0, OP2 (SVE_Pd, SME_PNn3_INDEX2), OP_SVE_VU_BHSD, 0, 0),
  SME2_INSN ("pext", 0x25207410, 0xff3ffe10, sme_size_22, 0, OP2 (SME_PdxN, SME_PNn3_INDEX1), OP_SVE_VU_BHSD, F_OD (2), 0),
  SME2_INSN ("ptrue", 0x25207810, 0xff3ffff8, sme_size_22, 0, OP1 (SME_PNd3), OP_SVE_V_BHSD, 0, 0),
  SME2_INSN ("sclamp", 0xc120c400, 0xff20fc01, sme_size_22, 0, OP3 (SME_Zdnx2, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  SME2_INSN ("sclamp", 0xc120cc00, 0xff20fc03, sme_size_22, 0, OP3 (SME_Zdnx4, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  SME2_INSN ("scvtf", 0xc122e000, 0xfffffc21, sve_misc, 0, OP2 (SME_Zdnx2, SME_Znx2), OP_SVE_SS, 0, 0),
  SME2_INSN ("scvtf", 0xc132e000, 0xfffffc63, sve_misc, 0, OP2 (SME_Zdnx4, SME_Znx4), OP_SVE_SS, 0, 0),
  SME2_INSN ("sdot", 0xc1501000, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("sdot", 0xc1509000, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX2), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("sdot", 0xc1601408, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("sdot", 0xc1701408, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("sdot", 0xc1e01408, 0xffe19c38, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zmx2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("sdot", 0xc1e11408, 0xffe39c78, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zmx4), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("sdot", 0xc1501020, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX2), OP_SVE_SBB, F_OD (2), 0),
  SME2_INSN ("sdot", 0xc1509020, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX2), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("sdot", 0xc1201400, 0xffb09c18, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD_BH, F_OD (2), 0),
  SME2_INSN ("sdot", 0xc1301400, 0xffb09c18, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD_BH, F_OD (4), 0),
  SME2_INSN ("sdot", 0xc1a01400, 0xffa19c38, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zmx2), OP_SVE_VVV_SD_BH, F_OD (2), 0),
  SME2_INSN ("sdot", 0xc1a11400, 0xffa39c78, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zmx4), OP_SVE_VVV_SD_BH, F_OD (4), 0),
  SME2_INSN ("sel", 0xc1208000, 0xff21e021, sme_size_22, 0, OP4 (SME_Zdnx2, SME_PNg3, SME_Znx2, SME_Zmx2), OP_SVE_VUVV_BHSD, 0, 0),
  SME2_INSN ("sel", 0xc1218000, 0xff23e063, sme_size_22, 0, OP4 (SME_Zdnx4, SME_PNg3, SME_Znx4, SME_Zmx4), OP_SVE_VUVV_BHSD, 0, 0),
  SME2_INSN ("smax", 0xc120a000, 0xff30ffe1, sme_size_22, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("smax", 0xc120a800, 0xff30ffe3, sme_size_22, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("smax", 0xc120b000, 0xff21ffe1, sme_size_22, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zmx2), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("smax", 0xc120b800, 0xff23ffe3, sme_size_22, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zmx4), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("smin", 0xc120a020, 0xff30ffe1, sme_size_22, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("smin", 0xc120a820, 0xff30ffe3, sme_size_22, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("smin", 0xc120b020, 0xff21ffe1, sme_size_22, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zmx2), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("smin", 0xc120b820, 0xff23ffe3, sme_size_22, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zmx4), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("smlal", 0xc1c01000, 0xfff01018, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm_INDEX3_10), OP_SVE_SHH, 0, 0),
  SME2_INSN ("smlal", 0xc1d01000, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("smlal", 0xc1d09000, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("smlal", 0xc1600c00, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm), OP_SVE_SHH, 0, 0),
  SME2_INSN ("smlal", 0xc1600800, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("smlal", 0xc1700800, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("smlal", 0xc1e00800, 0xffe19c3c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zmx2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("smlal", 0xc1e10800, 0xffe39c7c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zmx4), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("smlall", 0xc1000000, 0xfff0001c, sme_misc, 0, OP3 (SME_ZA_array_off2x4, SVE_Zn, SME_Zm_INDEX4_10), OP_SVE_SBB, 0, 0),
  SME2_INSN ("smlall", 0xc1100000, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx2, SME_Zm_INDEX4_1), OP_SVE_SBB, F_OD (2), 0),
  SME2_INSN ("smlall", 0xc1108000, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx4, SME_Zm_INDEX4_1), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("smlall", 0xc1200400, 0xffb09c1c, sme_int_sd, 0, OP3 (SME_ZA_array_off2x4, SVE_Zn, SME_Zm), OP_SVE_VVV_SD_BH, 0, 0),
  SME2_INSN ("smlall", 0xc1200000, 0xffb09c1e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD_BH, F_OD (2), 0),
  SME2_INSN ("smlall", 0xc1300000, 0xffb09c1e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD_BH, F_OD (4), 0),
  SME2_INSN ("smlall", 0xc1a00000, 0xffa19c3e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SME_Znx2, SME_Zmx2), OP_SVE_VVV_SD_BH, F_OD (2), 0),
  SME2_INSN ("smlall", 0xc1a10000, 0xffa39c7e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SME_Znx4, SME_Zmx4), OP_SVE_VVV_SD_BH, F_OD (4), 0),
  SME2_INSN ("smlsl", 0xc1c01008, 0xfff01018, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm_INDEX3_10), OP_SVE_SHH, 0, 0),
  SME2_INSN ("smlsl", 0xc1d01008, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("smlsl", 0xc1d09008, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("smlsl", 0xc1600c08, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm), OP_SVE_SHH, 0, 0),
  SME2_INSN ("smlsl", 0xc1600808, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("smlsl", 0xc1700808, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("smlsl", 0xc1e00808, 0xffe19c3c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zmx2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("smlsl", 0xc1e10808, 0xffe39c7c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zmx4), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("smlsll", 0xc1000008, 0xfff0001c, sme_misc, 0, OP3 (SME_ZA_array_off2x4, SVE_Zn, SME_Zm_INDEX4_10), OP_SVE_SBB, 0, 0),
  SME2_INSN ("smlsll", 0xc1100008, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx2, SME_Zm_INDEX4_1), OP_SVE_SBB, F_OD (2), 0),
  SME2_INSN ("smlsll", 0xc1108008, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx4, SME_Zm_INDEX4_1), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("smlsll", 0xc1200408, 0xffb09c1c, sme_int_sd, 0, OP3 (SME_ZA_array_off2x4, SVE_Zn, SME_Zm), OP_SVE_VVV_SD_BH, 0, 0),
  SME2_INSN ("smlsll", 0xc1200008, 0xffb09c1e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD_BH, F_OD (2), 0),
  SME2_INSN ("smlsll", 0xc1300008, 0xffb09c1e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD_BH, F_OD (4), 0),
  SME2_INSN ("smlsll", 0xc1a00008, 0xffa19c3e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SME_Znx2, SME_Zmx2), OP_SVE_VVV_SD_BH, F_OD (2), 0),
  SME2_INSN ("smlsll", 0xc1a10008, 0xffa39c7e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SME_Znx4, SME_Zmx4), OP_SVE_VVV_SD_BH, F_OD (4), 0),
  SME2_INSN ("smopa", 0xa0800008, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMHH, 0, 0),
  SME2_INSN ("smops", 0xa0800018, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMHH, 0, 0),
  SME2_INSN ("sqcvt", 0xc123e000, 0xfffffc20, sme_misc, 0, OP2 (SVE_Zd, SME_Znx2), OP_SVE_HS, 0, 0),
  SME2_INSN ("sqcvt", 0xc133e000, 0xff7ffc60, sme_sz_23, 0, OP2 (SVE_Zd, SME_Znx4), OP_SVE_VV_BH_SD, 0, 0),
  SME2_INSN ("sqcvtn", 0xc133e040, 0xff7ffc60, sme_sz_23, 0, OP2 (SVE_Zd, SME_Znx4), OP_SVE_VV_BH_SD, 0, 0),
  SME2_INSN ("sqcvtu", 0xc163e000, 0xfffffc20, sme_misc, 0, OP2 (SVE_Zd, SME_Znx2), OP_SVE_HS, 0, 0),
  SME2_INSN ("sqcvtu", 0xc173e000, 0xff7ffc60, sme_sz_23, 0, OP2 (SVE_Zd, SME_Znx4), OP_SVE_VV_BH_SD, 0, 0),
  SME2_INSN ("sqcvtun", 0xc173e040, 0xff7ffc60, sme_sz_23, 0, OP2 (SVE_Zd, SME_Znx4), OP_SVE_VV_BH_SD, 0, 0),
  SME2_INSN ("sqdmulh", 0xc120a400, 0xff30ffe1, sme_size_22, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("sqdmulh", 0xc120ac00, 0xff30ffe3, sme_size_22, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("sqdmulh", 0xc120b400, 0xff21ffe1, sme_size_22, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zmx2), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("sqdmulh", 0xc120bc00, 0xff23ffe3, sme_size_22, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zmx4), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("sqrshr", 0xc1e0d400, 0xfff0fc20, sme_misc, 0, OP3 (SVE_Zd, SME_Znx2, SME_SHRIMM4), OP_SVE_HSU, 0, 0),
  SME2_INSN ("sqrshr", 0xc120d800, 0xff20fc60, sme_shift, 0, OP3 (SVE_Zd, SME_Znx4, SME_SHRIMM5), OP_SVE_VVU_BH_SD, 0, 0),
  SME2_INSN ("sqrshrn", 0xc120dc00, 0xff20fc60, sme_shift, 0, OP3 (SVE_Zd, SME_Znx4, SME_SHRIMM5), OP_SVE_VVU_BH_SD, 0, 0),
  SME2_INSN ("sqrshru", 0xc1f0d400, 0xfff0fc20, sme_misc, 0, OP3 (SVE_Zd, SME_Znx2, SME_SHRIMM4), OP_SVE_HSU, 0, 0),
  SME2_INSN ("sqrshru", 0xc120d840, 0xff20fc60, sme_shift, 0, OP3 (SVE_Zd, SME_Znx4, SME_SHRIMM5), OP_SVE_VVU_BH_SD, 0, 0),
  SME2_INSN ("sqrshrun", 0xc120dc40, 0xff20fc60, sme_shift, 0, OP3 (SVE_Zd, SME_Znx4, SME_SHRIMM5), OP_SVE_VVU_BH_SD, 0, 0),
  SME2_INSN ("srshl", 0xc120a220, 0xff30ffe1, sme_size_22, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("srshl", 0xc120aa20, 0xff30ffe3, sme_size_22, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("srshl", 0xc120b220, 0xff21ffe1, sme_size_22, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zmx2), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("srshl", 0xc120ba20, 0xff23ffe3, sme_size_22, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zmx4), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("st1b", 0xa0600000, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_BUU, 0, 0),
  SME2_INSN ("st1b", 0xa0608000, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_BUU, 0, 0),
  SME2_INSN ("st1b", 0xa1600000, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_BUU, 0, 0),
  SME2_INSN ("st1b", 0xa1608000, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_BUU, 0, 0),
  SME2_INSN ("st1b", 0xa0200000, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR), OP_SVE_BUU, 0, 0),
  SME2_INSN ("st1b", 0xa0208000, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR), OP_SVE_BUU, 0, 0),
  SME2_INSN ("st1b", 0xa1200000, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR), OP_SVE_BUU, 0, 0),
  SME2_INSN ("st1b", 0xa1208000, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR), OP_SVE_BUU, 0, 0),
  SME2_INSN ("st1d", 0xa0606000, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_DUU, 0, 0),
  SME2_INSN ("st1d", 0xa060e000, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_DUU, 0, 0),
  SME2_INSN ("st1d", 0xa1606000, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_DUU, 0, 0),
  SME2_INSN ("st1d", 0xa160e000, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_DUU, 0, 0),
  SME2_INSN ("st1d", 0xa0206000, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DUU, 0, 0),
  SME2_INSN ("st1d", 0xa020e000, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DUU, 0, 0),
  SME2_INSN ("st1d", 0xa1206000, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DUU, 0, 0),
  SME2_INSN ("st1d", 0xa120e000, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DUU, 0, 0),
  SME2_INSN ("st1h", 0xa0602000, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_HUU, 0, 0),
  SME2_INSN ("st1h", 0xa060a000, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_HUU, 0, 0),
  SME2_INSN ("st1h", 0xa1602000, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_HUU, 0, 0),
  SME2_INSN ("st1h", 0xa160a000, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_HUU, 0, 0),
  SME2_INSN ("st1h", 0xa0202000, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HUU, 0, 0),
  SME2_INSN ("st1h", 0xa020a000, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HUU, 0, 0),
  SME2_INSN ("st1h", 0xa1202000, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HUU, 0, 0),
  SME2_INSN ("st1h", 0xa120a000, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HUU, 0, 0),
  SME2_INSN ("st1w", 0xa0604000, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_SUU, 0, 0),
  SME2_INSN ("st1w", 0xa060c000, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_SUU, 0, 0),
  SME2_INSN ("st1w", 0xa1604000, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_SUU, 0, 0),
  SME2_INSN ("st1w", 0xa160c000, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_SUU, 0, 0),
  SME2_INSN ("st1w", 0xa0204000, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SUU, 0, 0),
  SME2_INSN ("st1w", 0xa020c000, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SUU, 0, 0),
  SME2_INSN ("st1w", 0xa1204000, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SUU, 0, 0),
  SME2_INSN ("st1w", 0xa120c000, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SUU, 0, 0),
  SME2_INSN ("stnt1b", 0xa0600001, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_BUU, 0, 0),
  SME2_INSN ("stnt1b", 0xa0608001, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_BUU, 0, 0),
  SME2_INSN ("stnt1b", 0xa1600008, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_BUU, 0, 0),
  SME2_INSN ("stnt1b", 0xa1608008, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_BUU, 0, 0),
  SME2_INSN ("stnt1b", 0xa0200001, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR), OP_SVE_BUU, 0, 0),
  SME2_INSN ("stnt1b", 0xa0208001, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR), OP_SVE_BUU, 0, 0),
  SME2_INSN ("stnt1b", 0xa1200008, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR), OP_SVE_BUU, 0, 0),
  SME2_INSN ("stnt1b", 0xa1208008, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR), OP_SVE_BUU, 0, 0),
  SME2_INSN ("stnt1d", 0xa0606001, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_DUU, 0, 0),
  SME2_INSN ("stnt1d", 0xa060e001, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_DUU, 0, 0),
  SME2_INSN ("stnt1d", 0xa1606008, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_DUU, 0, 0),
  SME2_INSN ("stnt1d", 0xa160e008, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_DUU, 0, 0),
  SME2_INSN ("stnt1d", 0xa0206001, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DUU, 0, 0),
  SME2_INSN ("stnt1d", 0xa020e001, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DUU, 0, 0),
  SME2_INSN ("stnt1d", 0xa1206008, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DUU, 0, 0),
  SME2_INSN ("stnt1d", 0xa120e008, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL3), OP_SVE_DUU, 0, 0),
  SME2_INSN ("stnt1h", 0xa0602001, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_HUU, 0, 0),
  SME2_INSN ("stnt1h", 0xa060a001, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_HUU, 0, 0),
  SME2_INSN ("stnt1h", 0xa1602008, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_HUU, 0, 0),
  SME2_INSN ("stnt1h", 0xa160a008, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_HUU, 0, 0),
  SME2_INSN ("stnt1h", 0xa0202001, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HUU, 0, 0),
  SME2_INSN ("stnt1h", 0xa020a001, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HUU, 0, 0),
  SME2_INSN ("stnt1h", 0xa1202008, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HUU, 0, 0),
  SME2_INSN ("stnt1h", 0xa120a008, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL1), OP_SVE_HUU, 0, 0),
  SME2_INSN ("stnt1w", 0xa0604001, 0xfff0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_SUU, 0, 0),
  SME2_INSN ("stnt1w", 0xa060c001, 0xfff0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_SUU, 0, 0),
  SME2_INSN ("stnt1w", 0xa1604008, 0xfff0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x2xVL), OP_SVE_SUU, 0, 0),
  SME2_INSN ("stnt1w", 0xa160c008, 0xfff0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RI_S4x4xVL), OP_SVE_SUU, 0, 0),
  SME2_INSN ("stnt1w", 0xa0204001, 0xffe0e001, sve_misc, 0, OP3 (SME_Zdnx2, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SUU, 0, 0),
  SME2_INSN ("stnt1w", 0xa020c001, 0xffe0e003, sve_misc, 0, OP3 (SME_Zdnx4, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SUU, 0, 0),
  SME2_INSN ("stnt1w", 0xa1204008, 0xffe0e008, sve_misc, 0, OP3 (SME_Ztx2_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SUU, 0, 0),
  SME2_INSN ("stnt1w", 0xa120c008, 0xffe0e00c, sve_misc, 0, OP3 (SME_Ztx4_STRIDED, SME_PNg3, SVE_ADDR_RR_LSL2), OP_SVE_SUU, 0, 0),
  SME2_INSN ("str", 0xe13f8000, 0xfffffc1f, sme_misc, 0, OP2 (SME_ZT0, SIMD_ADDR_SIMPLE), {}, 0, 0),
  SME2_INSN ("sub", 0xc1a01c18, 0xffbf9c38, sme_int_sd, 0, OP2 (SME_ZA_array_off3_0, SME_Znx2), OP_SVE_VVV_SD, F_OD (2), 0),
  SME2_INSN ("sub", 0xc1a11c18, 0xffbf9c78, sme_int_sd, 0, OP2 (SME_ZA_array_off3_0, SME_Znx4), OP_SVE_VVV_SD, F_OD (4), 0),
  SME2_INSN ("sub", 0xc1201818, 0xffb09c18, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD, F_OD (2), 0),
  SME2_INSN ("sub", 0xc1301818, 0xffb09c18, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD, F_OD (4), 0),
  SME2_INSN ("sub", 0xc1a01818, 0xffa19c38, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zmx2), OP_SVE_VVV_SD, F_OD (2), 0),
  SME2_INSN ("sub", 0xc1a11818, 0xffa39c78, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zmx4), OP_SVE_VVV_SD, F_OD (4), 0),
  SME2_INSN ("sudot", 0xc1501038, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX2), OP_SVE_SBB, F_OD (2), 0),
  SME2_INSN ("sudot", 0xc1509038, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX2), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("sudot", 0xc1201418, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_SBB, F_OD (2), 0),
  SME2_INSN ("sudot", 0xc1301418, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("sumlall", 0xc1000014, 0xfff0001c, sme_misc, 0, OP3 (SME_ZA_array_off2x4, SVE_Zn, SME_Zm_INDEX4_10), OP_SVE_SBB, 0, 0),
  SME2_INSN ("sumlall", 0xc1100030, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx2, SME_Zm_INDEX4_1), OP_SVE_SBB, F_OD (2), 0),
  SME2_INSN ("sumlall", 0xc1108030, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx4, SME_Zm_INDEX4_1), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("sumlall", 0xc1200014, 0xfff09c1e, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SVE_ZnxN, SME_Zm), OP_SVE_SBB, F_OD (2), 0),
  SME2_INSN ("sumlall", 0xc1300014, 0xfff09c1e, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SVE_ZnxN, SME_Zm), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("sunpk", 0xc125e000, 0xff3ffc01, sme_size_22_hsd, 0, OP2 (SME_Zdnx2, SVE_Zn), OP_SVE_VV_HSD_BHS, 0, 0),
  SME2_INSN ("sunpk", 0xc135e000, 0xff3ffc23, sme_size_22_hsd, 0, OP2 (SME_Zdnx4, SME_Znx2), OP_SVE_VV_HSD_BHS, 0, 0),
  SME2_INSN ("suvdot", 0xc1508038, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX2), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("svdot", 0xc1500020, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("svdot", 0xc1508020, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX2), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("uclamp", 0xc120c401, 0xff20fc01, sme_size_22, 0, OP3 (SME_Zdnx2, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  SME2_INSN ("uclamp", 0xc120cc01, 0xff20fc03, sme_size_22, 0, OP3 (SME_Zdnx4, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  SME2_INSN ("ucvtf", 0xc122e020, 0xfffffc21, sve_misc, 0, OP2 (SME_Zdnx2, SME_Znx2), OP_SVE_SS, 0, 0),
  SME2_INSN ("ucvtf", 0xc132e020, 0xfffffc63, sve_misc, 0, OP2 (SME_Zdnx4, SME_Znx4), OP_SVE_SS, 0, 0),
  SME2_INSN ("udot", 0xc1501010, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("udot", 0xc1509010, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX2), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("udot", 0xc1601418, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("udot", 0xc1701418, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("udot", 0xc1e01418, 0xffe19c38, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zmx2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("udot", 0xc1e11418, 0xffe39c78, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zmx4), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("udot", 0xc1501030, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX2), OP_SVE_SBB, F_OD (2), 0),
  SME2_INSN ("udot", 0xc1509030, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX2), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("udot", 0xc1201410, 0xffb09c18, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD_BH, F_OD (2), 0),
  SME2_INSN ("udot", 0xc1301410, 0xffb09c18, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD_BH, F_OD (4), 0),
  SME2_INSN ("udot", 0xc1a01410, 0xffa19c38, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zmx2), OP_SVE_VVV_SD_BH, F_OD (2), 0),
  SME2_INSN ("udot", 0xc1a11410, 0xffa39c78, sme_int_sd, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zmx4), OP_SVE_VVV_SD_BH, F_OD (4), 0),
  SME2_INSN ("umax", 0xc120a001, 0xff30ffe1, sme_size_22, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("umax", 0xc120a801, 0xff30ffe3, sme_size_22, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("umax", 0xc120b001, 0xff21ffe1, sme_size_22, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zmx2), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("umax", 0xc120b801, 0xff23ffe3, sme_size_22, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zmx4), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("umin", 0xc120a021, 0xff30ffe1, sme_size_22, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("umin", 0xc120a821, 0xff30ffe3, sme_size_22, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("umin", 0xc120b021, 0xff21ffe1, sme_size_22, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zmx2), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("umin", 0xc120b821, 0xff23ffe3, sme_size_22, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zmx4), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("umlal", 0xc1c01010, 0xfff01018, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm_INDEX3_10), OP_SVE_SHH, 0, 0),
  SME2_INSN ("umlal", 0xc1d01010, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("umlal", 0xc1d09010, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("umlal", 0xc1600c10, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm), OP_SVE_SHH, 0, 0),
  SME2_INSN ("umlal", 0xc1600810, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("umlal", 0xc1700810, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("umlal", 0xc1e00810, 0xffe19c3c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zmx2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("umlal", 0xc1e10810, 0xffe39c7c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zmx4), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("umlall", 0xc1000010, 0xfff0001c, sme_misc, 0, OP3 (SME_ZA_array_off2x4, SVE_Zn, SME_Zm_INDEX4_10), OP_SVE_SBB, 0, 0),
  SME2_INSN ("umlall", 0xc1100010, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx2, SME_Zm_INDEX4_1), OP_SVE_SBB, F_OD (2), 0),
  SME2_INSN ("umlall", 0xc1108010, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx4, SME_Zm_INDEX4_1), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("umlall", 0xc1200410, 0xffb09c1c, sme_int_sd, 0, OP3 (SME_ZA_array_off2x4, SVE_Zn, SME_Zm), OP_SVE_VVV_SD_BH, 0, 0),
  SME2_INSN ("umlall", 0xc1200010, 0xffb09c1e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD_BH, F_OD (2), 0),
  SME2_INSN ("umlall", 0xc1300010, 0xffb09c1e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD_BH, F_OD (4), 0),
  SME2_INSN ("umlall", 0xc1a00010, 0xffa19c3e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SME_Znx2, SME_Zmx2), OP_SVE_VVV_SD_BH, F_OD (2), 0),
  SME2_INSN ("umlall", 0xc1a10010, 0xffa39c7e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SME_Znx4, SME_Zmx4), OP_SVE_VVV_SD_BH, F_OD (4), 0),
  SME2_INSN ("umlsl", 0xc1c01018, 0xfff01018, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm_INDEX3_10), OP_SVE_SHH, 0, 0),
  SME2_INSN ("umlsl", 0xc1d01018, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("umlsl", 0xc1d09018, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zm_INDEX3_2), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("umlsl", 0xc1600c18, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3x2, SVE_Zn, SME_Zm), OP_SVE_SHH, 0, 0),
  SME2_INSN ("umlsl", 0xc1600818, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("umlsl", 0xc1700818, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SVE_ZnxN, SME_Zm), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("umlsl", 0xc1e00818, 0xffe19c3c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx2, SME_Zmx2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("umlsl", 0xc1e10818, 0xffe39c7c, sme_misc, 0, OP3 (SME_ZA_array_off2x2, SME_Znx4, SME_Zmx4), OP_SVE_SHH, F_OD (4), 0),
  SME2_INSN ("umlsll", 0xc1000018, 0xfff0001c, sme_misc, 0, OP3 (SME_ZA_array_off2x4, SVE_Zn, SME_Zm_INDEX4_10), OP_SVE_SBB, 0, 0),
  SME2_INSN ("umlsll", 0xc1100018, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx2, SME_Zm_INDEX4_1), OP_SVE_SBB, F_OD (2), 0),
  SME2_INSN ("umlsll", 0xc1108018, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx4, SME_Zm_INDEX4_1), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("umlsll", 0xc1200418, 0xffb09c1c, sme_int_sd, 0, OP3 (SME_ZA_array_off2x4, SVE_Zn, SME_Zm), OP_SVE_VVV_SD_BH, 0, 0),
  SME2_INSN ("umlsll", 0xc1200018, 0xffb09c1e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD_BH, F_OD (2), 0),
  SME2_INSN ("umlsll", 0xc1300018, 0xffb09c1e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD_BH, F_OD (4), 0),
  SME2_INSN ("umlsll", 0xc1a00018, 0xffa19c3e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SME_Znx2, SME_Zmx2), OP_SVE_VVV_SD_BH, F_OD (2), 0),
  SME2_INSN ("umlsll", 0xc1a10018, 0xffa39c7e, sme_int_sd, 0, OP3 (SME_ZA_array_off1x4, SME_Znx4, SME_Zmx4), OP_SVE_VVV_SD_BH, F_OD (4), 0),
  SME2_INSN ("umopa", 0xa1800008, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMHH, 0, 0),
  SME2_INSN ("umops", 0xa1800018, 0xffe0001c, sme_misc, 0, OP5 (SME_ZAda_2b, SVE_Pg3, SME_Pm, SVE_Zn, SVE_Zm_16), OP_SVE_SMMHH, 0, 0),
  SME2_INSN ("uqcvt", 0xc123e020, 0xfffffc20, sme_misc, 0, OP2 (SVE_Zd, SME_Znx2), OP_SVE_HS, 0, 0),
  SME2_INSN ("uqcvt", 0xc133e020, 0xff7ffc60, sme_sz_23, 0, OP2 (SVE_Zd, SME_Znx4), OP_SVE_VV_BH_SD, 0, 0),
  SME2_INSN ("uqcvtn", 0xc133e060, 0xff7ffc60, sme_sz_23, 0, OP2 (SVE_Zd, SME_Znx4), OP_SVE_VV_BH_SD, 0, 0),
  SME2_INSN ("uqrshr", 0xc1e0d420, 0xfff0fc20, sme_misc, 0, OP3 (SVE_Zd, SME_Znx2, SME_SHRIMM4), OP_SVE_HSU, 0, 0),
  SME2_INSN ("uqrshr", 0xc120d820, 0xff20fc60, sme_shift, 0, OP3 (SVE_Zd, SME_Znx4, SME_SHRIMM5), OP_SVE_VVU_BH_SD, 0, 0),
  SME2_INSN ("uqrshrn", 0xc120dc20, 0xff20fc60, sme_shift, 0, OP3 (SVE_Zd, SME_Znx4, SME_SHRIMM5), OP_SVE_VVU_BH_SD, 0, 0),
  SME2_INSN ("urshl", 0xc120a221, 0xff30ffe1, sme_size_22, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("urshl", 0xc120aa21, 0xff30ffe3, sme_size_22, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zm), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("urshl", 0xc120b221, 0xff21ffe1, sme_size_22, 0, OP3 (SME_Zdnx2, SME_Zdnx2, SME_Zmx2), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("urshl", 0xc120ba21, 0xff23ffe3, sme_size_22, 0, OP3 (SME_Zdnx4, SME_Zdnx4, SME_Zmx4), OP_SVE_VVV_BHSD, 0, 1),
  SME2_INSN ("usdot", 0xc1501028, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX2), OP_SVE_SBB, F_OD (2), 0),
  SME2_INSN ("usdot", 0xc1509028, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX2), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("usdot", 0xc1201408, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_SBB, F_OD (2), 0),
  SME2_INSN ("usdot", 0xc1301408, 0xfff09c18, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SVE_ZnxN, SME_Zm), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("usdot", 0xc1a01408, 0xffe19c38, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zmx2), OP_SVE_SBB, F_OD (2), 0),
  SME2_INSN ("usdot", 0xc1a11408, 0xffe39c78, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zmx4), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("usmlall", 0xc1000004, 0xfff0001c, sme_misc, 0, OP3 (SME_ZA_array_off2x4, SVE_Zn, SME_Zm_INDEX4_10), OP_SVE_SBB, 0, 0),
  SME2_INSN ("usmlall", 0xc1100020, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx2, SME_Zm_INDEX4_1), OP_SVE_SBB, F_OD (2), 0),
  SME2_INSN ("usmlall", 0xc1108020, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx4, SME_Zm_INDEX4_1), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("usmlall", 0xc1200404, 0xfff09c1c, sme_misc, 0, OP3 (SME_ZA_array_off2x4, SVE_Zn, SME_Zm), OP_SVE_VVV_SD_BH, 0, 0),
  SME2_INSN ("usmlall", 0xc1200004, 0xfff09c1e, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD_BH, F_OD (2), 0),
  SME2_INSN ("usmlall", 0xc1300004, 0xfff09c1e, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SVE_ZnxN, SME_Zm), OP_SVE_VVV_SD_BH, F_OD (4), 0),
  SME2_INSN ("usmlall", 0xc1a00004, 0xffe19c3e, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx2, SME_Zmx2), OP_SVE_SBB, F_OD (2), 0),
  SME2_INSN ("usmlall", 0xc1a10004, 0xffe39c7e, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx4, SME_Zmx4), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("usvdot", 0xc1508028, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX2), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("uunpk", 0xc125e001, 0xff3ffc01, sme_size_22_hsd, 0, OP2 (SME_Zdnx2, SVE_Zn), OP_SVE_VV_HSD_BHS, 0, 0),
  SME2_INSN ("uunpk", 0xc135e001, 0xff3ffc23, sme_size_22_hsd, 0, OP2 (SME_Zdnx4, SME_Znx2), OP_SVE_VV_HSD_BHS, 0, 0),
  SME2_INSN ("uvdot", 0xc1500030, 0xfff09038, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX2), OP_SVE_SHH, F_OD (2), 0),
  SME2_INSN ("uvdot", 0xc1508030, 0xfff09078, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX2), OP_SVE_SBB, F_OD (4), 0),
  SME2_INSN ("uzp", 0xc120d001, 0xff20fc01, sme_size_22, 0, OP3 (SME_Zdnx2, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  SME2_INSN ("uzp", 0xc120d401, 0xffe0fc01, sme_misc, 0, OP3 (SME_Zdnx2, SVE_Zn, SVE_Zm_16), OP_SVE_QQQ, 0, 0),
  SME2_INSN ("uzp", 0xc136e002, 0xff3ffc63, sme_size_22, 0, OP2 (SME_Zdnx4, SME_Znx4), OP_SVE_VV_BHSD, 0, 0),
  SME2_INSN ("uzp", 0xc137e002, 0xfffffc63, sme_misc, 0, OP2 (SME_Zdnx4, SME_Znx4), OP_SVE_QQ, 0, 0),
  SME2_INSN ("whilege", 0x25204010, 0xff20dc18, sme_size_22, 0, OP4 (SME_PNd3, Rn, Rm, SME_VLxN_13), OP_SVE_VXXU_BHSD, 0, 0),
  SME2_INSN ("whilegt", 0x25204018, 0xff20dc18, sme_size_22, 0, OP4 (SME_PNd3, Rn, Rm, SME_VLxN_13), OP_SVE_VXXU_BHSD, 0, 0),
  SME2_INSN ("whilehi", 0x25204818, 0xff20dc18, sme_size_22, 0, OP4 (SME_PNd3, Rn, Rm, SME_VLxN_13), OP_SVE_VXXU_BHSD, 0, 0),
  SME2_INSN ("whilehs", 0x25204810, 0xff20dc18, sme_size_22, 0, OP4 (SME_PNd3, Rn, Rm, SME_VLxN_13), OP_SVE_VXXU_BHSD, 0, 0),
  SME2_INSN ("whilele", 0x25204418, 0xff20dc18, sme_size_22, 0, OP4 (SME_PNd3, Rn, Rm, SME_VLxN_13), OP_SVE_VXXU_BHSD, 0, 0),
  SME2_INSN ("whilelo", 0x25204c10, 0xff20dc18, sme_size_22, 0, OP4 (SME_PNd3, Rn, Rm, SME_VLxN_13), OP_SVE_VXXU_BHSD, 0, 0),
  SME2_INSN ("whilels", 0x25204c18, 0xff20dc18, sme_size_22, 0, OP4 (SME_PNd3, Rn, Rm, SME_VLxN_13), OP_SVE_VXXU_BHSD, 0, 0),
  SME2_INSN ("whilelt", 0x25204410, 0xff20dc18, sme_size_22, 0, OP4 (SME_PNd3, Rn, Rm, SME_VLxN_13), OP_SVE_VXXU_BHSD, 0, 0),
  SME2_INSN ("zero", 0xc0480001, 0xffffffff, sme_misc, 0, OP1 (SME_ZT0_LIST), {}, 0, 0),
  SME2_INSN ("zip", 0xc120d000, 0xff20fc01, sme_size_22, 0, OP3 (SME_Zdnx2, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_BHSD, 0, 0),
  SME2_INSN ("zip", 0xc120d400, 0xffe0fc01, sme_misc, 0, OP3 (SME_Zdnx2, SVE_Zn, SVE_Zm_16), OP_SVE_QQQ, 0, 0),
  SME2_INSN ("zip", 0xc136e000, 0xff3ffc63, sme_size_22, 0, OP2 (SME_Zdnx4, SME_Znx4), OP_SVE_VV_BHSD, 0, 0),
  SME2_INSN ("zip", 0xc137e000, 0xfffffc63, sme_misc, 0, OP2 (SME_Zdnx4, SME_Znx4), OP_SVE_QQ, 0, 0),

  /* SME2 I16I64 instructions.  */
  SME2_I16I64_INSN ("sdot", 0xc1d00008, 0xfff09838, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX1), OP_SVE_DHH, F_OD (2), 0),
  SME2_I16I64_INSN ("sdot", 0xc1d08008, 0xfff09878, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX1), OP_SVE_DHH, F_OD (4), 0),
  SME2_I16I64_INSN ("smlall", 0xc1800000, 0xfff0101c, sme_misc, 0, OP3 (SME_ZA_array_off2x4, SVE_Zn, SME_Zm_INDEX3_10), OP_SVE_DHH, 0, 0),
  SME2_I16I64_INSN ("smlall", 0xc1900000, 0xfff09838, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx2, SME_Zm_INDEX3_1), OP_SVE_DHH, F_OD (2), 0),
  SME2_I16I64_INSN ("smlall", 0xc1908000, 0xfff09878, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx4, SME_Zm_INDEX3_1), OP_SVE_DHH, F_OD (4), 0),
  SME2_I16I64_INSN ("smlsll", 0xc1800008, 0xfff0101c, sme_misc, 0, OP3 (SME_ZA_array_off2x4, SVE_Zn, SME_Zm_INDEX3_10), OP_SVE_DHH, 0, 0),
  SME2_I16I64_INSN ("smlsll", 0xc1900008, 0xfff09838, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx2, SME_Zm_INDEX3_1), OP_SVE_DHH, F_OD (2), 0),
  SME2_I16I64_INSN ("smlsll", 0xc1908008, 0xfff09878, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx4, SME_Zm_INDEX3_1), OP_SVE_DHH, F_OD (4), 0),
  SME2_I16I64_INSN ("svdot", 0xc1d08808, 0xfff09878, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX1), OP_SVE_DHH, F_OD (4), 0),
  SME2_I16I64_INSN ("udot", 0xc1d00018, 0xfff09838, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX1), OP_SVE_DHH, F_OD (2), 0),
  SME2_I16I64_INSN ("udot", 0xc1d08018, 0xfff09878, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX1), OP_SVE_DHH, F_OD (4), 0),
  SME2_I16I64_INSN ("umlall", 0xc1800010, 0xfff0101c, sme_misc, 0, OP3 (SME_ZA_array_off2x4, SVE_Zn, SME_Zm_INDEX3_10), OP_SVE_DHH, 0, 0),
  SME2_I16I64_INSN ("umlall", 0xc1900010, 0xfff09838, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx2, SME_Zm_INDEX3_1), OP_SVE_DHH, F_OD (2), 0),
  SME2_I16I64_INSN ("umlall", 0xc1908010, 0xfff09878, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx4, SME_Zm_INDEX3_1), OP_SVE_DHH, F_OD (4), 0),
  SME2_I16I64_INSN ("umlsll", 0xc1800018, 0xfff0101c, sme_misc, 0, OP3 (SME_ZA_array_off2x4, SVE_Zn, SME_Zm_INDEX3_10), OP_SVE_DHH, 0, 0),
  SME2_I16I64_INSN ("umlsll", 0xc1900018, 0xfff09838, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx2, SME_Zm_INDEX3_1), OP_SVE_DHH, F_OD (2), 0),
  SME2_I16I64_INSN ("umlsll", 0xc1908018, 0xfff09878, sme_misc, 0, OP3 (SME_ZA_array_off1x4, SME_Znx4, SME_Zm_INDEX3_1), OP_SVE_DHH, F_OD (4), 0),
  SME2_I16I64_INSN ("uvdot", 0xc1d08818, 0xfff09878, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX1), OP_SVE_DHH, F_OD (4), 0),

  /* SME2 F64F64 instructions.  */
  SME2_F64F64_INSN ("fmla", 0xc1d00000, 0xfff09838, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX1), OP_SVE_DDD, F_OD (2), 0),
  SME2_F64F64_INSN ("fmla", 0xc1d08000, 0xfff09878, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX1), OP_SVE_DDD, F_OD (4), 0),
  SME2_F64F64_INSN ("fmls", 0xc1d00010, 0xfff09838, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx2, SME_Zm_INDEX1), OP_SVE_DDD, F_OD (2), 0),
  SME2_F64F64_INSN ("fmls", 0xc1d08010, 0xfff09878, sme_misc, 0, OP3 (SME_ZA_array_off3_0, SME_Znx4, SME_Zm_INDEX1), OP_SVE_DDD, F_OD (4), 0),

  /* SIMD Dot Product (optional in v8.2-A).  */
  DOT_INSN ("udot", 0x2e009400, 0xbf20fc00, dotproduct, OP3 (Vd, Vn, Vm), QL_V3DOT, F_SIZEQ),
  DOT_INSN ("sdot", 0xe009400,  0xbf20fc00, dotproduct, OP3 (Vd, Vn, Vm), QL_V3DOT, F_SIZEQ),
  DOT_INSN ("udot", 0x2f00e000, 0xbf00f400, dotproduct, OP3 (Vd, Vn, Em), QL_V2DOT, F_SIZEQ),
  DOT_INSN ("sdot", 0xf00e000,  0xbf00f400, dotproduct, OP3 (Vd, Vn, Em), QL_V2DOT, F_SIZEQ),
/* Crypto SHA2 (optional in ARMv8.2-a).  */
  SHA2_INSN ("sha512h",   0xce608000, 0xffe0fc00, cryptosha2, OP3 (Fd, Fn, Vm), QL_SHA512UPT, 0),
  SHA2_INSN ("sha512h2",  0xce608400, 0xffe0fc00, cryptosha2, OP3 (Fd, Fn, Vm), QL_SHA512UPT, 0),
  SHA2_INSN ("sha512su0", 0xcec08000, 0xfffffc00, cryptosha2, OP2 (Vd, Vn), QL_V2SAME2D, 0),
  SHA2_INSN ("sha512su1", 0xce608800, 0xffe0fc00, cryptosha2, OP3 (Vd, Vn, Vm), QL_V3SAME2D, 0),
  /* Crypto SHA3 (optional in ARMv8.2-a).  */
  SHA3_INSN ("eor3",      0xce000000, 0xffe08000, cryptosha3, OP4 (Vd, Vn, Vm, Va), QL_V4SAME16B, 0),
  SHA3_INSN ("rax1",      0xce608c00, 0xffe0fc00, cryptosha3, OP3 (Vd, Vn, Vm), QL_V3SAME2D, 0),
  SHA3_INSN ("xar",       0xce800000, 0xffe00000, cryptosha3, OP4 (Vd, Vn, Vm, IMM), QL_XAR, 0),
  SHA3_INSN ("bcax",      0xce200000, 0xffe08000, cryptosha3, OP4 (Vd, Vn, Vm, Va), QL_V4SAME16B, 0),
  /* Crypto SM3 (optional in ARMv8.2-a).  */
  SM4_INSN ("sm3ss1",    0xce400000, 0xffe08000, cryptosm3, OP4 (Vd, Vn, Vm, Va), QL_V4SAME4S, 0),
  SM4_INSN ("sm3tt1a",   0xce408000, 0xffe0cc00, cryptosm3, OP3 (Vd, Vn, Em), QL_SM3TT, 0),
  SM4_INSN ("sm3tt1b",   0xce408400, 0xffe0cc00, cryptosm3, OP3 (Vd, Vn, Em), QL_SM3TT, 0),
  SM4_INSN ("sm3tt2a",   0xce408800, 0xffe0cc00, cryptosm3, OP3 (Vd, Vn, Em), QL_SM3TT, 0),
  SM4_INSN ("sm3tt2b",   0xce408c00, 0xffe0cc00, cryptosm3, OP3 (Vd, Vn, Em), QL_SM3TT, 0),
  SM4_INSN ("sm3partw1", 0xce60c000, 0xffe0fc00, cryptosm3, OP3 (Vd, Vn, Vm), QL_V3SAME4S, 0),
  SM4_INSN ("sm3partw2", 0xce60c400, 0xffe0fc00, cryptosm3, OP3 (Vd, Vn, Vm), QL_V3SAME4S, 0),
  /* Crypto SM4 (optional in ARMv8.2-a).  */
  SM4_INSN ("sm4e",    0xcec08400, 0xfffffc00, cryptosm4, OP2 (Vd, Vn), QL_V2SAME4S, 0),
  SM4_INSN ("sm4ekey", 0xce60c800, 0xffe0fc00, cryptosm4, OP3 (Vd, Vn, Vm), QL_V3SAME4S, 0),
  /* Crypto FP16 (optional in ARMv8.2-a).  */
  FP16_V8_2_INSN ("fmlal",  0xe20ec00,  0xffa0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3FML2S, 0),
  FP16_V8_2_INSN ("fmlsl",  0xea0ec00,  0xffa0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3FML2S, 0),
  FP16_V8_2_INSN ("fmlal2", 0x2e20cc00, 0xffa0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3FML2S, 0),
  FP16_V8_2_INSN ("fmlsl2", 0x2ea0cc00, 0xffa0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3FML2S, 0),

  FP16_V8_2_INSN ("fmlal",  0x4e20ec00, 0xffa0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3FML4S, 0),
  FP16_V8_2_INSN ("fmlsl",  0x4ea0ec00, 0xffa0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3FML4S, 0),
  FP16_V8_2_INSN ("fmlal2", 0x6e20cc00, 0xffa0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3FML4S, 0),
  FP16_V8_2_INSN ("fmlsl2", 0x6ea0cc00, 0xffa0fc00, asimdsame, OP3 (Vd, Vn, Vm), QL_V3FML4S, 0),

  FP16_V8_2_INSN ("fmlal",  0xf800000, 0xffc0f400, asimdelem, OP3 (Vd, Vn, Em16), QL_V2FML2S, 0),
  FP16_V8_2_INSN ("fmlsl",  0xf804000, 0xffc0f400, asimdelem, OP3 (Vd, Vn, Em16), QL_V2FML2S, 0),
  FP16_V8_2_INSN ("fmlal2", 0x2f808000, 0xffc0f400, asimdelem, OP3 (Vd, Vn, Em16), QL_V2FML2S, 0),
  FP16_V8_2_INSN ("fmlsl2", 0x2f80c000, 0xffc0f400, asimdelem, OP3 (Vd, Vn, Em16), QL_V2FML2S, 0),

  FP16_V8_2_INSN ("fmlal",  0x4f800000, 0xffc0f400, asimdelem, OP3 (Vd, Vn, Em16), QL_V2FML4S, 0),
  FP16_V8_2_INSN ("fmlsl",  0x4f804000, 0xffc0f400, asimdelem, OP3 (Vd, Vn, Em16), QL_V2FML4S, 0),
  FP16_V8_2_INSN ("fmlal2", 0x6f808000, 0xffc0f400, asimdelem, OP3 (Vd, Vn, Em16), QL_V2FML4S, 0),
  FP16_V8_2_INSN ("fmlsl2", 0x6f80c000, 0xffc0f400, asimdelem, OP3 (Vd, Vn, Em16), QL_V2FML4S, 0),
  /* System extensions ARMv8.4-a.  */
  FLAGM_INSN ("rmif",   0xba000400, 0xffe07c10, ic_system, OP3 (Rn, IMM_2, MASK), QL_RMIF, 0),
  FLAGM_INSN ("setf8",  0x3a00080d, 0xfffffc1f, ic_system, OP1 (Rn), QL_SETF, 0),
  FLAGM_INSN ("setf16", 0x3a00480d, 0xfffffc1f, ic_system, OP1 (Rn), QL_SETF, 0),
  /* Memory access instructions ARMv8.4-a.  */
  V8_4_INSN ("stlurb" ,  0x19000000, 0xffe00c00, ldst_unscaled, OP2 (Rt, ADDR_OFFSET), QL_STLW, 0),
  V8_4_INSN ("ldapurb",  0x19400000, 0xffe00c00, ldst_unscaled, OP2 (Rt, ADDR_OFFSET), QL_STLW, 0),
  V8_4_INSN ("ldapursb", 0x19c00000, 0xffe00c00, ldst_unscaled, OP2 (Rt, ADDR_OFFSET), QL_STLW, 0),
  V8_4_INSN ("ldapursb", 0x19800000, 0xffe00c00, ldst_unscaled, OP2 (Rt, ADDR_OFFSET), QL_STLX, 0),
  V8_4_INSN ("stlurh",   0x59000000, 0xffe00c00, ldst_unscaled, OP2 (Rt, ADDR_OFFSET), QL_STLW, 0),
  V8_4_INSN ("ldapurh",  0x59400000, 0xffe00c00, ldst_unscaled, OP2 (Rt, ADDR_OFFSET), QL_STLW, 0),
  V8_4_INSN ("ldapursh", 0x59c00000, 0xffe00c00, ldst_unscaled, OP2 (Rt, ADDR_OFFSET), QL_STLW, 0),
  V8_4_INSN ("ldapursh", 0x59800000, 0xffe00c00, ldst_unscaled, OP2 (Rt, ADDR_OFFSET), QL_STLX, 0),
  V8_4_INSN ("stlur",    0x99000000, 0xffe00c00, ldst_unscaled, OP2 (Rt, ADDR_OFFSET), QL_STLW, 0),
  V8_4_INSN ("ldapur",   0x99400000, 0xffe00c00, ldst_unscaled, OP2 (Rt, ADDR_OFFSET), QL_STLW, 0),
  V8_4_INSN ("ldapursw", 0x99800000, 0xffe00c00, ldst_unscaled, OP2 (Rt, ADDR_OFFSET), QL_STLX, 0),
  V8_4_INSN ("stlur",    0xd9000000, 0xffe00c00, ldst_unscaled, OP2 (Rt, ADDR_OFFSET), QL_STLX, 0),
  V8_4_INSN ("ldapur",   0xd9400000, 0xffe00c00, ldst_unscaled, OP2 (Rt, ADDR_OFFSET), QL_STLX, 0),

  /* Matrix Multiply instructions.  */
  INT8MATMUL_SVE_INSNC ("smmla",  0x45009800, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SBB, 0, C_SCAN_MOVPRFX, 0),
  INT8MATMUL_SVE_INSNC ("ummla",  0x45c09800, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SBB, 0, C_SCAN_MOVPRFX, 0),
  INT8MATMUL_SVE_INSNC ("usmmla", 0x45809800, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SBB, 0, C_SCAN_MOVPRFX, 0),
  INT8MATMUL_SVE_INSNC ("usdot",  0x44807800, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SBB, 0, C_SCAN_MOVPRFX, 0),
  INT8MATMUL_SVE_INSNC ("usdot",  0x44a01800, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX), OP_SVE_SBB, 0, C_SCAN_MOVPRFX, 0),
  INT8MATMUL_SVE_INSNC ("sudot",  0x44a01c00, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX), OP_SVE_SBB, 0, C_SCAN_MOVPRFX, 0),
  F32MATMUL_SVE_INSNC ("fmmla",   0x64a0e400, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_S, 0, C_SCAN_MOVPRFX, 0),
  F64MATMUL_SVE_INSNC ("fmmla",   0x64e0e400, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_VVV_D, 0, C_SCAN_MOVPRFX, 0),
  F64MATMUL_SVE_INSN ("ld1rob",  0xa4200000, 0xffe0e000, sve_misc, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX), OP_SVE_BZU, F_OD(1), 0),
  F64MATMUL_SVE_INSN ("ld1roh",  0xa4a00000, 0xffe0e000, sve_misc, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL1), OP_SVE_HZU, F_OD(1), 0),
  F64MATMUL_SVE_INSN ("ld1row",  0xa5200000, 0xffe0e000, sve_misc, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL2), OP_SVE_SZU, F_OD(1), 0),
  F64MATMUL_SVE_INSN ("ld1rod",  0xa5a00000, 0xffe0e000, sve_misc, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RX_LSL3), OP_SVE_DZU, F_OD(1), 0),
  F64MATMUL_SVE_INSN ("ld1rob",  0xa4202000, 0xfff0e000, sve_misc, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x32), OP_SVE_BZU, F_OD(1), 0),
  F64MATMUL_SVE_INSN ("ld1roh",  0xa4a02000, 0xfff0e000, sve_misc, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x32), OP_SVE_HZU, F_OD(1), 0),
  F64MATMUL_SVE_INSN ("ld1row",  0xa5202000, 0xfff0e000, sve_misc, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x32), OP_SVE_SZU, F_OD(1), 0),
  F64MATMUL_SVE_INSN ("ld1rod",  0xa5a02000, 0xfff0e000, sve_misc, OP3 (SVE_ZtxN, SVE_Pg3, SVE_ADDR_RI_S4x32), OP_SVE_DZU, F_OD(1), 0),
  F64MATMUL_SVE_INSN ("zip1",    0x05a00000, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_QQQ, 0, 0),
  F64MATMUL_SVE_INSN ("zip2",    0x05a00400, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_QQQ, 0, 0),
  F64MATMUL_SVE_INSN ("uzp1",    0x05a00800, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_QQQ, 0, 0),
  F64MATMUL_SVE_INSN ("uzp2",    0x05a00c00, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_QQQ, 0, 0),
  F64MATMUL_SVE_INSN ("trn1",    0x05a01800, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_QQQ, 0, 0),
  F64MATMUL_SVE_INSN ("trn2",    0x05a01c00, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_QQQ, 0, 0),
  /* Matrix Multiply advanced SIMD instructions.  */
  INT8MATMUL_INSN ("smmla",  0x4e80a400, 0xffe0fc00, aarch64_misc, OP3 (Vd, Vn, Vm), QL_MMLA64, 0),
  INT8MATMUL_INSN ("ummla",  0x6e80a400, 0xffe0fc00, aarch64_misc, OP3 (Vd, Vn, Vm), QL_MMLA64, 0),
  INT8MATMUL_INSN ("usmmla", 0x4e80ac00, 0xffe0fc00, aarch64_misc, OP3 (Vd, Vn, Vm), QL_MMLA64, 0),
  INT8MATMUL_INSN ("usdot",  0x0e809c00, 0xbfe0fc00, aarch64_misc, OP3 (Vd, Vn, Vm), QL_V3DOT, F_SIZEQ),
  INT8MATMUL_INSN ("usdot",  0x0f80f000, 0xbfc0f400, dotproduct, OP3 (Vd, Vn, Em), QL_V2DOT, F_SIZEQ),
  INT8MATMUL_INSN ("sudot",  0x0f00f000, 0xbfc0f400, dotproduct, OP3 (Vd, Vn, Em), QL_V2DOT, F_SIZEQ),

  /* BFloat instructions.  */
  BFLOAT16_SVE_INSNC ("bfdot",  0x64608000, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  BFLOAT16_SVE_INSNC ("bfdot",  0x64604000, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  BFLOAT16_SVE_INSNC ("bfmmla",  0x6460e400, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  BFLOAT16_SVE_INSNC ("bfcvt",  0x658aa000, 0xffffe000, sve_misc, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_HMS, 0, C_SCAN_MOVPRFX | C_MAX_ELEM, 0),
  BFLOAT16_SVE_INSNC ("bfcvtnt",  0x648aa000, 0xffffe000, sve_misc, OP3 (SVE_Zd, SVE_Pg3, SVE_Zn), OP_SVE_HMS, 0, 0, 0),
  BFLOAT16_SVE_INSNC ("bfmlalt",  0x64e08400, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  BFLOAT16_SVE_INSNC ("bfmlalb",  0x64e08000, 0xffe0fc00, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm_16), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  BFLOAT16_SVE_INSNC ("bfmlalt",  0x64e04400, 0xffe0f400, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  BFLOAT16_SVE_INSNC ("bfmlalb",  0x64e04000, 0xffe0f400, sve_misc, OP3 (SVE_Zd, SVE_Zn, SVE_Zm3_11_INDEX), OP_SVE_SHH, 0, C_SCAN_MOVPRFX, 0),
  /* BFloat Advanced SIMD instructions.  */
  BFLOAT16_INSN ("bfdot", 0x2e40fc00, 0xbfe0fc00, bfloat16, OP3 (Vd, Vn, Vm), QL_BFDOT64, F_SIZEQ),
  /* Using dotproduct as iclass to treat instruction similar to udot.  */
  BFLOAT16_INSN ("bfdot", 0x0f40f000, 0xbfc0f400, dotproduct, OP3 (Vd, Vn, Em), QL_BFDOT64I, F_SIZEQ),
  BFLOAT16_INSN ("bfmmla", 0x6e40ec00, 0xffe0fc00, bfloat16, OP3 (Vd, Vn, Vm), QL_BFMMLA, F_SIZEQ),
  BFLOAT16_INSN ("bfcvtn",  0x0ea16800, 0xfffffc00, bfloat16, OP2 (Vd, Vn), QL_BFCVTN64, 0),
  BFLOAT16_INSN ("bfcvtn2", 0x4ea16800, 0xfffffc00, bfloat16, OP2 (Vd, Vn), QL_BFCVTN2_64, 0),
  BFLOAT16_INSN ("bfcvt",  0x1e634000, 0xfffffc00, bfloat16, OP2 (Fd, Fn), QL_BFCVT64, 0),
  BFLOAT16_INSN ("bfmlalt", 0x6ec0fc00, 0xffe0fc00, bfloat16, OP3 (Vd, Vn, Vm), QL_BFMMLA, 0),
  BFLOAT16_INSN ("bfmlalb", 0x2ec0fc00, 0xffe0fc00, bfloat16, OP3 (Vd, Vn, Vm), QL_BFMMLA, 0),
  BFLOAT16_INSN ("bfmlalt", 0x4fc0f000, 0xffc0f400, bfloat16, OP3 (Vd, Vn, Em16), QL_V3BFML4S, 0),
  BFLOAT16_INSN ("bfmlalb", 0x0fc0f000, 0xffc0f400, bfloat16, OP3 (Vd, Vn, Em16), QL_V3BFML4S, 0),

  /* cpyfp cpyfprn cpyfpwn cpyfpn
     cpyfm cpyfmrn cpyfmwn cpyfmn
     cpyfe cpyfern cpyfewn cpyfen

     cpyfprt cpyfprtrn cpyfprtwn cpyfprtn
     cpyfmrt cpyfmrtrn cpyfmrtwn cpyfmrtn
     cpyfert cpyfertrn cpyfertwn cpyfertn

     cpyfpwt cpyfpwtrn cpyfpwtwn cpyfpwtn
     cpyfmwt cpyfmwtrn cpyfmwtwn cpyfmwtn
     cpyfewt cpyfewtrn cpyfewtwn cpyfewtn

     cpyfpt cpyfptrn cpyfptwn cpyfptn
     cpyfmt cpyfmtrn cpyfmtwn cpyfmtn
     cpyfet cpyfetrn cpyfetwn cpyfetn.  */
  MOPS_CPY_INSN ("cpyf", 0x19000400, 0xffe0fc00),

  /* cpyp cpyprn cpypwn cpypn
     cpym cpymrn cpymwn cpymn
     cpye cpyern cpyewn cpyen

     cpyprt cpyprtrn cpyprtwn cpyprtn
     cpymrt cpymrtrn cpymrtwn cpymrtn
     cpyert cpyertrn cpyertwn cpyertn

     cpypwt cpypwtrn cpypwtwn cpypwtn
     cpymwt cpymwtrn cpymwtwn cpymwtn
     cpyewt cpyewtrn cpyewtwn cpyewtn

     cpypt cpyptrn cpyptwn cpyptn
     cpymt cpymtrn cpymtwn cpymtn
     cpyet cpyetrn cpyetwn cpyetn.  */
  MOPS_CPY_INSN ("cpy", 0x1d000400, 0xffe0fc00),

  /* setp setpt setpn setptn
     setm setmt setmn setmtn
     sete setet seten setetn  */
  MOPS_SET_INSN ("set", 0x19c00400, 0xffe0fc00, MOPS_INSN),

  /* setgp setgpt setgpn setgptn
     setgm setgmt setgmn setgmtn
     setge setget setgen setgetn  */
  MOPS_SET_INSN ("setg", 0x1dc00400, 0xffe0fc00, MOPS_MEMTAG_INSN),

  HBC_INSN ("bc.c", 0x54000010, 0xff000010, condbranch, OP1 (ADDR_PCREL19), QL_PCREL_NIL, F_COND),

/* CSSC with immediates.  */
  CSSC_INSN ("smax", 0x11c00000, 0x7ffc0000, OP3 (Rd, Rn, CSSC_SIMM8), QL_R2NIL, F_SF),
  CSSC_INSN ("umax", 0x11c40000, 0x7ffc0000, OP3 (Rd, Rn, CSSC_UIMM8), QL_R2NIL, F_SF),
  CSSC_INSN ("smin", 0x11c80000, 0x7ffc0000, OP3 (Rd, Rn, CSSC_SIMM8), QL_R2NIL, F_SF),
  CSSC_INSN ("umin", 0x11cc0000, 0x7ffc0000, OP3 (Rd, Rn, CSSC_UIMM8), QL_R2NIL, F_SF),

/* CSSC with registers only.  */
  CSSC_INSN ("abs",  0x5ac02000, 0x7ffffc00, OP2 (Rd, Rn), QL_I2SAME, F_SF),
  CSSC_INSN ("cnt",  0x5ac01c00, 0x7ffffc00, OP2 (Rd, Rn), QL_I2SAME, F_SF),
  CSSC_INSN ("ctz",  0x5ac01800, 0x7ffffc00, OP2 (Rd, Rn), QL_I2SAME, F_SF),
  CSSC_INSN ("smax", 0x1ac06000, 0x7fe0fc00, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF),
  CSSC_INSN ("umax", 0x1ac06400, 0x7fe0fc00, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF),
  CSSC_INSN ("smin", 0x1ac06800, 0x7fe0fc00, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF),
  CSSC_INSN ("umin", 0x1ac06c00, 0x7fe0fc00, OP3 (Rd, Rn, Rm), QL_I3SAMER, F_SF),

  {0, 0, 0, 0, 0, 0, {}, {}, 0, 0, 0, NULL},
};

#ifdef AARCH64_OPERANDS
#undef AARCH64_OPERANDS
#endif

/* Macro-based operand decription; this will be fed into aarch64-gen for it
   to generate the structure aarch64_operands and the function
   aarch64_insert_operand and aarch64_extract_operand.

   These inserters and extracters in the description execute the conversion
   between the aarch64_opnd_info and value in the operand-related instruction
   field(s).  */

/* Y expects arguments (left to right) to be operand class, inserter/extractor
   name suffix, operand name, flags, related bitfield(s) and description.
   X only differs from Y by having the operand inserter and extractor names
   listed separately.  */

#define AARCH64_OPERANDS						\
    Y(INT_REG, regno, "Rd", 0, F(FLD_Rd), "an integer register")	\
    Y(INT_REG, regno, "Rn", 0, F(FLD_Rn), "an integer register")	\
    Y(INT_REG, regno, "Rm", 0, F(FLD_Rm), "an integer register")	\
    Y(INT_REG, regno, "Rt", 0, F(FLD_Rt), "an integer register")	\
    Y(INT_REG, regno, "Rt2", 0, F(FLD_Rt2), "an integer register")	\
    Y(INT_REG, regno, "Rt_LS64", 0, F(FLD_Rt), "an integer register")	\
    Y(INT_REG, regno, "Rt_SP", OPD_F_MAYBE_SP, F(FLD_Rt),		\
      "an integer or stack pointer register")				\
    Y(INT_REG, regno, "Rs", 0, F(FLD_Rs), "an integer register")	\
    Y(INT_REG, regno, "Ra", 0, F(FLD_Ra), "an integer register")	\
    X(INT_REG, ins_regno, ext_regrt_sysins, "Rt_SYS", 0, F(FLD_Rt),	\
      "an integer register")						\
    Y(INT_REG, regno, "Rd_SP", OPD_F_MAYBE_SP, F(FLD_Rd),		\
      "an integer or stack pointer register")				\
    Y(INT_REG, regno, "Rn_SP", OPD_F_MAYBE_SP, F(FLD_Rn),		\
      "an integer or stack pointer register")				\
    Y(INT_REG, regno, "Rm_SP", OPD_F_MAYBE_SP, F(FLD_Rm),		\
      "an integer or stack pointer register")				\
    X(INT_REG, 0, ext_regno_pair, "PAIRREG", 0, F(),			\
      "the second reg of a pair")					\
    Y(MODIFIED_REG, reg_extended, "Rm_EXT", 0, F(),			\
      "an integer register with optional extension")			\
    Y(MODIFIED_REG, reg_shifted, "Rm_SFT", 0, F(),			\
      "an integer register with optional shift")			\
    Y(FP_REG, regno, "Fd", 0, F(FLD_Rd), "a floating-point register")	\
    Y(FP_REG, regno, "Fn", 0, F(FLD_Rn), "a floating-point register")	\
    Y(FP_REG, regno, "Fm", 0, F(FLD_Rm), "a floating-point register")	\
    Y(FP_REG, regno, "Fa", 0, F(FLD_Ra), "a floating-point register")	\
    Y(FP_REG, ft, "Ft", 0, F(FLD_Rt), "a floating-point register")	\
    Y(FP_REG, regno, "Ft2", 0, F(FLD_Rt2), "a floating-point register")	\
    Y(SISD_REG, regno, "Sd", 0, F(FLD_Rd), "a SIMD scalar register")	\
    Y(SISD_REG, regno, "Sn", 0, F(FLD_Rn), "a SIMD scalar register")	\
    Y(SISD_REG, regno, "Sm", 0, F(FLD_Rm), "a SIMD scalar register")	\
    Y(SIMD_REG, regno, "Va", 0, F(FLD_Ra), "a SIMD vector register")	\
    Y(SIMD_REG, regno, "Vd", 0, F(FLD_Rd), "a SIMD vector register")	\
    Y(SIMD_REG, regno, "Vn", 0, F(FLD_Rn), "a SIMD vector register")	\
    Y(SIMD_REG, regno, "Vm", 0, F(FLD_Rm), "a SIMD vector register")	\
    Y(FP_REG, regno, "VdD1", 0, F(FLD_Rd),				\
      "the top half of a 128-bit FP/SIMD register")			\
    Y(FP_REG, regno, "VnD1", 0, F(FLD_Rn),				\
      "the top half of a 128-bit FP/SIMD register")			\
    Y(SIMD_ELEMENT, reglane, "Ed", 0, F(FLD_Rd),			\
      "a SIMD vector element")						\
    Y(SIMD_ELEMENT, reglane, "En", 0, F(FLD_Rn),			\
      "a SIMD vector element")						\
    Y(SIMD_ELEMENT, reglane, "Em", 0, F(FLD_Rm),			\
      "a SIMD vector element")						\
    Y(SIMD_ELEMENT, reglane, "Em16", 0, F(FLD_Rm),			\
      "a SIMD vector element limited to V0-V15")			\
    Y(SIMD_REGLIST, reglist, "LVn", 0, F(FLD_Rn),			\
      "a SIMD vector register list")					\
    Y(SIMD_REGLIST, ldst_reglist, "LVt", 0, F(),			\
      "a SIMD vector register list")					\
    Y(SIMD_REGLIST, ldst_reglist_r, "LVt_AL", 0, F(),			\
      "a SIMD vector register list")					\
    Y(SIMD_REGLIST, ldst_elemlist, "LEt", 0, F(),			\
      "a SIMD vector element list")					\
    Y(IMMEDIATE, imm, "CRn", 0, F(FLD_CRn),				\
      "a 4-bit opcode field named for historical reasons C0 - C15")	\
    Y(IMMEDIATE, imm, "CRm", 0, F(FLD_CRm),				\
      "a 4-bit opcode field named for historical reasons C0 - C15")	\
    Y(IMMEDIATE, imm, "IDX", 0, F(FLD_imm4_11),				\
      "an immediate as the index of the least significant byte")	\
    Y(IMMEDIATE, imm, "MASK", 0, F(FLD_imm4_0),				\
      "an immediate as the index of the least significant byte")	\
    Y(IMMEDIATE, advsimd_imm_shift, "IMM_VLSL", 0, F(),			\
      "a left shift amount for an AdvSIMD register")			\
    Y(IMMEDIATE, advsimd_imm_shift, "IMM_VLSR", 0, F(),			\
      "a right shift amount for an AdvSIMD register")			\
    Y(IMMEDIATE, advsimd_imm_modified, "SIMD_IMM", 0, F(),		\
      "an immediate")							\
    Y(IMMEDIATE, advsimd_imm_modified, "SIMD_IMM_SFT", 0, F(),		\
      "an 8-bit unsigned immediate with optional shift")		\
    Y(IMMEDIATE, advsimd_imm_modified, "SIMD_FPIMM", 0, F(),		\
      "an 8-bit floating-point constant")				\
    X(IMMEDIATE, 0, ext_shll_imm, "SHLL_IMM", 0, F(),			\
      "an immediate shift amount of 8, 16 or 32")			\
    X(IMMEDIATE, 0, 0, "IMM0", 0, F(), "0")				\
    X(IMMEDIATE, 0, 0, "FPIMM0", 0, F(), "0.0")				\
    Y(IMMEDIATE, fpimm, "FPIMM", 0, F(FLD_imm8),			\
      "an 8-bit floating-point constant")				\
    Y(IMMEDIATE, imm, "IMMR", 0, F(FLD_immr),				\
      "the right rotate amount")					\
    Y(IMMEDIATE, imm, "IMMS", 0, F(FLD_imm6_10),			\
      "the leftmost bit number to be moved from the source")		\
    Y(IMMEDIATE, imm, "WIDTH", 0, F(FLD_imm6_10),			\
      "the width of the bit-field")					\
    Y(IMMEDIATE, imm, "IMM", 0, F(FLD_imm6_10), "an immediate")         \
    Y(IMMEDIATE, imm, "IMM_2", 0, F(FLD_imm6_15), "an immediate")       \
    Y(IMMEDIATE, imm, "UIMM3_OP1", 0, F(FLD_op1),			\
      "a 3-bit unsigned immediate")					\
    Y(IMMEDIATE, imm, "UIMM3_OP2", 0, F(FLD_op2),			\
      "a 3-bit unsigned immediate")					\
    Y(IMMEDIATE, imm, "UIMM4", 0, F(FLD_CRm),				\
      "a 4-bit unsigned immediate")					\
    Y(IMMEDIATE, imm, "UIMM4_ADDG", 0, F(FLD_imm4_10),			\
      "a 4-bit unsigned Logical Address Tag modifier")			\
    Y(IMMEDIATE, imm, "UIMM7", 0, F(FLD_CRm, FLD_op2),			\
      "a 7-bit unsigned immediate")					\
    Y(IMMEDIATE, imm, "UIMM10", OPD_F_SHIFT_BY_4, F(FLD_immr),		\
      "a 10-bit unsigned multiple of 16")				\
    Y(IMMEDIATE, imm, "BIT_NUM", 0, F(FLD_b5, FLD_b40),			\
      "the bit number to be tested")					\
    Y(IMMEDIATE, imm, "EXCEPTION", 0, F(FLD_imm16_5),			\
      "a 16-bit unsigned immediate")					\
    Y(IMMEDIATE, imm, "UNDEFINED", 0, F(FLD_imm16_0),			\
      "a 16-bit unsigned immediate")					\
    Y(IMMEDIATE, imm, "CCMP_IMM", 0, F(FLD_imm5),			\
      "a 5-bit unsigned immediate")					\
    Y(IMMEDIATE, imm, "SIMM5", OPD_F_SEXT, F(FLD_imm5),			\
      "a 5-bit signed immediate")					\
    Y(IMMEDIATE, imm, "NZCV", 0, F(FLD_nzcv),				\
      "a flag bit specifier giving an alternative value for each flag")	\
    Y(IMMEDIATE, limm, "LIMM", 0, F(FLD_N,FLD_immr,FLD_imms),		\
      "Logical immediate")						\
    Y(IMMEDIATE, aimm, "AIMM", 0, F(FLD_shift,FLD_imm12),		\
      "a 12-bit unsigned immediate with optional left shift of 12 bits")\
    Y(IMMEDIATE, imm_half, "HALF", 0, F(FLD_imm16_5),			\
      "a 16-bit immediate with optional left shift")			\
    Y(IMMEDIATE, fbits, "FBITS", 0, F(FLD_scale),			\
      "the number of bits after the binary point in the fixed-point value")\
    X(IMMEDIATE, 0, 0, "IMM_MOV", 0, F(), "an immediate")		\
    Y(IMMEDIATE, imm_rotate2, "IMM_ROT1", 0, F(FLD_rotate1),		\
      "a 2-bit rotation specifier for complex arithmetic operations")	\
    Y(IMMEDIATE, imm_rotate2, "IMM_ROT2", 0, F(FLD_rotate2),		\
      "a 2-bit rotation specifier for complex arithmetic operations")	\
    Y(IMMEDIATE, imm_rotate1, "IMM_ROT3", 0, F(FLD_rotate3),		\
      "a 1-bit rotation specifier for complex arithmetic operations")	\
    Y(COND, cond, "COND", 0, F(), "a condition")			\
    Y(COND, cond, "COND1", 0, F(),					\
      "one of the standard conditions, excluding AL and NV.")		\
    X(ADDRESS, 0, ext_imm, "ADDR_ADRP", OPD_F_SEXT, F(FLD_immhi, FLD_immlo),\
      "21-bit PC-relative address of a 4KB page")			\
    Y(ADDRESS, imm, "ADDR_PCREL14", OPD_F_SEXT | OPD_F_SHIFT_BY_2,	\
      F(FLD_imm14), "14-bit PC-relative address")			\
    Y(ADDRESS, imm, "ADDR_PCREL19", OPD_F_SEXT | OPD_F_SHIFT_BY_2,	\
      F(FLD_imm19), "19-bit PC-relative address")			\
    Y(ADDRESS, imm, "ADDR_PCREL21", OPD_F_SEXT, F(FLD_immhi,FLD_immlo),	\
      "21-bit PC-relative address")					\
    Y(ADDRESS, imm, "ADDR_PCREL26", OPD_F_SEXT | OPD_F_SHIFT_BY_2,	\
      F(FLD_imm26), "26-bit PC-relative address")			\
    Y(ADDRESS, addr_simple, "ADDR_SIMPLE", 0, F(),			\
      "an address with base register (no offset)")			\
    Y(ADDRESS, addr_regoff, "ADDR_REGOFF", 0, F(),			\
      "an address with register offset")				\
    Y(ADDRESS, addr_simm, "ADDR_SIMM7", 0, F(FLD_imm7,FLD_index2),	\
      "an address with 7-bit signed immediate offset")			\
    Y(ADDRESS, addr_simm, "ADDR_SIMM9", 0, F(FLD_imm9,FLD_index),	\
      "an address with 9-bit signed immediate offset")			\
    Y(ADDRESS, addr_simm, "ADDR_SIMM9_2", 0, F(FLD_imm9,FLD_index),	\
      "an address with 9-bit negative or unaligned immediate offset")	\
    Y(ADDRESS, addr_simm10, "ADDR_SIMM10", 0, F(FLD_Rn,FLD_S_imm10,FLD_imm9,FLD_index),\
      "an address with an optional 10-bit scaled, signed immediate offset")		\
    Y(ADDRESS, addr_simm, "ADDR_SIMM11", 0, F(FLD_imm7,FLD_index2),\
      "an address with 11-bit signed immediate (multiple of 16) offset")\
    Y(ADDRESS, addr_uimm12, "ADDR_UIMM12", 0, F(FLD_Rn,FLD_imm12),	\
      "an address with scaled, unsigned immediate offset")		\
    Y(ADDRESS, addr_simm, "ADDR_SIMM13", 0, F(FLD_imm9,FLD_index),\
      "an address with 13-bit signed immediate (multiple of 16) offset")\
    Y(ADDRESS, addr_simple, "SIMD_ADDR_SIMPLE", 0, F(),			\
      "an address with base register (no offset)")			\
    Y(ADDRESS, addr_offset, "ADDR_OFFSET", 0, F(FLD_Rn,FLD_imm9,FLD_index),\
      "an address with an optional 8-bit signed immediate offset")	\
    Y(ADDRESS, simd_addr_post, "SIMD_ADDR_POST", 0, F(),		\
      "a post-indexed address with immediate or register increment")	\
    Y(SYSTEM, sysreg, "SYSREG", 0, F(), "a system register")		\
    Y(SYSTEM, pstatefield, "PSTATEFIELD", 0, F(),			\
      "a PSTATE field name")						\
    Y(SYSTEM, sysins_op, "SYSREG_AT", 0, F(),				\
      "an address translation operation specifier")			\
    Y(SYSTEM, sysins_op, "SYSREG_DC", 0, F(),				\
      "a data cache maintenance operation specifier")			\
    Y(SYSTEM, sysins_op, "SYSREG_IC", 0, F(),				\
      "an instruction cache maintenance operation specifier")		\
    Y(SYSTEM, sysins_op, "SYSREG_TLBI", 0, F(),				\
      "a TBL invalidation operation specifier")				\
    Y(SYSTEM, sysins_op, "SYSREG_SR", 0, F(),				\
      "a Speculation Restriction option name (RCTX)")			\
    Y(SYSTEM, barrier, "BARRIER", 0, F(),				\
      "a barrier option name")						\
    Y(SYSTEM, barrier_dsb_nxs, "BARRIER_DSB_NXS", 0, F(),				\
      "the DSB nXS option qualifier name SY, ISH, NSH, OSH or an optional 5-bit unsigned immediate")	\
    Y(SYSTEM, barrier, "BARRIER_ISB", 0, F(),				\
      "the ISB option name SY or an optional 4-bit unsigned immediate")	\
    Y(SYSTEM, prfop, "PRFOP", 0, F(),					\
      "a prefetch operation specifier")					\
    Y(SYSTEM, imm, "RPRFMOP", 0,					\
      F(FLD_imm1_15, FLD_imm2_12, FLD_imm3_0),	        		\
      "a range prefetch operation specifier")				\
    Y(SYSTEM, none, "BARRIER_PSB", 0, F (),				\
      "the PSB/TSB option name CSYNC")					\
    Y(SYSTEM, hint, "BTI", 0, F (),					\
      "BTI targets j/c/jc")						\
    Y(ADDRESS, sve_addr_ri_s4, "SVE_ADDR_RI_S4x16",			\
      4 << OPD_F_OD_LSB, F(FLD_Rn),					\
      "an address with a 4-bit signed offset, multiplied by 16")	\
    Y(ADDRESS, sve_addr_ri_s4, "SVE_ADDR_RI_S4x32",			\
      5 << OPD_F_OD_LSB, F(FLD_Rn),					\
      "an address with a 4-bit signed offset, multiplied by 32")	\
    Y(ADDRESS, sve_addr_ri_s4xvl, "SVE_ADDR_RI_S4xVL",			\
      0 << OPD_F_OD_LSB, F(FLD_Rn),					\
      "an address with a 4-bit signed offset, multiplied by VL")	\
    Y(ADDRESS, sve_addr_ri_s4xvl, "SVE_ADDR_RI_S4x2xVL",		\
      1 << OPD_F_OD_LSB, F(FLD_Rn),					\
      "an address with a 4-bit signed offset, multiplied by 2*VL")	\
    Y(ADDRESS, sve_addr_ri_s4xvl, "SVE_ADDR_RI_S4x3xVL",		\
      2 << OPD_F_OD_LSB, F(FLD_Rn),					\
      "an address with a 4-bit signed offset, multiplied by 3*VL")	\
    Y(ADDRESS, sve_addr_ri_s4xvl, "SVE_ADDR_RI_S4x4xVL",		\
      3 << OPD_F_OD_LSB, F(FLD_Rn),					\
      "an address with a 4-bit signed offset, multiplied by 4*VL")	\
    Y(ADDRESS, sve_addr_ri_s6xvl, "SVE_ADDR_RI_S6xVL",			\
      0 << OPD_F_OD_LSB, F(FLD_Rn),					\
      "an address with a 6-bit signed offset, multiplied by VL")	\
    Y(ADDRESS, sve_addr_ri_s9xvl, "SVE_ADDR_RI_S9xVL",			\
      0 << OPD_F_OD_LSB, F(FLD_Rn),					\
      "an address with a 9-bit signed offset, multiplied by VL")	\
    Y(ADDRESS, sve_addr_ri_u6, "SVE_ADDR_RI_U6", 0 << OPD_F_OD_LSB,	\
      F(FLD_Rn), "an address with a 6-bit unsigned offset")		\
    Y(ADDRESS, sve_addr_ri_u6, "SVE_ADDR_RI_U6x2", 1 << OPD_F_OD_LSB,	\
      F(FLD_Rn),							\
      "an address with a 6-bit unsigned offset, multiplied by 2")	\
    Y(ADDRESS, sve_addr_ri_u6, "SVE_ADDR_RI_U6x4", 2 << OPD_F_OD_LSB,	\
      F(FLD_Rn),							\
      "an address with a 6-bit unsigned offset, multiplied by 4")	\
    Y(ADDRESS, sve_addr_ri_u6, "SVE_ADDR_RI_U6x8", 3 << OPD_F_OD_LSB,	\
      F(FLD_Rn),							\
      "an address with a 6-bit unsigned offset, multiplied by 8")	\
    Y(ADDRESS, sve_addr_rr_lsl, "SVE_ADDR_R", 0 << OPD_F_OD_LSB,	\
      F(FLD_Rn,FLD_Rm), "an address with an optional scalar register offset")	\
    Y(ADDRESS, sve_addr_rr_lsl, "SVE_ADDR_RR", 0 << OPD_F_OD_LSB,	\
      F(FLD_Rn,FLD_Rm), "an address with a scalar register offset")	\
    Y(ADDRESS, sve_addr_rr_lsl, "SVE_ADDR_RR_LSL1", 1 << OPD_F_OD_LSB,	\
      F(FLD_Rn,FLD_Rm), "an address with a scalar register offset")	\
    Y(ADDRESS, sve_addr_rr_lsl, "SVE_ADDR_RR_LSL2", 2 << OPD_F_OD_LSB,	\
      F(FLD_Rn,FLD_Rm), "an address with a scalar register offset")	\
    Y(ADDRESS, sve_addr_rr_lsl, "SVE_ADDR_RR_LSL3", 3 << OPD_F_OD_LSB,	\
      F(FLD_Rn,FLD_Rm), "an address with a scalar register offset")	\
    Y(ADDRESS, sve_addr_rr_lsl, "SVE_ADDR_RR_LSL4", 4 << OPD_F_OD_LSB,	\
      F(FLD_Rn,FLD_Rm), "an address with a scalar register offset")	\
    Y(ADDRESS, sve_addr_rr_lsl, "SVE_ADDR_RX",				\
      (0 << OPD_F_OD_LSB) | OPD_F_NO_ZR, F(FLD_Rn,FLD_Rm),		\
      "an address with a scalar register offset")			\
    Y(ADDRESS, sve_addr_rr_lsl, "SVE_ADDR_RX_LSL1",			\
      (1 << OPD_F_OD_LSB) | OPD_F_NO_ZR, F(FLD_Rn,FLD_Rm),		\
      "an address with a scalar register offset")			\
    Y(ADDRESS, sve_addr_rr_lsl, "SVE_ADDR_RX_LSL2",			\
      (2 << OPD_F_OD_LSB) | OPD_F_NO_ZR, F(FLD_Rn,FLD_Rm),		\
      "an address with a scalar register offset")			\
    Y(ADDRESS, sve_addr_rr_lsl, "SVE_ADDR_RX_LSL3",			\
      (3 << OPD_F_OD_LSB) | OPD_F_NO_ZR, F(FLD_Rn,FLD_Rm),		\
      "an address with a scalar register offset")			\
    Y(ADDRESS, sve_addr_rr_lsl, "SVE_ADDR_ZX",				\
      0 << OPD_F_OD_LSB , F(FLD_SVE_Zn,FLD_Rm),				\
      "vector of address with a scalar register offset")		\
    Y(ADDRESS, sve_addr_rr_lsl, "SVE_ADDR_RZ", 0 << OPD_F_OD_LSB,	\
      F(FLD_Rn,FLD_SVE_Zm_16),						\
      "an address with a vector register offset")			\
    Y(ADDRESS, sve_addr_rr_lsl, "SVE_ADDR_RZ_LSL1", 1 << OPD_F_OD_LSB,	\
      F(FLD_Rn,FLD_SVE_Zm_16),						\
      "an address with a vector register offset")			\
    Y(ADDRESS, sve_addr_rr_lsl, "SVE_ADDR_RZ_LSL2", 2 << OPD_F_OD_LSB,	\
      F(FLD_Rn,FLD_SVE_Zm_16),						\
      "an address with a vector register offset")			\
    Y(ADDRESS, sve_addr_rr_lsl, "SVE_ADDR_RZ_LSL3", 3 << OPD_F_OD_LSB,	\
      F(FLD_Rn,FLD_SVE_Zm_16),						\
      "an address with a vector register offset")			\
    Y(ADDRESS, sve_addr_rz_xtw, "SVE_ADDR_RZ_XTW_14",			\
      0 << OPD_F_OD_LSB, F(FLD_Rn,FLD_SVE_Zm_16,FLD_SVE_xs_14),		\
      "an address with a vector register offset")			\
    Y(ADDRESS, sve_addr_rz_xtw, "SVE_ADDR_RZ_XTW_22",			\
      0 << OPD_F_OD_LSB, F(FLD_Rn,FLD_SVE_Zm_16,FLD_SVE_xs_22),		\
      "an address with a vector register offset")			\
    Y(ADDRESS, sve_addr_rz_xtw, "SVE_ADDR_RZ_XTW1_14",			\
      1 << OPD_F_OD_LSB, F(FLD_Rn,FLD_SVE_Zm_16,FLD_SVE_xs_14),		\
      "an address with a vector register offset")			\
    Y(ADDRESS, sve_addr_rz_xtw, "SVE_ADDR_RZ_XTW1_22",			\
      1 << OPD_F_OD_LSB, F(FLD_Rn,FLD_SVE_Zm_16,FLD_SVE_xs_22),		\
      "an address with a vector register offset")			\
    Y(ADDRESS, sve_addr_rz_xtw, "SVE_ADDR_RZ_XTW2_14",			\
      2 << OPD_F_OD_LSB, F(FLD_Rn,FLD_SVE_Zm_16,FLD_SVE_xs_14),		\
      "an address with a vector register offset")			\
    Y(ADDRESS, sve_addr_rz_xtw, "SVE_ADDR_RZ_XTW2_22",			\
      2 << OPD_F_OD_LSB, F(FLD_Rn,FLD_SVE_Zm_16,FLD_SVE_xs_22),		\
      "an address with a vector register offset")			\
    Y(ADDRESS, sve_addr_rz_xtw, "SVE_ADDR_RZ_XTW3_14",			\
      3 << OPD_F_OD_LSB, F(FLD_Rn,FLD_SVE_Zm_16,FLD_SVE_xs_14),		\
      "an address with a vector register offset")			\
    Y(ADDRESS, sve_addr_rz_xtw, "SVE_ADDR_RZ_XTW3_22",			\
      3 << OPD_F_OD_LSB, F(FLD_Rn,FLD_SVE_Zm_16,FLD_SVE_xs_22),		\
      "an address with a vector register offset")			\
    Y(ADDRESS, sve_addr_zi_u5, "SVE_ADDR_ZI_U5", 0 << OPD_F_OD_LSB,	\
      F(FLD_SVE_Zn), "an address with a 5-bit unsigned offset")		\
    Y(ADDRESS, sve_addr_zi_u5, "SVE_ADDR_ZI_U5x2", 1 << OPD_F_OD_LSB,	\
      F(FLD_SVE_Zn),							\
      "an address with a 5-bit unsigned offset, multiplied by 2")	\
    Y(ADDRESS, sve_addr_zi_u5, "SVE_ADDR_ZI_U5x4", 2 << OPD_F_OD_LSB,	\
      F(FLD_SVE_Zn),							\
      "an address with a 5-bit unsigned offset, multiplied by 4")	\
    Y(ADDRESS, sve_addr_zi_u5, "SVE_ADDR_ZI_U5x8", 3 << OPD_F_OD_LSB,	\
      F(FLD_SVE_Zn),							\
      "an address with a 5-bit unsigned offset, multiplied by 8")	\
    Y(ADDRESS, sve_addr_zz_lsl, "SVE_ADDR_ZZ_LSL", 0,			\
      F(FLD_SVE_Zn,FLD_SVE_Zm_16),					\
      "an address with a vector register offset")			\
    Y(ADDRESS, sve_addr_zz_sxtw, "SVE_ADDR_ZZ_SXTW", 0,			\
      F(FLD_SVE_Zn,FLD_SVE_Zm_16),					\
      "an address with a vector register offset")			\
    Y(ADDRESS, sve_addr_zz_uxtw, "SVE_ADDR_ZZ_UXTW", 0,			\
      F(FLD_SVE_Zn,FLD_SVE_Zm_16),					\
      "an address with a vector register offset")			\
    Y(IMMEDIATE, sve_aimm, "SVE_AIMM", 0, F(FLD_SVE_imm9),		\
      "a 9-bit unsigned arithmetic operand")				\
    Y(IMMEDIATE, sve_asimm, "SVE_ASIMM", 0, F(FLD_SVE_imm9),		\
      "a 9-bit signed arithmetic operand")				\
    Y(IMMEDIATE, fpimm, "SVE_FPIMM8", 0, F(FLD_SVE_imm8),		\
      "an 8-bit floating-point immediate")				\
    Y(IMMEDIATE, sve_float_half_one, "SVE_I1_HALF_ONE", 0,		\
      F(FLD_SVE_i1), "either 0.5 or 1.0")				\
    Y(IMMEDIATE, sve_float_half_two, "SVE_I1_HALF_TWO", 0,		\
      F(FLD_SVE_i1), "either 0.5 or 2.0")				\
    Y(IMMEDIATE, sve_float_zero_one, "SVE_I1_ZERO_ONE", 0,		\
      F(FLD_SVE_i1), "either 0.0 or 1.0")				\
    Y(IMMEDIATE, imm_rotate1, "SVE_IMM_ROT1", 0, F(FLD_SVE_rot1),	\
      "a 1-bit rotation specifier for complex arithmetic operations")	\
    Y(IMMEDIATE, imm_rotate2, "SVE_IMM_ROT2", 0, F(FLD_SVE_rot2),	\
      "a 2-bit rotation specifier for complex arithmetic operations")	\
    Y(IMMEDIATE, imm_rotate1, "SVE_IMM_ROT3", 0, F(FLD_SVE_rot3),	\
      "a 1-bit rotation specifier for complex arithmetic operations")	\
    Y(IMMEDIATE, inv_limm, "SVE_INV_LIMM", 0,				\
      F(FLD_SVE_N,FLD_SVE_immr,FLD_SVE_imms),				\
      "an inverted 13-bit logical immediate")				\
    Y(IMMEDIATE, limm, "SVE_LIMM", 0,					\
      F(FLD_SVE_N,FLD_SVE_immr,FLD_SVE_imms),				\
      "a 13-bit logical immediate")					\
    Y(IMMEDIATE, sve_limm_mov, "SVE_LIMM_MOV", 0,			\
      F(FLD_SVE_N,FLD_SVE_immr,FLD_SVE_imms),				\
      "a 13-bit logical move immediate")				\
    Y(IMMEDIATE, imm, "SVE_PATTERN", 0, F(FLD_SVE_pattern),		\
      "an enumeration value such as POW2")				\
    Y(IMMEDIATE, sve_scale, "SVE_PATTERN_SCALED", 0,			\
      F(FLD_SVE_pattern), "an enumeration value such as POW2")		\
    Y(IMMEDIATE, imm, "SVE_PRFOP", 0, F(FLD_SVE_prfop),			\
      "an enumeration value such as PLDL1KEEP")				\
    Y(PRED_REG, regno, "SVE_Pd", 0, F(FLD_SVE_Pd),			\
      "an SVE predicate register")					\
    Y(PRED_REG, regno, "SVE_PNd", 0, F(FLD_SVE_Pd),			\
      "an SVE predicate-as-counter register")				\
    Y(PRED_REG, regno, "SVE_Pg3", 0, F(FLD_SVE_Pg3),			\
      "an SVE predicate register")					\
    Y(PRED_REG, regno, "SVE_Pg4_5", 0, F(FLD_SVE_Pg4_5),		\
      "an SVE predicate register")					\
    Y(PRED_REG, regno, "SVE_Pg4_10", 0, F(FLD_SVE_Pg4_10),		\
      "an SVE predicate register")					\
    Y(PRED_REG, regno, "SVE_PNg4_10", 0, F(FLD_SVE_Pg4_10),		\
      "an SVE predicate-as-counter register")				\
    Y(PRED_REG, regno, "SVE_Pg4_16", 0, F(FLD_SVE_Pg4_16),		\
      "an SVE predicate register")					\
    Y(PRED_REG, regno, "SVE_Pm", 0, F(FLD_SVE_Pm),			\
      "an SVE predicate register")					\
    Y(PRED_REG, regno, "SVE_Pn", 0, F(FLD_SVE_Pn),			\
      "an SVE predicate register")					\
    Y(PRED_REG, regno, "SVE_PNn", 0, F(FLD_SVE_Pn),			\
      "an SVE predicate register")					\
    Y(PRED_REG, regno, "SVE_Pt", 0, F(FLD_SVE_Pt),			\
      "an SVE predicate register")					\
    Y(PRED_REG, regno, "SVE_PNt", 0, F(FLD_SVE_Pt),			\
      "an SVE predicate register")					\
    Y(INT_REG, regno, "SVE_Rm", 0, F(FLD_SVE_Rm),			\
      "an integer register or zero")					\
    Y(INT_REG, regno, "SVE_Rn_SP", OPD_F_MAYBE_SP, F(FLD_SVE_Rn),	\
      "an integer register or SP")					\
    Y(IMMEDIATE, sve_shlimm, "SVE_SHLIMM_PRED", 0,			\
      F(FLD_SVE_tszh,FLD_SVE_imm5), "a shift-left immediate operand")	\
    Y(IMMEDIATE, sve_shlimm, "SVE_SHLIMM_UNPRED", 0,			\
      F(FLD_SVE_tszh,FLD_imm5), "a shift-left immediate operand")	\
    Y(IMMEDIATE, sve_shlimm, "SVE_SHLIMM_UNPRED_22", 0,			\
      F(FLD_SVE_sz, FLD_SVE_tszl_19, FLD_SVE_imm3),			\
      "a shift-left immediate operand")					\
    Y(IMMEDIATE, sve_shrimm, "SVE_SHRIMM_PRED", 1 << OPD_F_OD_LSB,	\
      F(FLD_SVE_tszh,FLD_SVE_imm5), "a shift-right immediate operand")	\
    Y(IMMEDIATE, sve_shrimm, "SVE_SHRIMM_UNPRED", 1 << OPD_F_OD_LSB,	\
      F(FLD_SVE_tszh,FLD_imm5), "a shift-right immediate operand")	\
    Y(IMMEDIATE, sve_shrimm, "SVE_SHRIMM_UNPRED_22", 2 << OPD_F_OD_LSB,	\
      F(FLD_SVE_sz, FLD_SVE_tszl_19, FLD_SVE_imm3),			\
      "a shift-right immediate operand")				\
    Y(IMMEDIATE, imm, "SVE_SIMM5", OPD_F_SEXT, F(FLD_SVE_imm5),		\
      "a 5-bit signed immediate")					\
    Y(IMMEDIATE, imm, "SVE_SIMM5B", OPD_F_SEXT, F(FLD_SVE_imm5b),	\
      "a 5-bit signed immediate")					\
    Y(IMMEDIATE, imm, "SVE_SIMM6", OPD_F_SEXT, F(FLD_SVE_imms),		\
      "a 6-bit signed immediate")					\
    Y(IMMEDIATE, imm, "SVE_SIMM8", OPD_F_SEXT, F(FLD_SVE_imm8),		\
      "an 8-bit signed immediate")					\
    Y(IMMEDIATE, imm, "SVE_UIMM3", 0, F(FLD_SVE_imm3),			\
      "a 3-bit unsigned immediate")					\
    Y(IMMEDIATE, imm, "SVE_UIMM7", 0, F(FLD_SVE_imm7),			\
      "a 7-bit unsigned immediate")					\
    Y(IMMEDIATE, imm, "SVE_UIMM8", 0, F(FLD_SVE_imm8),			\
      "an 8-bit unsigned immediate")					\
    Y(IMMEDIATE, imm, "SVE_UIMM8_53", 0, F(FLD_imm5,FLD_imm3_10),		\
      "an 8-bit unsigned immediate")					\
    Y(SIMD_REG, regno, "SVE_VZn", 0, F(FLD_SVE_Zn), "a SIMD register")	\
    Y(SIMD_REG, regno, "SVE_Vd", 0, F(FLD_SVE_Vd), "a SIMD register")	\
    Y(SIMD_REG, regno, "SVE_Vm", 0, F(FLD_SVE_Vm), "a SIMD register")	\
    Y(SIMD_REG, regno, "SVE_Vn", 0, F(FLD_SVE_Vn), "a SIMD register")	\
    Y(SVE_REG, regno, "SVE_Za_5", 0, F(FLD_SVE_Za_5),			\
      "an SVE vector register")						\
    Y(SVE_REG, regno, "SVE_Za_16", 0, F(FLD_SVE_Za_16),			\
      "an SVE vector register")						\
    Y(SVE_REG, regno, "SVE_Zd", 0, F(FLD_SVE_Zd),			\
      "an SVE vector register")						\
    Y(SVE_REG, regno, "SVE_Zm_5", 0, F(FLD_SVE_Zm_5),			\
      "an SVE vector register")						\
    Y(SVE_REG, regno, "SVE_Zm_16", 0, F(FLD_SVE_Zm_16),			\
      "an SVE vector register")						\
    Y(SVE_REG, sve_quad_index, "SVE_Zm3_INDEX",				\
      3 << OPD_F_OD_LSB, F(FLD_SVE_Zm_16),				\
      "an indexed SVE vector register")					\
    Y(SVE_REG, sve_quad_index, "SVE_Zm3_11_INDEX", 			\
      3 << OPD_F_OD_LSB, F(FLD_SVE_i3h2, FLD_SVE_i3l, FLD_SVE_imm3),    \
      "an indexed SVE vector register")					\
    Y(SVE_REG, sve_quad_index, "SVE_Zm3_19_INDEX", 			\
      3 << OPD_F_OD_LSB, F(FLD_imm2_19, FLD_SVE_imm3),			\
      "an indexed SVE vector register")					\
    Y(SVE_REG, sve_quad_index, "SVE_Zm3_22_INDEX", 			\
      3 << OPD_F_OD_LSB, F(FLD_SVE_i3h, FLD_SVE_Zm_16),			\
      "an indexed SVE vector register")					\
    Y(SVE_REG, sve_quad_index, "SVE_Zm4_11_INDEX", 			\
      4 << OPD_F_OD_LSB, F(FLD_SVE_i2h, FLD_SVE_i3l, FLD_SVE_imm4),     \
      "an indexed SVE vector register")					\
    Y(SVE_REG, sve_quad_index, "SVE_Zm4_INDEX", 			\
      4 << OPD_F_OD_LSB, F(FLD_SVE_Zm_16),				\
      "an indexed SVE vector register")					\
    Y(SVE_REG, regno, "SVE_Zn", 0, F(FLD_SVE_Zn),			\
      "an SVE vector register")						\
    Y(SVE_REG, sve_index, "SVE_Zn_INDEX", 0, F(FLD_SVE_Zn),		\
      "an indexed SVE vector register")					\
    Y(SVE_REGLIST, sve_reglist, "SVE_ZnxN", 0, F(FLD_SVE_Zn),		\
      "a list of SVE vector registers")					\
    Y(SVE_REG, regno, "SVE_Zt", 0, F(FLD_SVE_Zt),			\
      "an SVE vector register")						\
    Y(SVE_REGLIST, sve_reglist, "SVE_ZtxN", 0, F(FLD_SVE_Zt),		\
      "a list of SVE vector registers")					\
    Y(SVE_REGLIST, sve_aligned_reglist, "SME_Zdnx2", 2 << OPD_F_OD_LSB,	\
      F(FLD_SME_Zdn2), "a list of SVE vector registers")		\
    Y(SVE_REGLIST, sve_aligned_reglist, "SME_Zdnx4", 4 << OPD_F_OD_LSB,	\
      F(FLD_SME_Zdn4), "a list of SVE vector registers")		\
    Y(SVE_REG, regno, "SME_Zm", 0, F(FLD_SME_Zm),			\
      "an SVE vector register")						\
    Y(SVE_REGLIST, sve_aligned_reglist, "SME_Zmx2", 2 << OPD_F_OD_LSB,	\
      F(FLD_SME_Zm2), "a list of SVE vector registers")			\
    Y(SVE_REGLIST, sve_aligned_reglist, "SME_Zmx4", 4 << OPD_F_OD_LSB,	\
      F(FLD_SME_Zm4), "a list of SVE vector registers")			\
    Y(SVE_REGLIST, sve_aligned_reglist, "SME_Znx2", 2 << OPD_F_OD_LSB,	\
      F(FLD_SME_Zn2), "a list of SVE vector registers")			\
    Y(SVE_REGLIST, sve_aligned_reglist, "SME_Znx4", 4 << OPD_F_OD_LSB,	\
      F(FLD_SME_Zn4), "a list of SVE vector registers")			\
    Y(SVE_REGLIST, sve_strided_reglist, "SME_Ztx2_STRIDED",		\
      2 << OPD_F_OD_LSB, F(FLD_SME_ZtT, FLD_SME_Zt3),			\
      "a list of SVE vector registers")					\
    Y(SVE_REGLIST, sve_strided_reglist, "SME_Ztx4_STRIDED",		\
      4 << OPD_F_OD_LSB, F(FLD_SME_ZtT, FLD_SME_Zt2),			\
      "a list of SVE vector registers")					\
    Y(SVE_REG, regno, "SME_ZAda_2b", 0, F(FLD_SME_ZAda_2b),		\
      "an SME ZA tile ZA0-ZA3")						\
    Y(SVE_REG, regno, "SME_ZAda_3b", 0, F(FLD_SME_ZAda_3b),		\
      "an SME ZA tile ZA0-ZA7")						\
    Y(ZA_ACCESS, sme_za_hv_tiles, "SME_ZA_HV_idx_src", 0,		\
      F(FLD_SME_size_22,FLD_SME_Q,FLD_SME_V,FLD_SME_Rv,FLD_imm4_5),	\
      "an SME horizontal or vertical vector access register")		\
    Y(ZA_ACCESS, sme_za_hv_tiles_range, "SME_ZA_HV_idx_srcxN", 0,	\
      F(FLD_SME_V,FLD_SME_Rv,FLD_imm3_5),				\
      "an SME horizontal or vertical vector access register")		\
    Y(ZA_ACCESS, sme_za_hv_tiles, "SME_ZA_HV_idx_dest", 0,		\
      F(FLD_SME_size_22,FLD_SME_Q,FLD_SME_V,FLD_SME_Rv,FLD_imm4_0),	\
      "an SME horizontal or vertical vector access register")		\
    Y(ZA_ACCESS, sme_za_hv_tiles_range, "SME_ZA_HV_idx_destxN", 0,	\
      F(FLD_SME_V,FLD_SME_Rv,FLD_imm3_0),				\
      "an SME horizontal or vertical vector access register")		\
    Y(SVE_REGLIST, sve_aligned_reglist, "SME_Pdx2", 2 << OPD_F_OD_LSB,	\
      F(FLD_SME_Pdx2), "a list of SVE predicate registers")		\
    Y(SVE_REGLIST, sve_reglist, "SME_PdxN", 0, F(FLD_SVE_Pd),		\
      "a list of SVE predicate registers")				\
    Y(PRED_REG, regno, "SME_Pm", 0, F(FLD_SME_Pm),			\
      "an SVE predicate register")					\
    Y(PRED_REG, regno, "SME_PNd3", 8 << OPD_F_OD_LSB, F(FLD_SME_PNd3),	\
      "an SVE predicate-as-counter register")				\
    Y(PRED_REG, regno, "SME_PNg3", 8 << OPD_F_OD_LSB, F(FLD_SVE_Pg3),	\
      "an SVE predicate-as-counter register")				\
    Y(PRED_REG, regno, "SME_PNn", 0, F(FLD_SVE_Pn),			\
      "an SVE predicate-as-counter register")				\
    Y(SVE_REG, simple_index, "SME_PNn3_INDEX1", 8 << OPD_F_OD_LSB,	\
      F(FLD_SME_PNn3, FLD_imm1_8),					\
      "an indexed SVE predicate-as-counter register")			\
    Y(SVE_REG, simple_index, "SME_PNn3_INDEX2", 8 << OPD_F_OD_LSB,	\
      F(FLD_SME_PNn3, FLD_imm2_8),					\
      "an indexed SVE predicate-as-counter register")			\
    Y(SVE_REG, imm, "SME_list_of_64bit_tiles", 0,			\
      F(FLD_SME_zero_mask), "a list of 64-bit ZA element tiles")	\
    Y(ZA_ACCESS, sme_za_hv_tiles, "SME_ZA_HV_idx_ldstr", 0,		\
      F(FLD_SME_size_22,FLD_index2,FLD_SME_V,FLD_SME_Rv,FLD_imm4_0),	\
      "an SME horizontal or vertical vector access register")		\
    Y(ZA_ACCESS, sme_za_array, "SME_ZA_array_off1x4",			\
      4 << OPD_F_OD_LSB, F(FLD_SME_Rv,FLD_imm1_0), "ZA array")		\
    Y(ZA_ACCESS, sme_za_array, "SME_ZA_array_off2x2",			\
      2 << OPD_F_OD_LSB, F(FLD_SME_Rv,FLD_imm2_0), "ZA array")		\
    Y(ZA_ACCESS, sme_za_array, "SME_ZA_array_off2x4",			\
      4 << OPD_F_OD_LSB, F(FLD_SME_Rv,FLD_imm2_0), "ZA array")		\
    Y(ZA_ACCESS, sme_za_array, "SME_ZA_array_off3_0", 0,		\
      F(FLD_SME_Rv,FLD_imm3_0), "ZA array")				\
    Y(ZA_ACCESS, sme_za_array, "SME_ZA_array_off3_5", 0,		\
      F(FLD_SME_Rv,FLD_imm3_5), "ZA array")				\
    Y(ZA_ACCESS, sme_za_array, "SME_ZA_array_off3x2",			\
      2 << OPD_F_OD_LSB, F(FLD_SME_Rv,FLD_imm3_0), "ZA array")		\
    Y(ZA_ACCESS, sme_za_array, "SME_ZA_array_off4", 0,			\
      F(FLD_SME_Rv,FLD_imm4_0), "ZA array")				\
    Y(ADDRESS, sme_addr_ri_u4xvl, "SME_ADDR_RI_U4xVL", 0 << OPD_F_OD_LSB, \
      F(FLD_Rn,FLD_imm4_0), "memory offset")				\
    Y(ADDRESS, sme_sm_za, "SME_SM_ZA", 0,				\
      F(FLD_CRm), "streaming mode")					\
    Y(SVE_REG, sme_pred_reg_with_index, "SME_PnT_Wm_imm", 0,		\
      F(FLD_SME_Rm,FLD_SVE_Pn,FLD_SME_i1,FLD_SME_tszh,FLD_SME_tszl),	\
      "Source scalable predicate register with index ")	\
    Y(IMMEDIATE, plain_shrimm, "SME_SHRIMM4", 0, F(FLD_SVE_imm4), 	\
      "a shift-right immediate operand")				\
    Y(IMMEDIATE, sve_shrimm, "SME_SHRIMM5", 1 << OPD_F_OD_LSB,		\
      F(FLD_SVE_tszh,FLD_SVE_imm5b), "a shift-right immediate operand")	\
    Y(SVE_REG, simple_index, "SME_Zm_INDEX1", 0,			\
      F(FLD_SME_Zm, FLD_imm1_10), "an indexed SVE vector register")	\
    Y(SVE_REG, simple_index, "SME_Zm_INDEX2", 0,			\
      F(FLD_SME_Zm, FLD_imm2_10), "an indexed SVE vector register")	\
    Y(SVE_REG, simple_index, "SME_Zm_INDEX3_1", 0,			\
      F(FLD_SME_Zm, FLD_imm1_10, FLD_imm2_1),				\
      "an indexed SVE vector register")					\
    Y(SVE_REG, simple_index, "SME_Zm_INDEX3_2", 0,			\
      F(FLD_SME_Zm, FLD_imm2_10, FLD_imm1_2),				\
      "an indexed SVE vector register")					\
    Y(SVE_REG, simple_index, "SME_Zm_INDEX3_10", 0,			\
      F(FLD_SME_Zm, FLD_imm1_15, FLD_imm2_10),				\
      "an indexed SVE vector register")					\
    Y(SVE_REG, simple_index, "SME_Zm_INDEX4_1", 0,			\
      F(FLD_SME_Zm, FLD_imm2_10, FLD_imm2_1),				\
      "an indexed SVE vector register")					\
    Y(SVE_REG, simple_index, "SME_Zm_INDEX4_10", 0,			\
      F(FLD_SME_Zm, FLD_imm1_15, FLD_imm3_10),				\
      "an indexed SVE vector register")					\
    Y(SVE_REG, simple_index, "SME_Zn_INDEX1_16", 0,			\
      F(FLD_SVE_Zn, FLD_imm1_16), "an indexed SVE vector register")	\
    Y(SVE_REG, simple_index, "SME_Zn_INDEX2_15", 0,			\
      F(FLD_SVE_Zn, FLD_imm2_15), "an indexed SVE vector register")	\
    Y(SVE_REG, simple_index, "SME_Zn_INDEX2_16", 0,			\
      F(FLD_SVE_Zn, FLD_imm2_16), "an indexed SVE vector register")	\
    Y(SVE_REG, simple_index, "SME_Zn_INDEX3_14", 0,			\
      F(FLD_SVE_Zn, FLD_imm3_14), "an indexed SVE vector register")	\
    Y(SVE_REG, simple_index, "SME_Zn_INDEX3_15", 0,			\
      F(FLD_SVE_Zn, FLD_imm3_15), "an indexed SVE vector register")	\
    Y(SVE_REG, simple_index, "SME_Zn_INDEX4_14", 0,			\
      F(FLD_SVE_Zn, FLD_imm4_14), "an indexed SVE vector register")	\
    Y(IMMEDIATE, imm, "SME_VLxN_10", 0, F(FLD_SME_VL_10),		\
      "VLx2 or VLx4")							\
    Y(IMMEDIATE, imm, "SME_VLxN_13", 0, F(FLD_SME_VL_13),		\
      "VLx2 or VLx4")							\
    Y(SYSTEM, none, "SME_ZT0", 0, F (), "ZT0")				\
    Y(IMMEDIATE, imm, "SME_ZT0_INDEX", OPD_F_SHIFT_BY_3,		\
      F (FLD_imm3_12), "a ZT0 index")					\
    Y(SYSTEM, none, "SME_ZT0_LIST", 0, F (), "{ ZT0 }")			\
    Y(IMMEDIATE, imm, "TME_UIMM16", 0, F(FLD_imm16_5),			\
      "a 16-bit unsigned immediate for TME tcancel")			\
    Y(SIMD_ELEMENT, reglane, "SM3_IMM2", 0, F(FLD_SM3_imm2),		\
      "an indexed SM3 vector immediate")				\
    /* These next two are really register fields; the [...] notation	\
       is just syntactic sugar.  */					\
    Y(INT_REG, x0_to_x30, "MOPS_ADDR_Rd", 0, F(FLD_Rd),			\
      "a register destination address with writeback")			\
    Y(INT_REG, x0_to_x30, "MOPS_ADDR_Rs", 0, F(FLD_Rs),			\
      "a register source address with writeback")			\
    Y(INT_REG, x0_to_x30, "MOPS_WB_Rd", 0, F(FLD_Rn),			\
      "an integer register with writeback")				\
    Y(IMMEDIATE, imm, "CSSC_SIMM8", OPD_F_SEXT, F(FLD_CSSC_imm8),	\
      "an 8-bit signed immediate")					\
    Y(IMMEDIATE, imm, "CSSC_UIMM8", 0, F(FLD_CSSC_imm8),		\
      "an 8-bit unsigned immediate")
