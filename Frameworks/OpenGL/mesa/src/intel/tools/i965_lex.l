%option yylineno
%option nounput
%{
#include <string.h>
#include "i965_asm.h"
#undef ALIGN16
#include "i965_gram.tab.h"

/* Locations */
int yycolumn = 1;

int saved_state = 0;
extern char *input_filename;

#define YY_NO_INPUT
#define YY_USER_ACTION                                  	\
	yylloc.first_line = yylloc.last_line = yylineno;	\
	yylloc.first_column = yycolumn;			        \
	yylloc.last_column = yycolumn + yyleng - 1;	        \
	yycolumn += yyleng;
%}

%x BLOCK_COMMENT
%x FILENAME
%x CHANNEL
%x REG
%x DOTSEL
%x LABEL
%x MSGDESC
%%

 /* eat up single line comment */
\/\/.*[\r\n]	{ yycolumn = 1; }

 /* eat up multiline comment */
\/\*		{ saved_state = YYSTATE; BEGIN(BLOCK_COMMENT); }

<BLOCK_COMMENT>\*\/	{ BEGIN(saved_state); }

<BLOCK_COMMENT>.     	{ }
<BLOCK_COMMENT>[\r\n]	{ }

<FILENAME>\"[^\"]+\"	{
			   char *name = malloc(yyleng - 1);
			   memmove(name, yytext + 1, yyleng - 2);
			   name[yyleng-1] = '\0';
			   input_filename = name;
			}

 /* null register */
null 		{ BEGIN(REG); return NULL_TOKEN; }

 /* Opcodes */
