// Test file for AArch64 GAS -- dtprel_g1

func:
	// BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1
	movz x9, #:dtprel_g1:x
