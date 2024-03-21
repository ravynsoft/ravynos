#source: tls-local-54.s
#as: --pic --no-underscore --em=criself -I$srcdir/$subdir
#ld: --shared -m crislinux --hash-style=sysv
#objdump: -s -t -R -p -T

# A DSO with a R_CRIS_32_GOT_GD against a local symbol.
# Check that we have proper NPTL/TLS markings and GOT.

.*:     file format elf32-cris

Program Header:
#...
     TLS off .*
         filesz 0x00000080 memsz 0x00000080 flags r--
#...
DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
00002218 R_CRIS_DTP        \*ABS\*\+0x0000002a

Contents of section .hash:
#...
Contents of section \.text:
 0114 6fae0c00 00000000                    .*
#...
Contents of section \.got:
 220c 9c210000 0+ 0+ 0+  .*
 221c 0+                             .*
