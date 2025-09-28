// Test file for AArch64 GAS -- dtprel_lo12_nc for LDST8

func:
	// BFD_RELOC_AARCH64_TLSLD_LDST8_DTPREL_LO12_NC
	ldrb  w29, [x2, #:dtprel_lo12_nc:sym]

