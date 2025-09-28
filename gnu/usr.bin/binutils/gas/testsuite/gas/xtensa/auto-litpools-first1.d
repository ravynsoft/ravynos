#as: --auto-litpools
#objdump: -ds
#name: auto-litpools-first1 (check that literal pool is created when source starts with literal)

.*: +file format .*xtensa.*
#...
Contents of section .text:
 0000 20170331 .*
#...
00000004 <f>:
.*4:.*l32r.*a2, 0.*
#...
