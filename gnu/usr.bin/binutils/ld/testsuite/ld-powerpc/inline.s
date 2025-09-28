	.text
	.globl _start
_start:
	std 2,24(1)
	.reloc .-4,R_PPC64_PLTSEQ,my_func
	addis 12,2,my_func@plt@ha  # .reloc .,R_PPC64_PLT16_HA,my_func
	ld 12,my_func@plt@l(12)    # .reloc .,R_PPC64_PLT16_LO_DS,my_func
	mtctr 12
	.reloc .-4,R_PPC64_PLTSEQ,my_func
	bctrl
	.reloc .-4,R_PPC64_PLTCALL,my_func
	ld 2,24(1)
