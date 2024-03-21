	.section	.text.loseme,"ax",%progbits
	.globl	loseme
	.type	loseme,%function
loseme:
	.cfi_startproc
	.cfi_personality 0,__gxx_personality_v0
	.long 0
	.cfi_endproc
	.size loseme, . - loseme

	.section	.text.loseme2,"ax",%progbits
	.globl	loseme2
	.type	loseme2,%function
loseme2:
	.cfi_startproc
	.cfi_personality 0,__gxx_personality_v1
	.long 0
	.cfi_endproc
	.size loseme2, . - loseme2

	.section	.text.main,"ax",%progbits
	.globl	main
	.type	main, %function
main:
	.cfi_startproc
	.long 0
	.cfi_endproc
	.size main, . - main
