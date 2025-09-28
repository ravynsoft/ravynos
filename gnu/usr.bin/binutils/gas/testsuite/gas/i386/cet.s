# Check CET instructions
	.text
_start:
	incsspd %ecx
	rdsspd %ecx
	saveprevssp
	rstorssp (%ecx)
	wrssd %eax, (%edx, %eax)
	wrussd %edx, (%edi, %ebp)
	setssbsy
	clrssbsy (%esp, %eax)
	endbr64
	endbr32

	.intel_syntax noprefix
	.rept 2
	incsspd ecx
	rdsspd ecx
	saveprevssp
	rstorssp QWORD PTR [ecx + eax - 0x70]
	wrssd [edx],eax
	wrssd dword ptr [eax],edx
	wrussd [edi + ebp],edx
	wrussd dword ptr [esi + ecx],edi
	setssbsy
	clrssbsy QWORD PTR [esp + eax * 2]
	endbr64
	endbr32
	.code16
	.endr
