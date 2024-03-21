# MPX instructions
	.allow_index_reg
	.text
start:
	### bndmk
	bndmk (%r11), %bnd1
	bndmk (%rax), %bnd1
	bndmk (0x399), %bnd1
	bndmk 0x3(%r9), %bnd1
	bndmk 0x3(%rax), %bnd1
	bndmk 0x3(,%r12,1), %bnd1
	bndmk (%rax,%rcx), %bnd1
	bndmk 0x3(%r11,%rax,2), %bnd1
	bndmk 0x3(%rbx,%r9,1), %bnd1

	### bndmov
	bndmov (%r11), %bnd1
	bndmov (%rax), %bnd1
	bndmov (0x399), %bnd1
	bndmov 0x3(%r9), %bnd2
	bndmov 0x3(%rax), %bnd2
	bndmov 0x3333(%rip), %bnd2
	bndmov 0x3(,%r12,1), %bnd0
	bndmov (%rax,%rdx), %bnd2
	bndmov 0x3(%r11,%rax,2), %bnd1
	bndmov 0x3(%rbx,%r9,1), %bnd1
	bndmov %bnd2, %bnd0

	bndmov %bnd1, (%r11)
	bndmov %bnd1, (%rax)
	bndmov %bnd1, (0x399)
	bndmov %bnd2, 0x3(%r9)
	bndmov %bnd2, 0x3(%rax)
	bndmov %bnd2, 0x3333(%rip)
	bndmov %bnd0, 0x3(,%r12,1)
	bndmov %bnd2, (%rax,%rdx)
	bndmov %bnd1, 0x3(%r11,%rax,2)
	bndmov %bnd1, 0x3(%rbx,%r9,1)
	bndmov %bnd0, %bnd2

	### bndcl
	bndcl (%r11), %bnd1
	bndcl (%rax), %bnd1
	bndcl %r11, %bnd1
	bndcl %rcx, %bnd1
	bndcl (0x399), %bnd1
	bndcl 0x3(%r9), %bnd2
	bndcl 0x3(%rax), %bnd2
	bndcl 0x3333(%rip), %bnd2
	bndcl 0x3(,%r12,1), %bnd0
	bndcl (%rax,%rdx), %bnd2
	bndcl 0x3(%r11,%rax,2), %bnd1
	bndcl 0x3(%rbx,%r9,1), %bnd1

	### bndcu
	bndcu (%r11), %bnd1
	bndcu (%rax), %bnd1
	bndcu %r11, %bnd1
	bndcu %rcx, %bnd1
	bndcu (0x399), %bnd1
	bndcu 0x3(%r9), %bnd2
	bndcu 0x3(%rax), %bnd2
	bndcu 0x3333(%rip), %bnd2
	bndcu 0x3(,%r12,1), %bnd0
	bndcu (%rax,%rdx), %bnd2
	bndcu 0x3(%r11,%rax,2), %bnd1
	bndcu 0x3(%rbx,%r9,1), %bnd1

	### bndcn
	bndcn (%r11), %bnd1
	bndcn (%rax), %bnd1
	bndcn %r11, %bnd1
	bndcn %rcx, %bnd1
	bndcn (0x399), %bnd1
	bndcn 0x3(%r9), %bnd2
	bndcn 0x3(%rax), %bnd2
	bndcn 0x3333(%rip), %bnd2
	bndcn 0x3(,%r12,1), %bnd0
	bndcn (%rax,%rdx), %bnd2
	bndcn 0x3(%r11,%rax,2), %bnd1
	bndcn 0x3(%rbx,%r9,1), %bnd1

	### bndstx
	bndstx %bnd0, 0x3(%rax,%rbx,1)
	bndstx %bnd2, 3(%rbx,%rdx)
	bndstx %bnd3, 0x399(%r12)
	bndstx %bnd1, 0x1234(%r11)
	bndstx %bnd2, 0x1234(%rbx)
	bndstx %bnd2, 3(,%rbx,1)
	bndstx %bnd2, 3(,%r12,1)
	bndstx %bnd1, (%rdx)

	### bndldx
	bndldx 0x3(%rax,%rbx,1), %bnd0
	bndldx 3(%rbx,%rdx), %bnd2
	bndldx 0x399(%r12), %bnd3
	bndldx 0x1234(%r11), %bnd1
	bndldx 0x1234(%rbx), %bnd2
	bndldx 3(,%rbx,1), %bnd2
	bndldx 3(,%r12,1), %bnd2
	bndldx (%rdx), %bnd1

	### bnd
	bnd call	foo
	bnd call	*(%rax)
	bnd call	*(%r11)
	bnd je	foo
	bnd jmp	foo
	bnd jmp	*(%rcx)
	bnd jmp	*(%r12)
	bnd ret

