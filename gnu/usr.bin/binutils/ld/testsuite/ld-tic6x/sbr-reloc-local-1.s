.text
.nocmp
	ldw .D2T2 *+b14(a),b1
	ldw .D2T2 *+b14(b),b1
	ldh .D2T2 *+b14(b),b1
	ldh .D2T2 *+b14(c),b1
	ldb .D2T2 *+b14(c),b1
	ldb .D2T2 *+b14(d),b1
	mvk .S1 $dpr_byte(d),a1
	mvkl .S1 $dpr_byte(c),a1
	mvkh .S1 $dpr_byte(d),a1
	mvkl .S1 $dpr_hword(b),a1
	mvkh .S1 $dpr_hword(c),a1
	mvkl .S1 $dpr_word(a),a1
	mvkh .S1 $dpr_word(b),a1
.data
a:
	.word 0
b:
	.short 0
c:
	.byte 0
d:
	.byte 0
