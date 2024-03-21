// Test file for AArch64 GAS -- dtprel_hi12

func:
	// BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_HI12
	add  x1, x26, #:dtprel_hi12:x
