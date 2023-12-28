// Test file for AArch64 GAS -- dtprel_g0 ILP32

func:
	// BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0
	movz w9, #:dtprel_g0:x
