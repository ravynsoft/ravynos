# .arch directives override previous attributes before instructions
# are seen, but not after.
.text
.globl f
f:
.arch c67x
.arch c62x
	nop
.arch c64x+
