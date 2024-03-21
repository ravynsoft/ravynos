	.section	.text
	.align	4
;;; all the ops should have the same offset.
	ld_s	r0,[gp,@a@sda]
	;; BFD_RELOC_ARC_SDA16_LD2
	ldh_s	r0,[gp,@a@sda]
	;; BFD_RELOC_ARC_SDA16_LD1
	ldb_s	r0,[gp,@a@sda]
	;; BFD_RELOC_ARC_SDA16_LD
	ld.as	r0,[gp,@a@sda]
	st.as	r0,[gp,@a@sda]
	;; BFD_RELOC_ARC_SDA_LDST2
	ld	r0,[gp,@a@sda]
	ldb	r0,[gp,@a@sda]
	ldh	r0,[gp,@a@sda]
	;; 	ldd	r0,[gp,@a@sda]
	;; BFD_RELOC_ARC_SDA_LDST
	sth.as	r0,[gp,@a@sda]
	;; BFD_RELOC_ARC_SDA_LDST1
	ld_s	r1,[gp,@a@sda]
	st_s	r0,[gp,@a@sda]
	;; BFD_ARC_SDA16_ST2
	add	r2, gp, @a@sda
	;; BFD_ARC_SDA32_ME

	.global	a
	.section	.sbss,"aw",@nobits
	.align 4
	.type	a, @object
	.size	a, 4
a:
	.zero	4
