// Test file for AArch64 GAS -- dtprel_lo12 for LDST8

func:
	// BFD_RELOC_AARCH64_TLSLD_LDST8_DTPREL_LO12
	ldrsb  x21, [x8, #:dtprel_lo12:sym]

