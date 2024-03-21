// Test file for AArch64 GAS -- dtprel_lo12 for LDST32

func:
	// BFD_RELOC_AARCH64_TLSLD_LDST32_DTPREL_LO12
	ldrsw  x27, [x4, #:dtprel_lo12:sym]

