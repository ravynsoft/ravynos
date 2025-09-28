#source: tls-local-59.s
#as: --pic --no-underscore --em=criself -I$srcdir/$subdir
#ld: -m crislinux --shared --hash-style=sysv
#objdump: -s -t -r -p -R -T

# A DSO with a R_CRIS_32_GOT_GD, a R_CRIS_16_GOT_GD, a
# R_CRIS_32_GOT_TPREL and a R_CRIS_16_GOT_TPREL against the same local
# symbol.  Check that we have proper NPTL/TLS markings and GOT.

.*:     file format elf32-cris

Program Header:
#...
     TLS off .*
         filesz 0x00000080 memsz 0x00000080 flags r--
#...
  FLAGS                0x00000010
#...
DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
0000229c R_CRIS_32_TPREL   \*ABS\*
000022a0 R_CRIS_DTP        \*ABS\*

Contents of section \.hash:
#...
Contents of section \.text:
 0184 6fae1000 00006fae 0c000000 5fae1000  .*
 0194 5fae0c00                             .*
#...
Contents of section \.got:
 2290 18220+ 0+ 0+ 0+  .*
 22a0 0+ 0+                    .*
