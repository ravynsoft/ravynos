	.text
	.globl _start
_start:
# original PCREL_OPT definition, with second insn immediately after first
	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0
	lbz 3,0(9)

# but we now allow an offset to the second insn
	pld 22,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
	nop
0:	lhz 4,0(22)

# in fact, it can even be before the "first" insn
0: 	lha 3,0(9)
	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0b-(.-8)
	bne 0b

# and of course, other local labels work
	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,.L1-(.-8)
.L1:	lwz 3,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	lwa 3,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	ld 3,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	lq 14,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	lfs 1,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	lfd 1,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	lxsd 30,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	lxssp 31,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	lxv 63,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	lxv 0,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	stb 3,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	sth 3,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	stw 3,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	std 3,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	stq 14,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	stfd 1,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	stfs 2,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	stxsd 30,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	stxssp 31,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	stxv 63,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	stxv 0,0(9)

#offsets are allowed too
	pld 9,sym@got@pcrel
0:
	lbz 3,0x1234(9)
	.reloc 0b-8,R_PPC64_PCREL_OPT,(.-4)-(0b-8)

#and prefix insns as the second insn
	pld 9,sym@got@pcrel
0:
	plq 4,0x12345678(9)
	.reloc 0b-8,R_PPC64_PCREL_OPT,(.-8)-(0b-8)

# This should not optimize
	.extern i
	.type i,@object
	pld 9,i@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	ld 0,0(9)

# and this should edit from GOT indirect to GOT relative
# ie. change the pld to paddi, leaving the lbz as is.
	pld 7,sym@got@pcrel
	lbz 6,0(7)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	lxvp 62,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	lxvp 0,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	stxvp 62,0(9)

	pld 9,sym@got@pcrel
		.reloc .-8,R_PPC64_PCREL_OPT,0f-(.-8)
0:	stxvp 0,0(9)

	.data
sym:	.space 32
