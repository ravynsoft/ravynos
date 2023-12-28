// Test file for AArch64 GAS -- dtprel_g0_nc

func:
	// BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0_NC
	movk x16, #:dtprel_g0_nc:x
