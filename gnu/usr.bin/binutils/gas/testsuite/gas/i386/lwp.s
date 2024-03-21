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

	slwpcb %edi
	slwpcb %esi
	slwpcb %ebp
	slwpcb %esp
	slwpcb %ebx
	slwpcb %edx
	slwpcb %ecx
	slwpcb %eax

	lwpins $0x12345678, %edi, %eax
	lwpins $0x12345678, %esi, %ecx
	lwpins $0x12345678, %ebp, %edx
	lwpins $0x12345678, %esp, %ebx
	lwpins $0x12345678, %ebx, %esp
	lwpins $0x12345678, %edx, %ebp
	lwpins $0x12345678, %ecx, %esi
	lwpins $0x12345678, %eax, %edi

	lwpval $0x12345678, %edi, %eax
	lwpval $0x12345678, %esi, %ecx
	lwpval $0x12345678, %ebp, %edx
	lwpval $0x12345678, %esp, %ebx
	lwpval $0x12345678, %ebx, %esp
	lwpval $0x12345678, %edx, %ebp
	lwpval $0x12345678, %ecx, %esi
	lwpval $0x12345678, %eax, %edi

	lwpins $0x12345678, (%edi), %eax
	lwpins $0x12345678, (%esi), %ecx
	lwpins $0x12345678, (%ebp), %edx
	lwpins $0x12345678, (%esp), %ebx
	lwpins $0x12345678, (%ebx), %esp
	lwpins $0x12345678, (%edx), %ebp
	lwpins $0x12345678, (%ecx), %esi
	lwpins $0x12345678, (%eax), %edi

	lwpval $0x12345678, (%edi), %eax
	lwpval $0x12345678, (%esi), %ecx
	lwpval $0x12345678, (%ebp), %edx
	lwpval $0x12345678, (%esp), %ebx
	lwpval $0x12345678, (%ebx), %esp
	lwpval $0x12345678, (%edx), %ebp
	lwpval $0x12345678, (%ecx), %esi
	lwpval $0x12345678, (%eax), %edi

	lwpins $0x12345678, 0xcafe(%edi), %eax
	lwpins $0x12345678, 0xcafe(%esi), %ecx
	lwpins $0x12345678, 0xcafe(%ebp), %edx
	lwpins $0x12345678, 0xcafe(%esp), %ebx
	lwpins $0x12345678, 0xcafe(%ebx), %esp
	lwpins $0x12345678, 0xcafe(%edx), %ebp
	lwpins $0x12345678, 0xcafe(%ecx), %esi
	lwpins $0x12345678, 0xcafe(%eax), %edi

	lwpval $0x12345678, 0xcafe(%edi), %eax
	lwpval $0x12345678, 0xcafe(%esi), %ecx
	lwpval $0x12345678, 0xcafe(%ebp), %edx
	lwpval $0x12345678, 0xcafe(%esp), %ebx
	lwpval $0x12345678, 0xcafe(%ebx), %esp
	lwpval $0x12345678, 0xcafe(%edx), %ebp
	lwpval $0x12345678, 0xcafe(%ecx), %esi
	lwpval $0x12345678, 0xcafe(%eax), %edi
