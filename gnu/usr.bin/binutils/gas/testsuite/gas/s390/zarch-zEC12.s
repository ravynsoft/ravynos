.text
foo:
	etnd	%r6
	ntstg	%r6,-5555(%r7,%r8)
	tabort	4000(%r6)
	tbegin	4000(%r6),65000
	tbeginc	4000(%r6),65000
	tend
	bpp	10,.,4000(%r6)
	bprp	10,.,.+24
	niai	10,13
	lat	%r6,-5555(%r7,%r8)
	lgat	%r6,-5555(%r7,%r8)
	lfhat	%r6,-5555(%r7,%r8)
	llgfat	%r6,-5555(%r7,%r8)
	llgtat	%r6,-5555(%r7,%r8)

	clt	%r6,10,-5555(%r7)
	clth	%r6,-5555(%r7)
	cltnle	%r6,-5555(%r7)
	cltl	%r6,-5555(%r7)
	cltnhe	%r6,-5555(%r7)
	cltlh	%r6,-5555(%r7)
	cltne	%r6,-5555(%r7)
	clte	%r6,-5555(%r7)
	cltnlh	%r6,-5555(%r7)
	clthe	%r6,-5555(%r7)
	cltnl	%r6,-5555(%r7)
	cltle	%r6,-5555(%r7)
	cltnh	%r6,-5555(%r7)

	clgt	%r6,10,-5555(%r7)
	clgth	%r6,-5555(%r7)
	clgtnle	%r6,-5555(%r7)
	clgtl	%r6,-5555(%r7)
	clgtnhe	%r6,-5555(%r7)
	clgtlh	%r6,-5555(%r7)
	clgtne	%r6,-5555(%r7)
	clgte	%r6,-5555(%r7)
	clgtnlh	%r6,-5555(%r7)
	clgthe	%r6,-5555(%r7)
	clgtnl	%r6,-5555(%r7)
	clgtle	%r6,-5555(%r7)
	clgtnh	%r6,-5555(%r7)

	risbgn	%r6,%r7,12,13,14
	risbgn	%r6,%r7,12,188,14
	risbgnz	%r6,%r7,12,20,14

	cdzt	%f6,4000(16,%r8),13
	cxzt	%f4,4000(34,%r8),13
	czdt	%f6,4000(16,%r8),13
	czxt	%f4,4000(34,%r8),13

	ppa	%r5,%r6,12
	crdte	%r5,%r6,%r9
	crdte	%r5,%r6,%r9,1

	bprp	10,bar,bar
	bprp	10,bar@PLT,bar@PLT

	bpp	10,bar@PLT,0
	bpp	10,baz,0
bar:
