	.text
	.globl	glb_proc
	.globl	data8
	.globl	data16
	.globl	data24
	.globl	data32
.L_proc:
	ld	(hl),.L_proc >> 0
	inc	hl
	ld	(hl),.L_proc >> 8
	inc	hl
	ld	(hl),(glb_proc) & 0xff
	inc	hl
	ld	(hl),glb_proc >> 8
	inc	hl
	ld	(hl),glb_proc >> 16
	inc	hl
	ld	(hl),glb_proc >> 24
	ld	bc,.L_label
	ld	de,glb_proc >> 0
	ld	hl,glb_proc >> 16
	djnz	start
	ld	a,data8
	ret
.L_label:
	.db	data8
	.dw	data16
	.d24	data24
	.d32	data32
	.db	data16 & 0xff
	.db	data16 >> 8
	.dw	data32 & 0xffff
	.dw	data32 >> 16
	.dw	data32 >> 24
	.dw	data24 >> 8
	.dw	data32 >> 8
	.end
