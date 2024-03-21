	# Declare a function called NAME and an __fn_NAME stub for it.
	# Make the stub use la_TYPE to load the target address into $2.
	.macro	stub,name,type
	.set	nomips16
	.section .mips16.fn.\name, "ax", @progbits
	.ent	__fn_\name
__fn_\name:
	la_\type \name
	mfc1	$4,$f12
	jr	$2
	nop
	.end	__fn_\name

	.set	mips16
	.text
	.ent	\name
\name:
__fn_local_\name:
	jr	$31
	nop
	.end	\name
	.endm

	# Like stub, but ensure NAME is a local symbol.
	.macro	lstub,name,type
	stub	\name, \type
	.equ	local_\name,1
	.endm

	# Like stub, but ensure NAME is a hidden symbol.
	.macro	hstub,name,type
	.globl	\name
	.hidden	\name
	stub	\name, \type
	.endm

	# Like lstub, but make the MIPS16 function global rather than local.
	.macro	gstub,name,type
	.globl	\name
	stub	\name, \type
	.endm

	# Use an absolute sequence to load NAME into a register.
	.macro	la_noshared,name
	lui	$2,%hi(\name)
	addiu	$2,$2,%lo(\name)
	.endm

	# Use the normal PIC sequence to load __fn_local_NAME into $2
	# and emit a dummy relocation against NAME.  This macro is always
	# used at the start of a function.
	.macro	la_shared,name
	.reloc	0,R_MIPS_NONE,\name
	.cpload	$25
	la	$2,__fn_local_\name
	.endm

	# Use TYPE (either LSTUB, HSTUB or GSTUB) to define functions
	# called a_NAME and b_NAME.  The former uses absolute accesses
	# and the latter uses PIC accesses.
	.macro	decl,name,type
	\type	a_\name, noshared
	\type	b_\name, shared
	.endm

	# Emit the MIPS16 PIC sequence for setting $28 from $25.
	# Make the value of $25 available in $2 as well.
	.macro	cpload_mips16
	li	$2,%hi(_gp_disp)
	addiu	$3,$pc,%lo(_gp_disp)
	sll	$2,16
	addu	$2,$2,$3
	move	$28,$2
	.endm

	# Likewise, but for non-MIPS16 code.
	.macro	cpload_nomips16
	.cpload	$25
	move	$2,$28
	.endm

	# Start a PIC function in ISA mode MODE, which is either "mips16"
	# or "nomips16".
	.macro	pic_prologue,mode
	cpload_\mode
	addiu	$sp,$sp,-32
	sw	$2,16($sp)
	sw	$31,20($sp)
	.endm

	# Use a PIC function to call NAME.
	.macro	pic_call,name,mode
	.ifdef	local_\name
	.ifc	\mode,mips16
	lw	$2,%got(__fn_local_\name)($2)
	addiu	$2,%lo(__fn_local_\name)
	.else
	lw	$2,%got(\name)($2)
	addiu	$2,%lo(\name)
	.endif
	.else
	lw	$2,%call16(\name)($2)
	.endif
	jalr	$2
	move	$25,$2
	lw	$2,16($sp)
	move	$28,$2
	.endm

	# Finish a PIC function started by pic_prologue.
	.macro	pic_epilogue
	lw	$2,20($sp)
	jr	$2
	addiu	$sp,$sp,32
	.endm

	# Use PIC %call16 sequences to call a_NAME and b_NAME.
	# MODE selects the ISA mode of the code: either "mips16"
	# or "nomips16".
	.macro	callpic,name,mode
	.text
	.set	\mode
	.ent	callpic_\name\()_\mode
callpic_\name\()_\mode:
	pic_prologue \mode
	pic_call     a_\name,\mode
	pic_call     b_\name,\mode
	pic_epilogue
	.end	callpic_\name\()_\mode
	.endm

	# Use absolute jals to call a_NAME and b_NAME.  MODE selects the
	# ISA mode of the code: either "mips16" or "nomips16".
	.macro	jals,name,mode
	.text
	.set	\mode
	.ent	jals_\name\()_\mode
jals_\name\()_\mode:
	.option	pic0
	jal	a_\name
	nop

	jal	b_\name
	nop
	.option	pic2
	.end	jals_\name\()_\mode
	.endm