add		{ yylval.integer = BRW_OPCODE_ADD; return ADD; }
add3		{ yylval.integer = BRW_OPCODE_ADD3; return ADD3; }
addc		{ yylval.integer = BRW_OPCODE_ADDC; return ADDC; }
and		{ yylval.integer = BRW_OPCODE_AND; return AND; }
asr		{ yylval.integer = BRW_OPCODE_ASR; return ASR; }
avg		{ yylval.integer = BRW_OPCODE_AVG; return AVG; }
bfe 		{ yylval.integer = BRW_OPCODE_BFE; return BFE; }
bfi1 		{ yylval.integer = BRW_OPCODE_BFI1; return BFI1; }
bfi2 		{ yylval.integer = BRW_OPCODE_BFI2; return BFI2; }
bfrev 		{ yylval.integer = BRW_OPCODE_BFREV; return BFREV; }
brc 		{ yylval.integer = BRW_OPCODE_BRC; return BRC; }
brd 		{ yylval.integer = BRW_OPCODE_BRD; return BRD; }
break 		{ yylval.integer = BRW_OPCODE_BREAK; return BREAK; }
call 		{ yylval.integer = BRW_OPCODE_CALL; return CALL; }
calla 		{ yylval.integer = BRW_OPCODE_CALLA; return CALLA; }
case 		{ yylval.integer = BRW_OPCODE_CASE; return CASE; }
cbit 		{ yylval.integer = BRW_OPCODE_CBIT; return CBIT; }
cmp 		{ yylval.integer = BRW_OPCODE_CMP; return CMP; }
cmpn 		{ yylval.integer = BRW_OPCODE_CMPN; return CMPN; }
cont 		{ yylval.integer = BRW_OPCODE_CONTINUE; return CONT; }
csel 		{ yylval.integer = BRW_OPCODE_CSEL; return CSEL; }
dim 		{ yylval.integer = BRW_OPCODE_DIM; return DIM; }
do 		{ yylval.integer = BRW_OPCODE_DO; return DO; }
dp2 		{ yylval.integer = BRW_OPCODE_DP2; return DP2; }
dp3 		{ yylval.integer = BRW_OPCODE_DP3; return DP3; }
dp4 		{ yylval.integer = BRW_OPCODE_DP4; return DP4; }
dp4a		{ yylval.integer = BRW_OPCODE_DP4A; return DP4A; }
dph 		{ yylval.integer = BRW_OPCODE_DPH; return DPH; }
else 		{ yylval.integer = BRW_OPCODE_ELSE; return ELSE; }
endif 		{ yylval.integer = BRW_OPCODE_ENDIF; return ENDIF; }
f16to32 	{ yylval.integer = BRW_OPCODE_F16TO32; return F16TO32; }
f32to16 	{ yylval.integer = BRW_OPCODE_F32TO16; return F32TO16; }
fbh 		{ yylval.integer = BRW_OPCODE_FBH; return FBH; }
fbl 		{ yylval.integer = BRW_OPCODE_FBL; return FBL; }
fork 		{ yylval.integer = BRW_OPCODE_FORK; return FORK; }
frc 		{ yylval.integer = BRW_OPCODE_FRC; return FRC; }
goto 		{ yylval.integer = BRW_OPCODE_GOTO; return GOTO; }
halt 		{ yylval.integer = BRW_OPCODE_HALT; return HALT; }
if 		{ yylval.integer = BRW_OPCODE_IF; return IF; }
iff 		{ yylval.integer = BRW_OPCODE_IFF; return IFF; }
illegal 	{ yylval.integer = BRW_OPCODE_ILLEGAL; return ILLEGAL; }
jmpi 		{ yylval.integer = BRW_OPCODE_JMPI; return JMPI; }
line 		{ yylval.integer = BRW_OPCODE_LINE; return LINE; }
lrp 		{ yylval.integer = BRW_OPCODE_LRP; return LRP; }
lzd 		{ yylval.integer = BRW_OPCODE_LZD; return LZD; }
mac 		{ yylval.integer = BRW_OPCODE_MAC; return MAC; }
mach 		{ yylval.integer = BRW_OPCODE_MACH; return MACH; }
mad 		{ yylval.integer = BRW_OPCODE_MAD; return MAD; }
madm 		{ yylval.integer = BRW_OPCODE_MADM; return MADM; }
mov 		{ yylval.integer = BRW_OPCODE_MOV; return MOV; }
movi 		{ yylval.integer = BRW_OPCODE_MOVI; return MOVI; }
mul 		{ yylval.integer = BRW_OPCODE_MUL; return MUL; }
mrest 		{ yylval.integer = BRW_OPCODE_MREST; return MREST; }
msave 		{ yylval.integer = BRW_OPCODE_MSAVE; return MSAVE; }
nenop 		{ yylval.integer = BRW_OPCODE_NENOP; return NENOP; }
nop 		{ yylval.integer = BRW_OPCODE_NOP; return NOP; }
not 		{ yylval.integer = BRW_OPCODE_NOT; return NOT; }
or 		{ yylval.integer = BRW_OPCODE_OR; return OR; }
pln 		{ yylval.integer = BRW_OPCODE_PLN; return PLN; }
pop 		{ yylval.integer = BRW_OPCODE_POP; return POP; }
push 		{ yylval.integer = BRW_OPCODE_PUSH; return PUSH; }
ret 		{ yylval.integer = BRW_OPCODE_RET; return RET; }
rndd 		{ yylval.integer = BRW_OPCODE_RNDD; return RNDD; }
rnde 		{ yylval.integer = BRW_OPCODE_RNDE; return RNDE; }
rndu 		{ yylval.integer = BRW_OPCODE_RNDU; return RNDU; }
rndz 		{ yylval.integer = BRW_OPCODE_RNDZ; return RNDZ; }
rol 		{ yylval.integer = BRW_OPCODE_ROL; return ROL; }
ror 		{ yylval.integer = BRW_OPCODE_ROR; return ROR; }
sad2 		{ yylval.integer = BRW_OPCODE_SAD2; return SAD2; }
sada2 		{ yylval.integer = BRW_OPCODE_SADA2; return SADA2; }
sel 		{ yylval.integer = BRW_OPCODE_SEL; return SEL; }
send 		{
			yylval.integer = BRW_OPCODE_SEND;
			return p->devinfo->ver < 12 ? SEND_GFX4 : SEND_GFX12;
		}
