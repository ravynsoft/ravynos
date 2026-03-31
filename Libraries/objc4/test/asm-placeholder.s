.macro NOP16
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
.endmacro

.macro NOP256
	NOP16
	NOP16
	NOP16
	NOP16
	NOP16
	NOP16
	NOP16
	NOP16
	NOP16
	NOP16
	NOP16
	NOP16
	NOP16
	NOP16
	NOP16
	NOP16
.endmacro

.text
	.globl _main
	.align 14
_main:
	// at least 1024 instruction bytes on all architectures
	NOP256
	NOP256
	NOP256
	NOP256
