#source: start1.s
#source: tls128.s
#source: tls-ld-5.s
#source: tls-gd-1.s
#source: tls-ldgd-14.s
#source: tls-x.s
#source: tls-z.s
#source: tls-hx1x2.s
#as: --pic --no-underscore --em=criself
#ld: -m crislinux
#objdump: -d -s -h -t -r -p

# Check that we have proper NPTL/TLS markings and GOT for two
# R_CRIS_16_GOT_GD and two R_CRIS_16_DTPRELs against different
# variables, for an executable.

.*:     file format elf32-cris

Program Header:
#...
     TLS off    0x0+a8 vaddr 0x0+820a8 paddr 0x0+820a8 align 2\*\*2
         filesz 0x0+90 memsz 0x0+90 flags r--
private flags = 0:
#...
  1 .tdata .*
                  CONTENTS.*
  2 .got .*
                  CONTENTS.*
SYMBOL TABLE:
#...
0+80 g       \.tdata	0+4 x
#...
0+8c g       .tdata	00000004 \.hidden x2
#...
0+84 g       \.tdata	0+4 z
#...
0+88 g       .tdata	00000004 \.hidden x1
#...
Contents of section \.text:
#...
Contents of section \.tdata:
#...
Contents of section \.got:
 82138 00000000 00000000 00000000 01000000  .*
 82148 00000000 01000000 80000000 01000000  .*
 82158 84000000   .*

Disassembly of section \.text:

00080094 <_start>:
   80094:	41b2                	moveq 1,\$r11
#...
00080098 <tlsdsofn>:
   80098:	5fae 8800           	move\.w 0x88,\$r10
   8009c:	5fbe 8c00           	move\.w 0x8c,\$r11

000800a0 <tlsdsofn0>:
   800a0:	5fae 1400           	move\.w 0x14,\$r10

000800a4 <tlsdsofn14>:
   800a4:	5fae 1c00           	move\.w 0x1c,\$r10