sendc 		{
			yylval.integer = BRW_OPCODE_SENDC;
			return  p->devinfo->ver < 12 ? SENDC_GFX4 : SENDC_GFX12;
		}
sends 	        { yylval.integer = BRW_OPCODE_SENDS; return SENDS; }
sendsc        	{ yylval.integer = BRW_OPCODE_SENDSC; return SENDSC; }
shl 		{ yylval.integer = BRW_OPCODE_SHL; return SHL; }
shr 		{ yylval.integer = BRW_OPCODE_SHR; return SHR; }
smov 		{ yylval.integer = BRW_OPCODE_SMOV; return SMOV; }
subb 		{ yylval.integer = BRW_OPCODE_SUBB; return SUBB; }
wait 		{ yylval.integer = BRW_OPCODE_WAIT; return WAIT; }
while 		{ yylval.integer = BRW_OPCODE_WHILE; return WHILE; }
xor 		{ yylval.integer = BRW_OPCODE_XOR; return XOR; }
sync		{ yylval.integer = BRW_OPCODE_SYNC; return SYNC; }

 /* extended math functions */
cos 		{ yylval.integer = BRW_MATH_FUNCTION_COS; return COS; }
exp 		{ yylval.integer = BRW_MATH_FUNCTION_EXP; return EXP; }
fdiv 		{ yylval.integer = BRW_MATH_FUNCTION_FDIV; return FDIV; }
inv 		{ yylval.integer = BRW_MATH_FUNCTION_INV; return INV; }
invm 		{ yylval.integer = GFX8_MATH_FUNCTION_INVM; return INVM; }
intdiv        	{
		   yylval.integer = BRW_MATH_FUNCTION_INT_DIV_QUOTIENT;
		   return INTDIV;
		}
intdivmod    	{
		   yylval.integer =
		      BRW_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER;
		   return INTDIVMOD;
		}
intmod      	{
		   yylval.integer = BRW_MATH_FUNCTION_INT_DIV_REMAINDER;
		   return INTMOD;
		}
log 		{ yylval.integer = BRW_MATH_FUNCTION_LOG; return LOG; }
pow 		{ yylval.integer = BRW_MATH_FUNCTION_POW; return POW; }
rsq 		{ yylval.integer = BRW_MATH_FUNCTION_RSQ; return RSQ; }
rsqrtm       	{ yylval.integer = GFX8_MATH_FUNCTION_RSQRTM; return RSQRTM; }
sin 		{ yylval.integer = BRW_MATH_FUNCTION_SIN; return SIN; }
sqrt 		{ yylval.integer = BRW_MATH_FUNCTION_SQRT; return SQRT; }
sincos       	{ yylval.integer = BRW_MATH_FUNCTION_SINCOS; return SINCOS; }

 /* sync instruction */
allrd		{ yylval.integer = TGL_SYNC_ALLRD; return ALLRD; }
allwr		{ yylval.integer = TGL_SYNC_ALLWR; return ALLWR; }
fence		{ yylval.integer = TGL_SYNC_FENCE; return FENCE; }
bar		{ yylval.integer = TGL_SYNC_BAR; return BAR; }
host		{ yylval.integer = TGL_SYNC_HOST; return HOST; }

 /* shared functions for send instruction */
sampler 		{ return SAMPLER; }
dp_sampler 		{ return DP_SAMPLER; }
gateway 		{ return GATEWAY; }
urb 			{ return URB; }
thread_spawner		{ return THREAD_SPAWNER; }
render            	{ return RENDER; }
const 			{ return CONST; }
data 			{ return DATA; }
cre 			{ return CRE; }
math 			{ return MATH; }
read 			{ return READ; }
write 			{ return WRITE; }
vme 			{ return VME; }
"pixel interp"		{ return PIXEL_INTERP; }
"dp data 1" 		{ return DP_DATA_1; }
"rt accel"		{ return RT_ACCEL; }
slm			{ return SLM; }
tgm			{ return TGM; }
ugm			{ return UGM; }

