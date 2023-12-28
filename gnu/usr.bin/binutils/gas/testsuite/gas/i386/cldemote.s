# Check 32bit CLDEMOTE instructions

	.allow_index_reg
	.text
_start:

	cldemote	(%ecx)
	cldemote	-123456(%esp,%esi,8)

	.intel_syntax noprefix
	cldemote	BYTE PTR [ecx]
	cldemote	BYTE PTR [esp+esi*8-123456]
