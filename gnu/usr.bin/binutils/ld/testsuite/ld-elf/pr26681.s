    	.text
	.globl _start
_start:
	.nop

	.section .unused1, "ax", %progbits
	.nop

	.section .gnu.note1, "o", %note, .unused1
	.word 2
