	;; These instructions can be found in the FRV Linux kernel.
	;; They used to fail to assemble on 64-bit host machines
	;; because of sign-extension problems.

	.text
	.global foo
foo:
	setlos   #0xffffe000, gr3
	sethi.p  %hi(~(0x80000000 | 0x40000000)), gr4
