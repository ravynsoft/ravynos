#name: Armv8.1-M Mainline Security Extensions instructions
#source: archv8m_1m-cmse-main.s
#as: -march=armv8.1-m.main -mimplicit-it=always
#objdump: -dr --prefix-addresses --show-raw-insn -marmv8.1-m.main

.*: +file format .*arm.*

Disassembly of section .text:
0+.* <[^>]*> e89f 0005 	clrm	{r0, r2}
0+.* <[^>]*> e89f 8000 	clrm	{APSR}
0+.* <[^>]*> e89f 8008 	clrm	{r3, APSR}
0+.* <[^>]*> bf08      	it	eq
0+.* <[^>]*> e89f 0010 	clrmeq	{r4}
0+.* <[^>]*> ec9f 0b00 	vscclrm	{VPR}
0+.* <[^>]*> ec9f fa01 	vscclrm	{s30, VPR}
0+.* <[^>]*> ec9f eb02 	vscclrm	{d14, VPR}
0+.* <[^>]*> ecdf 0a04 	vscclrm	{s1-s4, VPR}
0+.* <[^>]*> ec9f 1b08 	vscclrm	{d1-d4, VPR}
0+.* <[^>]*> ec9f 0a20 	vscclrm	{s0-s31, VPR}
0+.* <[^>]*> ec9f 0b20 	vscclrm	{d0-d15, VPR}
0+.* <[^>]*> bf18      	it	ne
0+.* <[^>]*> ecdf 1a01 	vscclrmne	{s3, VPR}
0+.* <[^>]*> ed92 2f80 	vldr	FPSCR, \[r2\]
0+.* <[^>]*> ed92 2f82 	vldr	FPSCR, \[r2, #8\]
0+.* <[^>]*> ed92 2f82 	vldr	FPSCR, \[r2, #8\]
0+.* <[^>]*> ed12 2f82 	vldr	FPSCR, \[r2, #-8\]
0+.* <[^>]*> edb2 2f82 	vldr	FPSCR, \[r2, #8\]!
0+.* <[^>]*> edb2 2f82 	vldr	FPSCR, \[r2, #8\]!
0+.* <[^>]*> ed32 2f82 	vldr	FPSCR, \[r2, #-8\]!
0+.* <[^>]*> ecb2 2f82 	vldr	FPSCR, \[r2\], #8
0+.* <[^>]*> ecb2 2f82 	vldr	FPSCR, \[r2\], #8
0+.* <[^>]*> ec32 2f82 	vldr	FPSCR, \[r2\], #-8
0+.* <[^>]*> ed93 4f80 	vldr	FPSCR_nzcvqc, \[r3\]
0+.* <[^>]*> edd3 8f80 	vldr	VPR, \[r3\]
0+.* <[^>]*> edd3 af80 	vldr	P0, \[r3\]
0+.* <[^>]*> edd3 cf80 	vldr	FPCXTNS, \[r3\]
0+.* <[^>]*> edd3 ef80 	vldr	FPCXTS, \[r3\]
0+.* <[^>]*> bfa8      	it	ge
0+.* <[^>]*> edd3 ef80 	vldrge	FPCXTS, \[r3\]
0+.* <[^>]*> ed82 2f80 	vstr	FPSCR, \[r2\]
0+.* <[^>]*> ed82 2f82 	vstr	FPSCR, \[r2, #8\]
0+.* <[^>]*> ed82 2f82 	vstr	FPSCR, \[r2, #8\]
0+.* <[^>]*> ed02 2f82 	vstr	FPSCR, \[r2, #-8\]
0+.* <[^>]*> eda2 2f82 	vstr	FPSCR, \[r2, #8\]!
0+.* <[^>]*> eda2 2f82 	vstr	FPSCR, \[r2, #8\]!
0+.* <[^>]*> ed22 2f82 	vstr	FPSCR, \[r2, #-8\]!
0+.* <[^>]*> eca2 2f82 	vstr	FPSCR, \[r2\], #8
0+.* <[^>]*> eca2 2f82 	vstr	FPSCR, \[r2\], #8
0+.* <[^>]*> ec22 2f82 	vstr	FPSCR, \[r2\], #-8
0+.* <[^>]*> ed83 4f80 	vstr	FPSCR_nzcvqc, \[r3\]
0+.* <[^>]*> edc3 8f80 	vstr	VPR, \[r3\]
0+.* <[^>]*> edc3 af80 	vstr	P0, \[r3\]
0+.* <[^>]*> edc3 cf80 	vstr	FPCXTNS, \[r3\]
0+.* <[^>]*> edc3 ef80 	vstr	FPCXTS, \[r3\]
0+.* <[^>]*> bfa8      	it	ge
0+.* <[^>]*> edc3 ef80 	vstrge	FPCXTS, \[r3\]
#...
