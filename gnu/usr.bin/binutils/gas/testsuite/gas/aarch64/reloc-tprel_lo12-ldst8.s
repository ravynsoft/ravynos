// Test file for AArch64 GAS -- tprel_lo12 for LDST8

func:
	// BFD_RELOC_AARCH64_TLSLE_LDST8_TPREL_LO12
	ldrsb  x21, [x8, #:tprel_lo12:sym]

