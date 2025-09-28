# Check 32bit WAITPKG instructions.

	.text
_start:
	.rept 2
	umonitor %eax
	umonitor %cx
	umwait %ecx
	umwait %ebx, %edx, %eax
	tpause %ecx
	tpause %ebx, %edx, %eax

	.intel_syntax noprefix

	umwait edi, edx, eax
	tpause edi, edx, eax

	.att_syntax prefix
	.code16
	.endr
