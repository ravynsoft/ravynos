# Test for Nios II 32-bit relocations

.global globalfunc
.text
.set norelax
.set noat
start:
	call localfunc
	call globalfunc

.align 8	
localfunc:
	nop
