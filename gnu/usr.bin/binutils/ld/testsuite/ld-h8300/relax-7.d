# name: H8300 Relaxation Test 7
# source: relax-7?.s
# ld: --relax
# objdump: -d -s --no-show-raw-insn

.*:     file format .*

Contents of section .text:
 0100 1a801aa2 7a01ffff 80000100 6f2201d0  [^\000]*
 0110 59206e0a ff016e8a ff016e1a 00016e9a  [^\000]*
 0120 00015470 6f02fff2 6f82fff2 6f120002  [^\000]*
 0130 6f920002 54700100 6f028004 01006f82  [^\000]*
 0140 80040100 6f120004 01006f92 00045470  [^\000]*
 0150 7a000100 78006b01 fff25470 78006a2a  [^\000]*
 0160 ffff7ff1 78006aaa ffff7ff1 78106a2a  [^\000]*
 0170 00008000 78106aaa 00008000 54707800  [^\000]*
 0180 6b22ffff 7ffa7800 6ba2ffff 7ffa7810  [^\000]*
 0190 6b220000 80007810 6ba20000 80005470  [^\000]*
 01a0 01007800 6b2200ff ff040100 78806ba2  [^\000]*
 01b0 00ffff04 01007810 6b220000 80000100  [^\000]*
 01c0 78906ba2 00008000 5470               [^\000]*
Contents of section .rodata:
 01cc 00000112 00000124 00000136 0000015c  [^\000]*
 01dc 0000017e 000001a0 00000150 01007800  [^\000]*
 01ec 6b200000 01e80000                    [^\000]*

Disassembly of section .text:

00000100 <_start>:
 100:	sub.l	er0,er0
 102:	sub.l	er2,er2
 104:	mov.l	#0xffff8000,er1
 10a:	mov.l	@\(0x1d0:16,er2\),er2
 110:	jmp	@er2

00000112 <.L20>:
 112:	mov.b	@\(0xff01:16,er0\),r2l
 116:	mov.b	r2l,@\(0xff01:16,er0\)
 11a:	mov.b	@\(0x1:16,er1\),r2l
 11e:	mov.b	r2l,@\(0x1:16,er1\)
 122:	rts[\t]*

00000124 <.L21>:
 124:	mov.w	@\(0xfff2:16,er0\),r2
 128:	mov.w	r2,@\(0xfff2:16,er0\)
 12c:	mov.w	@\(0x2:16,er1\),r2
 130:	mov.w	r2,@\(0x2:16,er1\)
 134:	rts[\t]*

00000136 <.L22>:
 136:	mov.l	@\(0x8004:16,er0\),er2
 13c:	mov.l	er2,@\(0x8004:16,er0\)
 142:	mov.l	@\(0x4:16,er1\),er2
 148:	mov.l	er2,@\(0x4:16,er1\)
 14e:	rts[\t]*

00000150 <.L100Relax>:
 150:	mov.l	#0x1007800,er0
 156:	mov.w	@0xfff2:16,r1
 15a:	rts[\t]*

0000015c <.L30noRelax>:
 15c:	mov.b	@\(0xffff7ff1:32,er0\),r2l
 164:	mov.b	r2l,@\(0xffff7ff1:32,er0\)
 16c:	mov.b	@\(0x8000:32,er1\),r2l
 174:	mov.b	r2l,@\(0x8000:32,er1\)
 17c:	rts[\t]*

0000017e <.L31noRelax>:
 17e:	mov.w	@\(0xffff7ffa:32,er0\),r2
 186:	mov.w	r2,@\(0xffff7ffa:32,er0\)
 18e:	mov.w	@\(0x8000:32,er1\),r2
 196:	mov.w	r2,@\(0x8000:32,er1\)
 19e:	rts[\t]*

000001a0 <.L32noRelax>:
 1a0:	mov.l	@\(0xffff04:32,er0\),er2
 1aa:	mov.l	er2,@\(0xffff04:32,er0\)
 1b4:	mov.l	@\(0x8000:32,er1\),er2
 1be:	mov.l	er2,@\(0x8000:32,er1\)
 1c8:	rts[\t]*
