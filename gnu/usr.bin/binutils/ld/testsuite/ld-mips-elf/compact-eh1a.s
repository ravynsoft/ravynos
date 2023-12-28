	.cfi_sections .eh_frame_entry
	.section .text.compact1a,"ax",@progbits
	.globl	compact1a
	.type	compact1a, @function
	.cfi_startproc
compact1a:
	sw	$2,16($fp)
	lw	$2,16($fp)
	lw	$5,4($3)
	addiu	$3,$2,1
	.cfi_fde_data 0x5e,0x3,0x59,0xf1
	.cfi_endproc
