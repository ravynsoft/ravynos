#source: start1.s
#source: tls-dso-tpoffgotcomm1.s --pic
#as: --no-underscore --em=criself -I$srcdir/$subdir
#ld: -m crislinux
#objdump: -d -s -h -t -p

# Like tls-dso-tpoffgotcomm1.d but making sure we can link *an
# executable* with TPOFFGOT relocs against common symbols.

.*:     file format elf32-cris

Program Header:
    LOAD off    0x0+ vaddr 0x0+80000 paddr 0x0+80000 align 2\*\*13
         filesz 0x0+a4 memsz 0x0+a4 flags r-x
    LOAD off    .*
         filesz .*
     TLS off    0x0+a4 vaddr .* paddr .* align 2\*\*2
         filesz 0x0+ memsz 0x0+8 flags r--
private flags = 0:

Sections:
#...
  2 \.got          0+14  0+820a4  0+820a4  0+a4  2\*\*2
                  CONTENTS, ALLOC, LOAD, DATA
SYMBOL TABLE:
#...
0+ g       \.tbss	0+4 foo
#...
0+4 g       \.tbss	0+4 bar
#...
Contents of section .got:
 820a4 00000000 00000000 00000000 f8ffffff  .*
 820b4 fcffffff                             .*

Disassembly of section \.text:

0+80094 <_start>:
   80094:	41b2                	moveq 1,\$r11
#...
0+80098 <do_test>:
   80098:	2f0e 0c00 0000      	add.d c <bar\+0x8>,\$r0
   8009e:	1f1e 1000           	add.w 0x10,\$r1
	\.\.\.
