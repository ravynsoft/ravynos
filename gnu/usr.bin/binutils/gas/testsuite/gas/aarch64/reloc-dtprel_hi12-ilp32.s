// Test file for AArch64 GAS -- dtprel_hi12 ILP32

func:
	// BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_HI12
	add  w1, w26, #:dtprel_hi12:x
