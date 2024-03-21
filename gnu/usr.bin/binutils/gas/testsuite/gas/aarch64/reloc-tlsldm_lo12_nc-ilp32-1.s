// Test file for AArch64 GAS -- reloc 85

func:
	// BFD_RELOC_AARCH64_TLSLD_ADD_LO12_NC
	add  x0, x0, #:tlsldm_lo12_nc:x
