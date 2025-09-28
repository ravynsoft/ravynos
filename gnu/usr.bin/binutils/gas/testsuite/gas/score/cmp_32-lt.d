#as:  -march=score3 -I${srcdir}/${subdir} -EL
#objdump:  -s
#source:  cmp_32.s

.*:     file format elf32-littlescore

Contents of section .text:
 0000 0f440f44 0f440f44 0f440f44 0f440f44  .*
 0010 0f441044 00461f46 10600f60 f061ef61  .*
 0020 10601060 10601060 10601060 10601060  .*
 0030 1062ef63 0b84df7f e8852100 08862100  .*
#pass
