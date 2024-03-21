// Test file for AArch64 GAS -- dtprel_g2

func:
	// BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G2
	movz x10, #:dtprel_g2:x