.intel_syntax noprefix
	bndmk bnd1, [r11]
	bndmk bnd1, [rax]
	bndmk bnd1, [0x399]
	bndmk bnd1, [r9+0x3]
	bndmk bnd1, [rax+0x3]
	bndmk bnd1, [1*r12+0x3]
	bndmk bnd1, [rax+rcx]
	bndmk bnd1, [r11+1*rax+0x3]
	bndmk bnd1, [rbx+1*r9+0x3]

	### bndmov
	bndmov bnd1, [r11]
	bndmov bnd1, [rax]
	bndmov bnd1, [0x399]
	bndmov bnd2, [r9+0x3]
	bndmov bnd2, [rax+0x3]
	bndmov bnd0, [1*r12+0x3]
	bndmov bnd2, [rax+rdx]
	bndmov bnd1, [r11+1*rax+0x3]
	bndmov bnd1, [rbx+1*r9+0x3]
	bndmov bnd0, bnd2

	bndmov [r11], bnd1
	bndmov [rax], bnd1
	bndmov [0x399], bnd1
	bndmov [r9+0x3], bnd2
	bndmov [rax+0x3], bnd2
	bndmov [1*r12+0x3], bnd0
	bndmov [rax+rdx], bnd2
	bndmov [r11+1*rax+0x3], bnd1
	bndmov [rbx+1*r9+0x3], bnd1
	bndmov bnd2, bnd0

	### bndcl
	bndcl bnd1, [r11]
	bndcl bnd1, [rax]
	bndcl bnd1, r11
	bndcl bnd1, rcx
	bndcl bnd1, [0x399]
	bndcl bnd1, [r9+0x3]
	bndcl bnd1, [rax+0x3]
	bndcl bnd1, [1*r12+0x3]
	bndcl bnd1, [rax+rcx]
	bndcl bnd1, [r11+1*rax+0x3]
	bndcl bnd1, [rbx+1*r9+0x3]

	### bndcu
	bndcu bnd1, [r11]
	bndcu bnd1, [rax]
	bndcu bnd1, r11
	bndcu bnd1, rcx
	bndcu bnd1, [0x399]
	bndcu bnd1, [r9+0x3]
	bndcu bnd1, [rax+0x3]
	bndcu bnd1, [1*r12+0x3]
	bndcu bnd1, [rax+rcx]
	bndcu bnd1, [r11+1*rax+0x3]
	bndcu bnd1, [rbx+1*r9+0x3]

	### bndcn
	bndcn bnd1, [r11]
	bndcn bnd1, [rax]
	bndcn bnd1, r11
	bndcn bnd1, rcx
	bndcn bnd1, [0x399]
	bndcn bnd1, [r9+0x3]
	bndcn bnd1, [rax+0x3]
	bndcn bnd1, [1*r9+0x3]
	bndcn bnd1, [rax+rcx]
	bndcn bnd1, [r11+1*rax+0x3]
	bndcn bnd1, [rbx+1*r9+0x3]

	### bndstx
	bndstx [rax+rbx*1+0x3], bnd0
	bndstx [rbx+rdx+3], bnd2
	bndstx [r12+0x399], bnd3
	bndstx [r11+0x1234], bnd1
	bndstx [rbx+0x1234], bnd2
	bndstx [1*rbx+3], bnd2
	bndstx [1*r12+3], bnd2
	bndstx [rdx], bnd1

	### bndldx
	bndldx bnd0, [rax+rbx*1+0x3]
	bndldx bnd2, [rbx+rdx+3]
	bndldx bnd3, [r12+0x399]
	bndldx bnd1, [r11+0x1234]
	bndldx bnd2, [rbx+0x1234]
	bndldx bnd2, [1*rbx+3]
	bndldx bnd2, [1*r12+3]
	bndldx bnd1, [rdx]

	### bnd
	bnd call	foo
	bnd call	rax
	bnd call	r11
	bnd je	foo
	bnd jmp	foo
	bnd jmp	rcx
	bnd jmp	r12
	bnd ret

foo:	bnd ret

	.att_syntax prefix
bad:
	# bndldx (%rax),(bad)
	.insn 0x0f1a, (%rax), %esi

	# bndmov (bad),%bnd0
	.insn 0x660f1a, %esp, %bnd0

	# bndmov with REX.B set
	.insn 0x660f1a, %r8d, %bnd0

	# bndmov with REX.R set
	.insn 0x660f1a, %bnd0, %r8d

	# bndmk (bad),%bnd0
	.insn 0xf30f1b, -0x6f6f6f70(%rip), %bnd0
