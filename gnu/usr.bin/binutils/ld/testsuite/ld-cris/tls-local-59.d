#source: start1.s
#source: tls-x.s
#source: tls-local-59.s
#as: --pic --no-underscore --em=criself -I$srcdir/$subdir
#ld: -m crislinux
#objdump: -s -t -r -p

# An executable with a R_CRIS_32_GOT_GD, a R_CRIS_16_GOT_GD, a
# R_CRIS_32_GOT_TPREL and a R_CRIS_16_GOT_TPREL against the same local
# symbol.  Check that we have proper NPTL/TLS markings and GOT.

.*:     file format elf32-cris

Program Header:
#...
     TLS off .*
         filesz 0x00000084 memsz 0x00000084 flags r--
#...
Contents of section .text:
 80094 41b20000 6fae1000 00006fae 0c000000  .*
 800a4 5fae1000 5fae0c00                    .*
#...
Contents of section \.got:
 82130 0+ 0+ 0+ 80ffffff  .*
 82140 010+ 040+   .*
