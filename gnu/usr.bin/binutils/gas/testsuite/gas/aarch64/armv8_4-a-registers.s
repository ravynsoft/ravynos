	.macro gen_mrs reg
	.irp m, 3, 11, 15
		MRS X\m, \reg
	.endr
	.endm

	.macro gen_tlbi reg
	.irp m, 3, 11, 15
		TLBI \reg, X\m
	.endr
	.endm
func:
	# Secure second stage
	gen_mrs VSTTBR_EL2
	gen_mrs VSTCR_EL2

	# Timer changes
	gen_mrs CNTP_TVAL_EL0
	gen_mrs CNTP_CTL_EL0
	gen_mrs CNTP_CVAL_EL0
	gen_mrs CNTV_TVAL_EL0
	gen_mrs CNTV_CTL_EL0
	gen_mrs CNTV_CVAL_EL0

	gen_mrs CNTHVS_TVAL_EL2
	gen_mrs CNTHVS_CVAL_EL2
	gen_mrs CNTHVS_CTL_EL2
	gen_mrs CNTHPS_TVAL_EL2
	gen_mrs CNTHPS_CVAL_EL2
	gen_mrs CNTHPS_CTL_EL2

	# Debug state
	gen_mrs SDER32_EL2

	# Nested Virtualization
	gen_mrs VNCR_EL2

	# PSTATE
	MSR DIT, #01
	MSR DIT, #00
	MSR DIT, X3
	MSR DIT, X11
	MSR DIT, X15
	gen_mrs DIT

	# TLB Maintenance instructions
	TLBI VMALLE1OS
	TLBI ALLE2OS
	TLBI ALLE1OS
	TLBI ALLE3OS
	TLBI VMALLS12E1OS
	gen_tlbi VAE1OS
	gen_tlbi ASIDE1OS
	gen_tlbi VAAE1OS
	gen_tlbi VALE1OS
	gen_tlbi VAALE1OS
	gen_tlbi IPAS2E1OS
	gen_tlbi IPAS2LE1OS
	gen_tlbi VAE2OS
	gen_tlbi VALE2OS
	gen_tlbi VAE3OS
	gen_tlbi VALE3OS

	# TLB Range Maintenance Instructions
	gen_tlbi RVAE1
	gen_tlbi RVAAE1
	gen_tlbi RVALE1
	gen_tlbi RVAALE1
	gen_tlbi RVAE1IS
	gen_tlbi RVAAE1IS
	gen_tlbi RVALE1IS
	gen_tlbi RVAALE1IS
	gen_tlbi RVAE1OS
	gen_tlbi RVAAE1OS
	gen_tlbi RVALE1OS
	gen_tlbi RVAALE1OS
	gen_tlbi RIPAS2E1IS
	gen_tlbi RIPAS2LE1IS
	gen_tlbi RIPAS2E1
	gen_tlbi RIPAS2LE1
	gen_tlbi RIPAS2E1OS
	gen_tlbi RIPAS2LE1OS
	gen_tlbi RVAE2
	gen_tlbi RVALE2
	gen_tlbi RVAE2IS
	gen_tlbi RVALE2IS
	gen_tlbi RVAE2OS
	gen_tlbi RVALE2OS
	gen_tlbi RVAE3
	gen_tlbi RVALE3
	gen_tlbi RVAE3IS
	gen_tlbi RVALE3IS
	gen_tlbi RVAE3OS
	gen_tlbi RVALE3OS
