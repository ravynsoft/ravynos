;;; Test if the assembler generates correctly all SDA relocations.

	.cpu HS
	.text
	;; BFD_RELOC_ARC_SDA_LDST (s9 range)
	ldd	r0,[gp,@b@sda]
	std	r2,[gp,@b@sda]

	;; BFD_RELOC_ARC_SDA_LDST2 (s11 range)
	ldd.as	r0,[gp,@b@sda]
	std.as	r2,[gp,@b@sda]

	;; BFD_RELOC_ARC_SDA16_ST2 (s11 range)
	ld_s	r1,[gp,@b@sda]
	st_s	r0,[gp,@b@sda]
