	.text
foo:
	.dc.l    0x12345678

	.section .rel.text,""
 .ifdef ELF64
	.8byte	0
	.8byte	RELOC
 .else
	.4byte	0
	.4byte	RELOC
 .endif
