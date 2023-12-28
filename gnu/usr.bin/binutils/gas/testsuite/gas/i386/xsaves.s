# Check 32bit XSAVES instructions

	.allow_index_reg
	.text
_start:

	xsaves	(%ecx)	 # XSAVES
	xsaves	-123456(%esp,%esi,8)	 # XSAVES
	xrstors	(%ecx)	 # XSAVES
	xrstors	-123456(%esp,%esi,8)	 # XSAVES

	.intel_syntax noprefix
	xsaves	[ecx]	 # XSAVES
	xsaves	[esp+esi*8-123456]	 # XSAVES
	xrstors	[ecx]	 # XSAVES
	xrstors	[esp+esi*8-123456]	 # XSAVES
