#source: start1.s
#source: tls-dtpoffdx.s
#source: tls-gd-1.s
#source: tls128.s
#source: tls-commx.s
#as: --no-underscore --em=criself --pic -I$srcdir/$subdir
#ld: -m crislinux
#objdump: -d -s -t -r -p

.*:     file format elf32-cris

Program Header:
#...
     TLS off  .*
         filesz 0x0+80 memsz 0x0+84 flags r--
private flags = 0:

SYMBOL TABLE:
#...
0+80 g       .tbss	0+4 x
#...
Contents of section .text:
 80094 41b20000 5fae0c00                    .*
Contents of section .tdata:
#...
Contents of section .got:
 8211c 00000000 00000000 00000000 01000000  .*
 8212c 80000000                             .*
Contents of section .debug_info:
 0000 80000000                             .*
Contents of section .debug_line:
#pass
