#source: start1.s
#source: tls-le-12s.s
#source: tls-z.s
#source: tls128.s
#as: --no-underscore --em=criself -I$srcdir/$subdir
#ld: -m crislinux
#objdump: -d -s -t -r -p -h

# Check that we have proper NPTL/TLS markings and no GOT for an
# executable with a single R_CRIS_16_TPREL.

.*:     file format elf32-cris

Program Header:
#...
     TLS off    0x0+9c vaddr 0x0+8209c paddr 0x0+8209c align 2\*\*2
         filesz 0x0+84 memsz 0x0+84 flags r--
private flags = 0:
#...
  1 .tdata .*
                  CONTENTS.*
SYMBOL TABLE:
#...
0+ g       \.tdata	0+4 z
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
   80098:	7fac 7cff           	movs\.w -132,\$r10
#...
