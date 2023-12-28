#as:  -march=score3 -I${srcdir}/${subdir} -EL
#objdump:  -s
#source:  mv_32.s

.*:     file format elf32-littlescore

Contents of section .text:
 0000 0f400f40 0f400f40 0f400f40 0f400f40  .*
 0010 0f400f42 10401042                    .*
#pass
