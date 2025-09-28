	.text
_start:
	.nop

	.irp n, ab, ij, xy
	.file "irp.s"
	.line 7
	.section .text.\n, "ax"
	.nop
	.nop
	.endr

	.text

	.irpc n, 123
	.file "irpc.s"
	.line 16
	.subsection \n
	.nop
	.nop
	.endr

	n = 9
	.rept 3
	.file "rept.s"
	.line 26
	.subsection n
	.nop
	.nop
	n = n - 1
	.endr

	.irp n, cd, nm
# 35 "irp.s"
	.section .text.\n, "ax"
	.nop
	.nop
	.endr

	.irp n, ef, kl
	.section .text.\n, "ax"
	.nop
	.nop
	.endr
