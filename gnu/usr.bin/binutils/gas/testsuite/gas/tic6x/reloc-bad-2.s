# Test expressions not representable by relocations.
.globl a
.globl b
.data
d:
	.word $DSBT_INDEX(__c6xabi_DSBT_BASE)
	.word $got(b)
	.word $dpr_got(a)
	.word $dpr_byte(b)
	.word $dpr_hword(a)
	.word $dpr_word(b)
	.word $pcr_offset(b,f)
.text
.nocmp
.globl f
f:
	addab .D1X b14,$dsbt_index(__c6xabi_DSBT_BASE),a5
	addab .D1X b14,$GOT(b),a5
	addab .D1X b14,$DPR_GOT(b),a5
	addab .D1X b14,$DPR_BYTE(b),a5
	addab .D1X b14,$DPR_HWORD(b),a5
	addab .D1X b14,$DPR_WORD(b),a5
	addab .D1X b14,$PCR_OFFSET(b,f),a5
	addah .D1X b14,$dsbt_index(__c6xabi_DSBT_BASE),a5
	addah .D1X b14,$GOT(b),a5
	addah .D1X b14,$DPR_GOT(b),a5
	addah .D1X b14,$DPR_BYTE(b),a5
	addah .D1X b14,$DPR_HWORD(b),a5
	addah .D1X b14,$DPR_WORD(b),a5
	addah .D1X b14,$PCR_OFFSET(b,f),a5
	addaw .D1X b14,$DPR_GOT(b),a5
	addaw .D1X b14,$DPR_BYTE(b),a5
	addaw .D1X b14,$DPR_HWORD(b),a5
	addaw .D1X b14,$DPR_WORD(b),a5
	addaw .D1X b14,$PCR_OFFSET(b,f),a5
	addk .S1 $dsbt_index(__c6xabi_DSBT_BASE),a7
	addk .S1 $got(b),a7
	addk .S1 $dpr_got(b),a7
	addk .S1 $dpr_hword(b),a7
	addk .S1 $dpr_word(b),a7
	addk .S1 $pcr_offset(b,f),a7
	mvk .S1 $dsbt_index(__c6xabi_DSBT_BASE),a7
	mvk .S1 $got(b),a7
	mvk .S1 $dpr_got(b),a7
	mvk .S1 $dpr_hword(b),a7
	mvk .S1 $dpr_word(b),a7
	mvkh .S1 $dsbt_index(__c6xabi_DSBT_BASE),a7
	mvkh .S1 $got(b),a7
	mvklh .S1 $dsbt_index(__c6xabi_DSBT_BASE),a7
	mvklh .S1 $got(b),a7
	mvkl .S1 $dsbt_index(__c6xabi_DSBT_BASE),a7
	mvkl .S1 $got(b),a7
	addkpc .S2 $dsbt_index(__c6xabi_DSBT_BASE),b3,0
	addkpc .S2 $GOT(b),b3,0
	addkpc .S2 $DPR_GOT(b),b3,0
	addkpc .S2 $DPR_BYTE(b),b3,0
	addkpc .S2 $DPR_HWORD(b),b3,0
	addkpc .S2 $DPR_WORD(b),b3,0
	addkpc .S2 $PCR_OFFSET(b,f),b3,0
	b .S1 $dsbt_index(__c6xabi_DSBT_BASE)
	b .S1 $GOT(b)
	b .S1 $DPR_GOT(b)
	b .S1 $DPR_BYTE(b)
	b .S1 $DPR_HWORD(b)
	b .S1 $DPR_WORD(b)
	b .S1 $PCR_OFFSET(b,f)
	call .S1 $dsbt_index(__c6xabi_DSBT_BASE)
	call .S1 $GOT(b)
	call .S1 $DPR_GOT(b)
	call .S1 $DPR_BYTE(b)
	call .S1 $DPR_HWORD(b)
	call .S1 $DPR_WORD(b)
	call .S1 $PCR_OFFSET(b,f)
	bdec .S1 $dsbt_index(__c6xabi_DSBT_BASE),a1
	bdec .S1 $GOT(b),a1
	bdec .S1 $DPR_GOT(b),a1
	bdec .S1 $DPR_BYTE(b),a1
	bdec .S1 $DPR_HWORD(b),a1
	bdec .S1 $DPR_WORD(b),a1
	bdec .S1 $PCR_OFFSET(b,f),a1
	bpos .S2 $dsbt_index(__c6xabi_DSBT_BASE),b1
	bpos .S2 $GOT(b),b1
	bpos .S2 $DPR_GOT(b),b1
	bpos .S2 $DPR_BYTE(b),b1
	bpos .S2 $DPR_HWORD(b),b1
	bpos .S2 $DPR_WORD(b),b1
	bpos .S2 $PCR_OFFSET(b,f),b1
	bnop .S1 $dsbt_index(__c6xabi_DSBT_BASE),1
	bnop .S1 $GOT(b),1
	bnop .S1 $DPR_GOT(b),1
	bnop .S1 $DPR_BYTE(b),1
	bnop .S1 $DPR_HWORD(b),1
	bnop .S1 $DPR_WORD(b),1
	bnop .S1 $PCR_OFFSET(b,f),1
	callnop $dsbt_index(__c6xabi_DSBT_BASE),1
	callnop $GOT(b),1
	callnop $DPR_GOT(b),1
	callnop $DPR_BYTE(b),1
	callnop $DPR_HWORD(b),1
	callnop $DPR_WORD(b),1
	callnop $PCR_OFFSET(b,f),1
	callp .S1 $dsbt_index(__c6xabi_DSBT_BASE),a3
	callp .S1 $GOT(b),a3
	callp .S1 $DPR_GOT(b),a3
	callp .S1 $DPR_BYTE(b),a3
	callp .S1 $DPR_HWORD(b),a3
	callp .S1 $DPR_WORD(b),a3
	callp .S1 $PCR_OFFSET(b,f),a3
	callret .S1 $dsbt_index(__c6xabi_DSBT_BASE)
	callret .S1 $GOT(b)
	callret .S1 $DPR_GOT(b)
	callret .S1 $DPR_BYTE(b)
	callret .S1 $DPR_HWORD(b)
	callret .S1 $DPR_WORD(b)
	callret .S1 $PCR_OFFSET(b,f)
	ret .S1 $dsbt_index(__c6xabi_DSBT_BASE)
	ret .S1 $GOT(b)
	ret .S1 $DPR_GOT(b)
	ret .S1 $DPR_BYTE(b)
	ret .S1 $DPR_HWORD(b)
	ret .S1 $DPR_WORD(b)
	ret .S1 $PCR_OFFSET(b,f)
	retp .S1 $dsbt_index(__c6xabi_DSBT_BASE),a3
	retp .S1 $GOT(b),a3
	retp .S1 $DPR_GOT(b),a3
	retp .S1 $DPR_BYTE(b),a3
	retp .S1 $DPR_HWORD(b),a3
	retp .S1 $DPR_WORD(b),a3
	retp .S1 $PCR_OFFSET(b,f),a3
	ldb .D2T2 *+b14($dsbt_index(__c6xabi_DSBT_BASE)),b1
	ldb .D2T2 *+b14($GOT(b)),b1
	ldb .D2T2 *+b14($DPR_GOT(b)),b1
	ldb .D2T2 *+b14($DPR_BYTE(b)),b1
	ldb .D2T2 *+b14($DPR_HWORD(b)),b1
	ldb .D2T2 *+b14($DPR_WORD(b)),b1
	ldb .D2T2 *+b14($PCR_OFFSET(b,f)),b1
	ldbu .D2T2 *+b14($dsbt_index(__c6xabi_DSBT_BASE)),b1
	ldbu .D2T2 *+b14($GOT(b)),b1
	ldbu .D2T2 *+b14($DPR_GOT(b)),b1
	ldbu .D2T2 *+b14($DPR_BYTE(b)),b1
	ldbu .D2T2 *+b14($DPR_HWORD(b)),b1
	ldbu .D2T2 *+b14($DPR_WORD(b)),b1
	ldbu .D2T2 *+b14($PCR_OFFSET(b,f)),b1
	ldh .D2T2 *+b14($dsbt_index(__c6xabi_DSBT_BASE)),b1
	ldh .D2T2 *+b14($GOT(b)),b1
	ldh .D2T2 *+b14($DPR_GOT(b)),b1
	ldh .D2T2 *+b14($DPR_BYTE(b)),b1
	ldh .D2T2 *+b14($DPR_HWORD(b)),b1
	ldh .D2T2 *+b14($DPR_WORD(b)),b1
	ldh .D2T2 *+b14($PCR_OFFSET(b,f)),b1
	ldhu .D2T2 *+b14($dsbt_index(__c6xabi_DSBT_BASE)),b1
	ldhu .D2T2 *+b14($GOT(b)),b1
	ldhu .D2T2 *+b14($DPR_GOT(b)),b1
	ldhu .D2T2 *+b14($DPR_BYTE(b)),b1
	ldhu .D2T2 *+b14($DPR_HWORD(b)),b1
	ldhu .D2T2 *+b14($DPR_WORD(b)),b1
	ldhu .D2T2 *+b14($PCR_OFFSET(b,f)),b1
	ldw .D2T2 *+b14($DPR_GOT(b)),b1
	ldw .D2T2 *+b14($DPR_BYTE(b)),b1
	ldw .D2T2 *+b14($DPR_HWORD(b)),b1
	ldw .D2T2 *+b14($DPR_WORD(b)),b1
	ldw .D2T2 *+b14($PCR_OFFSET(b,f)),b1
	stb .D2T2 b1,*+b14($dsbt_index(__c6xabi_DSBT_BASE))
	stb .D2T2 b1,*+b14($GOT(b))
	stb .D2T2 b1,*+b14($DPR_GOT(b))
	stb .D2T2 b1,*+b14($DPR_BYTE(b))
	stb .D2T2 b1,*+b14($DPR_HWORD(b))
	stb .D2T2 b1,*+b14($DPR_WORD(b))
	stb .D2T2 b1,*+b14($PCR_OFFSET(b,f))
	sth .D2T2 b1,*+b14($dsbt_index(__c6xabi_DSBT_BASE))
	sth .D2T2 b1,*+b14($GOT(b))
	sth .D2T2 b1,*+b14($DPR_GOT(b))
	sth .D2T2 b1,*+b14($DPR_BYTE(b))
	sth .D2T2 b1,*+b14($DPR_HWORD(b))
	sth .D2T2 b1,*+b14($DPR_WORD(b))
	sth .D2T2 b1,*+b14($PCR_OFFSET(b,f))
	stw .D2T2 b1,*+b14($DPR_GOT(b))
	stw .D2T2 b1,*+b14($DPR_BYTE(b))
	stw .D2T2 b1,*+b14($DPR_HWORD(b))
	stw .D2T2 b1,*+b14($DPR_WORD(b))
	stw .D2T2 b1,*+b14($PCR_OFFSET(b,f))
