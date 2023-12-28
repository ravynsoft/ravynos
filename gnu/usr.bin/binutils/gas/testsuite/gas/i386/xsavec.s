# Check 32bit XSAVEC instructions

	.allow_index_reg
	.text
_start:

	xsavec	(%ecx)	 # XSAVEC
	xsavec	-123456(%esp,%esi,8)	 # XSAVEC

	.intel_syntax noprefix
	xsavec	[ecx]	 # XSAVEC
	xsavec	[esp+esi*8-123456]	 # XSAVEC
