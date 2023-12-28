; Check that non-PIC TLS operands get their right relocation type.
; First some expected uses, similar to what GCC will emit.

	.section .tdata,"awT",@progbits
	.type	x, @object
	.size	x, 4
x:
	.dword 0

	.text
	.syntax no_register_prefix
start:
	sub.d x:GD,r1
	add.d extsym2:GD,r9
	move.d [r3+extsym:TPOFF],r10
	move.w extsym14:TPOFF16+77,r10

; Other for GAS valid operands (some with questionable PIC semantics).
	sub.d extsym4:GD+42,r9
	sub.d extsym4:GD-96,r3
	move.d [r7=r3+extsym10:GD-330],r13
	move.d [r11+extsym14:TPOFF16-256],r9
	add.d [r10+extsym3:TPOFF+56],r7,r8
	move.d [extsym5:IE],r1
	add.d extsym7:IE,r11
