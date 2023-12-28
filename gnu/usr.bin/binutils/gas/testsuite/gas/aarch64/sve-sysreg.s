	mrs	x0, ID_AA64ZFR0_EL1
	mrs	X27, id_aa64zfr0_el1

	mrs	x0, ZCR_EL1
	mrs	X27, zcr_el1
	msr	ZCR_EL1, X0
	msr	zcr_el1, x26

	mrs	x0, ZCR_EL12
	mrs	X27, zcr_el12
	msr	ZCR_EL12, X0
	msr	zcr_el12, x26

	mrs	x0, ZCR_EL2
	mrs	X27, zcr_el2
	msr	ZCR_EL2, X0
	msr	zcr_el2, x26

	mrs	x0, ZCR_EL3
	mrs	X27, zcr_el3
	msr	ZCR_EL3, X0
	msr	zcr_el3, x26
