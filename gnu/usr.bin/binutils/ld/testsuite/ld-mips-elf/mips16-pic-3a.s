	.abicalls
	.option	pic0
	.set	noreorder
	.include "mips16-pic-3.inc"

	# For symbols called by a .call stub in this file.
	hstub	unused1,mips16

	# For symbols called by a .call.fp stub in this file.
	hstub	unused2,mips16

	# For symbols called by a .call stub in another file.
	hstub	unused3,mips16

	# For symbols called by a .call.fp stub in another file.
	hstub	unused4,mips16


	# For symbols called by a .call stub in this file.
	lstub	used1,nomips16

	# For symbols called by a .call.fp stub in this file.
	lstub	used2,nomips16

	# For symbols called by a .call stub in this file.
	hstub	used3,nomips16

	# For symbols called by a .call.fp stub in this file.
	hstub	used4,nomips16

	# For symbols called by a .call stub in another file.
	hstub	used5,nomips16

	# For symbols called by a .call.fp stub in another file.
	hstub	used6,nomips16

	# For symbols called by a .call stub in this file.
	gstub	used7,nomips16

	# For symbols called by a .call.fp stub in this file.
	gstub	used8,nomips16

	# For symbols called by a .call stub in another file.
	gstub	used9,nomips16

	# For symbols called by a .call.fp stub in another file.
	gstub	used10,nomips16

	call_stub unused1
	call_stub used1
	call_stub used3
	call_stub used7
	call_stub extern1

	call_fp_stub unused2
	call_fp_stub used2
	call_fp_stub used4
	call_fp_stub used8
	call_fp_stub extern2
