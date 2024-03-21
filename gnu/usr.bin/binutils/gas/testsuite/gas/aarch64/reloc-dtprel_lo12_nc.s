// Test file for AArch64 GAS -- dtprel_lo12_nc

func:
	// BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12_NC
	add  x7, x26, #:dtprel_lo12_nc:x
