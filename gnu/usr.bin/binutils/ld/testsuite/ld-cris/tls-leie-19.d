#source: start1.s
#source: tls128.s
#source: tls-leie-19.s
#source: tls-le-12.s
#source: tls-ie-11.s --pic
#source: tls-hx1x2.s
#source: tls-x.s
#source: tls-z.s
#as: --no-underscore --em=criself
#ld: -m crislinux
#objdump: -d -s -h -t -r -p

# Check that we have proper NPTL/TLS markings and GOT for an
# executable with two R_CRIS_32_TPREL and two R_CRIS_32_GOT_TPREL,
# four different symbols.

.*:     file format elf32-cris

Program Header:
#...
     TLS off    0x0+b4 vaddr 0x0+820b4 paddr 0x0+820b4 align 2\*\*2
         filesz 0x0+90 memsz 0x0+90 flags r--
private flags = 0:
#...
  2 .got .*
                  CONTENTS.*
SYMBOL TABLE:
#...
0+84 l       \.tdata	0+4 x2
#...
0+80 l       \.tdata	0+4 x1
#...
0+88 g       .tdata	0+4 x
#...
0+8c g       .tdata	0+4 z
#...
Contents of section \.text:
#...
Contents of section \.got:
 82144 0+ 0+ 0+ f4ffffff  .*
 82154 f0ffffff  .*

Disassembly of section \.text:

00080094 <_start>:
   80094:	41b2                	moveq 1,\$r11
#...
00080098 <tlsfn19>:
   80098:	6fae f8ff ffff      	move.d 0xfffffff8,\$r10
#...

000800a0 <tlsfn12>:
   800a0:	6fae fcff ffff      	move.d 0xfffffffc,\$r10
#...

000800a8 <tlsdsofn>:
   800a8:	6fae 1000 0000      	move.d 10 <tls128\+0x10>,\$r10
   800ae:	6fbe 0c00 0000      	move.d c <tls128\+0xc>,\$r11
