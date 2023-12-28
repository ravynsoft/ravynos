	.text
	.org 0
;;; Rotates and shifts

	rlc a
	rlc b
	rlc c
	rlc d
	rlc e
	rlc h
	rlc l
	rlc (hl)
	rlc (ix+5)
	rlc (iy+5)
	rrc a
	rrc b
	rrc c
	rrc d
	rrc e
	rrc h
	rrc l
	rrc (hl)
	rrc (ix+5)
	rrc (iy+5)
	rl a
	rl b
	rl c
	rl d
	rl e
	rl h
	rl l
	rl (hl)
	rl (ix+5)
	rl (iy+5)
	rr a
	rr b
	rr c
	rr d
	rr e
	rr h
	rr l
	rr (hl)
	rr (ix+5)
	rr (iy+5)
	sla a
	sla b
	sla c
	sla d
	sla e
	sla h
	sla l
	sla (hl)
	sla (ix+5)
	sla (iy+5)
	sra a
	sra b
	sra c
	sra d
	sra e
	sra h
	sra l
	sra (hl)
	sra (ix+5)
	sra (iy+5)
	srl a
	srl b
	srl c
	srl d
	srl e
	srl h
	srl l
	srl (hl)
	srl (ix+5)
	srl (iy+5)

	rlca
	rrca
	rla
	rra
	rld
	rrd
