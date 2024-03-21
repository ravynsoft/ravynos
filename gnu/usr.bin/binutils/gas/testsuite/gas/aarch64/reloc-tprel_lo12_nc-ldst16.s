// Test file for AArch64 GAS -- tprel_lo12_nc for LDST16

func:
	// BFD_RELOC_AARCH64_TLSLE_LDST16_TPREL_LO12_NC
	ldrh  w27, [x4, #:tprel_lo12_nc:sym]
