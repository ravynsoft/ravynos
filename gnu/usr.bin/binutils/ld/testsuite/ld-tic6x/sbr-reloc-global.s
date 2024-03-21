.globl sw1
.globl sw2
.globl sh1
.globl sh2
.globl sb1
.globl sb2
.globl sb16a
.globl sb16b
.globl sbw
.globl shw
.globl sww
.text
.nocmp
	ldw .D2T2 *+b14(sw1),b1
	ldw .D2T2 *+b14(sw2),b1
	ldh .D2T2 *+b14(sh1),b1
	ldh .D2T2 *+b14(sh2),b1
	ldb .D2T2 *+b14(sb1),b1
	ldb .D2T2 *+b14(sb2),b1
	mvk .S1 $dpr_byte(sb16a),a1
	mvk .S1 $dpr_byte(sb16b),a1
	mvkl .S1 $dpr_byte(sbw),a1
	mvkh .S1 $dpr_byte(sbw),a1
	mvkl .S1 $dpr_hword(shw),a1
	mvkh .S1 $dpr_hword(shw),a1
	mvkl .S1 $dpr_word(sww),a1
	mvkh .S1 $dpr_word(sww),a1
