#source: start1.s
#source: tls128.s
#source: tls-le-12.s
#source: tls-z.s
#as: --no-underscore --em=criself -I$srcdir/$subdir
#ld: -m crislinux
#objdump: -d -s -t -r -p -h

# Check that we have proper NPTL/TLS markings and no GOT for an
# executable with a single R_CRIS_32_TPREL.

.*:     file format elf32-cris

Program Header:
#...
     TLS off    0x0+a0 vaddr 0x0+820a0 paddr 0x0+820a0 align 2\*\*2
         filesz 0x0+84 memsz 0x0+84 flags r--
private flags = 0:
#...
  1 .tdata .*
                  CONTENTS.*
SYMBOL TABLE:
#...
0+80 g       \.tdata	0+4 z
#...
Contents of section \.text:
#...
Contents of section \.tdata:
#...

Disassembly of section \.text:

00080094 <_start>:
   80094:	41b2                	moveq 1,\$r11
#...
00080098 <tlsfn12>:
   80098:	6fae fcff ffff      	move\.d 0xfffffffc,\$r10
#...
