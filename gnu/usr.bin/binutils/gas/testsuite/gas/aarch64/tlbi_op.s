// Test file for AArch64 GAS -- TLB invalidation instructions.

	.macro tlbi_m op has_xt
	.ifc \has_xt, 1
	tlbi	\op, x7
	.else
	tlbi	\op
	.endif
	.endm

	# Test case for tlbi operations
	.text

	tlbi_m	IPAS2E1IS, 1
	tlbi_m	IPAS2LE1IS, 1
	tlbi_m	VMALLE1IS , 0
	tlbi_m	ALLE2IS, 0
	tlbi_m	ALLE3IS, 0
	tlbi_m	VAE1IS, 1
	tlbi_m	VAE2IS, 1
	tlbi_m	VAE3IS, 1
	tlbi_m	ASIDE1IS, 1
	tlbi_m	VAAE1IS, 1
	tlbi_m	ALLE1IS, 0
	tlbi_m	VALE1IS, 1
	tlbi_m	VALE2IS, 1
	tlbi_m	VALE3IS, 1
	tlbi_m	VMALLS12E1IS, 0
	tlbi_m	VAALE1IS, 1
	tlbi_m	IPAS2E1, 1
	tlbi_m	IPAS2LE1, 1
	tlbi_m	VMALLE1 , 0
	tlbi_m	ALLE2 , 0
	tlbi_m	ALLE3, 0
	tlbi_m	VAE1, 1
	tlbi_m	VAE2, 1
	tlbi_m	VAE3, 1
	tlbi_m	ASIDE1, 1
	tlbi_m	VAAE1, 1
	tlbi_m	ALLE1, 0
	tlbi_m	VALE1, 1
	tlbi_m	VALE2, 1
	tlbi_m	VALE3, 1
	tlbi_m	VMALLS12E1, 0
	tlbi_m	VAALE1, 1
