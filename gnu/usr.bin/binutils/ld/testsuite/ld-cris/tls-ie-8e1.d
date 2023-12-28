#as: --no-underscore --em=criself
#ld: -m crislinux
#source: start1.s
#source: tls-ie-8e.s
#source: tls128g.s
#source: tls-x.s
#objdump: -d -s -h -t -r -p

# Executable with a single R_CRIS_32_IE, defined in the executable.
# Check that we have proper NPTL/TLS markings and a constant GOT.

.*:     file format elf32-cris

Program Header:
#...
     TLS off    0x0+a0 vaddr 0x0+820a0 paddr 0x0+820a0 align 2\*\*2
         filesz 0x0+84 memsz 0x0+84 flags r--
private flags = 0:

Sections:
#...
  2 .got[ 	]+ 0+10 .*
                  CONTENTS, ALLOC, LOAD, DATA

SYMBOL TABLE:
#...
0+80 g       \.tdata	0+4 x
#...
Contents of section .text:
 80094 41b20000 6fae3021 08000000           .*
#...
Contents of section .got:
 82124 00000000 00000000 00000000 fcffffff  .*

Disassembly of section .text:

00080094 <_start>:
   80094:	41b2                	moveq 1,\$r11
#...
00080098 <tlsfn>:
   80098:	6fae 3021 0800      	move.d 82130 <_GLOBAL_OFFSET_TABLE_\+0xc>,\$r10
#pass
