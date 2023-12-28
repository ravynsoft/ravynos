#as:  -march=score3 -I${srcdir}/${subdir} -EL
#objdump:  -s
#source:  logical_32.s

.*:     file format elf32-littlescore

Contents of section .text:
 0000 0f4b0f4b 0f4b0f4b 0f4b0f4b 0f4b0f4b  .*
 0010 0f4b0080 213c0080 20401082 20001082  .*
 0020 20440180 20080f4a 0f4a0f4a 0f4a0f4a  .*
 0030 0f4a0f4a 0f4a0f4a 0080233c 00802240  .*
 0040 10822200 10822244 01802208           .*
#pass
