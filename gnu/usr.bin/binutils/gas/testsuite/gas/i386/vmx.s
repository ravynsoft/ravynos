# VMX Instructions

	.text
foo:
	.rept 2

	vmcall
	vmlaunch
	vmresume
	vmxoff
	vmclear (%eax)
	vmptrld (%eax)
	vmptrst (%eax)
	vmxon (%eax)
	vmread %eax,%ebx
	vmreadl %eax,%ebx
	vmread %eax,(%ebx)
	vmreadl %eax,(%ebx)
	vmwrite %eax,%ebx
	vmwritel %eax,%ebx
	vmwrite (%eax),%ebx
	vmwritel (%eax),%ebx

	.code16
	.endr
	.p2align	4,0
