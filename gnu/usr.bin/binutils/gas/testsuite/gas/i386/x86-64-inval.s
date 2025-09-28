	.text
	.allow_index_reg
# All the following should be illegal for x86-64
        aaa		# illegal
        aad		# illegal
        aam		# illegal
        aas		# illegal
        arpl %ax,%ax	# illegal
        bound %eax,(%rax) # illegal
	calll *%eax	# 32-bit data size not allowed
        calll *(%ax)	# 32-bit data size not allowed
        calll *(%eax)	# 32-bit data size not allowed
        calll *(%r8)	# 32-bit data size not allowed
        calll *(%rax)	# 32-bit data size not allowed
	callq *(%ax)	# no 16-bit addressing
        daa		# illegal
        das		# illegal
        enterl $0,$0	# can't have 32-bit stack operands
        into		# illegal
foo:	jcxz foo	# No prefix exists to select CX as a counter
	jmpl *%eax	# 32-bit data size not allowed
	jmpl *(%ax)	# 32-bit data size not allowed
	jmpl *(%eax)	# 32-bit data size not allowed
	jmpl *(%r8)	# 32-bit data size not allowed
	jmpl *(%rax)	# 32-bit data size not allowed
	jmpq *(%ax)	# no 16-bit addressing
        lcalll $0,$0	# illegal
        lcallq $0,$0	# illegal
        ldsl %eax,(%rax) # illegal
        ldsq %rax,(%rax) # illegal
        lesl %eax,(%rax) # illegal
        lesq %rax,(%rax) # illegal
        ljmpl $0,$0	# illegal
        ljmpq $0,$0	# illegal
        ljmpq *(%rax)	# 64-bit data size not allowed
	loopw foo	# No prefix exists to select CX as a counter
	loopew foo	# No prefix exists to select CX as a counter
	loopnew foo	# No prefix exists to select CX as a counter
	loopnzw foo	# No prefix exists to select CX as a counter
	loopzw foo	# No prefix exists to select CX as a counter
        leavel		# can't have 32-bit stack operands
        pop %ds		# illegal
        pop %es		# illegal
        pop %ss		# illegal
        popa		# illegal
        popl %eax	# can't have 32-bit stack operands
        push %cs	# illegal
        push %ds	# illegal
        push %es	# illegal
        push %ss	# illegal
        pusha		# illegal
        pushl %eax	# can't have 32-bit stack operands
        pushfl		# can't have 32-bit stack operands
	popfl		# can't have 32-bit stack operands
	retl		# can't have 32-bit stack operands
	insertq $4,$2,%xmm2,%ebx # The last operand must be XMM register.
	fnstsw %eax
	fnstsw %al
	fstsw %eax
	fstsw %al
	in $8,%rax
	out %rax,$8
movzxl (%rax),%rax
movnti %ax, (%rax)
movntiw %ax, (%rax)

mov 0x80000000(%rax),%ebx
mov 0x80000000,%ebx

	add (%rip,%rsi), %eax
	add (%rsi,%rip), %eax
	add (,%rip), %eax
	add (%eip,%esi), %eax
	add (%esi,%eip), %eax
	add (,%eip), %eax
	add (%rsi,%esi), %eax
	add (%esi,%rsi), %eax
	add (%eiz), %eax
	add (%riz), %eax
	add (%rax), %riz
	add (%rax), %eiz

	.intel_syntax noprefix
	cmpxchg16b dword ptr [rax] # Must be oword
	movq xmm1, XMMWORD PTR [rsp]
	movq xmm1, DWORD PTR [rsp]
	movq xmm1, WORD PTR [rsp]
	movq xmm1, BYTE PTR [rsp]
	movq XMMWORD PTR [rsp],xmm1
	movq DWORD PTR [rsp],xmm1
	movq WORD PTR [rsp],xmm1
	movq BYTE PTR [rsp],xmm1
	fnstsw eax
	fnstsw al
	fstsw eax
	fstsw al
	in rax,8
	out 8,rax
movsx ax, [rax]
movsx eax, [rax]
movsx rax, [rax]
movzx ax, [rax]
movzx eax, [rax]
movzx rax, [rax]
movnti word ptr [rax], ax
	calld eax	# 32-bit data size not allowed
	calld [ax]	# 32-bit data size not allowed
	calld [eax]	# 32-bit data size not allowed
	calld [r8]	# 32-bit data size not allowed
	calld [rax]	# 32-bit data size not allowed
	callq [ax]	# no 16-bit addressing
	jmpd eax	# 32-bit data size not allowed
	jmpd [ax]	# 32-bit data size not allowed
	jmpd [eax]	# 32-bit data size not allowed
	jmpd [r8]	# 32-bit data size not allowed
	jmpd [rax]	# 32-bit data size not allowed
	jmpq [ax]	# no 16-bit addressing
	mov eax,[rax+0x876543210] # out of range displacement

	.att_syntax prefix
	movsd (%rsi), %ss:(%rdi), %ss:(%rax)

	incl	(%dx)
	incw	(%dx)
	mov	(%dx), %ax
	mov	%ax, (%dx)
