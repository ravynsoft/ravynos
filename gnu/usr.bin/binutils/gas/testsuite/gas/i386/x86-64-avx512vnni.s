# Check 64bit AVX512VNNI instructions

	.allow_index_reg
	.text
_start:
	vpdpwssd	%zmm17, %zmm18, %zmm18	 # AVX512VNNI
	vpdpwssd	%zmm17, %zmm18, %zmm18{%k5}	 # AVX512VNNI
	vpdpwssd	%zmm17, %zmm18, %zmm18{%k5}{z}	 # AVX512VNNI
	vpdpwssd	0x123(%rax,%r14,8), %zmm18, %zmm18	 # AVX512VNNI
	vpdpwssd	8128(%rdx), %zmm18, %zmm18	 # AVX512VNNI Disp8
	vpdpwssd	508(%rdx){1to16}, %zmm18, %zmm18	 # AVX512VNNI Disp8

	vpdpwssds	%zmm17, %zmm21, %zmm21	 # AVX512VNNI
	vpdpwssds	%zmm17, %zmm21, %zmm21{%k4}	 # AVX512VNNI
	vpdpwssds	%zmm17, %zmm21, %zmm21{%k4}{z}	 # AVX512VNNI
	vpdpwssds	0x123(%rax,%r14,8), %zmm21, %zmm21	 # AVX512VNNI
	vpdpwssds	8128(%rdx), %zmm21, %zmm21	 # AVX512VNNI Disp8
	vpdpwssds	508(%rdx){1to16}, %zmm21, %zmm21	 # AVX512VNNI Disp8

	vpdpbusd	%zmm18, %zmm21, %zmm23	 # AVX512VNNI
	vpdpbusd	%zmm18, %zmm21, %zmm23{%k4}	 # AVX512VNNI
	vpdpbusd	%zmm18, %zmm21, %zmm23{%k4}{z}	 # AVX512VNNI
	vpdpbusd	0x123(%rax,%r14,8), %zmm21, %zmm23	 # AVX512VNNI
	vpdpbusd	8128(%rdx), %zmm21, %zmm23	 # AVX512VNNI Disp8
	vpdpbusd	508(%rdx){1to16}, %zmm21, %zmm23	 # AVX512VNNI Disp8

	vpdpbusds	%zmm25, %zmm24, %zmm24	 # AVX512VNNI
	vpdpbusds	%zmm25, %zmm24, %zmm24{%k7}	 # AVX512VNNI
	vpdpbusds	%zmm25, %zmm24, %zmm24{%k7}{z}	 # AVX512VNNI
	vpdpbusds	0x123(%rax,%r14,8), %zmm24, %zmm24	 # AVX512VNNI
	vpdpbusds	8128(%rdx), %zmm24, %zmm24	 # AVX512VNNI Disp8
	vpdpbusds	508(%rdx){1to16}, %zmm24, %zmm24	 # AVX512VNNI Disp8

	.intel_syntax noprefix
	vpdpwssd	zmm28, zmm27, zmm17	 # AVX512VNNI
	vpdpwssd	zmm28{k7}, zmm27, zmm17	 # AVX512VNNI
	vpdpwssd	zmm28{k7}{z}, zmm27, zmm17	 # AVX512VNNI
	vpdpwssd	zmm28, zmm27, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VNNI
	vpdpwssd	zmm28, zmm27, ZMMWORD PTR [rdx+8128]	 # AVX512VNNI Disp8
	vpdpwssd	zmm28, zmm27, [rdx+508]{1to16}	 # AVX512VNNI Disp8

	vpdpwssds	zmm29, zmm28, zmm17	 # AVX512VNNI
	vpdpwssds	zmm29{k3}, zmm28, zmm17	 # AVX512VNNI
	vpdpwssds	zmm29{k3}{z}, zmm28, zmm17	 # AVX512VNNI
	vpdpwssds	zmm29, zmm28, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VNNI
	vpdpwssds	zmm29, zmm28, ZMMWORD PTR [rdx+8128]	 # AVX512VNNI Disp8
	vpdpwssds	zmm29, zmm28, [rdx+508]{1to16}	 # AVX512VNNI Disp8

	vpdpbusd	zmm28, zmm24, zmm21	 # AVX512VNNI
	vpdpbusd	zmm28{k6}, zmm24, zmm21	 # AVX512VNNI
	vpdpbusd	zmm28{k6}{z}, zmm24, zmm21	 # AVX512VNNI
	vpdpbusd	zmm28, zmm24, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VNNI
	vpdpbusd	zmm28, zmm24, ZMMWORD PTR [rdx+8128]	 # AVX512VNNI Disp8
	vpdpbusd	zmm28, zmm24, [rdx+508]{1to16}	 # AVX512VNNI Disp8

	vpdpbusds	zmm20, zmm17, zmm20	 # AVX512VNNI
	vpdpbusds	zmm20{k2}, zmm17, zmm20	 # AVX512VNNI
	vpdpbusds	zmm20{k2}{z}, zmm17, zmm20	 # AVX512VNNI
	vpdpbusds	zmm20, zmm17, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VNNI
	vpdpbusds	zmm20, zmm17, ZMMWORD PTR [rdx+8128]	 # AVX512VNNI Disp8
	vpdpbusds	zmm20, zmm17, [rdx+508]{1to16}	 # AVX512VNNI Disp8
