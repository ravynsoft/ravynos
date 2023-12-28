# Check 64bit BMI2 instructions

	.allow_index_reg
	.text
_start:

# Test for op r32, r/m32, imm8
	rorx $7,%eax,%ebx
	rorx $7,(%rcx),%ebx
	rorx $7,%r9d,%r15d
	rorx $7,(%rcx),%r15d

# Test for op r32, r32, r/m32
	mulx %eax,%ebx,%esi
	mulx (%rcx),%ebx,%esi
	mulx %r9d,%r15d,%r10d
	mulx (%rcx),%r15d,%r10d
	pdep %eax,%ebx,%esi
	pdep (%rcx),%ebx,%esi
	pdep %r9d,%r15d,%r10d
	pdep (%rcx),%r15d,%r10d
	pext %eax,%ebx,%esi
	pext (%rcx),%ebx,%esi
	pext %r9d,%r15d,%r10d
	pext (%rcx),%r15d,%r10d

# Test for op r32, r/m32, r32
	bzhi %eax,%ebx,%esi
	bzhi %ebx,(%rcx),%esi
	bzhi %r9d,%r15d,%r10d
	bzhi %r9d,(%rcx),%r10d
	sarx %eax,%ebx,%esi
	sarx %ebx,(%rcx),%esi
	sarx %r9d,%r15d,%r10d
	sarx %r9d,(%rcx),%r10d
	shlx %eax,%ebx,%esi
	shlx %ebx,(%rcx),%esi
	shlx %r9d,%r15d,%r10d
	shlx %r9d,(%rcx),%r10d
	shrx %eax,%ebx,%esi
	shrx %ebx,(%rcx),%esi
	shrx %r9d,%r15d,%r10d
	shrx %r9d,(%rcx),%r10d

# Test for op r64, r/m64, imm8
	rorx $7,%rax,%rbx
	rorx $7,(%rcx),%rbx
	rorx $7,%r9,%r15
	rorx $7,(%rcx),%r15

# Test for op r64, r64, r/m64
	mulx %rax,%rbx,%rsi
	mulx (%rcx),%rbx,%rsi
	mulx %r9,%r15,%r10
	mulx (%rcx),%r15,%r10
	pdep %rax,%rbx,%rsi
	pdep (%rcx),%rbx,%rsi
	pdep %r9,%r15,%r10
	pdep (%rcx),%r15,%r10
	pext %rax,%rbx,%rsi
	pext (%rcx),%rbx,%rsi
	pext %r9,%r15,%r10
	pext (%rcx),%r15,%r10

# Test for op r64, r/m64, r64
	bzhi %rax,%rbx,%rsi
	bzhi %rax,(%rcx),%rsi
	bzhi %r9,%r15,%r10
	bzhi %r9,(%rcx),%r10
	sarx %rax,%rbx,%rsi
	sarx %rax,(%rcx),%rsi
	sarx %r9,%r15,%r10
	sarx %r9,(%rcx),%r10
	shlx %rax,%rbx,%rsi
	shlx %rax,(%rcx),%rsi
	shlx %r9,%r15,%r10
	shlx %r9,(%rcx),%r10
	shrx %rax,%rbx,%rsi
	shrx %rax,(%rcx),%rsi
	shrx %r9,%r15,%r10
	shrx %r9,(%rcx),%r10

	.intel_syntax noprefix

# Test for op r32, r/m32, imm8
	rorx ebx,eax,7
	rorx ebx,DWORD PTR [rcx],7
	rorx r10d,r9d,7
	rorx r10d,DWORD PTR [rcx],7
	rorx ebx,[rcx],7

# Test for op r32, r32, r/m32
	mulx esi,ebx,eax
	mulx esi,ebx,DWORD PTR [rcx]
	mulx r15d,r10d,r9d
	mulx r15d,r10d,DWORD PTR [rcx]
	mulx esi,ebx,[rcx]
	pdep esi,ebx,eax
	pdep esi,ebx,DWORD PTR [rcx]
	pdep r15d,r10d,r9d
	pdep r15d,r10d,DWORD PTR [rcx]
	pdep esi,ebx,[rcx]
	pext esi,ebx,eax
	pext esi,ebx,DWORD PTR [rcx]
	pext r15d,r10d,r9d
	pext r15d,r10d,DWORD PTR [rcx]
	pext esi,ebx,[rcx]

# Test for op r32, r/m32, r32
	bzhi esi,ebx,eax
	bzhi esi,DWORD PTR [rcx],ebx
	bzhi r15d,r10d,r9d
	bzhi r15d,DWORD PTR [rcx],r9d
	bzhi esi,[rcx],ebx
	sarx esi,ebx,eax
	sarx esi,DWORD PTR [rcx],ebx
	sarx r15d,r10d,r9d
	sarx r15d,DWORD PTR [rcx],r9d
	sarx esi,[rcx],ebx
	shlx esi,ebx,eax
	shlx esi,DWORD PTR [rcx],ebx
	shlx r15d,r10d,r9d
	shlx r15d,DWORD PTR [rcx],r9d
	shlx esi,[rcx],ebx
	shrx esi,ebx,eax
	shrx esi,DWORD PTR [rcx],ebx
	shrx r15d,r10d,r9d
	shrx r15d,DWORD PTR [rcx],r9d
	shrx esi,[rcx],ebx

# Test for op r64, r/m64, imm8
	rorx rbx,rax,7
	rorx rbx,QWORD PTR [rcx],7
	rorx r15,r9,7
	rorx r15,QWORD PTR [rcx],7
	rorx rbx,[rcx],7

# Test for op r64, r64, r/m64
	mulx rsi,rbx,rax
	mulx rsi,rbx,QWORD PTR [rcx]
	mulx r10,r15,r9
	mulx r10,r15,QWORD PTR [rcx]
	mulx rsi,rbx,[rcx]
	pdep rsi,rbx,rax
	pdep rsi,rbx,QWORD PTR [rcx]
	pdep r10,r15,r9
	pdep r10,r15,QWORD PTR [rcx]
	pdep rsi,rbx,[rcx]
	pext rsi,rbx,rax
	pext rsi,rbx,QWORD PTR [rcx]
	pext r10,r15,r9
	pext r10,r15,QWORD PTR [rcx]
	pext rsi,rbx,[rcx]

# Test for op r64, r/m64, r64
	bzhi rsi,rbx,rax
	bzhi rsi,QWORD PTR [rcx],rax
	bzhi r10,r15,r9
	bzhi r10,QWORD PTR [rcx],r9
	bzhi rsi,[rcx],rax
	sarx rsi,rbx,rax
	sarx rsi,QWORD PTR [rcx],rax
	sarx r10,r15,r9
	sarx r10,QWORD PTR [rcx],r9
	sarx rsi,[rcx],rax
	shlx rsi,rbx,rax
	shlx rsi,QWORD PTR [rcx],rax
	shlx r10,r15,r9
	shlx r10,QWORD PTR [rcx],r9
	shlx rsi,[rcx],rax
	shrx rsi,rbx,rax
	shrx rsi,QWORD PTR [rcx],rax
	shrx r10,r15,r9
	shrx r10,QWORD PTR [rcx],r9
	shrx rsi,[rcx],rax
