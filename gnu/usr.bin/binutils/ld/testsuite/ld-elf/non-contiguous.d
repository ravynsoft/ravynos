#name: non-contiguous
#source: non-contiguous.s
#ld: --enable-non-contiguous-regions -T non-contiguous.ld
#objdump: -rdsh
#xfail: xtensa*

.*:     file format .*

Sections:
Idx Name          Size      VMA  *     LMA  *     File off  Algn
  0 \.raml         0000000c  0*1fff0000  0*1fff0000  .*  2\*\*.
                  CONTENTS, ALLOC, LOAD, DATA
  1 \.ramu         00000014  0*20000000  0*1fff000c  .*  2\*\*.
                  CONTENTS, ALLOC, LOAD, DATA
  2 \.ramz         0000003c  0*20040000  0*20000014  .*  2\*\*.
                  CONTENTS, ALLOC, LOAD, DATA


Contents of section .raml:
 1fff0000 (010+ 020+ 030+|0+01 0+02 0+03)           ............    
Contents of section .ramu:
 20000000 (040+ 050+ 060+ 070+|0+04 0+05 0+06 0+07)  ................
 20000010 (080+|0+08)                             ....            
Contents of section .ramz:
 20040000 09090909 09090909 09090909 09090909  ................
 20040010 09090909 09090909 09090909 09090909  ................
 20040020 09090909 09090909 09090909 09090909  ................
 20040030 09090909 09090909 09090909           ............    
