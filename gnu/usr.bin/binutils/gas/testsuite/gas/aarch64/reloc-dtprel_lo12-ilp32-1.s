// Test file for AArch64 GAS -- dtprel_lo12 ILP32

func:
	// BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12
	add  w8, w21, #:dtprel_lo12:x
