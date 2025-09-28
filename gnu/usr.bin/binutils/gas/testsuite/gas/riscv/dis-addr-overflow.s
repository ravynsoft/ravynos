.set __global_pointer$, 0x00000200

.ifdef rv64
topbase = 0xffffffff00000000
.else
topbase = 0
.endif

.set addr_load,              topbase + 0xffffeffc  # -0x1000 -4
.set addr_store,             topbase + 0xffffdff8  # -0x2000 -8
.set addr_jalr_1,            topbase + 0xffffd000  # -0x3000
.set addr_jalr_2,            topbase + 0xffffbff4  # -0x4000 -12
.set addr_jalr_3,            topbase + 0xffffb000  # -0x5000
.set addr_loadaddr,          topbase + 0xffff9ff0  # -0x6000 -16
.set addr_loadaddr_c,        topbase + 0xffff8fec  # -0x7000 -20
.set addr_loadaddr_w,        topbase + 0xffff7fe8  # -0x8000 -24
.set addr_loadaddr_w_c,      topbase + 0xffff6fe4  # -0x9000 -28
.set addr_rel_gp_pos,                  0x00000600  # __global_pointer$ + 0x400
.set addr_rel_gp_neg,        topbase + 0xfffffe00  # __global_pointer$ - 0x400
.set addr_rel_zero_pos,                0x00000100
.set addr_rel_zero_neg,      topbase + 0xfffff800  # -0x800
.set addr_jalr_rel_zero_pos,           0x00000104
.set addr_jalr_rel_zero_neg, topbase + 0xfffff804  # -0x7fc

target:
	.option	push
	.option	arch, -c
	## Use hi_addr
	# Load
	lui	t0, 0xfffff
	lw	s2, -4(t0)
	# Store
	lui	t1, 0xffffe
	sw	s3, -8(t1)
	# JALR (implicit destination, no offset)
	lui	t2, 0xffffd
	jalr	t2
	# JALR (implicit destination, with offset)
	lui	t3, 0xffffc
	jalr	-12(t3)
	# JALR (explicit destination, no offset)
	lui	t4, 0xffffb
	jalr	s4, t4
	# ADDI (not compressed)
	lui	t5, 0xffffa
	addi	s5, t5, -16
	# C.ADDI
	lui	t6, 0xffff9
	.option	pop
	c.addi	t6, -20
.ifdef rv64
	.option	push
	.option	arch, -c
	# ADDIW (not compressed)
	lui	s6, 0xffff8
	addiw	s7, s6, -24
	# C.ADDIW
	lui	s8, 0xffff7
	.option	pop
	c.addiw	s8, -28
.endif

	# Use addresses relative to gp
	lw	t0, 0x400(gp)
	lw	t1, -0x400(gp)
	# Use addresses relative to zero
	lw	t2, 0x100(zero)
	lw	t3, -0x800(zero)
	jalr	t4, 0x104(zero)
	jalr	t5, -0x7fc(zero)
