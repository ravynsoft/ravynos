.ifdef rv64
topbase = 0xffffffff00000000
.else
topbase = 0
.endif

.set __global_pointer$, topbase + 0xffffffff  # -1
.set addr_rel_gp_pos,             0x00000004  # +4
.set addr_rel_gp_neg,   topbase + 0xfffffffc  # -4

target:
	# Use addresses relative to gp
	# (gp is the highest address)
	lw	t0, +5(gp)
	lw	t1, -3(gp)
