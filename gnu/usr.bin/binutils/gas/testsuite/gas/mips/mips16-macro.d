#objdump: -dr -Mgpr-names=numeric
#as: -mabi=o64
#name: MIPS16 macros

.*: +file format .*mips.*


Disassembly of section \.text:

[ 0-9a-f]+ <foo>:
[ 0-9a-f]+:	eb9a      	div	\$0,\$3,\$4
[ 0-9a-f]+:	2c01      	bnez	\$4,[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	e8e5      	break	0x7
[ 0-9a-f]+:	ea12      	mflo	\$2
[ 0-9a-f]+:	ecbb      	divu	\$0,\$4,\$5
[ 0-9a-f]+:	2d01      	bnez	\$5,[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	e8e5      	break	0x7
[ 0-9a-f]+:	eb12      	mflo	\$3
[ 0-9a-f]+:	edde      	ddiv	\$0,\$5,\$6
[ 0-9a-f]+:	2e01      	bnez	\$6,[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	e8e5      	break	0x7
[ 0-9a-f]+:	ec12      	mflo	\$4
[ 0-9a-f]+:	eeff      	ddivu	\$0,\$6,\$7
[ 0-9a-f]+:	2f01      	bnez	\$7,[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	e8e5      	break	0x7
[ 0-9a-f]+:	ed12      	mflo	\$5
[ 0-9a-f]+:	ef1a      	div	\$0,\$7,\$16
[ 0-9a-f]+:	2801      	bnez	\$16,[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	e8e5      	break	0x7
[ 0-9a-f]+:	ee10      	mfhi	\$6
[ 0-9a-f]+:	ef3b      	divu	\$0,\$7,\$17
[ 0-9a-f]+:	2901      	bnez	\$17,[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	e8e5      	break	0x7
[ 0-9a-f]+:	ee10      	mfhi	\$6
[ 0-9a-f]+:	eb9e      	ddiv	\$0,\$3,\$4
[ 0-9a-f]+:	2c01      	bnez	\$4,[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	e8e5      	break	0x7
[ 0-9a-f]+:	ea10      	mfhi	\$2
[ 0-9a-f]+:	ecbf      	ddivu	\$0,\$4,\$5
[ 0-9a-f]+:	2d01      	bnez	\$5,[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	e8e5      	break	0x7
[ 0-9a-f]+:	eb10      	mfhi	\$3
[ 0-9a-f]+:	edd9      	multu	\$5,\$6
[ 0-9a-f]+:	ec12      	mflo	\$4
[ 0-9a-f]+:	eefd      	dmultu	\$6,\$7
[ 0-9a-f]+:	ed12      	mflo	\$5
[ 0-9a-f]+:	f7ef 4a1f 	addiu	\$2,32767
[ 0-9a-f]+:	4bf0      	addiu	\$3,-16
[ 0-9a-f]+:	f010 4c00 	addiu	\$4,-32768
[ 0-9a-f]+:	f7f7 476f 	addiu	\$3,\$7,16383
[ 0-9a-f]+:	408c      	addiu	\$4,\$16,-4
[ 0-9a-f]+:	f008 41a0 	addiu	\$5,\$17,-16384
[ 0-9a-f]+:	f7ef fd9f 	daddiu	\$4,32767
[ 0-9a-f]+:	fdda      	daddiu	\$6,-6
[ 0-9a-f]+:	f010 fde0 	daddiu	\$7,-32768
[ 0-9a-f]+:	f7f7 445f 	daddiu	\$2,\$4,16383
[ 0-9a-f]+:	4778      	daddiu	\$3,\$7,-8
[ 0-9a-f]+:	f008 4590 	daddiu	\$4,\$5,-16384
[ 0-9a-f]+:	ea6a      	cmp	\$2,\$3
[ 0-9a-f]+:	60fe      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	ecaa      	cmp	\$4,\$5
[ 0-9a-f]+:	61fe      	btnez	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	eee2      	slt	\$6,\$7
[ 0-9a-f]+:	61fe      	btnez	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	e823      	sltu	\$16,\$17
[ 0-9a-f]+:	61fe      	btnez	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	ef82      	slt	\$7,\$4
[ 0-9a-f]+:	60fe      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	eea3      	sltu	\$6,\$5
[ 0-9a-f]+:	60fe      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	ec02      	slt	\$4,\$16
[ 0-9a-f]+:	60fe      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	ed23      	sltu	\$5,\$17
[ 0-9a-f]+:	60fe      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	ee82      	slt	\$6,\$4
[ 0-9a-f]+:	61fe      	btnez	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	efa3      	sltu	\$7,\$5
[ 0-9a-f]+:	61fe      	btnez	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	7201      	cmpi	\$2,1
[ 0-9a-f]+:	60fe      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f7ff 731f 	cmpi	\$3,65535
[ 0-9a-f]+:	60fd      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	7401      	cmpi	\$4,1
[ 0-9a-f]+:	61fe      	btnez	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f7ff 751f 	cmpi	\$5,65535
[ 0-9a-f]+:	61fd      	btnez	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f010 5600 	slti	\$6,-32768
[ 0-9a-f]+:	61fd      	btnez	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f7ef 571f 	slti	\$7,32767
[ 0-9a-f]+:	61fd      	btnez	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f010 5800 	sltiu	\$16,-32768
[ 0-9a-f]+:	61fd      	btnez	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f7ef 591f 	sltiu	\$17,32767
[ 0-9a-f]+:	61fd      	btnez	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f010 5200 	slti	\$2,-32768
[ 0-9a-f]+:	61fd      	btnez	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f7ef 531f 	slti	\$3,32767
[ 0-9a-f]+:	61fd      	btnez	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f010 5c00 	sltiu	\$4,-32768
[ 0-9a-f]+:	61fd      	btnez	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f7ef 5d1f 	sltiu	\$5,32767
[ 0-9a-f]+:	61fd      	btnez	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f010 5600 	slti	\$6,-32768
[ 0-9a-f]+:	60fd      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f7ef 571e 	slti	\$7,32766
[ 0-9a-f]+:	60fd      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f010 5800 	sltiu	\$16,-32768
[ 0-9a-f]+:	60fd      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f7ef 591f 	sltiu	\$17,32767
[ 0-9a-f]+:	60fd      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f010 5200 	slti	\$2,-32768
[ 0-9a-f]+:	60fd      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f7ef 531f 	slti	\$3,32767
[ 0-9a-f]+:	60fd      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f010 5c00 	sltiu	\$4,-32768
[ 0-9a-f]+:	60fd      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	f7ef 5d1f 	sltiu	\$5,32767
[ 0-9a-f]+:	60fd      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	5200      	slti	\$2,0
[ 0-9a-f]+:	6001      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	ea4b      	neg	\$2
[ 0-9a-f]+:	5300      	slti	\$3,0
[ 0-9a-f]+:	6001      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	eb6b      	neg	\$3
[ 0-9a-f]+:	5500      	slti	\$5,0
[ 0-9a-f]+:	6785      	move	\$4,\$5
[ 0-9a-f]+:	6001      	bteqz	[0-9a-f]+ <[^>]*>
[ 0-9a-f]+:	ec8b      	neg	\$4
#pass
