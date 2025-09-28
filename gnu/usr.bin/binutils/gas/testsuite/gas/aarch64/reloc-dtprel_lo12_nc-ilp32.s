// Test file for AArch64 GAS -- dtprel_lo12_nc	ILP32

func:
	// BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12_NC
	add  w8, w21, #:dtprel_lo12_nc:x