";"    	{ return SEMICOLON; }
":"    	{ return COLON; }
"("    	{ return LPAREN; }
")"    	{ return RPAREN; }
"{"    	{ return LCURLY; }
"}"    	{ return RCURLY; }
"["    	{ return LSQUARE; }
"]"    	{ return RSQUARE; }
"<"    	{ return LANGLE; }
">"    	{ return RANGLE; }
","    	{ return COMMA; }
"."    	{ return DOT; }
"+"    	{ return PLUS; }
"-"    	{ return MINUS; }
"~"    	{ return MINUS; }
"(abs)"	{ return ABS; }


"VxH"             	{ return VxH; }
<REG>"<" 		{ return LANGLE; }
<REG>[0-9][0-9]* 	{
			   yylval.integer = strtoul(yytext, NULL, 10);
			   return INTEGER;
			}
<REG>">" 		{ return RANGLE; }
<REG>","		{ return COMMA; }
<REG>"."		{ BEGIN(DOTSEL); return DOT; }
<REG>";"		{ return SEMICOLON; }

<DOTSEL>"x"	        { yylval.integer = BRW_CHANNEL_X; return X; }
<DOTSEL>"y" 	        { yylval.integer = BRW_CHANNEL_Y; return Y; }
<DOTSEL>"z" 	        { yylval.integer = BRW_CHANNEL_Z; return Z; }
<DOTSEL>"w" 	        { yylval.integer = BRW_CHANNEL_W; return W; }
<DOTSEL>[0-9][0-9]* 	{
			   yylval.integer = strtoul(yytext, NULL, 10);
			   BEGIN(REG);
			   return INTEGER;
		        }
<DOTSEL>. 	        { yyless(0); BEGIN(INITIAL); }
<REG>.             	{ yyless(0); BEGIN(INITIAL); }

 /* Access mode */
"align1"	{ return ALIGN1; }
"align16"	{ return ALIGN16; }

 /* Accumulator write control */
AccWrEnable 	{ return ACCWREN; }

 /* Mask control (formerly WECtrl/Write Enable Control) */
"WE_all"	{ return WECTRL; }

 /* Compaction control */
compacted 	{ return CMPTCTRL; }

 /* Debug control */
breakpoint 	{ return BREAKPOINT; }

 /* Dependency control */
NoDDClr 	{ return NODDCLR; }
NoDDChk 	{ return NODDCHK; }

 /* End of thread */
EOT 		{ return EOT; }

 /* Mask control */
nomask      	{ return MASK_DISABLE; }

 /* Channel */
<CHANNEL>"x" 		{ yylval.integer = BRW_CHANNEL_X; return X; }
<CHANNEL>"y" 		{ yylval.integer = BRW_CHANNEL_Y; return Y; }
<CHANNEL>"z" 		{ yylval.integer = BRW_CHANNEL_Z; return Z; }
<CHANNEL>"w" 		{ yylval.integer = BRW_CHANNEL_W; return W; }
<CHANNEL>[0-9][0-9]* 	{
			   yylval.integer = strtoul(yytext, NULL, 10);
			   return INTEGER;
		        }
<CHANNEL>"."    	{ return DOT; }
<CHANNEL>. 		{ yyless(0); BEGIN(INITIAL); }


 /* Predicate Control */
