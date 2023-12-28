;-------------------------------------------------------------------------------
; Comments here

    .text
    .arm

;-------------------------------------------------------------------------------

    .ref	ext_sym
    .def	_test_func
    .asmfunc

_test_func
		stmfd	r13!, {r0 - r12, lr}; push registers and link register on to stack

        ldr		r12, sym1			; another comment
        ldr		r0,  [r12]
        tst		r0,  #0x8
        bne		aLabel
        ldr		r0,  [r12]

aLabel
		bl		ext_sym		; custom data abort handler required

		ldmfd	r13!, {r0 - r12, lr}; pop registers and link register from stack
		subs	pc, lr, #8

sym1		.word	0xFFFFF520


    .endasmfunc
	
	
