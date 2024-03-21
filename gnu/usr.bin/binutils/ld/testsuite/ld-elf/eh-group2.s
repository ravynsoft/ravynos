	.section	sect, "axG", %progbits, sectgroup, comdat
	.cfi_startproc
# Test intention is that LSDA must be provided by the discarded FDE.
# DW_EH_PE_udata8 = 4
# DW_EH_PE_udata4 = 3
	.ifdef		ELF64
	.cfi_lsda 4, lsda
	.else
	.cfi_lsda 3, lsda
	.endif
	.skip 16
	.cfi_endproc

	.section	.gcc_except_table, "a", %progbits
lsda:
