	.text
foo:
	.dc.l    0x12345678

	.section .rela.text,""
 .ifdef ELF64
	.8byte	0
	.8byte	RELOC
	.8byte	0
 .else
	.4byte	0
	.4byte	RELOC
	.4byte	0
 .endif
