	.macro	test_call,name
	.set	mips16
	.text
	.ent	test_\name
test_\name:
	jal	\name
	.end	test_\name
	.endm

	.macro	call_stub,name
	.set	nomips16
	.section .mips16.call.\name, "ax", @progbits
	.ent	__call_\name
__call_\name:
	la	$25,\name
	jr	$25
	mtc1	$4,$f12
	.end	__call_\name

	test_call \name
	.endm

	.macro	call_fp_stub,name
	.set	nomips16
	.section .mips16.call.fp.\name, "ax", @progbits
	.ent	__call_fp_\name
__call_fp_\name:
	move	$18,$31
	la	$25,\name
	jalr	$25
	mtc1	$4,$f12
	mfc1	$2,$f0
	jr	$18
	nop
	.end	__call_fp_\name

	test_call \name
	.endm

	.macro	lstub,name,mode
	.set	\mode
	.text
	.ent	\name
\name:
	jr	$31
	nop
	.end	\name
	.endm

	.macro	hstub,name,mode
	.globl	\name
	.hidden	\name
	lstub	\name, \mode
	.endm

	.macro	gstub,name,mode
	.globl	\name
	lstub	\name, \mode
	.endm
