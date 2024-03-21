// Test file for AArch64 GAS -- dtprel_g0

func:
	// BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0
	movz x9, #:dtprel_g0:x
