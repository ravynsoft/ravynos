#source: start1.s
#source: tls128.s
#source: tls-le-13.s
#source: tls-gd-3.s
#source: tls-legd-16.s
#source: tls-x.s
#source: tls-z.s
#source: tls-x1x2.s
#as: --no-underscore --em=criself
#ld: -m crislinux
#objdump: -d -s -h -t -r -p

# Check that we have proper NPTL/TLS markings and GOT for an
# executable with two R_CRIS_32_TPREL and two R_CRIS_32_GD, different
# symbols.

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
0+80 g       \.tdata	0+4 x
#...
0+8c g       \.tdata	0+4 x2
#...
0+84 g       \.tdata	0+4 z
#...
0+88 g       \.tdata	0+4 x1
#...
Contents of section \.text:
#...
Contents of section \.got:
 82144 0+ 0+ 0+ 010+  .*
 82154 80+ 010+ 840+   .*

Disassembly of section \.text:

00080094 <_start>:
   80094:	41b2                	moveq 1,\$r11
#...
00080098 <tlsfn13>:
   80098:	6fae f8ff ffff      	move.d 0xfffffff8,\$r10
   8009e:	6fae fcff ffff      	move.d 0xfffffffc,\$r10

000800a4 <tlsfn>:
   800a4:	6fae 5021 0800      	move.d 82150 <_GLOBAL_OFFSET_TABLE_\+0xc>,\$r10
#...

000800ac <tlsfn16>:
   800ac:	6fae 5821 0800      	move.d 82158 <_GLOBAL_OFFSET_TABLE_\+0x14>,\$r10
#...
