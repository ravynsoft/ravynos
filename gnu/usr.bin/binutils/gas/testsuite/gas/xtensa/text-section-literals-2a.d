#as: --auto-litpools
#objdump: -ds
#source: text-section-literals-2.s
#Lone literal assembled successfully with --auto-litpools

.*file format .*xtensa.*
#...
Contents of section .text:
 0000 12345678 .*
#...
