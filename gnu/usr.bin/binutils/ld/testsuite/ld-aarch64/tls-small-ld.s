// Test file for AArch64 LD -- reloc 518

	.text
func:
	// BFD_RELOC_AARCH64_TLSLD_ADD_LO12_NC
	add	x1, x2, #:tlsldm_lo12_nc:dummy
	// BFD_RELOC_AARCH64_TLSLD_ADR_PAGE21
	adrp	x0, :tlsldm:dummy
