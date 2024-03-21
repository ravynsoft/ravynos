#as: -march=armv8-a+memtag
# objdump: -d

.*: .*


Disassembly of section \.text:

0+0 <func>:
.*:	9ac01000 	irg	x0, x0, x0
.*:	9ac0101b 	irg	x27, x0, x0
.*:	9ac01360 	irg	x0, x27, x0
.*:	9adb1000 	irg	x0, x0, x27
.*:	9adb137b 	irg	x27, x27, x27
.*:	9adf101f 	irg	sp, x0
.*:	9adf13e0 	irg	x0, sp
.*:	9ac01400 	gmi	x0, x0, x0
.*:	9ac0141b 	gmi	x27, x0, x0
.*:	9ac01760 	gmi	x0, x27, x0
.*:	9adb1400 	gmi	x0, x0, x27
.*:	9adb177b 	gmi	x27, x27, x27
.*:	9ac017e0 	gmi	x0, sp, x0
.*:	9ac0141f 	gmi	xzr, x0, x0
.*:	91800000 	addg	x0, x0, #0x0, #0x0
.*:	9180001b 	addg	x27, x0, #0x0, #0x0
.*:	91800360 	addg	x0, x27, #0x0, #0x0
.*:	9180037b 	addg	x27, x27, #0x0, #0x0
.*:	91bf3fe0 	addg	x0, sp, #0x3f0, #0xf
.*:	91aa3c1f 	addg	sp, x0, #0x2a0, #0xf
.*:	d1800000 	subg	x0, x0, #0x0, #0x0
.*:	d180001b 	subg	x27, x0, #0x0, #0x0
.*:	d1800360 	subg	x0, x27, #0x0, #0x0
.*:	d180037b 	subg	x27, x27, #0x0, #0x0
.*:	d1bf3fe0 	subg	x0, sp, #0x3f0, #0xf
.*:	d1bf141f 	subg	sp, x0, #0x3f0, #0x5
.*:	9ac00000 	subp	x0, x0, x0
.*:	9ac0001b 	subp	x27, x0, x0
.*:	9ac00360 	subp	x0, x27, x0
.*:	9adb0000 	subp	x0, x0, x27
.*:	9adb037b 	subp	x27, x27, x27
.*:	9ac003e0 	subp	x0, sp, x0
.*:	9adf0000 	subp	x0, x0, sp
.*:	9ac0001f 	subp	xzr, x0, x0
.*:	bac00000 	subps	x0, x0, x0
.*:	bac0001b 	subps	x27, x0, x0
.*:	bac00360 	subps	x0, x27, x0
.*:	badb0000 	subps	x0, x0, x27
.*:	badb037b 	subps	x27, x27, x27
.*:	bac003e0 	subps	x0, sp, x0
.*:	badf0000 	subps	x0, x0, sp
.*:	bac0001f 	cmpp	x0, x0
.*:	bac0001f 	cmpp	x0, x0
.*:	bac0037f 	cmpp	x27, x0
.*:	badb001f 	cmpp	x0, x27
.*:	badb037f 	cmpp	x27, x27
.*:	bac003ff 	cmpp	sp, x0
.*:	badf001f 	cmpp	x0, sp
.*:	d9200800 	stg	x0, \[x0\]
.*:	d9200b60 	stg	x0, \[x27\]
.*:	d920081f 	stg	sp, \[x0\]
.*:	d93fb81b 	stg	x27, \[x0, #-80\]
.*:	d9200c00 	stg	x0, \[x0, #0\]!
.*:	d9200c1f 	stg	sp, \[x0, #0\]!
.*:	d920ac1b 	stg	x27, \[x0, #160\]!
.*:	d9200400 	stg	x0, \[x0\], #0
.*:	d920041f 	stg	sp, \[x0\], #0
.*:	d93a641b 	stg	x27, \[x0\], #-1440
.*:	d92ffbe0 	stg	x0, \[sp, #4080\]
.*:	d92ffbff 	stg	sp, \[sp, #4080\]
.*:	d9300bfb 	stg	x27, \[sp, #-4096\]
.*:	d92fffe0 	stg	x0, \[sp, #4080\]!
.*:	d93007ff 	stg	sp, \[sp\], #-4096
.*:	d9600800 	stzg	x0, \[x0\]
.*:	d9600b60 	stzg	x0, \[x27\]
.*:	d960081f 	stzg	sp, \[x0\]
.*:	d97fb81b 	stzg	x27, \[x0, #-80\]
.*:	d9600c00 	stzg	x0, \[x0, #0\]!
.*:	d9600c1f 	stzg	sp, \[x0, #0\]!
.*:	d960ac1b 	stzg	x27, \[x0, #160\]!
.*:	d9600400 	stzg	x0, \[x0\], #0
.*:	d960041f 	stzg	sp, \[x0\], #0
.*:	d97a641b 	stzg	x27, \[x0\], #-1440
.*:	d96ffbe0 	stzg	x0, \[sp, #4080\]
.*:	d96ffbff 	stzg	sp, \[sp, #4080\]
.*:	d9700bfb 	stzg	x27, \[sp, #-4096\]
.*:	d96fffe0 	stzg	x0, \[sp, #4080\]!
.*:	d97007ff 	stzg	sp, \[sp\], #-4096
.*:	d9a00800 	st2g	x0, \[x0\]
.*:	d9a00b60 	st2g	x0, \[x27\]
.*:	d9a0081f 	st2g	sp, \[x0\]
.*:	d9bfb81b 	st2g	x27, \[x0, #-80\]
.*:	d9a00c00 	st2g	x0, \[x0, #0\]!
.*:	d9a00c1f 	st2g	sp, \[x0, #0\]!
.*:	d9a0ac1b 	st2g	x27, \[x0, #160\]!
.*:	d9a00400 	st2g	x0, \[x0\], #0
.*:	d9a0041f 	st2g	sp, \[x0\], #0
.*:	d9ba641b 	st2g	x27, \[x0\], #-1440
.*:	d9affbe0 	st2g	x0, \[sp, #4080\]
.*:	d9affbff 	st2g	sp, \[sp, #4080\]
.*:	d9b00bfb 	st2g	x27, \[sp, #-4096\]
.*:	d9afffe0 	st2g	x0, \[sp, #4080\]!
.*:	d9b007ff 	st2g	sp, \[sp\], #-4096
.*:	d9e00800 	stz2g	x0, \[x0\]
.*:	d9e00b60 	stz2g	x0, \[x27\]
.*:	d9e0081f 	stz2g	sp, \[x0\]
.*:	d9ffb81b 	stz2g	x27, \[x0, #-80\]
.*:	d9e00c00 	stz2g	x0, \[x0, #0\]!
.*:	d9e00c1f 	stz2g	sp, \[x0, #0\]!
.*:	d9e0ac1b 	stz2g	x27, \[x0, #160\]!
.*:	d9e00400 	stz2g	x0, \[x0\], #0
.*:	d9e0041f 	stz2g	sp, \[x0\], #0
.*:	d9fa641b 	stz2g	x27, \[x0\], #-1440
.*:	d9effbe0 	stz2g	x0, \[sp, #4080\]
.*:	d9effbff 	stz2g	sp, \[sp, #4080\]
.*:	d9f00bfb 	stz2g	x27, \[sp, #-4096\]
.*:	d9efffe0 	stz2g	x0, \[sp, #4080\]!
.*:	d9f007ff 	stz2g	sp, \[sp\], #-4096
.*:	69000000 	stgp	x0, x0, \[x0\]
.*:	69006c00 	stgp	x0, x27, \[x0\]
.*:	6900001b 	stgp	x27, x0, \[x0\]
.*:	69006c1b 	stgp	x27, x27, \[x0\]
.*:	69000360 	stgp	x0, x0, \[x27\]
.*:	693d8000 	stgp	x0, x0, \[x0, #-80\]
.*:	69800000 	stgp	x0, x0, \[x0, #0\]!
.*:	69850000 	stgp	x0, x0, \[x0, #160\]!
.*:	68800000 	stgp	x0, x0, \[x0\], #0
.*:	68bb8000 	stgp	x0, x0, \[x0\], #-144
.*:	691f801f 	stgp	xzr, x0, \[x0, #1008\]
.*:	69207c00 	stgp	x0, xzr, \[x0, #-1024\]
.*:	699f83e0 	stgp	x0, x0, \[sp, #1008\]!
.*:	68a003e0 	stgp	x0, x0, \[sp\], #-1024
.*:	d9600000 	ldg	x0, \[x0\]
.*:	d960001b 	ldg	x27, \[x0\]
.*:	d9600360 	ldg	x0, \[x27\]
.*:	d960037b 	ldg	x27, \[x27\]
.*:	d96003e0 	ldg	x0, \[sp\]
.*:	d960001f 	ldg	xzr, \[x0\]
.*:	d96ff000 	ldg	x0, \[x0, #4080\]
.*:	d9700000 	ldg	x0, \[x0, #-4096\]
.*:	d9200000 	stzgm	x0, \[x0\]
.*:	d920001b 	stzgm	x27, \[x0\]
.*:	d9200360 	stzgm	x0, \[x27\]
.*:	d9200379 	stzgm	x25, \[x27\]
.*:	d92003e0 	stzgm	x0, \[sp\]
.*:	d920001f 	stzgm	xzr, \[x0\]
.*:	d9e00000 	ldgm	x0, \[x0\]
.*:	d9e0001b 	ldgm	x27, \[x0\]
.*:	d9e00360 	ldgm	x0, \[x27\]
.*:	d9e00379 	ldgm	x25, \[x27\]
.*:	d9e003e0 	ldgm	x0, \[sp\]
.*:	d9e0001f 	ldgm	xzr, \[x0\]
.*:	d9a00000 	stgm	x0, \[x0\]
.*:	d9a0001b 	stgm	x27, \[x0\]
.*:	d9a00360 	stgm	x0, \[x27\]
.*:	d9a00379 	stgm	x25, \[x27\]
.*:	d9a003e0 	stgm	x0, \[sp\]
.*:	d9a0001f 	stgm	xzr, \[x0\]
