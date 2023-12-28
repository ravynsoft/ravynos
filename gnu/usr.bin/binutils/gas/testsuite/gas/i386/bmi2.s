# Check 32bit BMI2 instructions

	.allow_index_reg
	.text
_start:

# Test for op r32, r/m32, imm8
	rorx $7,%eax,%ebx
	rorx $7,(%ecx),%ebx

# Test for op r32, r32, r/m32
	mulx %eax,%ebx,%esi
	mulx (%ecx),%ebx,%esi
	pdep %eax,%ebx,%esi
	pdep (%ecx),%ebx,%esi
	pext %eax,%ebx,%esi
	pext (%ecx),%ebx,%esi

# Test for op r32, r/m32, r32
	bzhi %eax,%ebx,%esi
	bzhi %ebx,(%ecx),%esi
	sarx %eax,%ebx,%esi
	sarx %ebx,(%ecx),%esi
	shlx %eax,%ebx,%esi
	shlx %ebx,(%ecx),%esi
	shrx %eax,%ebx,%esi
	shrx %ebx,(%ecx),%esi

	.intel_syntax noprefix

# Test for op r32, r/m32, imm8
	rorx ebx,eax,7
	rorx ebx,DWORD PTR [ecx],7
	rorx ebx,[ecx],7

# Test for op r32, r32, r/m32
	mulx esi,ebx,eax
	mulx esi,ebx,DWORD PTR [ecx]
	mulx esi,ebx,[ecx]
	pdep esi,ebx,eax
	pdep esi,ebx,DWORD PTR [ecx]
	pdep esi,ebx,[ecx]
	pext esi,ebx,eax
	pext esi,ebx,DWORD PTR [ecx]
	pext esi,ebx,[ecx]

# Test for op r32, r/m32, r32
	bzhi esi,ebx,eax
	bzhi esi,DWORD PTR [ecx],ebx
	bzhi esi,[ecx],ebx
	sarx esi,ebx,eax
	sarx esi,DWORD PTR [ecx],ebx
	sarx esi,[ecx],ebx
	shlx esi,ebx,eax
	shlx esi,DWORD PTR [ecx],ebx
	shlx esi,[ecx],ebx
	shrx esi,ebx,eax
	shrx esi,DWORD PTR [ecx],ebx
	shrx esi,[ecx],ebx
