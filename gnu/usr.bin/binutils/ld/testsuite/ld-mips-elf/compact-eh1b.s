	.cfi_sections .eh_frame_entry
	.section	.text.compact1b,"ax",@progbits
	.globl	compact1b
	.type	compact1b, @function
	.cfi_startproc
compact1b:
	lw	$31,44($sp)
	lw	$fp,40($sp)
	addiu	$sp,$sp,48
	j	$31
	nop
	.cfi_fde_data 0x55
	.cfi_endproc
	.globl	e22
	.cfi_startproc
__e22:
	sw	$2,24($fp)
	lw	$2,24($fp)
	xori	$2,$2,0x1
	andi	$2,$2,0x00ff
	.cfi_fde_data 0x55
	.cfi_endproc
