#source: pr20244-4.s
#as: --32
#ld: -m elf_i386 -z noseparate-code
#objdump: -s -j .got

.*: +file format .*

Contents of section .got:
 804908c 83800408 +.... +
#pass
