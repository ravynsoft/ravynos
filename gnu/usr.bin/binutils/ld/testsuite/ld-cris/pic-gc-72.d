#source: pic-gc-72.s
#source: expdref1.s
#source: expdyn1.s
#as: --pic --no-underscore --em=criself -I$srcdir/$subdir
#ld: --shared -m crislinux --gc-sections --hash-style=sysv
#objdump: -s -t -R -p -T

# Exercise PIC relocs through changed GC sweep function.
# There should be nothing left except from expdyn1.s.

.*:     file format elf32-cris
#...
DYNAMIC RELOCATION RECORDS \(none\)

Contents of section .hash:
#...
Contents of section .dynsym:
#...
Contents of section .dynstr:
#...
Contents of section .text:
.* 0f050f05                             .*
Contents of section .dynamic:
 2114 .*
#...
Contents of section .got:
.* 14210000 00000000 00000000           .*
Contents of section .data:
.* 00000000                             .*
