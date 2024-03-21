	.file	"foo.c"
.text
	.section	.text.foo1,"ax",@progbits
	.balign 2
	.global	foo1
	.type	foo1, @function
foo1:
; start of function
; framesize_regs:     0
; framesize_locals:   0
; framesize_outgoing: 0
; framesize:          0
; elim ap -> fp       2
; elim fp -> sp       0
; saved regs:(none)
	; start of prologue
	; end of prologue
	NOP
.L2:
	MOV.W	&a, R12
	CMP.W	#0, R12 { JNE	.L2
	MOV.B	#0, R12
	; start of epilogue
	RET
	.size	foo1, .-foo1
	.ident	"GCC: (jozef) 7.3.2"
