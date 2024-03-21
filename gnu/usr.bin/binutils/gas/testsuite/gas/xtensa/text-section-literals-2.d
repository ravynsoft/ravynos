#as: --text-section-literals
#objdump: -ds
#Lone literal assembled successfully with --text-section-literals

.*file format .*xtensa.*
#...
Contents of section .text:
 0000 12345678 .*
#...
