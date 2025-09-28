	.text

	e_stw    r12, 0x4C(r1)
	e_stw    r11, 0x48(r1)
	e_stw    r10, 0x44(r1)
	e_stw    r9,  0x40(r1)
	e_stw    r8,  0x3C(r1)
	e_stw    r7,  0x38(r1)
	e_stw    r6,  0x34(r1)
	e_stw    r5,  0x30(r1)
	e_stw    r4,  0x2c(r1)

        .globl   IV_table
	.section ".iv_handlers", "ax"
IV_table:
	e_b dummy
	.align 4
	e_b dummy
	.align 4
	e_b dummy
	.align 4
	e_b dummy
	.align 4
	e_b dummy
	.align 4
	e_b dummy
	.align 4
	e_b dummy
	.align 4
	e_b dummy
	.align 4
dummy:
	se_nop
	e_b dummy

	.section ".text_iv", "ax"
	e_lis r3, IV_table@h
	mtivpr r3
	e_li r3, IV_table@l+0x00
	mtivor0 r3
	e_li r3, IV_table@l+0x10
	mtivor1 r3
	e_li r3, IV_table@l+0x20
	mtivor2 r3

	.data
	.long 0xdeadbeef
