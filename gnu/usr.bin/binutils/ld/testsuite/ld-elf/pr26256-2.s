	.section .text,"ax",%progbits,unique,0
	.globl text0
text0:
	.nop
	.section .text,"ax",%progbits,unique,1
	.globl text1
text1:
	.nop
	.section .linkorder,"ao",%progbits,0,unique,0
	.globl linkorder2
linkorder2:
	.byte 0
	.section .linkorder,"ao",%progbits,text0
	.globl linkorder0
linkorder0:
	.byte 1
	.section .linkorder,"ao",%progbits,text1
	.globl linkorder1
linkorder1:
	.byte 2
	.section .linkorder,"a",%progbits
	.globl linkorder3
linkorder3:
	.byte 3
	.section .linkorder,"ao",%progbits,0,unique,3
	.globl linkorder4
linkorder4:
	.byte 4
	.text
	.global _start
_start:
	.nop
