#source: pr20244-1.s
#as: --32
#ld: -m elf_i386 -z noseparate-code
#objdump: -s -j .got

.*: +file format .*

Contents of section .got:
 804908c a0900408 a1900408 +........ +
#pass
