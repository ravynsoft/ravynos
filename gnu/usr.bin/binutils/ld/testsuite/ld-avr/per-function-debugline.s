	.file	"per-function-debugline.s"
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__SREG__ = 0x3f
__RAMPZ__ = 0x3b
__tmp_reg__ = 0
__zero_reg__ = 1
	.comm	g,2,1
	.section	.text.bar,"ax",@progbits
.global	bar
	.type	bar, @function
bar:
	push r28
	push r29
	rcall .
	in r28,__SP_L__
	in r29,__SP_H__
/* prologue: function */
/* frame size = 2 */
/* stack size = 4 */
.L__stack_usage = 4
	ldd r24,Y+1
	ldd r25,Y+2
	adiw r24,1
	std Y+2,r25
	std Y+1,r24
	nop
/* epilogue start */
	pop __tmp_reg__
	pop __tmp_reg__
	pop r29
	pop r28
	ret
	.size	bar, .-bar
	.section	.text.foo,"ax",@progbits
.global	foo
	.type	foo, @function
foo:
	push r28
	push r29
	in r28,__SP_L__
	in r29,__SP_H__
/* prologue: function */
/* frame size = 0 */
/* stack size = 2 */
.L__stack_usage = 2
	lds r24,g
	lds r25,g+1
	adiw r24,1
	sts g+1,r25
	sts g,r24
	nop
/* epilogue start */
	pop r29
	pop r28
	ret
	.size	foo, .-foo
	.section	.text.main,"ax",@progbits
.global	main
	.type	main, @function
main:
	push r28
	push r29
	in r28,__SP_L__
	in r29,__SP_H__
/* prologue: function */
/* frame size = 0 */
/* stack size = 2 */
.L__stack_usage = 2
	call foo
	lds r24,g
	lds r25,g+1
	sbiw r24,1
	sts g+1,r25
	sts g,r24
	lds r24,g
	lds r25,g+1
/* epilogue start */
	pop r29
	pop r28
	ret
	.size	main, .-main
	.ident	"GCC: (GNU) 6.0.0 20150630 (experimental)"
.global __do_clear_bss
