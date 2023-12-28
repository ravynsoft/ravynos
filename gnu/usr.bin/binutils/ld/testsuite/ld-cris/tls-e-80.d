#source: start1.s
#source: tls-tbss64.s
#source: tls-ie-8e.s
#source: tls-gd-2.s --pic
#source: tls-hx.s
#source: tls-le-12.s
#source: tls-z.s
#source: tls-ld-6.s --pic
#source: tls-ie-10.s --pic
#source: tls-hx1x2.s --pic
#as: --no-underscore --em=criself -I$srcdir/$subdir
#ld: -m crislinux
#objdump: -d -s -h -t -r -p

# Check that we have proper NPTL/TLS markings and GOT for an
# executable with a R_CRIS_32_GOT_GD, a R_CRIS_DTPREL, a
# R_CRIS_32_GOT_TPREL, a R_CRIS_32_TPREL and a R_CRIS_32_IE.

.*:     file format elf32-cris

Program Header:
    LOAD off    0x0+ vaddr 0x0+80000 paddr 0x0+80000 align 2\*\*13
         filesz 0x0+c0 memsz 0x0+c0 flags r-x
    LOAD off    0x0+c0 vaddr 0x0+820c0 paddr 0x0+820c0 align 2\*\*13
         filesz 0x0+30 memsz 0x0+30 flags rw-
     TLS off    0x0+c0 vaddr 0x0+820c0 paddr 0x0+820c0 align 2\*\*2
         filesz 0x0+10 memsz 0x0+50 flags r--
private flags = 0:

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 \.text         0+2c  0+80094  0+80094  0+94  2\*\*1
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 \.tdata        0+10  0+820c0  0+820c0  0+c0  2\*\*2
                  CONTENTS, ALLOC, LOAD, DATA, THREAD_LOCAL
  2 \.tbss         0+40  000820d0  0+820d0  0+d0  2\*\*2
                  ALLOC, THREAD_LOCAL
  3 \.got          0+20  0+820d0  0+820d0  0+d0  2\*\*2
                  CONTENTS, ALLOC, LOAD, DATA
SYMBOL TABLE:
0+80094 l    d  \.text	0+ \.text
0+820c0 l    d  \.tdata	0+ \.tdata
0+820d0 l    d  \.tbss	0+ \.tbss
0+820d0 l    d  \.got	0+ \.got
0+ l       \.tdata	0+4 x
0+820d0 l     O \.got	0+ _GLOBAL_OFFSET_TABLE_
0+800a0 g     F \.text	0+6 tlsdsofn2
0+800a8 g     F \.text	0+6 tlsfn12
0+c g       \.tdata	0+4 \.hidden x2
0+4 g       \.tdata	0+4 z
0+80094 g       \.text	0+ _start
0+80098 g     F \.text	0+6 tlsfn
0+820f0 g       \.got	0+ __bss_start
0+800b0 g     F \.text	0+6 tlsdsofn
0+8 g       \.tdata	0+4 \.hidden x1
0+820f0 g       \.got	0+ _edata
0+82100 g       \.got	0+ _end
0+10 g       \.tbss	0+40 gx
0+800b8 g     F \.text	0+6 tlsdsofn10

Contents of section \.text:
#...
Contents of section \.tdata:
 820c0 28000000 2a000000 29000000 2a000000  .*
Contents of section \.got:
 820d0 00000000 00000000 00000000 01000000  .*
 820e0 00000000 b0ffffff 01000000 00000000  .*

Disassembly of section \.text:

00080094 <_start>:
   80094:	41b2                	moveq 1,\$r11
#...
00080098 <tlsfn>:
   80098:	6fae e420 0800      	move.d 820e4 <_GLOBAL_OFFSET_TABLE_\+0x14>,\$r10
#...
000800a0 <tlsdsofn2>:
   800a0:	6fae 1800 0000      	move\.d 18 <gx\+0x8>,\$r10
#...
000800a8 <tlsfn12>:
   800a8:	6fae b4ff ffff      	move.d 0xffffffb4,\$r10
#...
000800b0 <tlsdsofn>:
   800b0:	6fae 0000 0000      	move\.d 0 <x>,\$r10
#...
000800b8 <tlsdsofn10>:
   800b8:	6fae 1400 0000      	move\.d 14 <gx\+0x4>,\$r10
#...
