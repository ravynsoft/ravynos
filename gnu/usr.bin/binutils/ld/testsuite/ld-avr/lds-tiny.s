	.file	"lds-tiny.s"
	.text
.global	main
	.type	main, @function
main:
.L__stack_usage = 0
	lds r18, 0x40
    lds r20, 0x42
    lds r21, 0x43
	ret
	.size	main, .-main
	.comm	myvar1,1,1
    .comm	myvar2,2,1

