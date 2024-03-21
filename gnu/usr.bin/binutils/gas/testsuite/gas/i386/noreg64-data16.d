#as: --defsym DATA16=1
#objdump: -dw
#name: 64-bit insns not sizeable through register operands w/ data16
#source: noreg64.s
#warning_output: noreg64-data16.e

.*: +file format .*

Disassembly of section .text:

0+ <noreg>:
 *[a-f0-9]+:	66 83 10 01          	adcw   \$0x1,\(%rax\)
 *[a-f0-9]+:	66 81 10 89 00       	adcw   \$0x89,\(%rax\)
 *[a-f0-9]+:	66 81 10 34 12       	adcw   \$0x1234,\(%rax\)
 *[a-f0-9]+:	66 81 10 78 56       	adcw   \$0x5678,\(%rax\)
 *[a-f0-9]+:	66 83 00 01          	addw   \$0x1,\(%rax\)
 *[a-f0-9]+:	66 81 00 89 00       	addw   \$0x89,\(%rax\)
 *[a-f0-9]+:	66 81 00 34 12       	addw   \$0x1234,\(%rax\)
 *[a-f0-9]+:	66 81 00 78 56       	addw   \$0x5678,\(%rax\)
 *[a-f0-9]+:	66 83 20 01          	andw   \$0x1,\(%rax\)
 *[a-f0-9]+:	66 81 20 89 00       	andw   \$0x89,\(%rax\)
 *[a-f0-9]+:	66 81 20 34 12       	andw   \$0x1234,\(%rax\)
 *[a-f0-9]+:	66 81 20 78 56       	andw   \$0x5678,\(%rax\)
 *[a-f0-9]+:	66 0f ba 20 01       	btw    \$0x1,\(%rax\)
 *[a-f0-9]+:	66 0f ba 38 01       	btcw   \$0x1,\(%rax\)
 *[a-f0-9]+:	66 0f ba 30 01       	btrw   \$0x1,\(%rax\)
 *[a-f0-9]+:	66 0f ba 28 01       	btsw   \$0x1,\(%rax\)
 *[a-f0-9]+:	66 ff 10             	callw  \*\(%rax\)
 *[a-f0-9]+:	66 83 38 01          	cmpw   \$0x1,\(%rax\)
 *[a-f0-9]+:	66 81 38 89 00       	cmpw   \$0x89,\(%rax\)
 *[a-f0-9]+:	66 81 38 34 12       	cmpw   \$0x1234,\(%rax\)
 *[a-f0-9]+:	66 81 38 78 56       	cmpw   \$0x5678,\(%rax\)
 *[a-f0-9]+:	66 a7                	cmpsw  %es:\(%rdi\),%ds:\(%rsi\)
 *[a-f0-9]+:	66 a7                	cmpsw  %es:\(%rdi\),%ds:\(%rsi\)
 *[a-f0-9]+:	66 f2 0f 38 f1 00    	crc32w \(%rax\),%eax
 *[a-f0-9]+:	66 f2 48 0f 38 f1 00 	data16 crc32q \(%rax\),%rax
 *[a-f0-9]+:	66 ff 08             	decw   \(%rax\)
 *[a-f0-9]+:	66 f7 30             	divw   \(%rax\)
 *[a-f0-9]+:	66 d8 00             	data16 fadds \(%rax\)
 *[a-f0-9]+:	66 d8 10             	data16 fcoms \(%rax\)
 *[a-f0-9]+:	66 d8 18             	data16 fcomps \(%rax\)
 *[a-f0-9]+:	66 d8 30             	data16 fdivs \(%rax\)
 *[a-f0-9]+:	66 d8 38             	data16 fdivrs \(%rax\)
 *[a-f0-9]+:	66 de 00             	data16 fiadds \(%rax\)
 *[a-f0-9]+:	66 de 10             	data16 ficoms \(%rax\)
 *[a-f0-9]+:	66 de 18             	data16 ficomps \(%rax\)
 *[a-f0-9]+:	66 de 30             	data16 fidivs \(%rax\)
 *[a-f0-9]+:	66 de 38             	data16 fidivrs \(%rax\)
 *[a-f0-9]+:	66 df 00             	data16 filds \(%rax\)
 *[a-f0-9]+:	66 de 08             	data16 fimuls \(%rax\)
 *[a-f0-9]+:	66 df 10             	data16 fists \(%rax\)
 *[a-f0-9]+:	66 df 18             	data16 fistps \(%rax\)
 *[a-f0-9]+:	66 df 08             	data16 fisttps \(%rax\)
 *[a-f0-9]+:	66 de 20             	data16 fisubs \(%rax\)
 *[a-f0-9]+:	66 de 28             	data16 fisubrs \(%rax\)
 *[a-f0-9]+:	66 d9 00             	data16 flds \(%rax\)
 *[a-f0-9]+:	66 d8 08             	data16 fmuls \(%rax\)
 *[a-f0-9]+:	66 d9 10             	data16 fsts \(%rax\)
 *[a-f0-9]+:	66 d9 18             	data16 fstps \(%rax\)
 *[a-f0-9]+:	66 d8 20             	data16 fsubs \(%rax\)
 *[a-f0-9]+:	66 d8 28             	data16 fsubrs \(%rax\)
 *[a-f0-9]+:	66 f7 38             	idivw  \(%rax\)
 *[a-f0-9]+:	66 f7 28             	imulw  \(%rax\)
 *[a-f0-9]+:	66 e5 00             	in     \$0x0,%ax
 *[a-f0-9]+:	66 ed                	in     \(%dx\),%ax
 *[a-f0-9]+:	66 ff 00             	incw   \(%rax\)
 *[a-f0-9]+:	66 6d                	insw   \(%dx\),%es:\(%rdi\)
 *[a-f0-9]+:	66 6d                	insw   \(%dx\),%es:\(%rdi\)
 *[a-f0-9]+:	66 cf                	iretw
 *[a-f0-9]+:	66 ff 20             	jmpw   \*\(%rax\)
 *[a-f0-9]+:	66 ff 18             	lcallw \*\(%rax\)
 *[a-f0-9]+:	66 0f 01 10          	data16 lgdt \(%rax\)
 *[a-f0-9]+:	66 0f 01 18          	data16 lidt \(%rax\)
 *[a-f0-9]+:	66 ff 28             	ljmpw  \*\(%rax\)
 *[a-f0-9]+:	66 0f 00 10          	data16 lldt \(%rax\)
 *[a-f0-9]+:	66 0f 01 30          	data16 lmsw \(%rax\)
 *[a-f0-9]+:	66 ad                	lods   %ds:\(%rsi\),%ax
 *[a-f0-9]+:	66 ad                	lods   %ds:\(%rsi\),%ax
 *[a-f0-9]+:	66 cb                	lretw
 *[a-f0-9]+:	66 ca 04 00          	lretw  \$0x4
 *[a-f0-9]+:	66 0f 00 18          	data16 ltr \(%rax\)
 *[a-f0-9]+:	66 c7 00 12 00       	movw   \$0x12,\(%rax\)
 *[a-f0-9]+:	66 c7 00 34 12       	movw   \$0x1234,\(%rax\)
 *[a-f0-9]+:	66 c7 00 78 56       	movw   \$0x5678,\(%rax\)
 *[a-f0-9]+:	66 8c 00             	data16 mov %es,\(%rax\)
 *[a-f0-9]+:	66 8e 00             	data16 mov \(%rax\),%es
 *[a-f0-9]+:	66 a5                	movsw  %ds:\(%rsi\),%es:\(%rdi\)
 *[a-f0-9]+:	66 a5                	movsw  %ds:\(%rsi\),%es:\(%rdi\)
 *[a-f0-9]+:	66 0f be 00          	movsbw \(%rax\),%ax
 *[a-f0-9]+:	66 48 0f be 00       	data16 movsbq \(%rax\),%rax
 *[a-f0-9]+:	66 0f b6 00          	movzbw \(%rax\),%ax
 *[a-f0-9]+:	66 48 0f b6 00       	data16 movzbq \(%rax\),%rax
 *[a-f0-9]+:	66 f7 20             	mulw   \(%rax\)
 *[a-f0-9]+:	66 f7 18             	negw   \(%rax\)
 *[a-f0-9]+:	66 0f 1f 00          	nopw   \(%rax\)
 *[a-f0-9]+:	66 f7 10             	notw   \(%rax\)
 *[a-f0-9]+:	66 83 08 01          	orw    \$0x1,\(%rax\)
 *[a-f0-9]+:	66 81 08 89 00       	orw    \$0x89,\(%rax\)
 *[a-f0-9]+:	66 81 08 34 12       	orw    \$0x1234,\(%rax\)
 *[a-f0-9]+:	66 81 08 78 56       	orw    \$0x5678,\(%rax\)
 *[a-f0-9]+:	66 e7 00             	out    %ax,\$0x0
 *[a-f0-9]+:	66 ef                	out    %ax,\(%dx\)
 *[a-f0-9]+:	66 6f                	outsw  %ds:\(%rsi\),\(%dx\)
 *[a-f0-9]+:	66 6f                	outsw  %ds:\(%rsi\),\(%dx\)
 *[a-f0-9]+:	66 8f 00             	popw   \(%rax\)
 *[a-f0-9]+:	66 0f a1             	popw   %fs
 *[a-f0-9]+:	66 ff 30             	pushw  \(%rax\)
 *[a-f0-9]+:	66 0f a0             	pushw  %fs
 *[a-f0-9]+:	66 d1 10             	rclw   \(%rax\)
 *[a-f0-9]+:	66 c1 10 02          	rclw   \$0x2,\(%rax\)
 *[a-f0-9]+:	66 d3 10             	rclw   %cl,\(%rax\)
 *[a-f0-9]+:	66 d1 10             	rclw   \(%rax\)
 *[a-f0-9]+:	66 d1 18             	rcrw   \(%rax\)
 *[a-f0-9]+:	66 c1 18 02          	rcrw   \$0x2,\(%rax\)
 *[a-f0-9]+:	66 d3 18             	rcrw   %cl,\(%rax\)
 *[a-f0-9]+:	66 d1 18             	rcrw   \(%rax\)
 *[a-f0-9]+:	66 d1 00             	rolw   \(%rax\)
 *[a-f0-9]+:	66 c1 00 02          	rolw   \$0x2,\(%rax\)
 *[a-f0-9]+:	66 d3 00             	rolw   %cl,\(%rax\)
 *[a-f0-9]+:	66 d1 00             	rolw   \(%rax\)
 *[a-f0-9]+:	66 d1 08             	rorw   \(%rax\)
 *[a-f0-9]+:	66 c1 08 02          	rorw   \$0x2,\(%rax\)
 *[a-f0-9]+:	66 d3 08             	rorw   %cl,\(%rax\)
 *[a-f0-9]+:	66 d1 08             	rorw   \(%rax\)
 *[a-f0-9]+:	66 83 18 01          	sbbw   \$0x1,\(%rax\)
 *[a-f0-9]+:	66 81 18 89 00       	sbbw   \$0x89,\(%rax\)
 *[a-f0-9]+:	66 81 18 34 12       	sbbw   \$0x1234,\(%rax\)
 *[a-f0-9]+:	66 81 18 78 56       	sbbw   \$0x5678,\(%rax\)
 *[a-f0-9]+:	66 af                	scas   %es:\(%rdi\),%ax
 *[a-f0-9]+:	66 af                	scas   %es:\(%rdi\),%ax
 *[a-f0-9]+:	66 d1 20             	shlw   \(%rax\)
 *[a-f0-9]+:	66 c1 20 02          	shlw   \$0x2,\(%rax\)
 *[a-f0-9]+:	66 d3 20             	shlw   %cl,\(%rax\)
 *[a-f0-9]+:	66 d1 20             	shlw   \(%rax\)
 *[a-f0-9]+:	66 d1 38             	sarw   \(%rax\)
 *[a-f0-9]+:	66 c1 38 02          	sarw   \$0x2,\(%rax\)
 *[a-f0-9]+:	66 d3 38             	sarw   %cl,\(%rax\)
 *[a-f0-9]+:	66 d1 38             	sarw   \(%rax\)
 *[a-f0-9]+:	66 d1 20             	shlw   \(%rax\)
 *[a-f0-9]+:	66 c1 20 02          	shlw   \$0x2,\(%rax\)
 *[a-f0-9]+:	66 d3 20             	shlw   %cl,\(%rax\)
 *[a-f0-9]+:	66 d1 20             	shlw   \(%rax\)
 *[a-f0-9]+:	66 d1 28             	shrw   \(%rax\)
 *[a-f0-9]+:	66 c1 28 02          	shrw   \$0x2,\(%rax\)
 *[a-f0-9]+:	66 d3 28             	shrw   %cl,\(%rax\)
 *[a-f0-9]+:	66 d1 28             	shrw   \(%rax\)
 *[a-f0-9]+:	66 ab                	stos   %ax,%es:\(%rdi\)
 *[a-f0-9]+:	66 ab                	stos   %ax,%es:\(%rdi\)
 *[a-f0-9]+:	66 83 28 01          	subw   \$0x1,\(%rax\)
 *[a-f0-9]+:	66 81 28 89 00       	subw   \$0x89,\(%rax\)
 *[a-f0-9]+:	66 81 28 34 12       	subw   \$0x1234,\(%rax\)
 *[a-f0-9]+:	66 81 28 78 56       	subw   \$0x5678,\(%rax\)
 *[a-f0-9]+:	66 0f 35             	data16 sysexitl
 *[a-f0-9]+:	66 0f 07             	data16 sysretl
 *[a-f0-9]+:	66 f7 00 89 00       	testw  \$0x89,\(%rax\)
 *[a-f0-9]+:	66 f7 00 34 12       	testw  \$0x1234,\(%rax\)
 *[a-f0-9]+:	66 f7 00 78 56       	testw  \$0x5678,\(%rax\)
 *[a-f0-9]+:	66 83 30 01          	xorw   \$0x1,\(%rax\)
 *[a-f0-9]+:	66 81 30 89 00       	xorw   \$0x89,\(%rax\)
 *[a-f0-9]+:	66 81 30 34 12       	xorw   \$0x1234,\(%rax\)
 *[a-f0-9]+:	66 81 30 78 56       	xorw   \$0x5678,\(%rax\)
#pass
