# MPX instructions
	.allow_index_reg
	.text

	### bndmk
	bndmk (%eax), %bnd1
	bndmk 0x3(%ecx,%ebx,1), %bnd1
	bndmk (%rip), %bnd3
	bndmk (%eip), %bnd2

	### bndmov
	bndmov (%r8d), %bnd1
	bndmov 0x3(%r9d,%edx,1), %bnd1

	bndmov %bnd1, (%eax)
	bndmov %bnd1, 0x3(%ecx,%eax,1)

	### bndcl
	bndcl (%ecx), %bnd1
	bndcl 0x3(%ecx,%eax,1), %bnd1
	bndcl %ecx, %bnd1
	bndcl %cx, %bnd1

	### bndcu
	bndcu (%ecx), %bnd1
	bndcu 0x3(%ecx,%eax,1), %bnd1
	bndcu %ecx, %bnd1
	bndcu %cx, %bnd1

	### bndcn
	bndcn (%ecx), %bnd1
	bndcn 0x3(%ecx,%eax,1), %bnd1
	bndcn %ecx, %bnd1
	bndcn %cx, %bnd1

	### bndstx
	bndstx %bnd0, 0x3(%eax,%ebx,1)
	bndstx %bnd2, 3(%ebx,1)
	bndstx %bnd1, (%r15,%rax,2)
	bndstx %bnd3, base(%rip)
	bndstx %bnd1, base(%eip)

	### bndldx
	bndldx 0x3(%eax,%ebx,1), %bnd0
	bndldx 3(%ebx,1), %bnd2
	bndldx (%rax,%r15,4), %bnd3
	bndldx base(%rip), %bnd1
	bndldx base(%eip), %bnd3

.intel_syntax noprefix
	bndmk bnd1, [eax]
	bndmk bnd1, [edx+1*eax+0x3]
	bndmk bnd3, [rip]
	bndmk bnd2, [eip]
	bndmk bnd2, [rax+rsp]

	### bndmov
	bndmov bnd1, [eax]
	bndmov bnd1, [edx+1*eax+0x3]

	bndmov [eax], bnd1
	bndmov [edx+1*eax+0x3], bnd1

	### bndcl
	bndcl bnd1, [eax]
	bndcl bnd1, [edx+1*eax+0x3]
	bndcl bnd1, eax
	bndcl bnd1, dx

	### bndcu
	bndcu bnd1, [eax]
	bndcu bnd1, [edx+1*eax+0x3]
	bndcu bnd1, eax
	bndcu bnd1, dx

	### bndcn
	bndcn bnd1, [eax]
	bndcn bnd1, [edx+1*eax+0x3]
	bndcn bnd1, eax
	bndcn bnd1, dx

	### bndstx
	bndstx [eax+ebx*1+0x3], bnd0
	bndstx [1*ebx+3], bnd2
	bndstx [r8+rdi*4], bnd2
	bndstx [rip+base], bnd1
	bndstx [eip+base], bnd3
	bndstx [rax+rsp], bnd3

	### bndldx
	bndldx bnd0, [eax+ebx*1+0x3]
	bndldx bnd2, [1*ebx+3]
	bndldx bnd2, [rdi+r8*8]
	bndldx bnd1, [rip+base]
	bndldx bnd3, [eip+base]
	bndldx bnd3, [rax+rsp]

	# Force a good alignment.
	.p2align	4,0
