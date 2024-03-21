# Disassembling with -Msuffix.

	.text
foo:
	monitor
	mwait

	vmcall
	vmlaunch
	vmresume
	vmxoff

	iretw
	iretl
	iretq
	sysretl
	mov	%rsp,%rbp
	sysretq

	.intel_syntax noprefix
	iretw
	iretd
	iret
	iretq
	sysretd
	mov	rbp,rsp
	sysretq
