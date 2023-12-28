# Check 32bit AVX512{VNNI,VL} instructions

	.allow_index_reg
	.text
_start:
	vpdpwssd	%xmm2, %xmm4, %xmm2{%k3}	 # AVX512{VNNI,VL}
	vpdpwssd	%xmm2, %xmm4, %xmm2{%k3}{z}	 # AVX512{VNNI,VL}
	vpdpwssd	-123456(%esp,%esi,8), %xmm4, %xmm2{%k1}	 # AVX512{VNNI,VL}
	vpdpwssd	2032(%edx), %xmm4, %xmm2{%k1}	 # AVX512{VNNI,VL} Disp8
	vpdpwssd	508(%edx){1to4}, %xmm4, %xmm2{%k1}	 # AVX512{VNNI,VL} Disp8
	vpdpwssd	%ymm1, %ymm3, %ymm3{%k1}	 # AVX512{VNNI,VL}
	vpdpwssd	%ymm1, %ymm3, %ymm3{%k1}{z}	 # AVX512{VNNI,VL}
	vpdpwssd	-123456(%esp,%esi,8), %ymm3, %ymm3{%k4}	 # AVX512{VNNI,VL}
	vpdpwssd	4064(%edx), %ymm3, %ymm3{%k4}	 # AVX512{VNNI,VL} Disp8
	vpdpwssd	508(%edx){1to8}, %ymm3, %ymm3{%k4}	 # AVX512{VNNI,VL} Disp8

	vpdpwssds	%xmm1, %xmm4, %xmm2{%k1}	 # AVX512{VNNI,VL}
	vpdpwssds	%xmm1, %xmm4, %xmm2{%k1}{z}	 # AVX512{VNNI,VL}
	vpdpwssds	-123456(%esp,%esi,8), %xmm4, %xmm2{%k4}	 # AVX512{VNNI,VL}
	vpdpwssds	2032(%edx), %xmm4, %xmm2{%k4}	 # AVX512{VNNI,VL} Disp8
	vpdpwssds	508(%edx){1to4}, %xmm4, %xmm2{%k4}	 # AVX512{VNNI,VL} Disp8
	vpdpwssds	%ymm4, %ymm1, %ymm4{%k7}	 # AVX512{VNNI,VL}
	vpdpwssds	%ymm4, %ymm1, %ymm4{%k7}{z}	 # AVX512{VNNI,VL}
	vpdpwssds	-123456(%esp,%esi,8), %ymm1, %ymm4{%k3}	 # AVX512{VNNI,VL}
	vpdpwssds	4064(%edx), %ymm1, %ymm4{%k3}	 # AVX512{VNNI,VL} Disp8
	vpdpwssds	508(%edx){1to8}, %ymm1, %ymm4{%k3}	 # AVX512{VNNI,VL} Disp8

	vpdpbusd	%xmm1, %xmm3, %xmm2{%k4}	 # AVX512{VNNI,VL}
	vpdpbusd	%xmm1, %xmm3, %xmm2{%k4}{z}	 # AVX512{VNNI,VL}
	vpdpbusd	-123456(%esp,%esi,8), %xmm3, %xmm2{%k2}	 # AVX512{VNNI,VL}
	vpdpbusd	2032(%edx), %xmm3, %xmm2{%k2}	 # AVX512{VNNI,VL} Disp8
	vpdpbusd	508(%edx){1to4}, %xmm3, %xmm2{%k2}	 # AVX512{VNNI,VL} Disp8
	vpdpbusd	%ymm2, %ymm2, %ymm2{%k5}	 # AVX512{VNNI,VL}
	vpdpbusd	%ymm2, %ymm2, %ymm2{%k5}{z}	 # AVX512{VNNI,VL}
	vpdpbusd	-123456(%esp,%esi,8), %ymm2, %ymm2{%k7}	 # AVX512{VNNI,VL}
	vpdpbusd	4064(%edx), %ymm2, %ymm2{%k7}	 # AVX512{VNNI,VL} Disp8
	vpdpbusd	508(%edx){1to8}, %ymm2, %ymm2{%k7}	 # AVX512{VNNI,VL} Disp8

	vpdpbusds	%xmm4, %xmm2, %xmm6{%k6}	 # AVX512{VNNI,VL}
	vpdpbusds	%xmm4, %xmm2, %xmm6{%k6}{z}	 # AVX512{VNNI,VL}
	vpdpbusds	-123456(%esp,%esi,8), %xmm2, %xmm6{%k4}	 # AVX512{VNNI,VL}
	vpdpbusds	2032(%edx), %xmm2, %xmm6{%k4}	 # AVX512{VNNI,VL} Disp8
	vpdpbusds	508(%edx){1to4}, %xmm2, %xmm6{%k4}	 # AVX512{VNNI,VL} Disp8
	vpdpbusds	%ymm1, %ymm3, %ymm4{%k7}	 # AVX512{VNNI,VL}
	vpdpbusds	%ymm1, %ymm3, %ymm4{%k7}{z}	 # AVX512{VNNI,VL}
	vpdpbusds	-123456(%esp,%esi,8), %ymm3, %ymm4{%k1}	 # AVX512{VNNI,VL}
	vpdpbusds	4064(%edx), %ymm3, %ymm4{%k1}	 # AVX512{VNNI,VL} Disp8
	vpdpbusds	508(%edx){1to8}, %ymm3, %ymm4{%k1}	 # AVX512{VNNI,VL} Disp8

	.intel_syntax noprefix
	vpdpwssd	xmm5{k1}, xmm2, xmm2	 # AVX512{VNNI,VL}
	vpdpwssd	xmm5{k1}{z}, xmm2, xmm2	 # AVX512{VNNI,VL}
	vpdpwssd	xmm5{k6}, xmm2, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VNNI,VL}
	vpdpwssd	xmm5{k6}, xmm2, XMMWORD PTR [edx+2032]	 # AVX512{VNNI,VL} Disp8
	vpdpwssd	xmm5{k6}, xmm2, [edx+508]{1to4}	 # AVX512{VNNI,VL} Disp8
	vpdpwssd	ymm1{k7}, ymm2, ymm4	 # AVX512{VNNI,VL}
	vpdpwssd	ymm1{k7}{z}, ymm2, ymm4	 # AVX512{VNNI,VL}
	vpdpwssd	ymm1{k6}, ymm2, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VNNI,VL}
	vpdpwssd	ymm1{k6}, ymm2, YMMWORD PTR [edx+4064]	 # AVX512{VNNI,VL} Disp8
	vpdpwssd	ymm1{k6}, ymm2, [edx+508]{1to8}	 # AVX512{VNNI,VL} Disp8

	vpdpwssds	xmm1{k2}, xmm4, xmm1	 # AVX512{VNNI,VL}
	vpdpwssds	xmm1{k2}{z}, xmm4, xmm1	 # AVX512{VNNI,VL}
	vpdpwssds	xmm1{k6}, xmm4, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VNNI,VL}
	vpdpwssds	xmm1{k6}, xmm4, XMMWORD PTR [edx+2032]	 # AVX512{VNNI,VL} Disp8
	vpdpwssds	xmm1{k6}, xmm4, [edx+508]{1to4}	 # AVX512{VNNI,VL} Disp8
	vpdpwssds	ymm3{k4}, ymm2, ymm4	 # AVX512{VNNI,VL}
	vpdpwssds	ymm3{k4}{z}, ymm2, ymm4	 # AVX512{VNNI,VL}
	vpdpwssds	ymm3{k5}, ymm2, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VNNI,VL}
	vpdpwssds	ymm3{k5}, ymm2, YMMWORD PTR [edx+4064]	 # AVX512{VNNI,VL} Disp8
	vpdpwssds	ymm3{k5}, ymm2, [edx+508]{1to8}	 # AVX512{VNNI,VL} Disp8

	vpdpbusd	xmm3{k7}, xmm4, xmm4	 # AVX512{VNNI,VL}
	vpdpbusd	xmm3{k7}{z}, xmm4, xmm4	 # AVX512{VNNI,VL}
	vpdpbusd	xmm3{k1}, xmm4, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VNNI,VL}
	vpdpbusd	xmm3{k1}, xmm4, XMMWORD PTR [edx+2032]	 # AVX512{VNNI,VL} Disp8
	vpdpbusd	xmm3{k1}, xmm4, [edx+508]{1to4}	 # AVX512{VNNI,VL} Disp8
	vpdpbusd	ymm6{k5}, ymm2, ymm4	 # AVX512{VNNI,VL}
	vpdpbusd	ymm6{k5}{z}, ymm2, ymm4	 # AVX512{VNNI,VL}
	vpdpbusd	ymm6{k5}, ymm2, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VNNI,VL}
	vpdpbusd	ymm6{k5}, ymm2, YMMWORD PTR [edx+4064]	 # AVX512{VNNI,VL} Disp8
	vpdpbusd	ymm6{k5}, ymm2, [edx+508]{1to8}	 # AVX512{VNNI,VL} Disp8

	vpdpbusds	xmm3{k5}, xmm3, xmm4	 # AVX512{VNNI,VL}
	vpdpbusds	xmm3{k5}{z}, xmm3, xmm4	 # AVX512{VNNI,VL}
	vpdpbusds	xmm3{k4}, xmm3, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VNNI,VL}
	vpdpbusds	xmm3{k4}, xmm3, XMMWORD PTR [edx+2032]	 # AVX512{VNNI,VL} Disp8
	vpdpbusds	xmm3{k4}, xmm3, [edx+508]{1to4}	 # AVX512{VNNI,VL} Disp8
	vpdpbusds	ymm2{k4}, ymm3, ymm4	 # AVX512{VNNI,VL}
	vpdpbusds	ymm2{k4}{z}, ymm3, ymm4	 # AVX512{VNNI,VL}
	vpdpbusds	ymm2{k1}, ymm3, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VNNI,VL}
	vpdpbusds	ymm2{k1}, ymm3, YMMWORD PTR [edx+4064]	 # AVX512{VNNI,VL} Disp8
	vpdpbusds	ymm2{k1}, ymm3, [edx+508]{1to8}	 # AVX512{VNNI,VL} Disp8
