#as: --auto-litpools
#objdump: -ds
#name: auto-litpools-first2 (check that literal pool with jump around is created for generated literal)

.*: +file format .*xtensa.*
#...
Contents of section .text:
 0000 20170331 .*
#...
00000004 <f>:
   4:.*addi.*a1.*
   7:.*l32r.*a2, 0.*
#...
