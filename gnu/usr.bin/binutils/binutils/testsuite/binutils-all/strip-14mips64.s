	.text
foo:
	.dc.l    0x12345678

	.section .rela.text,""
	.dc.a	 0
	.dc.l    0x000ffff0
	.dc.b    0, 0, 0, RELOC
	.dc.a	 0x00000001

	.dc.a	 0
	.dc.l    0
	.dc.b    0, 0, 0, 0
	.dc.a	 0
