	.file	"lds-mega.s"
__tmp_reg__ = 0
	.text
.global	main
	.type	main, @function
main:
.L__stack_usage = 0
	lds r24,256
	mov __tmp_reg__,r24
	lsl r0
	sbc r25,r25
	sts myvar2+1,r25
	sts myvar2,r24
	ldi r24,0
	ldi r25,0
	ret
	.size	main, .-main
	.comm	myvar2,2,1
	.comm	myvar1,2,1

