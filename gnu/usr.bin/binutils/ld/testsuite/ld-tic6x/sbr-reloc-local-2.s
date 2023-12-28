.text
.nocmp
	ldw .D2T2 *+b14(e),b1
	ldw .D2T2 *+b14(f),b1
	ldh .D2T2 *+b14(f),b1
	ldh .D2T2 *+b14(g),b1
	ldb .D2T2 *+b14(g),b1
	ldb .D2T2 *+b14(h),b1
	mvk .S1 $dpr_byte(h),a1
	mvkl .S1 $dpr_byte(g),a1
	mvkh .S1 $dpr_byte(h),a1
	mvkl .S1 $dpr_hword(f),a1
	mvkh .S1 $dpr_hword(g),a1
	mvkl .S1 $dpr_word(e),a1
	mvkh .S1 $dpr_word(f),a1
.data
e:
	.word 0
f:
	.short 0
g:
	.byte 0
h:
	.byte 0
