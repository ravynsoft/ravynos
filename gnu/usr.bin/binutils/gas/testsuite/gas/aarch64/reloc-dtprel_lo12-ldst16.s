// Test file for AArch64 GAS -- dtprel_lo12 for LDST16

func:
	// BFD_RELOC_AARCH64_TLSLD_LDST16_DTPREL_LO12
	ldrsh  x27, [x4, #:dtprel_lo12:sym]

