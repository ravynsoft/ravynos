#as: -moperand-check=none
#objdump: -dw
#name: 64-bit insns not sizeable through register operands

.*: +file format .*

Disassembly of section .text:

0+ <noreg>:
 *[a-f0-9]+:	83 10 01             	adcl   \$0x1,\(%rax\)
 *[a-f0-9]+:	81 10 89 00 00 00    	adcl   \$0x89,\(%rax\)
 *[a-f0-9]+:	81 10 34 12 00 00    	adcl   \$0x1234,\(%rax\)
 *[a-f0-9]+:	81 10 78 56 34 12    	adcl   \$0x12345678,\(%rax\)
 *[a-f0-9]+:	83 00 01             	addl   \$0x1,\(%rax\)
 *[a-f0-9]+:	81 00 89 00 00 00    	addl   \$0x89,\(%rax\)
 *[a-f0-9]+:	81 00 34 12 00 00    	addl   \$0x1234,\(%rax\)
 *[a-f0-9]+:	81 00 78 56 34 12    	addl   \$0x12345678,\(%rax\)
 *[a-f0-9]+:	83 20 01             	andl   \$0x1,\(%rax\)
 *[a-f0-9]+:	81 20 89 00 00 00    	andl   \$0x89,\(%rax\)
 *[a-f0-9]+:	81 20 34 12 00 00    	andl   \$0x1234,\(%rax\)
 *[a-f0-9]+:	81 20 78 56 34 12    	andl   \$0x12345678,\(%rax\)
 *[a-f0-9]+:	0f ba 20 01          	btl    \$0x1,\(%rax\)
 *[a-f0-9]+:	0f ba 38 01          	btcl   \$0x1,\(%rax\)
 *[a-f0-9]+:	0f ba 30 01          	btrl   \$0x1,\(%rax\)
 *[a-f0-9]+:	0f ba 28 01          	btsl   \$0x1,\(%rax\)
 *[a-f0-9]+:	ff 10                	call   \*\(%rax\)
 *[a-f0-9]+:	83 38 01             	cmpl   \$0x1,\(%rax\)
 *[a-f0-9]+:	81 38 89 00 00 00    	cmpl   \$0x89,\(%rax\)
 *[a-f0-9]+:	81 38 34 12 00 00    	cmpl   \$0x1234,\(%rax\)
 *[a-f0-9]+:	81 38 78 56 34 12    	cmpl   \$0x12345678,\(%rax\)
 *[a-f0-9]+:	a7                   	cmpsl  %es:\(%rdi\),%ds:\(%rsi\)
 *[a-f0-9]+:	a7                   	cmpsl  %es:\(%rdi\),%ds:\(%rsi\)
 *[a-f0-9]+:	f2 0f 38 f1 00       	crc32l \(%rax\),%eax
 *[a-f0-9]+:	f2 48 0f 38 f1 00    	crc32q \(%rax\),%rax
 *[a-f0-9]+:	ff 08                	decl   \(%rax\)
 *[a-f0-9]+:	f7 30                	divl   \(%rax\)
 *[a-f0-9]+:	d8 00                	fadds  \(%rax\)
 *[a-f0-9]+:	d8 10                	fcoms  \(%rax\)
 *[a-f0-9]+:	d8 18                	fcomps \(%rax\)
 *[a-f0-9]+:	d8 30                	fdivs  \(%rax\)
 *[a-f0-9]+:	d8 38                	fdivrs \(%rax\)
 *[a-f0-9]+:	de 00                	fiadds \(%rax\)
 *[a-f0-9]+:	de 10                	ficoms \(%rax\)
 *[a-f0-9]+:	de 18                	ficomps \(%rax\)
 *[a-f0-9]+:	de 30                	fidivs \(%rax\)
 *[a-f0-9]+:	de 38                	fidivrs \(%rax\)
 *[a-f0-9]+:	df 00                	filds  \(%rax\)
 *[a-f0-9]+:	de 08                	fimuls \(%rax\)
 *[a-f0-9]+:	df 10                	fists  \(%rax\)
 *[a-f0-9]+:	df 18                	fistps \(%rax\)
 *[a-f0-9]+:	df 08                	fisttps \(%rax\)
 *[a-f0-9]+:	de 20                	fisubs \(%rax\)
 *[a-f0-9]+:	de 28                	fisubrs \(%rax\)
 *[a-f0-9]+:	d9 00                	flds   \(%rax\)
 *[a-f0-9]+:	d8 08                	fmuls  \(%rax\)
 *[a-f0-9]+:	d9 10                	fsts   \(%rax\)
 *[a-f0-9]+:	d9 18                	fstps  \(%rax\)
 *[a-f0-9]+:	d8 20                	fsubs  \(%rax\)
 *[a-f0-9]+:	d8 28                	fsubrs \(%rax\)
 *[a-f0-9]+:	f7 38                	idivl  \(%rax\)
 *[a-f0-9]+:	f7 28                	imull  \(%rax\)
 *[a-f0-9]+:	e5 00                	in     \$0x0,%eax
 *[a-f0-9]+:	ed                   	in     \(%dx\),%eax
 *[a-f0-9]+:	ff 00                	incl   \(%rax\)
 *[a-f0-9]+:	6d                   	insl   \(%dx\),%es:\(%rdi\)
 *[a-f0-9]+:	6d                   	insl   \(%dx\),%es:\(%rdi\)
 *[a-f0-9]+:	cf                   	iret
 *[a-f0-9]+:	ff 20                	jmp    \*\(%rax\)
 *[a-f0-9]+:	ff 18                	lcall  \*\(%rax\)
 *[a-f0-9]+:	0f 01 10             	lgdt   \(%rax\)
 *[a-f0-9]+:	0f 01 18             	lidt   \(%rax\)
 *[a-f0-9]+:	ff 28                	ljmp   \*\(%rax\)
 *[a-f0-9]+:	0f 00 10             	lldt   \(%rax\)
 *[a-f0-9]+:	0f 01 30             	lmsw   \(%rax\)
 *[a-f0-9]+:	ad                   	lods   %ds:\(%rsi\),%eax
 *[a-f0-9]+:	ad                   	lods   %ds:\(%rsi\),%eax
 *[a-f0-9]+:	cb                   	lret
 *[a-f0-9]+:	ca 04 00             	lret   \$0x4
 *[a-f0-9]+:	0f 00 18             	ltr    \(%rax\)
 *[a-f0-9]+:	c7 00 12 00 00 00    	movl   \$0x12,\(%rax\)
 *[a-f0-9]+:	c7 00 34 12 00 00    	movl   \$0x1234,\(%rax\)
 *[a-f0-9]+:	c7 00 78 56 34 12    	movl   \$0x12345678,\(%rax\)
 *[a-f0-9]+:	8c 00                	mov    %es,\(%rax\)
 *[a-f0-9]+:	8e 00                	mov    \(%rax\),%es
 *[a-f0-9]+:	a5                   	movsl  %ds:\(%rsi\),%es:\(%rdi\)
 *[a-f0-9]+:	a5                   	movsl  %ds:\(%rsi\),%es:\(%rdi\)
 *[a-f0-9]+:	66 0f be 00          	movsbw \(%rax\),%ax
 *[a-f0-9]+:	0f be 00             	movsbl \(%rax\),%eax
 *[a-f0-9]+:	48 0f be 00          	movsbq \(%rax\),%rax
 *[a-f0-9]+:	66 0f b6 00          	movzbw \(%rax\),%ax
 *[a-f0-9]+:	0f b6 00             	movzbl \(%rax\),%eax
 *[a-f0-9]+:	48 0f b6 00          	movzbq \(%rax\),%rax
 *[a-f0-9]+:	f7 20                	mull   \(%rax\)
 *[a-f0-9]+:	f7 18                	negl   \(%rax\)
 *[a-f0-9]+:	0f 1f 00             	nopl   \(%rax\)
 *[a-f0-9]+:	f7 10                	notl   \(%rax\)
 *[a-f0-9]+:	83 08 01             	orl    \$0x1,\(%rax\)
 *[a-f0-9]+:	81 08 89 00 00 00    	orl    \$0x89,\(%rax\)
 *[a-f0-9]+:	81 08 34 12 00 00    	orl    \$0x1234,\(%rax\)
 *[a-f0-9]+:	81 08 78 56 34 12    	orl    \$0x12345678,\(%rax\)
 *[a-f0-9]+:	e7 00                	out    %eax,\$0x0
 *[a-f0-9]+:	ef                   	out    %eax,\(%dx\)
 *[a-f0-9]+:	6f                   	outsl  %ds:\(%rsi\),\(%dx\)
 *[a-f0-9]+:	6f                   	outsl  %ds:\(%rsi\),\(%dx\)
 *[a-f0-9]+:	8f 00                	pop    \(%rax\)
 *[a-f0-9]+:	0f a1                	pop    %fs
 *[a-f0-9]+:	f3 0f ae 20          	ptwritel \(%rax\)
 *[a-f0-9]+:	ff 30                	push   \(%rax\)
 *[a-f0-9]+:	0f a0                	push   %fs
 *[a-f0-9]+:	d1 10                	rcll   \(%rax\)
 *[a-f0-9]+:	c1 10 02             	rcll   \$0x2,\(%rax\)
 *[a-f0-9]+:	d3 10                	rcll   %cl,\(%rax\)
 *[a-f0-9]+:	d1 10                	rcll   \(%rax\)
 *[a-f0-9]+:	d1 18                	rcrl   \(%rax\)
 *[a-f0-9]+:	c1 18 02             	rcrl   \$0x2,\(%rax\)
 *[a-f0-9]+:	d3 18                	rcrl   %cl,\(%rax\)
 *[a-f0-9]+:	d1 18                	rcrl   \(%rax\)
 *[a-f0-9]+:	d1 00                	roll   \(%rax\)
 *[a-f0-9]+:	c1 00 02             	roll   \$0x2,\(%rax\)
 *[a-f0-9]+:	d3 00                	roll   %cl,\(%rax\)
 *[a-f0-9]+:	d1 00                	roll   \(%rax\)
 *[a-f0-9]+:	d1 08                	rorl   \(%rax\)
 *[a-f0-9]+:	c1 08 02             	rorl   \$0x2,\(%rax\)
 *[a-f0-9]+:	d3 08                	rorl   %cl,\(%rax\)
 *[a-f0-9]+:	d1 08                	rorl   \(%rax\)
 *[a-f0-9]+:	83 18 01             	sbbl   \$0x1,\(%rax\)
 *[a-f0-9]+:	81 18 89 00 00 00    	sbbl   \$0x89,\(%rax\)
 *[a-f0-9]+:	81 18 34 12 00 00    	sbbl   \$0x1234,\(%rax\)
 *[a-f0-9]+:	81 18 78 56 34 12    	sbbl   \$0x12345678,\(%rax\)
 *[a-f0-9]+:	af                   	scas   %es:\(%rdi\),%eax
 *[a-f0-9]+:	af                   	scas   %es:\(%rdi\),%eax
 *[a-f0-9]+:	d1 20                	shll   \(%rax\)
 *[a-f0-9]+:	c1 20 02             	shll   \$0x2,\(%rax\)
 *[a-f0-9]+:	d3 20                	shll   %cl,\(%rax\)
 *[a-f0-9]+:	d1 20                	shll   \(%rax\)
 *[a-f0-9]+:	d1 38                	sarl   \(%rax\)
 *[a-f0-9]+:	c1 38 02             	sarl   \$0x2,\(%rax\)
 *[a-f0-9]+:	d3 38                	sarl   %cl,\(%rax\)
 *[a-f0-9]+:	d1 38                	sarl   \(%rax\)
 *[a-f0-9]+:	d1 20                	shll   \(%rax\)
 *[a-f0-9]+:	c1 20 02             	shll   \$0x2,\(%rax\)
 *[a-f0-9]+:	d3 20                	shll   %cl,\(%rax\)
 *[a-f0-9]+:	d1 20                	shll   \(%rax\)
 *[a-f0-9]+:	d1 28                	shrl   \(%rax\)
 *[a-f0-9]+:	c1 28 02             	shrl   \$0x2,\(%rax\)
 *[a-f0-9]+:	d3 28                	shrl   %cl,\(%rax\)
 *[a-f0-9]+:	d1 28                	shrl   \(%rax\)
 *[a-f0-9]+:	ab                   	stos   %eax,%es:\(%rdi\)
 *[a-f0-9]+:	ab                   	stos   %eax,%es:\(%rdi\)
 *[a-f0-9]+:	83 28 01             	subl   \$0x1,\(%rax\)
 *[a-f0-9]+:	81 28 89 00 00 00    	subl   \$0x89,\(%rax\)
 *[a-f0-9]+:	81 28 34 12 00 00    	subl   \$0x1234,\(%rax\)
 *[a-f0-9]+:	81 28 78 56 34 12    	subl   \$0x12345678,\(%rax\)
 *[a-f0-9]+:	0f 35                	sysexitl
 *[a-f0-9]+:	0f 07                	sysretl
 *[a-f0-9]+:	f7 00 89 00 00 00    	testl  \$0x89,\(%rax\)
 *[a-f0-9]+:	f7 00 34 12 00 00    	testl  \$0x1234,\(%rax\)
 *[a-f0-9]+:	f7 00 78 56 34 12    	testl  \$0x12345678,\(%rax\)
 *[a-f0-9]+:	83 30 01             	xorl   \$0x1,\(%rax\)
 *[a-f0-9]+:	81 30 89 00 00 00    	xorl   \$0x89,\(%rax\)
 *[a-f0-9]+:	81 30 34 12 00 00    	xorl   \$0x1234,\(%rax\)
 *[a-f0-9]+:	81 30 78 56 34 12    	xorl   \$0x12345678,\(%rax\)
#pass
