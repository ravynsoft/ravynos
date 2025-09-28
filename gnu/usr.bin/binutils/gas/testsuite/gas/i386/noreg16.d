#as: -moperand-check=none
#objdump: -dwMi8086
#name: 16-bit insns not sizeable through register operands

.*: +file format .*

Disassembly of section .text:

0+ <noreg>:
 *[a-f0-9]+:	83 17 01             	adcw   \$0x1,\(%bx\)
 *[a-f0-9]+:	81 17 89 00          	adcw   \$0x89,\(%bx\)
 *[a-f0-9]+:	81 17 34 12          	adcw   \$0x1234,\(%bx\)
 *[a-f0-9]+:	83 07 01             	addw   \$0x1,\(%bx\)
 *[a-f0-9]+:	81 07 89 00          	addw   \$0x89,\(%bx\)
 *[a-f0-9]+:	81 07 34 12          	addw   \$0x1234,\(%bx\)
 *[a-f0-9]+:	83 27 01             	andw   \$0x1,\(%bx\)
 *[a-f0-9]+:	81 27 89 00          	andw   \$0x89,\(%bx\)
 *[a-f0-9]+:	81 27 34 12          	andw   \$0x1234,\(%bx\)
 *[a-f0-9]+:	0f ba 27 01          	btw    \$0x1,\(%bx\)
 *[a-f0-9]+:	0f ba 3f 01          	btcw   \$0x1,\(%bx\)
 *[a-f0-9]+:	0f ba 37 01          	btrw   \$0x1,\(%bx\)
 *[a-f0-9]+:	0f ba 2f 01          	btsw   \$0x1,\(%bx\)
 *[a-f0-9]+:	ff 17                	call   \*\(%bx\)
 *[a-f0-9]+:	83 3f 01             	cmpw   \$0x1,\(%bx\)
 *[a-f0-9]+:	81 3f 89 00          	cmpw   \$0x89,\(%bx\)
 *[a-f0-9]+:	81 3f 34 12          	cmpw   \$0x1234,\(%bx\)
 *[a-f0-9]+:	a7                   	cmpsw  %es:\(%di\),%ds:\(%si\)
 *[a-f0-9]+:	a7                   	cmpsw  %es:\(%di\),%ds:\(%si\)
 *[a-f0-9]+:	f2 0f 38 f1 07       	crc32w \(%bx\),%eax
 *[a-f0-9]+:	f2 0f 2a 07          	cvtsi2sd \(%bx\),%xmm0
 *[a-f0-9]+:	f3 0f 2a 07          	cvtsi2ss \(%bx\),%xmm0
 *[a-f0-9]+:	ff 0f                	decw   \(%bx\)
 *[a-f0-9]+:	f7 37                	divw   \(%bx\)
 *[a-f0-9]+:	d8 07                	fadds  \(%bx\)
 *[a-f0-9]+:	d8 17                	fcoms  \(%bx\)
 *[a-f0-9]+:	d8 1f                	fcomps \(%bx\)
 *[a-f0-9]+:	d8 37                	fdivs  \(%bx\)
 *[a-f0-9]+:	d8 3f                	fdivrs \(%bx\)
 *[a-f0-9]+:	de 07                	fiadds \(%bx\)
 *[a-f0-9]+:	de 17                	ficoms \(%bx\)
 *[a-f0-9]+:	de 1f                	ficomps \(%bx\)
 *[a-f0-9]+:	de 37                	fidivs \(%bx\)
 *[a-f0-9]+:	de 3f                	fidivrs \(%bx\)
 *[a-f0-9]+:	df 07                	filds  \(%bx\)
 *[a-f0-9]+:	de 0f                	fimuls \(%bx\)
 *[a-f0-9]+:	df 17                	fists  \(%bx\)
 *[a-f0-9]+:	df 1f                	fistps \(%bx\)
 *[a-f0-9]+:	df 0f                	fisttps \(%bx\)
 *[a-f0-9]+:	de 27                	fisubs \(%bx\)
 *[a-f0-9]+:	de 2f                	fisubrs \(%bx\)
 *[a-f0-9]+:	d9 07                	flds   \(%bx\)
 *[a-f0-9]+:	d8 0f                	fmuls  \(%bx\)
 *[a-f0-9]+:	d9 17                	fsts   \(%bx\)
 *[a-f0-9]+:	d9 1f                	fstps  \(%bx\)
 *[a-f0-9]+:	d8 27                	fsubs  \(%bx\)
 *[a-f0-9]+:	d8 2f                	fsubrs \(%bx\)
 *[a-f0-9]+:	f7 3f                	idivw  \(%bx\)
 *[a-f0-9]+:	f7 2f                	imulw  \(%bx\)
 *[a-f0-9]+:	e5 00                	in     \$0x0,%ax
 *[a-f0-9]+:	ed                   	in     \(%dx\),%ax
 *[a-f0-9]+:	ff 07                	incw   \(%bx\)
 *[a-f0-9]+:	6d                   	insw   \(%dx\),%es:\(%di\)
 *[a-f0-9]+:	6d                   	insw   \(%dx\),%es:\(%di\)
 *[a-f0-9]+:	ff 27                	jmp    \*\(%bx\)
 *[a-f0-9]+:	0f 01 17             	lgdtw  \(%bx\)
 *[a-f0-9]+:	0f 01 1f             	lidtw  \(%bx\)
 *[a-f0-9]+:	0f 00 17             	lldt   \(%bx\)
 *[a-f0-9]+:	0f 01 37             	lmsw   \(%bx\)
 *[a-f0-9]+:	ad                   	lods   %ds:\(%si\),%ax
 *[a-f0-9]+:	ad                   	lods   %ds:\(%si\),%ax
 *[a-f0-9]+:	0f 00 1f             	ltr    \(%bx\)
 *[a-f0-9]+:	c7 07 12 00          	movw   \$0x12,\(%bx\)
 *[a-f0-9]+:	c7 07 34 12          	movw   \$0x1234,\(%bx\)
 *[a-f0-9]+:	8c 07                	mov    %es,\(%bx\)
 *[a-f0-9]+:	8e 07                	mov    \(%bx\),%es
 *[a-f0-9]+:	a5                   	movsw  %ds:\(%si\),%es:\(%di\)
 *[a-f0-9]+:	a5                   	movsw  %ds:\(%si\),%es:\(%di\)
 *[a-f0-9]+:	0f be 07             	movsbw \(%bx\),%ax
 *[a-f0-9]+:	66 0f be 07          	movsbl \(%bx\),%eax
 *[a-f0-9]+:	0f b6 07             	movzbw \(%bx\),%ax
 *[a-f0-9]+:	66 0f b6 07          	movzbl \(%bx\),%eax
 *[a-f0-9]+:	f7 27                	mulw   \(%bx\)
 *[a-f0-9]+:	f7 1f                	negw   \(%bx\)
 *[a-f0-9]+:	0f 1f 07             	nopw   \(%bx\)
 *[a-f0-9]+:	f7 17                	notw   \(%bx\)
 *[a-f0-9]+:	83 0f 01             	orw    \$0x1,\(%bx\)
 *[a-f0-9]+:	81 0f 89 00          	orw    \$0x89,\(%bx\)
 *[a-f0-9]+:	81 0f 34 12          	orw    \$0x1234,\(%bx\)
 *[a-f0-9]+:	e7 00                	out    %ax,\$0x0
 *[a-f0-9]+:	ef                   	out    %ax,\(%dx\)
 *[a-f0-9]+:	6f                   	outsw  %ds:\(%si\),\(%dx\)
 *[a-f0-9]+:	6f                   	outsw  %ds:\(%si\),\(%dx\)
 *[a-f0-9]+:	8f 07                	pop    \(%bx\)
 *[a-f0-9]+:	07                   	pop    %es
 *[a-f0-9]+:	f3 0f ae 27          	ptwrite \(%bx\)
 *[a-f0-9]+:	ff 37                	push   \(%bx\)
 *[a-f0-9]+:	06                   	push   %es
 *[a-f0-9]+:	d1 17                	rclw   \(%bx\)
 *[a-f0-9]+:	c1 17 02             	rclw   \$0x2,\(%bx\)
 *[a-f0-9]+:	d3 17                	rclw   %cl,\(%bx\)
 *[a-f0-9]+:	d1 17                	rclw   \(%bx\)
 *[a-f0-9]+:	d1 1f                	rcrw   \(%bx\)
 *[a-f0-9]+:	c1 1f 02             	rcrw   \$0x2,\(%bx\)
 *[a-f0-9]+:	d3 1f                	rcrw   %cl,\(%bx\)
 *[a-f0-9]+:	d1 1f                	rcrw   \(%bx\)
 *[a-f0-9]+:	d1 07                	rolw   \(%bx\)
 *[a-f0-9]+:	c1 07 02             	rolw   \$0x2,\(%bx\)
 *[a-f0-9]+:	d3 07                	rolw   %cl,\(%bx\)
 *[a-f0-9]+:	d1 07                	rolw   \(%bx\)
 *[a-f0-9]+:	d1 0f                	rorw   \(%bx\)
 *[a-f0-9]+:	c1 0f 02             	rorw   \$0x2,\(%bx\)
 *[a-f0-9]+:	d3 0f                	rorw   %cl,\(%bx\)
 *[a-f0-9]+:	d1 0f                	rorw   \(%bx\)
 *[a-f0-9]+:	83 1f 01             	sbbw   \$0x1,\(%bx\)
 *[a-f0-9]+:	81 1f 89 00          	sbbw   \$0x89,\(%bx\)
 *[a-f0-9]+:	81 1f 34 12          	sbbw   \$0x1234,\(%bx\)
 *[a-f0-9]+:	af                   	scas   %es:\(%di\),%ax
 *[a-f0-9]+:	af                   	scas   %es:\(%di\),%ax
 *[a-f0-9]+:	d1 27                	shlw   \(%bx\)
 *[a-f0-9]+:	c1 27 02             	shlw   \$0x2,\(%bx\)
 *[a-f0-9]+:	d3 27                	shlw   %cl,\(%bx\)
 *[a-f0-9]+:	d1 27                	shlw   \(%bx\)
 *[a-f0-9]+:	d1 3f                	sarw   \(%bx\)
 *[a-f0-9]+:	c1 3f 02             	sarw   \$0x2,\(%bx\)
 *[a-f0-9]+:	d3 3f                	sarw   %cl,\(%bx\)
 *[a-f0-9]+:	d1 3f                	sarw   \(%bx\)
 *[a-f0-9]+:	d1 27                	shlw   \(%bx\)
 *[a-f0-9]+:	c1 27 02             	shlw   \$0x2,\(%bx\)
 *[a-f0-9]+:	d3 27                	shlw   %cl,\(%bx\)
 *[a-f0-9]+:	d1 27                	shlw   \(%bx\)
 *[a-f0-9]+:	d1 2f                	shrw   \(%bx\)
 *[a-f0-9]+:	c1 2f 02             	shrw   \$0x2,\(%bx\)
 *[a-f0-9]+:	d3 2f                	shrw   %cl,\(%bx\)
 *[a-f0-9]+:	d1 2f                	shrw   \(%bx\)
 *[a-f0-9]+:	ab                   	stos   %ax,%es:\(%di\)
 *[a-f0-9]+:	ab                   	stos   %ax,%es:\(%di\)
 *[a-f0-9]+:	83 2f 01             	subw   \$0x1,\(%bx\)
 *[a-f0-9]+:	81 2f 89 00          	subw   \$0x89,\(%bx\)
 *[a-f0-9]+:	81 2f 34 12          	subw   \$0x1234,\(%bx\)
 *[a-f0-9]+:	f7 07 89 00          	testw  \$0x89,\(%bx\)
 *[a-f0-9]+:	f7 07 34 12          	testw  \$0x1234,\(%bx\)
 *[a-f0-9]+:	c5 fb 2a 07          	vcvtsi2sd \(%bx\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7f 08 2a 07    	\{evex\} vcvtsi2sd \(%bx\),%xmm0,%xmm0
 *[a-f0-9]+:	c5 fa 2a 07          	vcvtsi2ss \(%bx\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7e 08 2a 07    	\{evex\} vcvtsi2ss \(%bx\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7f 08 7b 07    	vcvtusi2sd \(%bx\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7e 08 7b 07    	vcvtusi2ss \(%bx\),%xmm0,%xmm0
 *[a-f0-9]+:	83 37 01             	xorw   \$0x1,\(%bx\)
 *[a-f0-9]+:	81 37 89 00          	xorw   \$0x89,\(%bx\)
 *[a-f0-9]+:	81 37 34 12          	xorw   \$0x1234,\(%bx\)
#pass
