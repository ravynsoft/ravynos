#name: Z80/Z180 arch combination test
#source: dummy1.s -march=z80
#source: dummy2.s -march=z180
#ld: -e 0
#objdump: -f

.*:[     ]+file format (coff)|(elf32)\-z80
architecture: z180, flags 0x[0-9a-fA-F]+:
.*
.*

