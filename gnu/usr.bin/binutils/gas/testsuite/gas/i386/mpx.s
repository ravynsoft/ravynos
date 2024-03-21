# MPX instructions
	.allow_index_reg
	.text
start:
	### bndmk
	bndmk (%eax), %bnd1
	bndmk (0x399), %bnd1
	bndmk 0x3(%edx), %bnd1
	bndmk (%eax,%ecx), %bnd1
	bndmk (,%ecx,1), %bnd1
	bndmk 0x3(%ecx,%eax,1), %bnd1

	### bndmov
	bndmov (%eax), %bnd1
	bndmov (0x399), %bnd1
	bndmov 0x3(%edx), %bnd2
	bndmov (%eax,%edx), %bnd2
	bndmov (,%eax,1), %bnd2
	bndmov 0x3(%ecx,%eax,1), %bnd1
	bndmov %bnd2, %bnd0

	bndmov %bnd1, (%eax)
	bndmov %bnd1, (0x399)
	bndmov %bnd2, 0x3(%edx)
	bndmov %bnd2, (%eax,%edx)
	bndmov %bnd2, (,%eax,1)
	bndmov %bnd1, 0x3(%ecx,%eax,1)
	bndmov %bnd0, %bnd2

	### bndcl
	bndcl (%ecx), %bnd1
	bndcl %ecx, %bnd1
	bndcl (0x399), %bnd1
	bndcl 0x3(%edx), %bnd1
	bndcl (%eax,%ecx), %bnd1
	bndcl (,%ecx,1), %bnd1
	bndcl 0x3(%ecx,%eax,1), %bnd1

	### bndcu
	bndcu (%ecx), %bnd1
	bndcu %ecx, %bnd1
	bndcu (0x399), %bnd1
	bndcu 0x3(%edx), %bnd1
	bndcu (%eax,%ecx), %bnd1
	bndcu (,%ecx,1), %bnd1
	bndcu 0x3(%ecx,%eax,1), %bnd1

	### bndcn
	bndcn (%ecx), %bnd1
	bndcn %ecx, %bnd1
	bndcn (0x399), %bnd1
	bndcn 0x3(%edx), %bnd1
	bndcn (%eax,%ecx), %bnd1
	bndcn (,%ecx,1), %bnd1
	bndcn 0x3(%ecx,%eax,1), %bnd1

	### bndstx
	bndstx %bnd0, 0x3(%eax,%ebx,1)
	bndstx %bnd2, 3(%ebx,%edx)
	bndstx %bnd2, 3(,%edx,1)
	bndstx %bnd3, 0x399(%edx)
	bndstx %bnd2, 0x1234(%ebx)
	bndstx %bnd2, 3(%ebx,1)
	bndstx %bnd1, (%edx)

	### bndldx
	bndldx 0x3(%eax,%ebx,1), %bnd0
	bndldx 3(%ebx,%edx), %bnd2
	bndldx 3(,%edx,1), %bnd2
	bndldx 0x399(%edx), %bnd3
	bndldx 0x1234(%ebx), %bnd2
	bndldx 3(%ebx,1), %bnd2
	bndldx (%edx), %bnd1

	### bnd
	bnd call	foo
	bnd call	*(%eax)
	bnd je	foo
	bnd jmp	foo
	bnd jmp	*(%ecx)
	bnd ret

.intel_syntax noprefix
	bndmk bnd1, [eax]
	bndmk bnd1, [0x399]
	bndmk bnd1, [ecx+0x3]
	bndmk bnd1, [eax+ecx]
	bndmk bnd1, [ecx*1]
	bndmk bnd1, [edx+1*eax+0x3]

	### bndmov
	bndmov bnd1, [eax]
	bndmov bnd1, [0x399]
	bndmov bnd1, [ecx+0x3]
	bndmov bnd1, [eax+ecx]
	bndmov bnd1, [ecx*1]
	bndmov bnd1, [edx+1*eax+0x3]
	bndmov bnd0, bnd1

	bndmov [eax], bnd1
	bndmov [0x399], bnd1
	bndmov [ecx+0x3], bnd1
	bndmov [eax+ecx], bnd1
	bndmov [ecx*1], bnd1
	bndmov [edx+1*eax+0x3], bnd1
	bndmov bnd1, bnd0

	### bndcl
	bndcl bnd1, [eax]
	bndcl bnd1, ecx
	bndcl bnd1, [0x399]
	bndcl bnd1, [ecx+0x3]
	bndcl bnd1, [eax+ecx]
	bndcl bnd1, [ecx*1]
	bndcl bnd1, [edx+1*eax+0x3]

	### bndcu
	bndcu bnd1, [eax]
	bndcu bnd1, ecx
	bndcu bnd1, [0x399]
	bndcu bnd1, [ecx+0x3]
	bndcu bnd1, [eax+ecx]
	bndcu bnd1, [ecx*1]
	bndcu bnd1, [edx+1*eax+0x3]

	### bndcn
	bndcn bnd1, [eax]
	bndcn bnd1, ecx
	bndcn bnd1, [0x399]
	bndcn bnd1, [ecx+0x3]
	bndcn bnd1, [eax+ecx]
	bndcn bnd1, [ecx*1]
	bndcn bnd1, [edx+1*eax+0x3]

	### bndstx
	bndstx [eax+ebx*1+0x3], bnd0
	bndstx [ebx+edx+3], bnd2
	bndstx [ecx*1], bnd2
	bndstx [edx+0x399], bnd3
	bndstx [1*ebx+3], bnd2
	bndstx [edx], bnd1

	### bndldx
	bndldx bnd0, [eax+ebx*1+0x3]
	bndldx bnd2, [ebx+edx+3]
	bndldx bnd2, [ecx*1]
	bndldx bnd3, [edx+0x399]
	bndldx bnd2, [1*ebx+3]
	bndldx bnd1, [edx]

	### bnd
	bnd call	foo
	bnd call	eax
	bnd je	foo
	bnd jmp	foo
	bnd jmp	ecx
	bnd ret

foo:	bnd ret

	.att_syntax prefix
bad:
	# bndldx (%eax),(bad)
	.insn 0x0f1a, (%eax), %esi

	# bndmov (bad),%bnd0
	.insn 0x660f1a, %k4, %bnd0
