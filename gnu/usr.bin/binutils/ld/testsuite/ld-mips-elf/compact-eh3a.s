	.cfi_sections .eh_frame_entry
	.section	.text.compact3a,"ax",@progbits
	.globl	compact3a
	.type	compact3a, @function
	.cfi_startproc
compact3a:
	lw	$31,44($sp)
	lw	$fp,40($sp)
	addiu	$sp,$sp,48
	j	$31
	nop
	.cfi_endproc
	.globl	e22
	.cfi_startproc
__e22:
	sw	$2,24($fp)
	lw	$2,24($fp)
	xori	$3,$4,0x1
	andi	$5,$6,0x00ff
	.cfi_endproc
