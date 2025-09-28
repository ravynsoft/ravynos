;;; Test if the assembler generates correctly all SDA relocations.

	.cpu ARC700
	.text
	;; BFD_RELOC_ARC_SDA16_LD2 (s11 range)
	add_s	r0,gp,@a@sda
	ld_s	r0,[gp,@a@sda]

	;; BFD_RELOC_ARC_SDA16_LD1 (s10 range)
	ldw_s	r0,[gp,@a@sda]

	;; BFD_RELOC_ARC_SDA16_LD (s9 range)
	ldb_s	r0,[gp,@a@sda]

	;; BFD_RELOC_ARC_SDA_LDST2 (s11 range)
	ld.as	r12,[gp,@a@sda]
	st.as	r14,[gp,@a@sda]

	;; BFD_RELOC_ARC_SDA_LDST (s9 range)
	ld	r10,[gp,@a@sda]
	st	r9,[gp,@a@sda]
	ldw	r8,[gp,@a@sda]
	stw	r7,[gp,@a@sda]
	ldb	r6,[gp,@a@sda]
	stb	r5,[gp,@a@sda]

	;; BFD_RELOC_ARC_SDA_LDST1 (s10 range)
	ldw.as	r8,[gp,@a@sda]
	stw.as	r7,[gp,@a@sda]

	;; Undefined behavior. However it should be something like: LDST
	ldb.as	r8,[gp,@a@sda]
	stb.as	r7,[gp,@a@sda]

	;; BFD_RELOC_ARC_SDA_ME
	add	r1,gp,@a@sda
