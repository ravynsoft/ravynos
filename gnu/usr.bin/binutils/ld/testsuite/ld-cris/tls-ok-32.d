#source: tls-dtprelm.s --defsym r=32767
#as: --no-underscore --em=criself --pic
#ld: --shared -m crislinux --hash-style=sysv
#objdump: -s -j .got -j .text -j .tdata -R

# Check that a R_CRIS_16_DTPREL just below the theoretical limit
# works, in a DSO.

.*:     file format elf32-cris

DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
0000a1b0 R_CRIS_DTPMOD     \*ABS\*

Contents of section \.text:
 0130 5faeff7f                             .*
Contents of section \.tdata:
 2134 2a2a2a2a 2a2a2a2a 2a2a2a2a 2a2a2a2a  .*
#...
 a124 2a2a2a2a 2a2a2a2a 2a2a2a2a 2a2a2a2a  .*
Contents of section \.got:
 a1a4 34a10000 00000000 00000000 00000000  .*
 a1b4 00000000                             .*
