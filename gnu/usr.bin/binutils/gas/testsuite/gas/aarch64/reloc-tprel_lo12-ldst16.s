// Test file for AArch64 GAS -- tprel_lo12 for LDST16

func:
	// BFD_RELOC_AARCH64_TLSLE_LDST16_TPREL_LO12
	ldrsh  x27, [x4, #:tprel_lo12:sym]

