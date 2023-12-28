// Test file for AArch64 GAS -- tlsldm page ILP32

func:
	add	w1, w2, w3
	// BFD_RELOC_AARCH64_TLSLD_ADR_PAGE21
	adrp	x0, :tlsldm:dummy
