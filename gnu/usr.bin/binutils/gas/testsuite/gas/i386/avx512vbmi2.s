# Check 32bit AVX512VBMI2 instructions

	.allow_index_reg
	.text
_start:
	vpcompressb	%zmm6, (%ecx){%k7}	 # AVX512VBMI2
	vpcompressb	%zmm6, -123456(%esp,%esi,8)	 # AVX512VBMI2
	vpcompressb	%zmm6, 126(%edx)	 # AVX512VBMI2 Disp8
	vpcompressb	%zmm5, %zmm6	 # AVX512VBMI2
	vpcompressb	%zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpcompressb	%zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2

	vpcompressw	%zmm6, (%ecx){%k7}	 # AVX512VBMI2
	vpcompressw	%zmm6, -123456(%esp,%esi,8)	 # AVX512VBMI2
	vpcompressw	%zmm6, 128(%edx)	 # AVX512VBMI2 Disp8
	vpcompressw	%zmm5, %zmm6	 # AVX512VBMI2
	vpcompressw	%zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpcompressw	%zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2

	vpexpandb	(%ecx), %zmm6{%k7}	 # AVX512VBMI2
	vpexpandb	(%ecx), %zmm6{%k7}{z}	 # AVX512VBMI2
	vpexpandb	-123456(%esp,%esi,8), %zmm6	 # AVX512VBMI2
	vpexpandb	126(%edx), %zmm6	 # AVX512VBMI2 Disp8
	vpexpandb	%zmm5, %zmm6	 # AVX512VBMI2
	vpexpandb	%zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpexpandb	%zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2

	vpexpandw	(%ecx), %zmm6{%k7}	 # AVX512VBMI2
	vpexpandw	(%ecx), %zmm6{%k7}{z}	 # AVX512VBMI2
	vpexpandw	-123456(%esp,%esi,8), %zmm6	 # AVX512VBMI2
	vpexpandw	128(%edx), %zmm6	 # AVX512VBMI2 Disp8
	vpexpandw	%zmm5, %zmm6	 # AVX512VBMI2
	vpexpandw	%zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpexpandw	%zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2

	vpshldvw	%zmm4, %zmm5, %zmm6	 # AVX512VBMI2
	vpshldvw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpshldvw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2
	vpshldvw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI2
	vpshldvw	128(%edx), %zmm5, %zmm6	 # AVX512VBMI2 Disp8

	vpshldvd	%zmm4, %zmm5, %zmm6	 # AVX512VBMI2
	vpshldvd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpshldvd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2
	vpshldvd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI2
	vpshldvd	128(%edx), %zmm5, %zmm6	 # AVX512VBMI2 Disp8
	vpshldvd	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512VBMI2 Disp8

	vpshldvq	%zmm4, %zmm5, %zmm6	 # AVX512VBMI2
	vpshldvq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpshldvq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2
	vpshldvq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI2
	vpshldvq	128(%edx), %zmm5, %zmm6	 # AVX512VBMI2 Disp8
	vpshldvq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512VBMI2 Disp8

	vpshrdvw	%zmm4, %zmm5, %zmm6	 # AVX512VBMI2
	vpshrdvw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpshrdvw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2
	vpshrdvw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI2
	vpshrdvw	128(%edx), %zmm5, %zmm6	 # AVX512VBMI2 Disp8

	vpshrdvd	%zmm4, %zmm5, %zmm6	 # AVX512VBMI2
	vpshrdvd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpshrdvd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2
	vpshrdvd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI2
	vpshrdvd	128(%edx), %zmm5, %zmm6	 # AVX512VBMI2 Disp8
	vpshrdvd	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512VBMI2 Disp8

	vpshrdvq	%zmm4, %zmm5, %zmm6	 # AVX512VBMI2
	vpshrdvq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpshrdvq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2
	vpshrdvq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI2
	vpshrdvq	128(%edx), %zmm5, %zmm6	 # AVX512VBMI2 Disp8
	vpshrdvq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512VBMI2 Disp8

	vpshldw	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpshldw	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2
	vpshldw	$123, %zmm4, %zmm5, %zmm6	 # AVX512VBMI2
	vpshldw	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI2
	vpshldw	$123, 128(%edx), %zmm5, %zmm6	 # AVX512VBMI2 Disp8

	vpshldd	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpshldd	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2
	vpshldd	$123, %zmm4, %zmm5, %zmm6	 # AVX512VBMI2
	vpshldd	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI2
	vpshldd	$123, 128(%edx), %zmm5, %zmm6	 # AVX512VBMI2 Disp8
	vpshldd	$123, 508(%edx){1to16}, %zmm5, %zmm6	 # AVX512VBMI2 Disp8

	vpshldq	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpshldq	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2
	vpshldq	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI2
	vpshldq	$123, 128(%edx), %zmm5, %zmm6	 # AVX512VBMI2 Disp8
	vpshldq	$123, 1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512VBMI2 Disp8

	vpshrdw	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512VBMI2
	vpshrdw	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpshrdw	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2
	vpshrdw	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI2
	vpshrdw	$123, 128(%edx), %zmm5, %zmm6	 # AVX512VBMI2 Disp8

	vpshrdd	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512VBMI2
	vpshrdd	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpshrdd	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2
	vpshrdd	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI2
	vpshrdd	$123, 128(%edx), %zmm5, %zmm6	 # AVX512VBMI2 Disp8
	vpshrdd	$123, 508(%edx){1to16}, %zmm5, %zmm6	 # AVX512VBMI2 Disp8

	vpshrdq	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512VBMI2
	vpshrdq	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512VBMI2
	vpshrdq	$123, %zmm4, %zmm5, %zmm6	 # AVX512VBMI2
	vpshrdq	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512VBMI2
	vpshrdq	$123, 128(%edx), %zmm5, %zmm6	 # AVX512VBMI2 Disp8
	vpshrdq	$123, 1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512VBMI2 Disp8

	.intel_syntax noprefix
	vpcompressb	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512VBMI2
	vpcompressb	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512VBMI2
	vpcompressb	ZMMWORD PTR [edx+126], zmm6	 # AVX512VBMI2 Disp8
	vpcompressb	zmm6, zmm5	 # AVX512VBMI2
	vpcompressb	zmm6{k7}, zmm5	 # AVX512VBMI2
	vpcompressb	zmm6{k7}{z}, zmm5	 # AVX512VBMI2

	vpcompressw	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512VBMI2
	vpcompressw	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512VBMI2
	vpcompressw	ZMMWORD PTR [edx+128], zmm6	 # AVX512VBMI2 Disp8
	vpcompressw	zmm6, zmm5	 # AVX512VBMI2
	vpcompressw	zmm6{k7}, zmm5	 # AVX512VBMI2
	vpcompressw	zmm6{k7}{z}, zmm5	 # AVX512VBMI2

	vpexpandb	zmm6{k7}, ZMMWORD PTR [ecx]	 # AVX512VBMI2
	vpexpandb	zmm6{k7}{z}, ZMMWORD PTR [ecx]	 # AVX512VBMI2
	vpexpandb	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VBMI2
	vpexpandb	zmm6, ZMMWORD PTR [edx+126]	 # AVX512VBMI2 Disp8
	vpexpandb	zmm6, zmm5	 # AVX512VBMI2
	vpexpandb	zmm6{k7}, zmm5	 # AVX512VBMI2
	vpexpandb	zmm6{k7}{z}, zmm5	 # AVX512VBMI2

	vpexpandw	zmm6{k7}, ZMMWORD PTR [ecx]	 # AVX512VBMI2
	vpexpandw	zmm6{k7}{z}, ZMMWORD PTR [ecx]	 # AVX512VBMI2
	vpexpandw	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VBMI2
	vpexpandw	zmm6, ZMMWORD PTR [edx+128]	 # AVX512VBMI2 Disp8
	vpexpandw	zmm6, zmm5	 # AVX512VBMI2
	vpexpandw	zmm6{k7}, zmm5	 # AVX512VBMI2
	vpexpandw	zmm6{k7}{z}, zmm5	 # AVX512VBMI2

	vpshldvw	zmm6, zmm5, zmm4	 # AVX512VBMI2
	vpshldvw	zmm6{k7}, zmm5, zmm4	 # AVX512VBMI2
	vpshldvw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512VBMI2
	vpshldvw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VBMI2
	vpshldvw	zmm6, zmm5, ZMMWORD PTR [edx+128]	 # AVX512VBMI2 Disp8

	vpshldvd	zmm6, zmm5, zmm4	 # AVX512VBMI2
	vpshldvd	zmm6{k7}, zmm5, zmm4	 # AVX512VBMI2
	vpshldvd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512VBMI2
	vpshldvd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VBMI2
	vpshldvd	zmm6, zmm5, ZMMWORD PTR [edx+128]	 # AVX512VBMI2 Disp8
	vpshldvd	zmm6, zmm5, [edx+508]{1to16}	 # AVX512VBMI2 Disp8

	vpshldvq	zmm6, zmm5, zmm4	 # AVX512VBMI2
	vpshldvq	zmm6{k7}, zmm5, zmm4	 # AVX512VBMI2
	vpshldvq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512VBMI2
	vpshldvq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VBMI2
	vpshldvq	zmm6, zmm5, ZMMWORD PTR [edx+128]	 # AVX512VBMI2 Disp8
	vpshldvq	zmm6, zmm5, [edx+1016]{1to8}	 # AVX512VBMI2 Disp8

	vpshrdvw	zmm6, zmm5, zmm4	 # AVX512VBMI2
	vpshrdvw	zmm6{k7}, zmm5, zmm4	 # AVX512VBMI2
	vpshrdvw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512VBMI2
	vpshrdvw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VBMI2
	vpshrdvw	zmm6, zmm5, ZMMWORD PTR [edx+128]	 # AVX512VBMI2 Disp8

	vpshrdvd	zmm6, zmm5, zmm4	 # AVX512VBMI2
	vpshrdvd	zmm6{k7}, zmm5, zmm4	 # AVX512VBMI2
	vpshrdvd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512VBMI2
	vpshrdvd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VBMI2
	vpshrdvd	zmm6, zmm5, ZMMWORD PTR [edx+128]	 # AVX512VBMI2 Disp8
	vpshrdvd	zmm6, zmm5, [edx+508]{1to16}	 # AVX512VBMI2 Disp8

	vpshrdvq	zmm6, zmm5, zmm4	 # AVX512VBMI2
	vpshrdvq	zmm6{k7}, zmm5, zmm4	 # AVX512VBMI2
	vpshrdvq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512VBMI2
	vpshrdvq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512VBMI2
	vpshrdvq	zmm6, zmm5, ZMMWORD PTR [edx+128]	 # AVX512VBMI2 Disp8
	vpshrdvq	zmm6, zmm5, [edx+1016]{1to8}	 # AVX512VBMI2 Disp8

	vpshldw	zmm6, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshldw	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshldw	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshldw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512VBMI2
	vpshldw	zmm6, zmm5, ZMMWORD PTR [edx+128], 123	 # AVX512VBMI2 Disp8

	vpshldd	zmm6, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshldd	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshldd	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshldd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512VBMI2
	vpshldd	zmm6, zmm5, ZMMWORD PTR [edx+128], 123	 # AVX512VBMI2 Disp8
	vpshldd	zmm6, zmm5, [edx+508]{1to16}, 123	 # AVX512VBMI2 Disp8

	vpshldq	zmm6, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshldq	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshldq	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshldq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512VBMI2
	vpshldq	zmm6, zmm5, ZMMWORD PTR [edx+128], 123	 # AVX512VBMI2 Disp8
	vpshldq	zmm6, zmm5, [edx+1016]{1to8}, 123	 # AVX512VBMI2 Disp8

	vpshrdw	zmm6, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshrdw	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshrdw	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshrdw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512VBMI2
	vpshrdw	zmm6, zmm5, ZMMWORD PTR [edx+128], 123	 # AVX512VBMI2 Disp8

	vpshrdd	zmm6, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshrdd	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshrdd	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshrdd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512VBMI2
	vpshrdd	zmm6, zmm5, ZMMWORD PTR [edx+128], 123	 # AVX512VBMI2 Disp8
	vpshrdd	zmm6, zmm5, [edx+508]{1to16}, 123	 # AVX512VBMI2 Disp8

	vpshrdq	zmm6, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshrdq	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshrdq	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512VBMI2
	vpshrdq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512VBMI2
	vpshrdq	zmm6, zmm5, ZMMWORD PTR [edx+128], 123	 # AVX512VBMI2 Disp8
	vpshrdq	zmm6, zmm5, [edx+1016]{1to8}, 123	 # AVX512VBMI2 Disp8
