#source: start1.s
#source: tls128.s
#source: tls-ld-7.s
#source: tls-gd-2.s
#source: tls-ldgd-15.s
#source: tls-x.s
#source: tls-z.s
#source: tls-hx1x2.s
#as: --pic --no-underscore --em=criself
#ld: -m crislinux
#objdump: -d -s -h -t -r -p

# Check that we have proper NPTL/TLS markings and GOT for two
# R_CRIS_32_GOT_GD and two R_CRIS_32_DTPRELs against different
# variables, for an executable.

.*:     file format elf32-cris

Program Header:
#...
     TLS off    0x0+b4 vaddr 0x0+820b4 paddr 0x0+820b4 align 2\*\*2
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
 82144 00000000 00000000 00000000 01000000  .*
 82154 00000000 01000000 80000000 01000000  .*
 82164 84000000   .*

Disassembly of section \.text:

00080094 <_start>:
   80094:	41b2                	moveq 1,\$r11
#...
00080098 <tlsdsofn>:
   80098:	6fae 8800 0000      	move.d 88 <x1>,\$r10
   8009e:	6fbe 8c00 0000      	move.d 8c <x2>,\$r11
000800a4 <tlsdsofn2>:
   800a4:	6fae 1400 0000      	move.d 14 <tls128\+0x14>,\$r10
#...
000800ac <tlsdsofn14>:
   800ac:	6fae 1c00 0000      	move.d 1c <tls128\+0x1c>,\$r10
#...
