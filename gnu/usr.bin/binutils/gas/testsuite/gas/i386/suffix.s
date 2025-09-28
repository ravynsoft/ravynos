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
	iret
	sysretl
	sysret

	.intel_syntax noprefix
	iretw
	iretd
	iret
	sysretd
	sysret
