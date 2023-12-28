target:
	# zicsr
	csrr	t0, ustatus
	csrwi	ustatus, 0x0
	csrsi	ustatus, 0x0
	csrci	ustatus, 0x0
	csrw	ustatus, t0
	csrw	ustatus, 0x0
	csrs	ustatus, t0
	csrs	ustatus, 0x0
	csrc	ustatus, t0
	csrc	ustatus, 0x0
	csrrwi	t0, ustatus, 0x0
	csrrsi	t0, ustatus, 0x0
	csrrci	t0, ustatus, 0x0
	csrrw	t0, ustatus, t0
	csrrw	t0, ustatus, 0x0
	csrrs	t0, ustatus, t0
	csrrs	t0, ustatus, 0x0
	csrrc	t0, ustatus, t0
	csrrc	t0, ustatus, 0x0

	# zifencei
	fence.i
