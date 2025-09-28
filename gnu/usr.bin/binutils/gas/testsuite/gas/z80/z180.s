	.text
	.org	0
	;; Zylog Z180 instructions

; IN0 group
	in0 a,(0x5)
	in0 b,(0x5)
	in0 c,(0x5)
	in0 d,(0x5)
	in0 e,(0x5)
	in0 h,(0x5)
	in0 l,(0x5)

; OUT0 group
	out0 (0x5),a
	out0 (0x5),b
	out0 (0x5),c
	out0 (0x5),d
	out0 (0x5),e
	out0 (0x5),h
	out0 (0x5),l

; MLT group
	mlt bc
	mlt de
	mlt hl
	mlt sp

; TST group
	tst a
	tst b
	tst c
	tst d
	tst e
	tst h
	tst l
	tst (hl)
	tst 0fh

; TSTIO instruction
	tstio   0f0h

; SLP instruction
	slp

; Additional block I/O instructions
	OTIM
	OTDM
	OTIMR
	OTDMR
