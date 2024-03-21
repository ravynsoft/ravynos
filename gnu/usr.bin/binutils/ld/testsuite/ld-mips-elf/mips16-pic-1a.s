	.abicalls
	.set	noreorder
	.include "mips16-pic-1.inc"

	# Test local stubs that are only used by MIPS16 PIC calls in this file.
	decl	unused1,lstub
	callpic	unused1,mips16

	# Test local stubs that are only used by MIPS16 jals in this file.
	decl	unused2,lstub
	jals	unused2,mips16

	# Test local stubs that aren't called at all.
	decl	unused3,lstub

	# Test hidden stubs that are called by MIPS16 PIC calls in this file.
	decl	unused4,hstub
	callpic	unused4,mips16

	# Test hidden stubs that are called by MIPS16 jals in this file.
	decl	unused5,hstub
	jals	unused5,mips16

	# Test hidden stubs that are called by MIPS16 PIC calls in another file.
	decl	unused6,hstub

	# Test hidden stubs that are called by MIPS16 jals in another file.
	decl	unused7,hstub

	# Test hidden stubs that aren't called at all.
	decl	unused8,hstub

	# Test global stubs that are called by MIPS16 jals in this file.
	decl	unused9,gstub
	jals	unused9,mips16

	# Test global stubs that are called by MIPS16 jals in another file.
	decl	unused10,gstub

	# Test global stubs that aren't called at all.
	decl	unused11,gstub

	# Test local stubs that are used by non-MIPS16 PIC calls in this file.
	decl	used1,lstub
	callpic	used1,nomips16

	# Test local stubs that are used by non-MIPS16 jals in this file.
	decl	used2,lstub
	jals	used2,nomips16

	# Test local stubs that are used by both MIPS16 and non-MIPS16 PIC
	# calls in this file.
	decl	used3,lstub
	callpic	used3,nomips16
	callpic	used3,mips16

	# Test local stubs that are used by both MIPS16 and non-MIPS16 jals
	# in this file.
	decl	used4,lstub
	jals	used4,nomips16
	jals	used4,mips16

	# Test local stubs that are used by a combination of MIPS16 PIC calls
	# and non-MIPS16 jals in this file.
	decl	used5,lstub
	jals	used5,nomips16
	callpic	used5,mips16

	# Test hidden stubs that are used by non-MIPS16 PIC calls in this file.
	decl	used6,hstub
	callpic	used6,nomips16

	# Test hidden stubs that are used by non-MIPS16 jals in this file.
	decl	used7,hstub
	jals	used7,nomips16

	# Test hidden stubs that are used by non-MIPS16 PIC calls in another
	# file.
	decl	used8,hstub

	# Test hidden stubs that are used by non-MIPS16 jals in another
	# file.
	decl	used9,hstub

	# Test hidden stubs that are used by both MIPS16 and non-MIPS16 PIC
	# calls in this file.
	decl	used10,hstub
	callpic	used10,nomips16
	callpic	used10,mips16

	# Test hidden stubs that are used by both MIPS16 and non-MIPS16 jals
	# in this file.
	decl	used11,hstub
	jals	used11,nomips16
	jals	used11,mips16

	# Test hidden stubs that are used by a combination of MIPS16 PIC calls
	# and non-MIPS16 jals in this file.
	decl	used12,hstub
	jals	used12,nomips16
	callpic	used12,mips16

	# Test global stubs that are used by non-MIPS16 PIC calls in this file.
	decl	used13,gstub
	callpic	used13,nomips16

	# Test global stubs that are used by non-MIPS16 jals in this file.
	decl	used14,gstub
	jals	used14,nomips16

	# Test global stubs that are used by non-MIPS16 PIC calls in another
	# file.
	decl	used15,gstub

	# Test global stubs that are used by non-MIPS16 jals in another file.
	decl	used16,gstub

	# Test global stubs that are used by both MIPS16 and non-MIPS16 PIC
	# calls in this file.
	decl	used17,gstub
	callpic	used17,nomips16
	callpic	used17,mips16

	# Test global stubs that are used by both MIPS16 and non-MIPS16 jals
	# in this file.
	decl	used18,gstub
	jals	used18,nomips16
	jals	used18,mips16

	# Test global stubs that are used by a combination of MIPS16 PIC calls
	# and non-MIPS16 jals in this file.
	decl	used19,gstub
	jals	used19,nomips16
	callpic	used19,mips16

	# Test global stubs that are used by MIPS16 PIC calls in this file.
	# We currently force all targets of call16 relocations to be dynamic,
	# and the stub must be the definition of the dynamic symbol.
	decl	used20,gstub
	callpic	used20,mips16

	# Test global stubs that are used by MIPS16 PIC calls in another file.
	# Needed for the same reason as used21.
	decl	used21,gstub
