// Test file for AArch64 GAS -- gotoff_g1

func:
	// BFD_RELOC_AARCH64_MOVW_GOTOFF_G1
	movz    x28, #:gotoff_g1:x
