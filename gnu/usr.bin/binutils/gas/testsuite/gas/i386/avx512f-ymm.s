	.text
	.arch generic32
	.arch .avx512f
ymm:
	vpmovzxbd	%xmm0, %zmm0
	vpmovzxwd	%ymm0, %zmm0

	vcvtps2pd	%ymm0, %zmm0
	vcvtpd2ps	%zmm0, %ymm0
