	.text
	.global _start
_start:
	.dc.a foo
	.data
	.globl foo
foo:
	.byte 0

	.section	.note.GNU-stack
