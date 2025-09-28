	.data
	.align	4096
	.globl	sym
sym:	.word	0x12345678

local_sym:
	.word	0xdeadbeef

	.text
	.align	4096
.LLGETPC0:
	retl
	add	%o7, %l7, %l7

	.globl	foo
	.type	foo,#function
	.proc	04
foo:
	save	%sp, -104, %sp
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-4), %l7
	call	.LLGETPC0
	add	%l7, %lo(_GLOBAL_OFFSET_TABLE_+4), %l7
	nop
	sethi	%gdop_hix22(sym), %l1
	nop
	xor	%l1, %gdop_lox10(sym), %l1
	nop
	ld	[%l7 + %l1], %i0, %gdop(sym)
	nop
	sethi	%gdop_hix22(local_sym), %l1
	nop
	xor	%l1, %gdop_lox10(local_sym), %l1
	nop
	ld	[%l7 + %l1], %i0, %gdop(local_sym)
	nop
	ret
	restore
