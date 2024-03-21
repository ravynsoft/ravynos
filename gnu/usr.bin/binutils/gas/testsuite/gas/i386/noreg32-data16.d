#as: --defsym DATA16=1
#objdump: -dw
#name: 32-bit insns not sizeable through register operands w/ data16
#source: noreg32.s
#warning_output: noreg32-data16.e

.*: +file format .*

Disassembly of section .text:

0+ <noreg>:
 *[a-f0-9]+:	66 83 10 01          	adcw   \$0x1,\(%eax\)
 *[a-f0-9]+:	66 81 10 89 00       	adcw   \$0x89,\(%eax\)
 *[a-f0-9]+:	66 81 10 34 12       	adcw   \$0x1234,\(%eax\)
 *[a-f0-9]+:	66 81 10 78 56       	adcw   \$0x5678,\(%eax\)
 *[a-f0-9]+:	66 83 00 01          	addw   \$0x1,\(%eax\)
 *[a-f0-9]+:	66 81 00 89 00       	addw   \$0x89,\(%eax\)
 *[a-f0-9]+:	66 81 00 34 12       	addw   \$0x1234,\(%eax\)
 *[a-f0-9]+:	66 81 00 78 56       	addw   \$0x5678,\(%eax\)
 *[a-f0-9]+:	66 83 20 01          	andw   \$0x1,\(%eax\)
 *[a-f0-9]+:	66 81 20 89 00       	andw   \$0x89,\(%eax\)
 *[a-f0-9]+:	66 81 20 34 12       	andw   \$0x1234,\(%eax\)
 *[a-f0-9]+:	66 81 20 78 56       	andw   \$0x5678,\(%eax\)
 *[a-f0-9]+:	66 0f ba 20 01       	btw    \$0x1,\(%eax\)
 *[a-f0-9]+:	66 0f ba 38 01       	btcw   \$0x1,\(%eax\)
 *[a-f0-9]+:	66 0f ba 30 01       	btrw   \$0x1,\(%eax\)
 *[a-f0-9]+:	66 0f ba 28 01       	btsw   \$0x1,\(%eax\)
 *[a-f0-9]+:	66 ff 10             	callw  \*\(%eax\)
 *[a-f0-9]+:	66 83 38 01          	cmpw   \$0x1,\(%eax\)
 *[a-f0-9]+:	66 81 38 89 00       	cmpw   \$0x89,\(%eax\)
 *[a-f0-9]+:	66 81 38 34 12       	cmpw   \$0x1234,\(%eax\)
 *[a-f0-9]+:	66 81 38 78 56       	cmpw   \$0x5678,\(%eax\)
 *[a-f0-9]+:	66 a7                	cmpsw  %es:\(%edi\),%ds:\(%esi\)
 *[a-f0-9]+:	66 a7                	cmpsw  %es:\(%edi\),%ds:\(%esi\)
 *[a-f0-9]+:	66 f2 0f 38 f1 00    	crc32w \(%eax\),%eax
 *[a-f0-9]+:	f2 0f 2a 00          	cvtsi2sd \(%eax\),%xmm0
 *[a-f0-9]+:	f3 0f 2a 00          	cvtsi2ss \(%eax\),%xmm0
 *[a-f0-9]+:	66 ff 08             	decw   \(%eax\)
 *[a-f0-9]+:	66 f7 30             	divw   \(%eax\)
 *[a-f0-9]+:	66 d8 00             	data16 fadds \(%eax\)
 *[a-f0-9]+:	66 d8 10             	data16 fcoms \(%eax\)
 *[a-f0-9]+:	66 d8 18             	data16 fcomps \(%eax\)
 *[a-f0-9]+:	66 d8 30             	data16 fdivs \(%eax\)
 *[a-f0-9]+:	66 d8 38             	data16 fdivrs \(%eax\)
 *[a-f0-9]+:	66 de 00             	data16 fiadds \(%eax\)
 *[a-f0-9]+:	66 de 10             	data16 ficoms \(%eax\)
 *[a-f0-9]+:	66 de 18             	data16 ficomps \(%eax\)
 *[a-f0-9]+:	66 de 30             	data16 fidivs \(%eax\)
 *[a-f0-9]+:	66 de 38             	data16 fidivrs \(%eax\)
 *[a-f0-9]+:	66 df 00             	data16 filds \(%eax\)
 *[a-f0-9]+:	66 de 08             	data16 fimuls \(%eax\)
 *[a-f0-9]+:	66 df 10             	data16 fists \(%eax\)
 *[a-f0-9]+:	66 df 18             	data16 fistps \(%eax\)
 *[a-f0-9]+:	66 df 08             	data16 fisttps \(%eax\)
 *[a-f0-9]+:	66 de 20             	data16 fisubs \(%eax\)
 *[a-f0-9]+:	66 de 28             	data16 fisubrs \(%eax\)
 *[a-f0-9]+:	66 d9 00             	data16 flds \(%eax\)
 *[a-f0-9]+:	66 d8 08             	data16 fmuls \(%eax\)
 *[a-f0-9]+:	66 d9 10             	data16 fsts \(%eax\)
 *[a-f0-9]+:	66 d9 18             	data16 fstps \(%eax\)
 *[a-f0-9]+:	66 d8 20             	data16 fsubs \(%eax\)
 *[a-f0-9]+:	66 d8 28             	data16 fsubrs \(%eax\)
 *[a-f0-9]+:	66 f7 38             	idivw  \(%eax\)
 *[a-f0-9]+:	66 f7 28             	imulw  \(%eax\)
 *[a-f0-9]+:	66 e5 00             	in     \$0x0,%ax
 *[a-f0-9]+:	66 ed                	in     \(%dx\),%ax
 *[a-f0-9]+:	66 ff 00             	incw   \(%eax\)
 *[a-f0-9]+:	66 6d                	insw   \(%dx\),%es:\(%edi\)
 *[a-f0-9]+:	66 6d                	insw   \(%dx\),%es:\(%edi\)
 *[a-f0-9]+:	66 ff 20             	jmpw   \*\(%eax\)
 *[a-f0-9]+:	66 0f 01 10          	lgdtw  \(%eax\)
 *[a-f0-9]+:	66 0f 01 18          	lidtw  \(%eax\)
 *[a-f0-9]+:	66 0f 00 10          	data16 lldt \(%eax\)
 *[a-f0-9]+:	66 0f 01 30          	data16 lmsw \(%eax\)
 *[a-f0-9]+:	66 ad                	lods   %ds:\(%esi\),%ax
 *[a-f0-9]+:	66 ad                	lods   %ds:\(%esi\),%ax
 *[a-f0-9]+:	66 0f 00 18          	data16 ltr \(%eax\)
 *[a-f0-9]+:	66 c7 00 12 00       	movw   \$0x12,\(%eax\)
 *[a-f0-9]+:	66 c7 00 34 12       	movw   \$0x1234,\(%eax\)
 *[a-f0-9]+:	66 c7 00 78 56       	movw   \$0x5678,\(%eax\)
 *[a-f0-9]+:	66 8c 00             	data16 mov %es,\(%eax\)
 *[a-f0-9]+:	66 8e 00             	data16 mov \(%eax\),%es
 *[a-f0-9]+:	66 a5                	movsw  %ds:\(%esi\),%es:\(%edi\)
 *[a-f0-9]+:	66 a5                	movsw  %ds:\(%esi\),%es:\(%edi\)
 *[a-f0-9]+:	66 0f be 00          	movsbw \(%eax\),%ax
 *[a-f0-9]+:	66 0f be 00          	movsbw \(%eax\),%ax
 *[a-f0-9]+:	66 0f b6 00          	movzbw \(%eax\),%ax
 *[a-f0-9]+:	66 0f b6 00          	movzbw \(%eax\),%ax
 *[a-f0-9]+:	66 f7 20             	mulw   \(%eax\)
 *[a-f0-9]+:	66 f7 18             	negw   \(%eax\)
 *[a-f0-9]+:	66 0f 1f 00          	nopw   \(%eax\)
 *[a-f0-9]+:	66 f7 10             	notw   \(%eax\)
 *[a-f0-9]+:	66 83 08 01          	orw    \$0x1,\(%eax\)
 *[a-f0-9]+:	66 81 08 89 00       	orw    \$0x89,\(%eax\)
 *[a-f0-9]+:	66 81 08 34 12       	orw    \$0x1234,\(%eax\)
 *[a-f0-9]+:	66 81 08 78 56       	orw    \$0x5678,\(%eax\)
 *[a-f0-9]+:	66 e7 00             	out    %ax,\$0x0
 *[a-f0-9]+:	66 ef                	out    %ax,\(%dx\)
 *[a-f0-9]+:	66 6f                	outsw  %ds:\(%esi\),\(%dx\)
 *[a-f0-9]+:	66 6f                	outsw  %ds:\(%esi\),\(%dx\)
 *[a-f0-9]+:	66 8f 00             	popw   \(%eax\)
 *[a-f0-9]+:	66 07                	popw   %es
 *[a-f0-9]+:	f3 0f ae 20          	ptwrite \(%eax\)
 *[a-f0-9]+:	66 ff 30             	pushw  \(%eax\)
 *[a-f0-9]+:	66 06                	pushw  %es
 *[a-f0-9]+:	66 d1 10             	rclw   \(%eax\)
 *[a-f0-9]+:	66 c1 10 02          	rclw   \$0x2,\(%eax\)
 *[a-f0-9]+:	66 d3 10             	rclw   %cl,\(%eax\)
 *[a-f0-9]+:	66 d1 10             	rclw   \(%eax\)
 *[a-f0-9]+:	66 d1 18             	rcrw   \(%eax\)
 *[a-f0-9]+:	66 c1 18 02          	rcrw   \$0x2,\(%eax\)
 *[a-f0-9]+:	66 d3 18             	rcrw   %cl,\(%eax\)
 *[a-f0-9]+:	66 d1 18             	rcrw   \(%eax\)
 *[a-f0-9]+:	66 d1 00             	rolw   \(%eax\)
 *[a-f0-9]+:	66 c1 00 02          	rolw   \$0x2,\(%eax\)
 *[a-f0-9]+:	66 d3 00             	rolw   %cl,\(%eax\)
 *[a-f0-9]+:	66 d1 00             	rolw   \(%eax\)
 *[a-f0-9]+:	66 d1 08             	rorw   \(%eax\)
 *[a-f0-9]+:	66 c1 08 02          	rorw   \$0x2,\(%eax\)
 *[a-f0-9]+:	66 d3 08             	rorw   %cl,\(%eax\)
 *[a-f0-9]+:	66 d1 08             	rorw   \(%eax\)
 *[a-f0-9]+:	66 83 18 01          	sbbw   \$0x1,\(%eax\)
 *[a-f0-9]+:	66 81 18 89 00       	sbbw   \$0x89,\(%eax\)
 *[a-f0-9]+:	66 81 18 34 12       	sbbw   \$0x1234,\(%eax\)
 *[a-f0-9]+:	66 81 18 78 56       	sbbw   \$0x5678,\(%eax\)
 *[a-f0-9]+:	66 af                	scas   %es:\(%edi\),%ax
 *[a-f0-9]+:	66 af                	scas   %es:\(%edi\),%ax
 *[a-f0-9]+:	66 d1 20             	shlw   \(%eax\)
 *[a-f0-9]+:	66 c1 20 02          	shlw   \$0x2,\(%eax\)
 *[a-f0-9]+:	66 d3 20             	shlw   %cl,\(%eax\)
 *[a-f0-9]+:	66 d1 20             	shlw   \(%eax\)
 *[a-f0-9]+:	66 d1 38             	sarw   \(%eax\)
 *[a-f0-9]+:	66 c1 38 02          	sarw   \$0x2,\(%eax\)
 *[a-f0-9]+:	66 d3 38             	sarw   %cl,\(%eax\)
 *[a-f0-9]+:	66 d1 38             	sarw   \(%eax\)
 *[a-f0-9]+:	66 d1 20             	shlw   \(%eax\)
 *[a-f0-9]+:	66 c1 20 02          	shlw   \$0x2,\(%eax\)
 *[a-f0-9]+:	66 d3 20             	shlw   %cl,\(%eax\)
 *[a-f0-9]+:	66 d1 20             	shlw   \(%eax\)
 *[a-f0-9]+:	66 d1 28             	shrw   \(%eax\)
 *[a-f0-9]+:	66 c1 28 02          	shrw   \$0x2,\(%eax\)
 *[a-f0-9]+:	66 d3 28             	shrw   %cl,\(%eax\)
 *[a-f0-9]+:	66 d1 28             	shrw   \(%eax\)
 *[a-f0-9]+:	66 ab                	stos   %ax,%es:\(%edi\)
 *[a-f0-9]+:	66 ab                	stos   %ax,%es:\(%edi\)
 *[a-f0-9]+:	66 83 28 01          	subw   \$0x1,\(%eax\)
 *[a-f0-9]+:	66 81 28 89 00       	subw   \$0x89,\(%eax\)
 *[a-f0-9]+:	66 81 28 34 12       	subw   \$0x1234,\(%eax\)
 *[a-f0-9]+:	66 81 28 78 56       	subw   \$0x5678,\(%eax\)
 *[a-f0-9]+:	66 f7 00 89 00       	testw  \$0x89,\(%eax\)
 *[a-f0-9]+:	66 f7 00 34 12       	testw  \$0x1234,\(%eax\)
 *[a-f0-9]+:	66 f7 00 78 56       	testw  \$0x5678,\(%eax\)
 *[a-f0-9]+:	c5 fb 2a 00          	vcvtsi2sd \(%eax\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7f 08 2a 00    	\{evex\} vcvtsi2sd \(%eax\),%xmm0,%xmm0
 *[a-f0-9]+:	c5 fa 2a 00          	vcvtsi2ss \(%eax\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7e 08 2a 00    	\{evex\} vcvtsi2ss \(%eax\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7f 08 7b 00    	vcvtusi2sd \(%eax\),%xmm0,%xmm0
 *[a-f0-9]+:	62 f1 7e 08 7b 00    	vcvtusi2ss \(%eax\),%xmm0,%xmm0
 *[a-f0-9]+:	66 83 30 01          	xorw   \$0x1,\(%eax\)
 *[a-f0-9]+:	66 81 30 89 00       	xorw   \$0x89,\(%eax\)
 *[a-f0-9]+:	66 81 30 34 12       	xorw   \$0x1234,\(%eax\)
 *[a-f0-9]+:	66 81 30 78 56       	xorw   \$0x5678,\(%eax\)
#pass
