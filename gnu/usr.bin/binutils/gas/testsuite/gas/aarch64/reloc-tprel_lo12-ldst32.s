// Test file for AArch64 GAS -- tprel_lo12 for LDST32

func:
	// BFD_RELOC_AARCH64_TLSLE_LDST32_TPREL_LO12
	ldrsw  x27, [x4, #:tprel_lo12:sym]

