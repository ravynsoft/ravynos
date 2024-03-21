# Check 32bit BMI instructions

	.allow_index_reg
	.text
_start:

# Test for op r16, r/m16
	tzcnt %ax,%bx
	tzcnt (%ecx),%bx

# Test for op r32, r32, r/m32
	andn %eax,%ebx,%esi
	andn (%ecx),%ebx,%esi

# Test for op r32, r/m32, r32
	bextr %eax,%ebx,%esi
	bextr %ebx,(%ecx),%esi

# Test for op r32, r/m32
	tzcnt %eax,%ebx
	tzcnt (%ecx),%ebx
	blsi %eax,%ebx
	blsi (%ecx),%ebx
	blsmsk %eax,%ebx
	blsmsk (%ecx),%ebx
	blsr %eax,%ebx
	blsr (%ecx),%ebx

	.intel_syntax noprefix

# Test for op r16, r/m16
	tzcnt bx,ax
	tzcnt bx,WORD PTR [ecx]
	tzcnt bx,[ecx]

# Test for op r32, r32, r/m32
	andn esi,ebx,eax
	andn esi,ebx,DWORD PTR [ecx]
	andn esi,ebx,[ecx]

# Test for op r32, r/m32, r32
	bextr esi,ebx,eax
	bextr esi,DWORD PTR [ecx],ebx
	bextr esi,[ecx],ebx

# Test for op r32, r/m32
	tzcnt ebx,eax
	tzcnt ebx,DWORD PTR [ecx]
	tzcnt ebx,[ecx]
	blsi ebx,eax
	blsi ebx,DWORD PTR [ecx]
	blsi ebx,[ecx]
	blsmsk ebx,eax
	blsmsk ebx,DWORD PTR [ecx]
	blsmsk ebx,[ecx]
	blsr ebx,eax
	blsr ebx,DWORD PTR [ecx]
	blsr ebx,[ecx]
