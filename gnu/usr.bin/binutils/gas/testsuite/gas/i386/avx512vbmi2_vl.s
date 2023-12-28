# Check 32bit AVX512{VBMI2,VL} instructions

	.allow_index_reg
	.text
_start:
	vpcompressb	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{VBMI2,VL}
	vpcompressb	%xmm6, 126(%edx){%k7}	 # AVX512{VBMI2,VL} Disp8
	vpcompressb	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{VBMI2,VL}
	vpcompressb	%ymm6, 126(%edx){%k7}	 # AVX512{VBMI2,VL} Disp8
	vpcompressb	%xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpcompressb	%xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpcompressb	%ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpcompressb	%ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}

	vpcompressw	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{VBMI2,VL}
	vpcompressw	%xmm6, 128(%edx){%k7}	 # AVX512{VBMI2,VL} Disp8
	vpcompressw	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{VBMI2,VL}
	vpcompressw	%ymm6, 128(%edx){%k7}	 # AVX512{VBMI2,VL} Disp8
	vpcompressw	%xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpcompressw	%xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpcompressw	%ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpcompressw	%ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}

	vpexpandb	(%ecx), %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpexpandb	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpexpandb	126(%edx), %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpexpandb	(%ecx), %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpexpandb	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpexpandb	126(%edx), %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpexpandb	%xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpexpandb	%xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpexpandb	%ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpexpandb	%ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}

	vpexpandw	(%ecx), %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpexpandw	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpexpandw	128(%edx), %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpexpandw	(%ecx), %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpexpandw	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpexpandw	128(%edx), %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpexpandw	%xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpexpandw	%xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpexpandw	%ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpexpandw	%ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}

	vpshldvw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldvw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldvw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldvw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshldvw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldvw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldvw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldvw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8

	vpshldvd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldvd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldvd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldvd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshldvd	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshldvd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldvd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldvd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldvd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshldvd	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8

	vpshldvq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldvq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldvq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldvq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshldvq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshldvq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldvq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldvq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldvq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshldvq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8

	vpshrdvw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdvw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshrdvw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdvw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8

	vpshrdvd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdvd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshrdvd	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshrdvd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdvd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshrdvd	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8

	vpshrdvq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdvq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshrdvq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshrdvq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdvq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshrdvq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8

	vpshldw	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldw	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldw	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldw	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshldw	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldw	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldw	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldw	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8

	vpshldd	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldd	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldd	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldd	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshldd	$123, 508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshldd	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldd	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldd	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldd	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshldd	$123, 508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8

	vpshldq	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldq	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldq	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldq	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshldq	$123, 1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshldq	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldq	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldq	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshldq	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshldq	$123, 1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8

	vpshrdw	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdw	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdw	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdw	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshrdw	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdw	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdw	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdw	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8

	vpshrdd	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdd	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdd	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdd	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshrdd	$123, 508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshrdd	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdd	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdd	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdd	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshrdd	$123, 508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8

	vpshrdq	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdq	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdq	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdq	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshrdq	$123, 1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshrdq	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdq	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdq	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL}
	vpshrdq	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8
	vpshrdq	$123, 1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{VBMI2,VL} Disp8

	.intel_syntax noprefix
	vpcompressb	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{VBMI2,VL}
	vpcompressb	XMMWORD PTR [edx+126]{k7}, xmm6	 # AVX512{VBMI2,VL} Disp8
	vpcompressb	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{VBMI2,VL}
	vpcompressb	YMMWORD PTR [edx+126]{k7}, ymm6	 # AVX512{VBMI2,VL} Disp8
	vpcompressb	xmm6{k7}, xmm5	 # AVX512{VBMI2,VL}
	vpcompressb	xmm6{k7}{z}, xmm5	 # AVX512{VBMI2,VL}
	vpcompressb	ymm6{k7}, ymm5	 # AVX512{VBMI2,VL}
	vpcompressb	ymm6{k7}{z}, ymm5	 # AVX512{VBMI2,VL}

	vpcompressw	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{VBMI2,VL}
	vpcompressw	XMMWORD PTR [edx+128]{k7}, xmm6	 # AVX512{VBMI2,VL} Disp8
	vpcompressw	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{VBMI2,VL}
	vpcompressw	YMMWORD PTR [edx+128]{k7}, ymm6	 # AVX512{VBMI2,VL} Disp8
	vpcompressw	xmm6{k7}, xmm5	 # AVX512{VBMI2,VL}
	vpcompressw	xmm6{k7}{z}, xmm5	 # AVX512{VBMI2,VL}
	vpcompressw	ymm6{k7}, ymm5	 # AVX512{VBMI2,VL}
	vpcompressw	ymm6{k7}{z}, ymm5	 # AVX512{VBMI2,VL}

	vpexpandb	xmm6{k7}{z}, XMMWORD PTR [ecx]	 # AVX512{VBMI2,VL}
	vpexpandb	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpexpandb	xmm6{k7}, XMMWORD PTR [edx+126]	 # AVX512{VBMI2,VL} Disp8
	vpexpandb	ymm6{k7}{z}, YMMWORD PTR [ecx]	 # AVX512{VBMI2,VL}
	vpexpandb	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpexpandb	ymm6{k7}, YMMWORD PTR [edx+126]	 # AVX512{VBMI2,VL} Disp8
	vpexpandb	xmm6{k7}, xmm5	 # AVX512{VBMI2,VL}
	vpexpandb	xmm6{k7}{z}, xmm5	 # AVX512{VBMI2,VL}
	vpexpandb	ymm6{k7}, ymm5	 # AVX512{VBMI2,VL}
	vpexpandb	ymm6{k7}{z}, ymm5	 # AVX512{VBMI2,VL}

	vpexpandw	xmm6{k7}{z}, XMMWORD PTR [ecx]	 # AVX512{VBMI2,VL}
	vpexpandw	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpexpandw	xmm6{k7}, XMMWORD PTR [edx+128]	 # AVX512{VBMI2,VL} Disp8
	vpexpandw	ymm6{k7}{z}, YMMWORD PTR [ecx]	 # AVX512{VBMI2,VL}
	vpexpandw	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpexpandw	ymm6{k7}, YMMWORD PTR [edx+128]	 # AVX512{VBMI2,VL} Disp8
	vpexpandw	xmm6{k7}, xmm5	 # AVX512{VBMI2,VL}
	vpexpandw	xmm6{k7}{z}, xmm5	 # AVX512{VBMI2,VL}
	vpexpandw	ymm6{k7}, ymm5	 # AVX512{VBMI2,VL}
	vpexpandw	ymm6{k7}{z}, ymm5	 # AVX512{VBMI2,VL}

	vpshldvw	xmm6{k7}, xmm5, xmm4	 # AVX512{VBMI2,VL}
	vpshldvw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{VBMI2,VL}
	vpshldvw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpshldvw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{VBMI2,VL} Disp8
	vpshldvw	ymm6{k7}, ymm5, ymm4	 # AVX512{VBMI2,VL}
	vpshldvw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{VBMI2,VL}
	vpshldvw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpshldvw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{VBMI2,VL} Disp8

	vpshldvd	xmm6{k7}, xmm5, xmm4	 # AVX512{VBMI2,VL}
	vpshldvd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{VBMI2,VL}
	vpshldvd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpshldvd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{VBMI2,VL} Disp8
	vpshldvd	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{VBMI2,VL} Disp8
	vpshldvd	ymm6{k7}, ymm5, ymm4	 # AVX512{VBMI2,VL}
	vpshldvd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{VBMI2,VL}
	vpshldvd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpshldvd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{VBMI2,VL} Disp8
	vpshldvd	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{VBMI2,VL} Disp8

	vpshldvq	xmm6{k7}, xmm5, xmm4	 # AVX512{VBMI2,VL}
	vpshldvq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{VBMI2,VL}
	vpshldvq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpshldvq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{VBMI2,VL} Disp8
	vpshldvq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{VBMI2,VL} Disp8
	vpshldvq	ymm6{k7}, ymm5, ymm4	 # AVX512{VBMI2,VL}
	vpshldvq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{VBMI2,VL}
	vpshldvq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpshldvq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{VBMI2,VL} Disp8
	vpshldvq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{VBMI2,VL} Disp8

	vpshrdvw	xmm6{k7}, xmm5, xmm4	 # AVX512{VBMI2,VL}
	vpshrdvw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{VBMI2,VL}
	vpshrdvw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpshrdvw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{VBMI2,VL} Disp8
	vpshrdvw	ymm6{k7}, ymm5, ymm4	 # AVX512{VBMI2,VL}
	vpshrdvw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{VBMI2,VL}
	vpshrdvw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpshrdvw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{VBMI2,VL} Disp8

	vpshrdvd	xmm6{k7}, xmm5, xmm4	 # AVX512{VBMI2,VL}
	vpshrdvd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{VBMI2,VL}
	vpshrdvd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpshrdvd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{VBMI2,VL} Disp8
	vpshrdvd	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{VBMI2,VL} Disp8
	vpshrdvd	ymm6{k7}, ymm5, ymm4	 # AVX512{VBMI2,VL}
	vpshrdvd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{VBMI2,VL}
	vpshrdvd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpshrdvd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{VBMI2,VL} Disp8
	vpshrdvd	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{VBMI2,VL} Disp8

	vpshrdvq	xmm6{k7}, xmm5, xmm4	 # AVX512{VBMI2,VL}
	vpshrdvq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{VBMI2,VL}
	vpshrdvq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpshrdvq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{VBMI2,VL} Disp8
	vpshrdvq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{VBMI2,VL} Disp8
	vpshrdvq	ymm6{k7}, ymm5, ymm4	 # AVX512{VBMI2,VL}
	vpshrdvq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{VBMI2,VL}
	vpshrdvq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI2,VL}
	vpshrdvq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{VBMI2,VL} Disp8
	vpshrdvq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{VBMI2,VL} Disp8

	vpshldw	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{VBMI2,VL}
	vpshldw	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{VBMI2,VL}
	vpshldw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{VBMI2,VL}
	vpshldw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{VBMI2,VL} Disp8
	vpshldw	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{VBMI2,VL}
	vpshldw	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{VBMI2,VL}
	vpshldw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{VBMI2,VL}
	vpshldw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{VBMI2,VL} Disp8

	vpshldd	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{VBMI2,VL}
	vpshldd	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{VBMI2,VL}
	vpshldd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{VBMI2,VL}
	vpshldd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{VBMI2,VL} Disp8
	vpshldd	xmm6{k7}, xmm5, [edx+508]{1to4}, 123	 # AVX512{VBMI2,VL} Disp8
	vpshldd	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{VBMI2,VL}
	vpshldd	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{VBMI2,VL}
	vpshldd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{VBMI2,VL}
	vpshldd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{VBMI2,VL} Disp8
	vpshldd	ymm6{k7}, ymm5, [edx+508]{1to8}, 123	 # AVX512{VBMI2,VL} Disp8

	vpshldq	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{VBMI2,VL}
	vpshldq	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{VBMI2,VL}
	vpshldq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{VBMI2,VL}
	vpshldq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{VBMI2,VL} Disp8
	vpshldq	xmm6{k7}, xmm5, [edx+1016]{1to2}, 123	 # AVX512{VBMI2,VL} Disp8
	vpshldq	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{VBMI2,VL}
	vpshldq	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{VBMI2,VL}
	vpshldq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{VBMI2,VL}
	vpshldq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{VBMI2,VL} Disp8
	vpshldq	ymm6{k7}, ymm5, [edx+1016]{1to4}, 123	 # AVX512{VBMI2,VL} Disp8

	vpshrdw	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{VBMI2,VL}
	vpshrdw	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{VBMI2,VL}
	vpshrdw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{VBMI2,VL}
	vpshrdw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdw	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{VBMI2,VL}
	vpshrdw	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{VBMI2,VL}
	vpshrdw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{VBMI2,VL}
	vpshrdw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdd	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{VBMI2,VL}
	vpshrdd	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{VBMI2,VL}
	vpshrdd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{VBMI2,VL}
	vpshrdd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdd	xmm6{k7}, xmm5, [edx+508]{1to4}, 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdd	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{VBMI2,VL}
	vpshrdd	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{VBMI2,VL}
	vpshrdd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{VBMI2,VL}
	vpshrdd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdd	ymm6{k7}, ymm5, [edx+508]{1to8}, 123	 # AVX512{VBMI2,VL} Disp8

	vpshrdq	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{VBMI2,VL}
	vpshrdq	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{VBMI2,VL}
	vpshrdq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{VBMI2,VL}
	vpshrdq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdq	xmm6{k7}, xmm5, [edx+1016]{1to2}, 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdq	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{VBMI2,VL}
	vpshrdq	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{VBMI2,VL}
	vpshrdq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{VBMI2,VL}
	vpshrdq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdq	ymm6{k7}, ymm5, [edx+1016]{1to4}, 123	 # AVX512{VBMI2,VL} Disp8
