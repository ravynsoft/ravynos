#source: start1.s
#source: tls128.s
#source: tls-gd-3.s
#source: tls-x.s
#as: --no-underscore --em=criself
#ld: -m crislinux --gc-sections
#objdump: -s -t -r -p

# An executable with a single R_CRIS_32_GD, with gc.  Check that we
# have nothing left but the start symbol and its code.

.*:     file format elf32-cris

Program Header:
    LOAD off    0x0+ vaddr 0x0+80000 paddr 0x0+80000 align 2\*\*13
         filesz 0x0+58 memsz 0x0+58 flags r-x
private flags = 0:

SYMBOL TABLE:
0+80054 l    d  \.text	0+ \.text
0+80054 g       \.text	0+ _start
0+82058 g       \.text	0+ __bss_start
0+82058 g       \.text	0+ _edata
0+82060 g       \.text	0+ _end

Contents of section \.text:
 80054 41b20+                             .*
