# Test CRYPTO instructions
	.text
	md5
	sha1
	sha256
	sha512
	crc32c		%f2, %f4, %f6
	aes_kexpand0	%f4, %f6, %f8
	aes_kexpand1	%f6, %f8, 0x7, %f10
	aes_kexpand1	%f6, %f8, 6, %f10
	aes_kexpand2	%f8, %f10, %f12
	aes_eround01	%f10, %f12, %f14, %f16
	aes_eround23	%f12, %f14, %f16, %f18
	aes_dround01	%f14, %f16, %f18, %f20
	aes_dround23	%f16, %f18, %f20, %f22
	aes_eround01_l	%f18, %f20, %f22, %f24
	aes_eround23_l	%f20, %f22, %f24, %f26
	aes_dround01_l	%f22, %f24, %f26, %f28
	aes_dround23_l	%f24, %f26, %f28, %f30
	des_ip		%f32, %f34
	des_iip		%f34, %f36
	des_kexpand	%f36, 7, %f38
	des_round	%f38, %f40, %f42, %f44
	kasumi_fi_fi	%f42, %f44, %f46
	kasumi_fl_xor	%f44, %f46, %f48, %f50
	kasumi_fi_xor	%f46, %f48, %f50, %f52
	camellia_fl	%f50, %f52, %f54
	camellia_fli	%f52, %f54, %f56
	camellia_f	%f54, %f56, %f58, %f60
	mpmul	0
	mpmul	1
	mpmul	2
	mpmul	3
	mpmul	4
	mpmul	5
	mpmul	6
	mpmul	7
	mpmul	8
	mpmul	9
	mpmul	10
	mpmul	11
	mpmul	12
	mpmul	13
	mpmul	14
	mpmul	15
	mpmul	16
	mpmul	17
	mpmul	18
	mpmul	19
	mpmul	20
	mpmul	21
	mpmul	22
	mpmul	23
	mpmul	24
	mpmul	25
	mpmul	26
	mpmul	27
	mpmul	28
	mpmul	29
	mpmul	30
	mpmul	31
	montmul	0
	montmul	1
	montmul	2
	montmul	3
	montmul	4
	montmul	5
	montmul	6
	montmul	7
	montmul	8
	montmul	9
	montmul	10
	montmul	11
	montmul	12
	montmul	13
	montmul	14
	montmul	15
	montmul	16
	montmul	17
	montmul	18
	montmul	19
	montmul	20
	montmul	21
	montmul	22
	montmul	23
	montmul	24
	montmul	25
	montmul	26
	montmul	27
	montmul	28
	montmul	29
	montmul	30
	montmul	31
	montsqr	0
	montsqr	1
	montsqr	2
	montsqr	3
	montsqr	4
	montsqr	5
	montsqr	6
	montsqr	7
	montsqr	8
	montsqr	9
	montsqr	10
	montsqr	11
	montsqr	12
	montsqr	13
	montsqr	14
	montsqr	15
	montsqr	16
	montsqr	17
	montsqr	18
	montsqr	19
	montsqr	20
	montsqr	21
	montsqr	22
	montsqr	23
	montsqr	24
	montsqr	25
	montsqr	26
	montsqr	27
	montsqr	28
	montsqr	29
	montsqr	30
	montsqr	31
