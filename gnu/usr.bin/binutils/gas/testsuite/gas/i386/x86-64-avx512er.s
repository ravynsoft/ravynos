# Check 64bit AVX512ER instructions

	.allow_index_reg
	.text
_start:

	vexp2ps	%zmm29, %zmm30	 # AVX512ER
	vexp2ps	{sae}, %zmm29, %zmm30	 # AVX512ER
	vexp2ps	(%rcx), %zmm30	 # AVX512ER
	vexp2ps	0x123(%rax,%r14,8), %zmm30	 # AVX512ER
	vexp2ps	(%rcx){1to16}, %zmm30	 # AVX512ER
	vexp2ps	8128(%rdx), %zmm30	 # AVX512ER Disp8
	vexp2ps	8192(%rdx), %zmm30	 # AVX512ER
	vexp2ps	-8192(%rdx), %zmm30	 # AVX512ER Disp8
	vexp2ps	-8256(%rdx), %zmm30	 # AVX512ER
	vexp2ps	508(%rdx){1to16}, %zmm30	 # AVX512ER Disp8
	vexp2ps	512(%rdx){1to16}, %zmm30	 # AVX512ER
	vexp2ps	-512(%rdx){1to16}, %zmm30	 # AVX512ER Disp8
	vexp2ps	-516(%rdx){1to16}, %zmm30	 # AVX512ER

	vexp2pd	%zmm29, %zmm30	 # AVX512ER
	vexp2pd	{sae}, %zmm29, %zmm30	 # AVX512ER
	vexp2pd	(%rcx), %zmm30	 # AVX512ER
	vexp2pd	0x123(%rax,%r14,8), %zmm30	 # AVX512ER
	vexp2pd	(%rcx){1to8}, %zmm30	 # AVX512ER
	vexp2pd	8128(%rdx), %zmm30	 # AVX512ER Disp8
	vexp2pd	8192(%rdx), %zmm30	 # AVX512ER
	vexp2pd	-8192(%rdx), %zmm30	 # AVX512ER Disp8
	vexp2pd	-8256(%rdx), %zmm30	 # AVX512ER
	vexp2pd	1016(%rdx){1to8}, %zmm30	 # AVX512ER Disp8
	vexp2pd	1024(%rdx){1to8}, %zmm30	 # AVX512ER
	vexp2pd	-1024(%rdx){1to8}, %zmm30	 # AVX512ER Disp8
	vexp2pd	-1032(%rdx){1to8}, %zmm30	 # AVX512ER

	vrcp28ps	%zmm29, %zmm30	 # AVX512ER
	vrcp28ps	%zmm29, %zmm30{%k7}	 # AVX512ER
	vrcp28ps	%zmm29, %zmm30{%k7}{z}	 # AVX512ER
	vrcp28ps	{sae}, %zmm29, %zmm30	 # AVX512ER
	vrcp28ps	(%rcx), %zmm30	 # AVX512ER
	vrcp28ps	0x123(%rax,%r14,8), %zmm30	 # AVX512ER
	vrcp28ps	(%rcx){1to16}, %zmm30	 # AVX512ER
	vrcp28ps	8128(%rdx), %zmm30	 # AVX512ER Disp8
	vrcp28ps	8192(%rdx), %zmm30	 # AVX512ER
	vrcp28ps	-8192(%rdx), %zmm30	 # AVX512ER Disp8
	vrcp28ps	-8256(%rdx), %zmm30	 # AVX512ER
	vrcp28ps	508(%rdx){1to16}, %zmm30	 # AVX512ER Disp8
	vrcp28ps	512(%rdx){1to16}, %zmm30	 # AVX512ER
	vrcp28ps	-512(%rdx){1to16}, %zmm30	 # AVX512ER Disp8
	vrcp28ps	-516(%rdx){1to16}, %zmm30	 # AVX512ER

	vrcp28pd	%zmm29, %zmm30	 # AVX512ER
	vrcp28pd	%zmm29, %zmm30{%k7}	 # AVX512ER
	vrcp28pd	%zmm29, %zmm30{%k7}{z}	 # AVX512ER
	vrcp28pd	{sae}, %zmm29, %zmm30	 # AVX512ER
	vrcp28pd	(%rcx), %zmm30	 # AVX512ER
	vrcp28pd	0x123(%rax,%r14,8), %zmm30	 # AVX512ER
	vrcp28pd	(%rcx){1to8}, %zmm30	 # AVX512ER
	vrcp28pd	8128(%rdx), %zmm30	 # AVX512ER Disp8
	vrcp28pd	8192(%rdx), %zmm30	 # AVX512ER
	vrcp28pd	-8192(%rdx), %zmm30	 # AVX512ER Disp8
	vrcp28pd	-8256(%rdx), %zmm30	 # AVX512ER
	vrcp28pd	1016(%rdx){1to8}, %zmm30	 # AVX512ER Disp8
	vrcp28pd	1024(%rdx){1to8}, %zmm30	 # AVX512ER
	vrcp28pd	-1024(%rdx){1to8}, %zmm30	 # AVX512ER Disp8
	vrcp28pd	-1032(%rdx){1to8}, %zmm30	 # AVX512ER

	vrcp28ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512ER
	vrcp28ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512ER
	vrcp28ss	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512ER
	vrcp28ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512ER
	vrcp28ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512ER
	vrcp28ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER Disp8
	vrcp28ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER
	vrcp28ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER Disp8
	vrcp28ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER

	vrcp28sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512ER
	vrcp28sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512ER
	vrcp28sd	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512ER
	vrcp28sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512ER
	vrcp28sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512ER
	vrcp28sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER Disp8
	vrcp28sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER
	vrcp28sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER Disp8
	vrcp28sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER

	vrsqrt28ps	%zmm29, %zmm30	 # AVX512ER
	vrsqrt28ps	%zmm29, %zmm30{%k7}	 # AVX512ER
	vrsqrt28ps	%zmm29, %zmm30{%k7}{z}	 # AVX512ER
	vrsqrt28ps	{sae}, %zmm29, %zmm30	 # AVX512ER
	vrsqrt28ps	(%rcx), %zmm30	 # AVX512ER
	vrsqrt28ps	0x123(%rax,%r14,8), %zmm30	 # AVX512ER
	vrsqrt28ps	(%rcx){1to16}, %zmm30	 # AVX512ER
	vrsqrt28ps	8128(%rdx), %zmm30	 # AVX512ER Disp8
	vrsqrt28ps	8192(%rdx), %zmm30	 # AVX512ER
	vrsqrt28ps	-8192(%rdx), %zmm30	 # AVX512ER Disp8
	vrsqrt28ps	-8256(%rdx), %zmm30	 # AVX512ER
	vrsqrt28ps	508(%rdx){1to16}, %zmm30	 # AVX512ER Disp8
	vrsqrt28ps	512(%rdx){1to16}, %zmm30	 # AVX512ER
	vrsqrt28ps	-512(%rdx){1to16}, %zmm30	 # AVX512ER Disp8
	vrsqrt28ps	-516(%rdx){1to16}, %zmm30	 # AVX512ER

	vrsqrt28pd	%zmm29, %zmm30	 # AVX512ER
	vrsqrt28pd	%zmm29, %zmm30{%k7}	 # AVX512ER
	vrsqrt28pd	%zmm29, %zmm30{%k7}{z}	 # AVX512ER
	vrsqrt28pd	{sae}, %zmm29, %zmm30	 # AVX512ER
	vrsqrt28pd	(%rcx), %zmm30	 # AVX512ER
	vrsqrt28pd	0x123(%rax,%r14,8), %zmm30	 # AVX512ER
	vrsqrt28pd	(%rcx){1to8}, %zmm30	 # AVX512ER
	vrsqrt28pd	8128(%rdx), %zmm30	 # AVX512ER Disp8
	vrsqrt28pd	8192(%rdx), %zmm30	 # AVX512ER
	vrsqrt28pd	-8192(%rdx), %zmm30	 # AVX512ER Disp8
	vrsqrt28pd	-8256(%rdx), %zmm30	 # AVX512ER
	vrsqrt28pd	1016(%rdx){1to8}, %zmm30	 # AVX512ER Disp8
	vrsqrt28pd	1024(%rdx){1to8}, %zmm30	 # AVX512ER
	vrsqrt28pd	-1024(%rdx){1to8}, %zmm30	 # AVX512ER Disp8
	vrsqrt28pd	-1032(%rdx){1to8}, %zmm30	 # AVX512ER

	vrsqrt28ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512ER
	vrsqrt28ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512ER
	vrsqrt28ss	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512ER
	vrsqrt28ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512ER
	vrsqrt28ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512ER
	vrsqrt28ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER Disp8
	vrsqrt28ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER
	vrsqrt28ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER Disp8
	vrsqrt28ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER

	vrsqrt28sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512ER
	vrsqrt28sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512ER
	vrsqrt28sd	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512ER
	vrsqrt28sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512ER
	vrsqrt28sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512ER
	vrsqrt28sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER Disp8
	vrsqrt28sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER
	vrsqrt28sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER Disp8
	vrsqrt28sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512ER

	.intel_syntax noprefix
	vexp2ps	zmm30, zmm29	 # AVX512ER
	vexp2ps	zmm30, zmm29{sae}	 # AVX512ER
	vexp2ps	zmm30, ZMMWORD PTR [rcx]	 # AVX512ER
	vexp2ps	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512ER
	vexp2ps	zmm30, [rcx]{1to16}	 # AVX512ER
	vexp2ps	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512ER Disp8
	vexp2ps	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512ER
	vexp2ps	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512ER Disp8
	vexp2ps	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512ER
	vexp2ps	zmm30, [rdx+508]{1to16}	 # AVX512ER Disp8
	vexp2ps	zmm30, [rdx+512]{1to16}	 # AVX512ER
	vexp2ps	zmm30, [rdx-512]{1to16}	 # AVX512ER Disp8
	vexp2ps	zmm30, [rdx-516]{1to16}	 # AVX512ER

	vexp2pd	zmm30, zmm29	 # AVX512ER
	vexp2pd	zmm30, zmm29{sae}	 # AVX512ER
	vexp2pd	zmm30, ZMMWORD PTR [rcx]	 # AVX512ER
	vexp2pd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512ER
	vexp2pd	zmm30, [rcx]{1to8}	 # AVX512ER
	vexp2pd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512ER Disp8
	vexp2pd	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512ER
	vexp2pd	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512ER Disp8
	vexp2pd	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512ER
	vexp2pd	zmm30, [rdx+1016]{1to8}	 # AVX512ER Disp8
	vexp2pd	zmm30, [rdx+1024]{1to8}	 # AVX512ER
	vexp2pd	zmm30, [rdx-1024]{1to8}	 # AVX512ER Disp8
	vexp2pd	zmm30, [rdx-1032]{1to8}	 # AVX512ER

	vrcp28ps	zmm30, zmm29	 # AVX512ER
	vrcp28ps	zmm30{k7}, zmm29	 # AVX512ER
	vrcp28ps	zmm30{k7}{z}, zmm29	 # AVX512ER
	vrcp28ps	zmm30, zmm29{sae}	 # AVX512ER
	vrcp28ps	zmm30, ZMMWORD PTR [rcx]	 # AVX512ER
	vrcp28ps	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512ER
	vrcp28ps	zmm30, [rcx]{1to16}	 # AVX512ER
	vrcp28ps	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512ER Disp8
	vrcp28ps	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512ER
	vrcp28ps	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512ER Disp8
	vrcp28ps	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512ER
	vrcp28ps	zmm30, [rdx+508]{1to16}	 # AVX512ER Disp8
	vrcp28ps	zmm30, [rdx+512]{1to16}	 # AVX512ER
	vrcp28ps	zmm30, [rdx-512]{1to16}	 # AVX512ER Disp8
	vrcp28ps	zmm30, [rdx-516]{1to16}	 # AVX512ER

	vrcp28pd	zmm30, zmm29	 # AVX512ER
	vrcp28pd	zmm30{k7}, zmm29	 # AVX512ER
	vrcp28pd	zmm30{k7}{z}, zmm29	 # AVX512ER
	vrcp28pd	zmm30, zmm29{sae}	 # AVX512ER
	vrcp28pd	zmm30, ZMMWORD PTR [rcx]	 # AVX512ER
	vrcp28pd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512ER
	vrcp28pd	zmm30, [rcx]{1to8}	 # AVX512ER
	vrcp28pd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512ER Disp8
	vrcp28pd	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512ER
	vrcp28pd	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512ER Disp8
	vrcp28pd	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512ER
	vrcp28pd	zmm30, [rdx+1016]{1to8}	 # AVX512ER Disp8
	vrcp28pd	zmm30, [rdx+1024]{1to8}	 # AVX512ER
	vrcp28pd	zmm30, [rdx-1024]{1to8}	 # AVX512ER Disp8
	vrcp28pd	zmm30, [rdx-1032]{1to8}	 # AVX512ER

	vrcp28ss	xmm30{k7}, xmm29, xmm28	 # AVX512ER
	vrcp28ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512ER
	vrcp28ss	xmm30{k7}, xmm29, xmm28{sae}	 # AVX512ER
	vrcp28ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512ER
	vrcp28ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512ER
	vrcp28ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512ER Disp8
	vrcp28ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512ER
	vrcp28ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512ER Disp8
	vrcp28ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512ER

	vrcp28sd	xmm30{k7}, xmm29, xmm28	 # AVX512ER
	vrcp28sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512ER
	vrcp28sd	xmm30{k7}, xmm29, xmm28{sae}	 # AVX512ER
	vrcp28sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512ER
	vrcp28sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512ER
	vrcp28sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512ER Disp8
	vrcp28sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512ER
	vrcp28sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512ER Disp8
	vrcp28sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512ER

	vrsqrt28ps	zmm30, zmm29	 # AVX512ER
	vrsqrt28ps	zmm30{k7}, zmm29	 # AVX512ER
	vrsqrt28ps	zmm30{k7}{z}, zmm29	 # AVX512ER
	vrsqrt28ps	zmm30, zmm29{sae}	 # AVX512ER
	vrsqrt28ps	zmm30, ZMMWORD PTR [rcx]	 # AVX512ER
	vrsqrt28ps	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512ER
	vrsqrt28ps	zmm30, [rcx]{1to16}	 # AVX512ER
	vrsqrt28ps	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512ER Disp8
	vrsqrt28ps	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512ER
	vrsqrt28ps	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512ER Disp8
	vrsqrt28ps	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512ER
	vrsqrt28ps	zmm30, [rdx+508]{1to16}	 # AVX512ER Disp8
	vrsqrt28ps	zmm30, [rdx+512]{1to16}	 # AVX512ER
	vrsqrt28ps	zmm30, [rdx-512]{1to16}	 # AVX512ER Disp8
	vrsqrt28ps	zmm30, [rdx-516]{1to16}	 # AVX512ER

	vrsqrt28pd	zmm30, zmm29	 # AVX512ER
	vrsqrt28pd	zmm30{k7}, zmm29	 # AVX512ER
	vrsqrt28pd	zmm30{k7}{z}, zmm29	 # AVX512ER
	vrsqrt28pd	zmm30, zmm29{sae}	 # AVX512ER
	vrsqrt28pd	zmm30, ZMMWORD PTR [rcx]	 # AVX512ER
	vrsqrt28pd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512ER
	vrsqrt28pd	zmm30, [rcx]{1to8}	 # AVX512ER
	vrsqrt28pd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512ER Disp8
	vrsqrt28pd	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512ER
	vrsqrt28pd	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512ER Disp8
	vrsqrt28pd	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512ER
	vrsqrt28pd	zmm30, [rdx+1016]{1to8}	 # AVX512ER Disp8
	vrsqrt28pd	zmm30, [rdx+1024]{1to8}	 # AVX512ER
	vrsqrt28pd	zmm30, [rdx-1024]{1to8}	 # AVX512ER Disp8
	vrsqrt28pd	zmm30, [rdx-1032]{1to8}	 # AVX512ER

	vrsqrt28ss	xmm30{k7}, xmm29, xmm28	 # AVX512ER
	vrsqrt28ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512ER
	vrsqrt28ss	xmm30{k7}, xmm29, xmm28{sae}	 # AVX512ER
	vrsqrt28ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512ER
	vrsqrt28ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512ER
	vrsqrt28ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512ER Disp8
	vrsqrt28ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512ER
	vrsqrt28ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512ER Disp8
	vrsqrt28ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512ER

	vrsqrt28sd	xmm30{k7}, xmm29, xmm28	 # AVX512ER
	vrsqrt28sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512ER
	vrsqrt28sd	xmm30{k7}, xmm29, xmm28{sae}	 # AVX512ER
	vrsqrt28sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512ER
	vrsqrt28sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512ER
	vrsqrt28sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512ER Disp8
	vrsqrt28sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512ER
	vrsqrt28sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512ER Disp8
	vrsqrt28sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512ER

