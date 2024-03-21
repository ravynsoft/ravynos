// Test file for AArch64 GAS -- tlsdesc_off_g0_nc

func:
	// BFD_RELOC_AARCH64_TLSDESC_OFF_G0_NC
	movk    x28, #:tlsdesc_off_g0_nc:x
