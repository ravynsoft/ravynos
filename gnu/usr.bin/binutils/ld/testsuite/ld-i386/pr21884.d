#source: dummy.s
#as: --32
#ld: -m elf_i386 -T pr21884.t -b binary
#objdump: -b binary -s

.*:     file format binary

Contents of section .data:
#pass
