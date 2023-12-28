@ Test intended to fail for LDC group relocations.

@ We will place .text at 0x8000.

	.text
	.globl _start

_start:
	add	r0, r0, #:sb_g0_nc:(bar)
	ldc	0, c0, [r0, #:sb_g1:(bar)]

@ We will place the section foo at 0x118400 but that should be irrelevant
@ for sb_g* relocations.

	.section foo
	.set bar,foo + 0x123456

