// Test file for AArch64 GAS -- dtprel_g0_nc ILP32

func:
	// BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0_NC
	movk w16, #:dtprel_g0_nc:x
