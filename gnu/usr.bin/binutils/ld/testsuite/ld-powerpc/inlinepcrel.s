	.text
	.globl _start
_start:
	pld 12,0(0),1
	.reloc .-8,R_PPC64_PLT_PCREL34_NOTOC,my_func
	mtctr 12
	.reloc .-4,R_PPC64_PLTSEQ_NOTOC,my_func
	bctrl
	.reloc .-4,R_PPC64_PLTCALL_NOTOC,my_func
