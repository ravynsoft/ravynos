#source: start1.s
#source: tls128.s
#source: tls-le-13s.s
#source: tls-x1x2.s
#as: --no-underscore --em=criself -I$srcdir/$subdir
#ld: -m crislinux
#objdump: -d -s -t -r -p -h

# Check that we have proper NPTL/TLS markings and GOT for an
# executable with two R_CRIS_16_TPREL, different symbols.

.*:     file format elf32-cris

Program Header:
#...
     TLS off    0x0+a0 vaddr 0x0+820a0 paddr 0x0+820a0 align 2\*\*2
         filesz 0x0+88 memsz 0x0+88 flags r--
private flags = 0:
#...
  1 .tdata .*
                  CONTENTS.*
SYMBOL TABLE:
#...
0+84 g       \.tdata	0+4 x2
#...
0+80 g       \.tdata	0+4 x1
#...
Contents of section \.text:
#...
Contents of section \.tdata:
#...

Disassembly of section \.text:

00080094 <_start>:
   80094:	41b2                	moveq 1,\$r11
#...
00080098 <tlsfn13>:
   80098:	5fae f8ff           	move\.w 0xfff8,\$r10
   8009c:	5fae fcff           	move\.w 0xfffc,\$r10
