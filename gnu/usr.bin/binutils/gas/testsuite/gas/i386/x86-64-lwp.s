# Check 64bit LWP instructions

	.allow_index_reg
	.text
_start:

	llwpcb %eax
	llwpcb %ecx
	llwpcb %edx
	llwpcb %ebx
	llwpcb %esp
	llwpcb %ebp
	llwpcb %esi
	llwpcb %edi
	llwpcb %r8d
	llwpcb %r9d
	llwpcb %r10d
	llwpcb %r11d
	llwpcb %r12d
	llwpcb %r13d
	llwpcb %r14d
	llwpcb %r15d
	llwpcb %rax
	llwpcb %rcx
	llwpcb %rdx
	llwpcb %rbx
	llwpcb %rsp
	llwpcb %rbp
	llwpcb %rsi
	llwpcb %rdi
	llwpcb %r8
	llwpcb %r9
	llwpcb %r10
	llwpcb %r11
	llwpcb %r12
	llwpcb %r13
	llwpcb %r14
	llwpcb %r15

	slwpcb %r15
	slwpcb %r14
	slwpcb %r13
	slwpcb %r12
	slwpcb %r11
	slwpcb %r10
	slwpcb %r9
	slwpcb %r8
	slwpcb %rdi
	slwpcb %rsi
	slwpcb %rbp
	slwpcb %rsp
	slwpcb %rbx
	slwpcb %rdx
	slwpcb %rcx
	slwpcb %rax
	slwpcb %r15d
	slwpcb %r14d
	slwpcb %r13d
	slwpcb %r12d
	slwpcb %r11d
	slwpcb %r10d
	slwpcb %r9d
	slwpcb %r8d
	slwpcb %edi
	slwpcb %esi
	slwpcb %ebp
	slwpcb %esp
	slwpcb %ebx
	slwpcb %edx
	slwpcb %ecx
	slwpcb %eax

	lwpins $0x12345678, %r15d, %eax
	lwpins $0x12345678, %r14d, %ecx
	lwpins $0x12345678, %r13d, %edx
	lwpins $0x12345678, %r12d, %ebx
	lwpins $0x12345678, %r11d, %esp
	lwpins $0x12345678, %r10d, %ebp
	lwpins $0x12345678, %r9d, %esi
	lwpins $0x12345678, %r8d, %edi
	lwpins $0x12345678, %edi, %r8d
	lwpins $0x12345678, %esi, %r9d
	lwpins $0x12345678, %ebp, %r10d
	lwpins $0x12345678, %esp, %r11d
	lwpins $0x12345678, %ebx, %r12d
	lwpins $0x12345678, %edx, %r13d
	lwpins $0x12345678, %ecx, %r14d
	lwpins $0x12345678, %eax, %r15d
	lwpins $0x12345678, %r15d, %rax
	lwpins $0x12345678, %r14d, %rcx
	lwpins $0x12345678, %r13d, %rdx
	lwpins $0x12345678, %r12d, %rbx
	lwpins $0x12345678, %r11d, %rsp
	lwpins $0x12345678, %r10d, %rbp
	lwpins $0x12345678, %r9d, %rsi
	lwpins $0x12345678, %r8d, %rdi
	lwpins $0x12345678, %eax, %r8
	lwpins $0x12345678, %ecx, %r9
	lwpins $0x12345678, %edx, %r10
	lwpins $0x12345678, %ebx, %r11
	lwpins $0x12345678, %esp, %r12
	lwpins $0x12345678, %ebp, %r13
	lwpins $0x12345678, %esi, %r14
	lwpins $0x12345678, %edi, %r15

	lwpval $0x12345678, %r15d, %eax
	lwpval $0x12345678, %r14d, %ecx
	lwpval $0x12345678, %r13d, %edx
	lwpval $0x12345678, %r12d, %ebx
	lwpval $0x12345678, %r11d, %esp
	lwpval $0x12345678, %r10d, %ebp
	lwpval $0x12345678, %r9d, %esi
	lwpval $0x12345678, %r8d, %edi
	lwpval $0x12345678, %edi, %r8d
	lwpval $0x12345678, %esi, %r9d
	lwpval $0x12345678, %ebp, %r10d
	lwpval $0x12345678, %esp, %r11d
	lwpval $0x12345678, %ebx, %r12d
	lwpval $0x12345678, %edx, %r13d
	lwpval $0x12345678, %ecx, %r14d
	lwpval $0x12345678, %eax, %r15d
	lwpval $0x12345678, %r15d, %rax
	lwpval $0x12345678, %r14d, %rcx
	lwpval $0x12345678, %r13d, %rdx
	lwpval $0x12345678, %r12d, %rbx
	lwpval $0x12345678, %r11d, %rsp
	lwpval $0x12345678, %r10d, %rbp
	lwpval $0x12345678, %r9d, %rsi
	lwpval $0x12345678, %r8d, %rdi
	lwpval $0x12345678, %eax, %r8
	lwpval $0x12345678, %ecx, %r9
	lwpval $0x12345678, %edx, %r10
	lwpval $0x12345678, %ebx, %r11
	lwpval $0x12345678, %esp, %r12
	lwpval $0x12345678, %ebp, %r13
	lwpval $0x12345678, %esi, %r14
	lwpval $0x12345678, %edi, %r15

	lwpins $0x12345678, (%r15d), %eax
	lwpins $0x12345678, (%r14d), %ecx
	lwpins $0x12345678, (%r13d), %edx
	lwpins $0x12345678, (%r12d), %ebx
	lwpins $0x12345678, (%r11d), %esp
	lwpins $0x12345678, (%r10d), %ebp
	lwpins $0x12345678, (%r9d), %esi
	lwpins $0x12345678, (%r8d), %edi
	lwpins $0x12345678, (%edi), %r8d
	lwpins $0x12345678, (%esi), %r9d
	lwpins $0x12345678, (%ebp), %r10d
	lwpins $0x12345678, (%esp), %r11d
	lwpins $0x12345678, (%ebx), %r12d
	lwpins $0x12345678, (%edx), %r13d
	lwpins $0x12345678, (%ecx), %r14d
	lwpins $0x12345678, (%eax), %r15d
	lwpins $0x12345678, (%r15d), %rax
	lwpins $0x12345678, (%r14d), %rcx
	lwpins $0x12345678, (%r13d), %rdx
	lwpins $0x12345678, (%r12d), %rbx
	lwpins $0x12345678, (%r11d), %rsp
	lwpins $0x12345678, (%r10d), %rbp
	lwpins $0x12345678, (%r9d), %rsi
	lwpins $0x12345678, (%r8d), %rdi
	lwpins $0x12345678, (%eax), %r8
	lwpins $0x12345678, (%ecx), %r9
	lwpins $0x12345678, (%edx), %r10
	lwpins $0x12345678, (%ebx), %r11
	lwpins $0x12345678, (%esp), %r12
	lwpins $0x12345678, (%ebp), %r13
	lwpins $0x12345678, (%esi), %r14
	lwpins $0x12345678, (%edi), %r15

	lwpval $0x12345678, (%r15d), %eax
	lwpval $0x12345678, (%r14d), %ecx
	lwpval $0x12345678, (%r13d), %edx
	lwpval $0x12345678, (%r12d), %ebx
	lwpval $0x12345678, (%r11d), %esp
	lwpval $0x12345678, (%r10d), %ebp
	lwpval $0x12345678, (%r9d), %esi
	lwpval $0x12345678, (%r8d), %edi
	lwpval $0x12345678, (%edi), %r8d
	lwpval $0x12345678, (%esi), %r9d
	lwpval $0x12345678, (%ebp), %r10d
	lwpval $0x12345678, (%esp), %r11d
	lwpval $0x12345678, (%ebx), %r12d
	lwpval $0x12345678, (%edx), %r13d
	lwpval $0x12345678, (%ecx), %r14d
	lwpval $0x12345678, (%eax), %r15d
	lwpval $0x12345678, (%r15d), %rax
	lwpval $0x12345678, (%r14d), %rcx
	lwpval $0x12345678, (%r13d), %rdx
	lwpval $0x12345678, (%r12d), %rbx
	lwpval $0x12345678, (%r11d), %rsp
	lwpval $0x12345678, (%r10d), %rbp
	lwpval $0x12345678, (%r9d), %rsi
	lwpval $0x12345678, (%r8d), %rdi
	lwpval $0x12345678, (%eax), %r8
	lwpval $0x12345678, (%ecx), %r9
	lwpval $0x12345678, (%edx), %r10
	lwpval $0x12345678, (%ebx), %r11
	lwpval $0x12345678, (%esp), %r12
	lwpval $0x12345678, (%ebp), %r13
	lwpval $0x12345678, (%esi), %r14
	lwpval $0x12345678, (%edi), %r15

	lwpins $0x12345678, 0xcafe(%r15d), %eax
	lwpins $0x12345678, 0xcafe(%r14d), %ecx
	lwpins $0x12345678, 0xcafe(%r13d), %edx
	lwpins $0x12345678, 0xcafe(%r12d), %ebx
	lwpins $0x12345678, 0xcafe(%r11d), %esp
	lwpins $0x12345678, 0xcafe(%r10d), %ebp
	lwpins $0x12345678, 0xcafe(%r9d), %esi
	lwpins $0x12345678, 0xcafe(%r8d), %edi
	lwpins $0x12345678, 0xcafe(%edi), %r8d
	lwpins $0x12345678, 0xcafe(%esi), %r9d
	lwpins $0x12345678, 0xcafe(%ebp), %r10d
	lwpins $0x12345678, 0xcafe(%esp), %r11d
	lwpins $0x12345678, 0xcafe(%ebx), %r12d
	lwpins $0x12345678, 0xcafe(%edx), %r13d
	lwpins $0x12345678, 0xcafe(%ecx), %r14d
	lwpins $0x12345678, 0xcafe(%eax), %r15d
	lwpins $0x12345678, 0xcafe(%r15d), %rax
	lwpins $0x12345678, 0xcafe(%r14d), %rcx
	lwpins $0x12345678, 0xcafe(%r13d), %rdx
	lwpins $0x12345678, 0xcafe(%r12d), %rbx
	lwpins $0x12345678, 0xcafe(%r11d), %rsp
	lwpins $0x12345678, 0xcafe(%r10d), %rbp
	lwpins $0x12345678, 0xcafe(%r9d), %rsi
	lwpins $0x12345678, 0xcafe(%r8d), %rdi
	lwpins $0x12345678, 0xcafe(%eax), %r8
	lwpins $0x12345678, 0xcafe(%ecx), %r9
	lwpins $0x12345678, 0xcafe(%edx), %r10
	lwpins $0x12345678, 0xcafe(%ebx), %r11
	lwpins $0x12345678, 0xcafe(%esp), %r12
	lwpins $0x12345678, 0xcafe(%ebp), %r13
	lwpins $0x12345678, 0xcafe(%esi), %r14
	lwpins $0x12345678, 0xcafe(%edi), %r15

	lwpval $0x12345678, 0xcafe(%r15d), %eax
	lwpval $0x12345678, 0xcafe(%r14d), %ecx
	lwpval $0x12345678, 0xcafe(%r13d), %edx
	lwpval $0x12345678, 0xcafe(%r12d), %ebx
	lwpval $0x12345678, 0xcafe(%r11d), %esp
	lwpval $0x12345678, 0xcafe(%r10d), %ebp
	lwpval $0x12345678, 0xcafe(%r9d), %esi
	lwpval $0x12345678, 0xcafe(%r8d), %edi
	lwpval $0x12345678, 0xcafe(%edi), %r8d
	lwpval $0x12345678, 0xcafe(%esi), %r9d
	lwpval $0x12345678, 0xcafe(%ebp), %r10d
	lwpval $0x12345678, 0xcafe(%esp), %r11d
	lwpval $0x12345678, 0xcafe(%ebx), %r12d
	lwpval $0x12345678, 0xcafe(%edx), %r13d
	lwpval $0x12345678, 0xcafe(%ecx), %r14d
	lwpval $0x12345678, 0xcafe(%eax), %r15d
	lwpval $0x12345678, 0xcafe(%r15d), %rax
	lwpval $0x12345678, 0xcafe(%r14d), %rcx
	lwpval $0x12345678, 0xcafe(%r13d), %rdx
	lwpval $0x12345678, 0xcafe(%r12d), %rbx
	lwpval $0x12345678, 0xcafe(%r11d), %rsp
	lwpval $0x12345678, 0xcafe(%r10d), %rbp
	lwpval $0x12345678, 0xcafe(%r9d), %rsi
	lwpval $0x12345678, 0xcafe(%r8d), %rdi
	lwpval $0x12345678, 0xcafe(%eax), %r8
	lwpval $0x12345678, 0xcafe(%ecx), %r9
	lwpval $0x12345678, 0xcafe(%edx), %r10
	lwpval $0x12345678, 0xcafe(%ebx), %r11
	lwpval $0x12345678, 0xcafe(%esp), %r12
	lwpval $0x12345678, 0xcafe(%ebp), %r13
	lwpval $0x12345678, 0xcafe(%esi), %r14
	lwpval $0x12345678, 0xcafe(%edi), %r15
