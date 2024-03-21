#as: --defsym DATA32=1
#objdump: -dwMi8086
#name: 16-bit insns not sizeable through register operands w/ data32
#source: noreg16.s

.*: +file format .*

Disassembly of section .text:

0+ <noreg>:
 *[a-f0-9]+:	66 83 17 01          	adcl   \$0x1,\(%bx\)
 *[a-f0-9]+:	66 81 17 89 00 00 00 	adcl   \$0x89,\(%bx\)
 *[a-f0-9]+:	66 81 17 34 12 00 00 	adcl   \$0x1234,\(%bx\)
 *[a-f0-9]+:	66 83 07 01          	addl   \$0x1,\(%bx\)
 *[a-f0-9]+:	66 81 07 89 00 00 00 	addl   \$0x89,\(%bx\)
 *[a-f0-9]+:	66 81 07 34 12 00 00 	addl   \$0x1234,\(%bx\)
 *[a-f0-9]+:	66 83 27 01          	andl   \$0x1,\(%bx\)
 *[a-f0-9]+:	66 81 27 89 00 00 00 	andl   \$0x89,\(%bx\)
 *[a-f0-9]+:	66 81 27 34 12 00 00 	andl   \$0x1234,\(%bx\)
 *[a-f0-9]+:	66 0f ba 27 01       	btl    \$0x1,\(%bx\)
 *[a-f0-9]+:	66 0f ba 3f 01       	btcl   \$0x1,\(%bx\)
 *[a-f0-9]+:	66 0f ba 37 01       	btrl   \$0x1,\(%bx\)
 *[a-f0-9]+:	66 0f ba 2f 01       	btsl   \$0x1,\(%bx\)
 *[a-f0-9]+:	66 ff 17             	calll  \*\(%bx\)
 *[a-f0-9]+:	66 83 3f 01          	cmpl   \$0x1,\(%bx\)
 *[a-f0-9]+:	66 81 3f 89 00 00 00 	cmpl   \$0x89,\(%bx\)
 *[a-f0-9]+:	66 81 3f 34 12 00 00 	cmpl   \$0x1234,\(%bx\)
 *[a-f0-9]+:	66 a7                	cmpsl  %es:\(%di\),%ds:\(%si\)
 *[a-f0-9]+:	66 a7                	cmpsl  %es:\(%di\),%ds:\(%si\)
 *[a-f0-9]+:	66 f2 0f 38 f1 07    	crc32l \(%bx\),%eax
 *[a-f0-9]+:	f2 0f 2a 07          	cvtsi2sd \(%bx\),%xmm0
 *[a-f0-9]+:	f3 0f 2a 07          	cvtsi2ss \(%bx\),%xmm0
 *[a-f0-9]+:	66 ff 0f             	decl   \(%bx\)
 *[a-f0-9]+:	66 f7 37             	divl   \(%bx\)
 *[a-f0-9]+:	66 d8 07             	data32 fadds \(%bx\)
 *[a-f0-9]+:	66 d8 17             	data32 fcoms \(%bx\)
 *[a-f0-9]+:	66 d8 1f             	data32 fcomps \(%bx\)
 *[a-f0-9]+:	66 d8 37             	data32 fdivs \(%bx\)
 *[a-f0-9]+:	66 d8 3f             	data32 fdivrs \(%bx\)
 *[a-f0-9]+:	66 de 07             	data32 fiadds \(%bx\)
 *[a-f0-9]+:	66 de 17             	data32 ficoms \(%bx\)
 *[a-f0-9]+:	66 de 1f             	data32 ficomps \(%bx\)
 *[a-f0-9]+:	66 de 37             	data32 fidivs \(%bx\)
 *[a-f0-9]+:	66 de 3f             	data32 fidivrs \(%bx\)
 *[a-f0-9]+:	66 df 07             	data32 filds \(%bx\)
 *[a-f0-9]+:	66 de 0f             	data32 fimuls \(%bx\)
 *[a-f0-9]+:	66 df 17             	data32 fists \(%bx\)
 *[a-f0-9]+:	66 df 1f             	data32 fistps \(%bx\)
 *[a-f0-9]+:	66 df 0f             	data32 fisttps \(%bx\)
 *[a-f0-9]+:	66 de 27             	data32 fisubs \(%bx\)
 *[a-f0-9]+:	66 de 2f             	data32 fisubrs \(%bx\)
 *[a-f0-9]+:	66 d9 07             	data32 flds \(%bx\)
 *[a-f0-9]+:	66 d8 0f             	data32 fmuls \(%bx\)
 *[a-f0-9]+:	66 d9 17             	data32 fsts \(%bx\)
 *[a-f0-9]+:	66 d9 1f             	data32 fstps \(%bx\)
 *[a-f0-9]+:	66 d8 27             	data32 fsubs \(%bx\)
 *[a-f0-9]+:	66 d8 2f             	data32 fsubrs \(%bx\)
 *[a-f0-9]+:	66 f7 3f             	idivl  \(%bx\)
 *[a-f0-9]+:	66 f7 2f             	imull  \(%bx\)
 *[a-f0-9]+:	66 e5 00             	in     \$0x0,%eax
 *[a-f0-9]+:	66 ed                	in     \(%dx\),%eax
 *[a-f0-9]+:	66 ff 07             	incl   \(%bx\)
 *[a-f0-9]+:	66 6d                	insl   \(%dx\),%es:\(%di\)
 *[a-f0-9]+:	66 6d                	insl   \(%dx\),%es:\(%di\)
 *[a-f0-9]+:	66 ff 27             	jmpl   \*\(%bx\)
 *[a-f0-9]+:	66 0f 01 17          	lgdtl  \(%bx\)
 *[a-f0-9]+:	66 0f 01 1f          	lidtl  \(%bx\)
 *[a-f0-9]+:	66 0f 00 17          	data32 lldt \(%bx\)
 *[a-f0-9]+:	66 0f 01 37          	data32 lmsw \(%bx\)
 *[a-f0-9]+:	66 ad                	lods   %ds:\(%si\),%eax
 *[a-f0-9]+:	66 ad                	lods   %ds:\(%si\),%eax
 *[a-f0-9]+:	66 0f 00 1f          	data32 ltr \(%bx\)
 *[a-f0-9]+:	66 c7 07 12 00 00 00 	movl   \$0x12,\(%bx\)
 *[a-f0-9]+:	66 c7 07 34 12 00 00 	movl   \$0x1234,\(%bx\)
 *[a-f0-9]+:	66 8c 07             	data32 mov %es,\(%bx\)
 *[a-f0-9]+:	66 8e 07             	data32 mov \(%bx\),%es
 *[a-f0-9]+:	66 a5                	movsl  %ds:\(%si\),%es:\(%di\)
 *[a-f0-9]+:	66 a5                	movsl  %ds:\(%si\),%es:\(%di\)
 *[a-f0-9]+:	66 0f be 07          	movsbl \(%bx\),%eax
 *[a-f0-9]+:	66 0f be 07          	movsbl \(%bx\),%eax
 *[a-f0-9]+:	66 0f b6 07          	movzbl \(%bx\),%eax
 *[a-f0-9]+:	66 0f b6 07          	movzbl \(%bx\),%eax
 *[a-f0-9]+:	66 f7 27             	mull   \(%bx\)
 *[a-f0-9]+:	66 f7 1f             	negl   \(%bx\)
 *[a-f0-9]+:	66 0f 1f 07          	nopl   \(%bx\)
 *[a-f0-9]+:	66 f7 17             	notl   \(%bx\)
 *[a-f0-9]+:	66 83 0f 01          	orl    \$0x1,\(%bx\)
 *[a-f0-9]+:	66 81 0f 89 00 00 00 	orl    \$0x89,\(%bx\)
 *[a-f0-9]+:	66 81 0f 34 12 00 00 	orl    \$0x1234,\(%bx\)
 *[a-f0-9]+:	66 e7 00             	out    %eax,\$0x0
 *[a-f0-9]+:	66 ef                	out    %eax,\(%dx\)
 *[a-f0-9]+:	66 6f                	outsl  %ds:\(%si\),\(%dx\)
 *[a-f0-9]+:	66 6f                	outsl  %ds:\(%si\),\(%dx\)
 *[a-f0-9]+:	66 8f 07             	popl   \(%bx\)
 *[a-f0-9]+:	66 07                	popl   %es
 *[a-f0-9]+:	f3 0f ae 27          	ptwrite \(%bx\)
 *[a-f0-9]+:	66 ff 37             	pushl  \(%bx\)
 *[a-f0-9]+:	66 06                	pushl  %es
 *[a-f0-9]+:	66 d1 17             	rcll   \(%bx\)
 *[a-f0-9]+:	66 c1 17 02          	rcll   \$0x2,\(%bx\)
 *[a-f0-9]+:	66 d3 17             	rcll   %cl,\(%bx\)
 *[a-f0-9]+:	66 d1 17             	rcll   \(%bx\)
 *[a-f0-9]+:	66 d1 1f             	rcrl   \(%bx\)
 *[a-f0-9]+:	66 c1 1f 02          	rcrl   \$0x2,\(%bx\)
 *[a-f0-9]+:	66 d3 1f             	rcrl   %cl,\(%bx\)
 *[a-f0-9]+:	66 d1 1f             	rcrl   \(%bx\)
 *[a-f0-9]+:	66 d1 07             	roll   \(%bx\)
 *[a-f0-9]+:	66 c1 07 02          	roll   \$0x2,\(%bx\)
 *[a-f0-9]+:	66 d3 07             	roll   %cl,\(%bx\)
 *[a-f0-9]+:	66 d1 07             	roll   \(%bx\)
 *[a-f0-9]+:	66 d1 0f             	rorl   \(%bx\)
 *[a-f0-9]+:	66 c1 0f 02          	rorl   \$0x2,\(%bx\)
 *[a-f0-9]+:	66 d3 0f             	rorl   %cl,\(%bx\)
 *[a-f0-9]+:	66 d1 0f             	rorl   \(%bx\)
 *[a-f0-9]+:	66 83 1f 01          	sbbl   \$0x1,\(%bx\)
 *[a-f0-9]+:	66 81 1f 89 00 00 00 	sbbl   \$0x89,\(%bx\)
 *[a-f0-9]+:	66 81 1f 34 12 00 00 	sbbl   \$0x1234,\(%bx\)
 *[a-f0-9]+:	66 af                	scas   %es:\(%di\),%eax
 *[a-f0-9]+:	66 af                	scas   %es:\(%di\),%eax
 *[a-f0-9]+:	66 d1 27             	shll   \(%bx\)
 *[a-f0-9]+:	66 c1 27 02          	shll   \$0x2,\(%bx\)
 *[a-f0-9]+:	66 d3 27             	shll   %cl,\(%bx\)
 *[a-f0-9]+:	66 d1 27             	shll   \(%bx\)
 *[a-f0-9]+:	66 d1 3f             	sarl   \(%bx\)
 *[a-f0-9]+:	66 c1 3f 02          	sarl   \$0x2,\(%bx\)
 *[a-f0-9]+:	66 d3 3f             	sarl   %cl,\(%bx\)
 *[a-f0-9]+:	66 d1 3f             	sarl   \(%bx\)
 *[a-f0-9]+:	66 d1 27             	shll   \(%bx\)
 *[a-f0-9]+:	66 c1 27 02          	shll   \$0x2,\(%bx\)
 *[a-f0-9]+:	66 d3 27             	shll   %cl,\(%bx\)
 *[a-f0-9]+:	66 d1 27             	shll   \(%bx\)
 *[a-f0-9]+:	66 d1 2f             	shrl   \(%bx\)
 *[a-f0-9]+:	66 c1 2f 02          	shrl   \$0x2,\(%bx\)
 *[a-f0-9]+:	66 d3 2f             	shrl   %cl,\(%bx\)
 *[a-f0-9]+:	66 d1 2f             	shrl   \(%bx\)
 *[a-f0-9]+:	66 ab                	stos   %eax,%es:\(%di\)
 *[a-f0-9]+:	66 ab                	stos   %eax,%es:\(%di\)
 *[a-f0-9]+:	66 83 2f 01          	subl   \$0x1,\(%bx\)
 *[a-f0-9]+:	66 81 2f 89 00 00 00 	subl   \$0x89,\(%bx\)
 *[a-f0-9]+:	66 81 2f 34 12 00 00 	subl   \$0x1234,\(%bx\)
 *[a-f0-9]+:	66 f7 07 89 00 00 00 	testl  \$0x89,\(%bx\)
 *[a-f0-9]+:	66 f7 07 34 12 00 00 	testl  \$0x1234,\(%bx\)
 *[a-f0-9]+:	c5 fb 2a 07          	vcvtsi2sd \(%bx\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7f 08 2a 07    	\{evex\} vcvtsi2sd \(%bx\),%xmm0,%xmm0
 *[a-f0-9]+:	c5 fa 2a 07          	vcvtsi2ss \(%bx\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7e 08 2a 07    	\{evex\} vcvtsi2ss \(%bx\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7f 08 7b 07    	vcvtusi2sd \(%bx\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7e 08 7b 07    	vcvtusi2ss \(%bx\),%xmm0,%xmm0
 *[a-f0-9]+:	66 83 37 01          	xorl   \$0x1,\(%bx\)
 *[a-f0-9]+:	66 81 37 89 00 00 00 	xorl   \$0x89,\(%bx\)
 *[a-f0-9]+:	66 81 37 34 12 00 00 	xorl   \$0x1234,\(%bx\)
#pass
