#source: start1.s
#source: tls128.s
#source: tls-le-13s.s
#source: tls-gd-2.s --pic
#source: tls-ldgd-14.s --pic
#source: tls-x.s
#source: tls-z.s
#source: tls-x1x2.s
#as: --no-underscore --em=criself
#ld: -m crislinux
#objdump: -d -s -h -t -r -p

# Check that we have proper NPTL/TLS markings and GOT for an
# executable with two R_CRIS_16_TPREL a R_CRIS_32_GOT_GD and a
# R_CRIS_16_GOT_GD, different symbols.

.*:     file format elf32-cris

Program Header:
#...
     TLS off    0x0+ac vaddr 0x0+820ac paddr 0x0+820ac align 2\*\*2
         filesz 0x0+90 memsz 0x0+90 flags r--
private flags = 0:

#...
  2 .got .*
                  CONTENTS.*
SYMBOL TABLE:
#...
0+80 g       \.tdata	0+4 x
#...
0+8c g       \.tdata	0+4 x2
#...
0+84 g       \.tdata	0+4 z
#...
0+88 g       \.tdata	0+4 x1
#...
Contents of section \.text:
#...
Contents of section \.got:
 8213c 0+ 0+ 0+ 010+  .*
 8214c 80+ 010+ 840+   .*

Disassembly of section \.text:

00080094 <_start>:
   80094:	41b2                	moveq 1,\$r11
#...
00080098 <tlsfn13>:
   80098:	5fae f8ff           	move.w 0xfff8,\$r10
   8009c:	5fae fcff           	move.w 0xfffc,\$r10

000800a0 <tlsdsofn2>:
   800a0:	6fae 0c00 0000      	move.d c <tls128\+0xc>,\$r10
#...

000800a8 <tlsdsofn14>:
   800a8:	5fae 1400           	move.w 0x14,\$r10
