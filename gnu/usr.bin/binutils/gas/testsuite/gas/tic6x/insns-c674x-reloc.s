# Test C674x instructions generating relocations.
.data
w1:
	.word 1
w2:
	.word 2
.text
.nocmp
.globl ext1
.globl ext2
.globl ext3
.globl a1
.globl b1
.globl f
f:
	addab .D1X b14,ext1,a5
	addab .D2 b15,(ext2+7),b7
	addab .D1X b14,(a1),a20
	addab .D2 b14,(b1),b30
	addab .D1X b14,w2-w1,a15
	addab .D2 b14,w4-w3,b16
	addah .D1X b14,ext1,a5
	addah .D2 b15,(ext2+6),b7
	addah .D1X b14,(a1),a20
	addah .D2 b14,(b1),b30
	addah .D1X b14,w2-w1,a15
	addah .D2 b14,w4-w3,b16
	addaw .D1X b14,ext1,a5
	addaw .D2 b15,(ext2+8),b7
	addaw .D1X b14,(a1),a20
	addaw .D2 b14,(b1),b30
	addaw .D1X b14,w2-w1,a15
	addaw .D2 b14,w4-w3,b16
	addaw .D1X b14,$DSBT_INDEX(__c6xabi_DSBT_BASE),a5
	addaw .D2 b15,$GOT(ext2)+8,b7
	addk .S1 ext1+3,a1
	addk .S2 $dpr_byte(ext2)+5,b3
	addk .S1 w2-w1,a4
	addk .S2 w3-w4,b5
	mvk .S1 ext1+3,a1
	mvk .S2 $dpr_byte(ext2)+5,b3
	mvk .S1 w2-w1,a4
	mvk .S2 w3-w4,b5
	mvkh .S1 ext3+1,a1
	mvkh .S2 $DPR_GOT(ext2)+2,b2
	mvkh .S1 $DPR_BYTE(ext1)+3,a3
	mvkh .S2 $DPR_HWORD(ext3)+4,b4
	mvkh .S1 $DPR_WORD(ext2)+5,a5
	mvkh .S2 s1-s0,b6
	mvklh .S1 ext3+1,a1
	mvklh .S2 $DPR_GOT(ext2)+2,b2
	mvklh .S1 $DPR_BYTE(ext1)+3,a3
	mvklh .S2 $DPR_HWORD(ext3)+4,b4
	mvklh .S1 $DPR_WORD(ext2)+5,a5
	mvklh .S2 s1-s0,b6
	mvkl .S1 ext3+1,a1
	mvkl .S2 $DPR_GOT(ext2)+2,b2
	mvkl .S1 $DPR_BYTE(ext1)+3,a3
	mvkl .S2 $DPR_HWORD(ext3)+4,b4
	mvkl .S1 $DPR_WORD(ext2)+5,a5
	mvkl .S2 s1-s0,b6
	ldb .D2T2 *+b14(ext1),b1
	ldb .D2T1 *+b15(ext2+7),a1
	ldb .D2T2 *+b15(b1),b1
	ldb .D2T1 *+b14(a1),a1
	ldb .D2T2 *+b14(w2-w1),b1
	ldb .D2T1 *+b14(w4-w3),a1
	ldbu .D2T2 *+b14(ext1),b1
	ldbu .D2T1 *+b15(ext2+7),a1
	ldbu .D2T2 *+b15(b1),b1
	ldbu .D2T1 *+b14(a1),a1
	ldbu .D2T2 *+b14(w2-w1),b1
	ldbu .D2T1 *+b14(w4-w3),a1
	ldh .D2T2 *+b14(ext1),b1
	ldh .D2T1 *+b15(ext2+6),a1
	ldh .D2T2 *+b15(b1),b1
	ldh .D2T1 *+b14(a1),a1
	ldh .D2T2 *+b14(w2-w1),b1
	ldh .D2T1 *+b14(w4-w3),a1
	ldhu .D2T2 *+b14(ext1),b1
	ldhu .D2T1 *+b15(ext2+6),a1
	ldhu .D2T2 *+b15(b1),b1
	ldhu .D2T1 *+b14(a1),a1
	ldhu .D2T2 *+b14(w2-w1),b1
	ldhu .D2T1 *+b14(w4-w3),a1
	ldw .D2T2 *+b14(ext1),b1
	ldw .D2T1 *+b15(ext2+4),a1
	ldw .D2T2 *+b15(b1),b1
	ldw .D2T1 *+b14(a1),a1
	ldw .D2T2 *+b14(w2-w1),b1
	ldw .D2T1 *+b14(w4-w3),a1
	ldw .D2T2 *+b14($DSBT_INDEX(__c6xabi_DSBT_BASE)),b1
	ldw .D2T1 *+b14($GOT(ext2)+4),a1
	stb .D2T2 b1,*+b14(ext1)
	stb .D2T1 a1,*+b15(ext2+7)
	stb .D2T2 b1,*+b15(b1)
	stb .D2T1 a1,*+b14(a1)
	stb .D2T2 b1,*+b14(w2-w1)
	stb .D2T1 a1,*+b14(w4-w3)
	sth .D2T2 b1,*+b14(ext1)
	sth .D2T1 a1,*+b15(ext2+6)
	sth .D2T2 b1,*+b15(b1)
	sth .D2T1 a1,*+b14(a1)
	sth .D2T2 b1,*+b14(w2-w1)
	sth .D2T1 a1,*+b14(w4-w3)
	stw .D2T2 b1,*+b14(ext1)
	stw .D2T1 a1,*+b15(ext2+4)
	stw .D2T2 b1,*+b15(b1)
	stw .D2T1 a1,*+b14(a1)
	stw .D2T2 b1,*+b14(w2-w1)
	stw .D2T1 a1,*+b14(w4-w3)
	stw .D2T2 b1,*+b14($DSBT_INDEX(__c6xabi_DSBT_BASE))
	stw .D2T1 a1,*+b14($GOT(ext2)+4)
.data
w3:
	.word 3
w4:
	.word 4
s0:
	.space 131073
s1:
	.word 5
