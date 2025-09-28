# Check error for invalid {1toXX} and {2toXX} broadcasts.

	.allow_index_reg
	.text
_start:
	vcvtpd2ph (%ecx){1to16}, %xmm3
	vcvtuqq2ph -1024(%edx){1to32}, %xmm3
	vcvtdq2ph (%ecx){1to8}, %ymm3
	vcvtudq2ph -512(%edx){1to32}, %ymm3
	vcmpph  $123, (%ecx){1to16}, %zmm2, %k5
	vcmpph  $123, (%ecx){1to64}, %zmm2, %k5
	vfmadd132ph (%ecx){1to8}, %zmm2, %zmm3
	vfcmaddcph (%ecx){1to8}, %zmm2, %zmm3
	vfcmulcph (%ecx){1to32}, %zmm2, %zmm3
	vcvtdq2ph (%ecx){1to8}, %ymm3
	vfmaddcph (%ecx){1to8}, %zmm2, %zmm3
	vfmulcph -512(%edx){1to32}, %zmm2, %zmm3
	vfmulcph -512(%edx){1to4}, %zmm2, %zmm3

	.intel_syntax noprefix
	vcvtpd2ph xmm3, QWORD PTR [ecx]{1to16}
	vcvtuqq2ph xmm3, QWORD PTR [edx-1024]{1to32}
	vcvtdq2ph ymm3, DWORD PTR [ecx]{1to8}
	vcvtudq2ph ymm3, DWORD PTR [edx-512]{1to32}
	vcmpph k5, zmm2, WORD PTR [edx-256]{1to16}, 123
	vcmpph k5, zmm2, WORD PTR [edx-256]{1to64}, 123
	vfmsubadd231ph zmm3, zmm2, WORD PTR [edx-256]{1to8}
	vfcmaddcph zmm3, zmm2, DWORD PTR [ecx]{1to8}
	vfcmulcph zmm3, zmm2, DWORD PTR [ecx]{1to32}
	vcvtdq2ph ymm3, DWORD PTR [ecx]{1to8}
	vfcmaddcph zmm3, zmm2, DWORD PTR [ecx]{1to8}
	vfmulcph zmm3, zmm2, DWORD PTR [edx-512]{1to32}
	vfmulcph zmm3, zmm2, DWORD PTR [edx-512]{1to4}
