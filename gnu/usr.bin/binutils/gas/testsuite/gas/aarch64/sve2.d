#as: -march=armv8-a+sve2+sve2-aes+sve2-sm4+sve2-sha3+sve2-bitperm
#objdump: -dr

[^:]+:     file format .*


Disassembly of section \.text:

0+ <\.text>:
 *[0-9a-f]+:	0420bc20 	movprfx	z0, z1
 *[0-9a-f]+:	4542d020 	adclb	z0\.d, z1\.d, z2\.d
 *[0-9a-f]+:	451bd2b1 	adclb	z17\.s, z21\.s, z27\.s
 *[0-9a-f]+:	4500d000 	adclb	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	4540d000 	adclb	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	451bd6b1 	adclt	z17\.s, z21\.s, z27\.s
 *[0-9a-f]+:	4500d400 	adclt	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	4540d400 	adclt	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	457b62b1 	addhnb	z17\.b, z21\.h, z27\.h
 *[0-9a-f]+:	45606000 	addhnb	z0\.b, z0\.h, z0\.h
 *[0-9a-f]+:	45a06000 	addhnb	z0\.h, z0\.s, z0\.s
 *[0-9a-f]+:	45e06000 	addhnb	z0\.s, z0\.d, z0\.d
 *[0-9a-f]+:	457b66b1 	addhnt	z17\.b, z21\.h, z27\.h
 *[0-9a-f]+:	45606400 	addhnt	z0\.b, z0\.h, z0\.h
 *[0-9a-f]+:	45a06400 	addhnt	z0\.h, z0\.s, z0\.s
 *[0-9a-f]+:	45e06400 	addhnt	z0\.s, z0\.d, z0\.d
 *[0-9a-f]+:	04d12020 	movprfx	z0\.d, p0/m, z1\.d
 *[0-9a-f]+:	44d1a020 	addp	z0\.d, p0/m, z0\.d, z1\.d
 *[0-9a-f]+:	4411b6b1 	addp	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	4411a000 	addp	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	4451a000 	addp	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	4491a000 	addp	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44d1a000 	addp	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	4522e6b1 	aesd	z17\.b, z17\.b, z21\.b
 *[0-9a-f]+:	4522e400 	aesd	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	4522e2b1 	aese	z17\.b, z17\.b, z21\.b
 *[0-9a-f]+:	4522e000 	aese	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	4520e411 	aesimc	z17\.b, z17\.b
 *[0-9a-f]+:	4520e400 	aesimc	z0\.b, z0\.b
 *[0-9a-f]+:	4520e011 	aesmc	z17\.b, z17\.b
 *[0-9a-f]+:	4520e000 	aesmc	z0\.b, z0\.b
 *[0-9a-f]+:	04753b71 	bcax	z17\.d, z17\.d, z21\.d, z27\.d
 *[0-9a-f]+:	04603800 	bcax	z0\.d, z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	04353f71 	bsl	z17\.d, z17\.d, z21\.d, z27\.d
 *[0-9a-f]+:	04203c00 	bsl	z0\.d, z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	04753f71 	bsl1n	z17\.d, z17\.d, z21\.d, z27\.d
 *[0-9a-f]+:	04603c00 	bsl1n	z0\.d, z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	04b53f71 	bsl2n	z17\.d, z17\.d, z21\.d, z27\.d
 *[0-9a-f]+:	04a03c00 	bsl2n	z0\.d, z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	451bb6b1 	bdep	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	4500b400 	bdep	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	4540b400 	bdep	z0\.h, z0\.h, z0\.h
 *[0-9a-f]+:	4580b400 	bdep	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	45c0b400 	bdep	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	451bb2b1 	bext	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	4500b000 	bext	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	4540b000 	bext	z0\.h, z0\.h, z0\.h
 *[0-9a-f]+:	4580b000 	bext	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	45c0b000 	bext	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	451bbab1 	bgrp	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	4500b800 	bgrp	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	4540b800 	bgrp	z0\.h, z0\.h, z0\.h
 *[0-9a-f]+:	4580b800 	bgrp	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	45c0b800 	bgrp	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	4500dab1 	cadd	z17\.b, z17\.b, z21\.b, #90
 *[0-9a-f]+:	4500d800 	cadd	z0\.b, z0\.b, z0\.b, #90
 *[0-9a-f]+:	4540d800 	cadd	z0\.h, z0\.h, z0\.h, #90
 *[0-9a-f]+:	4580d800 	cadd	z0\.s, z0\.s, z0\.s, #90
 *[0-9a-f]+:	45c0d800 	cadd	z0\.d, z0\.d, z0\.d, #90
 *[0-9a-f]+:	4500dc00 	cadd	z0\.b, z0\.b, z0\.b, #270
 *[0-9a-f]+:	44bb42b1 	cdot	z17\.s, z21\.b, z3\.b\[3\], #0
 *[0-9a-f]+:	44a04000 	cdot	z0\.s, z0\.b, z0\.b\[0\], #0
 *[0-9a-f]+:	44a04400 	cdot	z0\.s, z0\.b, z0\.b\[0\], #90
 *[0-9a-f]+:	44a04800 	cdot	z0\.s, z0\.b, z0\.b\[0\], #180
 *[0-9a-f]+:	44a04c00 	cdot	z0\.s, z0\.b, z0\.b\[0\], #270
 *[0-9a-f]+:	44fb42b1 	cdot	z17\.d, z21\.h, z11\.h\[1\], #0
 *[0-9a-f]+:	44e04000 	cdot	z0\.d, z0\.h, z0\.h\[0\], #0
 *[0-9a-f]+:	44e04400 	cdot	z0\.d, z0\.h, z0\.h\[0\], #90
 *[0-9a-f]+:	44e04800 	cdot	z0\.d, z0\.h, z0\.h\[0\], #180
 *[0-9a-f]+:	44e04c00 	cdot	z0\.d, z0\.h, z0\.h\[0\], #270
 *[0-9a-f]+:	449b12b1 	cdot	z17\.s, z21\.b, z27\.b, #0
 *[0-9a-f]+:	44801000 	cdot	z0\.s, z0\.b, z0\.b, #0
 *[0-9a-f]+:	44c01000 	cdot	z0\.d, z0\.h, z0\.h, #0
 *[0-9a-f]+:	44801400 	cdot	z0\.s, z0\.b, z0\.b, #90
 *[0-9a-f]+:	44801800 	cdot	z0\.s, z0\.b, z0\.b, #180
 *[0-9a-f]+:	44801c00 	cdot	z0\.s, z0\.b, z0\.b, #270
 *[0-9a-f]+:	44bb62b1 	cmla	z17\.h, z21\.h, z3\.h\[3\], #0
 *[0-9a-f]+:	44a06000 	cmla	z0\.h, z0\.h, z0\.h\[0\], #0
 *[0-9a-f]+:	44a06400 	cmla	z0\.h, z0\.h, z0\.h\[0\], #90
 *[0-9a-f]+:	44a06800 	cmla	z0\.h, z0\.h, z0\.h\[0\], #180
 *[0-9a-f]+:	44a06c00 	cmla	z0\.h, z0\.h, z0\.h\[0\], #270
 *[0-9a-f]+:	44fb62b1 	cmla	z17\.s, z21\.s, z11\.s\[1\], #0
 *[0-9a-f]+:	44e06000 	cmla	z0\.s, z0\.s, z0\.s\[0\], #0
 *[0-9a-f]+:	44e06400 	cmla	z0\.s, z0\.s, z0\.s\[0\], #90
 *[0-9a-f]+:	44e06800 	cmla	z0\.s, z0\.s, z0\.s\[0\], #180
 *[0-9a-f]+:	44e06c00 	cmla	z0\.s, z0\.s, z0\.s\[0\], #270
 *[0-9a-f]+:	441b22b1 	cmla	z17\.b, z21\.b, z27\.b, #0
 *[0-9a-f]+:	44002000 	cmla	z0\.b, z0\.b, z0\.b, #0
 *[0-9a-f]+:	44402000 	cmla	z0\.h, z0\.h, z0\.h, #0
 *[0-9a-f]+:	44802000 	cmla	z0\.s, z0\.s, z0\.s, #0
 *[0-9a-f]+:	44c02000 	cmla	z0\.d, z0\.d, z0\.d, #0
 *[0-9a-f]+:	44002400 	cmla	z0\.b, z0\.b, z0\.b, #90
 *[0-9a-f]+:	44002800 	cmla	z0\.b, z0\.b, z0\.b, #180
 *[0-9a-f]+:	44002c00 	cmla	z0\.b, z0\.b, z0\.b, #270
 *[0-9a-f]+:	04353b71 	eor3	z17\.d, z17\.d, z21\.d, z27\.d
 *[0-9a-f]+:	04203800 	eor3	z0\.d, z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	451b92b1 	eorbt	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	45009000 	eorbt	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	45409000 	eorbt	z0\.h, z0\.h, z0\.h
 *[0-9a-f]+:	45809000 	eorbt	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	45c09000 	eorbt	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	451b96b1 	eortb	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	45009400 	eortb	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	45409400 	eortb	z0\.h, z0\.h, z0\.h
 *[0-9a-f]+:	45809400 	eortb	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	45c09400 	eortb	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	057b16b1 	ext	z17\.b, {z21\.b-z22\.b}, #221
 *[0-9a-f]+:	05600000 	ext	z0\.b, {z0\.b-z1\.b}, #0
 *[0-9a-f]+:	056003e0 	ext	z0\.b, {z31\.b-z0\.b}, #0
 *[0-9a-f]+:	645096b1 	faddp	z17\.h, p5/m, z17\.h, z21\.h
 *[0-9a-f]+:	64508000 	faddp	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	64908000 	faddp	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	64d08000 	faddp	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	6489b6b1 	fcvtlt	z17\.s, p5/m, z21\.h
 *[0-9a-f]+:	6489a000 	fcvtlt	z0\.s, p0/m, z0\.h
 *[0-9a-f]+:	64cbb6b1 	fcvtlt	z17\.d, p5/m, z21\.s
 *[0-9a-f]+:	64cba000 	fcvtlt	z0\.d, p0/m, z0\.s
 *[0-9a-f]+:	6488b6b1 	fcvtnt	z17\.h, p5/m, z21\.s
 *[0-9a-f]+:	6488a000 	fcvtnt	z0\.h, p0/m, z0\.s
 *[0-9a-f]+:	64cab6b1 	fcvtnt	z17\.s, p5/m, z21\.d
 *[0-9a-f]+:	64caa000 	fcvtnt	z0\.s, p0/m, z0\.d
 *[0-9a-f]+:	650ab6b1 	fcvtx	z17\.s, p5/m, z21\.d
 *[0-9a-f]+:	650aa000 	fcvtx	z0\.s, p0/m, z0\.d
 *[0-9a-f]+:	04d02020 	movprfx	z0\.d, p0/z, z1\.d
 *[0-9a-f]+:	650aa040 	fcvtx	z0\.s, p0/m, z2\.d
 *[0-9a-f]+:	640ab6b1 	fcvtxnt	z17\.s, p5/m, z21\.d
 *[0-9a-f]+:	640aa000 	fcvtxnt	z0\.s, p0/m, z0\.d
 *[0-9a-f]+:	651ab6b1 	flogb	z17\.h, p5/m, z21\.h
 *[0-9a-f]+:	651aa000 	flogb	z0\.h, p0/m, z0\.h
 *[0-9a-f]+:	651ca000 	flogb	z0\.s, p0/m, z0\.s
 *[0-9a-f]+:	651ea000 	flogb	z0\.d, p0/m, z0\.d
 *[0-9a-f]+:	645496b1 	fmaxnmp	z17\.h, p5/m, z17\.h, z21\.h
 *[0-9a-f]+:	64548000 	fmaxnmp	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	64948000 	fmaxnmp	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	64d48000 	fmaxnmp	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	645696b1 	fmaxp	z17\.h, p5/m, z17\.h, z21\.h
 *[0-9a-f]+:	64568000 	fmaxp	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	64968000 	fmaxp	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	64d68000 	fmaxp	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	645596b1 	fminnmp	z17\.h, p5/m, z17\.h, z21\.h
 *[0-9a-f]+:	64558000 	fminnmp	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	64958000 	fminnmp	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	64d58000 	fminnmp	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	645796b1 	fminp	z17\.h, p5/m, z17\.h, z21\.h
 *[0-9a-f]+:	64578000 	fminp	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	64978000 	fminp	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	64d78000 	fminp	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	64a542b1 	fmlalb	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	64b04800 	fmlalb	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	64a04000 	fmlalb	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	64bb82b1 	fmlalb	z17\.s, z21\.h, z27\.h
 *[0-9a-f]+:	64a08000 	fmlalb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	64a546b1 	fmlalt	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	64b04c00 	fmlalt	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	64a04400 	fmlalt	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	64bb86b1 	fmlalt	z17\.s, z21\.h, z27\.h
 *[0-9a-f]+:	64a08400 	fmlalt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	64a562b1 	fmlslb	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	64b06800 	fmlslb	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	64a06000 	fmlslb	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	64bba2b1 	fmlslb	z17\.s, z21\.h, z27\.h
 *[0-9a-f]+:	64a0a000 	fmlslb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	64a566b1 	fmlslt	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	64b06c00 	fmlslt	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	64a06400 	fmlslt	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	64bba6b1 	fmlslt	z17\.s, z21\.h, z27\.h
 *[0-9a-f]+:	64a0a400 	fmlslt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45bbd6b1 	histcnt	z17\.s, p5/z, z21\.s, z27\.s
 *[0-9a-f]+:	45a0c000 	histcnt	z0\.s, p0/z, z0\.s, z0\.s
 *[0-9a-f]+:	45e0c000 	histcnt	z0\.d, p0/z, z0\.d, z0\.d
 *[0-9a-f]+:	453ba2b1 	histseg	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	4520a000 	histseg	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	c41bd6b1 	ldnt1b	{z17\.d}, p5/z, \[z21\.d, x27\]
 *[0-9a-f]+:	c400c000 	ldnt1b	{z0\.d}, p0/z, \[z0\.d, x0\]
 *[0-9a-f]+:	c41fc000 	ldnt1b	{z0\.d}, p0/z, \[z0\.d, xzr\]
 *[0-9a-f]+:	c41fc000 	ldnt1b	{z0\.d}, p0/z, \[z0\.d, xzr\]
 *[0-9a-f]+:	841bb6b1 	ldnt1b	{z17\.s}, p5/z, \[z21\.s, x27\]
 *[0-9a-f]+:	8400a000 	ldnt1b	{z0\.s}, p0/z, \[z0\.s, x0\]
 *[0-9a-f]+:	841fa000 	ldnt1b	{z0\.s}, p0/z, \[z0\.s, xzr\]
 *[0-9a-f]+:	841fa000 	ldnt1b	{z0\.s}, p0/z, \[z0\.s, xzr\]
 *[0-9a-f]+:	c59bd6b1 	ldnt1d	{z17\.d}, p5/z, \[z21\.d, x27\]
 *[0-9a-f]+:	c580c000 	ldnt1d	{z0\.d}, p0/z, \[z0\.d, x0\]
 *[0-9a-f]+:	c59fc000 	ldnt1d	{z0\.d}, p0/z, \[z0\.d, xzr\]
 *[0-9a-f]+:	c59fc000 	ldnt1d	{z0\.d}, p0/z, \[z0\.d, xzr\]
 *[0-9a-f]+:	c49bd6b1 	ldnt1h	{z17\.d}, p5/z, \[z21\.d, x27\]
 *[0-9a-f]+:	c480c000 	ldnt1h	{z0\.d}, p0/z, \[z0\.d, x0\]
 *[0-9a-f]+:	c49fc000 	ldnt1h	{z0\.d}, p0/z, \[z0\.d, xzr\]
 *[0-9a-f]+:	c49fc000 	ldnt1h	{z0\.d}, p0/z, \[z0\.d, xzr\]
 *[0-9a-f]+:	849bb6b1 	ldnt1h	{z17\.s}, p5/z, \[z21\.s, x27\]
 *[0-9a-f]+:	8480a000 	ldnt1h	{z0\.s}, p0/z, \[z0\.s, x0\]
 *[0-9a-f]+:	849fa000 	ldnt1h	{z0\.s}, p0/z, \[z0\.s, xzr\]
 *[0-9a-f]+:	849fa000 	ldnt1h	{z0\.s}, p0/z, \[z0\.s, xzr\]
 *[0-9a-f]+:	841b96b1 	ldnt1sb	{z17\.s}, p5/z, \[z21\.s, x27\]
 *[0-9a-f]+:	84008000 	ldnt1sb	{z0\.s}, p0/z, \[z0\.s, x0\]
 *[0-9a-f]+:	841f8000 	ldnt1sb	{z0\.s}, p0/z, \[z0\.s, xzr\]
 *[0-9a-f]+:	841f8000 	ldnt1sb	{z0\.s}, p0/z, \[z0\.s, xzr\]
 *[0-9a-f]+:	c4008000 	ldnt1sb	{z0\.d}, p0/z, \[z0\.d, x0\]
 *[0-9a-f]+:	c41f8000 	ldnt1sb	{z0\.d}, p0/z, \[z0\.d, xzr\]
 *[0-9a-f]+:	c41f8000 	ldnt1sb	{z0\.d}, p0/z, \[z0\.d, xzr\]
 *[0-9a-f]+:	849b96b1 	ldnt1sh	{z17\.s}, p5/z, \[z21\.s, x27\]
 *[0-9a-f]+:	84808000 	ldnt1sh	{z0\.s}, p0/z, \[z0\.s, x0\]
 *[0-9a-f]+:	849f8000 	ldnt1sh	{z0\.s}, p0/z, \[z0\.s, xzr\]
 *[0-9a-f]+:	849f8000 	ldnt1sh	{z0\.s}, p0/z, \[z0\.s, xzr\]
 *[0-9a-f]+:	c4808000 	ldnt1sh	{z0\.d}, p0/z, \[z0\.d, x0\]
 *[0-9a-f]+:	c49f8000 	ldnt1sh	{z0\.d}, p0/z, \[z0\.d, xzr\]
 *[0-9a-f]+:	c49f8000 	ldnt1sh	{z0\.d}, p0/z, \[z0\.d, xzr\]
 *[0-9a-f]+:	c51b96b1 	ldnt1sw	{z17\.d}, p5/z, \[z21\.d, x27\]
 *[0-9a-f]+:	c5008000 	ldnt1sw	{z0\.d}, p0/z, \[z0\.d, x0\]
 *[0-9a-f]+:	c51f8000 	ldnt1sw	{z0\.d}, p0/z, \[z0\.d, xzr\]
 *[0-9a-f]+:	c51f8000 	ldnt1sw	{z0\.d}, p0/z, \[z0\.d, xzr\]
 *[0-9a-f]+:	851bb6b1 	ldnt1w	{z17\.s}, p5/z, \[z21\.s, x27\]
 *[0-9a-f]+:	8500a000 	ldnt1w	{z0\.s}, p0/z, \[z0\.s, x0\]
 *[0-9a-f]+:	851fa000 	ldnt1w	{z0\.s}, p0/z, \[z0\.s, xzr\]
 *[0-9a-f]+:	851fa000 	ldnt1w	{z0\.s}, p0/z, \[z0\.s, xzr\]
 *[0-9a-f]+:	c51bd6b1 	ldnt1w	{z17\.d}, p5/z, \[z21\.d, x27\]
 *[0-9a-f]+:	c500c000 	ldnt1w	{z0\.d}, p0/z, \[z0\.d, x0\]
 *[0-9a-f]+:	c51fc000 	ldnt1w	{z0\.d}, p0/z, \[z0\.d, xzr\]
 *[0-9a-f]+:	c51fc000 	ldnt1w	{z0\.d}, p0/z, \[z0\.d, xzr\]
 *[0-9a-f]+:	45359629 	match	p9\.b, p5/z, z17\.b, z21\.b
 *[0-9a-f]+:	45358220 	match	p0\.b, p0/z, z17\.b, z21\.b
 *[0-9a-f]+:	45208000 	match	p0\.b, p0/z, z0\.b, z0\.b
 *[0-9a-f]+:	45608000 	match	p0\.h, p0/z, z0\.h, z0\.h
 *[0-9a-f]+:	443b0ab1 	mla	z17\.h, z21\.h, z3\.h\[3\]
 *[0-9a-f]+:	44600800 	mla	z0\.h, z0\.h, z0\.h\[4\]
 *[0-9a-f]+:	44200800 	mla	z0\.h, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44bb0ab1 	mla	z17\.s, z21\.s, z3\.s\[3\]
 *[0-9a-f]+:	44a00800 	mla	z0\.s, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	44fb0ab1 	mla	z17\.d, z21\.d, z11\.d\[1\]
 *[0-9a-f]+:	44e00800 	mla	z0\.d, z0\.d, z0\.d\[0\]
 *[0-9a-f]+:	443b0eb1 	mls	z17\.h, z21\.h, z3\.h\[3\]
 *[0-9a-f]+:	44600c00 	mls	z0\.h, z0\.h, z0\.h\[4\]
 *[0-9a-f]+:	44200c00 	mls	z0\.h, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44bb0eb1 	mls	z17\.s, z21\.s, z3\.s\[3\]
 *[0-9a-f]+:	44a00c00 	mls	z0\.s, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	44fb0eb1 	mls	z17\.d, z21\.d, z11\.d\[1\]
 *[0-9a-f]+:	44e00c00 	mls	z0\.d, z0\.d, z0\.d\[0\]
 *[0-9a-f]+:	443bfab1 	mul	z17\.h, z21\.h, z3\.h\[3\]
 *[0-9a-f]+:	4460f800 	mul	z0\.h, z0\.h, z0\.h\[4\]
 *[0-9a-f]+:	4420f800 	mul	z0\.h, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44bbfab1 	mul	z17\.s, z21\.s, z3\.s\[3\]
 *[0-9a-f]+:	44a0f800 	mul	z0\.s, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	44fbfab1 	mul	z17\.d, z21\.d, z11\.d\[1\]
 *[0-9a-f]+:	44e0f800 	mul	z0\.d, z0\.d, z0\.d\[0\]
 *[0-9a-f]+:	043b62b1 	mul	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	04206000 	mul	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	04606000 	mul	z0\.h, z0\.h, z0\.h
 *[0-9a-f]+:	04a06000 	mul	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	04e06000 	mul	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	453b96b9 	nmatch	p9\.b, p5/z, z21\.b, z27\.b
 *[0-9a-f]+:	45208010 	nmatch	p0\.b, p0/z, z0\.b, z0\.b
 *[0-9a-f]+:	45608010 	nmatch	p0\.h, p0/z, z0\.h, z0\.h
 *[0-9a-f]+:	04f53f71 	nbsl	z17\.d, z17\.d, z21\.d, z27\.d
 *[0-9a-f]+:	04e03c00 	nbsl	z0\.d, z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	043b66b1 	pmul	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	04206400 	pmul	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	451b6ab1 	pmullb	z17\.q, z21\.d, z27\.d
 *[0-9a-f]+:	45006800 	pmullb	z0\.q, z0\.d, z0\.d
 *[0-9a-f]+:	455b6ab1 	pmullb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45406800 	pmullb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45c06800 	pmullb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	451b6eb1 	pmullt	z17\.q, z21\.d, z27\.d
 *[0-9a-f]+:	45006c00 	pmullt	z0\.q, z0\.d, z0\.d
 *[0-9a-f]+:	455b6eb1 	pmullt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45406c00 	pmullt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45c06c00 	pmullt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	457b6ab1 	raddhnb	z17\.b, z21\.h, z27\.h
 *[0-9a-f]+:	45606800 	raddhnb	z0\.b, z0\.h, z0\.h
 *[0-9a-f]+:	45a06800 	raddhnb	z0\.h, z0\.s, z0\.s
 *[0-9a-f]+:	45e06800 	raddhnb	z0\.s, z0\.d, z0\.d
 *[0-9a-f]+:	457b6eb1 	raddhnt	z17\.b, z21\.h, z27\.h
 *[0-9a-f]+:	45606c00 	raddhnt	z0\.b, z0\.h, z0\.h
 *[0-9a-f]+:	45a06c00 	raddhnt	z0\.h, z0\.s, z0\.s
 *[0-9a-f]+:	45e06c00 	raddhnt	z0\.s, z0\.d, z0\.d
 *[0-9a-f]+:	453bf6b1 	rax1	z17\.d, z21\.d, z27\.d
 *[0-9a-f]+:	4520f400 	rax1	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	45291ab1 	rshrnb	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f1800 	rshrnb	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45281800 	rshrnb	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f1800 	rshrnb	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45311800 	rshrnb	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45301800 	rshrnb	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f1800 	rshrnb	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45611800 	rshrnb	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45601800 	rshrnb	z0\.s, z0\.d, #32
 *[0-9a-f]+:	45291eb1 	rshrnt	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f1c00 	rshrnt	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45281c00 	rshrnt	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f1c00 	rshrnt	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45311c00 	rshrnt	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45301c00 	rshrnt	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f1c00 	rshrnt	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45611c00 	rshrnt	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45601c00 	rshrnt	z0\.s, z0\.d, #32
 *[0-9a-f]+:	457b7ab1 	rsubhnb	z17\.b, z21\.h, z27\.h
 *[0-9a-f]+:	45607800 	rsubhnb	z0\.b, z0\.h, z0\.h
 *[0-9a-f]+:	45a07800 	rsubhnb	z0\.h, z0\.s, z0\.s
 *[0-9a-f]+:	45e07800 	rsubhnb	z0\.s, z0\.d, z0\.d
 *[0-9a-f]+:	457b7eb1 	rsubhnt	z17\.b, z21\.h, z27\.h
 *[0-9a-f]+:	45607c00 	rsubhnt	z0\.b, z0\.h, z0\.h
 *[0-9a-f]+:	45a07c00 	rsubhnt	z0\.h, z0\.s, z0\.s
 *[0-9a-f]+:	45e07c00 	rsubhnt	z0\.s, z0\.d, z0\.d
 *[0-9a-f]+:	451bfab1 	saba	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	4500f800 	saba	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	4540f800 	saba	z0\.h, z0\.h, z0\.h
 *[0-9a-f]+:	4580f800 	saba	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	45c0f800 	saba	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	455bc2b1 	sabalb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	4540c000 	sabalb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	4580c000 	sabalb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c0c000 	sabalb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455bc6b1 	sabalt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	4540c400 	sabalt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	4580c400 	sabalt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c0c400 	sabalt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455b32b1 	sabdlb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45403000 	sabdlb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45803000 	sabdlb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c03000 	sabdlb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455b36b1 	sabdlt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45403400 	sabdlt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45803400 	sabdlt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c03400 	sabdlt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	4444b6b1 	sadalp	z17\.h, p5/m, z21\.b
 *[0-9a-f]+:	4444a000 	sadalp	z0\.h, p0/m, z0\.b
 *[0-9a-f]+:	4484a000 	sadalp	z0\.s, p0/m, z0\.h
 *[0-9a-f]+:	44c4a000 	sadalp	z0\.d, p0/m, z0\.s
 *[0-9a-f]+:	455b02b1 	saddlb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45400000 	saddlb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45800000 	saddlb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c00000 	saddlb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455b82b1 	saddlbt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45408000 	saddlbt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45808000 	saddlbt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c08000 	saddlbt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455b06b1 	saddlt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45400400 	saddlt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45800400 	saddlt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c00400 	saddlt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455b42b1 	saddwb	z17\.h, z21\.h, z27\.b
 *[0-9a-f]+:	45404000 	saddwb	z0\.h, z0\.h, z0\.b
 *[0-9a-f]+:	45804000 	saddwb	z0\.s, z0\.s, z0\.h
 *[0-9a-f]+:	45c04000 	saddwb	z0\.d, z0\.d, z0\.s
 *[0-9a-f]+:	455b46b1 	saddwt	z17\.h, z21\.h, z27\.b
 *[0-9a-f]+:	45404400 	saddwt	z0\.h, z0\.h, z0\.b
 *[0-9a-f]+:	45804400 	saddwt	z0\.s, z0\.s, z0\.h
 *[0-9a-f]+:	45c04400 	saddwt	z0\.d, z0\.d, z0\.s
 *[0-9a-f]+:	459bd2b1 	sbclb	z17\.s, z21\.s, z27\.s
 *[0-9a-f]+:	4580d000 	sbclb	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	45c0d000 	sbclb	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	459bd6b1 	sbclt	z17\.s, z21\.s, z27\.s
 *[0-9a-f]+:	4580d400 	sbclt	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	45c0d400 	sbclt	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	441096b1 	shadd	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44108000 	shadd	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44508000 	shadd	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44908000 	shadd	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44d08000 	shadd	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	452912b1 	shrnb	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f1000 	shrnb	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45281000 	shrnb	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f1000 	shrnb	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45311000 	shrnb	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45301000 	shrnb	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f1000 	shrnb	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45611000 	shrnb	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45601000 	shrnb	z0\.s, z0\.d, #32
 *[0-9a-f]+:	452916b1 	shrnt	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f1400 	shrnt	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45281400 	shrnt	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f1400 	shrnt	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45311400 	shrnt	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45301400 	shrnt	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f1400 	shrnt	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45611400 	shrnt	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45601400 	shrnt	z0\.s, z0\.d, #32
 *[0-9a-f]+:	441296b1 	shsub	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44128000 	shsub	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44528000 	shsub	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44928000 	shsub	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44d28000 	shsub	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	441696b1 	shsubr	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44168000 	shsubr	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44568000 	shsubr	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44968000 	shsubr	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44d68000 	shsubr	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	4509f6b1 	sli	z17\.b, z21\.b, #1
 *[0-9a-f]+:	4508f400 	sli	z0\.b, z0\.b, #0
 *[0-9a-f]+:	450ff400 	sli	z0\.b, z0\.b, #7
 *[0-9a-f]+:	4510f400 	sli	z0\.h, z0\.h, #0
 *[0-9a-f]+:	451ff400 	sli	z0\.h, z0\.h, #15
 *[0-9a-f]+:	4540f400 	sli	z0\.s, z0\.s, #0
 *[0-9a-f]+:	455ff400 	sli	z0\.s, z0\.s, #31
 *[0-9a-f]+:	4580f400 	sli	z0\.d, z0\.d, #0
 *[0-9a-f]+:	45dff400 	sli	z0\.d, z0\.d, #63
 *[0-9a-f]+:	4523e2b1 	sm4e	z17\.s, z17\.s, z21\.s
 *[0-9a-f]+:	4523e000 	sm4e	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	453bf2b1 	sm4ekey	z17\.s, z21\.s, z27\.s
 *[0-9a-f]+:	4520f000 	sm4ekey	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	4414b6b1 	smaxp	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	4414a000 	smaxp	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	4454a000 	smaxp	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	4494a000 	smaxp	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44d4a000 	smaxp	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	4416b6b1 	sminp	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	4416a000 	sminp	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	4456a000 	sminp	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	4496a000 	sminp	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44d6a000 	sminp	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	44a582b1 	smlalb	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b08800 	smlalb	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a08000 	smlalb	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e982b1 	smlalb	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f08800 	smlalb	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e08000 	smlalb	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	445b42b1 	smlalb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	44404000 	smlalb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	44804000 	smlalb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	44c04000 	smlalb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	44a586b1 	smlalt	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b08c00 	smlalt	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a08400 	smlalt	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e986b1 	smlalt	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f08c00 	smlalt	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e08400 	smlalt	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	445b46b1 	smlalt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	44404400 	smlalt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	44804400 	smlalt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	44c04400 	smlalt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	44a5a2b1 	smlslb	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b0a800 	smlslb	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a0a000 	smlslb	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e9a2b1 	smlslb	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f0a800 	smlslb	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e0a000 	smlslb	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	445b52b1 	smlslb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	44405000 	smlslb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	44805000 	smlslb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	44c05000 	smlslb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	44a5a6b1 	smlslt	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b0ac00 	smlslt	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a0a400 	smlslt	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e9a6b1 	smlslt	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f0ac00 	smlslt	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e0a400 	smlslt	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	445b56b1 	smlslt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	44405400 	smlslt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	44805400 	smlslt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	44c05400 	smlslt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	043b6ab1 	smulh	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	04206800 	smulh	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	04606800 	smulh	z0\.h, z0\.h, z0\.h
 *[0-9a-f]+:	04a06800 	smulh	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	04e06800 	smulh	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	44a5c2b1 	smullb	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b0c800 	smullb	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a0c000 	smullb	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e9c2b1 	smullb	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f0c800 	smullb	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e0c000 	smullb	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	455b72b1 	smullb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45407000 	smullb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45807000 	smullb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c07000 	smullb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	44a5c6b1 	smullt	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b0cc00 	smullt	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a0c400 	smullt	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e9c6b1 	smullt	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f0cc00 	smullt	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e0c400 	smullt	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	455b76b1 	smullt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45407400 	smullt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45807400 	smullt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c07400 	smullt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	052d96b1 	splice	z17\.b, p5, {z21\.b-z22\.b}
 *[0-9a-f]+:	052d8000 	splice	z0\.b, p0, {z0\.b-z1\.b}
 *[0-9a-f]+:	056d8000 	splice	z0\.h, p0, {z0\.h-z1\.h}
 *[0-9a-f]+:	05ad8000 	splice	z0\.s, p0, {z0\.s-z1\.s}
 *[0-9a-f]+:	05ed8000 	splice	z0\.d, p0, {z0\.d-z1\.d}
 *[0-9a-f]+:	052d83e0 	splice	z0\.b, p0, {z31\.b-z0\.b}
 *[0-9a-f]+:	4408b6b1 	sqabs	z17\.b, p5/m, z21\.b
 *[0-9a-f]+:	4408a000 	sqabs	z0\.b, p0/m, z0\.b
 *[0-9a-f]+:	4448a000 	sqabs	z0\.h, p0/m, z0\.h
 *[0-9a-f]+:	4488a000 	sqabs	z0\.s, p0/m, z0\.s
 *[0-9a-f]+:	44c8a000 	sqabs	z0\.d, p0/m, z0\.d
 *[0-9a-f]+:	441896b1 	sqadd	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44188000 	sqadd	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44588000 	sqadd	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44988000 	sqadd	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44d88000 	sqadd	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	4501dab1 	sqcadd	z17\.b, z17\.b, z21\.b, #90
 *[0-9a-f]+:	4501dc00 	sqcadd	z0\.b, z0\.b, z0\.b, #270
 *[0-9a-f]+:	4501d800 	sqcadd	z0\.b, z0\.b, z0\.b, #90
 *[0-9a-f]+:	4541d800 	sqcadd	z0\.h, z0\.h, z0\.h, #90
 *[0-9a-f]+:	4581d800 	sqcadd	z0\.s, z0\.s, z0\.s, #90
 *[0-9a-f]+:	45c1d800 	sqcadd	z0\.d, z0\.d, z0\.d, #90
 *[0-9a-f]+:	44a522b1 	sqdmlalb	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b02800 	sqdmlalb	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a02000 	sqdmlalb	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e922b1 	sqdmlalb	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f02800 	sqdmlalb	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e02000 	sqdmlalb	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	445b62b1 	sqdmlalb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	44406000 	sqdmlalb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	44806000 	sqdmlalb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	44c06000 	sqdmlalb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	445b0ab1 	sqdmlalbt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	44400800 	sqdmlalbt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	44800800 	sqdmlalbt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	44c00800 	sqdmlalbt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	44a526b1 	sqdmlalt	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b02c00 	sqdmlalt	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a02400 	sqdmlalt	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e926b1 	sqdmlalt	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f02c00 	sqdmlalt	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e02400 	sqdmlalt	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	445b66b1 	sqdmlalt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	44406400 	sqdmlalt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	44806400 	sqdmlalt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	44c06400 	sqdmlalt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	44a532b1 	sqdmlslb	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b03800 	sqdmlslb	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a03000 	sqdmlslb	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e932b1 	sqdmlslb	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f03800 	sqdmlslb	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e03000 	sqdmlslb	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	445b6ab1 	sqdmlslb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	44406800 	sqdmlslb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	44806800 	sqdmlslb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	44c06800 	sqdmlslb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	445b0eb1 	sqdmlslbt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	44400c00 	sqdmlslbt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	44800c00 	sqdmlslbt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	44c00c00 	sqdmlslbt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	44a536b1 	sqdmlslt	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b03c00 	sqdmlslt	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a03400 	sqdmlslt	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e936b1 	sqdmlslt	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f03c00 	sqdmlslt	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e03400 	sqdmlslt	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	445b6eb1 	sqdmlslt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	44406c00 	sqdmlslt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	44806c00 	sqdmlslt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	44c06c00 	sqdmlslt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	4425f2b1 	sqdmulh	z17\.h, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	4468f000 	sqdmulh	z0\.h, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	4420f000 	sqdmulh	z0\.h, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44a5f2b1 	sqdmulh	z17\.s, z21\.s, z5\.s\[0\]
 *[0-9a-f]+:	44b8f000 	sqdmulh	z0\.s, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44a0f000 	sqdmulh	z0\.s, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	44e9f2b1 	sqdmulh	z17\.d, z21\.d, z9\.d\[0\]
 *[0-9a-f]+:	44f0f000 	sqdmulh	z0\.d, z0\.d, z0\.d\[1\]
 *[0-9a-f]+:	44e0f000 	sqdmulh	z0\.d, z0\.d, z0\.d\[0\]
 *[0-9a-f]+:	043b72b1 	sqdmulh	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	04207000 	sqdmulh	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	04607000 	sqdmulh	z0\.h, z0\.h, z0\.h
 *[0-9a-f]+:	04a07000 	sqdmulh	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	04e07000 	sqdmulh	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	44a5e2b1 	sqdmullb	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b0e800 	sqdmullb	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a0e000 	sqdmullb	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e9e2b1 	sqdmullb	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f0e800 	sqdmullb	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e0e000 	sqdmullb	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	455b62b1 	sqdmullb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45406000 	sqdmullb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45806000 	sqdmullb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c06000 	sqdmullb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	44a5e6b1 	sqdmullt	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b0ec00 	sqdmullt	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a0e400 	sqdmullt	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e9e6b1 	sqdmullt	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f0ec00 	sqdmullt	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e0e400 	sqdmullt	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	455b66b1 	sqdmullt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45406400 	sqdmullt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45806400 	sqdmullt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c06400 	sqdmullt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	4409b6b1 	sqneg	z17\.b, p5/m, z21\.b
 *[0-9a-f]+:	4409a000 	sqneg	z0\.b, p0/m, z0\.b
 *[0-9a-f]+:	4449a000 	sqneg	z0\.h, p0/m, z0\.h
 *[0-9a-f]+:	4489a000 	sqneg	z0\.s, p0/m, z0\.s
 *[0-9a-f]+:	44c9a000 	sqneg	z0\.d, p0/m, z0\.d
 *[0-9a-f]+:	44a572b1 	sqrdcmlah	z17\.h, z21\.h, z5\.h\[0\], #0
 *[0-9a-f]+:	44b87000 	sqrdcmlah	z0\.h, z0\.h, z0\.h\[3\], #0
 *[0-9a-f]+:	44a07400 	sqrdcmlah	z0\.h, z0\.h, z0\.h\[0\], #90
 *[0-9a-f]+:	44a07800 	sqrdcmlah	z0\.h, z0\.h, z0\.h\[0\], #180
 *[0-9a-f]+:	44a07c00 	sqrdcmlah	z0\.h, z0\.h, z0\.h\[0\], #270
 *[0-9a-f]+:	44e972b1 	sqrdcmlah	z17\.s, z21\.s, z9\.s\[0\], #0
 *[0-9a-f]+:	44f07000 	sqrdcmlah	z0\.s, z0\.s, z0\.s\[1\], #0
 *[0-9a-f]+:	44e07400 	sqrdcmlah	z0\.s, z0\.s, z0\.s\[0\], #90
 *[0-9a-f]+:	44e07800 	sqrdcmlah	z0\.s, z0\.s, z0\.s\[0\], #180
 *[0-9a-f]+:	44e07c00 	sqrdcmlah	z0\.s, z0\.s, z0\.s\[0\], #270
 *[0-9a-f]+:	441b32b1 	sqrdcmlah	z17\.b, z21\.b, z27\.b, #0
 *[0-9a-f]+:	44003000 	sqrdcmlah	z0\.b, z0\.b, z0\.b, #0
 *[0-9a-f]+:	44003400 	sqrdcmlah	z0\.b, z0\.b, z0\.b, #90
 *[0-9a-f]+:	44003800 	sqrdcmlah	z0\.b, z0\.b, z0\.b, #180
 *[0-9a-f]+:	44003c00 	sqrdcmlah	z0\.b, z0\.b, z0\.b, #270
 *[0-9a-f]+:	44403000 	sqrdcmlah	z0\.h, z0\.h, z0\.h, #0
 *[0-9a-f]+:	44803000 	sqrdcmlah	z0\.s, z0\.s, z0\.s, #0
 *[0-9a-f]+:	44c03000 	sqrdcmlah	z0\.d, z0\.d, z0\.d, #0
 *[0-9a-f]+:	442512b1 	sqrdmlah	z17\.h, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44681000 	sqrdmlah	z0\.h, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44201000 	sqrdmlah	z0\.h, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44a512b1 	sqrdmlah	z17\.s, z21\.s, z5\.s\[0\]
 *[0-9a-f]+:	44b81000 	sqrdmlah	z0\.s, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44a01000 	sqrdmlah	z0\.s, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	44e912b1 	sqrdmlah	z17\.d, z21\.d, z9\.d\[0\]
 *[0-9a-f]+:	44f01000 	sqrdmlah	z0\.d, z0\.d, z0\.d\[1\]
 *[0-9a-f]+:	44e01000 	sqrdmlah	z0\.d, z0\.d, z0\.d\[0\]
 *[0-9a-f]+:	441b72b1 	sqrdmlah	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	44007000 	sqrdmlah	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	44407000 	sqrdmlah	z0\.h, z0\.h, z0\.h
 *[0-9a-f]+:	44807000 	sqrdmlah	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	44c07000 	sqrdmlah	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	442516b1 	sqrdmlsh	z17\.h, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44681400 	sqrdmlsh	z0\.h, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44201400 	sqrdmlsh	z0\.h, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44a516b1 	sqrdmlsh	z17\.s, z21\.s, z5\.s\[0\]
 *[0-9a-f]+:	44b81400 	sqrdmlsh	z0\.s, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44a01400 	sqrdmlsh	z0\.s, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	44e916b1 	sqrdmlsh	z17\.d, z21\.d, z9\.d\[0\]
 *[0-9a-f]+:	44f01400 	sqrdmlsh	z0\.d, z0\.d, z0\.d\[1\]
 *[0-9a-f]+:	44e01400 	sqrdmlsh	z0\.d, z0\.d, z0\.d\[0\]
 *[0-9a-f]+:	441b76b1 	sqrdmlsh	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	44007400 	sqrdmlsh	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	44407400 	sqrdmlsh	z0\.h, z0\.h, z0\.h
 *[0-9a-f]+:	44807400 	sqrdmlsh	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	44c07400 	sqrdmlsh	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	4425f6b1 	sqrdmulh	z17\.h, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	4468f400 	sqrdmulh	z0\.h, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	4420f400 	sqrdmulh	z0\.h, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44a5f6b1 	sqrdmulh	z17\.s, z21\.s, z5\.s\[0\]
 *[0-9a-f]+:	44b8f400 	sqrdmulh	z0\.s, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44a0f400 	sqrdmulh	z0\.s, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	44e9f6b1 	sqrdmulh	z17\.d, z21\.d, z9\.d\[0\]
 *[0-9a-f]+:	44f0f400 	sqrdmulh	z0\.d, z0\.d, z0\.d\[1\]
 *[0-9a-f]+:	44e0f400 	sqrdmulh	z0\.d, z0\.d, z0\.d\[0\]
 *[0-9a-f]+:	043b76b1 	sqrdmulh	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	04207400 	sqrdmulh	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	04607400 	sqrdmulh	z0\.h, z0\.h, z0\.h
 *[0-9a-f]+:	04a07400 	sqrdmulh	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	04e07400 	sqrdmulh	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	440a96b1 	sqrshl	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	440a8000 	sqrshl	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	444a8000 	sqrshl	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	448a8000 	sqrshl	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44ca8000 	sqrshl	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	440e96b1 	sqrshlr	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	440e8000 	sqrshlr	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	444e8000 	sqrshlr	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	448e8000 	sqrshlr	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44ce8000 	sqrshlr	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	45292ab1 	sqrshrnb	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f2800 	sqrshrnb	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45282800 	sqrshrnb	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f2800 	sqrshrnb	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45312800 	sqrshrnb	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45302800 	sqrshrnb	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f2800 	sqrshrnb	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45612800 	sqrshrnb	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45602800 	sqrshrnb	z0\.s, z0\.d, #32
 *[0-9a-f]+:	45292eb1 	sqrshrnt	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f2c00 	sqrshrnt	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45282c00 	sqrshrnt	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f2c00 	sqrshrnt	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45312c00 	sqrshrnt	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45302c00 	sqrshrnt	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f2c00 	sqrshrnt	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45612c00 	sqrshrnt	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45602c00 	sqrshrnt	z0\.s, z0\.d, #32
 *[0-9a-f]+:	45290ab1 	sqrshrunb	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f0800 	sqrshrunb	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45280800 	sqrshrunb	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f0800 	sqrshrunb	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45310800 	sqrshrunb	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45300800 	sqrshrunb	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f0800 	sqrshrunb	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45610800 	sqrshrunb	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45600800 	sqrshrunb	z0\.s, z0\.d, #32
 *[0-9a-f]+:	45290eb1 	sqrshrunt	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f0c00 	sqrshrunt	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45280c00 	sqrshrunt	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f0c00 	sqrshrunt	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45310c00 	sqrshrunt	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45300c00 	sqrshrunt	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f0c00 	sqrshrunt	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45610c00 	sqrshrunt	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45600c00 	sqrshrunt	z0\.s, z0\.d, #32
 *[0-9a-f]+:	04069531 	sqshl	z17\.b, p5/m, z17\.b, #1
 *[0-9a-f]+:	04068100 	sqshl	z0\.b, p0/m, z0\.b, #0
 *[0-9a-f]+:	040681e0 	sqshl	z0\.b, p0/m, z0\.b, #7
 *[0-9a-f]+:	04068200 	sqshl	z0\.h, p0/m, z0\.h, #0
 *[0-9a-f]+:	040683e0 	sqshl	z0\.h, p0/m, z0\.h, #15
 *[0-9a-f]+:	04468000 	sqshl	z0\.s, p0/m, z0\.s, #0
 *[0-9a-f]+:	044683e0 	sqshl	z0\.s, p0/m, z0\.s, #31
 *[0-9a-f]+:	04868000 	sqshl	z0\.d, p0/m, z0\.d, #0
 *[0-9a-f]+:	04c683e0 	sqshl	z0\.d, p0/m, z0\.d, #63
 *[0-9a-f]+:	440896b1 	sqshl	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44088000 	sqshl	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44488000 	sqshl	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44888000 	sqshl	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44c88000 	sqshl	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	440c96b1 	sqshlr	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	440c8000 	sqshlr	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	444c8000 	sqshlr	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	448c8000 	sqshlr	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44cc8000 	sqshlr	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	040f9531 	sqshlu	z17\.b, p5/m, z17\.b, #1
 *[0-9a-f]+:	040f8100 	sqshlu	z0\.b, p0/m, z0\.b, #0
 *[0-9a-f]+:	040f81e0 	sqshlu	z0\.b, p0/m, z0\.b, #7
 *[0-9a-f]+:	040f8200 	sqshlu	z0\.h, p0/m, z0\.h, #0
 *[0-9a-f]+:	040f83e0 	sqshlu	z0\.h, p0/m, z0\.h, #15
 *[0-9a-f]+:	044f8000 	sqshlu	z0\.s, p0/m, z0\.s, #0
 *[0-9a-f]+:	044f83e0 	sqshlu	z0\.s, p0/m, z0\.s, #31
 *[0-9a-f]+:	048f8000 	sqshlu	z0\.d, p0/m, z0\.d, #0
 *[0-9a-f]+:	04cf83e0 	sqshlu	z0\.d, p0/m, z0\.d, #63
 *[0-9a-f]+:	452922b1 	sqshrnb	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f2000 	sqshrnb	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45282000 	sqshrnb	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f2000 	sqshrnb	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45312000 	sqshrnb	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45302000 	sqshrnb	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f2000 	sqshrnb	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45612000 	sqshrnb	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45602000 	sqshrnb	z0\.s, z0\.d, #32
 *[0-9a-f]+:	452926b1 	sqshrnt	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f2400 	sqshrnt	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45282400 	sqshrnt	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f2400 	sqshrnt	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45312400 	sqshrnt	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45302400 	sqshrnt	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f2400 	sqshrnt	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45612400 	sqshrnt	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45602400 	sqshrnt	z0\.s, z0\.d, #32
 *[0-9a-f]+:	452902b1 	sqshrunb	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f0000 	sqshrunb	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45280000 	sqshrunb	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f0000 	sqshrunb	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45310000 	sqshrunb	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45300000 	sqshrunb	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f0000 	sqshrunb	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45610000 	sqshrunb	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45600000 	sqshrunb	z0\.s, z0\.d, #32
 *[0-9a-f]+:	452906b1 	sqshrunt	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f0400 	sqshrunt	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45280400 	sqshrunt	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f0400 	sqshrunt	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45310400 	sqshrunt	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45300400 	sqshrunt	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f0400 	sqshrunt	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45610400 	sqshrunt	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45600400 	sqshrunt	z0\.s, z0\.d, #32
 *[0-9a-f]+:	441a96b1 	sqsub	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	441a8000 	sqsub	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	445a8000 	sqsub	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	449a8000 	sqsub	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44da8000 	sqsub	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	441e96b1 	sqsubr	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	441e8000 	sqsubr	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	445e8000 	sqsubr	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	449e8000 	sqsubr	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44de8000 	sqsubr	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	452842b1 	sqxtnb	z17\.b, z21\.h
 *[0-9a-f]+:	45284000 	sqxtnb	z0\.b, z0\.h
 *[0-9a-f]+:	45304000 	sqxtnb	z0\.h, z0\.s
 *[0-9a-f]+:	45604000 	sqxtnb	z0\.s, z0\.d
 *[0-9a-f]+:	452846b1 	sqxtnt	z17\.b, z21\.h
 *[0-9a-f]+:	45284400 	sqxtnt	z0\.b, z0\.h
 *[0-9a-f]+:	45304400 	sqxtnt	z0\.h, z0\.s
 *[0-9a-f]+:	45604400 	sqxtnt	z0\.s, z0\.d
 *[0-9a-f]+:	452852b1 	sqxtunb	z17\.b, z21\.h
 *[0-9a-f]+:	45285000 	sqxtunb	z0\.b, z0\.h
 *[0-9a-f]+:	45305000 	sqxtunb	z0\.h, z0\.s
 *[0-9a-f]+:	45605000 	sqxtunb	z0\.s, z0\.d
 *[0-9a-f]+:	452856b1 	sqxtunt	z17\.b, z21\.h
 *[0-9a-f]+:	45285400 	sqxtunt	z0\.b, z0\.h
 *[0-9a-f]+:	45305400 	sqxtunt	z0\.h, z0\.s
 *[0-9a-f]+:	45605400 	sqxtunt	z0\.s, z0\.d
 *[0-9a-f]+:	441496b1 	srhadd	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44148000 	srhadd	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44548000 	srhadd	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44948000 	srhadd	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44d48000 	srhadd	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	4509f2b1 	sri	z17\.b, z21\.b, #7
 *[0-9a-f]+:	4508f000 	sri	z0\.b, z0\.b, #8
 *[0-9a-f]+:	450ff000 	sri	z0\.b, z0\.b, #1
 *[0-9a-f]+:	4510f000 	sri	z0\.h, z0\.h, #16
 *[0-9a-f]+:	451ff000 	sri	z0\.h, z0\.h, #1
 *[0-9a-f]+:	4540f000 	sri	z0\.s, z0\.s, #32
 *[0-9a-f]+:	455ff000 	sri	z0\.s, z0\.s, #1
 *[0-9a-f]+:	4580f000 	sri	z0\.d, z0\.d, #64
 *[0-9a-f]+:	45dff000 	sri	z0\.d, z0\.d, #1
 *[0-9a-f]+:	440296b1 	srshl	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44028000 	srshl	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44428000 	srshl	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44828000 	srshl	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44c28000 	srshl	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	440696b1 	srshlr	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44068000 	srshlr	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44468000 	srshlr	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44868000 	srshlr	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44c68000 	srshlr	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	040c9531 	srshr	z17\.b, p5/m, z17\.b, #7
 *[0-9a-f]+:	040c8100 	srshr	z0\.b, p0/m, z0\.b, #8
 *[0-9a-f]+:	040c81e0 	srshr	z0\.b, p0/m, z0\.b, #1
 *[0-9a-f]+:	040c8200 	srshr	z0\.h, p0/m, z0\.h, #16
 *[0-9a-f]+:	040c83e0 	srshr	z0\.h, p0/m, z0\.h, #1
 *[0-9a-f]+:	044c8000 	srshr	z0\.s, p0/m, z0\.s, #32
 *[0-9a-f]+:	044c83e0 	srshr	z0\.s, p0/m, z0\.s, #1
 *[0-9a-f]+:	048c8000 	srshr	z0\.d, p0/m, z0\.d, #64
 *[0-9a-f]+:	04cc83e0 	srshr	z0\.d, p0/m, z0\.d, #1
 *[0-9a-f]+:	4509eab1 	srsra	z17\.b, z21\.b, #7
 *[0-9a-f]+:	4508e800 	srsra	z0\.b, z0\.b, #8
 *[0-9a-f]+:	450fe800 	srsra	z0\.b, z0\.b, #1
 *[0-9a-f]+:	4510e800 	srsra	z0\.h, z0\.h, #16
 *[0-9a-f]+:	451fe800 	srsra	z0\.h, z0\.h, #1
 *[0-9a-f]+:	4540e800 	srsra	z0\.s, z0\.s, #32
 *[0-9a-f]+:	455fe800 	srsra	z0\.s, z0\.s, #1
 *[0-9a-f]+:	4580e800 	srsra	z0\.d, z0\.d, #64
 *[0-9a-f]+:	45dfe800 	srsra	z0\.d, z0\.d, #1
 *[0-9a-f]+:	4509a2b1 	sshllb	z17\.h, z21\.b, #1
 *[0-9a-f]+:	4508a000 	sshllb	z0\.h, z0\.b, #0
 *[0-9a-f]+:	450fa000 	sshllb	z0\.h, z0\.b, #7
 *[0-9a-f]+:	4510a000 	sshllb	z0\.s, z0\.h, #0
 *[0-9a-f]+:	451fa000 	sshllb	z0\.s, z0\.h, #15
 *[0-9a-f]+:	4540a000 	sshllb	z0\.d, z0\.s, #0
 *[0-9a-f]+:	455fa000 	sshllb	z0\.d, z0\.s, #31
 *[0-9a-f]+:	4509a6b1 	sshllt	z17\.h, z21\.b, #1
 *[0-9a-f]+:	4508a400 	sshllt	z0\.h, z0\.b, #0
 *[0-9a-f]+:	450fa400 	sshllt	z0\.h, z0\.b, #7
 *[0-9a-f]+:	4510a400 	sshllt	z0\.s, z0\.h, #0
 *[0-9a-f]+:	451fa400 	sshllt	z0\.s, z0\.h, #15
 *[0-9a-f]+:	4540a400 	sshllt	z0\.d, z0\.s, #0
 *[0-9a-f]+:	455fa400 	sshllt	z0\.d, z0\.s, #31
 *[0-9a-f]+:	4509e2b1 	ssra	z17\.b, z21\.b, #7
 *[0-9a-f]+:	4508e000 	ssra	z0\.b, z0\.b, #8
 *[0-9a-f]+:	450fe000 	ssra	z0\.b, z0\.b, #1
 *[0-9a-f]+:	4510e000 	ssra	z0\.h, z0\.h, #16
 *[0-9a-f]+:	451fe000 	ssra	z0\.h, z0\.h, #1
 *[0-9a-f]+:	4540e000 	ssra	z0\.s, z0\.s, #32
 *[0-9a-f]+:	455fe000 	ssra	z0\.s, z0\.s, #1
 *[0-9a-f]+:	4580e000 	ssra	z0\.d, z0\.d, #64
 *[0-9a-f]+:	45dfe000 	ssra	z0\.d, z0\.d, #1
 *[0-9a-f]+:	455b12b1 	ssublb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45401000 	ssublb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45801000 	ssublb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c01000 	ssublb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455b8ab1 	ssublbt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45408800 	ssublbt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45808800 	ssublbt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c08800 	ssublbt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455b16b1 	ssublt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45401400 	ssublt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45801400 	ssublt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c01400 	ssublt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455b8eb1 	ssubltb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45408c00 	ssubltb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45808c00 	ssubltb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c08c00 	ssubltb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455b52b1 	ssubwb	z17\.h, z21\.h, z27\.b
 *[0-9a-f]+:	45405000 	ssubwb	z0\.h, z0\.h, z0\.b
 *[0-9a-f]+:	45805000 	ssubwb	z0\.s, z0\.s, z0\.h
 *[0-9a-f]+:	45c05000 	ssubwb	z0\.d, z0\.d, z0\.s
 *[0-9a-f]+:	455b56b1 	ssubwt	z17\.h, z21\.h, z27\.b
 *[0-9a-f]+:	45405400 	ssubwt	z0\.h, z0\.h, z0\.b
 *[0-9a-f]+:	45805400 	ssubwt	z0\.s, z0\.s, z0\.h
 *[0-9a-f]+:	45c05400 	ssubwt	z0\.d, z0\.d, z0\.s
 *[0-9a-f]+:	e45b36b1 	stnt1b	{z17\.s}, p5, \[z21\.s, x27\]
 *[0-9a-f]+:	e4402000 	stnt1b	{z0\.s}, p0, \[z0\.s, x0\]
 *[0-9a-f]+:	e45f2000 	stnt1b	{z0\.s}, p0, \[z0\.s, xzr\]
 *[0-9a-f]+:	e45f2000 	stnt1b	{z0\.s}, p0, \[z0\.s, xzr\]
 *[0-9a-f]+:	e41b36b1 	stnt1b	{z17\.d}, p5, \[z21\.d, x27\]
 *[0-9a-f]+:	e4002000 	stnt1b	{z0\.d}, p0, \[z0\.d, x0\]
 *[0-9a-f]+:	e41f2000 	stnt1b	{z0\.d}, p0, \[z0\.d, xzr\]
 *[0-9a-f]+:	e41f2000 	stnt1b	{z0\.d}, p0, \[z0\.d, xzr\]
 *[0-9a-f]+:	e59b36b1 	stnt1d	{z17\.d}, p5, \[z21\.d, x27\]
 *[0-9a-f]+:	e5802000 	stnt1d	{z0\.d}, p0, \[z0\.d, x0\]
 *[0-9a-f]+:	e59f2000 	stnt1d	{z0\.d}, p0, \[z0\.d, xzr\]
 *[0-9a-f]+:	e59f2000 	stnt1d	{z0\.d}, p0, \[z0\.d, xzr\]
 *[0-9a-f]+:	e4db36b1 	stnt1h	{z17\.s}, p5, \[z21\.s, x27\]
 *[0-9a-f]+:	e4c02000 	stnt1h	{z0\.s}, p0, \[z0\.s, x0\]
 *[0-9a-f]+:	e4df2000 	stnt1h	{z0\.s}, p0, \[z0\.s, xzr\]
 *[0-9a-f]+:	e4df2000 	stnt1h	{z0\.s}, p0, \[z0\.s, xzr\]
 *[0-9a-f]+:	e49b36b1 	stnt1h	{z17\.d}, p5, \[z21\.d, x27\]
 *[0-9a-f]+:	e4802000 	stnt1h	{z0\.d}, p0, \[z0\.d, x0\]
 *[0-9a-f]+:	e49f2000 	stnt1h	{z0\.d}, p0, \[z0\.d, xzr\]
 *[0-9a-f]+:	e49f2000 	stnt1h	{z0\.d}, p0, \[z0\.d, xzr\]
 *[0-9a-f]+:	e55b36b1 	stnt1w	{z17\.s}, p5, \[z21\.s, x27\]
 *[0-9a-f]+:	e5402000 	stnt1w	{z0\.s}, p0, \[z0\.s, x0\]
 *[0-9a-f]+:	e55f2000 	stnt1w	{z0\.s}, p0, \[z0\.s, xzr\]
 *[0-9a-f]+:	e55f2000 	stnt1w	{z0\.s}, p0, \[z0\.s, xzr\]
 *[0-9a-f]+:	e51b36b1 	stnt1w	{z17\.d}, p5, \[z21\.d, x27\]
 *[0-9a-f]+:	e5002000 	stnt1w	{z0\.d}, p0, \[z0\.d, x0\]
 *[0-9a-f]+:	e51f2000 	stnt1w	{z0\.d}, p0, \[z0\.d, xzr\]
 *[0-9a-f]+:	e51f2000 	stnt1w	{z0\.d}, p0, \[z0\.d, xzr\]
 *[0-9a-f]+:	457b72b1 	subhnb	z17\.b, z21\.h, z27\.h
 *[0-9a-f]+:	45607000 	subhnb	z0\.b, z0\.h, z0\.h
 *[0-9a-f]+:	45a07000 	subhnb	z0\.h, z0\.s, z0\.s
 *[0-9a-f]+:	45e07000 	subhnb	z0\.s, z0\.d, z0\.d
 *[0-9a-f]+:	457b76b1 	subhnt	z17\.b, z21\.h, z27\.h
 *[0-9a-f]+:	45607400 	subhnt	z0\.b, z0\.h, z0\.h
 *[0-9a-f]+:	45a07400 	subhnt	z0\.h, z0\.s, z0\.s
 *[0-9a-f]+:	45e07400 	subhnt	z0\.s, z0\.d, z0\.d
 *[0-9a-f]+:	441c96b1 	suqadd	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	441c8000 	suqadd	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	445c8000 	suqadd	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	449c8000 	suqadd	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44dc8000 	suqadd	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	053b2ab1 	tbl	z17\.b, {z21\.b-z22\.b}, z27\.b
 *[0-9a-f]+:	05202800 	tbl	z0\.b, {z0\.b-z1\.b}, z0\.b
 *[0-9a-f]+:	05602800 	tbl	z0\.h, {z0\.h-z1\.h}, z0\.h
 *[0-9a-f]+:	05a02800 	tbl	z0\.s, {z0\.s-z1\.s}, z0\.s
 *[0-9a-f]+:	05e02800 	tbl	z0\.d, {z0\.d-z1\.d}, z0\.d
 *[0-9a-f]+:	05202be0 	tbl	z0\.b, {z31\.b-z0\.b}, z0\.b
 *[0-9a-f]+:	053b2eb1 	tbx	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	05202c00 	tbx	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	05602c00 	tbx	z0\.h, z0\.h, z0\.h
 *[0-9a-f]+:	05a02c00 	tbx	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	05e02c00 	tbx	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	451bfeb1 	uaba	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	4500fc00 	uaba	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	4540fc00 	uaba	z0\.h, z0\.h, z0\.h
 *[0-9a-f]+:	4580fc00 	uaba	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	45c0fc00 	uaba	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	455bcab1 	uabalb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	4540c800 	uabalb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	4580c800 	uabalb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c0c800 	uabalb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455bceb1 	uabalt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	4540cc00 	uabalt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	4580cc00 	uabalt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c0cc00 	uabalt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455b3ab1 	uabdlb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45403800 	uabdlb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45803800 	uabdlb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c03800 	uabdlb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455b3eb1 	uabdlt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45403c00 	uabdlt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45803c00 	uabdlt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c03c00 	uabdlt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	4445b6b1 	uadalp	z17\.h, p5/m, z21\.b
 *[0-9a-f]+:	4445a000 	uadalp	z0\.h, p0/m, z0\.b
 *[0-9a-f]+:	4485a000 	uadalp	z0\.s, p0/m, z0\.h
 *[0-9a-f]+:	44c5a000 	uadalp	z0\.d, p0/m, z0\.s
 *[0-9a-f]+:	455b0ab1 	uaddlb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45400800 	uaddlb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45800800 	uaddlb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c00800 	uaddlb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455b0eb1 	uaddlt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45400c00 	uaddlt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45800c00 	uaddlt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c00c00 	uaddlt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455b4ab1 	uaddwb	z17\.h, z21\.h, z27\.b
 *[0-9a-f]+:	45404800 	uaddwb	z0\.h, z0\.h, z0\.b
 *[0-9a-f]+:	45804800 	uaddwb	z0\.s, z0\.s, z0\.h
 *[0-9a-f]+:	45c04800 	uaddwb	z0\.d, z0\.d, z0\.s
 *[0-9a-f]+:	455b4eb1 	uaddwt	z17\.h, z21\.h, z27\.b
 *[0-9a-f]+:	45404c00 	uaddwt	z0\.h, z0\.h, z0\.b
 *[0-9a-f]+:	45804c00 	uaddwt	z0\.s, z0\.s, z0\.h
 *[0-9a-f]+:	45c04c00 	uaddwt	z0\.d, z0\.d, z0\.s
 *[0-9a-f]+:	441196b1 	uhadd	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44118000 	uhadd	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44518000 	uhadd	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44918000 	uhadd	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44d18000 	uhadd	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	441396b1 	uhsub	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44138000 	uhsub	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44538000 	uhsub	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44938000 	uhsub	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44d38000 	uhsub	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	441796b1 	uhsubr	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44178000 	uhsubr	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44578000 	uhsubr	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44978000 	uhsubr	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44d78000 	uhsubr	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	4415b6b1 	umaxp	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	4415a000 	umaxp	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	4455a000 	umaxp	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	4495a000 	umaxp	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44d5a000 	umaxp	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	4417b6b1 	uminp	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	4417a000 	uminp	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	4457a000 	uminp	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	4497a000 	uminp	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44d7a000 	uminp	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	44a592b1 	umlalb	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b09800 	umlalb	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a09000 	umlalb	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e992b1 	umlalb	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f09800 	umlalb	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e09000 	umlalb	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	445b4ab1 	umlalb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	44404800 	umlalb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	44804800 	umlalb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	44c04800 	umlalb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	44a596b1 	umlalt	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b09c00 	umlalt	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a09400 	umlalt	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e996b1 	umlalt	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f09c00 	umlalt	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e09400 	umlalt	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	445b4eb1 	umlalt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	44404c00 	umlalt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	44804c00 	umlalt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	44c04c00 	umlalt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	44a5b2b1 	umlslb	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b0b800 	umlslb	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a0b000 	umlslb	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e9b2b1 	umlslb	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f0b800 	umlslb	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e0b000 	umlslb	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	445b5ab1 	umlslb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	44405800 	umlslb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	44805800 	umlslb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	44c05800 	umlslb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	44a5b6b1 	umlslt	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b0bc00 	umlslt	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a0b400 	umlslt	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e9b6b1 	umlslt	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f0bc00 	umlslt	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e0b400 	umlslt	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	445b5eb1 	umlslt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	44405c00 	umlslt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	44805c00 	umlslt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	44c05c00 	umlslt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	043b6eb1 	umulh	z17\.b, z21\.b, z27\.b
 *[0-9a-f]+:	04206c00 	umulh	z0\.b, z0\.b, z0\.b
 *[0-9a-f]+:	04606c00 	umulh	z0\.h, z0\.h, z0\.h
 *[0-9a-f]+:	04a06c00 	umulh	z0\.s, z0\.s, z0\.s
 *[0-9a-f]+:	04e06c00 	umulh	z0\.d, z0\.d, z0\.d
 *[0-9a-f]+:	44a5d2b1 	umullb	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b0d800 	umullb	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a0d000 	umullb	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e9d2b1 	umullb	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f0d800 	umullb	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e0d000 	umullb	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	455b7ab1 	umullb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45407800 	umullb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45807800 	umullb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c07800 	umullb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	44a5d6b1 	umullt	z17\.s, z21\.h, z5\.h\[0\]
 *[0-9a-f]+:	44b0dc00 	umullt	z0\.s, z0\.h, z0\.h\[5\]
 *[0-9a-f]+:	44a0d400 	umullt	z0\.s, z0\.h, z0\.h\[0\]
 *[0-9a-f]+:	44e9d6b1 	umullt	z17\.d, z21\.s, z9\.s\[0\]
 *[0-9a-f]+:	44f0dc00 	umullt	z0\.d, z0\.s, z0\.s\[3\]
 *[0-9a-f]+:	44e0d400 	umullt	z0\.d, z0\.s, z0\.s\[0\]
 *[0-9a-f]+:	455b7eb1 	umullt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45407c00 	umullt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45807c00 	umullt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c07c00 	umullt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	441996b1 	uqadd	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44198000 	uqadd	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44598000 	uqadd	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44998000 	uqadd	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44d98000 	uqadd	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	440b96b1 	uqrshl	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	440b8000 	uqrshl	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	444b8000 	uqrshl	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	448b8000 	uqrshl	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44cb8000 	uqrshl	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	440f96b1 	uqrshlr	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	440f8000 	uqrshlr	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	444f8000 	uqrshlr	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	448f8000 	uqrshlr	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44cf8000 	uqrshlr	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	45293ab1 	uqrshrnb	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f3800 	uqrshrnb	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45283800 	uqrshrnb	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f3800 	uqrshrnb	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45313800 	uqrshrnb	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45303800 	uqrshrnb	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f3800 	uqrshrnb	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45613800 	uqrshrnb	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45603800 	uqrshrnb	z0\.s, z0\.d, #32
 *[0-9a-f]+:	45293eb1 	uqrshrnt	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f3c00 	uqrshrnt	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45283c00 	uqrshrnt	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f3c00 	uqrshrnt	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45313c00 	uqrshrnt	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45303c00 	uqrshrnt	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f3c00 	uqrshrnt	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45613c00 	uqrshrnt	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45603c00 	uqrshrnt	z0\.s, z0\.d, #32
 *[0-9a-f]+:	04079531 	uqshl	z17\.b, p5/m, z17\.b, #1
 *[0-9a-f]+:	04078100 	uqshl	z0\.b, p0/m, z0\.b, #0
 *[0-9a-f]+:	040781e0 	uqshl	z0\.b, p0/m, z0\.b, #7
 *[0-9a-f]+:	04078200 	uqshl	z0\.h, p0/m, z0\.h, #0
 *[0-9a-f]+:	040783e0 	uqshl	z0\.h, p0/m, z0\.h, #15
 *[0-9a-f]+:	04478000 	uqshl	z0\.s, p0/m, z0\.s, #0
 *[0-9a-f]+:	044783e0 	uqshl	z0\.s, p0/m, z0\.s, #31
 *[0-9a-f]+:	04878000 	uqshl	z0\.d, p0/m, z0\.d, #0
 *[0-9a-f]+:	04c783e0 	uqshl	z0\.d, p0/m, z0\.d, #63
 *[0-9a-f]+:	440996b1 	uqshl	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44098000 	uqshl	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44498000 	uqshl	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44898000 	uqshl	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44c98000 	uqshl	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	440d96b1 	uqshlr	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	440d8000 	uqshlr	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	444d8000 	uqshlr	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	448d8000 	uqshlr	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44cd8000 	uqshlr	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	452932b1 	uqshrnb	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f3000 	uqshrnb	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45283000 	uqshrnb	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f3000 	uqshrnb	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45313000 	uqshrnb	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45303000 	uqshrnb	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f3000 	uqshrnb	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45613000 	uqshrnb	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45603000 	uqshrnb	z0\.s, z0\.d, #32
 *[0-9a-f]+:	452936b1 	uqshrnt	z17\.b, z21\.h, #7
 *[0-9a-f]+:	452f3400 	uqshrnt	z0\.b, z0\.h, #1
 *[0-9a-f]+:	45283400 	uqshrnt	z0\.b, z0\.h, #8
 *[0-9a-f]+:	453f3400 	uqshrnt	z0\.h, z0\.s, #1
 *[0-9a-f]+:	45313400 	uqshrnt	z0\.h, z0\.s, #15
 *[0-9a-f]+:	45303400 	uqshrnt	z0\.h, z0\.s, #16
 *[0-9a-f]+:	457f3400 	uqshrnt	z0\.s, z0\.d, #1
 *[0-9a-f]+:	45613400 	uqshrnt	z0\.s, z0\.d, #31
 *[0-9a-f]+:	45603400 	uqshrnt	z0\.s, z0\.d, #32
 *[0-9a-f]+:	441b96b1 	uqsub	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	441b8000 	uqsub	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	445b8000 	uqsub	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	449b8000 	uqsub	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44db8000 	uqsub	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	441f96b1 	uqsubr	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	441f8000 	uqsubr	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	445f8000 	uqsubr	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	449f8000 	uqsubr	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44df8000 	uqsubr	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	45284ab1 	uqxtnb	z17\.b, z21\.h
 *[0-9a-f]+:	45284800 	uqxtnb	z0\.b, z0\.h
 *[0-9a-f]+:	45304800 	uqxtnb	z0\.h, z0\.s
 *[0-9a-f]+:	45604800 	uqxtnb	z0\.s, z0\.d
 *[0-9a-f]+:	45284eb1 	uqxtnt	z17\.b, z21\.h
 *[0-9a-f]+:	45284c00 	uqxtnt	z0\.b, z0\.h
 *[0-9a-f]+:	45304c00 	uqxtnt	z0\.h, z0\.s
 *[0-9a-f]+:	45604c00 	uqxtnt	z0\.s, z0\.d
 *[0-9a-f]+:	4480b6b1 	urecpe	z17\.s, p5/m, z21\.s
 *[0-9a-f]+:	4480a000 	urecpe	z0\.s, p0/m, z0\.s
 *[0-9a-f]+:	441596b1 	urhadd	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44158000 	urhadd	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44558000 	urhadd	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44958000 	urhadd	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44d58000 	urhadd	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	440396b1 	urshl	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44038000 	urshl	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44438000 	urshl	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44838000 	urshl	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44c38000 	urshl	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	440796b1 	urshlr	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	44078000 	urshlr	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	44478000 	urshlr	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	44878000 	urshlr	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44c78000 	urshlr	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	040d9531 	urshr	z17\.b, p5/m, z17\.b, #7
 *[0-9a-f]+:	040d8100 	urshr	z0\.b, p0/m, z0\.b, #8
 *[0-9a-f]+:	040d81e0 	urshr	z0\.b, p0/m, z0\.b, #1
 *[0-9a-f]+:	040d8200 	urshr	z0\.h, p0/m, z0\.h, #16
 *[0-9a-f]+:	040d83e0 	urshr	z0\.h, p0/m, z0\.h, #1
 *[0-9a-f]+:	044d8000 	urshr	z0\.s, p0/m, z0\.s, #32
 *[0-9a-f]+:	044d83e0 	urshr	z0\.s, p0/m, z0\.s, #1
 *[0-9a-f]+:	048d8000 	urshr	z0\.d, p0/m, z0\.d, #64
 *[0-9a-f]+:	04cd83e0 	urshr	z0\.d, p0/m, z0\.d, #1
 *[0-9a-f]+:	4481b6b1 	ursqrte	z17\.s, p5/m, z21\.s
 *[0-9a-f]+:	4481a000 	ursqrte	z0\.s, p0/m, z0\.s
 *[0-9a-f]+:	4509eeb1 	ursra	z17\.b, z21\.b, #7
 *[0-9a-f]+:	4508ec00 	ursra	z0\.b, z0\.b, #8
 *[0-9a-f]+:	450fec00 	ursra	z0\.b, z0\.b, #1
 *[0-9a-f]+:	4510ec00 	ursra	z0\.h, z0\.h, #16
 *[0-9a-f]+:	451fec00 	ursra	z0\.h, z0\.h, #1
 *[0-9a-f]+:	4540ec00 	ursra	z0\.s, z0\.s, #32
 *[0-9a-f]+:	455fec00 	ursra	z0\.s, z0\.s, #1
 *[0-9a-f]+:	4580ec00 	ursra	z0\.d, z0\.d, #64
 *[0-9a-f]+:	45dfec00 	ursra	z0\.d, z0\.d, #1
 *[0-9a-f]+:	4509aab1 	ushllb	z17\.h, z21\.b, #1
 *[0-9a-f]+:	4508a800 	ushllb	z0\.h, z0\.b, #0
 *[0-9a-f]+:	450fa800 	ushllb	z0\.h, z0\.b, #7
 *[0-9a-f]+:	4510a800 	ushllb	z0\.s, z0\.h, #0
 *[0-9a-f]+:	451fa800 	ushllb	z0\.s, z0\.h, #15
 *[0-9a-f]+:	4540a800 	ushllb	z0\.d, z0\.s, #0
 *[0-9a-f]+:	455fa800 	ushllb	z0\.d, z0\.s, #31
 *[0-9a-f]+:	4509aeb1 	ushllt	z17\.h, z21\.b, #1
 *[0-9a-f]+:	4508ac00 	ushllt	z0\.h, z0\.b, #0
 *[0-9a-f]+:	450fac00 	ushllt	z0\.h, z0\.b, #7
 *[0-9a-f]+:	4510ac00 	ushllt	z0\.s, z0\.h, #0
 *[0-9a-f]+:	451fac00 	ushllt	z0\.s, z0\.h, #15
 *[0-9a-f]+:	4540ac00 	ushllt	z0\.d, z0\.s, #0
 *[0-9a-f]+:	455fac00 	ushllt	z0\.d, z0\.s, #31
 *[0-9a-f]+:	441d96b1 	usqadd	z17\.b, p5/m, z17\.b, z21\.b
 *[0-9a-f]+:	441d8000 	usqadd	z0\.b, p0/m, z0\.b, z0\.b
 *[0-9a-f]+:	445d8000 	usqadd	z0\.h, p0/m, z0\.h, z0\.h
 *[0-9a-f]+:	449d8000 	usqadd	z0\.s, p0/m, z0\.s, z0\.s
 *[0-9a-f]+:	44dd8000 	usqadd	z0\.d, p0/m, z0\.d, z0\.d
 *[0-9a-f]+:	4509e6b1 	usra	z17\.b, z21\.b, #7
 *[0-9a-f]+:	4508e400 	usra	z0\.b, z0\.b, #8
 *[0-9a-f]+:	450fe400 	usra	z0\.b, z0\.b, #1
 *[0-9a-f]+:	4510e400 	usra	z0\.h, z0\.h, #16
 *[0-9a-f]+:	451fe400 	usra	z0\.h, z0\.h, #1
 *[0-9a-f]+:	4540e400 	usra	z0\.s, z0\.s, #32
 *[0-9a-f]+:	455fe400 	usra	z0\.s, z0\.s, #1
 *[0-9a-f]+:	4580e400 	usra	z0\.d, z0\.d, #64
 *[0-9a-f]+:	45dfe400 	usra	z0\.d, z0\.d, #1
 *[0-9a-f]+:	455b1ab1 	usublb	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45401800 	usublb	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45801800 	usublb	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c01800 	usublb	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455b1eb1 	usublt	z17\.h, z21\.b, z27\.b
 *[0-9a-f]+:	45401c00 	usublt	z0\.h, z0\.b, z0\.b
 *[0-9a-f]+:	45801c00 	usublt	z0\.s, z0\.h, z0\.h
 *[0-9a-f]+:	45c01c00 	usublt	z0\.d, z0\.s, z0\.s
 *[0-9a-f]+:	455b5ab1 	usubwb	z17\.h, z21\.h, z27\.b
 *[0-9a-f]+:	45405800 	usubwb	z0\.h, z0\.h, z0\.b
 *[0-9a-f]+:	45805800 	usubwb	z0\.s, z0\.s, z0\.h
 *[0-9a-f]+:	45c05800 	usubwb	z0\.d, z0\.d, z0\.s
 *[0-9a-f]+:	455b5eb1 	usubwt	z17\.h, z21\.h, z27\.b
 *[0-9a-f]+:	45405c00 	usubwt	z0\.h, z0\.h, z0\.b
 *[0-9a-f]+:	45805c00 	usubwt	z0\.s, z0\.s, z0\.h
 *[0-9a-f]+:	45c05c00 	usubwt	z0\.d, z0\.d, z0\.s
 *[0-9a-f]+:	253b12a9 	whilege	p9\.b, x21, x27
 *[0-9a-f]+:	25201000 	whilege	p0\.b, x0, x0
 *[0-9a-f]+:	252013e0 	whilege	p0\.b, xzr, x0
 *[0-9a-f]+:	253f1000 	whilege	p0\.b, x0, xzr
 *[0-9a-f]+:	25601000 	whilege	p0\.h, x0, x0
 *[0-9a-f]+:	25a01000 	whilege	p0\.s, x0, x0
 *[0-9a-f]+:	25e01000 	whilege	p0\.d, x0, x0
 *[0-9a-f]+:	253b02a9 	whilege	p9\.b, w21, w27
 *[0-9a-f]+:	25200000 	whilege	p0\.b, w0, w0
 *[0-9a-f]+:	252003e0 	whilege	p0\.b, wzr, w0
 *[0-9a-f]+:	253f0000 	whilege	p0\.b, w0, wzr
 *[0-9a-f]+:	25600000 	whilege	p0\.h, w0, w0
 *[0-9a-f]+:	25a00000 	whilege	p0\.s, w0, w0
 *[0-9a-f]+:	25e00000 	whilege	p0\.d, w0, w0
 *[0-9a-f]+:	253b12b9 	whilegt	p9\.b, x21, x27
 *[0-9a-f]+:	25201010 	whilegt	p0\.b, x0, x0
 *[0-9a-f]+:	252013f0 	whilegt	p0\.b, xzr, x0
 *[0-9a-f]+:	253f1010 	whilegt	p0\.b, x0, xzr
 *[0-9a-f]+:	25601010 	whilegt	p0\.h, x0, x0
 *[0-9a-f]+:	25a01010 	whilegt	p0\.s, x0, x0
 *[0-9a-f]+:	25e01010 	whilegt	p0\.d, x0, x0
 *[0-9a-f]+:	253b02b9 	whilegt	p9\.b, w21, w27
 *[0-9a-f]+:	25200010 	whilegt	p0\.b, w0, w0
 *[0-9a-f]+:	252003f0 	whilegt	p0\.b, wzr, w0
 *[0-9a-f]+:	253f0010 	whilegt	p0\.b, w0, wzr
 *[0-9a-f]+:	25600010 	whilegt	p0\.h, w0, w0
 *[0-9a-f]+:	25a00010 	whilegt	p0\.s, w0, w0
 *[0-9a-f]+:	25e00010 	whilegt	p0\.d, w0, w0
 *[0-9a-f]+:	253b1ab9 	whilehi	p9\.b, x21, x27
 *[0-9a-f]+:	25201810 	whilehi	p0\.b, x0, x0
 *[0-9a-f]+:	25201bf0 	whilehi	p0\.b, xzr, x0
 *[0-9a-f]+:	253f1810 	whilehi	p0\.b, x0, xzr
 *[0-9a-f]+:	25601810 	whilehi	p0\.h, x0, x0
 *[0-9a-f]+:	25a01810 	whilehi	p0\.s, x0, x0
 *[0-9a-f]+:	25e01810 	whilehi	p0\.d, x0, x0
 *[0-9a-f]+:	253b0ab9 	whilehi	p9\.b, w21, w27
 *[0-9a-f]+:	25200810 	whilehi	p0\.b, w0, w0
 *[0-9a-f]+:	25200bf0 	whilehi	p0\.b, wzr, w0
 *[0-9a-f]+:	253f0810 	whilehi	p0\.b, w0, wzr
 *[0-9a-f]+:	25600810 	whilehi	p0\.h, w0, w0
 *[0-9a-f]+:	25a00810 	whilehi	p0\.s, w0, w0
 *[0-9a-f]+:	25e00810 	whilehi	p0\.d, w0, w0
 *[0-9a-f]+:	253b1aa9 	whilehs	p9\.b, x21, x27
 *[0-9a-f]+:	25201800 	whilehs	p0\.b, x0, x0
 *[0-9a-f]+:	25201be0 	whilehs	p0\.b, xzr, x0
 *[0-9a-f]+:	253f1800 	whilehs	p0\.b, x0, xzr
 *[0-9a-f]+:	25601800 	whilehs	p0\.h, x0, x0
 *[0-9a-f]+:	25a01800 	whilehs	p0\.s, x0, x0
 *[0-9a-f]+:	25e01800 	whilehs	p0\.d, x0, x0
 *[0-9a-f]+:	253b0aa9 	whilehs	p9\.b, w21, w27
 *[0-9a-f]+:	25200800 	whilehs	p0\.b, w0, w0
 *[0-9a-f]+:	25200be0 	whilehs	p0\.b, wzr, w0
 *[0-9a-f]+:	253f0800 	whilehs	p0\.b, w0, wzr
 *[0-9a-f]+:	25600800 	whilehs	p0\.h, w0, w0
 *[0-9a-f]+:	25a00800 	whilehs	p0\.s, w0, w0
 *[0-9a-f]+:	25e00800 	whilehs	p0\.d, w0, w0
 *[0-9a-f]+:	253b32b9 	whilerw	p9\.b, x21, x27
 *[0-9a-f]+:	25203010 	whilerw	p0\.b, x0, x0
 *[0-9a-f]+:	25603010 	whilerw	p0\.h, x0, x0
 *[0-9a-f]+:	25a03010 	whilerw	p0\.s, x0, x0
 *[0-9a-f]+:	25e03010 	whilerw	p0\.d, x0, x0
 *[0-9a-f]+:	253b32a9 	whilewr	p9\.b, x21, x27
 *[0-9a-f]+:	25203000 	whilewr	p0\.b, x0, x0
 *[0-9a-f]+:	25603000 	whilewr	p0\.h, x0, x0
 *[0-9a-f]+:	25a03000 	whilewr	p0\.s, x0, x0
 *[0-9a-f]+:	25e03000 	whilewr	p0\.d, x0, x0
 *[0-9a-f]+:	042936b1 	xar	z17\.b, z17\.b, z21\.b, #7
 *[0-9a-f]+:	04283400 	xar	z0\.b, z0\.b, z0\.b, #8
 *[0-9a-f]+:	042f3400 	xar	z0\.b, z0\.b, z0\.b, #1
 *[0-9a-f]+:	04303400 	xar	z0\.h, z0\.h, z0\.h, #16
 *[0-9a-f]+:	043f3400 	xar	z0\.h, z0\.h, z0\.h, #1
 *[0-9a-f]+:	04603400 	xar	z0\.s, z0\.s, z0\.s, #32
 *[0-9a-f]+:	047f3400 	xar	z0\.s, z0\.s, z0\.s, #1
 *[0-9a-f]+:	04a03400 	xar	z0\.d, z0\.d, z0\.d, #64
 *[0-9a-f]+:	04ff3400 	xar	z0\.d, z0\.d, z0\.d, #1
