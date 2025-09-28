#source: start1.s
#source: tls-gd-2.s --pic
#source: tls-hx.s
#source: tls-ld-6.s --pic
#source: tls-ie-10.s --pic
#source: tls-hx1x2.s --pic
#as: --no-underscore --em=criself -I$srcdir/$subdir
#ld: -m crislinux
#objdump: -d -s -h -t -r -p

# Check that we have proper NPTL/TLS markings and GOT for an
# executable with a R_CRIS_32_GOT_GD, a R_CRIS_DTPREL and a
# R_CRIS_32_GOT_TPREL.

.*:     file format elf32-cris

Program Header:
    LOAD off    0x0+ vaddr 0x0+80000 paddr 0x0+80000 align 2\*\*13
         filesz 0x0+b0 memsz 0x0+b0 flags r-x
    LOAD off    0x0+b0 vaddr 0x0+820b0 paddr 0x0+820b0 align 2\*\*13
         filesz 0x0+2c memsz 0x0+2c flags rw-
     TLS off    0x0+b0 vaddr 0x0+820b0 paddr 0x0+820b0 align 2\*\*2
         filesz 0x0+c memsz 0x0+c flags r--
private flags = 0:

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 \.text         0000001c  00080094  00080094  00000094  2\*\*1
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 \.tdata        0000000c  000820b0  000820b0  000000b0  2\*\*2
                  CONTENTS, ALLOC, LOAD, DATA, THREAD_LOCAL
  2 \.got          00000020  000820bc  000820bc  000000bc  2\*\*2
                  CONTENTS, ALLOC, LOAD, DATA
SYMBOL TABLE:
0+80094 l    d  \.text	0+ \.text
0+820b0 l    d  \.tdata	0+ \.tdata
0+820bc l    d  \.got	0+ \.got
0+ l       \.tdata	0+4 x
0+820bc l     O \.got	0+ _GLOBAL_OFFSET_TABLE_
0+80098 g     F \.text	0+6 tlsdsofn2
0+8 g       \.tdata	0+4 \.hidden x2
0+80094 g       \.text	0+ _start
0+820dc g       \.got	0+ __bss_start
0+800a0 g     F \.text	0+6 tlsdsofn
0+4 g       \.tdata	0+4 \.hidden x1
0+820dc g       \.got	0+ _edata
0+820e0 g       \.got	0+ _end
0+800a8 g     F \.text	0+6 tlsdsofn10

Contents of section \.text:
#...
Contents of section \.tdata:
 820b0 28000000 29000000 2a000000           .*
Contents of section \.got:
 820bc 00000000 00000000 00000000 01000000  .*
 820cc 00000000 f4ffffff 01000000 00000000  .*

Disassembly of section \.text:

00080094 <_start>:
   80094:	41b2                	moveq 1,\$r11
#...
00080098 <tlsdsofn2>:
   80098:	6fae 1800 0000      	move\.d 18 <x2\+0x10>,\$r10
#...
000800a0 <tlsdsofn>:
   800a0:	6fae 0000 0000      	move\.d 0 <x>,\$r10
#...
000800a8 <tlsdsofn10>:
   800a8:	6fae 1400 0000      	move\.d 14 <x2\+0xc>,\$r10
#...
