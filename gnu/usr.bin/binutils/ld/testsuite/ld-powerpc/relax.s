	.globl _start
_start:
	bl near
	bl far
	bl near
	bl far
	b _start
