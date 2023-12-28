.globl _start

.section .text.hot
_start:
	la.global $r20,cc
	jirl $r1, $r20, 0
	addi.w  $r11,$r0,93
	addi.w  $r4,$r0,0
	syscall 0
