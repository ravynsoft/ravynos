#as: --pic --no-underscore --em=criself -I$srcdir/$subdir
#ld: -m crislinux --gc-sections -u gc76fn
#source: start1.s
#source: tls-gc-76.s
#source: tls-hx.s
#objdump: -s -t -r -p

# Executable with a single R_CRIS_32_DTPREL, gc:ed away.
# A GOT reference through a local symbol to a variable survives gc.


.*:     file format elf32-cris

Program Header:
    LOAD off    0x0+ vaddr 0x0+80000 paddr 0x0+80000 align 2\*\*13
         filesz 0x0+80 memsz 0x0+80 flags r-x
    LOAD off    0x0+80 vaddr 0x0+82080 paddr 0x0+82080 align 2\*\*13
         filesz 0x0+14 memsz 0x0+14 flags rw-
private flags = 0:

SYMBOL TABLE:
0+80074 l    d  \.text	0+ \.text
0+82080 l    d  \.got	0+ \.got
0+82090 l    d  \.data	0+ \.data
0+ l    df \*ABS\*	0+ .*
0+82090 l     O \.data	0+4 gc76var
0+ l    df \*ABS\*	0+ .*
0+82080 l     O \.got	0+ _GLOBAL_OFFSET_TABLE_
0+80074 g       \.text	0+ _start
0+82094 g       \.data	0+ __bss_start
0+82094 g       \.data	0+ _edata
0+820a0 g       \.data	0+ _end
0+80078 g     F \.text	0+6 gc76fn

Contents of section \.text:
 80074 41b20+ 6fae0c00 0+  .*
Contents of section \.got:
 82080 0+ 0+ 0+ 90200800  .*
Contents of section \.data:
 82090 0+             .*
