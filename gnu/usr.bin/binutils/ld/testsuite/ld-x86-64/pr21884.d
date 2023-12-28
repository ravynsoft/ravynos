#source: dummy.s
#as: --64
#ld: -m elf_x86_64 -T pr21884.t -b binary
#objdump: -b binary -s

.*:     file format binary

Contents of section .data:
#pass
