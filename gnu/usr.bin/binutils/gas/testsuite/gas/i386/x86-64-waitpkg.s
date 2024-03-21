# Check 64bit WAITPKG instructions.

	.text
_start:
	umonitor %rax
	umonitor %r10
	umonitor %r10d
	umwait %ecx
	umwait %r10d
	umwait %edi, %edx, %eax
	tpause %ecx
	tpause %r10d
	tpause %edi, %edx, %eax

	.intel_syntax noprefix

	umwait esi, edx, eax
	tpause esi, edx, eax
