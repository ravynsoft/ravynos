// Test file for AArch64 GAS -- tprel_lo12_nc for LDST8

func:
	// BFD_RELOC_AARCH64_TLSLE_LDST8_TPREL_LO12_NC
	ldrb  w29, [x2, #:tprel_lo12_nc:sym]