<CHANNEL>".anyv"	{ yylval.integer = BRW_PREDICATE_ALIGN1_ANYV; return ANYV; }
<CHANNEL>".allv"      	{ yylval.integer = BRW_PREDICATE_ALIGN1_ALLV; return ALLV; }
<CHANNEL>".any2h"	{ yylval.integer = BRW_PREDICATE_ALIGN1_ANY2H; return ANY2H; }
<CHANNEL>".all2h"	{ yylval.integer = BRW_PREDICATE_ALIGN1_ALL2H; return ALL2H; }
<CHANNEL>".any4h"	{ yylval.integer = BRW_PREDICATE_ALIGN16_ANY4H; return ANY4H; }
<CHANNEL>".all4h"	{ yylval.integer = BRW_PREDICATE_ALIGN16_ALL4H; return ALL4H; }
<CHANNEL>".any8h"	{ yylval.integer = BRW_PREDICATE_ALIGN1_ANY8H; return ANY8H; }
<CHANNEL>".all8h"	{ yylval.integer = BRW_PREDICATE_ALIGN1_ALL8H; return ALL8H; }
<CHANNEL>".any16h"	{ yylval.integer = BRW_PREDICATE_ALIGN1_ANY16H; return ANY16H; }
<CHANNEL>".all16h"	{ yylval.integer = BRW_PREDICATE_ALIGN1_ALL16H; return ALL16H; }
<CHANNEL>".any32h"	{ yylval.integer = BRW_PREDICATE_ALIGN1_ANY32H; return ANY32H; }
<CHANNEL>".all32h"	{ yylval.integer = BRW_PREDICATE_ALIGN1_ALL32H; return ALL32H; }

 /* Saturation */
".sat"		{ return SATURATE; }

 /* Thread control */
atomic       	{ return ATOMIC; }
switch       	{ return SWITCH; }

 /* compression control */
compr 		{ return COMPR; }
compr4    	{ return COMPR4; }
sechalf 	{ return SECHALF; }

 /* Quarter Control */
1[HNQ]       	{ }
"2Q"	        { return QTR_2Q; }
"3Q"	        { return QTR_3Q; }
"4Q"	        { return QTR_4Q; }
"2H"	        { return QTR_2H; }
"2N"	        { return QTR_2N; }
"3N"	        { return QTR_3N; }
"4N"	        { return QTR_4N; }
"5N"	        { return QTR_5N; }
"6N"	        { return QTR_6N; }
"7N"	        { return QTR_7N; }
"8N"	        { return QTR_8N; }

 /* data types */
:?B 	{ return TYPE_B; }
:?D 	{ return TYPE_D; }
:?DF 	{ return TYPE_DF; }
:?F 	{ return TYPE_F; }
:?HF 	{ return TYPE_HF; }
:?NF 	{ return TYPE_NF; }
:?Q 	{ return TYPE_Q; }
:?UB 	{ return TYPE_UB; }
:?UD 	{ return TYPE_UD; }
:?UW 	{ return TYPE_UW; }
:?UQ 	{ return TYPE_UQ; }
:?UV 	{ return TYPE_UV; }
:?V 	{ return TYPE_V; }
:?VF 	{ return TYPE_VF; }
:?W 	{ return TYPE_W; }

 /* Address registers */
"a0" 		{ return ADDRREG; }

 /* accumulator registers */
"acc"[0-9]+ 	{ yylval.integer = atoi(yytext + 3); return ACCREG; }

 /* channel enable registers */
"ce0"		{ return CHANNELENABLEREG; }

 /* control registers */
"cr0" 		{ return CONTROLREG; }

 /* flag registers */
"f"[0|1] 	{ BEGIN(CHANNEL); yylval.integer = atoi(yytext + 1); return FLAGREG; }

 /* message control registers */
"m" 		{ return MSGREGFILE; }
m[0-9]+ 	{ yylval.integer = atoi(yytext + 1); BEGIN(REG); return MSGREG; }

 /* state register */
sr[0-9]+ 	{ yylval.integer = atoi(yytext + 2); return STATEREG; }

 /* notification registers */
"n0"  		{ BEGIN(REG); return NOTIFYREG; }

 /* IP register */
"ip" 		{ return IPREG; }

 /* Thread control register */
"tdr0"		{ return THREADREG; }

 /* performance register */
"tm0" 		{ BEGIN(REG); return PERFORMANCEREG; }

[gr][0-9]+ 	{
		   yylval.integer = atoi(yytext + 1);
		   BEGIN(REG); return GENREG;
		}
[gr] 		{ return GENREGFILE; }
"mask"[0-9]+ 	{ yylval.integer = atoi(yytext + 4); return MASKREG; }

 /* Conditional modifiers */
