// Test file for AArch64 GAS -- tprel_lo12_nc for LDST32

func:
	// BFD_RELOC_AARCH64_TLSLE_LDST32_TPREL_LO12_NC
	ldrsw  x20, [x7, #:tprel_lo12_nc:sym]

