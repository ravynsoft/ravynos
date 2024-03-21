@ Test intended to fail for ALU group relocations.

@ We will place .text at 0x8000.

	.text
	.globl _start

_start:
	add r0, r0, #:sb_g0:(bar)

@ We will place the section foo at 0x1208000 but that should be irrelevant
@ for sb_g* relocations.

	.section foo
	.set bar,foo + 0x1234

