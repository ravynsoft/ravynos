	.section ".text_iv", "ax"
	e_lis r3, IV_table@h
	mtivpr r3
	e_li r3, IV_table@l+0x00
	mtivor0 r3
	e_li r3, IV_table@l+0x10
	mtivor1 r3
	e_li r3, IV_table@l+0x20
	mtivor2 r3
