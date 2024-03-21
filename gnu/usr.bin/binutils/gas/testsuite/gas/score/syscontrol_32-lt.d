#as:  -march=score3 -I${srcdir}/${subdir} -EL
#objdump:  -s
#source:  syscontrol_32.s

.*:     file format elf32-littlescore

Contents of section .text:
 0000 20003f00 20002000 20002000 20002000  .*
 0010 20002000 00000000 00000000 00000000  .*
 0020 00000000 00000000                    .*
#pass
