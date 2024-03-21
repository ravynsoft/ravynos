// Test file for AArch64 GAS -- tlsdesc_off_g1

func:
	// BFD_RELOC_AARCH64_TLSDESC_OFF_G1
	movz    x28, #:tlsdesc_off_g1:x
