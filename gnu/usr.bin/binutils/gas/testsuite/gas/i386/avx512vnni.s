# Check 32bit AVX512VNNI instructions

	.allow_index_reg
	.text
_start:
	vpdpwssd	%zmm3, %zmm1, %zmm4	 # AVX512VNNI
	vpdpwssd	%zmm3, %zmm1, %zmm4{%k1}	 # AVX512VNNI
	vpdpwssd	%zmm3, %zmm1, %zmm4{%k1}{z}	 # AVX512VNNI
	vpdpwssd	-123456(%esp,%esi,8), %zmm1, %zmm4	 # AVX512VNNI
	vpdpwssd	8128(%edx), %zmm1, %zmm4	 # AVX512VNNI Disp8
	vpdpwssd	508(%edx){1to16}, %zmm1, %zmm4	 # AVX512VNNI Disp8

	vpdpwssds	%zmm4, %zmm5, %zmm2	 # AVX512VNNI
	vpdpwssds	%zmm4, %zmm5, %zmm2{%k6}	 # AVX512VNNI
	vpdpwssds	%zmm4, %zmm5, %zmm2{%k6}{z}	 # AVX512VNNI
	vpdpwssds	-123456(%esp,%esi,8), %zmm5, %zmm2	 # AVX512VNNI
	vpdpwssds	8128(%edx), %zmm5, %zmm2	 # AVX512VNNI Disp8
	vpdpwssds	508(%edx){1to16}, %zmm5, %zmm2	 # AVX512VNNI Disp8

	vpdpbusd	%zmm3, %zmm2, %zmm5	 # AVX512VNNI
	vpdpbusd	%zmm3, %zmm2, %zmm5{%k1}	 # AVX512VNNI
	vpdpbusd	%zmm3, %zmm2, %zmm5{%k1}{z}	 # AVX512VNNI
	vpdpbusd	-123456(%esp,%esi,8), %zmm2, %zmm5	 # AVX512VNNI
	vpdpbusd	8128(%edx), %zmm2, %zmm5	 # AVX512VNNI Disp8
	vpdpbusd	508(%edx){1to16}, %zmm2, %zmm5	 # AVX512VNNI Disp8

	vpdpbusds	%zmm1, %zmm3, %zmm5	 # AVX512VNNI
	vpdpbusds	%zmm1, %zmm3, %zmm5{%k2}	 # AVX512VNNI
	vpdpbusds	%zmm1, %zmm3, %zmm5{%k2}{z}	 # AVX512VNNI
	vpdpbusds	-123456(%esp,%esi,8), %zmm3, %zmm5	 # AVX512VNNI
	vpdpbusds	8128(%edx), %zmm3, %zmm5	 # AVX512VNNI Disp8
	vpdpbusds	508(%edx){1to16}, %zmm3, %zmm5	 # AVX512VNNI Disp8

	.intel_syntax noprefix
	vpdpwssd	zmm3, zmm4, zmm1	 # AVX512VNNI
	vpdpwssd	zmm3{k3}, zmm4, zmm1	 # AVX512VNNI
	vpdpwssd	zmm3{k3}{z}, zmm4, zmm1	 # AVX512VNNI
	vpdpwssd	zmm3, zmm4, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VNNI
	vpdpwssd	zmm3, zmm4, ZMMWORD PTR [edx+8128]	 # AVX512VNNI Disp8
	vpdpwssd	zmm3, zmm4, [edx+508]{1to16}	 # AVX512VNNI Disp8

	vpdpwssds	zmm3, zmm1, zmm2	 # AVX512VNNI
	vpdpwssds	zmm3{k7}, zmm1, zmm2	 # AVX512VNNI
	vpdpwssds	zmm3{k7}{z}, zmm1, zmm2	 # AVX512VNNI
	vpdpwssds	zmm3, zmm1, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VNNI
	vpdpwssds	zmm3, zmm1, ZMMWORD PTR [edx+8128]	 # AVX512VNNI Disp8
	vpdpwssds	zmm3, zmm1, [edx+508]{1to16}	 # AVX512VNNI Disp8

	vpdpbusd	zmm3, zmm4, zmm1	 # AVX512VNNI
	vpdpbusd	zmm3{k6}, zmm4, zmm1	 # AVX512VNNI
	vpdpbusd	zmm3{k6}{z}, zmm4, zmm1	 # AVX512VNNI
	vpdpbusd	zmm3, zmm4, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VNNI
	vpdpbusd	zmm3, zmm4, ZMMWORD PTR [edx+8128]	 # AVX512VNNI Disp8
	vpdpbusd	zmm3, zmm4, [edx+508]{1to16}	 # AVX512VNNI Disp8
	vpdpbusds	zmm1, zmm1, zmm1	 # AVX512VNNI
	vpdpbusds	zmm1{k1}, zmm1, zmm1	 # AVX512VNNI
	vpdpbusds	zmm1{k1}{z}, zmm1, zmm1	 # AVX512VNNI
	vpdpbusds	zmm1, zmm1, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VNNI
	vpdpbusds	zmm1, zmm1, ZMMWORD PTR [edx+8128]	 # AVX512VNNI Disp8
	vpdpbusds	zmm1, zmm1, [edx+508]{1to16}	 # AVX512VNNI Disp8
