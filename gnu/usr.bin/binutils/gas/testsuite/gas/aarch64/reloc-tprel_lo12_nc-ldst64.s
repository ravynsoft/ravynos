// Test file for AArch64 GAS -- tprel_lo12_nc for LDST64

func:
	// BFD_RELOC_AARCH64_TLSLE_LDST64_TPREL_LO12_NC
	ldr  x27, [x4, #:tprel_lo12_nc:sym]

