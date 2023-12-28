# .arch directives merge attributes after instructions are seen.
.text
.globl f
f:
	nop
.arch c67x
.arch c64x+
