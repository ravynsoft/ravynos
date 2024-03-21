// Test file for AArch64 LD -- tlsldm

	.text
func:
	add	x1, x2, x3
	// BFD_RELOC_AARCH64_TLSLD_ADR_PREL21
	adr	x0, :tlsldm:dummy
