#name: s390x opcode
#objdump: -drw

.*: +file format .*

Disassembly of section .text:

.* <foo>:
.*:	b3 70 00 62 [	 ]*lpdfr	%f6,%f2
.*:	b3 71 00 62 [	 ]*lndfr	%f6,%f2
.*:	b3 72 10 62 [	 ]*cpsdr	%f6,%f1,%f2
.*:	b3 73 00 62 [	 ]*lcdfr	%f6,%f2
.*:	b3 c1 00 62 [	 ]*ldgr	%f6,%r2
.*:	b3 cd 00 26 [	 ]*lgdr	%r2,%f6
.*:	b3 d2 40 62 [	 ]*adtr	%f6,%f2,%f4
.*:	b3 da 40 89 [	 ]*axtr	%f8,%f9,%f4
.*:	b3 e4 00 62 [	 ]*cdtr	%f6,%f2
.*:	b3 ec 00 10 [	 ]*cxtr	%f1,%f0
.*:	b3 e0 00 62 [	 ]*kdtr	%f6,%f2
.*:	b3 e8 00 62 [	 ]*kxtr	%f6,%f2
.*:	b3 f4 00 62 [	 ]*cedtr	%f6,%f2
.*:	b3 fc 00 10 [	 ]*cextr	%f1,%f0
.*:	b3 f1 00 62 [	 ]*cdgtr	%f6,%r2
.*:	b3 f9 00 12 [	 ]*cxgtr	%f1,%r2
.*:	b3 f3 00 62 [	 ]*cdstr	%f6,%r2
.*:	b3 fb 00 62 [	 ]*cxstr	%f6,%r2
.*:	b3 f2 00 62 [	 ]*cdutr	%f6,%r2
.*:	b3 fa 00 12 [	 ]*cxutr	%f1,%r2
.*:	b3 e1 10 26 [	 ]*cgdtr	%r2,1,%f6
.*:	b3 e9 10 21 [	 ]*cgxtr	%r2,1,%f1
.*:	b3 e3 0d 63 [	 ]*csdtr	%r6,%f3,13
.*:	b3 eb 0d 61 [	 ]*csxtr	%r6,%f1,13
.*:	b3 e2 00 26 [	 ]*cudtr	%r2,%f6
.*:	b3 ea 00 21 [	 ]*cuxtr	%r2,%f1
.*:	b3 d1 40 62 [	 ]*ddtr	%f6,%f2,%f4
.*:	b3 d9 40 10 [	 ]*dxtr	%f1,%f0,%f4
.*:	b3 e5 00 26 [	 ]*eedtr	%r2,%f6
.*:	b3 ed 00 21 [	 ]*eextr	%r2,%f1
.*:	b3 e7 00 26 [	 ]*esdtr	%r2,%f6
.*:	b3 ef 00 21 [	 ]*esxtr	%r2,%f1
.*:	b3 f6 20 64 [	 ]*iedtr	%f6,%f2,%r4
.*:	b3 fe 00 14 [	 ]*iextr	%f1,%f0,%r4
.*:	b3 d6 00 62 [	 ]*ltdtr	%f6,%f2
.*:	b3 de 00 54 [	 ]*ltxtr	%f5,%f4
.*:	b3 d7 13 62 [	 ]*fidtr	%f6,1,%f2,3
.*:	b3 df 13 54 [	 ]*fixtr	%f5,1,%f4,3
.*:	b2 bd 10 03 [	 ]*lfas	3\(%r1\)
.*:	b3 d4 01 62 [	 ]*ldetr	%f6,%f2,1
.*:	b3 dc 01 42 [	 ]*lxdtr	%f4,%f2,1
.*:	b3 d5 13 62 [	 ]*ledtr	%f6,1,%f2,3
.*:	b3 dd 13 64 [	 ]*ldxtr	%f6,1,%f4,3
.*:	b3 d0 40 62 [	 ]*mdtr	%f6,%f2,%f4
.*:	b3 d8 40 98 [	 ]*mxtr	%f9,%f8,%f4
.*:	b3 f5 21 64 [	 ]*qadtr	%f6,%f2,%f4,1
.*:	b3 fd 81 94 [	 ]*qaxtr	%f9,%f8,%f4,1
.*:	b3 f7 21 64 [	 ]*rrdtr	%f6,%f2,%r4,1
.*:	b3 ff 81 94 [	 ]*rrxtr	%f9,%f8,%r4,1
.*:	b2 b9 10 03 [	 ]*srnmt	3\(%r1\)
.*:	b3 85 00 20 [	 ]*sfasr	%r2
.*:	ed 21 40 03 60 40 [	 ]*sldt	%f6,%f2,3\(%r1,%r4\)
.*:	ed 41 40 03 50 48 [	 ]*slxt	%f5,%f4,3\(%r1,%r4\)
.*:	ed 21 40 03 60 41 [	 ]*srdt	%f6,%f2,3\(%r1,%r4\)
.*:	ed 41 40 03 50 49 [	 ]*srxt	%f5,%f4,3\(%r1,%r4\)
.*:	b3 d3 40 62 [	 ]*sdtr	%f6,%f2,%f4
.*:	b3 db 40 51 [	 ]*sxtr	%f5,%f1,%f4
.*:	ed 61 20 03 00 50 [	 ]*tdcet	%f6,3\(%r1,%r2\)
.*:	ed 61 20 03 00 54 [	 ]*tdcdt	%f6,3\(%r1,%r2\)
.*:	ed 51 20 03 00 58 [	 ]*tdcxt	%f5,3\(%r1,%r2\)
.*:	ed 61 20 03 00 51 [	 ]*tdget	%f6,3\(%r1,%r2\)
.*:	ed 61 20 03 00 55 [	 ]*tdgdt	%f6,3\(%r1,%r2\)
.*:	ed 51 20 03 00 59 [	 ]*tdgxt	%f5,3\(%r1,%r2\)
.*:	01 0a [	 ]*pfpo
.*:	c8 31 10 0a 20 14 [	 ]*ectg	10\(%r1\),20\(%r2\),%r3
.*:	c8 32 10 0a 20 14 [	 ]*csst	10\(%r1\),20\(%r2\),%r3
# Expect 2 bytes of padding.
.*:	07 07 [	 ]*nopr	%r7
