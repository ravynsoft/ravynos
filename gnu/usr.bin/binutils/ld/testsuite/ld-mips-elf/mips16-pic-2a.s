	.abicalls
	.set	noreorder
	.include "mips16-pic-1.inc"

	# Test local stubs that are only used by MIPS16 PIC calls in this file.
	lstub	unused1,shared

	# Test local stubs that aren't called at all.
	lstub	unused2,shared

	# Test hidden stubs that are called by MIPS16 PIC calls in this file.
	hstub	unused3,shared

	# Test hidden stubs that are called by MIPS16 PIC calls in another file.
	hstub	unused4,shared

	# Test hidden stubs that aren't called at all.
	hstub	unused5,shared


	# Test local stubs that are referenced by absolute relocations
	# in this file.
	lstub	used1,shared

	# Test hidden stubs that are referenced by absolute relocations
	# in this file.
	hstub	used2,shared

	# Test hidden stubs that are referenced by absolute relocations
	# in another file.
	hstub	used3,shared

	# Test global stubs that are referenced by absolute relocations
	# in this file.
	gstub	used4,shared

	# Test global stubs that are referenced by absolute relocations
	# in another file.
	gstub	used5,shared

	# Test global stubs that are called by MIPS16 PIC calls in this file.
	gstub	used6,shared

	# Test global stubs that are called by MIPS16 PIC calls in another file.
	gstub	used7,shared

	# Test global stubs that aren't referenced at all.
	gstub	used8,shared

	.set	mips16
	.ent	foo
foo:
	pic_prologue mips16
	pic_call     unused1,mips16
	pic_call     unused3,mips16
	pic_call     used6,mips16
	pic_epilogue
	.end	foo

	.data
	.word	used1
	.word	used2
	.word	used4
