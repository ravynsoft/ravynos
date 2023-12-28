;;; Test if the assembler correctly generates TLS relocations

	.cpu HS
	.text
	;; RELOC_ARC_TLS_IE_GOT
	ld	r2,[pcl,@var@tlsie]

	;; RELOC_ARC_TLS_GD_GOT
	add	r0,pcl,@var@tlsgd

	;; RELOC_ARC_TLS_DTPOFF
	add	r1,r0,@var@dtpoff

	;; RELOC_ARC_TLS_LE_32
	add 	r0,r25,@var@tpoff

	;; RELOC_ARC_TLS_GD_LD
	.tls_gd_ld @.tdata`bl @func@plt
