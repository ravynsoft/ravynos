	.abicalls
	.set	noreorder

	# Define a stub for f1, which is defined in another file.
	#
	# (It's questionable whether defining the stub and real function
	# in separate files is really valid or useful.  However, if we
	# accept it without error, we should do something useful with it.)

	.section .mips16.fn.f1, "ax", @progbits
	.ent	__fn
__fn:
	.reloc	0,R_MIPS_NONE,f1
	la	$25,_f1
	jr	$25
	mfc1	$4,$f12
	.end	__fn

	# Define dummy functions for the executable to call.

	.macro	dummy,name
	.text
	.global	\name
	.type	\name,@function
	.ent	\name
\name:
	jr	$31
	nop
	.end	\name
	.endm

	dummy	f2
	dummy	f3
