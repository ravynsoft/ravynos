# Check 32bit AVX512{F,VL} WIG instructions

	.allow_index_reg
	.text
_start:
	vpmovsxbd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	508(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbd	512(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	-512(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbd	-516(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	1016(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbd	1024(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	-1024(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbd	-1032(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	254(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbq	256(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	-256(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbq	-258(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	508(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbq	512(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	-512(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbq	-516(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	1016(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwd	1024(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	-1024(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwd	-1032(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	2032(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwd	2048(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	-2048(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwd	-2064(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	508(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwq	512(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	-512(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwq	-516(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	1016(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwq	1024(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	-1024(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwq	-1032(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	508(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbd	512(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	-512(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbd	-516(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	1016(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbd	1024(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	-1024(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbd	-1032(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	254(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbq	256(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	-256(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbq	-258(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	508(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbq	512(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	-512(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbq	-516(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	1016(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwd	1024(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	-1024(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwd	-1032(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	2032(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwd	2048(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	-2048(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwd	-2064(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	508(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwq	512(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	-512(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwq	-516(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	1016(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwq	1024(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	-1024(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwq	-1032(%edx), %ymm6{%k7}	 # AVX512{F,VL}

	.intel_syntax noprefix
	vpmovsxbd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxbd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxbd	xmm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxbd	xmm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxbd	xmm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpmovsxbd	xmm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vpmovsxbd	xmm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpmovsxbd	xmm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vpmovsxbd	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxbd	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxbd	ymm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxbd	ymm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxbd	ymm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpmovsxbd	ymm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpmovsxbd	ymm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpmovsxbd	ymm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpmovsxbq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxbq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxbq	xmm6{k7}, WORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxbq	xmm6{k7}, WORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxbq	xmm6{k7}, WORD PTR [edx+254]	 # AVX512{F,VL} Disp8
	vpmovsxbq	xmm6{k7}, WORD PTR [edx+256]	 # AVX512{F,VL}
	vpmovsxbq	xmm6{k7}, WORD PTR [edx-256]	 # AVX512{F,VL} Disp8
	vpmovsxbq	xmm6{k7}, WORD PTR [edx-258]	 # AVX512{F,VL}
	vpmovsxbq	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxbq	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxbq	ymm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxbq	ymm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxbq	ymm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpmovsxbq	ymm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vpmovsxbq	ymm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpmovsxbq	ymm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vpmovsxwd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxwd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxwd	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxwd	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxwd	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpmovsxwd	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpmovsxwd	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpmovsxwd	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpmovsxwd	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxwd	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxwd	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxwd	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxwd	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpmovsxwd	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpmovsxwd	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpmovsxwd	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpmovsxwq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxwq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxwq	xmm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxwq	xmm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxwq	xmm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpmovsxwq	xmm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vpmovsxwq	xmm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpmovsxwq	xmm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vpmovsxwq	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxwq	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxwq	ymm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxwq	ymm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxwq	ymm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpmovsxwq	ymm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpmovsxwq	ymm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpmovsxwq	ymm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpmovzxbd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxbd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxbd	xmm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxbd	xmm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxbd	xmm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpmovzxbd	xmm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vpmovzxbd	xmm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpmovzxbd	xmm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vpmovzxbd	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxbd	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxbd	ymm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxbd	ymm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxbd	ymm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpmovzxbd	ymm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpmovzxbd	ymm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpmovzxbd	ymm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpmovzxbq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxbq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxbq	xmm6{k7}, WORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxbq	xmm6{k7}, WORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxbq	xmm6{k7}, WORD PTR [edx+254]	 # AVX512{F,VL} Disp8
	vpmovzxbq	xmm6{k7}, WORD PTR [edx+256]	 # AVX512{F,VL}
	vpmovzxbq	xmm6{k7}, WORD PTR [edx-256]	 # AVX512{F,VL} Disp8
	vpmovzxbq	xmm6{k7}, WORD PTR [edx-258]	 # AVX512{F,VL}
	vpmovzxbq	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxbq	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxbq	ymm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxbq	ymm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxbq	ymm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpmovzxbq	ymm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vpmovzxbq	ymm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpmovzxbq	ymm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vpmovzxwd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxwd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxwd	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxwd	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxwd	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpmovzxwd	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpmovzxwd	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpmovzxwd	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpmovzxwd	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxwd	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxwd	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxwd	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxwd	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpmovzxwd	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpmovzxwd	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpmovzxwd	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpmovzxwq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxwq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxwq	xmm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxwq	xmm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxwq	xmm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpmovzxwq	xmm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vpmovzxwq	xmm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpmovzxwq	xmm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vpmovzxwq	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxwq	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxwq	ymm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxwq	ymm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxwq	ymm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpmovzxwq	ymm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpmovzxwq	ymm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpmovzxwq	ymm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
