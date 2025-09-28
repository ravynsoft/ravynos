# Check illegal AVX512F instructions
	.text
	.allow_index_reg
_start:
	mov {sae}, %eax{%k1}
	mov {sae}, %eax
	mov %ebx, %eax{%k2}
	vaddps %zmm3, %zmm1, %zmm2{z}{%k1}{z}
	vaddps %zmm3, %zmm1{%k3}, %zmm2{z}
	vaddps %zmm3, %zmm1{%k1}, %zmm2{%k2}

	vcvtps2pd (%eax), %zmm1{1to8}
	vcvtps2pd (%eax){1to16}, %zmm1

	vcvtps2pd (%eax){%k1}, %zmm1
	vcvtps2pd (%eax){z}, %zmm1

	vgatherqpd (%rdi,%zmm2,8),%zmm6
	vgatherqpd (%edi),%zmm6{%k1}
	vgatherqpd (%zmm2),%zmm6{%k1}
	vpscatterdd %zmm6,(%edi){%k1}
	vpscatterdd %zmm6,(%zmm2){%k1}

	.intel_syntax noprefix
	mov eax{k1}, {sae}
	mov eax, {sae}
	mov eax{k2}, ebx
	vaddps zmm2{z}{k1}{z}, zmm1, zmm3
	vaddps zmm2{z}, zmm1{k3}, zmm3
	vaddps zmm2{k2}, zmm1{k1}, zmm3

	vcvtps2pd zmm1{1to8}, [eax]
	vcvtps2pd zmm1, [eax]{1to16}

	vcvtps2pd zmm1, [eax]{k1}
	vcvtps2pd zmm1, [eax]{z}

	vgatherqpd zmm6, ZMMWORD PTR [rdi+zmm2*8]
	vgatherqpd zmm6{k1}, ZMMWORD PTR [edi]
	vgatherqpd zmm6{k1}, ZMMWORD PTR [zmm2+eiz]
	vpscatterdd ZMMWORD PTR [edi]{k1}, zmm6
	vpscatterdd ZMMWORD PTR [zmm2+eiz]{k1}, zmm6

	vaddps zmm2, zmm1, QWORD PTR [eax]{1to8}
	vaddps zmm2, zmm1, QWORD PTR [eax]{1to16}
	vaddpd zmm2, zmm1, DWORD PTR [eax]{1to8}
	vaddpd zmm2, zmm1, DWORD PTR [eax]{1to16}
	vaddps zmm2, zmm1, ZMMWORD PTR [eax]{1to16}
	vaddps zmm2, zmm1, DWORD PTR [eax]
	vaddpd zmm2, zmm1, QWORD PTR [eax]

	.att_syntax prefix
	vaddps %zmm0, %zmm1, %zmm2{%ecx}
	vaddps %zmm0, %zmm1, %zmm2{z}

	.intel_syntax noprefix
	vaddps zmm2{ecx}, zmm1, zmm0
	vaddps zmm2{z}, zmm1, zmm0

	.att_syntax prefix
	vmovaps (%eax){1to2}, %zmm1
	vmovaps (%eax){1to4}, %zmm1
	vmovaps (%eax){1to8}, %zmm1
	vmovaps (%eax){1to16}, %zmm1

	vcvtps2pd (%eax){1to2}, %zmm1
	vcvtps2pd (%eax){1to4}, %zmm1
	vcvtps2pd (%eax){1to8}, %zmm1
	vcvtps2pd (%eax){1to16}, %zmm1

	vcvtps2pd (%eax){1to2}, %ymm1
	vcvtps2pd (%eax){1to4}, %ymm1
	vcvtps2pd (%eax){1to8}, %ymm1
	vcvtps2pd (%eax){1to16}, %ymm1

	vcvtps2pd (%eax){1to2}, %xmm1
	vcvtps2pd (%eax){1to4}, %xmm1
	vcvtps2pd (%eax){1to8}, %xmm1
	vcvtps2pd (%eax){1to16}, %xmm1

	vaddps (%eax){1to2}, %zmm1, %zmm2
	vaddps (%eax){1to4}, %zmm1, %zmm2
	vaddps (%eax){1to8}, %zmm1, %zmm2
	vaddps (%eax){1to16}, %zmm1, %zmm2

	vaddps (%eax){1to2}, %ymm1, %ymm2
	vaddps (%eax){1to4}, %ymm1, %ymm2
	vaddps (%eax){1to8}, %ymm1, %ymm2
	vaddps (%eax){1to16}, %ymm1, %ymm2

	vaddps (%eax){1to2}, %xmm1, %xmm2
	vaddps (%eax){1to4}, %xmm1, %xmm2
	vaddps (%eax){1to8}, %xmm1, %xmm2
	vaddps (%eax){1to16}, %xmm1, %xmm2

	vaddpd (%eax){1to2}, %zmm1, %zmm2
	vaddpd (%eax){1to4}, %zmm1, %zmm2
	vaddpd (%eax){1to8}, %zmm1, %zmm2
	vaddpd (%eax){1to16}, %zmm1, %zmm2

	vaddpd (%eax){1to2}, %ymm1, %ymm2
	vaddpd (%eax){1to4}, %ymm1, %ymm2
	vaddpd (%eax){1to8}, %ymm1, %ymm2
	vaddpd (%eax){1to16}, %ymm1, %ymm2

	vaddpd (%eax){1to2}, %xmm1, %xmm2
	vaddpd (%eax){1to4}, %xmm1, %xmm2
	vaddpd (%eax){1to8}, %xmm1, %xmm2
	vaddpd (%eax){1to16}, %xmm1, %xmm2

	.intel_syntax noprefix
	vcvtps2pd zmm1, QWORD PTR [eax]
	vcvtps2pd ymm1, QWORD PTR [eax]
	vcvtps2pd xmm1, QWORD PTR [eax]

	vcvtps2pd xmm1, DWORD PTR [eax]{1to2}
	vcvtps2pd xmm1, DWORD PTR [eax]{1to4}
	vcvtps2pd xmm1, DWORD PTR [eax]{1to8}
	vcvtps2pd xmm1, DWORD PTR [eax]{1to16}

	vaddps zmm2, zmm1, QWORD PTR [eax]
	vaddps ymm2, ymm1, QWORD PTR [eax]
	vaddps xmm2, xmm1, QWORD PTR [eax]

	vaddps zmm2, zmm1, DWORD PTR [eax]{1to2}
	vaddps zmm2, zmm1, DWORD PTR [eax]{1to4}
	vaddps zmm2, zmm1, DWORD PTR [eax]{1to8}
	vaddps zmm2, zmm1, DWORD PTR [eax]{1to16}

	vaddps ymm2, ymm1, DWORD PTR [eax]{1to2}
	vaddps ymm2, ymm1, DWORD PTR [eax]{1to4}
	vaddps ymm2, ymm1, DWORD PTR [eax]{1to8}
	vaddps ymm2, ymm1, DWORD PTR [eax]{1to16}

	vaddps xmm2, xmm1, DWORD PTR [eax]{1to2}
	vaddps xmm2, xmm1, DWORD PTR [eax]{1to4}
	vaddps xmm2, xmm1, DWORD PTR [eax]{1to8}
	vaddps xmm2, xmm1, DWORD PTR [eax]{1to16}

	vaddpd zmm2, zmm1, DWORD PTR [eax]
	vaddpd ymm2, ymm1, DWORD PTR [eax]
	vaddpd xmm2, xmm1, DWORD PTR [eax]

	vaddpd zmm2, zmm1, QWORD PTR [eax]{1to2}
	vaddpd zmm2, zmm1, QWORD PTR [eax]{1to4}
	vaddpd zmm2, zmm1, QWORD PTR [eax]{1to8}
	vaddpd zmm2, zmm1, QWORD PTR [eax]{1to16}

	vaddpd ymm2, ymm1, QWORD PTR [eax]{1to2}
	vaddpd ymm2, ymm1, QWORD PTR [eax]{1to4}
	vaddpd ymm2, ymm1, QWORD PTR [eax]{1to8}
	vaddpd ymm2, ymm1, QWORD PTR [eax]{1to16}

	vaddpd xmm2, xmm1, QWORD PTR [eax]{1to2}
	vaddpd xmm2, xmm1, QWORD PTR [eax]{1to4}
	vaddpd xmm2, xmm1, QWORD PTR [eax]{1to8}
	vaddpd xmm2, xmm1, QWORD PTR [eax]{1to16}

	vcvtps2qq xmm0, DWORD PTR [eax]

	.att_syntax prefix
	vcmppd $0, %zmm0, %zmm0, %k0{%k1}{z}
	vcmpps $0, %zmm0, %zmm0, %k0{%k1}{z}
	vcmpsd $0, %xmm0, %xmm0, %k0{%k1}{z}
	vcmpss $0, %xmm0, %xmm0, %k0{%k1}{z}

	vcompresspd %zmm0, (%eax){%k1}{z}
	vcompressps %zmm0, (%eax){%k1}{z}

	vcvtps2ph $0, %zmm0, (%eax){%k1}{z}

	vextractf32x4 $0, %zmm0, (%eax){%k1}{z}
	vextractf32x8 $0, %zmm0, (%eax){%k1}{z}
	vextractf64x2 $0, %zmm0, (%eax){%k1}{z}
	vextractf64x4 $0, %zmm0, (%eax){%k1}{z}

	vextracti32x4 $0, %zmm0, (%eax){%k1}{z}
	vextracti32x8 $0, %zmm0, (%eax){%k1}{z}
	vextracti64x2 $0, %zmm0, (%eax){%k1}{z}
	vextracti64x4 $0, %zmm0, (%eax){%k1}{z}

	vfpclasspd $0, %zmm0, %k0{%k1}{z}
	vfpclassps $0, %zmm0, %k0{%k1}{z}
	vfpclasssd $0, %xmm0, %k0{%k1}{z}
	vfpclassss $0, %xmm0, %k0{%k1}{z}

	vgatherdpd (%eax,%ymm1), %zmm0{%k1}{z}
	vgatherdps (%eax,%zmm1), %zmm0{%k1}{z}
	vgatherqpd (%eax,%zmm1), %zmm0{%k1}{z}
	vgatherqps (%eax,%zmm1), %ymm0{%k1}{z}

	vgatherpf0dpd (%eax,%ymm1){%k1}{z}
	vgatherpf0dps (%eax,%zmm1){%k1}{z}
	vgatherpf0qpd (%eax,%zmm1){%k1}{z}
	vgatherpf0qps (%eax,%zmm1){%k1}{z}

	vgatherpf1dpd (%eax,%ymm1){%k1}{z}
	vgatherpf1dps (%eax,%zmm1){%k1}{z}
	vgatherpf1qpd (%eax,%zmm1){%k1}{z}
	vgatherpf1qps (%eax,%zmm1){%k1}{z}

	vmovapd %zmm0, (%eax){%k1}{z}
	vmovaps %zmm0, (%eax){%k1}{z}

	vmovdqa32 %zmm0, (%eax){%k1}{z}
	vmovdqa64 %zmm0, (%eax){%k1}{z}

	vmovdqu8 %zmm0, (%eax){%k1}{z}
	vmovdqu16 %zmm0, (%eax){%k1}{z}
	vmovdqu32 %zmm0, (%eax){%k1}{z}
	vmovdqu64 %zmm0, (%eax){%k1}{z}

	vmovsd %xmm0, (%eax){%k1}{z}
	vmovss %xmm0, (%eax){%k1}{z}

	vmovupd %zmm0, (%eax){%k1}{z}
	vmovups %zmm0, (%eax){%k1}{z}

	vpcmpb $0, %zmm0, %zmm0, %k0{%k1}{z}
	vpcmpd $0, %zmm0, %zmm0, %k0{%k1}{z}
	vpcmpq $0, %zmm0, %zmm0, %k0{%k1}{z}
	vpcmpw $0, %zmm0, %zmm0, %k0{%k1}{z}

	vpcmpeqb %zmm0, %zmm0, %k0{%k1}{z}
	vpcmpeqd %zmm0, %zmm0, %k0{%k1}{z}
	vpcmpeqq %zmm0, %zmm0, %k0{%k1}{z}
	vpcmpeqw %zmm0, %zmm0, %k0{%k1}{z}

	vpcmpgtb %zmm0, %zmm0, %k0{%k1}{z}
	vpcmpgtd %zmm0, %zmm0, %k0{%k1}{z}
	vpcmpgtq %zmm0, %zmm0, %k0{%k1}{z}
	vpcmpgtw %zmm0, %zmm0, %k0{%k1}{z}

	vpcmpub $0, %zmm0, %zmm0, %k0{%k1}{z}
	vpcmpud $0, %zmm0, %zmm0, %k0{%k1}{z}
	vpcmpuq $0, %zmm0, %zmm0, %k0{%k1}{z}
	vpcmpuw $0, %zmm0, %zmm0, %k0{%k1}{z}

	vpcompressb %zmm0, (%eax){%k1}{z}
	vpcompressd %zmm0, (%eax){%k1}{z}
	vpcompressq %zmm0, (%eax){%k1}{z}
	vpcompressw %zmm0, (%eax){%k1}{z}

	vpgatherdd (%eax,%zmm1), %zmm0{%k1}{z}
	vpgatherdq (%eax,%ymm1), %zmm0{%k1}{z}
	vpgatherqd (%eax,%zmm1), %ymm0{%k1}{z}
	vpgatherqq (%eax,%zmm1), %zmm0{%k1}{z}

	vpmovdb %zmm0, (%eax){%k1}{z}
	vpmovdw %zmm0, (%eax){%k1}{z}

	vpmovqb %zmm0, (%eax){%k1}{z}
	vpmovqd %zmm0, (%eax){%k1}{z}
	vpmovqw %zmm0, (%eax){%k1}{z}

	vpmovsdb %zmm0, (%eax){%k1}{z}
	vpmovsdw %zmm0, (%eax){%k1}{z}

	vpmovsqb %zmm0, (%eax){%k1}{z}
	vpmovsqd %zmm0, (%eax){%k1}{z}
	vpmovsqw %zmm0, (%eax){%k1}{z}

	vpmovswb %zmm0, (%eax){%k1}{z}

	vpmovusdb %zmm0, (%eax){%k1}{z}
	vpmovusdw %zmm0, (%eax){%k1}{z}

	vpmovusqb %zmm0, (%eax){%k1}{z}
	vpmovusqd %zmm0, (%eax){%k1}{z}
	vpmovusqw %zmm0, (%eax){%k1}{z}

	vpmovuswb %zmm0, (%eax){%k1}{z}

	vpmovwb %zmm0, (%eax){%k1}{z}

	vpscatterdd %zmm0, (%eax,%zmm1){%k1}{z}
	vpscatterdq %zmm0, (%eax,%ymm1){%k1}{z}
	vpscatterqd %ymm0, (%eax,%zmm1){%k1}{z}
	vpscatterqq %zmm0, (%eax,%zmm1){%k1}{z}

	vpshufbitqmb %zmm0, %zmm0, %k0{%k1}{z}

	vptestmb %zmm0, %zmm0, %k0{%k1}{z}
	vptestmd %zmm0, %zmm0, %k0{%k1}{z}
	vptestmq %zmm0, %zmm0, %k0{%k1}{z}
	vptestmw %zmm0, %zmm0, %k0{%k1}{z}

	vptestnmb %zmm0, %zmm0, %k0{%k1}{z}
	vptestnmd %zmm0, %zmm0, %k0{%k1}{z}
	vptestnmq %zmm0, %zmm0, %k0{%k1}{z}
	vptestnmw %zmm0, %zmm0, %k0{%k1}{z}

	vscatterdpd %zmm0, (%eax,%ymm1){%k1}{z}
	vscatterdps %zmm0, (%eax,%zmm1){%k1}{z}
	vscatterqpd %zmm0, (%eax,%zmm1){%k1}{z}
	vscatterqps %ymm0, (%eax,%zmm1){%k1}{z}

	vscatterpf0dpd (%eax,%ymm1){%k1}{z}
	vscatterpf0dps (%eax,%zmm1){%k1}{z}
	vscatterpf0qpd (%eax,%zmm1){%k1}{z}
	vscatterpf0qps (%eax,%zmm1){%k1}{z}

	vscatterpf1dpd (%eax,%ymm1){%k1}{z}
	vscatterpf1dps (%eax,%zmm1){%k1}{z}
	vscatterpf1qpd (%eax,%zmm1){%k1}{z}
	vscatterpf1qps (%eax,%zmm1){%k1}{z}

	vdpbf16ps 8(%eax){1to8}, %zmm2, %zmm2
	vcvtne2ps2bf16 8(%eax){1to8}, %zmm2, %zmm2

	vcvtneps2bf16 (%eax){1to2}, %ymm1
	vcvtneps2bf16 (%eax){1to4}, %ymm1
	vcvtneps2bf16 (%eax){1to8}, %ymm1
	vcvtneps2bf16 (%eax){1to16}, %ymm1

	vcvtneps2bf16 (%eax){1to2}, %xmm1
	vcvtneps2bf16 (%eax){1to4}, %xmm1
	vcvtneps2bf16 (%eax){1to8}, %xmm1
	vcvtneps2bf16 (%eax){1to16}, %xmm1

	vaddps $0xcc, %zmm0, %zmm0, %zmm0
	vcmpss $0, $0xcc, %xmm0, %xmm0, %k0

	vaddss {sae}, %xmm0, %xmm0, %xmm0
	vcmpps $0, {rn-sae}, %zmm0, %zmm0, %k0

	.intel_syntax noprefix
	vaddps zmm2, zmm1, WORD BCST [eax]
	vaddps zmm2, zmm1, DWORD BCST [eax]
	vaddps zmm2, zmm1, QWORD BCST [eax]
	vaddps zmm2, zmm1, ZMMWORD BCST [eax]

	vaddpd zmm2, zmm1, WORD BCST [eax]
	vaddpd zmm2, zmm1, DWORD BCST [eax]
	vaddpd zmm2, zmm1, QWORD BCST [eax]
	vaddpd zmm2, zmm1, ZMMWORD BCST [eax]

	.att_syntax prefix
	vaddps {rn-sae}, %zmm0, %zmm0, %zmm0
	vaddps %zmm0, {rn-sae}, %zmm0, %zmm0
	vaddps %zmm0, %zmm0, {rn-sae}, %zmm0
	vaddps %zmm0, %zmm0, %zmm0, {rn-sae}

	vcmpps {sae}, $0, %zmm0, %zmm0, %k0
	vcmpps $0, {sae}, %zmm0, %zmm0, %k0
	vcmpps $0, %zmm0, {sae}, %zmm0, %k0
	vcmpps $0, %zmm0, %zmm0, {sae}, %k0
	vcmpps $0, %zmm0, %zmm0, %k0, {sae}

	vcvtsi2ss {rn-sae}, %eax, %xmm0, %xmm0
	vcvtsi2ss %eax, {rn-sae}, %xmm0, %xmm0
	vcvtsi2ss %eax, %xmm0, {rn-sae}, %xmm0
	vcvtsi2ss %eax, %xmm0, %xmm0, {rn-sae}

	.intel_syntax noprefix
	vaddps zmm0{rn-sae}, zmm0, zmm0
	vaddps zmm0, zmm0{rn-sae}, zmm0
	vaddps zmm0, zmm0, zmm0{rn-sae}

	vcmpps k0{sae}, zmm0, zmm0, 0
	vcmpps k0, zmm0{sae}, zmm0, 0
	vcmpps k0, zmm0, zmm0{sae}, 0
	vcmpps k0, zmm0, zmm0, 0{sae}

	vcvtsi2ss xmm0{rn-sae}, xmm0, eax
	vcvtsi2ss xmm0, xmm0{rn-sae}, eax
	vcvtsi2ss xmm0, xmm0, eax{rn-sae}

	.p2align 4
