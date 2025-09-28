// Test file for AArch64 GAS -- gotoff_g0_nc

func:
	// BFD_RELOC_AARCH64_MOVW_GOTOFF_G0_NC
	movk    x28, #:gotoff_g0_nc:x
