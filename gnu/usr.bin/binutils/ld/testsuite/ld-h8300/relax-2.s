	.h8300h
	.globl	_start
_start:
	mov.b	@foo:16,r0l
	mov.b	r0l,@foo:16
	mov.b	@bar:32,r0l
	mov.b	r0l,@bar:32

	mov.w	@foo:16,r0
	mov.w	r0,@foo:16
	mov.w	@bar:32,r0
	mov.w	r0,@bar:32

	mov.l	@foo:16,er0
	mov.l	er0,@foo:16
	mov.l	@bar:32,er0
	mov.l	er0,@bar:32

	.equ	foo,0xffff64
	.equ	bar,0x4320
