# Check VAES instructions

	.allow_index_reg
	.text
_start:
# Tests for op ymm/mem256, ymm, ymm
	vaesenc %ymm4,%ymm6,%ymm2
	vaesenc (%ecx),%ymm6,%ymm7
	vaesenclast %ymm4,%ymm6,%ymm2
	vaesenclast (%ecx),%ymm6,%ymm7
	vaesdec %ymm4,%ymm6,%ymm2
	vaesdec (%ecx),%ymm6,%ymm7
	vaesdeclast %ymm4,%ymm6,%ymm2
	vaesdeclast (%ecx),%ymm6,%ymm7
	
	.intel_syntax noprefix

# Tests for op ymm/mem256, ymm, ymm
	vaesenc ymm2,ymm6,ymm4
	vaesenc ymm7,ymm6,YMMWORD PTR [ecx]
	vaesenc ymm7,ymm6,[ecx]
	vaesenclast ymm2,ymm6,ymm4
	vaesenclast ymm7,ymm6,YMMWORD PTR [ecx]
	vaesenclast ymm7,ymm6,[ecx]
	vaesdec ymm2,ymm6,ymm4
	vaesdec ymm7,ymm6,YMMWORD PTR [ecx]
	vaesdec ymm7,ymm6,[ecx]
	vaesdeclast ymm2,ymm6,ymm4
	vaesdeclast ymm7,ymm6,YMMWORD PTR [ecx]
	vaesdeclast ymm7,ymm6,[ecx]
