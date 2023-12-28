	.file	"tester.c"
.text
	.global	a
.data
	.balign 2
	.type	a, @object
	.size	a, 2
a:
	.short	5
.text
	.balign 2
	.global	foo
	.type	foo, @function
foo:
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
	MOV.W	R12, @R1
	MOV.W	@R1, R12
	ADD.W	#-2, R12
	MOV.W	@R12, R12
	CMP.W	#0, R12 { JEQ	.L2
	MOV.B	#0, R12
	BR	#.L3
.L2:
	MOV.B	#1, R12
.L3:
	; start of epilogue
	ADD.W	#2, R1
	RET
	.size	foo, .-foo
	.balign 2
	.global	main
	.type	main, @function
main:
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
	MOV.W	#a, R12
	CALL	#foo
	; start of epilogue
	.refsym	__crt0_call_exit
	RET
	.size	main, .-main
	.ident	"GCC: (jozef) 7.3.2"
