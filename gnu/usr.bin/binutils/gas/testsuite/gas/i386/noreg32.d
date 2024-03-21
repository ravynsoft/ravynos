#as: -moperand-check=none
#objdump: -dw
#name: 32-bit insns not sizeable through register operands

.*: +file format .*

Disassembly of section .text:

0+ <noreg>:
 *[a-f0-9]+:	83 10 01             	adcl   \$0x1,\(%eax\)
 *[a-f0-9]+:	81 10 89 00 00 00    	adcl   \$0x89,\(%eax\)
 *[a-f0-9]+:	81 10 34 12 00 00    	adcl   \$0x1234,\(%eax\)
 *[a-f0-9]+:	81 10 78 56 34 12    	adcl   \$0x12345678,\(%eax\)
 *[a-f0-9]+:	83 00 01             	addl   \$0x1,\(%eax\)
 *[a-f0-9]+:	81 00 89 00 00 00    	addl   \$0x89,\(%eax\)
 *[a-f0-9]+:	81 00 34 12 00 00    	addl   \$0x1234,\(%eax\)
 *[a-f0-9]+:	81 00 78 56 34 12    	addl   \$0x12345678,\(%eax\)
 *[a-f0-9]+:	83 20 01             	andl   \$0x1,\(%eax\)
 *[a-f0-9]+:	81 20 89 00 00 00    	andl   \$0x89,\(%eax\)
 *[a-f0-9]+:	81 20 34 12 00 00    	andl   \$0x1234,\(%eax\)
 *[a-f0-9]+:	81 20 78 56 34 12    	andl   \$0x12345678,\(%eax\)
 *[a-f0-9]+:	0f ba 20 01          	btl    \$0x1,\(%eax\)
 *[a-f0-9]+:	0f ba 38 01          	btcl   \$0x1,\(%eax\)
 *[a-f0-9]+:	0f ba 30 01          	btrl   \$0x1,\(%eax\)
 *[a-f0-9]+:	0f ba 28 01          	btsl   \$0x1,\(%eax\)
 *[a-f0-9]+:	ff 10                	call   \*\(%eax\)
 *[a-f0-9]+:	83 38 01             	cmpl   \$0x1,\(%eax\)
 *[a-f0-9]+:	81 38 89 00 00 00    	cmpl   \$0x89,\(%eax\)
 *[a-f0-9]+:	81 38 34 12 00 00    	cmpl   \$0x1234,\(%eax\)
 *[a-f0-9]+:	81 38 78 56 34 12    	cmpl   \$0x12345678,\(%eax\)
 *[a-f0-9]+:	a7                   	cmpsl  %es:\(%edi\),%ds:\(%esi\)
 *[a-f0-9]+:	a7                   	cmpsl  %es:\(%edi\),%ds:\(%esi\)
 *[a-f0-9]+:	f2 0f 38 f1 00       	crc32l \(%eax\),%eax
 *[a-f0-9]+:	f2 0f 2a 00          	cvtsi2sd \(%eax\),%xmm0
 *[a-f0-9]+:	f3 0f 2a 00          	cvtsi2ss \(%eax\),%xmm0
 *[a-f0-9]+:	ff 08                	decl   \(%eax\)
 *[a-f0-9]+:	f7 30                	divl   \(%eax\)
 *[a-f0-9]+:	d8 00                	fadds  \(%eax\)
 *[a-f0-9]+:	d8 10                	fcoms  \(%eax\)
 *[a-f0-9]+:	d8 18                	fcomps \(%eax\)
 *[a-f0-9]+:	d8 30                	fdivs  \(%eax\)
 *[a-f0-9]+:	d8 38                	fdivrs \(%eax\)
 *[a-f0-9]+:	de 00                	fiadds \(%eax\)
 *[a-f0-9]+:	de 10                	ficoms \(%eax\)
 *[a-f0-9]+:	de 18                	ficomps \(%eax\)
 *[a-f0-9]+:	de 30                	fidivs \(%eax\)
 *[a-f0-9]+:	de 38                	fidivrs \(%eax\)
 *[a-f0-9]+:	df 00                	filds  \(%eax\)
 *[a-f0-9]+:	de 08                	fimuls \(%eax\)
 *[a-f0-9]+:	df 10                	fists  \(%eax\)
 *[a-f0-9]+:	df 18                	fistps \(%eax\)
 *[a-f0-9]+:	df 08                	fisttps \(%eax\)
 *[a-f0-9]+:	de 20                	fisubs \(%eax\)
 *[a-f0-9]+:	de 28                	fisubrs \(%eax\)
 *[a-f0-9]+:	d9 00                	flds   \(%eax\)
 *[a-f0-9]+:	d8 08                	fmuls  \(%eax\)
 *[a-f0-9]+:	d9 10                	fsts   \(%eax\)
 *[a-f0-9]+:	d9 18                	fstps  \(%eax\)
 *[a-f0-9]+:	d8 20                	fsubs  \(%eax\)
 *[a-f0-9]+:	d8 28                	fsubrs \(%eax\)
 *[a-f0-9]+:	f7 38                	idivl  \(%eax\)
 *[a-f0-9]+:	f7 28                	imull  \(%eax\)
 *[a-f0-9]+:	e5 00                	in     \$0x0,%eax
 *[a-f0-9]+:	ed                   	in     \(%dx\),%eax
 *[a-f0-9]+:	ff 00                	incl   \(%eax\)
 *[a-f0-9]+:	6d                   	insl   \(%dx\),%es:\(%edi\)
 *[a-f0-9]+:	6d                   	insl   \(%dx\),%es:\(%edi\)
 *[a-f0-9]+:	ff 20                	jmp    \*\(%eax\)
 *[a-f0-9]+:	0f 01 10             	lgdtl  \(%eax\)
 *[a-f0-9]+:	0f 01 18             	lidtl  \(%eax\)
 *[a-f0-9]+:	0f 00 10             	lldt   \(%eax\)
 *[a-f0-9]+:	0f 01 30             	lmsw   \(%eax\)
 *[a-f0-9]+:	ad                   	lods   %ds:\(%esi\),%eax
 *[a-f0-9]+:	ad                   	lods   %ds:\(%esi\),%eax
 *[a-f0-9]+:	0f 00 18             	ltr    \(%eax\)
 *[a-f0-9]+:	c7 00 12 00 00 00    	movl   \$0x12,\(%eax\)
 *[a-f0-9]+:	c7 00 34 12 00 00    	movl   \$0x1234,\(%eax\)
 *[a-f0-9]+:	c7 00 78 56 34 12    	movl   \$0x12345678,\(%eax\)
 *[a-f0-9]+:	8c 00                	mov    %es,\(%eax\)
 *[a-f0-9]+:	8e 00                	mov    \(%eax\),%es
 *[a-f0-9]+:	a5                   	movsl  %ds:\(%esi\),%es:\(%edi\)
 *[a-f0-9]+:	a5                   	movsl  %ds:\(%esi\),%es:\(%edi\)
 *[a-f0-9]+:	66 0f be 00          	movsbw \(%eax\),%ax
 *[a-f0-9]+:	0f be 00             	movsbl \(%eax\),%eax
 *[a-f0-9]+:	66 0f b6 00          	movzbw \(%eax\),%ax
 *[a-f0-9]+:	0f b6 00             	movzbl \(%eax\),%eax
 *[a-f0-9]+:	f7 20                	mull   \(%eax\)
 *[a-f0-9]+:	f7 18                	negl   \(%eax\)
 *[a-f0-9]+:	0f 1f 00             	nopl   \(%eax\)
 *[a-f0-9]+:	f7 10                	notl   \(%eax\)
 *[a-f0-9]+:	83 08 01             	orl    \$0x1,\(%eax\)
 *[a-f0-9]+:	81 08 89 00 00 00    	orl    \$0x89,\(%eax\)
 *[a-f0-9]+:	81 08 34 12 00 00    	orl    \$0x1234,\(%eax\)
 *[a-f0-9]+:	81 08 78 56 34 12    	orl    \$0x12345678,\(%eax\)
 *[a-f0-9]+:	e7 00                	out    %eax,\$0x0
 *[a-f0-9]+:	ef                   	out    %eax,\(%dx\)
 *[a-f0-9]+:	6f                   	outsl  %ds:\(%esi\),\(%dx\)
 *[a-f0-9]+:	6f                   	outsl  %ds:\(%esi\),\(%dx\)
 *[a-f0-9]+:	8f 00                	pop    \(%eax\)
 *[a-f0-9]+:	07                   	pop    %es
 *[a-f0-9]+:	f3 0f ae 20          	ptwrite \(%eax\)
 *[a-f0-9]+:	ff 30                	push   \(%eax\)
 *[a-f0-9]+:	06                   	push   %es
 *[a-f0-9]+:	d1 10                	rcll   \(%eax\)
 *[a-f0-9]+:	c1 10 02             	rcll   \$0x2,\(%eax\)
 *[a-f0-9]+:	d3 10                	rcll   %cl,\(%eax\)
 *[a-f0-9]+:	d1 10                	rcll   \(%eax\)
 *[a-f0-9]+:	d1 18                	rcrl   \(%eax\)
 *[a-f0-9]+:	c1 18 02             	rcrl   \$0x2,\(%eax\)
 *[a-f0-9]+:	d3 18                	rcrl   %cl,\(%eax\)
 *[a-f0-9]+:	d1 18                	rcrl   \(%eax\)
 *[a-f0-9]+:	d1 00                	roll   \(%eax\)
 *[a-f0-9]+:	c1 00 02             	roll   \$0x2,\(%eax\)
 *[a-f0-9]+:	d3 00                	roll   %cl,\(%eax\)
 *[a-f0-9]+:	d1 00                	roll   \(%eax\)
 *[a-f0-9]+:	d1 08                	rorl   \(%eax\)
 *[a-f0-9]+:	c1 08 02             	rorl   \$0x2,\(%eax\)
 *[a-f0-9]+:	d3 08                	rorl   %cl,\(%eax\)
 *[a-f0-9]+:	d1 08                	rorl   \(%eax\)
 *[a-f0-9]+:	83 18 01             	sbbl   \$0x1,\(%eax\)
 *[a-f0-9]+:	81 18 89 00 00 00    	sbbl   \$0x89,\(%eax\)
 *[a-f0-9]+:	81 18 34 12 00 00    	sbbl   \$0x1234,\(%eax\)
 *[a-f0-9]+:	81 18 78 56 34 12    	sbbl   \$0x12345678,\(%eax\)
 *[a-f0-9]+:	af                   	scas   %es:\(%edi\),%eax
 *[a-f0-9]+:	af                   	scas   %es:\(%edi\),%eax
 *[a-f0-9]+:	d1 20                	shll   \(%eax\)
 *[a-f0-9]+:	c1 20 02             	shll   \$0x2,\(%eax\)
 *[a-f0-9]+:	d3 20                	shll   %cl,\(%eax\)
 *[a-f0-9]+:	d1 20                	shll   \(%eax\)
 *[a-f0-9]+:	d1 38                	sarl   \(%eax\)
 *[a-f0-9]+:	c1 38 02             	sarl   \$0x2,\(%eax\)
 *[a-f0-9]+:	d3 38                	sarl   %cl,\(%eax\)
 *[a-f0-9]+:	d1 38                	sarl   \(%eax\)
 *[a-f0-9]+:	d1 20                	shll   \(%eax\)
 *[a-f0-9]+:	c1 20 02             	shll   \$0x2,\(%eax\)
 *[a-f0-9]+:	d3 20                	shll   %cl,\(%eax\)
 *[a-f0-9]+:	d1 20                	shll   \(%eax\)
 *[a-f0-9]+:	d1 28                	shrl   \(%eax\)
 *[a-f0-9]+:	c1 28 02             	shrl   \$0x2,\(%eax\)
 *[a-f0-9]+:	d3 28                	shrl   %cl,\(%eax\)
 *[a-f0-9]+:	d1 28                	shrl   \(%eax\)
 *[a-f0-9]+:	ab                   	stos   %eax,%es:\(%edi\)
 *[a-f0-9]+:	ab                   	stos   %eax,%es:\(%edi\)
 *[a-f0-9]+:	83 28 01             	subl   \$0x1,\(%eax\)
 *[a-f0-9]+:	81 28 89 00 00 00    	subl   \$0x89,\(%eax\)
 *[a-f0-9]+:	81 28 34 12 00 00    	subl   \$0x1234,\(%eax\)
 *[a-f0-9]+:	81 28 78 56 34 12    	subl   \$0x12345678,\(%eax\)
 *[a-f0-9]+:	f7 00 89 00 00 00    	testl  \$0x89,\(%eax\)
 *[a-f0-9]+:	f7 00 34 12 00 00    	testl  \$0x1234,\(%eax\)
 *[a-f0-9]+:	f7 00 78 56 34 12    	testl  \$0x12345678,\(%eax\)
 *[a-f0-9]+:	c5 fb 2a 00          	vcvtsi2sd \(%eax\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7f 08 2a 00    	\{evex\} vcvtsi2sd \(%eax\),%xmm0,%xmm0
 *[a-f0-9]+:	c5 fa 2a 00          	vcvtsi2ss \(%eax\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7e 08 2a 00    	\{evex\} vcvtsi2ss \(%eax\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7f 08 7b 00    	vcvtusi2sd \(%eax\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7e 08 7b 00    	vcvtusi2ss \(%eax\),%xmm0,%xmm0
 *[a-f0-9]+:	83 30 01             	xorl   \$0x1,\(%eax\)
 *[a-f0-9]+:	81 30 89 00 00 00    	xorl   \$0x89,\(%eax\)
 *[a-f0-9]+:	81 30 34 12 00 00    	xorl   \$0x1234,\(%eax\)
 *[a-f0-9]+:	81 30 78 56 34 12    	xorl   \$0x12345678,\(%eax\)
#pass
