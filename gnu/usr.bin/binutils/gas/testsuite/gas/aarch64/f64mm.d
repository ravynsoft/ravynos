#as: -march=armv8-a+sve+f64mm
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <\.text>:
 *[0-9a-f]+:	64fbe6b1 	fmmla	z17\.d, z21\.d, z27\.d
 *[0-9a-f]+:	64e0e400 	fmmla	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	a43b17f1 	ld1rob	{z17\.b}, p5/z, \[sp, x27\]
 *[0-9a-f]+:	a42003e0 	ld1rob	{z0\.b}, p0/z, \[sp, x0\]
 *[0-9a-f]+:	a4bb17f1 	ld1roh	{z17\.h}, p5/z, \[sp, x27, lsl #1\]
 *[0-9a-f]+:	a4a003e0 	ld1roh	{z0\.h}, p0/z, \[sp, x0, lsl #1\]
 *[0-9a-f]+:	a53b17f1 	ld1row	{z17\.s}, p5/z, \[sp, x27, lsl #2\]
 *[0-9a-f]+:	a52003e0 	ld1row	{z0\.s}, p0/z, \[sp, x0, lsl #2\]
 *[0-9a-f]+:	a5bb17f1 	ld1rod	{z17\.d}, p5/z, \[sp, x27, lsl #3\]
 *[0-9a-f]+:	a5a003e0 	ld1rod	{z0\.d}, p0/z, \[sp, x0, lsl #3\]
 *[0-9a-f]+:	a43b1411 	ld1rob	{z17\.b}, p5/z, \[x0, x27\]
 *[0-9a-f]+:	a4200000 	ld1rob	{z0\.b}, p0/z, \[x0, x0\]
 *[0-9a-f]+:	a4bb1411 	ld1roh	{z17\.h}, p5/z, \[x0, x27, lsl #1\]
 *[0-9a-f]+:	a4a00000 	ld1roh	{z0\.h}, p0/z, \[x0, x0, lsl #1\]
 *[0-9a-f]+:	a53b1411 	ld1row	{z17\.s}, p5/z, \[x0, x27, lsl #2\]
 *[0-9a-f]+:	a5200000 	ld1row	{z0\.s}, p0/z, \[x0, x0, lsl #2\]
 *[0-9a-f]+:	a5bb1411 	ld1rod	{z17\.d}, p5/z, \[x0, x27, lsl #3\]
 *[0-9a-f]+:	a5a00000 	ld1rod	{z0\.d}, p0/z, \[x0, x0, lsl #3\]
 *[0-9a-f]+:	a42037f1 	ld1rob	{z17\.b}, p5/z, \[sp\]
 *[0-9a-f]+:	a42723e0 	ld1rob	{z0\.b}, p0/z, \[sp, #224\]
 *[0-9a-f]+:	a42823e0 	ld1rob	{z0\.b}, p0/z, \[sp, #-256\]
 *[0-9a-f]+:	a4a037f1 	ld1roh	{z17\.h}, p5/z, \[sp\]
 *[0-9a-f]+:	a4a723e0 	ld1roh	{z0\.h}, p0/z, \[sp, #224\]
 *[0-9a-f]+:	a4a823e0 	ld1roh	{z0\.h}, p0/z, \[sp, #-256\]
 *[0-9a-f]+:	a52037f1 	ld1row	{z17\.s}, p5/z, \[sp\]
 *[0-9a-f]+:	a52723e0 	ld1row	{z0\.s}, p0/z, \[sp, #224\]
 *[0-9a-f]+:	a52823e0 	ld1row	{z0\.s}, p0/z, \[sp, #-256\]
 *[0-9a-f]+:	a5a037f1 	ld1rod	{z17\.d}, p5/z, \[sp\]
 *[0-9a-f]+:	a5a723e0 	ld1rod	{z0\.d}, p0/z, \[sp, #224\]
 *[0-9a-f]+:	a5a823e0 	ld1rod	{z0\.d}, p0/z, \[sp, #-256\]
 *[0-9a-f]+:	a4203411 	ld1rob	{z17\.b}, p5/z, \[x0\]
 *[0-9a-f]+:	a4272000 	ld1rob	{z0\.b}, p0/z, \[x0, #224\]
 *[0-9a-f]+:	a4282000 	ld1rob	{z0\.b}, p0/z, \[x0, #-256\]
 *[0-9a-f]+:	a4a03411 	ld1roh	{z17\.h}, p5/z, \[x0\]
 *[0-9a-f]+:	a4a72000 	ld1roh	{z0\.h}, p0/z, \[x0, #224\]
 *[0-9a-f]+:	a4a82000 	ld1roh	{z0\.h}, p0/z, \[x0, #-256\]
 *[0-9a-f]+:	a5203411 	ld1row	{z17\.s}, p5/z, \[x0\]
 *[0-9a-f]+:	a5272000 	ld1row	{z0\.s}, p0/z, \[x0, #224\]
 *[0-9a-f]+:	a5282000 	ld1row	{z0\.s}, p0/z, \[x0, #-256\]
 *[0-9a-f]+:	a5a03411 	ld1rod	{z17\.d}, p5/z, \[x0\]
 *[0-9a-f]+:	a5a72000 	ld1rod	{z0\.d}, p0/z, \[x0, #224\]
 *[0-9a-f]+:	a5a82000 	ld1rod	{z0\.d}, p0/z, \[x0, #-256\]
 *[0-9a-f]+:	05a502b1 	zip1	z17\.q, z21\.q, z5\.q
 *[0-9a-f]+:	05a00000 	zip1	z0\.q, z0\.q, z0\.q
 *[0-9a-f]+:	05a506b1 	zip2	z17\.q, z21\.q, z5\.q
 *[0-9a-f]+:	05a00400 	zip2	z0\.q, z0\.q, z0\.q
 *[0-9a-f]+:	05a50ab1 	uzp1	z17\.q, z21\.q, z5\.q
 *[0-9a-f]+:	05a00800 	uzp1	z0\.q, z0\.q, z0\.q
 *[0-9a-f]+:	05a50eb1 	uzp2	z17\.q, z21\.q, z5\.q
 *[0-9a-f]+:	05a00c00 	uzp2	z0\.q, z0\.q, z0\.q
 *[0-9a-f]+:	05a51ab1 	trn1	z17\.q, z21\.q, z5\.q
 *[0-9a-f]+:	05a01800 	trn1	z0\.q, z0\.q, z0\.q
 *[0-9a-f]+:	05a51eb1 	trn2	z17\.q, z21\.q, z5\.q
 *[0-9a-f]+:	05a01c00 	trn2	z0\.q, z0\.q, z0\.q
