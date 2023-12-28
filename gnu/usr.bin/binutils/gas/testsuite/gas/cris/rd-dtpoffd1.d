#objdump: -sr
#as: --em=criself

# Checking .dtpoffd output.

.*:     file format .*-cris

RELOCATION RECORDS FOR \[.text\]:
OFFSET +TYPE +VALUE
0+4 R_CRIS_32_DTPREL  extsym\+0x0000002a
0+c R_CRIS_32_DTPREL  x\+0x00000002

Contents of section .text:
 0000 54686973 00000000 69732061 00000000  .*
 0010 99665655                             .*
Contents of section .tdata:
 0000 00000000                             .*
