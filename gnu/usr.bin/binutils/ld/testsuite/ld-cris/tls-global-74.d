#source: start1.s
#source: tls-x.s
#source: tls-gd-2.s
#source: tls-ie-10.s
#source: tls-gd-1.s
#source: tls-ie-8.s
#as: --pic --no-underscore --em=criself -I$srcdir/$subdir
#ld: -m crislinux
#objdump: -s -t -r -p

# An executable with a R_CRIS_32_GOT_GD, a R_CRIS_16_GOT_GD, a
# R_CRIS_32_GOT_TPREL and a R_CRIS_16_GOT_TPREL against the same
# symbol.  Check that we have proper NPTL/TLS markings and GOT.

.*:     file format elf32-cris

Program Header:
#...
     TLS off .*
         filesz 0x0+4 memsz 0x0+4 flags r--
#...
Contents of section .text:
 80094 41b20000 6fae1000 0+ 6fae0c0+  .*
 800a4 0+ 5fae1000 5fae0c00        .*
#...
Contents of section \.got:
 820b4 0+ 0+ 0+ fcffffff  .*
 820c4 010+ 0+   .*
