#as:  -march=score3 -I${srcdir}/${subdir} -EL
#objdump:  -s
#source:  shift_32.s

.*:     file format elf32-littlescore

Contents of section .text:
 0000 00581f58 e059ff59 00580058 00580058  .*
 0010 00580058 00580058 00807100 02807000  .*
 0020 10827000 005a1f5a e05bff5b 005a005a  .*
 0030 005a005a 005a005a 005a005a 00807500  .*
 0040 02807400 10827400                    .*
#pass
