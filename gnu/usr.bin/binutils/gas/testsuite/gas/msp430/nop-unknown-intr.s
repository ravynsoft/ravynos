	.text

;;; Test for warnings when an instruction might change interrupt state, but
;;; the assembler doesn't know whether interrupts will be enabled or disabled.
;;; "MOV &FOO,R10" is used as an artbitrary statement which isn't a NOP, to
;;; break up the instructions being tested.

;;; Moving a value in memory into SR might change interrupt state
	MOV &FOO,R2

	MOV &FOO,R10

;;; Moving a value from a register into SR might change interrupt state
	MOV R7,R2

	MOV &FOO,R10
