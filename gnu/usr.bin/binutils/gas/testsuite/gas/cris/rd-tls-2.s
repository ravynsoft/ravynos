; Check that TLS PIC operands get their right relocation type.
; First some expected uses, similar to what GCC will emit.
	.text
	.syntax no_register_prefix
start:
	move.d [r3+extsym:TPOFFGOT],r10
	move.d extsym5:TPOFFGOT,r8
	move.w extsym9:TPOFFGOT16,r8
	move.d [r3+extsym:GDGOTREL],r10
	move.d [r13+extsym13:TPOFFGOT16],r10
	move.w extsym14:GDGOTREL16,r10
	sub.d extsym4:DTPREL+22,r9
	sub.w extsym4:DTPREL16-86,r3

; Other for GAS valid operands (some with questionable PIC semantics).
	sub.d [r3+extsym3:TPOFFGOT],r4,r10
	sub.d extsym4:GDGOTREL+42,r9
	sub.d extsym4:TPOFFGOT-96,r3
	add.d [r10+extsym3:TPOFFGOT+56],r7,r8
	move.d [r5+extsym6:TPOFFGOT+10],r1
	add.d [r10+extsym3:TPOFFGOT-560],r4,r8
	move.d [r5+extsym6:TPOFFGOT-110],r12
	move.d [r9=r5+extsym6:TPOFFGOT-220],r12
	sub.d [r12+extsym3:TPOFFGOT16-156],r9,r8
	move.d [r11+extsym14:GDGOTREL16-256],r9
	add.d [r10+extsym3:GDGOTREL+56],r7,r8
