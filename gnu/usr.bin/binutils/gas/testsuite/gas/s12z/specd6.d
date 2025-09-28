#objdump: -d
#name:    
#source:  specd6.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	fc          	cmp x, y
   1:	fd          	sub d6, x, y
   2:	fe          	sub d6, y, x
