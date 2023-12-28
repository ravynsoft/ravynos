#as: -march=armv8.6-a+bf16+sve
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

0+ <\.text>:
 *[0-9a-f]+:	647b82b1 	bfdot	z17\.s, z21\.h, z27\.h
 *[0-9a-f]+:	64608000 	bfdot	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	647d42b1 	bfdot	z17\.s, z21\.h, z5\.h\[3\]
 *[0-9a-f]+:	64784000 	bfdot	z0\.s, z0\.h, z0\.h\[3\]
 *[0-9a-f]+:	64604000 	bfdot	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	647be6b1 	bfmmla	z17\.s, z21\.h, z27\.h
 *[0-9a-f]+:	6460e400 	bfmmla	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	658ab6b1 	bfcvt	z17\.h, p5/m, z21\.s
 *[0-9a-f]+:	658aa000 	bfcvt	z0\.h, p0/m, z0\.s
 *[0-9a-f]+:	648ab6b1 	bfcvtnt	z17\.h, p5/m, z21\.s
 *[0-9a-f]+:	648aa000 	bfcvtnt	z0\.h, p0/m, z0\.s
 *[0-9a-f]+:	64fb86b1 	bfmlalt	z17\.s, z21\.h, z27\.h
 *[0-9a-f]+:	64e08400 	bfmlalt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	64fb82b1 	bfmlalb	z17\.s, z21\.h, z27\.h
 *[0-9a-f]+:	64e08000 	bfmlalb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	64e546b1 	bfmlalt	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	64f84c00 	bfmlalt	z0\.s, z0\.h, z0\.h\[7\]
 *[0-9a-f]+:	64e542b1 	bfmlalb	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	64f84800 	bfmlalb	z0\.s, z0\.h, z0\.h\[7\]
 *[0-9a-f]+:	2e5bfeb1 	bfdot	v17\.2s, v21\.4h, v27\.4h
 *[0-9a-f]+:	2e40fc00 	bfdot	v0\.2s, v0\.4h, v0\.4h
 *[0-9a-f]+:	6e5bfeb1 	bfdot	v17\.4s, v21\.8h, v27\.8h
 *[0-9a-f]+:	6e40fc00 	bfdot	v0\.4s, v0\.8h, v0\.8h
 *[0-9a-f]+:	0f7bfab1 	bfdot	v17\.2s, v21\.4h, v27\.2h\[3\]
 *[0-9a-f]+:	0f60f800 	bfdot	v0\.2s, v0\.4h, v0\.2h\[3\]
 *[0-9a-f]+:	4f7bfab1 	bfdot	v17\.4s, v21\.8h, v27\.2h\[3\]
 *[0-9a-f]+:	4f60f800 	bfdot	v0\.4s, v0\.8h, v0\.2h\[3\]
 *[0-9a-f]+:	0f5bf2b1 	bfdot	v17\.2s, v21\.4h, v27\.2h\[0\]
 *[0-9a-f]+:	0f40f000 	bfdot	v0\.2s, v0\.4h, v0\.2h\[0\]
 *[0-9a-f]+:	4f5bf2b1 	bfdot	v17\.4s, v21\.8h, v27\.2h\[0\]
 *[0-9a-f]+:	4f40f000 	bfdot	v0\.4s, v0\.8h, v0\.2h\[0\]
 *[0-9a-f]+:	6e5beeb1 	bfmmla	v17\.4s, v21\.8h, v27\.8h
 *[0-9a-f]+:	6e40ec00 	bfmmla	v0\.4s, v0\.8h, v0\.8h
 *[0-9a-f]+:	2edbfeb1 	bfmlalb	v17\.4s, v21\.8h, v27\.8h
 *[0-9a-f]+:	2ec0fc00 	bfmlalb	v0\.4s, v0\.8h, v0\.8h
 *[0-9a-f]+:	6edbfeb1 	bfmlalt	v17\.4s, v21\.8h, v27\.8h
 *[0-9a-f]+:	6ec0fc00 	bfmlalt	v0\.4s, v0\.8h, v0\.8h
 *[0-9a-f]+:	0fcff2b1 	bfmlalb	v17\.4s, v21\.8h, v15\.h\[0\]
 *[0-9a-f]+:	0ff0f800 	bfmlalb	v0\.4s, v0\.8h, v0\.h\[7\]
 *[0-9a-f]+:	4fcff2b1 	bfmlalt	v17\.4s, v21\.8h, v15\.h\[0\]
 *[0-9a-f]+:	4ff0f800 	bfmlalt	v0\.4s, v0\.8h, v0\.h\[7\]
 *[0-9a-f]+:	0ea16ab1 	bfcvtn	v17\.4h, v21\.4s
 *[0-9a-f]+:	0ea16800 	bfcvtn	v0\.4h, v0\.4s
 *[0-9a-f]+:	4ea16ab1 	bfcvtn2	v17\.8h, v21\.4s
 *[0-9a-f]+:	4ea16800 	bfcvtn2	v0\.8h, v0\.4s
 *[0-9a-f]+:	1e6342b1 	bfcvt	h17, s21
 *[0-9a-f]+:	1e634000 	bfcvt	h0, s0
