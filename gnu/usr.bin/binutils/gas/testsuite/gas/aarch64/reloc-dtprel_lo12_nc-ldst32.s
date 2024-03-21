// Test file for AArch64 GAS -- dtprel_lo12_nc for LDST32

func:
	// BFD_RELOC_AARCH64_TLSLD_LDST32_DTPREL_LO12_NC
	ldrsw  x20, [x7, #:dtprel_lo12_nc:sym]

