// Test file for AArch64 GAS -- dtprel_lo12

func:
	// BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12
	add  x7, x26, #:dtprel_lo12:x
