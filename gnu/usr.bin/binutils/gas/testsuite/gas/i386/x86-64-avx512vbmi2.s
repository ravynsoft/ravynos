# Check 64bit AVX512VBMI2 instructions

	.allow_index_reg
	.text
_start:
	vpcompressb	%zmm30, (%rcx){%k7}	 # AVX512VBMI2
	vpcompressb	%zmm30, 0x123(%rax,%r14,8)	 # AVX512VBMI2
	vpcompressb	%zmm30, 126(%rdx)	 # AVX512VBMI2 Disp8
	vpcompressb	%zmm29, %zmm30	 # AVX512VBMI2
	vpcompressb	%zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpcompressb	%zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2

	vpcompressw	%zmm30, (%rcx){%k7}	 # AVX512VBMI2
	vpcompressw	%zmm30, 0x123(%rax,%r14,8)	 # AVX512VBMI2
	vpcompressw	%zmm30, 254(%rdx)	 # AVX512VBMI2 Disp8
	vpcompressw	%zmm29, %zmm30	 # AVX512VBMI2
	vpcompressw	%zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpcompressw	%zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2

	vpexpandb	(%rcx), %zmm30{%k7}	 # AVX512VBMI2
	vpexpandb	(%rcx), %zmm30{%k7}{z}	 # AVX512VBMI2
	vpexpandb	0x123(%rax,%r14,8), %zmm30	 # AVX512VBMI2
	vpexpandb	126(%rdx), %zmm30	 # AVX512VBMI2 Disp8
	vpexpandb	%zmm29, %zmm30	 # AVX512VBMI2
	vpexpandb	%zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpexpandb	%zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2

	vpexpandw	(%rcx), %zmm30{%k7}	 # AVX512VBMI2
	vpexpandw	(%rcx), %zmm30{%k7}{z}	 # AVX512VBMI2
	vpexpandw	0x123(%rax,%r14,8), %zmm30	 # AVX512VBMI2
	vpexpandw	254(%rdx), %zmm30	 # AVX512VBMI2 Disp8
	vpexpandw	%zmm29, %zmm30	 # AVX512VBMI2
	vpexpandw	%zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpexpandw	%zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2

	vpshldvw	%zmm28, %zmm29, %zmm30	 # AVX512VBMI2
	vpshldvw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpshldvw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2
	vpshldvw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI2
	vpshldvw	8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI2

	vpshldvd	%zmm28, %zmm29, %zmm30	 # AVX512VBMI2
	vpshldvd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpshldvd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2
	vpshldvd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI2
	vpshldvd	8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI2
	vpshldvd	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512VBMI2 Disp8

	vpshldvq	%zmm28, %zmm29, %zmm30	 # AVX512VBMI2
	vpshldvq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpshldvq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2
	vpshldvq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI2
	vpshldvq	8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI2
	vpshldvq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512VBMI2 Disp8

	vpshrdvw	%zmm28, %zmm29, %zmm30	 # AVX512VBMI2
	vpshrdvw	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpshrdvw	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2
	vpshrdvw	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI2
	vpshrdvw	8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI2

	vpshrdvd	%zmm28, %zmm29, %zmm30	 # AVX512VBMI2
	vpshrdvd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpshrdvd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2
	vpshrdvd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI2
	vpshrdvd	8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI2

	vpshrdvq	%zmm28, %zmm29, %zmm30	 # AVX512VBMI2
	vpshrdvq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpshrdvq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2
	vpshrdvq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI2
	vpshrdvq	8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI2

	vpshldw	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512VBMI2
	vpshldw	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpshldw	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2
	vpshldw	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI2
	vpshldw	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI2

	vpshldd	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512VBMI2
	vpshldd	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpshldd	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2
	vpshldd	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI2
	vpshldd	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI2

	vpshldq	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512VBMI2
	vpshldq	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpshldq	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2
	vpshldq	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI2
	vpshldq	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI2
	vpshldq	$123, 1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512VBMI2

	vpshrdw	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512VBMI2
	vpshrdw	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpshrdw	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2
	vpshrdw	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI2
	vpshrdw	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI2

	vpshrdd	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512VBMI2
	vpshrdd	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpshrdd	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2
	vpshrdd	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI2
	vpshrdd	$123, (%rcx){1to16}, %zmm29, %zmm30	 # AVX512VBMI2
	vpshrdd	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI2

	vpshrdq	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512VBMI2
	vpshrdq	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512VBMI2
	vpshrdq	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512VBMI2
	vpshrdq	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512VBMI2
	vpshrdq	$123, (%rcx){1to8}, %zmm29, %zmm30	 # AVX512VBMI2
	vpshrdq	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512VBMI2

	.intel_syntax noprefix
	vpcompressb	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512VBMI2
	vpcompressb	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512VBMI2
	vpcompressb	ZMMWORD PTR [rdx+126], zmm30	 # AVX512VBMI2 Disp8
	vpcompressb	zmm30, zmm29	 # AVX512VBMI2
	vpcompressb	zmm30{k7}, zmm29	 # AVX512VBMI2
	vpcompressb	zmm30{k7}{z}, zmm29	 # AVX512VBMI2

	vpcompressw	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512VBMI2
	vpcompressw	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512VBMI2
	vpcompressw	ZMMWORD PTR [rdx+254], zmm30	 # AVX512VBMI2 Disp8
	vpcompressw	zmm30, zmm29	 # AVX512VBMI2
	vpcompressw	zmm30{k7}, zmm29	 # AVX512VBMI2
	vpcompressw	zmm30{k7}{z}, zmm29	 # AVX512VBMI2

	vpexpandb	zmm30{k7}, ZMMWORD PTR [rcx]	 # AVX512VBMI2
	vpexpandb	zmm30{k7}{z}, ZMMWORD PTR [rcx]	 # AVX512VBMI2
	vpexpandb	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VBMI2
	vpexpandb	zmm30, ZMMWORD PTR [rdx+126]	 # AVX512VBMI2 Disp8
	vpexpandb	zmm30, zmm29	 # AVX512VBMI2
	vpexpandb	zmm30{k7}, zmm29	 # AVX512VBMI2
	vpexpandb	zmm30{k7}{z}, zmm29	 # AVX512VBMI2

	vpexpandw	zmm30{k7}, ZMMWORD PTR [rcx]	 # AVX512VBMI2
	vpexpandw	zmm30{k7}{z}, ZMMWORD PTR [rcx]	 # AVX512VBMI2
	vpexpandw	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VBMI2
	vpexpandw	zmm30, ZMMWORD PTR [rdx+254]	 # AVX512VBMI2 Disp8
	vpexpandw	zmm30, zmm29	 # AVX512VBMI2
	vpexpandw	zmm30{k7}, zmm29	 # AVX512VBMI2
	vpexpandw	zmm30{k7}{z}, zmm29	 # AVX512VBMI2

	vpshldvw	zmm30, zmm29, zmm28	 # AVX512VBMI2
	vpshldvw	zmm30{k7}, zmm29, zmm28	 # AVX512VBMI2
	vpshldvw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512VBMI2
	vpshldvw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VBMI2
	vpshldvw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512VBMI2 Disp8

	vpshldvd	zmm30, zmm29, zmm28	 # AVX512VBMI2
	vpshldvd	zmm30{k7}, zmm29, zmm28	 # AVX512VBMI2
	vpshldvd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512VBMI2
	vpshldvd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VBMI2
	vpshldvd	zmm30, zmm29, [rcx]{1to16}	 # AVX512VBMI2
	vpshldvd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512VBMI2 Disp8
	vpshldvd	zmm30, zmm29, [rdx+508]{1to16}	 # AVX512VBMI2 Disp8

	vpshldvq	zmm30, zmm29, zmm28	 # AVX512VBMI2
	vpshldvq	zmm30{k7}, zmm29, zmm28	 # AVX512VBMI2
	vpshldvq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512VBMI2
	vpshldvq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VBMI2
	vpshldvq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512VBMI2 Disp8
	vpshldvq	zmm30, zmm29, [rdx+1016]{1to8}	 # AVX512VBMI2 Disp8

	vpshrdvw	zmm30, zmm29, zmm28	 # AVX512VBMI2
	vpshrdvw	zmm30{k7}, zmm29, zmm28	 # AVX512VBMI2
	vpshrdvw	zmm30{k7}{z}, zmm29, zmm28	 # AVX512VBMI2
	vpshrdvw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VBMI2
	vpshrdvw	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512VBMI2 Disp8

	vpshrdvd	zmm30, zmm29, zmm28	 # AVX512VBMI2
	vpshrdvd	zmm30{k7}, zmm29, zmm28	 # AVX512VBMI2
	vpshrdvd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512VBMI2
	vpshrdvd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VBMI2
	vpshrdvd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512VBMI2 Disp8
	vpshrdvd	zmm30, zmm29, [rdx+508]{1to16}	 # AVX512VBMI2 Disp8

	vpshrdvq	zmm30, zmm29, zmm28	 # AVX512VBMI2
	vpshrdvq	zmm30{k7}, zmm29, zmm28	 # AVX512VBMI2
	vpshrdvq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512VBMI2
	vpshrdvq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VBMI2
	vpshrdvq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512VBMI2 Disp8
	vpshrdvq	zmm30, zmm29, [rdx+1016]{1to8}	 # AVX512VBMI2 Disp8

	vpshldw	zmm30, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshldw	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshldw	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshldw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512VBMI2
	vpshldw	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512VBMI2 Disp8

	vpshldd	zmm30, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshldd	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshldd	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshldd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512VBMI2
	vpshldd	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512VBMI2 Disp8
	vpshldd	zmm30, zmm29, [rdx+508]{1to16}, 123	 # AVX512VBMI2 Disp8

	vpshldq	zmm30, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshldq	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshldq	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshldq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512VBMI2
	vpshldq	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512VBMI2 Disp8
	vpshldq	zmm30, zmm29, [rdx+1016]{1to8}, 123	 # AVX512VBMI2 Disp8

	vpshrdw	zmm30, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshrdw	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshrdw	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshrdw	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512VBMI2
	vpshrdw	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512VBMI2 Disp8

	vpshrdd	zmm30, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshrdd	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshrdd	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshrdd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512VBMI2
	vpshrdd	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512VBMI2 Disp8
	vpshrdd	zmm30, zmm29, [rdx+508]{1to16}, 123	 # AVX512VBMI2 Disp8

	vpshrdq	zmm30, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshrdq	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshrdq	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512VBMI2
	vpshrdq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512VBMI2
	vpshrdq	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512VBMI2 Disp8
	vpshrdq	zmm30, zmm29, [rdx+1016]{1to8}, 123	 # AVX512VBMI2 Disp8
