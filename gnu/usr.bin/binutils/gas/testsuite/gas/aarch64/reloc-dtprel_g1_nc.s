// Test file for AArch64 GAS -- dtprel_g1_nc

func:
	// BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1_NC
	movk x9, #:dtprel_g1_nc:x
