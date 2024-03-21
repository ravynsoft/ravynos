# Check 64bit VAES instructions

	.allow_index_reg
	.text
_start:
# Tests for op ymm/mem256, ymm, ymm
	vaesenc %ymm4,%ymm6,%ymm2
	vaesenc (%rcx),%ymm6,%ymm7
	vaesenclast %ymm4,%ymm6,%ymm2
	vaesenclast (%rcx),%ymm6,%ymm7
	vaesdec %ymm4,%ymm6,%ymm2
	vaesdec (%rcx),%ymm6,%ymm7
	vaesdeclast %ymm4,%ymm6,%ymm2
	vaesdeclast (%rcx),%ymm6,%ymm7
	
	.intel_syntax noprefix

# Tests for op ymm/mem256, ymm, ymm
	vaesenc ymm2,ymm6,ymm4
	vaesenc ymm7,ymm6,YMMWORD PTR [rcx]
	vaesenc ymm7,ymm6,[rcx]
	vaesenclast ymm2,ymm6,ymm4
	vaesenclast ymm7,ymm6,YMMWORD PTR [rcx]
	vaesenclast ymm7,ymm6,[rcx]
	vaesdec ymm2,ymm6,ymm4
	vaesdec ymm7,ymm6,YMMWORD PTR [rcx]
	vaesdec ymm7,ymm6,[rcx]
	vaesdeclast ymm2,ymm6,ymm4
	vaesdeclast ymm7,ymm6,YMMWORD PTR [rcx]
	vaesdeclast ymm7,ymm6,[rcx]
