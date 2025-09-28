#source: start1.s
#source: tls128.s
#source: tls-le-13.s
#source: tls-x1x2.s
#as: --no-underscore --em=criself -I$srcdir/$subdir
#ld: -m crislinux
#objdump: -d -s -t -r -p -h

# Check that we have proper NPTL/TLS markings and GOT for an
# executable with a single R_CRIS_32_TPREL.

.*:     file format elf32-cris

Program Header:
#...
     TLS off    0x0+a4 vaddr 0x0+820a4 paddr 0x0+820a4 align 2\*\*2
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
   80098:	6fae f8ff ffff      	move\.d 0xfffffff8,\$r10
   8009e:	6fae fcff ffff      	move\.d 0xfffffffc,\$r10
#...