".e" 	{ yylval.integer = BRW_CONDITIONAL_Z; return EQUAL; }
".g" 	{ yylval.integer = BRW_CONDITIONAL_G; return GREATER; }
".ge"	{ yylval.integer = BRW_CONDITIONAL_GE; return GREATER_EQUAL; }
".l"	{ yylval.integer = BRW_CONDITIONAL_L; return LESS; }
".le"	{ yylval.integer = BRW_CONDITIONAL_LE; return LESS_EQUAL; }
".ne"	{ yylval.integer = BRW_CONDITIONAL_NZ; return NOT_EQUAL; }
".nz"	{ yylval.integer = BRW_CONDITIONAL_NZ; return NOT_ZERO; }
".o"	{ yylval.integer = BRW_CONDITIONAL_O; return OVERFLOW; }
".r"	{ yylval.integer = BRW_CONDITIONAL_R; return ROUND_INCREMENT; }
".u"	{ yylval.integer = BRW_CONDITIONAL_U; return UNORDERED; }
".z"	{ yylval.integer = BRW_CONDITIONAL_Z; return ZERO; }

 /* Eat up JIP and UIP token, their values will be parsed
  * in numeric section
  */
"JIP: "		{ BEGIN(LABEL); }
"UIP: "		{ BEGIN(LABEL); }
"Jump: "       	{ }
"Pop: "		{ }
[ \t]+ 		{ }

"MsgDesc: "		{ BEGIN(MSGDESC); return MSGDESC_BEGIN; }
<MSGDESC>ex_bso		{ return EX_BSO; }
<MSGDESC>src1_len	{ return SRC1_LEN; }
<MSGDESC>"="		{ return ASSIGN; }
<MSGDESC>[0-9][0-9]*	{
			   yylval.integer = strtoul(yytext, NULL, 10);
			   return INTEGER;
		        }
<MSGDESC>"{"    	{ yyless(0); BEGIN(INITIAL); return MSGDESC_END; }
<MSGDESC>.      	{ }

"0x"[0-9a-f][0-9a-f]* 	{
			   yylval.llint = strtoull(yytext + 2, NULL, 16);
			   return LONG;
			}
[0-9][0-9]* 		{
			   yylval.llint = strtoll(yytext, NULL, 10);
			   return LONG;
			}

 /* jump label target */
[a-zA-Z_][0-9a-zA-Z_]*":" {
	yylval.string = ralloc_strdup(p->mem_ctx, yytext);
	/* Stomp the trailing ':' */
	yylval.string[yyleng - 1] = '\0';
	return JUMP_LABEL_TARGET;
}

 /* jump label */
<LABEL>[a-zA-Z_][0-9a-zA-Z_]* {
	yylval.string = ralloc_strdup(p->mem_ctx, yytext);
	BEGIN(INITIAL);
	return JUMP_LABEL;
}

 /* SWSB */
"@"[1-7]	{ yylval.integer = atoi(yytext + 1); return REG_DIST_CURRENT; }
"F@"[1-7]	{ yylval.integer = atoi(yytext + 2); return REG_DIST_FLOAT; }
"I@"[1-7]	{ yylval.integer = atoi(yytext + 2); return REG_DIST_INT; }
"L@"[1-7]	{ yylval.integer = atoi(yytext + 2); return REG_DIST_LONG; }
"A@"[1-7]	{ yylval.integer = atoi(yytext + 2); return REG_DIST_ALL; }

"$"[0-9]*	{ yylval.integer = atoi(yytext + 1); return SBID_ALLOC; }
"$"[0-9]*".src"	{ yylval.integer = atoi(yytext + 1); return SBID_WAIT_SRC; }
"$"[0-9]*".dst"	{ yylval.integer = atoi(yytext + 1); return SBID_WAIT_DST; }

\n 	{ yycolumn = 1; }

. 	{
	   fprintf(stderr, "%s: %d: %s: at \"%s\"\n",
	           input_filename, yylineno,
	           "unexpected token", lex_text());
	}
%%

char *
lex_text(void)
{
	return yytext;
}

#ifndef yywrap
int yywrap()
{
	return -1;
}
#endif
