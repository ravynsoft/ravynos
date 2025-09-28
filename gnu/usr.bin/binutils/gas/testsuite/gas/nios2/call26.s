# Test for Nios II 32-bit relocations

.global globalfunc
.text
.set norelax
start:
	call localfunc
	call globalfunc

.align 8	
localfunc:
	nop
