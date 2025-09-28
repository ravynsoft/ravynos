@ Test intended to fail for LDRS group relocations.

@ We will place .text at 0x8000.

	.text
	.globl _start

_start:
	add	r0, r0, #:pc_g0_nc:(bar)
	ldrd	r2, [r0, #:pc_g1:(bar + 4)]

@ We will place the section foo at 0x8000100.

	.section foo

bar:
	mov r0, #0
