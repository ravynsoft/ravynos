	.text
	.arch generic32

#; func_locvars
#; - function with a space on the stack 
#;   allocated for local variables

func_locvars:
	.cfi_startproc
	
	#; alocate space for local vars
	sub	$0x1234,%esp
	.cfi_adjust_cfa_offset	0x1234
	
	#; dummy body
	movl	$1,%eax
	
	#; release space of local vars and return
	add	$0x1234,%esp
	.cfi_adjust_cfa_offset	-0x1234
	ret
	.cfi_endproc

#; func_prologue
#; - functions that begins with standard
#;   prologue: "pushq %rbp; movq %rsp,%rbp"

func_prologue:
	.cfi_startproc
	
	#; prologue, CFI is valid after 
	#; each instruction.
	pushl	%ebp
	.cfi_def_cfa_offset	8
	.cfi_offset		ebp,-8
	movl	%esp, %ebp
	.cfi_def_cfa_register	ebp

	#; function body
	call	func_locvars
	addl	$3, %eax

	#; epilogue with valid CFI
	#; (we're better than gcc :-)
	leave
	.cfi_def_cfa_register	esp
	ret
	.cfi_endproc

#; func_otherreg
#; - function that moves frame pointer to 
#;   another register (ebx) and then allocates 
#;   a space for local variables

func_otherreg:
	.cfi_startproc

	#; save frame pointer to ebx
	mov	%esp,%ebx
	.cfi_def_cfa_register	ebx

	#; alocate space for local vars
	#;  (no .cfi_{def,adjust}_cfa_offset here,
	#;   because CFA is computed from ebx!)
	sub	$100,%esp

	#; function body
	call	func_prologue
	add	$2, %eax
	
	#; restore frame pointer from ebx
	mov	%ebx,%esp
	.cfi_def_cfa		esp,4
	ret
	.cfi_endproc

#; main
#; - typical function
main:
	.cfi_startproc
	
	#; only function body that doesn't
	#; touch the stack at all.
	call	func_otherreg
	
	#; return
	ret
	.cfi_endproc

#; _start
#; - standard entry point

	.globl	_start
_start:
	.cfi_startproc
	call	main
	movl	%eax,%edi
	movl	$0x1,%eax
	int	$0x80
	hlt
	.cfi_endproc

#; func_all_registers
#; - test for all .cfi register numbers. 
#;   This function is never called and the CFI info doesn't make sense.

func_all_registers:
	.cfi_startproc simple

	.cfi_undefined eip	; nop
	.cfi_undefined eax	; nop
	.cfi_undefined ecx	; nop
	.cfi_undefined edx	; nop
	.cfi_undefined ebx	; nop
	.cfi_undefined esp	; nop
	.cfi_undefined ebp	; nop
	.cfi_undefined esi	; nop
	.cfi_undefined edi	; nop
	.cfi_undefined eflags	; nop

	.cfi_undefined es	; nop
	.cfi_undefined cs	; nop
	.cfi_undefined ds	; nop
	.cfi_undefined ss	; nop
	.cfi_undefined fs	; nop
	.cfi_undefined gs	; nop
	.cfi_undefined tr	; nop
	.cfi_undefined ldtr	; nop

	.cfi_undefined mxcsr	; nop
	.cfi_undefined xmm0	; nop
	.cfi_undefined xmm1	; nop
	.cfi_undefined xmm2	; nop
	.cfi_undefined xmm3	; nop
	.cfi_undefined xmm4	; nop
	.cfi_undefined xmm5	; nop
	.cfi_undefined xmm6	; nop
	.cfi_undefined xmm7	; nop

	.cfi_undefined fcw	; nop
	.cfi_undefined fsw	; nop
	.cfi_undefined st	; nop
	.cfi_undefined st(1)	; nop
	.cfi_undefined st(2)	; nop
	.cfi_undefined st(3)	; nop
	.cfi_undefined st(4)	; nop
	.cfi_undefined st(5)	; nop
	.cfi_undefined st(6)	; nop
	.cfi_undefined st(7)	; nop

	.cfi_undefined mm0	; nop
	.cfi_undefined mm1	; nop
	.cfi_undefined mm2	; nop
	.cfi_undefined mm3	; nop
	.cfi_undefined mm4	; nop
	.cfi_undefined mm5	; nop
	.cfi_undefined mm6	; nop
	.cfi_undefined mm7	; nop

	.cfi_undefined k0	; nop
	.cfi_undefined k1	; nop
	.cfi_undefined k2	; nop
	.cfi_undefined k3	; nop
	.cfi_undefined k4	; nop
	.cfi_undefined k5	; nop
	.cfi_undefined k6	; nop
	.cfi_undefined k7	; nop

	.cfi_endproc
