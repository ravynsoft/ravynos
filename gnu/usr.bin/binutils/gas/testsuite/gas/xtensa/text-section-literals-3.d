#as: --text-section-literals
#objdump: -ds

.*file format .*xtensa.*
#...
Contents of section .text:
 0000 12345678 .*
#...
00000004 <foo>:
.*4:.*l32r.*a2, 0.*
#...
