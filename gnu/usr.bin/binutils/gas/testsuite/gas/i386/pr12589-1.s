	 .text
	 .globl _start
_start:
	jmp scn_pnp
zerob:
	zeroln = zerob - _start
	.=.+zeroln
scn_pnp:
	mov %eax,%eax
