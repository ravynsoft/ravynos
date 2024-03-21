# Check 32bit AVX512ER instructions

	.allow_index_reg
	.text
_start:

	vexp2ps	%zmm5, %zmm6	 # AVX512ER
	vexp2ps	{sae}, %zmm5, %zmm6	 # AVX512ER
	vexp2ps	(%ecx), %zmm6	 # AVX512ER
	vexp2ps	-123456(%esp,%esi,8), %zmm6	 # AVX512ER
	vexp2ps	(%eax){1to16}, %zmm6	 # AVX512ER
	vexp2ps	8128(%edx), %zmm6	 # AVX512ER Disp8
	vexp2ps	8192(%edx), %zmm6	 # AVX512ER
	vexp2ps	-8192(%edx), %zmm6	 # AVX512ER Disp8
	vexp2ps	-8256(%edx), %zmm6	 # AVX512ER
	vexp2ps	508(%edx){1to16}, %zmm6	 # AVX512ER Disp8
	vexp2ps	512(%edx){1to16}, %zmm6	 # AVX512ER
	vexp2ps	-512(%edx){1to16}, %zmm6	 # AVX512ER Disp8
	vexp2ps	-516(%edx){1to16}, %zmm6	 # AVX512ER

	vexp2pd	%zmm5, %zmm6	 # AVX512ER
	vexp2pd	{sae}, %zmm5, %zmm6	 # AVX512ER
	vexp2pd	(%ecx), %zmm6	 # AVX512ER
	vexp2pd	-123456(%esp,%esi,8), %zmm6	 # AVX512ER
	vexp2pd	(%eax){1to8}, %zmm6	 # AVX512ER
	vexp2pd	8128(%edx), %zmm6	 # AVX512ER Disp8
	vexp2pd	8192(%edx), %zmm6	 # AVX512ER
	vexp2pd	-8192(%edx), %zmm6	 # AVX512ER Disp8
	vexp2pd	-8256(%edx), %zmm6	 # AVX512ER
	vexp2pd	1016(%edx){1to8}, %zmm6	 # AVX512ER Disp8
	vexp2pd	1024(%edx){1to8}, %zmm6	 # AVX512ER
	vexp2pd	-1024(%edx){1to8}, %zmm6	 # AVX512ER Disp8
	vexp2pd	-1032(%edx){1to8}, %zmm6	 # AVX512ER

	vrcp28ps	%zmm5, %zmm6	 # AVX512ER
	vrcp28ps	%zmm5, %zmm6{%k7}	 # AVX512ER
	vrcp28ps	%zmm5, %zmm6{%k7}{z}	 # AVX512ER
	vrcp28ps	{sae}, %zmm5, %zmm6	 # AVX512ER
	vrcp28ps	(%ecx), %zmm6	 # AVX512ER
	vrcp28ps	-123456(%esp,%esi,8), %zmm6	 # AVX512ER
	vrcp28ps	(%eax){1to16}, %zmm6	 # AVX512ER
	vrcp28ps	8128(%edx), %zmm6	 # AVX512ER Disp8
	vrcp28ps	8192(%edx), %zmm6	 # AVX512ER
	vrcp28ps	-8192(%edx), %zmm6	 # AVX512ER Disp8
	vrcp28ps	-8256(%edx), %zmm6	 # AVX512ER
	vrcp28ps	508(%edx){1to16}, %zmm6	 # AVX512ER Disp8
	vrcp28ps	512(%edx){1to16}, %zmm6	 # AVX512ER
	vrcp28ps	-512(%edx){1to16}, %zmm6	 # AVX512ER Disp8
	vrcp28ps	-516(%edx){1to16}, %zmm6	 # AVX512ER

	vrcp28pd	%zmm5, %zmm6	 # AVX512ER
	vrcp28pd	%zmm5, %zmm6{%k7}	 # AVX512ER
	vrcp28pd	%zmm5, %zmm6{%k7}{z}	 # AVX512ER
	vrcp28pd	{sae}, %zmm5, %zmm6	 # AVX512ER
	vrcp28pd	(%ecx), %zmm6	 # AVX512ER
	vrcp28pd	-123456(%esp,%esi,8), %zmm6	 # AVX512ER
	vrcp28pd	(%eax){1to8}, %zmm6	 # AVX512ER
	vrcp28pd	8128(%edx), %zmm6	 # AVX512ER Disp8
	vrcp28pd	8192(%edx), %zmm6	 # AVX512ER
	vrcp28pd	-8192(%edx), %zmm6	 # AVX512ER Disp8
	vrcp28pd	-8256(%edx), %zmm6	 # AVX512ER
	vrcp28pd	1016(%edx){1to8}, %zmm6	 # AVX512ER Disp8
	vrcp28pd	1024(%edx){1to8}, %zmm6	 # AVX512ER
	vrcp28pd	-1024(%edx){1to8}, %zmm6	 # AVX512ER Disp8
	vrcp28pd	-1032(%edx){1to8}, %zmm6	 # AVX512ER

	vrcp28ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512ER
	vrcp28ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512ER
	vrcp28ss	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512ER
	vrcp28ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512ER
	vrcp28ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512ER
	vrcp28ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER Disp8
	vrcp28ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER
	vrcp28ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER Disp8
	vrcp28ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER

	vrcp28sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512ER
	vrcp28sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512ER
	vrcp28sd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512ER
	vrcp28sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512ER
	vrcp28sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512ER
	vrcp28sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER Disp8
	vrcp28sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER
	vrcp28sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER Disp8
	vrcp28sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER

	vrsqrt28ps	%zmm5, %zmm6	 # AVX512ER
	vrsqrt28ps	%zmm5, %zmm6{%k7}	 # AVX512ER
	vrsqrt28ps	%zmm5, %zmm6{%k7}{z}	 # AVX512ER
	vrsqrt28ps	{sae}, %zmm5, %zmm6	 # AVX512ER
	vrsqrt28ps	(%ecx), %zmm6	 # AVX512ER
	vrsqrt28ps	-123456(%esp,%esi,8), %zmm6	 # AVX512ER
	vrsqrt28ps	(%eax){1to16}, %zmm6	 # AVX512ER
	vrsqrt28ps	8128(%edx), %zmm6	 # AVX512ER Disp8
	vrsqrt28ps	8192(%edx), %zmm6	 # AVX512ER
	vrsqrt28ps	-8192(%edx), %zmm6	 # AVX512ER Disp8
	vrsqrt28ps	-8256(%edx), %zmm6	 # AVX512ER
	vrsqrt28ps	508(%edx){1to16}, %zmm6	 # AVX512ER Disp8
	vrsqrt28ps	512(%edx){1to16}, %zmm6	 # AVX512ER
	vrsqrt28ps	-512(%edx){1to16}, %zmm6	 # AVX512ER Disp8
	vrsqrt28ps	-516(%edx){1to16}, %zmm6	 # AVX512ER

	vrsqrt28pd	%zmm5, %zmm6	 # AVX512ER
	vrsqrt28pd	%zmm5, %zmm6{%k7}	 # AVX512ER
	vrsqrt28pd	%zmm5, %zmm6{%k7}{z}	 # AVX512ER
	vrsqrt28pd	{sae}, %zmm5, %zmm6	 # AVX512ER
	vrsqrt28pd	(%ecx), %zmm6	 # AVX512ER
	vrsqrt28pd	-123456(%esp,%esi,8), %zmm6	 # AVX512ER
	vrsqrt28pd	(%eax){1to8}, %zmm6	 # AVX512ER
	vrsqrt28pd	8128(%edx), %zmm6	 # AVX512ER Disp8
	vrsqrt28pd	8192(%edx), %zmm6	 # AVX512ER
	vrsqrt28pd	-8192(%edx), %zmm6	 # AVX512ER Disp8
	vrsqrt28pd	-8256(%edx), %zmm6	 # AVX512ER
	vrsqrt28pd	1016(%edx){1to8}, %zmm6	 # AVX512ER Disp8
	vrsqrt28pd	1024(%edx){1to8}, %zmm6	 # AVX512ER
	vrsqrt28pd	-1024(%edx){1to8}, %zmm6	 # AVX512ER Disp8
	vrsqrt28pd	-1032(%edx){1to8}, %zmm6	 # AVX512ER

	vrsqrt28ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512ER
	vrsqrt28ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512ER
	vrsqrt28ss	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512ER
	vrsqrt28ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512ER
	vrsqrt28ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512ER
	vrsqrt28ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER Disp8
	vrsqrt28ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER
	vrsqrt28ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER Disp8
	vrsqrt28ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER

	vrsqrt28sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512ER
	vrsqrt28sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512ER
	vrsqrt28sd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512ER
	vrsqrt28sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512ER
	vrsqrt28sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512ER
	vrsqrt28sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER Disp8
	vrsqrt28sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER
	vrsqrt28sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER Disp8
	vrsqrt28sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512ER

	.intel_syntax noprefix
	vexp2ps	zmm6, zmm5	 # AVX512ER
	vexp2ps	zmm6, zmm5{sae}	 # AVX512ER
	vexp2ps	zmm6, ZMMWORD PTR [ecx]	 # AVX512ER
	vexp2ps	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512ER
	vexp2ps	zmm6, [eax]{1to16}	 # AVX512ER
	vexp2ps	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512ER Disp8
	vexp2ps	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512ER
	vexp2ps	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512ER Disp8
	vexp2ps	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512ER
	vexp2ps	zmm6, [edx+508]{1to16}	 # AVX512ER Disp8
	vexp2ps	zmm6, [edx+512]{1to16}	 # AVX512ER
	vexp2ps	zmm6, [edx-512]{1to16}	 # AVX512ER Disp8
	vexp2ps	zmm6, [edx-516]{1to16}	 # AVX512ER

	vexp2pd	zmm6, zmm5	 # AVX512ER
	vexp2pd	zmm6, zmm5{sae}	 # AVX512ER
	vexp2pd	zmm6, ZMMWORD PTR [ecx]	 # AVX512ER
	vexp2pd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512ER
	vexp2pd	zmm6, [eax]{1to8}	 # AVX512ER
	vexp2pd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512ER Disp8
	vexp2pd	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512ER
	vexp2pd	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512ER Disp8
	vexp2pd	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512ER
	vexp2pd	zmm6, [edx+1016]{1to8}	 # AVX512ER Disp8
	vexp2pd	zmm6, [edx+1024]{1to8}	 # AVX512ER
	vexp2pd	zmm6, [edx-1024]{1to8}	 # AVX512ER Disp8
	vexp2pd	zmm6, [edx-1032]{1to8}	 # AVX512ER

	vrcp28ps	zmm6, zmm5	 # AVX512ER
	vrcp28ps	zmm6{k7}, zmm5	 # AVX512ER
	vrcp28ps	zmm6{k7}{z}, zmm5	 # AVX512ER
	vrcp28ps	zmm6, zmm5{sae}	 # AVX512ER
	vrcp28ps	zmm6, ZMMWORD PTR [ecx]	 # AVX512ER
	vrcp28ps	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512ER
	vrcp28ps	zmm6, [eax]{1to16}	 # AVX512ER
	vrcp28ps	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512ER Disp8
	vrcp28ps	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512ER
	vrcp28ps	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512ER Disp8
	vrcp28ps	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512ER
	vrcp28ps	zmm6, [edx+508]{1to16}	 # AVX512ER Disp8
	vrcp28ps	zmm6, [edx+512]{1to16}	 # AVX512ER
	vrcp28ps	zmm6, [edx-512]{1to16}	 # AVX512ER Disp8
	vrcp28ps	zmm6, [edx-516]{1to16}	 # AVX512ER

	vrcp28pd	zmm6, zmm5	 # AVX512ER
	vrcp28pd	zmm6{k7}, zmm5	 # AVX512ER
	vrcp28pd	zmm6{k7}{z}, zmm5	 # AVX512ER
	vrcp28pd	zmm6, zmm5{sae}	 # AVX512ER
	vrcp28pd	zmm6, ZMMWORD PTR [ecx]	 # AVX512ER
	vrcp28pd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512ER
	vrcp28pd	zmm6, [eax]{1to8}	 # AVX512ER
	vrcp28pd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512ER Disp8
	vrcp28pd	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512ER
	vrcp28pd	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512ER Disp8
	vrcp28pd	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512ER
	vrcp28pd	zmm6, [edx+1016]{1to8}	 # AVX512ER Disp8
	vrcp28pd	zmm6, [edx+1024]{1to8}	 # AVX512ER
	vrcp28pd	zmm6, [edx-1024]{1to8}	 # AVX512ER Disp8
	vrcp28pd	zmm6, [edx-1032]{1to8}	 # AVX512ER

	vrcp28ss	xmm6{k7}, xmm5, xmm4	 # AVX512ER
	vrcp28ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512ER
	vrcp28ss	xmm6{k7}, xmm5, xmm4{sae}	 # AVX512ER
	vrcp28ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512ER
	vrcp28ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512ER
	vrcp28ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512ER Disp8
	vrcp28ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512ER
	vrcp28ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512ER Disp8
	vrcp28ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512ER

	vrcp28sd	xmm6{k7}, xmm5, xmm4	 # AVX512ER
	vrcp28sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512ER
	vrcp28sd	xmm6{k7}, xmm5, xmm4{sae}	 # AVX512ER
	vrcp28sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512ER
	vrcp28sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512ER
	vrcp28sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512ER Disp8
	vrcp28sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512ER
	vrcp28sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512ER Disp8
	vrcp28sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512ER

	vrsqrt28ps	zmm6, zmm5	 # AVX512ER
	vrsqrt28ps	zmm6{k7}, zmm5	 # AVX512ER
	vrsqrt28ps	zmm6{k7}{z}, zmm5	 # AVX512ER
	vrsqrt28ps	zmm6, zmm5{sae}	 # AVX512ER
	vrsqrt28ps	zmm6, ZMMWORD PTR [ecx]	 # AVX512ER
	vrsqrt28ps	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512ER
	vrsqrt28ps	zmm6, [eax]{1to16}	 # AVX512ER
	vrsqrt28ps	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512ER Disp8
	vrsqrt28ps	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512ER
	vrsqrt28ps	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512ER Disp8
	vrsqrt28ps	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512ER
	vrsqrt28ps	zmm6, [edx+508]{1to16}	 # AVX512ER Disp8
	vrsqrt28ps	zmm6, [edx+512]{1to16}	 # AVX512ER
	vrsqrt28ps	zmm6, [edx-512]{1to16}	 # AVX512ER Disp8
	vrsqrt28ps	zmm6, [edx-516]{1to16}	 # AVX512ER

	vrsqrt28pd	zmm6, zmm5	 # AVX512ER
	vrsqrt28pd	zmm6{k7}, zmm5	 # AVX512ER
	vrsqrt28pd	zmm6{k7}{z}, zmm5	 # AVX512ER
	vrsqrt28pd	zmm6, zmm5{sae}	 # AVX512ER
	vrsqrt28pd	zmm6, ZMMWORD PTR [ecx]	 # AVX512ER
	vrsqrt28pd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512ER
	vrsqrt28pd	zmm6, [eax]{1to8}	 # AVX512ER
	vrsqrt28pd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512ER Disp8
	vrsqrt28pd	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512ER
	vrsqrt28pd	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512ER Disp8
	vrsqrt28pd	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512ER
	vrsqrt28pd	zmm6, [edx+1016]{1to8}	 # AVX512ER Disp8
	vrsqrt28pd	zmm6, [edx+1024]{1to8}	 # AVX512ER
	vrsqrt28pd	zmm6, [edx-1024]{1to8}	 # AVX512ER Disp8
	vrsqrt28pd	zmm6, [edx-1032]{1to8}	 # AVX512ER

	vrsqrt28ss	xmm6{k7}, xmm5, xmm4	 # AVX512ER
	vrsqrt28ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512ER
	vrsqrt28ss	xmm6{k7}, xmm5, xmm4{sae}	 # AVX512ER
	vrsqrt28ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512ER
	vrsqrt28ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512ER
	vrsqrt28ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512ER Disp8
	vrsqrt28ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512ER
	vrsqrt28ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512ER Disp8
	vrsqrt28ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512ER

	vrsqrt28sd	xmm6{k7}, xmm5, xmm4	 # AVX512ER
	vrsqrt28sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512ER
	vrsqrt28sd	xmm6{k7}, xmm5, xmm4{sae}	 # AVX512ER
	vrsqrt28sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512ER
	vrsqrt28sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512ER
	vrsqrt28sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512ER Disp8
	vrsqrt28sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512ER
	vrsqrt28sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512ER Disp8
	vrsqrt28sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512ER

