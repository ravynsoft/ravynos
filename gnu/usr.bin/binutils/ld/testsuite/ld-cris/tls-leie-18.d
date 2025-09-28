#source: start1.s
#source: tls128.s
#source: tls-le-13s.s
#source: tls-ie-9.s --pic
#source: tls-x1x2.s
#as: --no-underscore --em=criself
#ld: -m crislinux
#objdump: -d -s -h -t -r -p

# Check that we have proper NPTL/TLS markings and GOT for an
# executable with two R_CRIS_16_TPREL and two R_CRIS_16_GOT_TPREL, no
# same reloc to same symbol; two different symbols.

.*:     file format elf32-cris

Program Header:
#...
     TLS off    0x0+a8 vaddr 0x0+820a8 paddr 0x0+820a8 align 2\*\*2
         filesz 0x0+88 memsz 0x0+88 flags r--
private flags = 0:
#...
  2 .got .*
                  CONTENTS.*
SYMBOL TABLE:
#...
0+84 g       \.tdata	0+4 x2
#...
0+80 g       \.tdata	0+4 x1
#...
Contents of section \.text:
#...
Contents of section \.got:
 82130 0+ 0+ 0+ fcffffff  .*
 82140 f8ffffff  .*

Disassembly of section \.text:

00080094 <_start>:
   80094:	41b2                	moveq 1,\$r11
#...
00080098 <tlsfn13>:
   80098:	5fae f8ff           	move.w 0xfff8,\$r10
   8009c:	5fae fcff           	move.w 0xfffc,\$r10

000800a0 <tlsdsofn9>:
   800a0:	5fae 1000           	move.w 0x10,\$r10
   800a4:	5fbe 0c00           	move.w 0xc,\$r11
