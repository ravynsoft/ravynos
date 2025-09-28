# Check error for invalid {1toXX} and {2toXX} broadcasts.

	.allow_index_reg
	.text
_start:
	vcvtpd2ph (%ecx){1to16}, %xmm30
	vcvtuqq2ph -1024(%edx){1to32}, %xmm30
	vcvtdq2ph (%ecx){1to8}, %ymm30
	vcvtudq2ph -512(%edx){1to32}, %ymm30
	vcmpph  $123, (%ecx){1to16}, %zmm29, %k5
	vcmpph  $123, (%ecx){1to64}, %zmm29, %k5
	vfmadd132ph (%ecx){1to8}, %zmm29, %zmm3
	vfcmaddcph (%ecx){1to8}, %zmm29, %zmm3
	vfcmulcph (%ecx){1to32}, %zmm29, %zmm3
	vcvtdq2ph (%ecx){1to8}, %ymm30
	vfmaddcph (%ecx){1to8}, %zmm29, %zmm3
	vfmulcph -512(%edx){1to32}, %zmm29, %zmm3
	vfmulcph -512(%edx){1to4}, %zmm29, %zmm3

	.intel_syntax noprefix
	vcvtpd2ph xmm30, QWORD PTR [ecx]{1to16}
	vcvtuqq2ph xmm30, QWORD PTR [edx-1024]{1to32}
	vcvtdq2ph ymm30, DWORD PTR [ecx]{1to8}
	vcvtudq2ph ymm30, DWORD PTR [edx-512]{1to32}
	vcmpph k5, zmm29, WORD PTR [edx-256]{1to16}, 123
	vcmpph k5, zmm29, WORD PTR [edx-256]{1to64}, 123
	vfmsubadd231ph zmm30, zmm2, WORD PTR [edx-256]{1to8}
	vfcmaddcph zmm3, zmm29, DWORD PTR [ecx]{1to8}
	vfcmulcph zmm3, zmm29, DWORD PTR [ecx]{1to32}
	vcvtdq2ph ymm30, DWORD PTR [ecx]{1to8}
	vfcmaddcph zmm30, zmm2, DWORD PTR [ecx]{1to8}
	vfmulcph zmm30, zmm2, DWORD PTR [edx-512]{1to32}
	vfmulcph zmm30, zmm2, DWORD PTR [edx-512]{1to4}
