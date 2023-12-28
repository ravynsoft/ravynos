#source: start1.s
#source: tls-e-tpiecomm1.s
#as: --no-underscore --em=criself -I$srcdir/$subdir
#ld: -m crislinux
#objdump: -d -s -h -t -r -p

# Make sure we can link a file with IE relocs against common
# symbols and that the values entered in the GOT are right.

.*:     file format elf32-cris

Program Header:
    LOAD off    0x0+ vaddr 0x0+80000 paddr 0x0+80000 align 2\*\*13
         filesz 0x0+a4 memsz 0x0+a4 flags r-x
    LOAD off    0x0+a4 vaddr 0x0+820a4 paddr 0x0+820a4 align 2\*\*13
         filesz 0x0+14 memsz 0x0+14 flags rw-
     TLS off    0x0+a4 vaddr 0x0+820a4 paddr 0x0+820a4 align 2\*\*2
         filesz 0x0+ memsz 0x0+8 flags r--
private flags = 0:

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 \.text         0+10  0+80094  0+80094  0+94  2\*\*1
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 \.tbss         0+8  0+820a4  0+820a4  0+a4  2\*\*2
                  ALLOC, THREAD_LOCAL
  2 \.got          0+14  0+820a4  0+820a4  0+a4  2\*\*2
                  CONTENTS, ALLOC, LOAD, DATA
SYMBOL TABLE:
#...
0+ g       .tbss	0+4 foo
#...
0+4 g       .tbss	0+4 bar
#...
Contents of section \.got:
 820a4 00000000 00000000 00000000 f8ffffff  .*
 820b4 fcffffff                             .*

Disassembly of section \.text:

0+80094 <_start>:
   80094:	41b2                	moveq 1,\$r11
	\.\.\.

0+80098 <do_test>:
   80098:	6f0e b020 0800      	move\.d 820b0 <_GLOBAL_OFFSET_TABLE_\+0xc>,\$r0
   8009e:	2f1e b420 0800      	add\.d 820b4 <_GLOBAL_OFFSET_TABLE_\+0x10>,\$r1
