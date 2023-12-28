# Check 64bit AVX512{VBMI2,VL} instructions

	.allow_index_reg
	.text
_start:
	vpcompressb	%xmm30, (%rcx){%k7}	 # AVX512{VBMI2,VL}
	vpcompressb	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{VBMI2,VL}
	vpcompressb	%xmm30, 127(%rdx)	 # AVX512{VBMI2,VL} Disp8
	vpcompressb	%ymm30, (%rcx){%k7}	 # AVX512{VBMI2,VL}
	vpcompressb	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{VBMI2,VL}
	vpcompressb	%ymm30, 127(%rdx)	 # AVX512{VBMI2,VL} Disp8
	vpcompressb	%xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpcompressb	%xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpcompressb	%xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpcompressb	%ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpcompressb	%ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpcompressb	%ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}

	vpcompressw	%xmm30, (%rcx){%k7}	 # AVX512{VBMI2,VL}
	vpcompressw	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{VBMI2,VL}
	vpcompressw	%xmm30, 254(%rdx)	 # AVX512{VBMI2,VL} Disp8
	vpcompressw	%ymm30, (%rcx){%k7}	 # AVX512{VBMI2,VL}
	vpcompressw	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{VBMI2,VL}
	vpcompressw	%ymm30, 254(%rdx)	 # AVX512{VBMI2,VL} Disp8
	vpcompressw	%xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpcompressw	%xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpcompressw	%xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpcompressw	%ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpcompressw	%ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpcompressw	%ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}

	vpexpandb	(%rcx), %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpexpandb	(%rcx), %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpexpandb	0x123(%rax,%r14,8), %xmm30	 # AVX512{VBMI2,VL}
	vpexpandb	127(%rdx), %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpexpandb	(%rcx), %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpexpandb	(%rcx), %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpexpandb	0x123(%rax,%r14,8), %ymm30	 # AVX512{VBMI2,VL}
	vpexpandb	127(%rdx), %ymm30	 # AVX512{VBMI2,VL} Disp8
	vpexpandb	%xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpexpandb	%xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpexpandb	%xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpexpandb	%ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpexpandb	%ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpexpandb	%ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}

	vpexpandw	(%rcx), %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpexpandw	(%rcx), %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpexpandw	0x123(%rax,%r14,8), %xmm30	 # AVX512{VBMI2,VL}
	vpexpandw	254(%rdx), %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpexpandw	(%rcx), %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpexpandw	(%rcx), %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpexpandw	0x123(%rax,%r14,8), %ymm30	 # AVX512{VBMI2,VL}
	vpexpandw	254(%rdx), %ymm30	 # AVX512{VBMI2,VL} Disp8
	vpexpandw	%xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpexpandw	%xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpexpandw	%xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpexpandw	%ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpexpandw	%ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpexpandw	%ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}

	vpshldvw	%xmm28, %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshldvw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpshldvw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldvw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshldvw	2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshldvw	%ymm28, %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshldvw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpshldvw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldvw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshldvw	4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8

	vpshldvd	%xmm28, %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshldvd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpshldvd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldvd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshldvd	2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshldvd	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshldvd	%ymm28, %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshldvd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpshldvd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldvd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshldvd	4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8
	vpshldvd	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8

	vpshldvq	%xmm28, %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshldvq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpshldvq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldvq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshldvq	2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshldvq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshldvq	%ymm28, %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshldvq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpshldvq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldvq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshldvq	4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8
	vpshldvq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8

	vpshrdvw	%xmm28, %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshrdvw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdvw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshrdvw	2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshrdvw	%ymm28, %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshrdvw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdvw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshrdvw	4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8

	vpshrdvd	%xmm28, %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshrdvd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdvd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshrdvd	2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshrdvd	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshrdvd	%ymm28, %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshrdvd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdvd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshrdvd	4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8
	vpshrdvd	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8

	vpshrdvq	%xmm28, %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshrdvq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdvq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshrdvq	2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshrdvq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshrdvq	%ymm28, %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshrdvq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpshrdvq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdvq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshrdvq	4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8
	vpshrdvq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8

	vpshldw	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshldw	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpshldw	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldw	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshldw	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshldw	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshldw	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpshldw	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldw	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshldw	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8

	vpshldd	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshldd	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpshldd	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshldd	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshldd	$123, 508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshldd	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshldd	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpshldd	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldd	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshldd	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8
	vpshldd	$123, 508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8

	vpshldq	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshldq	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpshldq	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldq	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshldq	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshldq	$123, 1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshldq	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshldq	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpshldq	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshldq	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshldq	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8
	vpshldq	$123, 1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8

	vpshrdw	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshrdw	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpshrdw	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdw	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshrdw	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshrdw	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshrdw	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpshrdw	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdw	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshrdw	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8

	vpshrdd	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshrdd	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpshrdd	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshrdd	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshrdd	$123, 508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshrdd	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshrdd	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpshrdd	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdd	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshrdd	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8
	vpshrdd	$123, 508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8

	vpshrdq	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshrdq	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI2,VL}
	vpshrdq	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdq	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI2,VL}
	vpshrdq	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshrdq	$123, 1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{VBMI2,VL} Disp8
	vpshrdq	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshrdq	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI2,VL}
	vpshrdq	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI2,VL}
	vpshrdq	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI2,VL}
	vpshrdq	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8
	vpshrdq	$123, 1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{VBMI2,VL} Disp8

	.intel_syntax noprefix
	vpcompressb	XMMWORD PTR [rcx]{k7}, xmm30	 # AVX512{VBMI2,VL}
	vpcompressb	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{VBMI2,VL}
	vpcompressb	XMMWORD PTR [rdx+127], xmm30	 # AVX512{VBMI2,VL} Disp8
	vpcompressb	YMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{VBMI2,VL}
	vpcompressb	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{VBMI2,VL}
	vpcompressb	YMMWORD PTR [rdx+127], ymm30	 # AVX512{VBMI2,VL} Disp8
	vpcompressb	xmm30, xmm29	 # AVX512{VBMI2,VL}
	vpcompressb	xmm30{k7}, xmm29	 # AVX512{VBMI2,VL}
	vpcompressb	xmm30{k7}{z}, xmm29	 # AVX512{VBMI2,VL}
	vpcompressb	ymm30, ymm29	 # AVX512{VBMI2,VL}
	vpcompressb	ymm30{k7}, ymm29	 # AVX512{VBMI2,VL}
	vpcompressb	ymm30{k7}{z}, ymm29	 # AVX512{VBMI2,VL}

	vpcompressw	XMMWORD PTR [rcx]{k7}, xmm30	 # AVX512{VBMI2,VL}
	vpcompressw	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{VBMI2,VL}
	vpcompressw	XMMWORD PTR [rdx+254], xmm30	 # AVX512{VBMI2,VL} Disp8
	vpcompressw	YMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{VBMI2,VL}
	vpcompressw	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{VBMI2,VL}
	vpcompressw	YMMWORD PTR [rdx+254], ymm30	 # AVX512{VBMI2,VL} Disp8
	vpcompressw	xmm30, xmm29	 # AVX512{VBMI2,VL}
	vpcompressw	xmm30{k7}, xmm29	 # AVX512{VBMI2,VL}
	vpcompressw	xmm30{k7}{z}, xmm29	 # AVX512{VBMI2,VL}
	vpcompressw	ymm30, ymm29	 # AVX512{VBMI2,VL}
	vpcompressw	ymm30{k7}, ymm29	 # AVX512{VBMI2,VL}
	vpcompressw	ymm30{k7}{z}, ymm29	 # AVX512{VBMI2,VL}

	vpexpandb	xmm30{k7}, XMMWORD PTR [rcx]	 # AVX512{VBMI2,VL}
	vpexpandb	xmm30{k7}{z}, XMMWORD PTR [rcx]	 # AVX512{VBMI2,VL}
	vpexpandb	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpexpandb	xmm30, XMMWORD PTR [rdx+127]	 # AVX512{VBMI2,VL} Disp8
	vpexpandb	ymm30{k7}, YMMWORD PTR [rcx]	 # AVX512{VBMI2,VL}
	vpexpandb	ymm30{k7}{z}, YMMWORD PTR [rcx]	 # AVX512{VBMI2,VL}
	vpexpandb	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpexpandb	ymm30, YMMWORD PTR [rdx+127]	 # AVX512{VBMI2,VL} Disp8
	vpexpandb	xmm30, xmm29	 # AVX512{VBMI2,VL}
	vpexpandb	xmm30{k7}, xmm29	 # AVX512{VBMI2,VL}
	vpexpandb	xmm30{k7}{z}, xmm29	 # AVX512{VBMI2,VL}
	vpexpandb	ymm30, ymm29	 # AVX512{VBMI2,VL}
	vpexpandb	ymm30{k7}, ymm29	 # AVX512{VBMI2,VL}
	vpexpandb	ymm30{k7}{z}, ymm29	 # AVX512{VBMI2,VL}

	vpexpandw	xmm30{k7}, XMMWORD PTR [rcx]	 # AVX512{VBMI2,VL}
	vpexpandw	xmm30{k7}{z}, XMMWORD PTR [rcx]	 # AVX512{VBMI2,VL}
	vpexpandw	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpexpandw	xmm30, XMMWORD PTR [rdx+254]	 # AVX512{VBMI2,VL} Disp8
	vpexpandw	ymm30{k7}, YMMWORD PTR [rcx]	 # AVX512{VBMI2,VL}
	vpexpandw	ymm30{k7}{z}, YMMWORD PTR [rcx]	 # AVX512{VBMI2,VL}
	vpexpandw	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpexpandw	ymm30, YMMWORD PTR [rdx+254]	 # AVX512{VBMI2,VL} Disp8
	vpexpandw	xmm30, xmm29	 # AVX512{VBMI2,VL}
	vpexpandw	xmm30{k7}, xmm29	 # AVX512{VBMI2,VL}
	vpexpandw	xmm30{k7}{z}, xmm29	 # AVX512{VBMI2,VL}
	vpexpandw	ymm30, ymm29	 # AVX512{VBMI2,VL}
	vpexpandw	ymm30{k7}, ymm29	 # AVX512{VBMI2,VL}
	vpexpandw	ymm30{k7}{z}, ymm29	 # AVX512{VBMI2,VL}

	vpshldvw	xmm30, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshldvw	xmm30{k7}, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshldvw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshldvw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpshldvw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{VBMI2,VL} Disp8
	vpshldvw	ymm30, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshldvw	ymm30{k7}, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshldvw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshldvw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpshldvw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{VBMI2,VL} Disp8

	vpshldvd	xmm30, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshldvd	xmm30{k7}, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshldvd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshldvd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpshldvd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{VBMI2,VL} Disp8
	vpshldvd	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{VBMI2,VL} Disp8
	vpshldvd	ymm30, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshldvd	ymm30{k7}, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshldvd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshldvd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpshldvd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{VBMI2,VL} Disp8
	vpshldvd	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{VBMI2,VL} Disp8

	vpshldvq	xmm30, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshldvq	xmm30{k7}, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshldvq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshldvq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpshldvq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{VBMI2,VL} Disp8
	vpshldvq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{VBMI2,VL} Disp8
	vpshldvq	ymm30, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshldvq	ymm30{k7}, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshldvq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshldvq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpshldvq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{VBMI2,VL} Disp8
	vpshldvq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{VBMI2,VL} Disp8

	vpshrdvw	xmm30, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshrdvw	xmm30{k7}, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshrdvw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshrdvw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpshrdvw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{VBMI2,VL} Disp8
	vpshrdvw	ymm30, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshrdvw	ymm30{k7}, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshrdvw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshrdvw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpshrdvw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{VBMI2,VL} Disp8

	vpshrdvd	xmm30, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshrdvd	xmm30{k7}, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshrdvd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshrdvd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpshrdvd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{VBMI2,VL} Disp8
	vpshrdvd	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{VBMI2,VL} Disp8
	vpshrdvd	ymm30, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshrdvd	ymm30{k7}, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshrdvd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshrdvd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpshrdvd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{VBMI2,VL} Disp8
	vpshrdvd	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{VBMI2,VL} Disp8

	vpshrdvq	xmm30, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshrdvq	xmm30{k7}, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshrdvq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{VBMI2,VL}
	vpshrdvq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpshrdvq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{VBMI2,VL} Disp8
	vpshrdvq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{VBMI2,VL} Disp8
	vpshrdvq	ymm30, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshrdvq	ymm30{k7}, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshrdvq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{VBMI2,VL}
	vpshrdvq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI2,VL}
	vpshrdvq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{VBMI2,VL} Disp8
	vpshrdvq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{VBMI2,VL} Disp8

	vpshldw	xmm30, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldw	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldw	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{VBMI2,VL}
	vpshldw	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{VBMI2,VL} Disp8
	vpshldw	ymm30, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldw	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldw	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{VBMI2,VL}
	vpshldw	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{VBMI2,VL} Disp8

	vpshldd	xmm30, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldd	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldd	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{VBMI2,VL}
	vpshldd	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{VBMI2,VL} Disp8
	vpshldd	xmm30, xmm29, [rdx+508]{1to4}, 123	 # AVX512{VBMI2,VL} Disp8
	vpshldd	ymm30, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldd	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldd	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{VBMI2,VL}
	vpshldd	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{VBMI2,VL} Disp8
	vpshldd	ymm30, ymm29, [rdx+508]{1to8}, 123	 # AVX512{VBMI2,VL} Disp8

	vpshldq	xmm30, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldq	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldq	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{VBMI2,VL}
	vpshldq	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{VBMI2,VL} Disp8
	vpshldq	xmm30, xmm29, [rdx+1016]{1to2}, 123	 # AVX512{VBMI2,VL} Disp8
	vpshldq	ymm30, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldq	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldq	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshldq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{VBMI2,VL}
	vpshldq	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{VBMI2,VL} Disp8
	vpshldq	ymm30, ymm29, [rdx+1016]{1to4}, 123	 # AVX512{VBMI2,VL} Disp8

	vpshrdw	xmm30, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdw	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdw	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{VBMI2,VL}
	vpshrdw	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdw	ymm30, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdw	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdw	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{VBMI2,VL}
	vpshrdw	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{VBMI2,VL} Disp8

	vpshrdd	xmm30, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdd	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdd	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{VBMI2,VL}
	vpshrdd	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdd	xmm30, xmm29, [rdx+508]{1to4}, 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdd	ymm30, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdd	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdd	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{VBMI2,VL}
	vpshrdd	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdd	ymm30, ymm29, [rdx+508]{1to8}, 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdq	xmm30, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdq	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdq	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{VBMI2,VL}
	vpshrdq	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdq	xmm30, xmm29, [rdx+1016]{1to2}, 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdq	ymm30, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdq	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdq	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{VBMI2,VL}
	vpshrdq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{VBMI2,VL}
	vpshrdq	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{VBMI2,VL} Disp8
	vpshrdq	ymm30, ymm29, [rdx+1016]{1to4}, 123	 # AVX512{VBMI2,VL} Disp8
