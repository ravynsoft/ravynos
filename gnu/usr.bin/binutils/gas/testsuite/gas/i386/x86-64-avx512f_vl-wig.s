# Check 64bit AVX512{F,VL} WIG instructions

	.allow_index_reg
	.text
_start:
	vpmovsxbd	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovsxbd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsxbd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbd	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovsxbd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovsxbd	508(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxbd	512(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxbd	-512(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxbd	-516(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxbd	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovsxbd	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovsxbd	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbd	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovsxbd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovsxbd	1016(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxbd	1024(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxbd	-1024(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxbd	-1032(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxbq	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovsxbq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsxbq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbq	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovsxbq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovsxbq	254(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxbq	256(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxbq	-256(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxbq	-258(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxbq	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovsxbq	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovsxbq	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbq	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovsxbq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovsxbq	508(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxbq	512(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxbq	-512(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxbq	-516(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxwd	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovsxwd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsxwd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwd	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovsxwd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovsxwd	1016(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxwd	1024(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxwd	-1024(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxwd	-1032(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxwd	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovsxwd	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovsxwd	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwd	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovsxwd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovsxwd	2032(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxwd	2048(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxwd	-2048(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxwd	-2064(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxwq	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovsxwq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsxwq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwq	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovsxwq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovsxwq	508(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxwq	512(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxwq	-512(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxwq	-516(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxwq	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovsxwq	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovsxwq	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwq	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovsxwq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovsxwq	1016(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxwq	1024(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxwq	-1024(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxwq	-1032(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxbd	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovzxbd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovzxbd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbd	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovzxbd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovzxbd	508(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxbd	512(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxbd	-512(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxbd	-516(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxbd	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovzxbd	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovzxbd	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbd	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovzxbd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovzxbd	1016(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxbd	1024(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxbd	-1024(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxbd	-1032(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxbq	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovzxbq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovzxbq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbq	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovzxbq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovzxbq	254(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxbq	256(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxbq	-256(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxbq	-258(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxbq	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovzxbq	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovzxbq	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbq	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovzxbq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovzxbq	508(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxbq	512(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxbq	-512(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxbq	-516(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxwd	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovzxwd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovzxwd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwd	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovzxwd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovzxwd	1016(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxwd	1024(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxwd	-1024(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxwd	-1032(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxwd	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovzxwd	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovzxwd	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwd	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovzxwd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovzxwd	2032(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxwd	2048(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxwd	-2048(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxwd	-2064(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxwq	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovzxwq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovzxwq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwq	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovzxwq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovzxwq	508(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxwq	512(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxwq	-512(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxwq	-516(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxwq	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovzxwq	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovzxwq	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwq	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovzxwq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovzxwq	1016(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxwq	1024(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxwq	-1024(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxwq	-1032(%rdx), %ymm30	 # AVX512{F,VL}

	.intel_syntax noprefix
	vpmovsxbd	xmm30, xmm29	 # AVX512{F,VL}
	vpmovsxbd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxbd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxbd	xmm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxbd	xmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxbd	xmm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpmovsxbd	xmm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpmovsxbd	xmm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpmovsxbd	xmm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpmovsxbd	ymm30, xmm29	 # AVX512{F,VL}
	vpmovsxbd	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxbd	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxbd	ymm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxbd	ymm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxbd	ymm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpmovsxbd	ymm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpmovsxbd	ymm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpmovsxbd	ymm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpmovsxbq	xmm30, xmm29	 # AVX512{F,VL}
	vpmovsxbq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxbq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxbq	xmm30, WORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxbq	xmm30, WORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxbq	xmm30, WORD PTR [rdx+254]	 # AVX512{F,VL} Disp8
	vpmovsxbq	xmm30, WORD PTR [rdx+256]	 # AVX512{F,VL}
	vpmovsxbq	xmm30, WORD PTR [rdx-256]	 # AVX512{F,VL} Disp8
	vpmovsxbq	xmm30, WORD PTR [rdx-258]	 # AVX512{F,VL}
	vpmovsxbq	ymm30, xmm29	 # AVX512{F,VL}
	vpmovsxbq	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxbq	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxbq	ymm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxbq	ymm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxbq	ymm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpmovsxbq	ymm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpmovsxbq	ymm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpmovsxbq	ymm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpmovsxwd	xmm30, xmm29	 # AVX512{F,VL}
	vpmovsxwd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxwd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxwd	xmm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxwd	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxwd	xmm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpmovsxwd	xmm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpmovsxwd	xmm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpmovsxwd	xmm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpmovsxwd	ymm30, xmm29	 # AVX512{F,VL}
	vpmovsxwd	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxwd	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxwd	ymm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxwd	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxwd	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpmovsxwd	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpmovsxwd	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpmovsxwd	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpmovsxwq	xmm30, xmm29	 # AVX512{F,VL}
	vpmovsxwq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxwq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxwq	xmm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxwq	xmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxwq	xmm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpmovsxwq	xmm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpmovsxwq	xmm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpmovsxwq	xmm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpmovsxwq	ymm30, xmm29	 # AVX512{F,VL}
	vpmovsxwq	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxwq	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxwq	ymm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxwq	ymm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxwq	ymm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpmovsxwq	ymm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpmovsxwq	ymm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpmovsxwq	ymm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpmovzxbd	xmm30, xmm29	 # AVX512{F,VL}
	vpmovzxbd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxbd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxbd	xmm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxbd	xmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxbd	xmm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpmovzxbd	xmm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpmovzxbd	xmm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpmovzxbd	xmm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpmovzxbd	ymm30, xmm29	 # AVX512{F,VL}
	vpmovzxbd	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxbd	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxbd	ymm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxbd	ymm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxbd	ymm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpmovzxbd	ymm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpmovzxbd	ymm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpmovzxbd	ymm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpmovzxbq	xmm30, xmm29	 # AVX512{F,VL}
	vpmovzxbq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxbq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxbq	xmm30, WORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxbq	xmm30, WORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxbq	xmm30, WORD PTR [rdx+254]	 # AVX512{F,VL} Disp8
	vpmovzxbq	xmm30, WORD PTR [rdx+256]	 # AVX512{F,VL}
	vpmovzxbq	xmm30, WORD PTR [rdx-256]	 # AVX512{F,VL} Disp8
	vpmovzxbq	xmm30, WORD PTR [rdx-258]	 # AVX512{F,VL}
	vpmovzxbq	ymm30, xmm29	 # AVX512{F,VL}
	vpmovzxbq	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxbq	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxbq	ymm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxbq	ymm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxbq	ymm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpmovzxbq	ymm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpmovzxbq	ymm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpmovzxbq	ymm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpmovzxwd	xmm30, xmm29	 # AVX512{F,VL}
	vpmovzxwd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxwd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxwd	xmm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxwd	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxwd	xmm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpmovzxwd	xmm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpmovzxwd	xmm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpmovzxwd	xmm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpmovzxwd	ymm30, xmm29	 # AVX512{F,VL}
	vpmovzxwd	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxwd	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxwd	ymm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxwd	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxwd	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpmovzxwd	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpmovzxwd	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpmovzxwd	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpmovzxwq	xmm30, xmm29	 # AVX512{F,VL}
	vpmovzxwq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxwq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxwq	xmm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxwq	xmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxwq	xmm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpmovzxwq	xmm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpmovzxwq	xmm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpmovzxwq	xmm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpmovzxwq	ymm30, xmm29	 # AVX512{F,VL}
	vpmovzxwq	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxwq	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxwq	ymm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxwq	ymm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxwq	ymm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpmovzxwq	ymm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpmovzxwq	ymm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpmovzxwq	ymm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
