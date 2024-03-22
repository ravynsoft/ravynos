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
#define YYDEBUG 0

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "asm.h"


int yyget_lineno(void);

#ifdef YYDEBUG
int yydebug;
#endif

extern int yylex(void);
typedef void *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *);
extern void yy_delete_buffer(YY_BUFFER_STATE);

int yyparse(void);

void yyerror(const char *error);
void yyerror(const char *error)
{
	fprintf(stderr, "error at line %d: %s\n", yyget_lineno(), error);
}

static struct afuc_instr *instr;   /* current instruction */

static void
new_instr(afuc_opc opc)
{
	instr = next_instr(opc);
}

static void
dst(int num)
{
	instr->dst = num;
}

static void
src1(int num)
{
	instr->src1 = num;
}

static void
src2(int num)
{
	instr->src2 = num;
}

static void
immed(int num)
{
	instr->immed = num;
	instr->has_immed = true;
}

static void
shift(int num)
{
	instr->shift = num;
	instr->has_shift = true;
}

static void
bit(int num)
{
	instr->bit = num;
	instr->has_bit = true;
}

static void
literal(uint32_t num)
{
	instr->literal = num;
	instr->is_literal = true;
}

static void
label(const char *str)
{
	instr->label = str;
}

%}

%union {
	int tok;
	uint32_t num;
	const char *str;
}

%token <num> T_INT
%token <num> T_HEX
%token <num> T_CONTROL_REG
%token <num> T_SQE_REG
%token <str> T_LABEL_DECL
%token <str> T_LABEL_REF
%token <num> T_LITERAL
%token <num> T_BIT
%token <num> T_REGISTER

%token <tok> T_OP_NOP
%token <tok> T_OP_ADD
%token <tok> T_OP_ADDHI
%token <tok> T_OP_SUB
%token <tok> T_OP_SUBHI
%token <tok> T_OP_AND
%token <tok> T_OP_OR
%token <tok> T_OP_XOR
%token <tok> T_OP_NOT
%token <tok> T_OP_SHL
%token <tok> T_OP_USHR
%token <tok> T_OP_ISHR
%token <tok> T_OP_ROT
%token <tok> T_OP_MUL8
%token <tok> T_OP_MIN
%token <tok> T_OP_MAX
%token <tok> T_OP_CMP
%token <tok> T_OP_BIC
%token <tok> T_OP_MSB
%token <tok> T_OP_SETBIT
%token <tok> T_OP_CLRBIT
%token <tok> T_OP_BFI
%token <tok> T_OP_UBFX
%token <tok> T_OP_MOV
%token <tok> T_OP_CWRITE
%token <tok> T_OP_CREAD
%token <tok> T_OP_SWRITE
%token <tok> T_OP_SREAD
%token <tok> T_OP_STORE
%token <tok> T_OP_LOAD
%token <tok> T_OP_BRNE
%token <tok> T_OP_BREQ
%token <tok> T_OP_RET
%token <tok> T_OP_IRET
%token <tok> T_OP_CALL
%token <tok> T_OP_JUMP
%token <tok> T_OP_WAITIN
%token <tok> T_OP_PREEMPTLEAVE
%token <tok> T_OP_SETSECURE
%token <tok> T_LSHIFT
%token <tok> T_REP
%token <num> T_XMOV
%token <num> T_SDS

%type <num> reg
%type <num> immediate

%error-verbose

%start instrs

%%

instrs:            instr_or_label instrs
|                  instr_or_label

instr_or_label:    instr_r
|                  T_REP instr_r    { instr->rep = true; }
|                  branch_instr
|                  other_instr
|                  T_LABEL_DECL   { decl_label($1); }

/* instructions that can optionally have (rep) flag: */
instr_r:           alu_instr           { instr->xmov = 0; }
|                  T_XMOV alu_instr    { instr->xmov = $1; }
|                  load_instr
|                  store_instr

/* need to special case:
 * - not (single src, possibly an immediate)
 * - msb (single src, must be reg)
 * - mov (single src, plus possibly a shift)
 * from the other ALU instructions:
 */

alu_msb_instr:     T_OP_MSB reg ',' reg        { new_instr(OPC_MSB); dst($2); src1($4); }

alu_not_instr:     T_OP_NOT reg ',' reg        { new_instr(OPC_NOT); dst($2); src1($4); }
|                  T_OP_NOT reg ',' immediate  { new_instr(OPC_NOT); dst($2); immed($4); }

alu_mov_instr:     T_OP_MOV reg ',' reg        { new_instr(OPC_OR); dst($2); src1(0); src2($4); }
|                  T_OP_MOV reg ',' immediate T_LSHIFT immediate {
                       new_instr(OPC_MOVI); dst($2); immed($4); shift($6);
}
|                  T_OP_MOV reg ',' immediate  { new_instr(OPC_MOVI); dst($2); immed($4); shift(0); }
|                  T_OP_MOV reg ',' T_LABEL_REF T_LSHIFT immediate {
                       new_instr(OPC_MOVI); dst($2); label($4); shift($6);
}
|                  T_OP_MOV reg ',' T_LABEL_REF { new_instr(OPC_MOVI); dst($2); label($4); shift(0); }

