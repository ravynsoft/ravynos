	.text
	.global _start
_start:
	.long foo
	.data
	.globl foo
foo:
	.byte 0
