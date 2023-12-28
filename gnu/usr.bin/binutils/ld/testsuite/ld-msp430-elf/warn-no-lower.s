	.file	"tester.c"
.text
	.global	foo
	.section	.rodata
.LC0:
	.string	"bar"
	.section	.data,"aw",@progbits
	.balign 2
	.type	foo, @object
	.size	foo, 2
foo:
	.short	.LC0
	.section	.text,"ax",@progbits
	.balign 2
	.global	main
	.type	main, @function
main:
; start of function
; framesize_regs:     0
; framesize_locals:   2
; framesize_outgoing: 0
; framesize:          2
; elim ap -> fp       2
; elim fp -> sp       2
; saved regs:(none)
	; start of prologue
	SUB.W	#2, R1
	; end of prologue
	MOV.W	#1, @R1
	BR	#.L2
.L3:
	MOV.W	&foo, R12
	ADD.W	#-1, R12
	MOV.W	R12, &foo
.L2:
	MOV.W	@R1, R12
	CMP.W	#0, R12 { JNE	.L3
	MOV.B	#0, R12
	; start of epilogue
	.refsym	__crt0_call_exit
	ADD.W	#2, R1
	RET
	.size	main, .-main
	.ident	"GCC: (jozef) 7.3.2"