alu_2src_op:       T_OP_ADD       { new_instr(OPC_ADD); }
|                  T_OP_ADDHI     { new_instr(OPC_ADDHI); }
|                  T_OP_SUB       { new_instr(OPC_SUB); }
|                  T_OP_SUBHI     { new_instr(OPC_SUBHI); }
|                  T_OP_AND       { new_instr(OPC_AND); }
|                  T_OP_OR        { new_instr(OPC_OR); }
|                  T_OP_XOR       { new_instr(OPC_XOR); }
|                  T_OP_SHL       { new_instr(OPC_SHL); }
|                  T_OP_USHR      { new_instr(OPC_USHR); }
|                  T_OP_ISHR      { new_instr(OPC_ISHR); }
|                  T_OP_ROT       { new_instr(OPC_ROT); }
|                  T_OP_MUL8      { new_instr(OPC_MUL8); }
|                  T_OP_MIN       { new_instr(OPC_MIN); }
|                  T_OP_MAX       { new_instr(OPC_MAX); }
|                  T_OP_CMP       { new_instr(OPC_CMP); }
|                  T_OP_BIC       { new_instr(OPC_BIC); }

alu_2src_instr:    alu_2src_op reg ',' reg ',' reg { dst($2); src1($4); src2($6); }
|                  alu_2src_op reg ',' reg ',' immediate { dst($2); src1($4); immed($6); }

alu_setbit_src2:    T_BIT { bit($1); instr->opc = OPC_SETBITI; }
|                   reg   { src2($1); }

alu_clrsetbit_instr: T_OP_SETBIT reg ',' reg ',' alu_setbit_src2 { new_instr(OPC_SETBIT); dst($2); src1($4); }
|                    T_OP_CLRBIT reg ',' reg ',' T_BIT { new_instr(OPC_CLRBIT); dst($2); src1($4); bit($6); }

alu_bitfield_op:  T_OP_UBFX { new_instr(OPC_UBFX); }
|                 T_OP_BFI  { new_instr(OPC_BFI); }

alu_bitfield_instr: alu_bitfield_op reg ',' reg ',' T_BIT ',' T_BIT { dst($2); src1($4); bit($6); immed($8); }

alu_instr:         alu_2src_instr
|                  alu_msb_instr
|                  alu_not_instr
|                  alu_mov_instr
|                  alu_clrsetbit_instr
|                  alu_bitfield_instr

load_op:           T_OP_LOAD           { new_instr(OPC_LOAD); }
|                  T_OP_CREAD          { new_instr(OPC_CREAD); }
|                  T_OP_SREAD          { new_instr(OPC_SREAD); }
store_op:          T_OP_STORE          { new_instr(OPC_STORE); }
|                  T_OP_CWRITE         { new_instr(OPC_CWRITE); instr->sds = 0; }
|                  T_SDS T_OP_CWRITE   { new_instr(OPC_CWRITE); instr->sds = $1; }
|                  T_OP_SWRITE         { new_instr(OPC_SWRITE); }

preincrement:
|              '!'   { instr->preincrement = true; }

load_instr:        load_op reg ',' '[' reg '+' immediate ']' preincrement {
                       dst($2); src1($5); immed($7);
}
store_instr:       store_op reg ',' '[' reg '+' immediate ']' preincrement {
                       src1($2); src2($5); immed($7);
}

branch_op:         T_OP_BRNE      { new_instr(OPC_BRNE); }
|                  T_OP_BREQ      { new_instr(OPC_BREQ); }

branch_instr:      branch_op reg ',' T_BIT ',' T_LABEL_REF     { src1($2); bit($4); label($6); }
|                  branch_op reg ',' immediate ',' T_LABEL_REF { src1($2); immed($4); label($6); }

other_instr:       T_OP_CALL T_LABEL_REF { new_instr(OPC_CALL); label($2); }
|                  T_OP_PREEMPTLEAVE T_LABEL_REF { new_instr(OPC_PREEMPTLEAVE); label($2); }
|                  T_OP_SETSECURE reg ',' T_LABEL_REF { new_instr(OPC_SETSECURE); src1($2); label($4); }
|                  T_OP_RET              { new_instr(OPC_RET); }
|                  T_OP_IRET             { new_instr(OPC_IRET); }
|                  T_OP_JUMP T_LABEL_REF { new_instr(OPC_JUMP); label($2); }
|                  T_OP_WAITIN           { new_instr(OPC_WAITIN); }
|                  T_OP_NOP              { new_instr(OPC_NOP); }
|                  T_LITERAL             { new_instr(OPC_RAW_LITERAL); literal($1); }

reg:               T_REGISTER

immediate:         T_HEX
|                  T_INT
|                  T_CONTROL_REG
|                  T_CONTROL_REG '+' immediate { $$ = $1 + $3; }
|                  T_SQE_REG
|                  T_SQE_REG '+' immediate { $$ = $1 + $3; }

