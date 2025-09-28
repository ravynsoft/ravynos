# Check 64bit BMI instructions

	.allow_index_reg
	.text
_start:

# Test for op r16, r/m16
	tzcnt %ax,%bx
	tzcnt (%rcx),%bx
	tzcnt (%rcx),%r15w

# Test for op r32, r32, r/m32
	andn %eax,%ebx,%esi
	andn (%rcx),%ebx,%esi
	andn %r9d,%r15d,%r10d
	andn (%rcx),%r15d,%r10d

# Test for op r32, r/m32, r32
	bextr %eax,%ebx,%esi
	bextr %ebx,(%rcx),%esi
	bextr %r9d,%r15d,%r10d
	bextr %r9d,(%rcx),%r10d

# Test for op r32, r/m32
	tzcnt %eax,%ebx
	tzcnt (%rcx),%ebx
	tzcnt (%rcx),%r15d
	blsi %eax,%ebx
	blsi (%rcx),%ebx
	blsi (%rcx),%r15d
	blsmsk %eax,%ebx
	blsmsk (%rcx),%ebx
	blsmsk (%rcx),%r15d
	blsr %eax,%ebx
	blsr (%rcx),%ebx
	blsr (%rcx),%r15d

# Test for op r64, r64, r/m64
	andn %rax,%rbx,%rsi
	andn (%rcx),%rbx,%rsi
	andn %r9,%r15,%r10
	andn (%rcx),%r15,%r10

# Test for op r64, r/m64, r64
	bextr %rax,%rbx,%rsi
	bextr %rax,(%rcx),%rsi
	bextr %r9,%r15,%r10
	bextr %r9,(%rcx),%r10

# Test for op r64, r/m64
	tzcnt %rax,%rbx
	tzcnt (%rcx),%rbx
	tzcnt %r9,%r15
	tzcnt (%rcx),%r15
	blsi %rax,%rbx
	blsi (%rcx),%rbx
	blsi %r9,%r15
	blsi (%rcx),%r15
	blsmsk %rax,%rbx
	blsmsk (%rcx),%rbx
	blsmsk %r9,%r15
	blsmsk (%rcx),%r15
	blsr %rax,%rbx
	blsr (%rcx),%rbx
	blsr %r9,%r15
	blsr (%rcx),%r15

	.intel_syntax noprefix

# Test for op r16, r/m16
	tzcnt bx,ax
	tzcnt bx,WORD PTR [rcx]
	tzcnt r10w,WORD PTR [rcx]
	tzcnt bx,[rcx]

# Test for op r32, r32, r/m32
	andn esi,ebx,eax
	andn esi,ebx,DWORD PTR [rcx]
	andn r15d,r10d,r9d
	andn r15d,r10d,DWORD PTR [rcx]
	andn esi,ebx,[rcx]

# Test for op r32, r/m32, r32
	bextr esi,ebx,eax
	bextr esi,DWORD PTR [rcx],ebx
	bextr r15d,r10d,r9d
	bextr r15d,DWORD PTR [rcx],r9d
	bextr esi,[rcx],ebx

# Test for op r32, r/m32
	tzcnt ebx,eax
	tzcnt ebx,DWORD PTR [rcx]
	tzcnt r10d,DWORD PTR [rcx]
	tzcnt ebx,[rcx]
	blsi ebx,eax
	blsi ebx,DWORD PTR [rcx]
	blsi r10d,DWORD PTR [rcx]
	blsi ebx,[rcx]
	blsmsk ebx,eax
	blsmsk ebx,DWORD PTR [rcx]
	blsmsk r10d,DWORD PTR [rcx]
	blsmsk ebx,[rcx]
	blsr ebx,eax
	blsr ebx,DWORD PTR [rcx]
	blsr r10d,DWORD PTR [rcx]
	blsr ebx,[rcx]

# Test for op r64, r64, r/m64
	andn rsi,rbx,rax
	andn rsi,rbx,QWORD PTR [rcx]
	andn r10,r15,r9
	andn r10,r15,QWORD PTR [rcx]
	andn rsi,rbx,[rcx]

# Test for op r64, r/m64, r64
	bextr rsi,rbx,rax
	bextr rsi,QWORD PTR [rcx],rax
	bextr r10,r15,r9
	bextr r10,QWORD PTR [rcx],r9
	bextr rsi,[rcx],rax

# Test for op r64, r/m64
	tzcnt rbx,rax
	tzcnt rbx,QWORD PTR [rcx]
	tzcnt r15,r9
	tzcnt r15,QWORD PTR [rcx]
	tzcnt rbx,[rcx]
	blsi rbx,rax
	blsi rbx,QWORD PTR [rcx]
	blsi r15,r9
	blsi r15,QWORD PTR [rcx]
	blsi rbx,[rcx]
	blsmsk rbx,rax
	blsmsk rbx,QWORD PTR [rcx]
	blsmsk r15,r9
	blsmsk r15,QWORD PTR [rcx]
	blsmsk rbx,[rcx]
	blsr rbx,rax
	blsr rbx,QWORD PTR [rcx]
	blsr r15,r9
	blsr r15,QWORD PTR [rcx]
	blsr rbx,[rcx]
