	.text
foo:
	.dc.l    0x12345678

	.section .rel.text,""
	.ifdef	 ELF64

	.dc.a	 0
	.dc.a    0x000ffff000000000 + RELOC

	.dc.a	 0
	.dc.a    0
	.else

	# Some targets, such as `m68hc11-*', use 16-bit addresses.
	# With them `.dc.a' emits 16-bit quantities, so we need to use
	# `.dc.l' for 32-bit relocation data.
	.dc.l	 0
	.dc.l	 0x0ffff000 + RELOC

	.dc.l	 0
	.dc.l    0
	.endif
