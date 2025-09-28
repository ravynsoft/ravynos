#source: start1.s
#source: tls-dtpoffdx.s
#source: tls-gd-1.s
#source: tls128.s
#source: tls-commx.s
#as: --no-underscore --em=criself --pic -I$srcdir/$subdir
#ld: -m crislinux --shared --hash-style=sysv
#objdump: -d -s -t -r -p

# Linking a file with a DTPOFF from a .dtpoffd of a GD COMMON
# variable, PIC (globally defined here but overridable).  Making sure
# it links and that the offset is correct, locally.

.*:     file format elf32-cris

Program Header:
#...
     TLS off   .*
         filesz 0x0+80 memsz 0x0+84 flags r--
#...
SYMBOL TABLE:
#...
0+80 g       .tbss	0+4 x
#...
Contents of section .rela.dyn:
 0140 50220000 17020000 00000000           .*
Contents of section .text:
 014c 41b20000 5fae0c00                    .*
#...
Contents of section .got:
 2244 d4210000 00000000 00000000 00000000  .*
 2254 00000000                             .*
Contents of section .debug_info:
 0000 80000000                             .*
#...
Disassembly of section .text:
#pass
