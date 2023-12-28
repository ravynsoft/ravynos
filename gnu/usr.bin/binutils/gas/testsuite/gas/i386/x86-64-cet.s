# Check 64bit CET instructions
	.text
_start:
	incsspd %r12d
	incsspq %rax
	rdsspd %r12d
	rdsspq %rax
	saveprevssp
	rstorssp (%r12)
	wrssd %eax, (%r12)
	wrssq %rdx, (%rcx, %r15)
	wrussd %eax, (%r12)
	wrussq %rcx, (%rbx, %rax)
	setssbsy
	clrssbsy (%rsi, %r12)
	endbr64
	endbr32

	.intel_syntax noprefix
	incsspd r12d
	incsspq rax
	rdsspd r12d
	rdsspq rax
	saveprevssp
	rstorssp QWORD PTR [r12]
	wrssd [r12],eax
	wrssd dword ptr [rax],r12d
	wrssq [rcx+r15],rdx
	wrssq qword ptr [rdx+r15],rcx
	wrussd [r12],eax
	wrussd dword ptr [rax],r12d
	wrussq [rbx+rax],rcx
	wrussq qword ptr [rcx+rax],rbx
	setssbsy
	clrssbsy QWORD PTR [rsi+r12]
	endbr64
	endbr32
